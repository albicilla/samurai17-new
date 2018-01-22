import sys
import itertools
import random
import math

import gym
import numpy as np
import gym.spaces

from builtins import input
import json
from py3_linesegment import LineSegment

class SamuraiEnv(gym.Env):
    metadata = {'render.modes': ['human', 'ansi']}
    MAX_STEPS = 100

    def __init__(self):
        super().__init__()
        # action_space, observation_space, reward_range を設定する
        self.action_space = gym.spaces.Discrete(9) 
        self.observation_space = gym.spaces.Box(
            low=0,
            high=len(self.FIELD_TYPES),
            shape=self.MAP.shape
        )
        self.reward_range = [-1., 100.]
        mapFile = open('../samples/course01.smrjky', 'r')
        self.map = Map(json.load(mapFile))
        self._reset()

    # 必須
    def _reset(self):
        # 諸々の変数を初期化する
        self.done = False
        self.step = 0
        self.state = State(width, height, self.view)
        # ２つ目は速度、ここは曖昧、reward, doneはなくて良い
        return self.state.observe(), (0,0)

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
        self.map.move_jockey(self.map.player, acc)
        observation, info = self.state.observe() # TODO
        reward = self._get_reward()
        return observation, reward, self.done, info



    # 必須
    def _render(self):
        pass

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
    
    def observe(self):
        # TODO: ゴールした時にその場で実行が終わってしまうと、ゴールしたかどうか知ることができない
        if ps[0][0][1] > self.state.h-1:
            self.done = True

        return self.state.observe(), info

    # 前に進んだ量を報酬にしても良いかも
    def _get_reward(self):
        if self.done:
            return max(1000 - self.step, 100)
        return 0

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


class State:
    def __init__(self, map):
        self.maxw = 20
        # 最大視野の想定 予選は視野>= 5
        self.maxVision = 40
        self.maxh = 100
        self.shape = (self.maxh+self.maxVision*2, self.maxw)
        self.w = map.w
        self.h = map.h
        self.player = map.player
        self.vision = map.vision
        # 障害物マップ
        self.m = np.ones(self.shape)
        # 左寄せ、右壁は障害物にしておく, 上下は空白
        self.m[:, :self.w] = 0
        self.m[self.maxVision:-self.maxVision, :] = np.array(map.m)
        
        # 0: unknown, 1: known
        self.unknownMap = np.zeros(self.shape)
        self.playerMap = np.zeros(self.shape)
        self.enemyMap = np.zeros(self.shape)
        self.goalMap = np.zeros(self.shape)
        self.goalMap[self.h-1:] = 1
        self.maxy = 0

    def setline(self, y, l):
        self.m[y, 0:self.w] = l
        self.unknownMap[y, 0:self.w] = 1
        self.maxy = max(self.maxy, y)

    def getXY(self, x, y):
        if x < 0 or x >= self.w or y < 0 or y > self.maxy:
            return 1
        return self.m[y][x]

    def getP(self, p):
        return self.getXY(p[0], p[1])

    def initPosMap(self):
        self.playerMap = np.zeros(self.shape)
        self.enemyMap = np.zeros(self.shape)

    def observe(self):
        self.initPosMap()
        # y, xの順で指定
        self.playerMap[tuple(self.player.pos[::-1])] = 1
        # EnemyMap追加予定地
        known_range = (self.player.pos[1]-self.vision, self.player.pos[1]+self.vision)
        self.unknownMap[known_range[0]:known_range[1], :] = 1
        return np.vstack(self.m, self.unknownMap, self.goalMap, self.playerMap, self.playerSpeed, self.enemyMap, self.enemySpeed)


class Jockey:
    def __init__(self, x, y, vx, vy):
        self.pos = np.array((x, y))
        self.speed = np.array((vx, vy))


class Map:
    def __init__(self, smrjky, default=0, out_of_bound=1):
        self.w = smrjky["width"]
        self.h = smrjky["length"]
        self.m = smrjky["obstacles"]
        self.vision = smrjky["vision"]
        self.player = Jockey(smrjky["x0"], 0, 0, 0)
        self.maxy = 0
        self.dv = default
        self.ob = out_of_bound

    def setline(self, y, l):
        if y < 0:
            return
        while len(self.m) <= y:
            self.m.append([self.dv] * self.w)
        self.m[y] = l
        self.maxy = max(self.maxy, y)

    def getXY(self, x, y):
        if x < 0 or x >= self.w or y < 0:
            return self.ob
        if y > self.maxy:
            return self.dv
        return self.m[y][x]

    def getP(self, p):
        return self.getXY(p[0], p[1])

    def setXY(self, x, y, v):
        if x < 0 or x >= self.w or y < 0:
            return self.ob
        while self.maxy < y:
            self.maxy += 1
            self.m.append([self.dv] * self.w)
        self.m[y][x] = v

    def has_collision(self, p, next_p):
        if self.getP(p) > 0 or self.getP(next_p) > 0:
            return True
        move = LineSegment(p, next_p)
        dp = next_p - p
        xlen, ylen = abs(dp[0]), abs(dp[1])
        dx, dy = np.sign(dp[0]), np.sign(dp[1])
        if xlen == 0:
            for i in range(1, ylen, 1):
                if self.getXY(p[0], p[1] + dy * i) > 0:
                    return True
            return False
        if ylen == 0:
            for i in range(1, xlen, 1):
                if self.getXY(p[0] + dx * i, p[1]) > 0:
                    return True
            return False
        for i in range(0, xlen, 1):
            for j in range(0, ylen, 1):
                x = int(p[0] + dx * i)
                y = int(p[1] + dy * j)
                if self.getXY(x, y) > 0 and move.internal(([x, y])):
                    return True
                nx = x + dx
                ny = y + dy
                if self.getXY(x, y) > 0 and self.getXY(nx, ny) > 0 and move.intersects(LineSegment([x, y], [nx, ny])):
                    return True
                if self.getXY(x, y) > 0 and self.getXY(nx, y) > 0 and move.intersects(LineSegment([x, y], [nx, y])):
                    return True
                if self.getXY(x, y) > 0 and self.getXY(x, ny) > 0 and move.intersects(LineSegment([x, y], [x, ny])):
                    return True
                if self.getXY(x, ny) > 0 and self.getXY(nx, y) > 0 and move.intersects(LineSegment([x, ny], [nx, y])):
                    return True
        return False

    def __str__(self):
        return str(self.m)

    def move_jockey(self, jockey, acc):
        jockey.speed = jockey.speed + acc
        next_p = jockey.pos + jockey.speed
        # 衝突した場合は位置は変えない
        if self.map.has_collision(jockey.pos, next_p):
            pass
        else:
            jockey.pos = next_p


class RaceState:
    def __init__(self, smrjky):
        # self.enemy = Jockey(smrjky["x1"], 0, 0, 0)

