import sys
import itertools
import random
import math

import gym
import numpy as np
import gym.spaces

from builtins import input
from .map_class import Map
from .state import State
import json
import copy

#参考: OpenAI Gym で自前の環境をつくる - Qiita: https://qiita.com/ohtaman/items/edcb3b0a2ff9d48a7def

# 単体動作するversion
class SamuraiGym2(gym.Env):
    metadata = {'render.modes': ['human', 'ansi']}
    # MAX_STEPS = 100

    def __init__(self):
        super().__init__()
        # max(forwardView, backView)+10必要
        # +10はバグよけ
        self.maxVision = 30
        self.forwardView = 20
        self.backView = 10
        # action_space, observation_space, reward_range を設定する
        self.action_space = gym.spaces.Discrete(9) 
        self.observation_space = gym.spaces.Box(
            low=0,
            high=1,
            shape=(6, self.backView+1+self.forwardView, 20)
        )
        self.reward_range = [-10., 10000.]
        # course01はlength+visionが110あるので避ける
        with open('../samples/course07middle.smrjky', 'r') as mapFile:
        # with open('../samples/course07.smrjky', 'r') as mapFile:
            self.map = Map(json.load(mapFile), self.maxVision)
        self._reset()

    # 必須
    def _reset(self):
        # 諸々の変数を初期化する
        self.isDone = False
        self.isClashed = False
        self.step_num = 0
        self.state = State(self.map, self.maxVision, self.forwardView, self.backView)
        self.map.state = self.state
        # self.map.resetState()
        # ２つ目は速度、ここは曖昧、reward, doneはなくて良い
        self.log0 = []
        self.log1 = []
        # enemyは移動しないので、resetでlogを作ってしまう
        log1Start = self.map.enemy.logString()
        log1Start["step"] = self.step_num
        log1Start["result"] = 0
        self.log1.append(log1Start)

        return self.state.observe()

    # 必須
    def _step(self, action):
        # 1ステップ進める処理を記述。戻り値は observation, reward, done(ゲーム終了したか), info(追加の情報の辞書)
        self.step_num += 1
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
        self.isDone, self.isClashed = self.map.player.move_jockey(acc)
        observation = self.state.observe()
        reward = self.get_reward5()

        log0Current = self.map.player.logString()
        # この実装では動いていない状態をstep0とする
        # Logのstepはこの実装のstep1がstep0なので-1する
        log0Current["step"] = self.step_num -1
        log0Current["result"] = 1 if self.isClashed else 0
        self.log0.append(log0Current)
        return observation, reward, self.isDone, {}

    # 必須
    def _render(self, mode='human', close=False):
        # print("renderWasCalled")
        if mode == 'human':
            outfile = sys.stdout
            maps = self.state.to_string()[:,::-1]
            shape = maps.shape
            print("##############################")

            # multi map debug print
            # for i, map1 in enumerate(maps):
            #     print()
            #     print("map" + str(i))
            #     self.printMap(map1, outfile)
            ####

            # mapの各位置を合計 shape=self.shape
            maps = np.sum(maps, axis=0)
            self.printMap(maps, outfile)

            print('turn: ' + str(self.step_num))
            pos = "pos: " + str(tuple(self.map.player.pos))
            speed = "speed: " + str(tuple(self.map.player.speed))
            print(pos)
            print(speed)
            if self.isClashed:
                print("CLASED!")
            if self.isDone:
                print("GOAL!!!")
            # print(self.state.shape)
            # print("printShape: " + str(shape))
        # return outfile

        # 公式のViewerで読めるlogfileを出力
        if self.isDone:
            log = {}
            log["filetype"] = "race log"
            # file type以外はそのままでOK
            course = copy.deepcopy(self.map.smrjky)
            course["filetype"] = "race course"
            log["course"] = course
            log["name0"] = "DQN0"
            log["name1"] = "NotMoveAgent"
            log["time0"] = self.step_num
            log["time1"] = 999999
            log["log0"] = self.log0
            log["log1"] = self.log1
            with open("gamelog3.json", "w") as logfile:
                json.dump(log, logfile, indent=2, default=self.support_not_JSONserializable_object)

    def printMap(self, map1, outfile):
        map1 = map1.tolist()
        outfile.write(
                '\n'.join(
                    [' '.join(
                        [self.change(num) for num in row]
                    ) for row in map1]
                ) + '\n'
            )

    def change(self, num):
        if num == 0:
            return '-'
        elif 100 < num < 10000:
            # print("Speed x+y: " + str(int(num)))
            return '@'
        else:
            # floatだと見難いのでintにする
            return str(int(num))

    def support_not_JSONserializable_object(self, o):
        # if type(o) == 'numpy.int64':
        return int(o)
        raise TypeError(repr(o) + " is not JSON serializable")

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
            return max(1000 - self.step_num, 100)
        return 0

    def get_reward2(self):
        if self.isDone:
            return 1000
        return -1

    # mapの長さでstepを正規化するために
    # ave_speed = length / step
    # で報酬を作る

    # max_step: 1024とすると
    # x = step
    # x: log2(x): (11-log2(x))*100
    # 1024: 10: 100
    # 512:  9:  200
    # 256:  8:  300
    # 128:  7:  400
    # 64:   6:  500
    # 32:   5:  600
    # 16:   4:  700
    #  8:   3:  800
    #  4:   2
    #  2:   1

    # speed
    # 0.01
    # 0.1
    # 1
    # 10
    # 100

    # 11-log2(x)
    # 

    def get_reward3(self):
        if self.isDone:
            # 初期も意味のある報酬がもらえるように大きめに設定
            return max(1000 - self.step_num*1, 100)
        elif self.isClashed:
            return -5
        else:
            # ぶつからずに進めた時はy座標に進んだ距離 * 1
            # y座標のマイナス方向に進んだ時はマイナス
            return self.map.player.speed[1] * 1


    # speed	*5	+50
    # 0.1	0.5	50.5
    # 1	    5	55
    # 3	    15	65
    # 5	    25	75
    # 8	    40	90
    # 10	50	100
    # DQNもreward_clippingしているし
    # Goal時のrewardで他のrewardが無視されないように
    # 小さめに抑える
    def get_reward4(self):
        speed = self.state.goalLength / self.step_num
        if self.isDone:
            return max(speed*5 + 50, 50)
        elif self.isClashed:
            return -5
        else:
            # ぶつからずに進めた時はy座標に進んだ距離 * 1
            # y座標のマイナス方向に進んだ時はマイナス
            return self.map.player.speed[1] * 1

    
    # goal報酬無しでデバックしやすく
    # 学習初期用
    # DQNのreward_clippingを参考
    def get_reward5(self):
        speed = self.state.goalLength / self.step_num
        # if self.isDone:
        #     return max(speed*5 + 50, 50)
        if self.isClashed:
            return -1
        else:
            # ぶつからずに進めた時はy座標に進んだ距離 * 1
            # y座標のマイナス方向に進んだ時はマイナス
            return self.map.player.speed[1] * 1


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
