import gym
import gymEnv2



'''
ReadMe

> cd here
> python human_palyer.py

毎ターンactionを数字で入力してください
例: 2 => speed+(1,1)

action = 
0|1|2
3|4|5
6|7|8

入力待ち時にCtrl-Dで終了します


記号:
@: 自機
1: 障害物
3: unknown(まだ見えていない場所)
6: goalとgoalの向こう側

重なった場合は和を表示します
例: 7は1と6が重なっている場所です。
'''

ENV_NAME = 'Samurai-v0'
env = gym.make(ENV_NAME)
env.reset()
env.render()

for i in range(1000):
    string = input()
    if string.isdigit():
        next_action = int(string)
        if 0 <= next_action <= 8:
            observation, reward, isDone, info = env.step(next_action)
            env.render()
            print("Reward: " + str(reward))
            if isDone:
                break
            continue
    print("please input 0~8")

