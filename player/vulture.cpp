#include <bits/stdc++.h>
#include <fstream>
#define REP(i, a, b) for (int(i) = (a); i < (b); ++(i))
#define rep(i, n) REP(i, 0, n)
#define PER(i, a, b) for (int(i) = (a - 1); i >= (b); --(i))
#define per(i, n) PER(i, n, 0)
#define each(i, n) for (auto &i : n)
#define clr(a) memset((a), 0, sizeof(a))
#define mclr(a) memset((a), -1, sizeof(a))
#define all(a) (a).begin(), (a).end()
#define dum(val) cerr << #val " = " << val << " ";
#define dump(val) cerr << #val " = " << val << endl;
#define FILL(a, v) fill(a, a + sizeof(a) / sizeof(*a), v)

using namespace std;
//9ならほぼのコースで
int MAX_DEPTH = 1;
int MAX_ENEMY_DEPTH = 1;
int ThinkTime;
int StepLimit;
int CourseWidth;
int CourseLength;
int vision;
double ini_y=0;
const double EPS=0.1;
int OVERCLOCK=0;
//flag_general はcourse03みたいなコースのtimeout対策
bool flag_general=true;

vector<vector<int>> course(210, vector<int>(110, 1));
#define COURSE(i,j)  course[(i)+(int)10][(j)+(int)10]


struct NowState{
    int step,lefttime;
    int myposx,myposy,myvelox,myveloy;
    int eneposx,eneposy,enevelox,eneveloy;
};

struct IntVec {
    int x, y;
    IntVec(int x = 0, int y = 0): x(x), y(y) {};
    inline IntVec operator+(const IntVec &another) const {
        return IntVec(x+another.x, y+another.y);
    }
    inline IntVec operator-(const IntVec &another) const {
        return IntVec(x-another.x, y-another.y);
    }
    inline bool operator==(const IntVec &another) const {
        return x == another.x && y == another.y;
    }
    inline bool operator!=(const IntVec &another) const {
        return !(*this == another);
    }
    inline bool operator<(const IntVec &another) const {
        return y != another.y ? y < another.y : x < another.x;
    }
};

typedef IntVec Point;
struct PlayerState {
    //自機の位置と速度  スペシャルコスト
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

    PlayerState(Point p, IntVec v) : position(p), velocity(v){}
};

struct RaceState {
    int step;
    int timeLeft;
    Point position, oppPosition;
    IntVec velocity, oppVelocity;
    RaceState(istream &in);
};
RaceState::RaceState(istream &in) {
    in >> step
       >> timeLeft
       >> position.x >> position.y
       >> velocity.x >> velocity.y
       >> oppPosition.x >> oppPosition.y
       >> oppVelocity.x >> oppVelocity.y;
}
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
struct LineSegment {
    Point p1, p2;
    LineSegment() {};
    LineSegment(Point p1, Point p2): p1(p1), p2(p2) {};
    bool goesThru(const Point &p) const;
    bool intersects(const LineSegment &l) const;
};
//global宣言
//候補を格納するqueue
queue<Candidate *> candidates;
queue<Candidate *> enemy_candidates;
auto cmp = [](const Candidate *left, const Candidate *right) { return (left->cost) != (right->cost) ? (left->cost) < (right->cost):-(left->state.velocity.y) < -(right->state.velocity.y); };
priority_queue<Candidate *, vector<Candidate *>, decltype(cmp)> AllCandidates(cmp);
priority_queue<Candidate *, vector<Candidate *>, decltype(cmp)> Enemy_AllCandidates(cmp);
Candidate nullCand(0, PlayerState(0,0), nullptr, IntVec(0, 0),0);
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


unsigned xorshft(){
    static unsigned y = 2463534242;
    return y^=(y^=(y^=y<<13)>>17)<<5;
}
bool obstacled(const Point &from, const Point &to) {
    LineSegment m(from, to);
    int
            x1 = from.x, y1 = from.y,
            x2 = to.x, y2 = to.y;
    if (COURSE(y2,x2) == 1) return true;
    int xstep = x2 > x1 ? 1 : -1;
    if (y1 == y2) {
        for (int x = x1; x != x2; x += xstep) {
            if (COURSE(y1,x) == 1) return true;
        }
        return false;
    }
    int ystep = y2 > y1 ? 1 : -1;
    if (x1 == x2) {
        for (int y = y1; y != y2; y += ystep) {
            if (COURSE(y,x1) == 1) return true;
        }
        return false;
    }
    for (int y = y1; y != y2; y += ystep) {
        int ny = y + ystep;
        for (int x = x1; x != x2; x += xstep) {
            int nx = x + xstep;
            if (COURSE(y,x) == 1 && m.goesThru({x, y})) {
                return true;
            }
            if ((COURSE(y,x) == 1 && COURSE(ny,nx) == 1 &&
                 LineSegment(Point(x, y), Point(nx, ny)).intersects(m)) ||
                (COURSE(ny,x) == 1 && COURSE(ny,nx) == 1 &&
                 LineSegment(Point(x, ny), Point(nx, ny)).intersects(m)) ||
                (COURSE(y,nx) == 1 && COURSE(ny,nx) == 1 &&
                 LineSegment(Point(nx, y), Point(nx, ny)).intersects(m)) ||
                (COURSE(ny,x) == 1 && COURSE(y,nx) == 1 &&
                 LineSegment(Point(x, ny), Point(nx, y)).intersects(m))) {
                return true;
            }
        }
    }
    return false;
}

IntVec enemyPlay(RaceState &rs) {
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
                //cerr<<"cax="<<cax<<" "<<"cay="<<cay<<endl;
                //次の速度
                IntVec nextVelo = c->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = c->state.position + nextVelo;
                //(step数が0　|| ライバルと衝突しない) &&　コース状の障害物に衝突しない
                if ((c->step != 0 ||
                     !LineSegment(c->state.position, nextPos).goesThru(rs.position)) &&
                    !obstacled(c->state.position, nextPos)) {
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(c->step + 1, next, c, IntVec(cax, cay),0);

                    //次の候補でゴールが可能ならば
                    if (nextPos.y >= CourseLength) {
                        //tに今回の候補でゴールに着くまでの時間を記録
                        double t = c->step +
                                   (double) (CourseLength - c->state.position.y) / nextVelo.y;
                        //暫定goalTimeより小さければそれに決定
                        if (t < goalTime) {
                            best = nextCand;
                            goalTime = t;
                        }
                    } else if (reached_enemy.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //コースをはみ出していなければ
                        if (c->step<MAX_ENEMY_DEPTH&&nextPos.y < CourseLength) {
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
                         obstacled(c->state.position, nextPos)) {
                    //移動はできない
                    nextPos=c->state.position;
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(c->step + 1, next, c, IntVec(cax, cay),0);


                    if (reached_enemy.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (c->step<MAX_ENEMY_DEPTH&&nextPos.y < CourseLength) {
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

                }
            }
        }
    } while (!enemy_candidates.empty());

    cerr<<"roop finish"<<endl;

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


//1なら自分2なら相手 0ならどちらにもない
int ifPredominance(Candidate *now,RaceState &rs){
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
    int cc2=CCW(rs.oppPosition,nextoppPos,rs.position);
    //相手の予告線のちょうど上に行けるなら評価+ 逆なら- 相手と同じベクトルの時
    //今の相手の座標、次の相手の座標、次の自分の座標
    if(cc==ON_SEGMENT && dominance==1 && depth==0 && cc2!=ONLINE_BACK){
        // cerr<<"ONSEG1"<<endl;
        ret+=0.2;
    }

    if(cc==ON_SEGMENT && dominance==2 && depth==0 && cc2!=ONLINE_BACK){
        // cerr<<"ONSEG2"<<endl;
        ret=-0.2;
    }

    //
    if(LineSegment(rs.oppPosition,nextoppPos).intersects(LineSegment(rs.position,nextPos)) && dominance==1&&depth==0 &&!LineSegment(rs.position, nextPos).goesThru(rs.oppPosition)){
        //  cerr<<"insec1"<<endl;
        ret+=0.1;
    }
    if(LineSegment(rs.oppPosition,nextoppPos).intersects(LineSegment(rs.position,nextPos)) && dominance==2&&depth==0){
        // cerr<<"insec2"<<endl;

        ret-=0.1;
    }


    if(cc==ONLINE_BACK)ret-=0.01;

    return ret;
}
//評価関数 今と次の行く場所、盤面の状態 nextoppPosはenemyplayによる次の相手の予測位置
double calcCost(Candidate *now, Point nextPos, RaceState &rs,int depth,Point nextVelo) {
    double ret = 0;
    //評価値
    //y方向が高ければ高い方が良い
    ret += (((double)nextPos.y)-ini_y);

    //visionに応じて速度のペナルティを発生させる
    //cerr<<abs((nextVelo.y)/2) <<abs((nextVelo.x)/2)<<(course.vision+1)/2<<endl;
    if( ( abs((nextVelo.y)-1) + abs((nextVelo.x)) )>(vision+1)/2)ret-=50;

    if(nextVelo.x<=0 &&((nextVelo.x)+nextPos.x < 0+nextVelo.x)){
        ret-=10;
    }

    if(nextVelo.x >0 && ((nextVelo.x)+nextPos.x)>CourseWidth -nextVelo.x){
        ret-=10;
    }

    //横に行き過ぎはダメ
    if(nextVelo.x>CourseWidth/2)ret-=50;

    //視界の外への移動に対する罰則
    //if(course.vision+ini_y<nextPos.y)ret-=10;

    //少ない種数の方が優秀
    ret-=depth*0.1;
    return ret;
}


//次の候補
//次の9方向に行った時のstateが配列として返される
vector<Candidate *> generate_next_status(Candidate *ca, RaceState &rs,int depth,int dominance,Point nextoppPos) {
    //次にいける9^step個の候補を格納する配列
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
        cerr <<"here"<<endl;
        //もし深さが0の時specialcostを計算　そうでないなら親から継承する

        candidates.pop();
        //行き先を9種類全てループ
        for (int cay = -1; cay <= 1; cay++) {
            for (int cax = -1; cax <=1; cax++) {
                //次の速度
                IntVec nextVelo = now->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = now->state.position + nextVelo;


                if(depth==0){
                    special_cost=calcCostInher(rs,depth,dominance,nextoppPos,nextPos);
                }else{
                    special_cost=now->special_cost;
                    // if(special_cost>0)cerr<<" "<<special_cost<<endl;
                }

                //if(abs(special_cost)>EPS)cerr<<"特別special_cost= "<<special_cost<<endl;

                bool coli=LineSegment(now->state.position, nextPos).goesThru(rs.oppPosition);
                //cerr<<"ber"
                bool same_pos=(befo_rs.x==now->state.position.x && befo_rs.y==now->state.position.y);

                //障害物に衝突しない　異動先が(視界外+次の速度)<-キモ！にするとcourse09で行けるようになるがそれはそれ他のコースがダメになる　への移動は弾く
                if (!obstacled(now->state.position, nextPos) && (nextPos.y<=((ini_y+vision)+OVERCLOCK))  ) {
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
                    if (nextPos.y >= CourseLength && nextPos.y<=(ini_y+vision) ){
                        // cerr<<"y="<<nextPos.y<<" "<<course.length<<endl;
                        //tに今回の候補でゴールに着くまでの時間を記録
                        double t = (double)nextCand->step + (double) (CourseLength - now->state.position.y) / (double)nextVelo.y;
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
                        if (nextPos.y < CourseLength && nextPos.x >= 0 &&
                            nextPos.x < CourseWidth) {
                            //評価値の計算
                            nextCand->cost = calcCost(now, nextPos, rs,depth,nextVelo)+nextCand->special_cost;


                            //nextにたどり着けることを記録
                            reached[next]++;
                            //次行ける結果を格納
                            AllCandidates.push(nextCand);
                            ret.push_back(nextCand);
                        }

                    }
                }
                else if(flag_general&&(nextPos.y<=(ini_y+vision) ) ){
                    //足しておいたのを打ち消す
                    nextPos=now->state.position;
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    //のちに同じでもこれ経由は壁に当たりつつなのでsp-0.5
                    Candidate *nextCand =
                            new Candidate(now->step+1 , next, now, IntVec(cax, cay),special_cost-0.5);


                    if (reached.count(next) == 0 ) { //そこにたどり着くのが最初の候補であれば 速度の加減
                        //コースをはみ出していなければ
                        if (nextPos.y+nextVelo.y < CourseLength && nextPos.x >= 0 &&
                            nextPos.x < CourseWidth) {
                            //評価値の計算
                            nextCand->cost = calcCost(now, nextPos, rs,depth,nextVelo)+nextCand->special_cost;
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

IntVec play(RaceState &rs){
    cerr<<"now.step="<<rs.step<<endl;
    ofstream outputfile("/Users/albicilla/programming/samurai/samurai17/test"+to_string(rs.step)+".txt");

    for(int tate=rs.position.y+vision;tate>=rs.position.y-vision;tate--){
        {
            rep(yoko,course[tate+10].size()){
                //cerr<<COURSE(tate,yoko);
                //ファイル書き出し
                outputfile<<COURSE(tate,yoko);
            }
            //cerr<<endl;
            //
            outputfile<<endl;
        }
    }
    cerr<<"obs"<<endl;

    reached.clear();
    while(!candidates.empty())candidates.pop();
    goalTime = numeric_limits<double>::max();

    PlayerState initial(rs.position,rs.velocity);
    ini_y = rs.position.y;

    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(rs.step, initial, nullptr, IntVec(0, 0),0);

    //最も良い候補を保存する変数
    BestCandidate=&nullCand;

    Candidate *best;
    best = &initialCand;
    goalTime = numeric_limits<double>::max();

    //初期化
    while (!candidates.empty()) candidates.pop();

    //最も良い候補を保存する変数
    BestCandidate=&nullCand;

    candidates.push(&initialCand);

    //探索の深さごとの候補を格納する配列
    vector<Candidate *> status_2[MAX_DEPTH + 10];
    status_2[0].push_back(&initialCand);

    double ans_depth1=0;
    //1なら自分2なら相手 0ならどちらにもない
    int dominance = ifPredominance(&initialCand,rs);

    cerr<<"ene"<<endl;
    //貪欲サンプルにより的の次の座標を取得
    Point nextoppPos = rs.oppPosition+enemyPlay(rs)+rs.oppVelocity;
    cerr<<"ene finish"<<endl;

    for (int depth = 0; depth < MAX_DEPTH; depth++) {

        //深さ depthの状態を列挙
        for (Candidate *state : status_2[depth]) {
            //cerr<<"depthのところ"<<endl;
            //cerr <<"generate_next_status(state,course).size: "<<endl;


            for (Candidate *next_state: generate_next_status(state,rs,depth,dominance,nextoppPos)) {
                status_2[depth + 1].push_back(next_state);
            }
        }

        //深さ1時点での答えを回収
        if(depth==0)ans_depth1=AllCandidates.top()->cost;
    }

    best = &nullCand;

    if(BestCandidate!=&nullCand){
        best=BestCandidate;
    }else if(!AllCandidates.empty())best=AllCandidates.top();

    while(!AllCandidates.empty())AllCandidates.pop();
    //得られた評価がきの浅いところ かつ　ベストが得られない　だったらgeneralモードoff
    if((best->cost<=ans_depth1+3)&& BestCandidate==&nullCand){
        flag_general=false;
    }

    if(best==&nullCand){
        int ax=0,ay=0;
        if(rs.velocity.x<0)ax+=1;
        else if (rs.velocity.x > 0) ax -= 1;
        if (rs.velocity.y < 0) ay += 1;
        else if (rs.velocity.y > 0) ay -= 1;
        return IntVec(ax, ay);
    }
    Candidate *c = best;
    while (c->from != &initialCand) c = c->from;
    return c->how;
}

int main(){
    //初期入力
    cin>>ThinkTime>>StepLimit>>CourseWidth>>CourseLength>>vision;
    cout<<0<<endl;


    while(1){
        RaceState rs(cin);
        //NowState now;
        //cin>>now.step>>now.lefttime>>now.myposx>>now.myposy>>now.myvelox>>now.myveloy>>now.eneposx>>now.eneposy>>now.enevelox>>now.eneveloy;
        for(int i=-vision;i<=vision;i++){
            course[rs.position.y+i+10].clear();
            rep(j,10){
                course[rs.position.y+i+10].push_back(1);
            }
            rep(j,CourseWidth){
                int a;
                cin>>a;
                course[rs.position.y+i+10].push_back(a);
            }
        }


        IntVec accel = play(rs);
        cerr  << "accel.x" << accel.x << " " << "accel.y" << accel.y << endl;

        cout<<accel.x<<' '<<accel.y<<endl;
    }

}