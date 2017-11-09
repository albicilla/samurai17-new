//
// Created by 拓真 on 2017/11/09.
//

#include <map>
#include <queue>
#include <limits>
#include "raceState.hpp"

const int searchDepth = 0;
const int MAX_DEPTH = 0;



struct PlayerState {
    //自機の位置と速度
    Point position;
    IntVec velocity;

    //比較するための比較演算子
    bool operator<(const PlayerState &ps) const {
        return
                position != ps.position ?
                position < ps.position :
                velocity < ps.velocity;
    }

    PlayerState(Point p, IntVec v) : position(p), velocity(v){}
};

struct Candidate {
    int step;            // Steps needed to come here
    PlayerState state;        // State of the player
    Candidate *from;        // Came here from this place
    IntVec how;            //   with this acceleration
    Candidate(int t, PlayerState s, Candidate *f, IntVec h) :
            step(t), state(s), from(f), how(h) {}
};

//global宣言
//候補を格納するqueue
queue<Candidate *> candidates;
//たどり着けるかを記録するmap
map<PlayerState, Candidate *> reached;


//次の候補
//次の9方向に行った時のstateが配列として返される
vector<Candidate*> generate_next_status(Candidate *ca,const Course &course){


    //次にいける９^step個(reachedで一応枝刈りしてる)の候補を格納する配列
    vector<Candidate*> ret;

    candidates.push(ca);
    while(!candidates.empty()){
        Candidate *now = candidates.front();
        candidates.pop();
        //行き先を9種類全てループ
        for(int cay = 1; cay != -2;cay--){
            for(int cax = -1;cax != 2; cax++){
                //cerr<<"search: "<<cay<<" "<<cax<<endl;
                //次の速度
                IntVec nextVelo = now->state.velocity + IntVec(cax,cay);
                //次の一
                Point nextPos = now->state.position + nextVelo;

                //ステップ数が0でなく障害物に衝突しない
                if(
                   !course.obstacled(now->state.position,nextPos)){
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos,nextVelo);
                    Candidate *nextCand =
                            new Candidate(now->step + 1, next, now, IntVec(cax, cay));
                    if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (now->step < searchDepth && nextPos.y < course.length) {
                            //次の候補に追加
                            candidates.push(nextCand);
                        }
                        //nextにたどり着けることを記録
                        reached[next] = nextCand;
                        //次行ける結果を格納
                        ret.push_back(nextCand);

                    }



                }
            }
        }

    }

    return ret;
}

// rs.positon 自機の現在地　rs.velocity 自機の現在の速度 course コースの情報
IntVec play(RaceState &rs, const Course &course) {

    //初期化
    while (!candidates.empty()) candidates.pop();
    reached.clear();


    //initialはプレイヤーの状態 自機の位置、速度　相手の位置、速度
    PlayerState initial(rs.position, rs.velocity);
    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(0, initial, nullptr, IntVec(0, 0));
    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[initial] = &initialCand;
    //最も良い候補を保存する変数
    candidates.push(&initialCand);


    //探索の深さごとの候補を格納する配列
    vector<Candidate*> status[MAX_DEPTH+1];
    status[0].push_back(&initialCand);


    for(int depth=0;depth<MAX_DEPTH;++depth){
        //ビームサーチの切り捨て部分
        //ここに書く

        //ここまで


        //深さ depthの状態を列挙
        for(Candidate* state : status[depth]){
            for(Candidate* next_state: generate_next_status(state,course)) {
               status[depth + 1].push_back(next_state);
            }
        }
    }


    //sort(status[MAX_DEPTH].begin(),status[MAX_DEPTH].end());



    Candidate *best = status[MAX_DEPTH][0];


    if (best == &initialCand) {
        // No good move found
        // Slowing down for a while might be a good strategy
        int ax = 0, ay = 0;
        if (rs.velocity.x < 0) ax += 1;
        else if (rs.velocity.x > 0) ax -= 1;
        if (rs.velocity.y < 0) ay += 1;
        else if (rs.velocity.y > 0) ay -= 1;
        return IntVec(ax, ay);
    }
    Candidate *c = best;
    while (c->from != &initialCand) c = c->from;
    return c->how;
}

int main(int argc, char *argv[]) {
    Course course(cin);
    cout << 0 << endl;
    cout.flush();
    while (true) {
        RaceState rs(cin, course);
        IntVec accel = play(rs, course);
        cout << accel.x << ' ' << accel.y << endl;
    }
}
