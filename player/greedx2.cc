#include <map>
#include <queue>
#include <limits>
#include "raceState.hpp"

const int searchDepth = 1;
const int MAX_DEPTH = 9;
const int MAX_ENEMY_DEPTH = 1;

bool flag_general=false;

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

    bool operator!=(const PlayerState &ps) const {
        return position != ps.position;
    }

    int operator-(const PlayerState &ps) const {
        return position.y - ps.position.y;
    }

    int operator+(const PlayerState &ps) const {
        return position.y + ps.position.y;
    }

    PlayerState(Point p, IntVec v) : position(p), velocity(v) {}
};

struct Candidate {
    int step;            // Steps needed to come here
    PlayerState state;        // State of the player
    Candidate *from;        // Came here from this place
    IntVec how;            //   with this acceleration
    Candidate(int t, PlayerState s, Candidate *f, IntVec h) :
            step(t), state(s), from(f), how(h) {}

    double cost = 0.0;
};

// 比較関数を定義
bool GoodEvalOrder(const Candidate *left, const Candidate *right) {
    return left->cost < right->cost;
}

//global宣言
//候補を格納するqueue
queue<Candidate *> candidates;
queue<Candidate *> enemy_candidates;
auto cmp = [](const Candidate *left, const Candidate *right) { return (left->cost) < (right->cost); };
priority_queue<Candidate *, vector<Candidate *>, decltype(cmp)> AllCandidates(cmp);
priority_queue<Candidate *, vector<Candidate *>, decltype(cmp)> Enemy_AllCandidates(cmp);
Candidate *BestCandidate;
//goaltime初期化
double goalTime = numeric_limits<double>::max();
array<IntVec,3> enemy_assumption;
IntVec befo_rs=IntVec(-1,-1);
//たどり着けるかを記録するmap
map<PlayerState, int> reached;

enum ECompare {
    NO_PREDOMINANCE = 0,
    MY_PREDOMINANCE = 1,
    ENEMY_PREDOMINANCE = 2,
};

int temp_y=0;
double ini_y=0;

IntVec enemyPlay(RaceState &rs, const Course &course) {
    //候補を格納するqueue
    queue<Candidate *> candidates;
    //たどり着けるかを記録するmap
    map<PlayerState, Candidate*> reached;
    //initialはプレイヤーの状態
    PlayerState initial(rs.position, rs.velocity);
    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(0, initial, nullptr, IntVec(0, 0));
    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[initial] = &initialCand;
    //最も良い候補を保存する変数
    Candidate *best = &initialCand;
    double goalTime = numeric_limits<double>::max();
    candidates.push(&initialCand);

    //幅優先探索
    do {
        Candidate *c = candidates.front();
        candidates.pop();
        //速度を9種類全てループ
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++) {
                //次の速度
                IntVec nextVelo = c->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = c->state.position + nextVelo;
                //(step数が0　|| ライバルと衝突しない) &&　コース状の障害物に衝突しない
                if ((c->step != 0 ||
                     !LineSegment(c->state.position, nextPos).goesThru(rs.oppPosition)) &&
                    !course.obstacled(c->state.position, nextPos)) {
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(c->step + 1, next, c, IntVec(cax, cay));

                    //次の候補でゴールが可能ならば
                    if (nextPos.y >= course.length) {
                        //tに今回の候補でゴールに着くまでの時間を記録
                        double t = c->step +
                                   (double) (course.length - c->state.position.y) / nextVelo.y;
                        //暫定goalTimeより小さければそれに決定
                        cerr<<"goalTime="<<goalTime<<endl;
                        if (t < goalTime) {
                            best = nextCand;
                            goalTime = t;
                        }
                    } else if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //コースをはみ出していなければ
                        if (nextPos.y < course.length) {
                            //次の候補に追加
                            candidates.push(nextCand);
                        }
                        //nextにたどり着けることを記録
                        reached[next] = nextCand;
                        //nextのy座標が現在のbestのy座標より大きいならbestを更新
                        if (nextPos.y > best->state.position.y) {
                            best = nextCand;
                        }
                    }
                    //(step数が0　|| ライバルと衝突しない) &&　コース状の障害物に衝突する
                }else if((c->step != 0 ||
                     !LineSegment(c->state.position, nextPos).goesThru(rs.oppPosition)) &&
                        course.obstacled(c->state.position, nextPos)) {
                        //移動はできない
                        nextPos=c->state.position;
                        //次のプレイヤーの位置、速度を次の候補変数に格納
                        PlayerState next(nextPos, nextVelo);
                        Candidate *nextCand =
                                new Candidate(c->step + 1, next, c, IntVec(cax, cay));


                        if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                            //探索深さよりも浅く　かつ　コースをはみ出していなければ
                            if (nextPos.y < course.length) {
                                //次の候補に追加
                                candidates.push(nextCand);
                            }
                            //nextにたどり着けることを記録
                            reached[next] = nextCand;
                            //nextのy座標が現在のbestのy座標より大きいならbestを更新
                            if (nextPos.y > best->state.position.y) {
                                best = nextCand;
                            }
                        }

                }
            }
        }
    } while (!candidates.empty());



    if (best == &initialCand) {
        // 良い動きが見つからなかったよ！
         //Slowing down for a while might be a good strategy
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

//優先権判定 1なら自分2なら相手 0ならどちらにもない
int judgePredominance(Candidate *now, Point nextPos, RaceState &rs,const Course &course) {
    int ret = NO_PREDOMINANCE;

    //相手の行動を予測　想定される行動の上位1つかえす IntVecで
    IntVec enemy_accel=enemyPlay(rs,course);

    bool prec0 = (rs.position.y < rs.oppPosition.y) ||
                 ((rs.position.y == rs.oppPosition.y) &&
                  rs.position.x < rs.oppPosition.x);
    LineSegment Me = LineSegment(now->state.position, nextPos);

    //y方向が相手より下であるか　同じかつ相手のx方向が小さい時
    if (prec0) {
        //優先権があるので相手の進路を妨害することの評価値をあげる
        Point nextOppPosN = rs.oppPosition + rs.oppVelocity + enemy_accel;
        //相手の行動で想定される行動上位1候補のループ
        for (int idx = 0; idx < 1; idx++) {
            //動線の一致　かつ　根元では一致しない
            if (Me.goesThru(nextOppPosN)){
                // Going through the opponent's position is not allowed even with precedence
                ret = ENEMY_PREDOMINANCE;
            }else{
                ret = MY_PREDOMINANCE;
            }

        }

    } else {
        //そうでなければ逆に相手に動線を跨がれないように評価を逆転させる
        //相手の行動で想定される行動上位1候補のループ
        for (int idx = 0; idx < 1; idx++) {
            Point nextOppPosN = rs.oppPosition + rs.oppVelocity + enemy_accel;
            LineSegment Enemy = LineSegment(rs.oppPosition, nextOppPosN);

            if (Enemy.goesThru(nextOppPosN)){
                // Going through the opponent's position is not allowed even with precedence
                ret = MY_PREDOMINANCE;
            }else{
                ret = ENEMY_PREDOMINANCE;
            }
        }
    }
    return ret;
}
//評価関数 今と次の行く場所、盤面の状態
double calcCost(Candidate *now, Point nextPos, RaceState &rs,const Course &course,int depth) {
    double ret = 0;
    //評価値
    //y方向が高ければ高い方が良い
    ret += (((double)nextPos.y)-ini_y);
    //絶対値の方が強い
    //ret+=nextPos.y;
    //-now->state.position.y;
    //x方向への移動が大きければ大きいほど良い
    //ret+=abs(now->state.position.x-nextPos.x);
    //壁から離れていた方が良い
    //costX-=abs(nextPos.x-7);
    //step0の時のみ優先権の評価を入れる
//    if(depth==0){
//        switch (judgePredominance(now, nextPos, rs,course)) {
//            //どちらにも優先権がない
//            case NO_PREDOMINANCE:
//                break;
//                //こちらに優先権がある
//            case MY_PREDOMINANCE:
//                ret += 10;
//                break;
//                //相手に優先権がある
//            case ENEMY_PREDOMINANCE:
//                ret -= 10;
//                break;
//            default:
//                cerr  << "優先権の判定でエラーが出ています" << endl;
//                break;
//        }
//    }
//    //y方向の速度が大きいと良い
//    ret+=(nextPos.y - now->state.position.y)/10;
//    //x方向の速度が大きいと良い
//    ret+=(nextPos.x - now->state.position.x)/10;

    //視界に対しての速度制限
    //if(course.vision<rs.velocity.y)ret-=10;
    return ret;
}


//次の候補
//次の9方向に行った時のstateが配列として返される
vector<Candidate *> generate_next_status(Candidate *ca, const Course &course, RaceState &rs,int depth) {
    //次にいける９^step個の候補を格納する配列
    vector<Candidate *> ret;
    //初期化
    while (!candidates.empty()) candidates.pop();




    candidates.push(ca);

    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[ca->state]++;

    befo_rs.x=rs.position.x;
    befo_rs.y=rs.position.y;

    while (!candidates.empty()) {
        Candidate *now = candidates.front();
        //前の評価値を伝搬
        double pastCost = now->cost;
        //cerr <<"here"<<endl;
        candidates.pop();
        //行き先を9種類全てループ
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++) {
                //次の速度
                IntVec nextVelo = now->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = now->state.position + nextVelo;

                bool coli=LineSegment(now->state.position, nextPos).goesThru(rs.oppPosition);
                //cerr<<"ber"
                bool same_pos=(befo_rs.x==now->state.position.x && befo_rs.y==now->state.position.y);

                //障害物に衝突しない　スタートが視界外への移動は弾く
                if (!course.obstacled(now->state.position, nextPos) ) {
                    //もしdepth=0で相手の駒との接触があれば速度のみ変化
                    if(depth == 0 && LineSegment(now->state.position, nextPos).goesThru(rs.oppPosition)){
                        //足しておいたのを打ち消す
                        nextPos=Point(nextPos.x-nextVelo.x,nextPos.y-nextVelo.y);
                    }
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    // cerr <<"nextCand"<<endl;
                    Candidate *nextCand =
                            new Candidate(now->step+1 , next, now, IntVec(cax, cay));
                    //cerr<<"nextCand->step="<<nextCand->step<<endl;

                    //次の候補でゴールが可能ならば
                    if (nextPos.y >= course.length) {
                       // cerr<<"y="<<nextPos.y<<" "<<course.length<<endl;
                        //tに今回の候補でゴールに着くまでの時間を記録
                        double t = (double)nextCand->step + (double) (course.length - now->state.position.y) / (double)nextVelo.y;
                        //暫定goalTimeより小さければそれに決定
                        if (t < goalTime) {
                            //
                           // cerr<<"y="<<nextPos.y<<" "<<course.length<<endl;
                            //cerr<<"goalTime="<<t<<endl;
                            BestCandidate=nextCand;
                            goalTime = t;
                        }
                    } else if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (nextPos.y < course.length && nextPos.x >= 0 &&
                            nextPos.x < course.width) {
                            //評価値の計算
                            nextCand->cost = calcCost(now, nextPos, rs,course,depth) ;
                          //  if(nextCand->step>37)cerr <<"cost:"<<nextCand->cost<<endl;
                            //次の候補に追加
                            //candidates.push(nextCand);

                            //nextにたどり着けることを記録
                            reached[next]++;
                            //cerr <<"statusにpush?"<<endl;

                            //nextのy座標が現在のbestのy座標より大きいなら を更新
                            // if (nextPos.y > BestCandidate->state.position.y) {
                            //次行ける結果を格納
                            AllCandidates.push(nextCand);
                            ret.push_back(nextCand);
                            //  }
                        }

                    }
                }else if(course.obstacled(now->state.position, nextPos)&& (flag_general)||(same_pos)){
                    //足しておいたのを打ち消す
                    nextPos=now->state.position;
//                    cerr<<"nextPosx= "<<nextPos.x<<" nextPosy="<<nextPos.y<<endl;
//                    cerr<<"nextVelo.x="<<nextVelo.x<<" nextVelo.y"<<nextVelo.y<<endl;


                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(now->step+1 , next, now, IntVec(cax, cay));


                    if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //コースをはみ出していなければ
                        if (nextPos.y < course.length && nextPos.x >= 0 &&
                            nextPos.x < course.width) {
                            //評価値の計算
                            nextCand->cost = calcCost(now, nextPos, rs,course,depth) ;
                            //cerr <<"cost:"<<nextCand->cost<<endl;
                            //次の候補に追加
                            //candidates.push(nextCand);

                            //nextにたどり着けることを記録
                            reached[next]++;
                            //cerr <<"statusにpush?"<<endl;

                            //nextのy座標が現在のbestのy座標より大きいなら を更新
                            // if (nextPos.y > BestCandidate->state.position.y) {
                            //次行ける結果を格納
                            AllCandidates.push(nextCand);
                            ret.push_back(nextCand);
                            //  }
                        }

                    }

                }
            }
        }

    }

    return ret;
}

// rs.positon 自機の現在地　rs.velocity 自機の現在の速度 course コースの情報
IntVec play(RaceState &rs, const Course &course) {

    reached.clear();
    //cerr  << "s" << endl;
    //初期化
    while (!candidates.empty()) candidates.pop();
    goalTime = numeric_limits<double>::max();

    //cerr  << "t" << endl;
    //initialはプレイヤーの状態 自機の位置、速度
    PlayerState initial(rs.position, rs.velocity);

    ini_y=rs.position.y;

    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(rs.step, initial, nullptr, IntVec(0, 0));
//
//    //最も良い候補を保存する変数
    BestCandidate=&initialCand;
//
//    candidates.push(&initialCand);
//
//
//
//    //探索の深さごとの候補を格納する配列
//    vector<Candidate *> status[MAX_DEPTH + 10];
//    status[0].push_back(&initialCand);
//
    //相手の行動を予測　想定される行動の上位1つかえす IntVecで
    //IntVec enemy_accel=enemyPlay(rs,course);
//
//
//    for (int depth = 0; depth < MAX_DEPTH; depth++) {
//        //ビームサーチの切り捨て部分
//
//        const int BEAM_WIDTH = 1;
//        //評価値が良い順でsort
//        sort(status[depth].begin(), status[depth].end(), GoodEvalOrder);
//        //上位BEAM_WIDTHに入らないものを削除
//        if (status[depth].size() > BEAM_WIDTH)
//            status[depth].erase(status[depth].begin() + BEAM_WIDTH, status[depth].end());
//
//
//
//        //cerr  << "depth:" << depth << endl;
//
//        //深さ depthの状態を列挙
//        for (Candidate *state : status[depth]) {
//            //cerr<<"depthのところ"<<endl;
//            //cerr <<"generate_next_status(state,course).size: "<<endl;
//            for (Candidate *next_state: generate_next_status(state, course, rs,depth,enemy_accel)) {
//                //cerr <<"next_State";
//                status[depth + 1].push_back(next_state);
//                //cerr <<"push_Backした！"<<endl;
//            }
//            //cerr <<"中のforループ終了";
//        }
//        //cerr  << "depth search巡回中〜" << endl;
//    }

    //cerr  << "size計算するよ" << endl;

//    long long size = (long long) status[MAX_DEPTH].size();
//    cerr  << "size=" << size << endl;

//
    Candidate *best;
    best = &initialCand;
    goalTime = numeric_limits<double>::max();
//
//    if(BestCandidate!=&initialCand){
//        best=BestCandidate;
//    }
//    else if (!AllCandidates.empty())best = AllCandidates.top();
//
//    while (!AllCandidates.empty())AllCandidates.pop();


    //ここまで最初の暫定的な答えを探るためのビームサーチ
    // temp_y=best->state.position.y;
    //temp_yの取得方法を相手基準に
    //temp_y=rs.oppPosition.y+rs.oppVelocity.y;

    //cerr<<"temp_y="<<temp_y<<endl;

    //初期化
    while (!candidates.empty()) candidates.pop();

    //cerr  << "t" << endl;
    //initialはプレイヤーの状態 自機の位置、速度

    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    //Candidate initialCand(0, initial, nullptr, IntVec(0, 0));

    //最も良い候補を保存する変数
    BestCandidate=&initialCand;

    candidates.push(&initialCand);

    //探索の深さごとの候補を格納する配列
    vector<Candidate *> status_2[MAX_DEPTH + 10];
    status_2[0].push_back(&initialCand);

    //相手の行動を予測　想定される行動の上位1つかえす IntVecで 最初のビームの時やった
    //IntVec enemy_accel=enemyPlay(rs,course);

    double ans_depth1=0;


    for (int depth = 0; depth < MAX_DEPTH; depth++) {

        //cerr  << "depth:" << depth << endl;

        //深さ depthの状態を列挙
        for (Candidate *state : status_2[depth]) {
            //cerr<<"depthのところ"<<endl;
            //cerr <<"generate_next_status(state,course).size: "<<endl;

            //最低速度狩り
//            int leftSteps=MAX_DEPTH-depth;
//            if(state->state.velocity.y<((temp_y-state->state.position.y)-(((leftSteps+1)*leftSteps)/2))/(leftSteps*2)){
//                if(temp_y>state->state.position.y && temp_y< course.length) {
//                    continue;
//                }
//            }

            for (Candidate *next_state: generate_next_status(state, course, rs,depth)) {
                status_2[depth + 1].push_back(next_state);
                //cerr <<"push_Backした！"<<endl;
            }
            //cerr <<"中のforループ終了";
        }

        //深さ1時点での答えを回収
        if(depth==0)ans_depth1=AllCandidates.top()->cost;
        //cerr  << "depth search巡回中〜" << endl;
    }

    best = &initialCand;

    if(BestCandidate!=&initialCand){
        best=BestCandidate;
         cerr<<"choose bestCand"<<endl;
    }
    else if (!AllCandidates.empty())best = AllCandidates.top();

    while (!AllCandidates.empty())AllCandidates.pop();


    cerr<<"bestの価値="<<(double)best->cost<<endl;

    flag_general=false;

    //得られた評価がきの浅いところ かつ　ベストが得られない　だったらgeneralモードon
    if((best->cost<=ans_depth1)&& BestCandidate==&initialCand){
        flag_general=true;
        
    }

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
    int counter = 0;
    while (true) {
        counter++;
        RaceState rs(cin, course);
        IntVec accel = play(rs, course);
        cerr  << "今のstep数" << counter << endl;
        cerr  << "x:" << rs.position.x << " y:" << rs.position.y << endl;
        cerr  << "accel.x" << accel.x << " " << "accel.y" << accel.y << endl;
        if(abs(accel.x)>1 || (accel.y)>1)cerr<<"ERROR!!"<<endl;
        cout << accel.x << ' ' << accel.y << endl;
    }
}