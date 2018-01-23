import numpy as np

class State:
    def __init__(self, map, maxVision, forwardView, backView):
        # DQNのatari
        # (4, 84, 84) = 28224
        # 直近の4layerを重ねてる
        # Samurai
        # (9, 180, 20) = 32400
        # enemyMapを除いた場合
        # (6, 180, 20) = 21600
        self.maxw = 20
        # 最大視野の想定 予選は視野>= 5
        self.maxVision = maxVision
        self.forwardView = forwardView
        self.backView = backView
        self.maxh = 100
        self.shape = (self.maxh+self.maxVision*2, self.maxw)
        print(self.shape)
        self.goalLength = map.h
        # map.mの大きさ
        self.w = map.w
        self.h = map.h + map.vision

        self.player = map.player
        self.vision = map.vision

        # 障害物マップ
        self.m = np.ones(self.shape)
        # 左寄せ、右壁は障害物にしておく, 上下は空白
        # maxVisionより下、つまり初期位置より下は1
        self.m[maxVision:, :self.w] = 0
        self.m[self.maxVision:self.maxVision+self.h, :map.w] = np.array(map.m)
        
        # 0: unknown, 1: known
        self.knownMap = np.zeros(self.shape)
        self.goalMap = np.zeros(self.shape)
        # goalLengthがindexなので修正無しでOK
        # index + 大きさ = index
        self.goalMap[self.maxVision+self.goalLength:] = 1
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
        # pos = (self.player.map_pos.x, self.player.map_pos.y)
        mapPos = self.player.map_pos()
        self.playerMap[mapPos[::-1]] = 1
        self.playerSpeedX[mapPos[::-1]] = self.player.speed[0]
        self.playerSpeedY[mapPos[::-1]] = self.player.speed[1]
        # EnemyMap追加予定地
        # known_range = (self.player.pos[1]-self.vision, self.player.pos[1]+self.vision+1)
        self.knownMap = np.zeros(self.shape)
        # self.knownMap[mapPos[1]-self.vision:mapPos[1]+self.vision+1, :] = 1
        # 右側の範囲外はすべてknown
        self.knownMap[:, self.w:] = 1
        # 現在地より下はすべてknown
        # 自分の位置 + visionの座標が見え、範囲選択用に+1
        self.knownMap[:mapPos[1]+self.vision+1, :] = 1
        # 要素ごとの積, ipythonで確認済み
        # 1のとこだけそのまま、0のとこは0になる
        self.limitedM = self.m * self.knownMap

        # stacked = np.array((self.limitedM, self.knownMap, self.goalMap, self.playerMap, self.playerSpeedX, self.playerSpeedY, self.enemyMap, self.enemySpeedX, self.enemySpeedY))
        # self.ndarrayは変更しないように
        stacked = np.array((self.limitedM, self.knownMap, self.goalMap, self.playerMap, self.playerSpeedX, self.playerSpeedY))

        # map0,1,2はrollすると先頭が末尾に来てしまってまずい
        # map3,4,5は値がひとつしかないのでrollしてもＯＫ
        # rolled_stacked = np.roll(stacked, -self.player.pos[1], axis=1)
        # 障害物: 追加はなし
        # rolled_stacked[0][-1, :] = 0
        # known_map: unknownで追加
        # rolled_stacked[1][-1, :] = 0

        ## rollに頭を悩ませるなら、任意の範囲を切り取るようにしたほうがよさげ
        ## 例えば(pos-40..pos+40)など
        shiftedMap = stacked[:, mapPos[1]-self.backView:mapPos[1]+self.forwardView+1, :].copy()


        print('shiftedMap: ' + str(shiftedMap.shape))
        self._observationCache = shiftedMap
        return shiftedMap

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
        cache[3] *= 1000
        # playerSpeedX, playerSpeedYはそのまま

        # mapの各位置を合計 shape=self.shape
        # sumMap = np.sum(cache, axis=0)
        # return sumMap
        return cache



