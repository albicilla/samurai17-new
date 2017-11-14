import json
import random
from datetime import datetime

w = 15
l = 100
v = 8
x0 = 5
x1 = 9

obstacles = []

# 障害物の形状を分布で与える 実際は高さを掛けて使う
distribution = {
    "triangle": [1 / 3, 2 / 3, 1, 2 / 3, 1 / 3],
    "square": [1, 1, 1, 1, 1],
    "delta_top": [1, 0.8, 0.6, 0.4, 0.2],
    "delta_bottom": [0.2, 0.4, 0.6, 0.8, 1]
}

# 分布を使って10個間隔で生成する
for i in range(0, l, 10):
    x_pos = random.randint(0, 2)  # 基準になるx方向の位置を左・中央・右から選ぶ
    width = random.randint(1, w - 3)  # 障害物の幅を決める

    dist = random.choice(list(distribution.keys()))

    # 多角形を分布に沿って生成する
    obs = []
    for j, d in enumerate(distribution[dist]):
        y = i + j + len(dist) // 2
        if x_pos == 0:
            obs.append({"x": int(d * width), "y": y})
            obs.append({"x": 0, "y": y})

        elif x_pos == 1:
            obs.append({"x": w // 2 - int(d * width // 2), "y": y})
            obs.append({"x": w // 2 + int(d * width // 2), "y": y})
        else:
            obs.append({"x": w - int(d * width) - 1, "y": y})
            obs.append({"x": w - 1, "y": y})

    obstacles.append(obs)

ret = {"filetype": "race course for editing",
       "width": w,
       "length": l,
       "vision": v,
       "thinkTime": 2000,
       "stepLimit": l,
       "x0": x0,
       "x1": x1,
       "obstacles": obstacles
       }

print(json.dumps(ret))
with open(f"{datetime.now().strftime('%Y%m%d-%H%M%S')}.json", "w") as f:
    json.dump(ret, f)
