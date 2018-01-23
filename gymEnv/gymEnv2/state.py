import numpy as np

class State:
    def __init__(self, map):
        # DQNのatari
        # (4, 84, 84) = 28224
        # 直近の4layerを重ねてる
        # Samurai
        # (9, 180, 20) = 32400
        # enemyMapを除いた場合
        # (6, 180, 20) = 21600
        self.maxw = 20
        # 最大視野の想定 予選は視野>= 5
        self.maxVision = 40
        self.maxh = 100
        self.shape = (self.maxh+self.maxVision*2, self.maxw)
        self.w = map.w
        self.h = map.h + map.vision
        self.goalH = map.h
        self.player = map.player
        self.vision = map.vision

        # 障害物マップ
        self.m = np.ones(self.shape)
        # 左寄せ、右壁は障害物にしておく, 上下は空白
        self.m[:, :self.w] = 0
        self.m[self.maxVision:self.maxVision+self.h, :map.w] = np.array(map.m)
        
        # 0: unknown, 1: known
        self.knownMap = np.zeros(self.shape)
        self.goalMap = np.zeros(self.shape)
        self.goalMap[self.h-1:] = 1
        self.playerMap = np.zeros(self.shape)
        self.playerSpeedX = np.zeros(self.shape)
        self.playerSpeedY = np.zeros(self.shape)
        self.enemyMap = np.zeros(self.shape)
        self.enemySpeedX = np.zeros(self.shape)
        self.enemySpeedY = np.zeros(self.shape)
        self.maxy = 0

    def initPosMap(self):
        self.playerMap = np.zeros(self.shape)
        self.playerSpeedX = np.zeros(self.shape)
        self.playerSpeedY = np.zeros(self.shape)
        self.enemyMap = np.zeros(self.shape)
        self.enemySpeedX = np.zeros(self.shape)
        self.enemySpeedY = np.zeros(self.shape)

    def observe(self):
        self.initPosMap()
        # y, xの順で指定
        self.playerMap[tuple(self.player.pos[::-1])] = 1
        self.playerSpeedX[tuple(self.player.pos[::-1])] = self.player.speed[0]
        self.playerSpeedY[tuple(self.player.pos[::-1])] = self.player.speed[1]
        # EnemyMap追加予定地
        known_range = (self.player.pos[1]-self.vision, self.player.pos[1]+self.vision+1)
        self.knownMap = np.zeros(self.shape)
        self.knownMap[known_range[0]:known_range[1], :] = 1
        # 要素ごとの積, ipythonで確認済み
        # 1のとこだけそのまま、0のとこは0になる
        self.limitedM = self.m * self.knownMap

        # stacked = np.array((self.limitedM, self.knownMap, self.goalMap, self.playerMap, self.playerSpeedX, self.playerSpeedY, self.enemyMap, self.enemySpeedX, self.enemySpeedY))
        stacked = np.array((self.limitedM, self.knownMap, self.goalMap, self.playerMap, self.playerSpeedX, self.playerSpeedY))
        rolled_stacked = np.roll(stacked, -self.player.pos[1], axis=0)
        self._observationCache = rolled_stacked
        return rolled_stacked

    def to_string(self):
        cache = self._observationCache.copy()
        # MapObject
        cache[0] *= 1
        # knownMap known:1, unknown: 0
        #  => known: 0, unknown: 3
        cache[1] -= 1
        cache[1] *= -3
        # goalMap
        cache[2] *= 6
        # playerMap
        cache[3] *= 100
        # playerSpeedX, playerSpeedYはそのまま

        # mapの各位置を合計 shape=self.shape
        sumMap = np.sum(cache, axis=0)
        return sumMap



