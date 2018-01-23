
import numpy as np
from .py3_linesegment import LineSegment


# class Pos:
#     def __init__(self, x, y):
#         self.x = x
#         self.y = y


class Jockey:
    def __init__(self, x, y, vx, vy, maxVision):
        self.maxVision = maxVision
        self.pos = np.array((x, y))
        self.speed = np.array((vx, vy))

    def map_pos(self):
        ''' return (x, y) '''
        pos = (self.pos[0], self.pos[1] + self.maxVision)
        # pos = Pos(self.pos[0] + self.vision, self.pos[1])
        return pos


class Map:
    def __init__(self, smrjky, maxVision, default=0, out_of_bound=1):
        self.w = smrjky["width"]
        self.h = smrjky["length"]
        self.m = smrjky["obstacles"]
        self.vision = smrjky["vision"]
        # self.enemy = Jockey(smrjky["x1"], 0, 0, 0)
        self.player = Jockey(smrjky["x0"], 0, 0, 0, maxVision)
        self.maxy = 0
        self.dv = default
        self.ob = out_of_bound
        self.state = None

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
        ''' ゴールしたかどうかを返す '''
        jockey.speed = jockey.speed + acc
        next_p = jockey.pos + jockey.speed
        # 衝突した場合は位置は変えない
        if self.map.has_collision(jockey.pos, next_p):
            pass
        else:
            jockey.pos = next_p
            if next_p >= self.h:
                return True
        return False
                

