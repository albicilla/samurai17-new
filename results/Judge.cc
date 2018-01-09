#include <fstream>
#include <iostream>
#include <string>
using namespace std;
#define dump(x) cerr<<#x<<"="<<x<<endl


pair<double,double> pick_score(istream& ifs){
  string length;

  for(int i=0;i<4;i++){
    getline(ifs,length);
  }
  //cout<<length<<endl;

  //知りたい数字は二個目のセミコロンの右
  int t=2;
  int num=-1;
  string numS="";
  for(int i=0;i<length.size();i++){
    if(length[i]==':')t--;
    if(t==0 && length[i]!=' ' && length[i]!=':' && length[i]!=','){
      numS+=length[i];
    }
  }

  //obstacleの行を読み飛ばし
  //cout<<"numS="<<numS<<endl;
  num=stoi(numS);
  //cout<<"num="<<num<<endl;
  num+=5;
  for(int i=0;i<num;i++){
    getline(ifs,numS);
  }

  string timeA;
  string time0="",time1="";
  getline(ifs, timeA);
  //dump(timeA);
  int memoidx=0;
  t=9;
  for(int i=0;i<timeA.size();i++){
    t--;
    if(timeA[i]==','){
      memoidx=i;
      break;
    }
    if(t<=0 && timeA[i]!=' ' && timeA[i]!=':' && timeA[i]!=','){
      time0+=timeA[i];
    }
  }
  t=10;
  for(int i=memoidx;i<timeA.size();i++){
    t--;
    if(t<=0 && timeA[i]!=' ' && timeA[i]!=':' && timeA[i]!=','){
      time1+=timeA[i];
    }
  }
  // dump(time0);
  // dump(time1);
  double Time0=stod(time0);
  double Time1=stod(time1);
  //cerr<<Time0<<" "<<Time1<<endl;
  pair<double,double> ret=make_pair(Time0,Time1);
  return ret;

}

int main()
{
    /*
    Rは初の試合で赤だった方
    generated_result1とgenerated_result2は位置を入れ替えた連番の結果である必要がある。
    */
    double mutch=0;
    double R=0,B=0;
    for(int stage=1;stage<100;stage+=2){
    //  dump('a');
      string stream1="generated_result"+to_string(stage);
      string stream2="generated_result"+to_string(stage+1);
      stream1+=".out";
      stream2+=".out";
      // dump(stream1);
      // dump(stream2);
      std::ifstream ifs(stream1);
      std::ifstream jfs(stream2);

      string str;
      if (ifs.fail())
      {
          cerr << "失敗　でも気にすんな" << std::endl;
          break;
      }
      if(jfs.fail()){
        cerr<<"失敗2"<<endl;
        return -1;
      }
      pair<double,double> score=pick_score(ifs);

      //std::cout<<"time0="<<score.first<<" time1="<<score.second<<std::endl;

      pair<double,double> score1=pick_score(jfs);

      //std::cout<<"time0="<<score1.first<<" time1="<<score1.second<<std::endl;

      if(score.first+score1.second < score.second+score1.first){
        cout<<"R"<<endl;
        R++;
      }else if(score.first+score1.second > score.second+score1.first){
        cout<<"B"<<endl;
        B++;
      }else{
        cout<<"D"<<endl;
      }
      mutch++;

    }

    dump(R);
    dump(B);
    dump(mutch);
    cout<<"左のAIの勝率 "<<R/mutch *100<<"% 右のAIの勝率"<<B/mutch *100<<"%"<<endl;





    return 0;
}
