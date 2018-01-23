import gym
import gymEnv2

ENV_NAME = 'Samurai-v0'
env = gym.make(ENV_NAME)
env.reset()
next_action = 1
env.step(next_action)
env.render()

