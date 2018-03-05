# 既存のコースをいい感じにランダムに切り貼りしてコースを生成します
#
# How to use
# python course_generator.py [course_dir] [save_dir] [num]
# ex) python course_generator.py course/ testcourse/generated/ 200
#
# Requirement
# Python3 / numpy / PIL(Pillow)

import sys
import json
import random
import os
import numpy as np
from PIL import Image

class Course:
    """
    SamurAI Jokey course.
    Using json and numpy.
    
    """
    import json
    import numpy as np
    
    def __init__(self):
        self.length = 100
        self.obstacles = np.array([])
        self.stepLimit = 200
        self.thinkTime = 4000
        self.vision = 20
        self.width = 20
        self.x0 = 1
        self.x1 = 2
    
    # コースの読み込み
    def load(self, course_path):
        with open(course_path, "r") as c:
            course = json.load(c)
        
        self.length = course["length"]
        self.obstacles = np.array(course["obstacles"][10: -10]).astype("uint8")
        self.stepLimit = course["stepLimit"]
        self.thinkTime = course["thinkTime"]
        self.vision = course["vision"]
        self.width = course["width"]
        self.x0 = course["x0"]
        self.x1 = course["x1"]
        
        print("loaded: " + course_path)
    
    # コースの切り出し
    def get_slice(self, start, end, horizontal=True):
        if horizontal:
            sliced = self.obstacles[start: end]

            # 進行不可能な場合、左端を進行可能にする
            for i, row in enumerate(sliced):
                if len(np.where(row==1)) == len(row):
                    sliced[i][0] = 0

            return self.obstacles[start: end]
        else:
            # 垂直に切り出すことはあまり無いはず
            print("I will implement someday XD.")
    
    # コースの結合
    def join(self, sub_course, add_to_top=True):
        padding = np.zeros([5, self.width])
        if self.obstacles.size == 0:
            self.obstacles = sub_course
        elif add_to_top:
            self.obstacles = np.concatenate([sub_course, padding, self.obstacles], axis=0)
        else:
            self.obstacles = np.concatenate([self.obstacles, padding, sub_course], axis=0)
        self.length += (len(sub_course) + 5)
        print("New length:", self.length)
    
    # コースの保存
    def save(self, name):
        # 余白の付与
        padding = np.zeros([10, self.width])
        self.obstacles = np.concatenate([padding, self.obstacles, padding], axis=0)
        
        # ダンプ
        smrjky = {}
        
        smrjky['filetype'] = "race course"
        smrjky["length"] = self.length
        # ToDo: どこかでデータ型が浮動小数点になっている
        smrjky['obstacles'] = self.obstacles.astype("uint8").tolist()
        smrjky['stepLimit'] = self.stepLimit
        smrjky['thinkTime'] = self.thinkTime
        smrjky['vision'] = self.vision
        smrjky['width'] = self.width
        
        # 初期位置はランダム
        smrjky['x0'] = random.randint(1, self.width // 2)
        smrjky['x1'] = random.randint((self.width // 2 + 1), self.width - 1)
        
        with open(name, "w") as c:
            json.dump(smrjky, c)
        
        print("course was saved at " + name)


# PILの機能を利用してサイズ変更
def resize(obstacles ,width, height):
    image = Image.fromarray(np.array(obstacles, dtype="uint8"))
    # ToDo: 縮小の度合いによっては進行不可能なコースが出来上がる可能性あり
    image = image.resize((width, height), Image.BICUBIC)
    return np.asarray(image)


if __name__ == "__main__":
    # コマンドライン引数の受け取り
    args = sys.argv
    
    # 指定したディレクトリ内のコースをすべて取得
    base_courses = [args[1] + course for course in os.listdir(args[1]) if course.endswith(".smrjky")]
    
    # コース生成のパラメーター
    # stepLimit, visionは、利用したコースと、切り抜いた割合に応じて算出
    # 例) 高さ100, stepLimit100のコースから高さ40、高さ150, stepLimit200のコースから高さ60を切り貼りした場合
    #     stepLimit = 100x40/100 + 200x60/150
    #
    # thinkTime = stepLimit * 200 + 1000
    
    slice_params = np.arange(20, 36).tolist()
    width_params = np.arange(5, 21).tolist()
    
    # 指定された数のコースを生成
    for i in range(int(args[3])):
        # 生成に利用するコース数
        course_count = random.randint(2, 5)
        
        # パラメーターの初期化
        stepLimit = 0
        vision = 0
        
        # 生成するコースのインスタンスを生成
        width = random.choice(width_params)
        course = Course()
        course.width = width
        
        # 既存のコースを読み込んで切り貼り
        for j in range(course_count):
            slice_param = random.choice(slice_params)
            
            # コースの読み込み
            sub_course = Course()
            sub_course.load(random.choice(base_courses))
            
            try:
                slice_start = random.randint(0, sub_course.length - slice_param)
            except:
                continue
            sliced_obstacles = sub_course.get_slice(slice_start, slice_start + slice_param)

            # パラメーターの更新
            stepLimit += sub_course.stepLimit * slice_param // (sub_course.length+20)
            vision += sub_course.vision * slice_param // (sub_course.length+20)
            
            # 幅を合わせ、高さをランダムに変形して結合
            course.join(resize(sliced_obstacles, width, random.choice(slice_params)))
        
        # 保存
        course.stepLimit = stepLimit
        course.thinkTime = stepLimit * 200 + 1000
        course.vision = vision
        course.save(args[2] + "course" + str(i) + ".smrjky")

