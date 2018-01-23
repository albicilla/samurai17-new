import gym
import gymEnv2

ENV_NAME = 'Samurai-v0'
env = gym.make(ENV_NAME)
env.reset()
env.render()

for i in range(100):
    next_action = int(input())
    env.step(next_action)
    env.render()

