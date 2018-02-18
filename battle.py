#!/usr/bin/env python
import os
import sys
import json
import datetime
import itertools
import subprocess

# 実行可能形式ファイルへのパス
official = '.official/official'

# playerの名前と実行可能形式ファイルへのパス 名前に空白を入れるとバグるので注意 nameはユニークでないとバグる
player = [
    {'name': 'ghost2', 'path': 'player/ghost2/ghost2'},
    {'name': 'ice', 'path': 'player/ice/ice'},
    {'name': 'nine-ball', 'path': 'player/ice/nine-ball'},
    {'name': 'sample', 'path': 'player/sample/greedy'},
]

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


def battle(c: str, p0: dict, p1: dict) -> (bytes, bytes):
    # logは未対応…
    ret = subprocess.run([official, c, p0['path'], p0['name'], p1['path'], p1['name']],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
    return ret.stdout, ret.stderr


def output_racelog(out: bytes, c: str, p0: str, p1: str):
    cp = os.path.basename(c)
    t = datetime.datetime.now().strftime('%m%d%H%M%S')
    path = f'racelog/{t}-{cp}-{p0}-{p1}.racelog'
    with open(path, 'wb') as f:
        f.write(out)


if __name__ == '__main__':
    #  予選形式の総当たり戦を行う
    score = {}
    for p in player:
        score[p['name']] = {'point': 0, 'time': 0.0}

    for card in list(itertools.combinations(player, 2)):
        print(f"{card[0]['name']} VS {card[1]['name']}", file=sys.stderr)
        for c in course:
            tmp_win = {card[0]['name']: 0, card[1]['name']: 0}

            # 実行処理を2回書きたくないのでループを使って無理やり変数名を揃えている
            for p0, p1 in zip(card, card[::-1]):
                ret_out, ret_err = battle(c, p0, p1)  # 対戦
                output_racelog(ret_out, c, p0['name'], p1['name'])  # ファイル出力

                # 対戦結果の表示
                ret = json.loads(ret_out)
                print(f"course: {os.path.basename(c)} {p0['name']}: {ret['time0']} {p1['name']}: {ret['time1']}",
                      file=sys.stderr)
                if ret_err:
                    print(f"WARNING! {ret_err.decode('utf-8')}", file=sys.stderr)

                # 集計 timeはこのまま使い、勝ち負けは一旦記憶してあとで組み合わせごとの勝敗を計算する
                score[p0['name']]['time'] += ret['time0']
                score[p1['name']]['time'] += ret['time1']
                if float(ret['time0']) < float(ret['time1']):  # Win p0
                    tmp_win[p0['name']] += 1
                elif float(ret['time0']) > float(ret['time1']):  # Win p1
                    tmp_win[p1['name']] += 1

            # 組み合わせの勝敗計算
            if tmp_win[p0['name']] > tmp_win[p1['name']]:  # Win p0
                score[p0['name']]['point'] += 2
            elif tmp_win[p0['name']] < tmp_win[p1['name']]:  # Win p1
                score[p1['name']]['point'] += 2
            else:  # Draw
                score[p0['name']]['point'] += 1
                score[p1['name']]['point'] += 1

    # 最終結果表示
    # pointは降順、timeは昇順でソート
    score = sorted(score.items(), key=lambda s: (-s[1]['point'], s[1]['time']))
    for i, s in enumerate(score):
        print(f"{i + 1:2d}位: {s[0]:10s}  {s[1]['point']:2d}pts  {s[1]['time']:5.4f}steps")
