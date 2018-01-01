#include <map>
#include <queue>
#include <limits>
#include <tuple>
#include <iostream>
#include <random>
#include "raceState.hpp"

#define INF 1e9
#define TEISU 7
/*
 * 負の添え字を扱うためのマクロ
 * 参考:http://albicilla.hatenablog.com/
 *
 * Q new_row,new_col,new_vv,new_vh,roop
 */

double Q[110][110][40][40][10];
double policy[110][110][40][40];
//現在地
int start_y,start_x;

#define QDP(i,j,k,l,m)  Q[(i)+(int)3][(j)+(int)3][(k)+(int)15][(l)+(int)15][(m)]
#define policyDP(i,j,k,l) policy[(i)+(int)3][(j)+(int)3][(k)+(int)15][(l)+(int)15]

int debug[30][100];
void show_debug(){
    for(int x=-1;x<20;x++){
        for(int y=-1;y<100;y++){

            int temp=0;
           // for(int roop=0;roop<9;roop++){
                for(int j=-5;j<5;j++){
                    for(int k=-5;k<5;k++){
                        temp+=policyDP(y,x,j,k);
                    }
                }
           // }

            debug[x][y]=temp;

        }
    }


    for(int i=-1;i<20;i++){
        for(int j=-1;j<40;j++){
            cerr<<debug[i][j]<<" ";
        }
        cerr<<endl;
    }
    cerr<<endl;
}

//イプシロングリーディでランダムな結果か政策の結果を採用するかの決定
/*
 * 参考 http://vivi.dyndns.org/tech/cpp/random.html
 */
int generate_action_e(int action,int eps){
    std::random_device rnd;     // 非決定的な乱数生成器
    if(rnd()%100<eps){
        return rnd()%9;
    }else{
        //cerr<<"takea ation from policy"<<action<<endl;
        return action;
    }
}

//ゴール
bool game_over(int final_r,int final_c,const RaceState &rs,const Course &rt){
    if(rt.width <= final_c && 0<=final_r && final_r>=start_y+TEISU){
        //cerr<<"gameover"<<endl;
        return 1;
    }
    else return false;
}
//場外
bool outside(int final_r,int final_c,const Course &rt){
    if(0>final_r || final_r>rt.length || 0>final_c || final_c>rt.width)return 1;
    else {
        //cerr<<"not outside"<<endl;
        return false;
    }
}
//衝突
bool collision(int r,int c,int final_r,int final_c,const RaceState &rs,const Course &course){
    //cerr << "r="<<r<<" c="<<c<<endl;
    //cerr<<"final_r="<<final_r <<" final_c="<<final_c<<endl;

    if(!course.obstacled(Point(c,r), Point(final_c,final_r))){
        //cerr<<"not collision"<<endl;
        return false;
    }else {
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

velo_set change_velocity(int av, int ah, int action){

    if(action==0){
        av++;
        ah--;
    }
    if(action==1){
        av++;
    }
    if(action==2){
        av++;
        ah++;
    }
    if(action==3){
        ah--;
    }
    if(action==5){
        ah++;
    }
    if(action==6){
        av--;
        ah--;
    }
    if(action==7){
        av--;
    }
    if(action==8){
        av--;
        ah++;
    }

    velo_set ret={av,ah};
    return ret;
}

Point action_to_velocity(int action) {

    int av=0,ah=0;
    if(action==0){
        av++;
        ah--;
    }
    if(action==1){
        av++;
    }
    if(action==2){
        av++;
        ah++;
    }
    if(action==3){
        ah--;
    }
    if(action==5){
        ah++;
    }
    if(action==6){
        av--;
        ah--;
    }
    if(action==7){
        av--;
    }
    if(action==8){
        av--;
        ah++;
    }
    Point ret={av,ah};
    return ret;
}

//グローバルでやって良いか？
int reward=0, new_row=0, new_col=0, new_vv=0, new_vh=0;


//とった行動によって得られる報酬と次の状態を
void generate_reward_and_next_state(int row,int col,int vv,int vh,int action,const RaceState &rs,const Course &course){

    //vv,vhを更新
    auto nv = change_velocity(vv,vh,action);

    int final_r,final_c;
    final_r = row + nv.y;
    final_c = col + nv.x;
   // cerr << course.length<<" "<<course.width<<endl;


    if(game_over(final_r,final_c,rs,course)){
        reward=1000;
        new_row=final_r;
        new_col=final_c;
        new_vv=0;
        new_vh=0;
        return ;
    }else if(outside(final_r,final_c,course)){
        reward=-100;
        new_row=row;
        new_col=col;
        new_vv=vv;
        new_vh=vh;
        //cerr<<"outside"<<endl;
        return ;
    }else if(collision(row,col,final_r,final_c,rs,course)){
        reward=-10;
        new_row=row;
        new_col=col;
        new_vv=vv;
        new_vh=vh;
        return ;
    }

    //cerr<<"can go to"<<final_r<<" "<<final_c<<endl;

    reward=-1;
    new_row=final_r;
    new_col=final_c;
    new_vv=vv;
    new_vh=vh;


}



//TODO

//c++に書き換えないといけないpythonの関数たち
/*
 *
 *np.amax() done
 *np.argmax() done
 */
/*
 * change_velocity() done
 */


// rs.positon 駒の位置　rs.velocity 現在の速度 course コースの情報
void q_learning(const RaceState &rs, const Course &course){
    start_y=rs.position.y;
    start_x=rs.position.x;
    cerr<<"start_x="<<start_x<<" start_y"<<start_y<<endl;
    for(int i=0;i<10; i++ ) {
        int row=rs.position.y,col=rs.position.x,vv=0,vh=0;
        double eps = 40*pow(0.8,i);

        while(1){
            //cerr<<"row="<<row<<" col="<<col<<endl;
            //行動決定
            int action = generate_action_e(policyDP(row,col,vv,vh),eps);

            //報酬と次の状態
            //グローバルでやって良いか？
            //int reward, new_row, new_col, new_vv, new_vh;
            generate_reward_and_next_state(row,col,vv,vh,action,rs,course);
            //cerr<<"reward="<<reward<<endl;
            //9つのactionのうち最も大きな結果を求める
            double np_amax = -1e9;
            for(int roop=0;roop<9;roop++){
                if(np_amax<Q[new_row][new_col][new_vv][new_vh][roop]){
                    np_amax=Q[new_row][new_col][new_vv][new_vh][roop];
                }
            }

            double alpha = 0.9;
            double gamma = 0.9;
            //Q関数の更新
            QDP(new_row,new_col,new_vv,new_vh,action)+=alpha*(reward+gamma*np_amax)-Q[row][col][vv][vh][action];

            //9つのactionのうち最も大きな結果を出すものを求める
            double np_argmax_temp=-INF;
            int np_argmax=0;
            for(int roop=0;roop<9;roop++){
                if(np_argmax_temp<QDP(new_row,new_col,new_vv,new_vh,roop)){
                    np_argmax_temp=QDP(new_row,new_col,new_vv,new_vh,roop);
                    np_argmax=roop;
                }
            }

            //更新したQ関数に基づいてpolicyを更新
            policyDP(row,col,vv,vh)=np_argmax;

            //位置座標の更新
            row= new_row;
            col= new_col;
            vv = new_vv;
            vh = new_vh;

            //ゴールもしくはコース外に行ったらエピソード終了
            if(game_over(row,col,rs,course))break;
        }
    }
}






// rs.positon 敵の現在地　rs.velocity 現在の速度 course コースの情報
IntVec play(RaceState &rs, const Course &course) {

    int row=rs.position.y;
    int col=rs.position.x;
    int vv=rs.velocity.y;
    int vh=rs.velocity.x;


    int action=policyDP(row,col,vv,vh);

    Point ans=action_to_velocity(action);
    return Point(ans.x,ans.y);
}



int main(int argc, char *argv[]) {
    Course course(cin);
    cout << 0 << endl;
    cout.flush();
    RaceState rs(cin, course);
    //Q学習(強化学習で政策テーブルを作成)
    q_learning(rs,course);

    show_debug();

    //軽く視覚化
    /*
     *  現在位置の評価値を以下のように定める
     *
     *  (new_row,new_col)にたいして
     *
     *  rep(roop,9)rep()
     */


    while (true) {
        IntVec accel = play(rs, course);
        cout << accel.x << ' ' << accel.y << endl;
        RaceState rs(cin, course);
    }
}
