import numpy as np

class Jockey:
    def __init__(self, map_instance, x, y, vx, vy, maxVision):
        # mapはMapclassのinstanceを渡す
        self.map = map_instance
        self.maxVision = maxVision
        self.pos = np.array((x, y))
        self.prevPos = np.array((x,y))
        self.acc = np.array((0,0))
        self.prevSpeed = np.array((vx, vy))
        self.speed = np.array((vx, vy))

    def move_jockey(self, acc):
        ''' return (ゴールしたかどうか, ぶつかったかどうか) '''
        self.prevSpeed = self.speed
        self.acc = acc
        self.speed = self.speed + acc
        next_p = self.pos + self.speed
        clashed = False
        # 衝突した場合は位置は変えない
        if self.map.has_collision(self.pos, next_p):
            # prevPosを更新するためにset_posをする
            self.set_pos(self.pos)
            clashed = True
        else:
            self.set_pos(next_p)
            # 移動した場合のみゴール判定
            if next_p[1] >= self.map.h:
                return True, clashed
        return False, clashed

    def set_pos(self, next_p):
        self.prevPos = self.pos.copy()
        self.pos = next_p

    def map_pos(self):
        ''' return (x, y) '''
        pos = (self.pos[0], self.pos[1] + self.maxVision)
        # pos = Pos(self.pos[0] + self.vision, self.pos[1])
        return pos

    def logString(self):
        log = {}
        log["before"] = {
            "x": self.prevPos[0],
            "y": self.prevPos[1]
        }
        log["after"] = {
            'x': self.pos[0],
            'y': self.pos[1]
        }
        # logのvelocityはaccが足される前の速度なので注意
        # つまり最初は0,0
        log["velocity"] = {
            'x': self.prevSpeed[0],
            'y': self.prevSpeed[1]
        }
        log["acceleration"] = {
            'x': self.acc[0],
            'y': self.acc[1]
        }
        return log