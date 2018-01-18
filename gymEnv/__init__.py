from gym.envs.registration import register

register(
    id='Samurai-v0',
    entry_point='gymEnv.samuraiEnv:SamuraiEnv'
)
