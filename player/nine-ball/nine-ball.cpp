#include <map>
#include <queue>
#include <limits>
#include <cmath>
#include "raceState.hpp"

const int searchDepth = 1;
//9ならほぼのコースで
int MAX_DEPTH = 9;
const int MAX_ENEMY_DEPTH = 7;
const double EPS=0.1;

bool flag_general=false;
bool flag_use_greedy2=false;
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
    double special_cost=0.0; //
    Candidate(int t, PlayerState s, Candidate *f, IntVec h,double sc) :
            step(t), state(s), from(f), how(h) ,special_cost(sc){}

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
auto cmp = [](const Candidate *left, const Candidate *right) { return (left->cost) != (right->cost) ? (left->cost) < (right->cost):-(left->state.velocity.y) < -(right->state.velocity.y); };
priority_queue<Candidate *, vector<Candidate *>, decltype(cmp)> AllCandidates(cmp);
priority_queue<Candidate *, vector<Candidate *>, decltype(cmp)> Enemy_AllCandidates(cmp);
Candidate *BestCandidate;
//goaltime初期化
double goalTime = numeric_limits<double>::max();
IntVec befo_rs=IntVec(-1,-1);
//たどり着けるかを記録するmap
map<PlayerState, int> reached;
map<PlayerState, int> reached_enemy;

enum ECompare {
    NO_PREDOMINANCE = 0,
    MY_PREDOMINANCE = 1,
    ENEMY_PREDOMINANCE = 2,
};

static const int COUNTER_CLOCKWISE=1;
static const int CLOCKWISE = -1;
static const int ONLINE_BACK=2;
static const int ONLINE_FRONT = -2;
static const int ON_SEGMENT = 0;


double dot(Point a,Point b){
    return a.x*b.x+a.y*b.y;
}
double cross(Point a,Point b){
    return a.x*b.y-a.y*b.x;
}
//線分p0,p1に対するp2の位置 ヘッダのccwと混同しないようね
int CCW(Point p0,Point p1,Point p2){
    Point a = p1 - p0;
    Point b = p2 - p0;
    if(cross(a,b)> EPS)return COUNTER_CLOCKWISE;
    if(cross(a,b)< -EPS)return CLOCKWISE;
    if(dot(a,b) < -EPS)return ONLINE_BACK;
    if((a.x*a.x+a.y*a.y)<(b.x*b.x+b.y*b.y))return ONLINE_FRONT;
    if(p2==p0||p2==p1)return ONLINE_BACK;

    return ON_SEGMENT;
}

int temp_y=0;
double ini_y=0;

IntVec enemyPlay(RaceState &rs, const Course &course) {
    //候補を格納するqueue
   // queue<Candidate *> candidates;
    //たどり着けるかを記録するmap
    //initialはプレイヤーの状態
    PlayerState initial(rs.oppPosition, rs.oppVelocity);
    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(0, initial, nullptr, IntVec(0, 0),0);
    reached_enemy.clear();
    //最も良い候補を保存する変数
    Candidate *best = &initialCand;
    double goalTime = numeric_limits<double>::max();
    enemy_candidates.push(&initialCand);

    //幅優先探索
    do {
        Candidate *c = enemy_candidates.front();
        enemy_candidates.pop();
        //速度を9種類全てループ
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++) {
                //次の速度
                IntVec nextVelo = c->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = c->state.position + nextVelo;
                //(step数が0　|| ライバルと衝突しない) &&　コース状の障害物に衝突しない
                if ((c->step != 0 ||
                     !LineSegment(c->state.position, nextPos).goesThru(rs.position)) &&
                    !course.obstacled(c->state.position, nextPos)) {
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(c->step + 1, next, c, IntVec(cax, cay),0);

                    //次の候補でゴールが可能ならば
                    if (nextPos.y >= course.length) {
                        //tに今回の候補でゴールに着くまでの時間を記録
                        double t = c->step +
                                   (double) (course.length - c->state.position.y) / nextVelo.y;
                        //暫定goalTimeより小さければそれに決定
                        if (t < goalTime) {
                            best = nextCand;
                            goalTime = t;
                        }
                    } else if (reached_enemy.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //コースをはみ出していなければ
                        if (c->step<MAX_ENEMY_DEPTH&&nextPos.y < course.length) {
                            //次の候補に追加
                            enemy_candidates.push(nextCand);
                        }
                        //nextにたどり着けることを記録
                        reached_enemy[next]++;
                        //nextのy座標が現在のbestのy座標より大きいならbestを更新
                        if (nextPos.y > best->state.position.y) {
                            best = nextCand;
                        }
                    }
                    //(step数が0　|| ライバルと衝突しない) &&　コース状の障害物に衝突する
                }else if((c->step != 0 ||
                          !LineSegment(c->state.position, nextPos).goesThru(rs.position)) &&
                         course.obstacled(c->state.position, nextPos)) {
                    //移動はできない
                    nextPos=c->state.position;
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(c->step + 1, next, c, IntVec(cax, cay),0);


                    if (reached_enemy.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (c->step<MAX_ENEMY_DEPTH&&nextPos.y < course.length) {
                            //次の候補に追加
                            candidates.push(nextCand);
                        }
                        //nextにたどり着けることを記録
                        reached_enemy[next]++;
                        //nextのy座標が現在のbestのy座標より大きいならbestを更新
                        if (nextPos.y > best->state.position.y) {
                            best = nextCand;
                        }
                    }

                }
            }
        }
    } while (!enemy_candidates.empty());



    if (best == &initialCand) {
        // 良い動きが見つからなかったよ！
        //Slowing down for a while might be a good strategy
        int ax = 0, ay = 0;
        if (rs.oppVelocity.x < 0) ax += 1;
        else if (rs.oppVelocity.x > 0) ax -= 1;
        if (rs.oppVelocity.y < 0) ay += 1;
        else if (rs.oppVelocity.y > 0) ay -= 1;
        return IntVec(ax, ay);
    }
    Candidate *c = best;
    while (c->from != &initialCand) c = c->from;
    return c->how;
}

//優先権判定 1なら自分2なら相手 0ならどちらにもない 探索前にint dominance
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

//1なら自分2なら相手 0ならどちらにもない
int ifPredominance(Candidate *now,RaceState &rs,const Course &course){
    if(now->state.position.y>rs.oppPosition.y) {
        return 2;
    }else if(now->state.position.y<rs.oppPosition.y){
        return 1;
    }else{
        if(now->state.position.x<rs.oppPosition.x){
            return 1;
        }else if(now->state.position.x>rs.oppPosition.x){
            return 2;
        }else {
            return 0;
        }
    }
}

double calcCostInher(RaceState &rs,int depth,int dominance,Point nextoppPos,Point nextPos){
    double ret=0;
    int cc=CCW(rs.oppPosition,nextoppPos,nextPos);
    //相手の予告線のちょうど上に行けるなら評価+ 逆なら-
    //今の相手の座標、次の相手の座標、次の自分の座標
    if(cc==ON_SEGMENT && dominance==1 && depth==0){
        ret=0.5;
    }

    if(cc==ON_SEGMENT && dominance==2 && depth==0){
        ret=-0.5;
    }

   if(cc==ONLINE_BACK)ret-=0.2;

    return ret;
}
//評価関数 今と次の行く場所、盤面の状態 nextoppPosはenemyplayによる次の相手の予測位置
double calcCost(Candidate *now, Point nextPos, RaceState &rs,const Course &course,int depth,Point nextVelo) {
    double ret = 0;
    //評価値
    //y方向が高ければ高い方が良い
    ret += (((double)nextPos.y)-ini_y);

    //visionに応じて速度のペナルティを発生させる
    if(nextVelo.y>(course.vision+1)/2)ret-=50;

    //横に行き過ぎはダメ
    if(nextVelo.x>course.width/2)ret-=50;

    //視界の外への移動に対する罰則
    //if(course.vision+ini_y<nextPos.y)ret-=10;

    //少ない種数の方が優秀
    ret-=depth*0.1;
    return ret;
}


//次の候補
//次の9方向に行った時のstateが配列として返される
vector<Candidate *> generate_next_status(Candidate *ca, const Course &course, RaceState &rs,int depth,int dominance,Point nextoppPos) {
    //次にいける９^step個の候補を格納する配列
    vector<Candidate *> ret;
    //初期化
    while (!candidates.empty()) candidates.pop();

    candidates.push(ca);

    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[ca->state]++;

    befo_rs.x=rs.position.x;
    befo_rs.y=rs.position.y;

    double special_cost=0;


    while (!candidates.empty()) {
        Candidate *now = candidates.front();
        //前の評価値を伝搬
        //double pastCost = now->cost;
        //cerr <<"here"<<endl;
        //もし深さが0の時specialcostを計算　そうでないなら親から継承する

        candidates.pop();
        //行き先を9種類全てループ
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++) {
                //次の速度
                IntVec nextVelo = now->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = now->state.position + nextVelo;


                if(depth==0){
                    special_cost=calcCostInher(rs,depth,dominance,nextoppPos,nextPos);
                }else{
                    special_cost=now->special_cost;
                  //  if(special_cost>0)cerr<<" "<<special_cost<<endl;
                }

                //if(abs(special_cost)>EPS)cerr<<"特別special_cost= "<<special_cost<<endl;

                bool coli=LineSegment(now->state.position, nextPos).goesThru(rs.oppPosition);
                //cerr<<"ber"
                bool same_pos=(befo_rs.x==now->state.position.x && befo_rs.y==now->state.position.y);

                //障害物に衝突しない　スタートが視界外への移動は弾く
                if (!course.obstacled(now->state.position, nextPos) && !(nextVelo.y>=course.vision) && (nextPos.y >= now->state.position.y+nextVelo.y)) {
                    //もしdepth=0で相手の駒との接触があれば速度のみ変化
                    if(depth == 0 && LineSegment(now->state.position, nextPos).goesThru(rs.oppPosition)){
                        //足しておいたのを打ち消す
                        nextPos=Point(nextPos.x-nextVelo.x,nextPos.y-nextVelo.y);
                    }
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    // cerr <<"nextCand"<<endl;
                    Candidate *nextCand =
                            new Candidate(now->step+1 , next, now, IntVec(cax, cay),special_cost);
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
                            nextCand->cost = calcCost(now, nextPos, rs,course,depth,nextVelo)+special_cost;

                            //nextにたどり着けることを記録
                            reached[next]++;
                            //次行ける結果を格納
                            AllCandidates.push(nextCand);
                            ret.push_back(nextCand);
                        }

                    }
                }
                else if(((flag_general)||(same_pos)) && !(flag_use_greedy2)&& (nextPos.y >= now->state.position.y+nextVelo.y)){
                    //足しておいたのを打ち消す
                    nextPos=now->state.position;
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    //のちに同じでもこれ経由は壁に当たりつつなのでsp-0.5
                    Candidate *nextCand =
                            new Candidate(now->step+1 , next, now, IntVec(cax, cay),special_cost-0.5);


                    if (reached.count(next) == 0 && abs(nextVelo.x)+abs(nextVelo.y)>-1) { //そこにたどり着くのが最初の候補であれば 速度の加減
                        //コースをはみ出していなければ
                        if (nextPos.y+nextVelo.y < course.length && nextPos.x >= 0 &&
                            nextPos.x < course.width) {
                            //評価値の計算
                            nextCand->cost = calcCost(now, nextPos, rs,course,depth,nextVelo)+special_cost;
                            //cerr <<"cost:"<<nextCand->cost<<endl;
                            //次の候補に追加
                            //candidates.push(nextCand);

                            //nextにたどり着けることを記録
                            reached[next]++;
                            //cerr <<"statusにpush?"<<endl;

                            //nextのy座標が現在のbestのy座標より大きいなら を更新
                            //次行ける結果を格納
                            AllCandidates.push(nextCand);
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

    //なくてもいいかも
    flag_use_greedy2=false;
//    if(rs.velocity.y>=course.vision-3){
//        flag_use_greedy2=true;
//        MAX_DEPTH=15;
//    }
//    if(!flag_use_greedy2)MAX_DEPTH=9;

    //敵よりy座標が高かったら消極的に
//    if(rs.position.y-rs.oppPosition.y> 5){
//        flag_use_greedy2=true;
//    }

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
    Candidate initialCand(rs.step, initial, nullptr, IntVec(0, 0),0);
//
//    //最も良い候補を保存する変数
    BestCandidate=&initialCand;

    Candidate *best;
    best = &initialCand;
    goalTime = numeric_limits<double>::max();

    //初期化
    while (!candidates.empty()) candidates.pop();


    //最も良い候補を保存する変数
    BestCandidate=&initialCand;

    candidates.push(&initialCand);

    //探索の深さごとの候補を格納する配列
    vector<Candidate *> status_2[MAX_DEPTH + 10];
    status_2[0].push_back(&initialCand);

    double ans_depth1=0;
    //1なら自分2なら相手 0ならどちらにもない
    int dominance = ifPredominance(&initialCand,rs,course);
    //貪欲サンプルにより的の次の座標を取得
    Point nextoppPos = rs.oppPosition+enemyPlay(rs,course)+rs.oppVelocity;


    for (int depth = 0; depth < MAX_DEPTH; depth++) {

        //深さ depthの状態を列挙
        for (Candidate *state : status_2[depth]) {
            //cerr<<"depthのところ"<<endl;
            //cerr <<"generate_next_status(state,course).size: "<<endl;


            for (Candidate *next_state: generate_next_status(state, course, rs,depth,dominance,nextoppPos)) {
                status_2[depth + 1].push_back(next_state);
            }
        }

        //深さ1時点での答えを回収
        if(depth==0)ans_depth1=AllCandidates.top()->cost;
    }

    best = &initialCand;

    if(BestCandidate!=&initialCand){
        best=BestCandidate;
        cerr<<"choose bestCand"<<endl;
    }
    else if (!AllCandidates.empty())best = AllCandidates.top();

    cerr<<"best->cost"<<best->cost<<endl;

    while (!AllCandidates.empty())AllCandidates.pop();



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
        cerr  << "opx:"<<rs.oppPosition.x << "opy;" << rs.oppPosition.y<<endl;
        cerr <<"oppvx:" <<rs.oppVelocity.x << "oppvy:" <<rs.oppVelocity.y<<endl;
        cerr  << "accel.x" << accel.x << " " << "accel.y" << accel.y << endl;
        if(abs(accel.x)>1 || (accel.y)>1)cerr<<"ERROR!!"<<endl;
        cout << accel.x << ' ' << accel.y << endl;
    }
}