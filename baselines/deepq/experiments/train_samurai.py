import gym
import gymEnv2

from baselines import deepq
from model import model, duelingCustom_model, dueling_model 

from baselines.common.schedules import LinearSchedule
import simple

# def callback(lcl, glb):
#     # stop training if reward exceeds 199
#     is_solved = lcl['t'] > 100 and sum(lcl['episode_rewards'][-101:-1]) / 100 >= 199
#     return is_solved

# def learn(env,
#           q_func,
#           lr=5e-4,
#           max_timesteps=100000,
#           buffer_size=50000,
#           exploration_fraction=0.1,
# t=max_timesteps * exploration_fractionでeがfinalまで落ちる
#           exploration_final_eps=0.02,
#           train_freq=1,
#           batch_size=32,
#           print_freq=100,
#           checkpoint_freq=10000,
#           learning_starts=1000,
#           gamma=1.0,
#           target_network_update_freq=500,
#           prioritized_replay=False,
#           prioritized_replay_alpha=0.6,
#           prioritized_replay_beta0=0.4,
#           prioritized_replay_beta_iters=None,
#           prioritized_replay_eps=1e-6,
#           param_noise=False,
#           callback=None):

max_timesteps=100000
exploration_fraction=0.1
exploration_start_eps=0.5
exploration_final_eps=0.05

exploration = LinearSchedule(schedule_timesteps=int(exploration_fraction * max_timesteps),
                                initial_p=exploration_start_eps,
                                final_p=exploration_final_eps)

def main():
    env = gym.make("Samurai-v0")
    # def model_wrapper(img_in, num_actions, scope, **kwargs):
    #     actual_model = duelingCustom_model
    #     return actual_model(img_in, num_actions, scope, layer_norm=False, **kwargs)
    # q_func = deepq.models.mlp([64])
    # q_func = model_wrapper
    q_func = dueling_model
    # q_func = duelingCustom_model

    # double-qはデフォルトで有効
    act = simple.learn(
        env,
        q_func,
        # lr=5e-4,
        lr=1e-3,
        max_timesteps=max_timesteps,
        buffer_size=50000,
        exploration=exploration,
        train_freq=1,
        batch_size=32,
        print_freq=10,
        # このiterごとにviewer用のlogを出力
        output_for_viewer=100,
        checkpoint_freq=10000,
        learning_starts=1000,
        gamma=1.0,
        target_network_update_freq=500,
        prioritized_replay=True,
        prioritized_replay_alpha=0.6,
        prioritized_replay_beta0=0.4,
        prioritized_replay_beta_iters=None,
        prioritized_replay_eps=1e-6,
        param_noise=False,
        callback=None
    )
    print("Saving model to samurai_model.pkl")
    act.save("samurai_model.pkl")


if __name__ == '__main__':
    main()
