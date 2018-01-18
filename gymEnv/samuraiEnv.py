import sys

import gym
import numpy as np
import gym.spaces

# import map
from builtins import input


class MyEnv(gym.Env):
    metadata = {'render.modes': ['human', 'ansi']}
        # FIELD_TYPES = [
        #     'S',  # 0: スタート
        #     'G',  # 1: ゴール
        #     '~',  # 2: 芝生(敵の現れる確率1/10)
        #     'w',  # 3: 森(敵の現れる確率1/2)
        #     '=',  # 4: 毒沼(1step毎に1のダメージ, 敵の現れる確率1/2)
        #     'A',  # 5: 山(歩けない)
        #     'Y',  # 6: 勇者
        # ]
        # MAP = np.array([
        #     [5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5],  # "AAAAAAAAAAAA"
        #     [5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2],  # "AA~~~~~~~~~~"
        #     [5, 5, 2, 0, 2, 2, 5, 2, 2, 4, 2, 2],  # "AA~S~~A~~=~~"
        #     [5, 2, 2, 2, 2, 2, 5, 5, 4, 4, 2, 2],  # "A~~~~~AA==~~"
        #     [2, 2, 3, 3, 3, 3, 5, 5, 2, 2, 3, 3],  # "~~wwwwAA~~ww"
        #     [2, 3, 3, 3, 3, 5, 2, 2, 1, 2, 2, 3],  # "~wwwwA~~G~~w"
        #     [2, 2, 2, 2, 2, 2, 4, 4, 2, 2, 2, 2],  # "~~~~~~==~~~~"
        # ])
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

    def _reset(self):
        # map.init()
        self.total_time = int(self.readline())
        self.max_step = int(self.readline())
        width, height = [int(x) for x in self.readline().split()]
        self.view = int(self.readline())
        # 初期化終了
        print(0)
        self.map = Map(width, height, self.view)
        self.done = False
        return map.observe()
        # # 諸々の変数を初期化する
        # self.pos = self._find_pos('S')[0]
        # self.goal = self._find_pos('G')[0]
        # self.damage = 0
        # self.steps = 0
        # return self._observe()


    def readline(self):
        x = input()
        print(str(x), file=sys.stderr)
        return x
    

    def observe(self):                                
        # while True:
        step = int(self.readline())
        time = int(self.readline())
        ps = []
        for j in range(2):
            xs = [int(x) for x in self.readline().split()]
            # x,y, vx,vy
            p = [np.array([xs[0], xs[1]]), np.array([xs[2], xs[3]])]
            ps.append(tuple(p))
        map.initPosMap()
        map.playerMap[ps[0][0][0], ps[0][0][1]] = 1
        map.enemyMap[ps[1][0][0], ps[1][0][1]] = 1
        for y in range(ps[0][0][1] - self.view, ps[0][0][1] + self.view + 1, 1):
            ls = [int(v) for v in self.readline().split()]
            if y > 0:
                map.setline(y, ls)
        return map.observe


    def _step(self, action):
        # 1ステップ進める処理を記述。戻り値は observation, reward, done(ゲーム終了したか), info(追加の情報の辞書)
        print(action)
        return map.observe() # TODO
        # if action == 0:
        #     next_pos = self.pos + [1, 1]
        # elif action == 1:
        #     next_pos = self.pos + [0, -1]
        # elif action == 2:
        #     next_pos = self.pos + [1, 0]
        # elif action == 3:
        #     next_pos = self.pos + [-1, 0]

        # if self._is_movable(next_pos):
        #     self.pos = next_pos
        #     moved = True
        # else:
        #     moved = False

        # observation = self._observe()
        # reward = self._get_reward(self.pos, moved)
        # self.damage += self._get_damage(self.pos)
        # self.done = self._is_done()
        # return observation, reward, self.done, {}

    # def _render(self, mode='human', close=False):
    #     # human の場合はコンソールに出力。ansiの場合は StringIO を返す
    #     outfile = StringIO() if mode == 'ansi' else sys.stdout
    #     outfile.write('\n'.join(' '.join(
    #             self.FIELD_TYPES[elem] for elem in row
    #             ) for row in self._observe()
    #         ) + '\n'
    #     )
    #     return outfile

    def _close(self):
        pass

    def _seed(self, seed=None):
        pass

    def _get_reward(self, pos, moved):
        # 報酬を返す。報酬の与え方が難しいが、ここでは
        # - ゴールにたどり着くと 100 ポイント
        # - ダメージはゴール時にまとめて計算
        # - 1ステップごとに-1ポイント(できるだけ短いステップでゴールにたどり着きたい)
        # とした
        if moved and (self.goal == pos).all():
            return max(100 - self.damage, 0)
        else:
            return -1


    def _is_done(self):
        # 今回は最大で self.MAX_STEPS までとした
        if (self.pos == self.goal).all():
            return True
        elif self.steps > self.MAX_STEPS:
            return True
        else:
            return False

    def _find_pos(self, field_type):
        return np.array(list(zip(*np.where(
        self.MAP == self.FIELD_TYPES.index(field_type)
    ))))


class Map:
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