import gym
import gymEnv2

ENV_NAME = 'Samurai-v0'
env = gym.make(ENV_NAME)
env.reset()
env.render()
action = 1
# env.step(action)

