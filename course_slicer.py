#!/usr/bin/env python
import os
import json
import copy

# コースファイルへのパス
course = [
    'course/course01.smrjky',
    'course/course02.smrjky',
    'course/course03.smrjky',
    'course/course04.smrjky',
    'course/course05.smrjky',
    'course/course06.smrjky',
    'course/course07.smrjky',
    'course/course08.smrjky',
    'course/course09.smrjky',
    'course/course10.smrjky',
    'course/noObstacle.smrjky',
    'course/sample-course.smrjky',
]

output_dir = 'course/sliced'

if __name__ == '__main__':
    if not os.path.isdir(output_dir):
        os.mkdir(output_dir)

    for c in course:
        filename, _ = os.path.splitext(os.path.basename(c))
        with open(c, 'r') as in_f:
            data = json.load(in_f)
            # スライスしたあとの1単位の長さ よしなに書き換えて
            slice_length = data['vision'] + 2
            n = data['length']//slice_length
            for i in range(0, n):
                path = f"{output_dir}/{filename}-{data['width']}-{i}.smrjky"
                ret = copy.deepcopy(data)
                ret['obstacles'] = ret['obstacles'][i*slice_length:(i+1)*slice_length]

                with open(path, 'w') as out_f:
                    json.dump(ret, out_f)
