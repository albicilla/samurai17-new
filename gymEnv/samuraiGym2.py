import sys
import itertools
import random
import math

import gym
import numpy as np
import gym.spaces

from builtins import input
from map import Map
from state import State
import json

#参考: OpenAI Gym で自前の環境をつくる - Qiita: https://qiita.com/ohtaman/items/edcb3b0a2ff9d48a7def

# 単体動作するversion
class SamuraiEnv2(gym.Env):
    metadata = {'render.modes': ['human', 'ansi']}
    # MAX_STEPS = 100

    def __init__(self):
        super().__init__()
        # action_space, observation_space, reward_range を設定する
        self.action_space = gym.spaces.Discrete(9) 
        self.observation_space = gym.spaces.Box(
            low=0,
            high=1,
            shape=(6, 180, 20)
        )
        self.reward_range = [-1., 1000.]
        mapFile = open('../samples/course01.smrjky', 'r')
        self.map = Map(json.load(mapFile))
        self._reset()

    # 必須
    def _reset(self):
        # 諸々の変数を初期化する
        self.isDone = False
        self.step = 0
        self.state = State(self.map)
        # ２つ目は速度、ここは曖昧、reward, doneはなくて良い
        return self.state.observe()

    # 必須
    def _step(self, action):
        # 1ステップ進める処理を記述。戻り値は observation, reward, done(ゲーム終了したか), info(追加の情報の辞書)
        if action == 0:
            acc = (-1, 1)
        elif action == 1:
            acc = (0, 1)
        elif action == 2:
            acc = (1, 1)
        elif action == 3:
            acc = (-1, 0)
        elif action == 4:
            acc = (0, 0)
        elif action == 5:
            acc = (1, 0)
        elif action == 6:
            acc = (-1, -1)
        elif action == 7:
            acc = (0, -1)
        elif action == 8:
            acc = (1, -1)
        # move_jockeyはゴールしたかどうかを返す
        self.step += 1
        self.isDone = self.map.move_jockey(self.map.player, acc)
        observation = self.state.observe()
        reward = self.get_reward2()
        return observation, reward, self.isDone, {}

    # 必須
    def _render(self, mode='human', close=False):
        if mode == 'human':
            outfile = sys.stdout
            sumMap = self.state.to_string()
            sumMap = list(sumMap)
            outfile.write(
                    '\n'.join(
                        [' '.join(
                            [' ' if elem==0 else str(elem) for elem in row]
                        ) for row in sumMap]
                    ) + '\n'
                )
        return outfile


    # def _render(self, mode='human', close=False):
    #     # human の場合はコンソールに出力。ansiの場合は StringIO を返す
    #     outfile = StringIO() if mode == 'ansi' else sys.stdout
    #     outfile.write('\n'.join(' '.join(
    #             self.FIELD_TYPES[elem] for elem in row
    #             ) for row in self._observe()
    #         ) + '\n'
    #     )
    #     return outfile

    # 継承
    def _close(self):
        pass

    # 継承
    def _seed(self, seed=None):
        pass

    def readline(self):
        x = input()
        print(str(x), file=sys.stderr)
        return x
    
    # 前に進んだ量を報酬にしても良いかも
    def get_reward(self):
        if self.isDone:
            # TODO: GoalTimeでスコアが変わるように
            return max(1000 - self.step, 100)
        return 0

    def get_reward2(self):
        if self.isDone:
            return 1000
        return -1

    # def _get_reward(self, pos, moved):
    #     # 報酬を返す。報酬の与え方が難しいが、ここでは
    #     # - ゴールにたどり着くと 100 ポイント
    #     # - ダメージはゴール時にまとめて計算
    #     # - 1ステップごとに-1ポイント(できるだけ短いステップでゴールにたどり着きたい)
    #     # とした
    #     if moved and (self.goal == pos).all():
    #         return max(100 - self.damage, 0)
    #     else:
    #         return -1
