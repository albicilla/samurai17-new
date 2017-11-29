#include <bits/stdc++.h>
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
int maze[110][20];
string obstacle;

void dig(int x,int y,int len){
  maze[y][x]=0;
  if(y==len)return;
  //srand((unsigned int)time(NULL));
  //0左 1上 2上　3右
  random_device rnd;     // 非決定的な乱数生成器
  int togo=rnd()%4;
  if(togo==0){
    if(x>0)
    dig(x-1,y,len);
    else
    dig(x,y,len);
  }else if(togo==1 || togo==2){
    if(len==0)dig(x,y-1,len);
    else dig(x,y+1,len);
  }else{
    if(x<=14)
    dig(x+1,y,len);
    else
    dig(x,y,len);

  }
}
int main(){
 for(int i=0;i<100;i++){
   for(int j=0;j<15;j++){
     maze[i][j]=1;
   }
 }

 int x0,x1;
 x0=5; x1=9;

 rep(i,3)dig(x0,100,0);
 rep(i,3)dig(x1,100,0);
 rep(i,3)dig(x0,0,100);
 rep(i,3)dig(x1,0,100);



 for(int i=0;i<99;i++){
   obstacle+='[';
   for(int j=0;j<14;j++){
     //cerr<<maze[i][j];
     obstacle+=to_string(maze[i][j]);
     obstacle+=',';
   }
   obstacle+=to_string(maze[i][14]);
   obstacle+="],";
   //cerr<<endl;
 }
 obstacle+='[';
 rep(j,14){
   obstacle+='0';
   obstacle+=',';
 }
 obstacle+='0';
 obstacle+=']';

 cout<<"{\"filetype\":\"race course\",\"width\":15,\"length\":100,\"vision\":20,\"thinkTime\":20000,\"stepLimit\":100,\"x0\":5,\"x1\":9,\"obstacles\":";
 cout<<"["<<obstacle<<"]}"<<endl;


}
