import sys

import gym
import numpy as np
import gym.spaces

from builtins import input


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
        self._reset()

    # 必須
    def _reset(self):
        # 諸々の変数を初期化する
        self.done = False
        self.step = 0
        self.total_time = int(self.readline())
        self.max_step = int(self.readline())
        width, height = [int(x) for x in self.readline().split()]
        self.view = int(self.readline())
        # 初期化終了
        print(0)
        self.state = State(width, height, self.view)
        # ２つ目は速度、ここは曖昧、reward, doneはなくて良い
        return self.state.observe(), (0,0)

    # 必須
    def _step(self, action):
        # 1ステップ進める処理を記述。戻り値は observation, reward, done(ゲーム終了したか), info(追加の情報の辞書)
        print(action)
        observation, info = self.state.observe() # TODO
        reward = self._get_reward()
        return observation, reward, self.done, info
        # if action == 0:
        #     next_pos = self.pos + [1, 1]
        # elif action == 1:
        #     next_pos = self.pos + [0, -1]
        # elif action == 2:
        #     next_pos = self.pos + [1, 0]
        # elif action == 3:
        #     next_pos = self.pos + [-1, 0]


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
        # while True:
        self.step = int(self.readline())
        time = int(self.readline())
        ps = []
        for j in range(2):
            xs = [int(x) for x in self.readline().split()]
            # x,y,(index) vx,vy
            p = [np.array([xs[0], xs[1]]), np.array([xs[2], xs[3]])]
            ps.append(tuple(p))
        # TODO: ゴールした時にその場で実行が終わってしまうと、ゴールしたかどうか知ることができない
        if ps[0][0][1] > self.state.h-1:
            self.done = True

        self.state.initPosMap()
        self.state.playerMap[ps[0][0][0], ps[0][0][1]] = 1
        self.state.enemyMap[ps[1][0][0], ps[1][0][1]] = 1
        for y in range(ps[0][0][1] - self.view, ps[0][0][1] + self.view + 1, 1):
            ls = [int(v) for v in self.readline().split()]
            if y > 0:
                self.state.setline(y, ls)
        info = {"playerSpeed:" (ps[0][1], ps[1][1])}
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
    def __init__(self, width, height, view):
        self.maxw = 20
        self.maxh = 140
        self.shape = (self.maxh, self.maxw)
        self.w = width
        self.h = height
        # 障害物マップ
        self.m = np.zeros(self.shape)
        # 左寄せ、右壁は障害物にしておく
        self.m[:, width-1:] = 1
        # 0: unknown, 1: known
        self.unknownMap = np.zeros(self.shape)
        self.unknownMap[:, width-1:] = 1
        # self.m = [[0] * width for y in range(height + view + 1)]
        self.playerMap = np.zeros(self.shape)
        self.enemyMap = np.zeros(self.shape)
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
        return np.vstack(self.m, self.unknownMap, self.playerMap, self.unknownMap)