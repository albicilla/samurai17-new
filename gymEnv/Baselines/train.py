import gymEnv3

import argparse
import gym
import numpy as np
import os
import tensorflow as tf
import tempfile
import time
import json

import baselines.common.tf_util as U

from baselines import logger
from baselines import deepq
from baselines.deepq.replay_buffer import ReplayBuffer, PrioritizedReplayBuffer
from baselines.common.misc_util import (
    boolean_flag,
    pickle_load,
    pretty_eta,
    relatively_safe_pickle_dump,
    set_global_seeds,
    RunningAvg,
)
from baselines.common.schedules import LinearSchedule, PiecewiseSchedule
from baselines import bench
# from baselines.common.atari_wrappers_deprecated import wrap_dqn
from baselines.common.azure_utils import Container
from model import model, dueling_model



def parse_args():
    parser = argparse.ArgumentParser("DQN experiments for Atari games")
    # Environment
    # 直接書き換えたので関係なし
    parser.add_argument("--env", type=str, default="Pong", help="name of the game")
    parser.add_argument("--seed", type=int, default=42, help="which seed to use")
    # Core DQN parameters
    # parser.add_argument("--replay-buffer-size", type=int, default=int(1e6), help="replay buffer size")
    parser.add_argument("--replay-buffer-size", type=int, default=int(5e5), help="replay buffer size")
    parser.add_argument("--lr", type=float, default=1e-4, help="learning rate for Adam optimizer")
    # 2 * 100,000,000 200M
    # 1億step
    # parser.add_argument("--num-steps", type=int, default=int(2e8), help="total number of steps to run the environment for")
    parser.add_argument("--num-steps", type=int, default=int(1e7), help="total number of steps to run the environment for")
    parser.add_argument("--batch-size", type=int, default=32, help="number of transitions to optimize at the same time")
    # 何stepにつき一度学習するか
    # args.learning_freqで使われている
    # batch-size=32なので、1observationが平均8回使われる?
    parser.add_argument("--learning-freq", type=int, default=4, help="number of iterations between every optimization step")
    # parser.add_argument("--target-update-freq", type=int, default=40000, help="number of iterations between every target network update")
    parser.add_argument("--target-update-freq", type=int, default=4000, help="number of iterations between every target network update")
    parser.add_argument("--param-noise-update-freq", type=int, default=50, help="number of iterations between every re-scaling of the parameter noise")
    parser.add_argument("--param-noise-reset-freq", type=int, default=10000, help="maximum number of steps to take per episode before re-perturbing the exploration policy")
    # Bells and whistles
    boolean_flag(parser, "double-q", default=True, help="whether or not to use double q learning")
    boolean_flag(parser, "dueling", default=True, help="whether or not to use dueling model")
    boolean_flag(parser, "prioritized", default=True, help="whether or not to use prioritized replay buffer")
    # e-greedyの代わりに、ネットワークの重みにノイズを入れる
    boolean_flag(parser, "param-noise", default=False, help="whether or not to use parameter space noise for exploration")
    # layer-normalization、layer上のニューロン間で出力を正規化
    # CNNと相性が良くないかも？   
    # とりあえず入れない
    boolean_flag(parser, "layer-norm", default=False, help="whether or not to use layer norm (should be True if param_noise is used)")
    boolean_flag(parser, "gym-monitor", default=False, help="whether or not to use a OpenAI Gym monitor (results in slower training due to video recording)")
    parser.add_argument("--prioritized-alpha", type=float, default=0.6, help="alpha parameter for prioritized replay buffer")
    parser.add_argument("--prioritized-beta0", type=float, default=0.4, help="initial value of beta parameters for prioritized replay")
    parser.add_argument("--prioritized-eps", type=float, default=1e-6, help="eps parameter for prioritized replay buffer")
    # Checkpointing
    # Noneでも保存される
    parser.add_argument("--save-dir", type=str, default="save", help="directory in which training state and model should be saved.")
    parser.add_argument("--save-azure-container", type=str, default=None,
                        help="It present data will saved/loaded from Azure. Should be in format ACCOUNT_NAME:ACCOUNT_KEY:CONTAINER")
    # 1Mごとに保存
    # parser.add_argument("--save-freq", type=int, default=1e6, help="save model once every time this many iterations are completed")
    # debugは100k = 1e5ごとの保存のが良さそう
    parser.add_argument("--save-freq", type=int, default=1e5, help="save model once every time this many iterations are completed")
    boolean_flag(parser, "load-on-start", default=True, help="if true and model was previously saved then training will be resumed")
    return parser.parse_args()


# 2/8 OK
def make_env(game_name):
    # env = gym.make(game_name + "NoFrameskip-v4")
    env = gym.make("Samurai-v0")
    # 必要っぽい？
    monitored_env = bench.Monitor(env, logger.get_dir())  # puts rewards and number of steps in info, before environment is wrapped
    print("save dir: " + str(logger.get_dir()))
    # env = deepq.wrap_atari_dqn(monitored_env)  # applies a bunch of modification to simplify the observation space (downsample, make b/w)
    return monitored_env, monitored_env


# 2/8 ok
def maybe_save_model(savedir, container, state):
    """This function checkpoints the model and state of the training algorithm."""
    if savedir is None:
        return
    start_time = time.time()
    model_dir = "model-{}".format(state["num_iters"])
    U.save_state(os.path.join(savedir, model_dir, "saved"))
    if container is not None:
        container.put(os.path.join(savedir, model_dir), model_dir)
    # state = {replay_buffer, num_iters}
    relatively_safe_pickle_dump(state, os.path.join(savedir, 'training_state.pkl.zip'), compression=True)
    if container is not None:
        container.put(os.path.join(savedir, 'training_state.pkl.zip'), 'training_state.pkl.zip')
    #### TODO: !!!!
    # relatively_safe_pickle_dump(state["monitor_state"], os.path.join(savedir, 'monitor_state.pkl'))
    if container is not None:
        container.put(os.path.join(savedir, 'monitor_state.pkl'), 'monitor_state.pkl')
    logger.log("Saved model in {} seconds\n".format(time.time() - start_time))


# 2/8 ok
# maybe_save_modelしたものをload
def maybe_load_model(savedir, container):
    """Load model if present at the specified path."""
    if savedir is None:
        return

    state_path = os.path.join(os.path.join(savedir, 'training_state.pkl.zip'))
    if container is not None:
        logger.log("Attempting to download model from Azure")
        found_model = container.get(savedir, 'training_state.pkl.zip')
    else:
        found_model = os.path.exists(state_path)
    if found_model:
        state = pickle_load(state_path, compression=True)
        model_dir = "model-{}".format(state["num_iters"])
        if container is not None:
            container.get(savedir, model_dir)
        U.load_state(os.path.join(savedir, model_dir, "saved"))
        logger.log("Loaded models checkpoint at {} iterations".format(state["num_iters"]))
        return state


if __name__ == '__main__':
    print('start')
    args = parse_args()
    # Parse savedir and azure container.
    # default 'save'
    savedir = args.save_dir
    if savedir is None:
        savedir = os.getenv('OPENAI_LOGDIR', None)
    if args.save_azure_container is not None:
        account_name, account_key, container_name = args.save_azure_container.split(":")
        container = Container(account_name=account_name,
                              account_key=account_key,
                              container_name=container_name,
                              maybe_create=True)
        if savedir is None:
            # Careful! This will not get cleaned up. Docker spoils the developers.
            savedir = tempfile.TemporaryDirectory().name
    else:
        container = None
    # Create and seed the env.
    env, monitored_env = make_env(args.env)
    print("observation_space: " + str(env.observation_space.shape))
    print("action_shape: " + str(env.action_space.shape))
    if args.seed > 0:
        set_global_seeds(args.seed)
        env.unwrapped.seed(args.seed)

    # default False
    if args.gym_monitor and savedir:
        env = gym.wrappers.Monitor(env, os.path.join(savedir, 'gym_monitor'), force=True)

    if savedir:
        os.makedirs(savedir, exist_ok=True)
        file_path = os.path.join(savedir, 'args.json')
        print("save_file: " + str(file_path))
        with open(file_path, 'w') as f:
            json.dump(vars(args), f)

    with U.make_session(4) as sess:
        # Create training graph and replay buffer
        def model_wrapper(img_in, num_actions, scope, **kwargs):
            actual_model = dueling_model if args.dueling else model
            return actual_model(img_in, num_actions, scope, layer_norm=args.layer_norm, **kwargs)
        act, train, update_target, debug = deepq.build_train(
            make_obs_ph=lambda name: U.myUint8Input(env.observation_space.shape, name=name),
            q_func=model_wrapper,
            num_actions=env.action_space.n,
            optimizer=tf.train.AdamOptimizer(learning_rate=args.lr, epsilon=1e-4),
            gamma=0.99,
            grad_norm_clipping=10,
            double_q=args.double_q,
            param_noise=args.param_noise
        )

        approximate_num_iters = args.num_steps / 4
        # exploration = PiecewiseSchedule([
        #     (0, 1.0),
        #     (approximate_num_iters / 50, 0.1),
        #     (approximate_num_iters / 5, 0.01)
        # ], outside_value=0.01)

        # exploration = PiecewiseSchedule([
        #     (0, 1.0),
        #     (args.num_steps * 1/3, 0.1),
        #     (args.num_steps * 2/3, 0.01)
        # ], outside_value=0.01)

        exploration = PiecewiseSchedule([
            (0, 1.0),
            (args.num_steps * 1/3, 0.1),
            (args.num_steps * 2/3, 0.01)
        ], outside_value=0.01)

        if args.prioritized:
            replay_buffer = PrioritizedReplayBuffer(args.replay_buffer_size, args.prioritized_alpha)
            beta_schedule = LinearSchedule(approximate_num_iters, initial_p=args.prioritized_beta0, final_p=1.0)
        else:
            replay_buffer = ReplayBuffer(args.replay_buffer_size)

        U.initialize()
        update_target()
        num_iters = 0
        rewards_arr = []

        # Load the model
        if args.load_on_start:
            state = maybe_load_model(savedir, container)
            if state is not None:
                num_iters, replay_buffer = state["num_iters"], state["replay_buffer"],
                rewards_arr = state["rewards_arr"]
                # TODO: monitor
                # monitored_env.set_state(state["monitor_state"])

        start_time, start_steps = None, None
        steps_per_iter = RunningAvg(0.999)
        iteration_time_est = RunningAvg(0.999)
        prev_print = 0
        # loadするときはかならずgymがresetされる
        # stepを保存することは考えなくて良い
        obs = env.reset()
        num_iters_since_reset = 0
        reset = True

        epi_reward = 0

        # Main training loop
        while True:
            num_iters += 1
            if num_iters % 20000 == 0:
                print("iters: " + str(num_iters))
            num_iters_since_reset += 1

            # Take action and store transition in the replay buffer.
            kwargs = {}
            if not args.param_noise:
                update_eps = exploration.value(num_iters)
                update_param_noise_threshold = 0.
            else:
                if args.param_noise_reset_freq > 0 and num_iters_since_reset > args.param_noise_reset_freq:
                    # Reset param noise policy since we have exceeded the maximum number of steps without a reset.
                    reset = True

                update_eps = 0.01  # ensures that we cannot get stuck completely
                # Compute the threshold such that the KL divergence between perturbed and non-perturbed
                # policy is comparable to eps-greedy exploration with eps = exploration.value(t).
                # See Appendix C.1 in Parameter Space Noise for Exploration, Plappert et al., 2017
                # for detailed explanation.
                update_param_noise_threshold = -np.log(1. - exploration.value(num_iters) + exploration.value(num_iters) / float(env.action_space.n))
                kwargs['reset'] = reset
                kwargs['update_param_noise_threshold'] = update_param_noise_threshold
                kwargs['update_param_noise_scale'] = (num_iters % args.param_noise_update_freq == 0)

            # x = 0
            # y = 0
            # actionConversion = [
            #     [0,1,2],
            #     [3,4,5],
            #     [6,7,8]
            # ]
            # speed = env.map.player.speed
            # if speed[0] > 5:
            #     x = -1
            # elif speed[0] < -5:
            #     x = 1
            # if speed[1] > 5:
            #     y = -1
            # elif speed[1] < -5:
            #     y = 1
            # if x == 0 and y == 0:
            #     action = act(np.array(obs)[None], stochastic=False, update_eps=update_eps, **kwargs)[0]
            # else:
            #     action = 
            action = act(np.array(obs)[None], update_eps=update_eps, **kwargs)[0]
            reset = False
            new_obs, rew, done, info = env.step(action)
            epi_reward += rew
            replay_buffer.add(obs, action, rew, new_obs, float(done))
            obs = new_obs
            if done:
                rewards_arr.append(epi_reward)
                epi_reward = 0
                if len(rewards_arr) % 100 == 0:
                    print("render for viewer")
                    env.render(mode='viewer')
                num_iters_since_reset = 0
                obs = env.reset()
                reset = True

            if (num_iters > max(5 * args.batch_size, args.replay_buffer_size // 20) and
                    num_iters % args.learning_freq == 0):
                # Sample a bunch of transitions from replay buffer
                if args.prioritized:
                    experience = replay_buffer.sample(args.batch_size, beta=beta_schedule.value(num_iters))
                    (obses_t, actions, rewards, obses_tp1, dones, weights, batch_idxes) = experience
                else:
                    obses_t, actions, rewards, obses_tp1, dones = replay_buffer.sample(args.batch_size)
                    weights = np.ones_like(rewards)
                # Minimize the error in Bellman's equation and compute TD-error
                td_errors = train(obses_t, actions, rewards, obses_tp1, dones, weights)
                # Update the priorities in the replay buffer
                if args.prioritized:
                    new_priorities = np.abs(td_errors) + args.prioritized_eps
                    replay_buffer.update_priorities(batch_idxes, new_priorities)
            # Update target network.
            if num_iters % args.target_update_freq == 0:
                update_target()

            if start_time is not None:
                # steps_per_iter.update(info["steps"] - start_steps)
                steps_per_iter.update(1.0)
                iteration_time_est.update(time.time() - start_time)
            start_time, start_steps = time.time(), float(info["steps"])
            # if steps_per_iter._value:
            #     print('steps_per_iter: ' + str(float(steps_per_iter)))

            # Save the model and training state.
            if num_iters > 0 and (num_iters % args.save_freq == 0 or num_iters > args.num_steps):
                maybe_save_model(savedir, container, {
                    'replay_buffer': replay_buffer,
                    'num_iters': num_iters,
                    'rewards_arr': rewards_arr
                    # 'monitor_state': monitored_env.get_state(),
                })

            if num_iters > args.num_steps:
                break

            if done and (num_iters - prev_print > 1000):
                prev_print = num_iters
                steps_left = args.num_steps - num_iters
                completion = np.round(num_iters / args.num_steps * 100, 1)

                logger.record_tabular("% completion", completion)
                logger.record_tabular("goal step", info["steps"])
                logger.record_tabular("num_iters", num_iters)
                # logger.record_tabular("episodes", len(info["rewards"]))
                logger.record_tabular("episodes", len(rewards_arr))
                # logger.record_tabular("reward", str(["rewards"][-1]))
                logger.record_tabular("reward", str(rewards_arr[-1]))
                # if len(info["rewards"]) >= 10:
                if len(rewards_arr) >= 10:
                    # logger.record_tabular("reward (100 epi mean)", np.mean(info["rewards"][-100:]))
                    logger.record_tabular("reward (100 epi mean)", np.mean(rewards_arr[-100:]))
                logger.record_tabular("exploration", exploration.value(num_iters))
                if args.prioritized:
                    logger.record_tabular("max priority", replay_buffer._max_priority)
                # if steps_per_iter._value:
                #     print('steps_per_iter: ' + str(float(steps_per_iter)))
                fps_estimate = (1.0 / (float(iteration_time_est) + 1e-6)
                                if steps_per_iter._value is not None else "calculating...")
                logger.dump_tabular()
                logger.log()
                logger.log("ETA: " + pretty_eta(int(steps_left / fps_estimate)))
                logger.log()
