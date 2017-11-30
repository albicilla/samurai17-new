#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;
#define FOR(i,a,b) for(int i=a;i<b;i++)
#define rep(i,b) FOR(i,0,b)
#define INF mugen
#define dump(x) cerr<<#x<<"="<<x<<endl
#define all(a) (a).begin(),(a).end()
typedef vector<int> V;
typedef vector<V> VV;
typedef vector<VV> VVV;
template <class T> void chmin(T & a, T const & b) { if (b < a) a = b; }

using ll = long long;
const ll mod = LLONG_MAX;

//高さ　横
int height=100,width=15;
//上を選ぶ確率
int choice_up=70;
int thinkTime = 20000;
int repeat=3;
int maze[1123456][1123456];
string obstacle;

void dig(int x,int y,int len){
  maze[y][x]=0;
  if(y==len)return;
  //srand((unsigned int)time(NULL));
  //choice_upの確率で上に低いとエラーが出るので注意
  random_device rnd;     // 非決定的な乱数生成器
  int togo=rnd()%100+1;
  if(togo<=choice_up){
    //cout<<"up"<<endl;
    if(len==0)dig(x,y-1,len);
    else dig(x,y+1,len);
  }else if(togo <= (100-choice_up )/2 + choice_up){
    if(x>0)
    dig(x-1,y,len);
    else
    dig(x,y,len);
  }else{
    if(x<=width-1)
    dig(x+1,y,len);
    else
    dig(x,y,len);

  }
}
int ctoi(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	return 0;
}

int main(int argc,char* argv[]){


 //オプションの受け取り
 int opt;

 opterr = 0; //getopt()のエラーメッセージを無効にする。

 while ((opt = getopt(argc, argv, "x:y:g:t:r:h")) != -1) {
    //コマンドライン引数のオプションがなくなるまで繰り返す
    switch (opt) {
        case 'x':
            //printf("-xがオプションとして渡されました\n");
            width = atoi(optarg);
            break;

        case 'y':
            //printf("-yがオプションとして渡されました\n");
            height = atoi(optarg);
            break;
        case 'g':
            //printf("-gがオプションとして渡されました\n");
            //printf("引数optarg = %s\n", optarg);
            choice_up= atoi(optarg);
            //cout<<choice_up<<endl;
            break;
        case 't':
          thinkTime = atoi(optarg);
          break;
        case 'r':
          repeat = atoi(optarg);
        case 'h':
            //printf("-hがオプションとして渡されました\n");
            printf("-x [argument] argumentの幅 -y [argument] argumentの高さ -g [argument] argumentの確率で上方向を選択 既定値70 \n");
            break;

        default: /* '?' */
            //指定していないオプションが渡された場合
            printf("Usage: %s [-f arg] [-g arg] [-h] arg1 ...\n", argv[0]);
            break;
    }
 }
 for(int i=0;i<height;i++){
   for(int j=0;j<width;j++){
     maze[i][j]=1;
   }
 }

 int x0,x1;
 x0=5; x1=9;

 //cout<<"初期化終了"<<endl;

 rep(i,repeat)dig(x0,height,0);
 rep(i,repeat)dig(x1,height,0);
 rep(i,repeat)dig(x0,0,height);
 rep(i,repeat)dig(x1,0,height);

 //cout<<"生成終了"<<endl;

 for(int i=0;i<height-1;i++){
   obstacle+='[';
   for(int j=0;j<width-1;j++){
     //cerr<<maze[i][j];
     obstacle+=to_string(maze[i][j]);
     obstacle+=',';
   }
   obstacle+=to_string(maze[i][width-1]);
   obstacle+="],";
   //cerr<<endl;
 }
 obstacle+='[';
 rep(j,width-1){
   obstacle+='0';
   obstacle+=',';
 }
 obstacle+='0';
 obstacle+=']';

 cout<<"{\"filetype\":\"race course\",\"width\":";
 cout<<width;
 cout<<",\"length\":";
 cout<<height;
 cout<<",\"vision\":20,\"thinkTime\":";
 cout<<thinkTime;
 cout<<",\"stepLimit\":100,\"x0\":5,\"x1\":9,\"obstacles\":";
 cout<<"["<<obstacle<<"]}"<<endl;


}
