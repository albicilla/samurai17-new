#include <map>
#include <queue>
#include <limits>
#include <iostream>
#include <random>
#include "raceState.hpp"

#define FOR(i,a,b) for(int i=a;i<b;i++)
#define rep(i,b) FOR(i,0,b)
#define INF 1e9
#define EPISODE_LOOP 1000
/*
 * 負の添え字を扱うためのマクロ
 * 参考:http://albicilla.hatenablog.com/
 *
 * Q new_x,new_y,new_vx,new_vy,roop
 */
int Q[110][110][40][40][10];
int policy[110][110][40][40];
//開始時の地点
int start_x,start_y;
//各エピソード時点での開始時の地点
int episode_starty;
//視界を持つグローバル変数
int vision=99;

#define QDP(i,j,k,l,m)  Q[(i)][(j)][(k)+(int)15][(l)+(int)15][(m)]
#define policyDP(i,j,k,l) policy[(i)][(j)][(k)+(int)15][(l)+(int)15]

int debug[30][100];
void show_debug(){
    for(int x=0;x<20;x++){
        for(int y=0;y<100;y++){

            int temp=0;
            for(int roop=0;roop<9;roop++){
                for(int j=-2;j<2;j++){
                    for(int k=-2;k<2;k++){
                        temp+=QDP(x,y,j,k,roop);
                    }
                }
            }

            debug[x][y]=temp;

        }
    }


    for(int j=0;j<20;j++){
        for(int i=0;i<101;i++){
            cerr<<debug[j][i]<<" ";
        }
        cerr<<endl;
    }
    cerr<<endl;
}



int debug_policy[30][100];
void show_debug_policy(){
    //初期値
    rep(i,30)rep(j,100)debug_policy[i][j]=9;

    for(int x=-1;x<20;x++){
        for(int y=-1;y<100;y++){

            //それぞれの速度のときにそのマスに来た時の行き先のカウントをする配列
            int cnt[10];
            rep(i,10)cnt[i]=0;

            for(int j=-2;j<2;j++){
                for(int k=-2;k<2;k++){
                    cnt[policyDP(x,y,j,k)]++;
                }
            }
            int temp=0;
            rep(i,10){
                if(i!=0 && cnt[i]>temp){
                    debug_policy[x][y]=i;
                }
            }

        }
    }


    for(int i=-1;i<20;i++){
        for(int j=-1;j<40;j++){
            cerr<<debug_policy[i][j]<<" ";
        }
        cerr<<endl;
    }
    cerr<<endl;
}

//episode内のターン数
int turn = 0;


int generate_seed(){
    return turn;
}
//イプシロングリーディでランダムな結果か政策の結果を採用するかの決定
/*
 *  c++の乱数生成 　参考　http://qnighy.hatenablog.com/entry/2015/07/01/235907
 */
int generate_action_e(int action,int eps){
    //random_device rand_src;
    int seed=generate_seed();
    mt19937 rand_src(seed);
    if(rand_src()%100<eps){
        return rand_src()%9;
    }else{
        //cerr<<"take ation from policy"<<action<<endl;
        return action;
    }
}

//ゴール
bool game_over(int final_x,int final_y,const RaceState &rs,const Course &rt){
    if(rt.width >= final_x && final_x>=0 && final_y>=start_y + vision){
        //cerr<<"gameover"<<endl;
        return 1;
    }
    else return false;
}
//場外
bool outside(int final_x,int final_y,const Course &rt){
    if(0>final_x || final_x>rt.width || 0>final_y)return 1;
    else {
        //cerr<<"not outside"<<endl;
        return false;
    }
}
//衝突
bool collision(int x,int y,int final_x,int final_y,const RaceState &rs,const Course &course){
    //cerr << "r="<<r<<" c="<<c<<endl;
    //cerr<<"final_x="<<final_x <<" final_y="<<final_y<<endl;

    if(!course.obstacled(Point(x,y), Point(final_x,final_y))){
        //cerr<<"not collision"<<endl;
        return false;
    }else {
        //cerr<<"collision"<<endl;
        return 1;
    }
}


//とったactionによって現在の速度ベクトルが変化
/*
 *  012
 *  345
 *  678
 *
 *  参考　http://secret-garden.hatenablog.com/entry/2016/05/23/152601
 */
typedef struct {
    int x;
    int y;
} velo_set;

velo_set change_velocity(int ax, int ay, int action){

    if(action==0){
        ay++;
        ax--;
    }
    if(action==1){
        ay++;
    }
    if(action==2){
        ay++;
        ax++;
    }
    if(action==3){
        ax--;
    }
    if(action==5){
        ax++;
    }
    if(action==6){
        ay--;
        ax--;
    }
    if(action==7){
        ay--;
    }
    if(action==8){
        ay--;
        ax++;
    }

    velo_set ret={ax,ay};
    return ret;
}

Point action_to_velocity(int action) {

    int ax=0,ay=0;
    if(action==0){
        ay++;
        ax--;
    }
    if(action==1){
        ay++;
    }
    if(action==2){
        ay++;
        ax++;
    }
    if(action==3){
        ax--;
    }
    if(action==5){
        ax++;
    }
    if(action==6){
        ay--;
        ax--;
    }
    if(action==7){
        ay--;
    }
    if(action==8){
        ay--;
        ax++;
    }
    Point ret={ax,ay};
    return ret;
}

//グローバルでやって良いか？
//int reward=0;
//int new_x=0, new_y=0, new_vx=0, new_vy=0;
typedef struct {
    int reward=0;
    int new_x=0;
    int new_y=0;
    int new_vx=0;
    int new_vy=0;
} reward_and_next_state_set;

//とった行動によって得られる報酬と次の状態を
reward_and_next_state_set generate_reward_and_next_state(int x,int y,int vx,int vy,int action,const RaceState &rs,const Course &course){
    int reward=0,new_x=0,new_y=0,new_vx=0,new_vy=0;
    //vx,vyを更新
    auto nv = change_velocity(vx,vy,action);

    //reward-=(turn)*2;

    int final_x, final_y;

    final_x = x + nv.x;
    final_y = y + nv.y;
    // cerr << course.length<<" "<<course.width<<endl;

    //速度が大きすぎるのはあり得ないので除外
    if(abs(nv.x)>14 ||abs(nv.y)>14 ) {
        //cerr<<"too large speed"<<endl;
        reward-=100;
        new_x=x;
        new_y=y;
        new_vx=0;
        new_vy=0;
        auto ret = {reward,new_x,new_y,new_vx,new_vy};
    }else if(game_over(final_x,final_y,rs,course)){
       reward+=1000000;
        new_x=final_x;
        new_y=final_y;
        new_vx=nv.x;
        new_vy=nv.y;
    }else if(outside(final_x,final_y,course)){
        reward-=100;
        new_x=x;
        new_y=y;
        new_vx=nv.x;
        new_vy=nv.y;
        //cerr<<"outside"<<endl;
    }else if(collision(x,y,final_x,final_y,rs,course)){
        reward-=120;
        new_x=x;
        new_y=y;
        new_vx=nv.x;
        new_vy=nv.y;
    }else {
        //cerr<<"can go to"<<final_x<<" "<<final_y<<endl;

        reward-=5;
        new_x=final_x;
        new_y=final_y;
        new_vx=nv.x;
        new_vy=nv.y;
        reward+=(new_y-y)*1;

    }
    reward_and_next_state_set ret={reward,new_x,new_y,new_vx,new_vy};
    return ret;

}



//TODO
/*
 *x座標 y座標整理 done
 *
 * generate_reward_and_next_state()
で返す変数int reward=0;
int new_x=0, new_y=0, new_vx=0, new_vy=0;
を構造体で返すようにする
 */

// rs.positon 駒の位置　rs.velocity 現在の速度 course コースの情報
void q_learning(const RaceState &rs, const Course &course){
    start_x=rs.position.x;
    start_y=rs.position.y;
    //とりあえず100で
    vision = 100;
    cerr<<"vision="<<vision<<endl;


    cerr<<"start_x="<<start_x<<" start_y"<<start_y<<endl;
    for(int epi=0;epi<EPISODE_LOOP; epi++ ) {
        turn =0;
        cerr<<"runnning episode... "<<epi<<endl;
        episode_starty=rs.position.y;
        int y=rs.position.y,x=rs.position.x,vx=0,vy=0;
        double eps = 90*pow(0.99,epi);
        if(eps<=20)eps=20;
        // cerr<<"eps="<<eps<<endl;
        while(1){
            //cerr<<"x="<<x<<" y="<<y<<endl;
            //行動決定
            int action = generate_action_e(policyDP(x,y,vx,vy),eps);

            //報酬と次の状態
            auto S=generate_reward_and_next_state(x,y,vx,vy,action,rs,course);
            //cerr<<"reward="<<reward<<endl;
            //cerr<<"new_vx="<<new_vx<<" new_vy="<<new_vy<<endl;


            //9つのactionのうち最も大きな結果を求める
            double np_amax = -INF;
            for(int roop=0;roop<9;roop++){
                if(np_amax<QDP(S.new_x,S.new_y,S.new_vx,S.new_vy,roop)){
                    np_amax=QDP(S.new_x,S.new_y,S.new_vx,S.new_vy,roop);
                }
            }

            double alpha = 0.5;
            double gamma = 0.9;

            //cerr<<"S.reward="<<S.reward<<endl;
            //Q関数の更新
            QDP(x,y,vx,vy,action)+=alpha*(S.reward+gamma*np_amax-QDP(x,y,vx,vy,action));

            //9つのactionのうち最も大きな結果を出すものを求める
            double np_argmax_temp=-INF;
            int np_argmax=0;
            for(int roop=0;roop<9;roop++){
                if(np_argmax_temp<QDP(x,y,vx,vy,roop)){
                    np_argmax_temp=QDP(x,y,vx,vy,roop);
                    np_argmax=roop;
                }
            }

            //更新したQ関数に基づいてpolicyを更新
            policyDP(x,y,vx,vy)=np_argmax;

            //位置座標の更新
            x= S.new_x;
            y= S.new_y;
            vx = S.new_vx;
            vy = S.new_vy;

            //ゴールもしくは見てるコース外に行ったらエピソード終了
            if(game_over(x,y,rs,course))break;

            turn ++;
        }
    }
}

// rs.positon 敵の現在地　rs.velocity 現在の速度 course コースの情報
IntVec play(RaceState &rs, const Course &course) {

    int y=rs.position.y;
    int x=rs.position.x;
    int vy=rs.velocity.y;
    int vx=rs.velocity.x;
    //cerr<<"vx="<<vx<<" vy="<<vy<<endl;
    //cerr<<"x="<<x<<"y="<<y<<endl;


    int action=policyDP(x,y,vx,vy);




    Point ans=action_to_velocity(action);

    for(int x=0;x<20;x++){
        for(int y=0;y<100;y++){

            int temp=0;
            for(int roop=0;roop<9;roop++){
                for(int j=-2;j<2;j++){
                    for(int k=-2;k<4;k++){
                        // cerr<<"QDP("<<x<<","<<y<<","<<j<<","<<k<<","<<roop<<")="<<QDP(x,y,j,k,roop)<<endl;
                    }
                }
            }
        }
    }



    cerr<<"bestQ-point"<<"QDP("<<x<<","<<y<<","<<vx<<","<<vy<<","<<action<<")="<<QDP(x,y,vx,vy,action)<<endl;

    return Point(ans.x,ans.y);
}



int main(int argc, char *argv[]) {
    Course course(cin);
    cout << 0 << endl;
    cout.flush();
    RaceState rs(cin, course);
    //Qtableを初期化
    //rep(i,2)rep(j,2)rep(k,40)rep(l,40)rep(m,10)Q[i][j][k][l][m]=-100;
    //policyを初期化
    rep(i,110)rep(j,110)rep(k,40)rep(l,40)policy[i][j][k][l]=1;
    //Q学習(強化学習で政策テーブルを作成)
    q_learning(rs,course);

   // show_debug();

    //軽く視覚化
    /*
     *  現在位置の評価値を以下のように定める
     *
     *  (new_row,new_col)にたいして
     *
     *  rep(roop,9)rep()
     */

    IntVec accel = play(rs, course);
    cout << accel.x << ' ' << accel.y << endl;

    while (true) {
        RaceState rs(cin, course);
        IntVec accel = play(rs, course);
        cout << accel.x << ' ' << accel.y << endl;

    }
}
