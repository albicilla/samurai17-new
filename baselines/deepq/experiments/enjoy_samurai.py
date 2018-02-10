import gym
import gymEnv2

from baselines import deepq


def main():
    env = gym.make("Samurai-v0")
    act = deepq.load("samurai_model.pkl")

    for _ in range(1):
        obs, done = env.reset(), False
        episode_rew = 0
        while not done:
            obs, rew, done, _ = env.step(act(obs[None])[0])
            print('rew: ' + str(rew))
            episode_rew += rew
            env.render(mode="human+viewer")
        print("Episode reward", episode_rew)


# if __name__ == '__main__':
print('start enjoy samurai')
main()
