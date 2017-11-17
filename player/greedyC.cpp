#include <map>
#include <queue>
#include <limits>
#include "raceState.hpp"

const int searchDepth = 10;
const int MAX_DEPTH = 10;
const int MAX_ENEMY_DEPTH = 1;

struct PlayerState {
    //自機の位置と速度
    Point position;
    IntVec velocity;

    //比較するための比較演算子
    bool operator<(const PlayerState &ps) const {
        return
                position != ps.position ?
                position < ps.position :
                velocity < velocity;
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
array<IntVec, 3> enemy_assumption;


enum ECompare {
    NO_PREDOMINANCE = 0,
    MY_PREDOMINANCE = 1,
    ENEMY_PREDOMINANCE = 2,
};

//適用の評価関数 今と次の行く場所、盤面の状態
int enemy_calcCost(Candidate *now, Point nextPos, RaceState &rs) {
    int ret = 0;
    //評価値
    //y方向が高ければ高い方が良い
    ret += (nextPos.y);
    return ret;
}


//敵の次の候補を計算する関数、詳細は自機の関数のコメント参照
vector<Candidate *> generate_next_enemy_status(Candidate *ca, const Course &course, RaceState &rs) {
    cerr<<"runnning()... generate_next_enemy_Status"<<endl;
    //次にいける９^step個の候補を格納する配列
    vector<Candidate *> ret;
    //初期化
    while (!candidates.empty()) candidates.pop();
    //たどり着けるかを記録するmap
    map<PlayerState, int> reached;
    reached.clear();


    candidates.push(ca);

    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[ca->state]++;
    //cerr<<"whileのまえ"<<endl;
    while (!candidates.empty()) {
        Candidate *enemy_now = candidates.front();
        candidates.pop();
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++){
                //次の速度
                IntVec nextVelo = enemy_now->state.velocity + IntVec(cax, cay);
                //次の一
                Point nextPos = enemy_now->state.position + nextVelo;
                //cerr<<"now->step:"<<enemy_now->step<<endl;
                //障害物に衝突しない
                if ( nextPos.y < course.length &&!course.obstacled(enemy_now->state.position, nextPos) && enemy_now->step < 3) {
                    //cerr<<"nextCand"<<endl;
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next2(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(enemy_now->step + 1, next2, enemy_now, IntVec(cax, cay));
                    if (reached.count(next2) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (enemy_now->step < searchDepth && nextPos.y <= course.length && nextPos.x > 0 &&
                            nextPos.x <= course.width) {
                            //評価値の計算
                            nextCand->cost = enemy_calcCost(enemy_now, nextPos, rs);
                            //次の候補に追加
                            candidates.push(nextCand);
                            Enemy_AllCandidates.push(nextCand);
                            //nextにたどり着けることを記録
                            reached[next2]++;
                            //次行ける結果を格納
                            ret.push_back(nextCand);
                        }

                    }
                }
                //cerr<<"if文の終わりを告げる"<<endl;
            }
        }

    }
    //cerr<<"finish generate_next_enemy_status()"<<endl;

    return ret;
}


void enemyPlay(RaceState &rs, const Course &course) {
    cerr<<"runnning enemyPlay()..."<<endl;
    //初期化
    while (!enemy_candidates.empty()) enemy_candidates.pop();
    for (int i = 0; i < 3; i++)enemy_assumption[i] = IntVec(0, 0);

    //enemy_initialは相手プレイヤーの状態 敵機の位置、速度
    PlayerState enemy_initial(rs.oppPosition, rs.oppVelocity);

    //step
    Candidate enemy_initialCand(0, enemy_initial, nullptr, IntVec(0, 0));


    //敵の最も良い候補を保存する変数
    enemy_candidates.push(&enemy_initialCand);


    //探索の深さごとの候補を格納する配列
    vector<Candidate *> enemy_status[MAX_ENEMY_DEPTH + 10];
    enemy_status[0].push_back(&enemy_initialCand);


    for (int depth = 0; depth < MAX_ENEMY_DEPTH; depth++) {
        //ビームサーチの切り捨て部分
        //ここに書く
        const int BEAM_WIDTH = 8;
        //評価値が良い順でsort
        sort(enemy_status[depth].begin(), enemy_status[depth].end(), GoodEvalOrder);
        //上位BEAM_WIDTHに入らないものを削除
        if (enemy_status[depth].size() > BEAM_WIDTH)
            enemy_status[depth].erase(enemy_status[depth].begin() + BEAM_WIDTH, enemy_status[depth].end());
        //ここまで

        cerr<<"enemy depth:"<<depth<<endl;

        //深さ depthの状態を列挙
        for (Candidate *state : enemy_status[depth]) {
            //cerr<<"generate_next_status(state,course).size: "<<endl;
            for (Candidate *next_state: generate_next_enemy_status(state, course, rs)) {
                //cerr<<"next_State";
                enemy_status[depth + 1].push_back(next_state);
            }
        }
    }

    cerr<<"enemyの探索終了"<<endl;

    for(int i=0;i<3 && !Enemy_AllCandidates.empty();i++){
        Candidate* temp = Enemy_AllCandidates.top();
        Enemy_AllCandidates.pop();
        if (!Enemy_AllCandidates.empty())enemy_assumption[i] = IntVec(temp->state.velocity.x,temp->state.velocity.y);
    }
    while (!Enemy_AllCandidates.empty())Enemy_AllCandidates.pop();
}
//優先権判定 1なら自分2なら相手 0ならどちらにもない
int judgePredominance(Candidate *now, Point nextPos, RaceState &rs,const Course &course) {
    int ret = NO_PREDOMINANCE;

    //step数が0であり　y方向が相手より下であるか　同じかつ相手のx方向が大きい時
    if (now->step == 0 &&
        (rs.position.y < rs.oppPosition.y || ((rs.position.y == rs.oppPosition.y) && rs.position.x < rs.oppPosition.x))) {
        //優先権があるので相手の進路を妨害することの評価値をあげる
        //Point nextOppPosN = rs.oppPosition + rs.oppVelocity;
        LineSegment Me = LineSegment(now->state.position, nextPos);
        //相手の行動で想定される行動上位1候補のループ
        for (int idx = 0; idx < 1; idx++) {
            IntVec VeloCandidate = enemy_assumption[idx];
            Point nextOppPosCandidate = rs.oppPosition + rs.oppVelocity + VeloCandidate;
            LineSegment Enemy = LineSegment(rs.oppPosition, nextOppPosCandidate);
            //動線の一致　かつ　根元では一致しない
            if (LineSegment(Me).intersects(Enemy) && nextPos != rs.oppPosition) {
                ret = MY_PREDOMINANCE;
            } else if (nextPos == rs.oppPosition) {
                //根元一致だと衝突扱いで動けない
                ret = ENEMY_PREDOMINANCE;
            }
        }

    } else if (now->step == 0) {
        //そうでなければ逆に相手に動線を跨がれないように評価を逆転させる
        LineSegment Me = LineSegment(now->state.position, nextPos);
        //相手の行動で想定される行動上位1候補のループ
        for (int idx = 0; idx < 1; idx++) {
            IntVec VeloCandidate = enemy_assumption[idx];
            Point nextOppPosCandidate = rs.oppPosition + rs.oppVelocity + VeloCandidate;
            LineSegment Enemy = LineSegment(rs.oppPosition, nextOppPosCandidate);
            //動線の一致　かつ　根元では一致しない
            if (LineSegment(Me).intersects(Enemy) && nextPos != rs.oppPosition) {
                ret = ENEMY_PREDOMINANCE;
            }else if (nextPos == rs.oppPosition+rs.oppVelocity) {
                //根元一致だと衝突扱いで動けない
                ret = MY_PREDOMINANCE;
            }
        }
    }
    return ret;
}

//評価関数 今と次の行く場所、盤面の状態
int calcCost(Candidate *now, Point nextPos, RaceState &rs,const Course &course) {
    int ret = 0;
    //評価値
    //y方向が高ければ高い方が良い
    ret += (nextPos.y);
    //x方向への移動が大きければ大きいほど良い
    //ret+=abs(now->state.position.x-nextPos.x);
    //壁から離れていた方が良い
    //costX-=abs(nextPos.x-7);
    //step0の時のみ優先権の評価を入れる
    if(now->step==0){
        switch (judgePredominance(now, nextPos, rs,course)) {
            //どちらにも優先権がない
            case NO_PREDOMINANCE:
                break;
                //こちらに優先権がある
            case MY_PREDOMINANCE:
                ret += 1000;
                break;
                //相手に優先権がある
            case ENEMY_PREDOMINANCE:
                ret -= 1000;
                break;
            default:
                cerr << "優先権の判定でエラーが出ています" << endl;
                break;
        }
    }

    return ret;
}

//次の候補
//次の9方向に行った時のstateが配列として返される
vector<Candidate *> generate_next_status(Candidate *ca, const Course &course, RaceState &rs) {
    //次にいける９^step個の候補を格納する配列
    vector<Candidate *> ret;
    //初期化
    while (!candidates.empty()) candidates.pop();
    //たどり着けるかを記録するmap
    map<PlayerState, int> reached;
    reached.clear();


    candidates.push(ca);

    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[ca->state]++;
    while (!candidates.empty()) {
        Candidate *now = candidates.front();
        //前の評価値を伝搬
        double pastCost = now->cost;
        //cerr<<"here"<<endl;
        candidates.pop();
        //行き先を9種類全てループ
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++) {
                //cerr<<"search: "<<cay<<" "<<cax<<endl;
                //cerr<<"now->step: "<<now->step<<endl;
                //次の速度
                IntVec nextVelo = now->state.velocity + IntVec(cax, cay);
                //次の一
                Point nextPos = now->state.position + nextVelo;

                //障害物に衝突しない
                if (!course.obstacled(now->state.position, nextPos) && now->step < searchDepth) {
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    //cerr<<"nextCand"<<endl;
                    Candidate *nextCand =
                            new Candidate(now->step + 1, next, now, IntVec(cax, cay));
                    if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (now->step < searchDepth && nextPos.y <= course.length && nextPos.x > 0 &&
                            nextPos.x <= course.width) {
                            //評価値の計算
                            nextCand->cost = calcCost(now, nextPos, rs,course) + pastCost;
                            //cerr<<"cost:"<<nextCand->cost<<endl;
                            //次の候補に追加
                            candidates.push(nextCand);
                            AllCandidates.push(nextCand);
                            //nextにたどり着けることを記録
                            reached[next]++;
                            //cerr<<"statusにpush?"<<endl;
                            //次行ける結果を格納
                            ret.push_back(nextCand);
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

    cerr << "s" << endl;
    //初期化
    while (!candidates.empty()) candidates.pop();

    cerr << "t" << endl;
    //initialはプレイヤーの状態 自機の位置、速度
    PlayerState initial(rs.position, rs.velocity);

    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(0, initial, nullptr, IntVec(0, 0));


    //最も良い候補を保存する変数
    candidates.push(&initialCand);



    //探索の深さごとの候補を格納する配列
    vector<Candidate *> status[MAX_DEPTH + 10];
    status[0].push_back(&initialCand);

    //相手の行動を予測　想定される行動の上位3つかえす IntVecでenemy_assumption 降順
    enemyPlay(rs,course);


    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        //ビームサーチの切り捨て部分
        //ここに書く
        const int BEAM_WIDTH = 80;
        //評価値が良い順でsort
        sort(status[depth].begin(), status[depth].end(), GoodEvalOrder);
        //上位BEAM_WIDTHに入らないものを削除
        if (status[depth].size() > BEAM_WIDTH)
            status[depth].erase(status[depth].begin() + BEAM_WIDTH, status[depth].end());
        //ここまで

        cerr << "depth:" << depth << endl;

        //深さ depthの状態を列挙
        for (Candidate *state : status[depth]) {
            //cerr<<"generate_next_status(state,course).size: "<<endl;
            for (Candidate *next_state: generate_next_status(state, course, rs)) {
                //cerr<<"next_State";
                status[depth + 1].push_back(next_state);
                //cerr<<"push_Backした！"<<endl;
            }
            //cerr<<"中のforループ終了";
        }
        cerr << "depth search巡回中〜" << endl;
    }

    cerr << "size計算するよ" << endl;

    long long size = (long long) status[MAX_DEPTH].size();
    cerr << "size=" << size << endl;
    Candidate *best;
    best = &initialCand;
    if (!AllCandidates.empty())best = AllCandidates.top();

    while (!AllCandidates.empty())AllCandidates.pop();

    cerr << "best?" << endl;

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
        cerr << "今のstep数" << counter << endl;
        cerr << "x:" << rs.position.x << " y:" << rs.position.y << endl;
        cerr << "accel.x" << accel.x << " " << "accel.y" << accel.y << endl;
        cout << accel.x << ' ' << accel.y << endl;
    }
}