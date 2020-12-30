#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <fstream>
#include <sstream>
using namespace std;
struct datum {                                  //CSVファイルから読み込んだ一行ごとのデータ
    long long time;                             //yyyymmddhhmmss
    string address;                             //IPアドレス
    string ping;                                //ping値
    bool timeout;                               //タイムアウトかどうか
    int timeoutNum;                             //連続タイムアウト回数

    datum(){
        long long time = 0;string address,ping = "0"; bool timeout = false; int timeoutNum = 0;
    }
};

//yyyymmddhhmmssをtm型構造体に計算する
tm calcDate(tm a,long long time){
    a.tm_year = time / 10000000000 - 1900;
    a.tm_mon = (time / 100000000) % 100 - 1; 
    a.tm_mday = (time / 1000000) % 100;
    a.tm_hour = (time / 10000) % 100;
    a.tm_min = (time / 100) % 100;
    a.tm_sec = time % 100;
    return a;
}

//日付同士を減算する関数
double minusDate(long  long afTime,long long beTime){
    tm time1 = calcDate(time1,beTime);
    tm time2 = calcDate(time2,afTime);
    time_t ret1 = mktime(&time1);
    if(ret1 == -1) cerr << "Can't calculate after time in calcDate method."<< endl;
    time_t ret2 = mktime(&time2);
    if(ret2 == -1) cerr << "Can't calculate before time in calcDate method." << endl;
    double result = difftime(ret2,ret1);
    return result;
}



//それまでのデータに同じIPアドレスが存在するかを調べる関数 T:同じIPが存在する F:同じIPが存在しない
bool hasIP(vector<datum> data,size_t dataSize,string IP,int& exitNum){
    bool flag = false;
    for(int i=0;i<dataSize;i++){
        if(data.at(i).address == IP){
            flag = true;
            exitNum = i;
            break;
        }
    }
    return flag;
}

//Vectorの全要素を出力する関数
void outputVec(vector<datum> data){
    for(int i=0;i<data.size();i++){
        //cout << "================ vector data ===============" << "  No." << i <<  endl;
        cout <<  " time:" << data[i].time << " IP address:" << data[i].address << " ping:" << data[i].ping << endl;
        //cout << "================ vector data ===============" << endl;

    }
}

//datum構造体の全メンバを出力する関数
void outputDatum(datum data){
    //cout << "================  temp ===============" << endl;
    cout << "[Message] Reading data is [ Date: " << data.time << ", IP: " << data.address << ", Ping:" << data.ping << " ]" << endl;
    //cout << "================ temp ===============" <<  endl;
}



int main() {
    vector<datum> timeoutServers;                   //タイムアウトしたサーバーデータを格納するvector
    vector<datum> brokenServers;                    //故障したサーバデータを格納するvector
    vector<string> strBuffer(3);                    //データ読み込み用バッファ
    datum temp;                                     //一時保存用datum型構造体
    int lineNum = 0;                                //現在格納している行番号
    int diffSec = 0;                                //復旧までの時間
    vector<string> outputText;                      //CSV出力用vector
    string buf;                                     //CSV出力用vectorのバッファ
    bool exitTS = false;
    bool exitBS = false;                            //読み込んだデータがvectorに格納されているかを示すフラグ
    int tsNum,bsNum = 0;                            //読み込んだデータが既にvectorに格納されている場合のインデックス
    int N;                                          //故障とみなす連続タイムアウト回数


    cout << "[Message] Please enter timeout limit for judgeing broken servers : ";
    cin >> N;
    
    ifstream ifs("data.csv");
    if(!ifs) cerr << "[ERROR] Can't open selected file." << endl;
    string line = "";
    while (getline(ifs,line)) {                             
        string str = "";
        istringstream stream(line);
        int i = 0;
        while(getline(stream,str,',')){
            strBuffer[i] = str;
            i++;
        }
        i = 0;
        temp.time = stoll(strBuffer[0]);                //string型日付をlong long型にして構造体tempに入れる
        temp.address = strBuffer[1];                    //string型IPアドレスを構造体tempに入れる
        temp.ping = strBuffer[2];                       //string型ping値を構造体tempに入れる

        if(temp.ping == "-") temp.timeout = true;        //ping値がタイムアウトの時はtimeoutフラグを立てる
        else temp.timeout = false;
        outputDatum(temp);
        //vectorに既に格納されているか、されていた場合のインデックスを調べる
        exitTS = hasIP(timeoutServers,timeoutServers.size(),temp.address,tsNum);
        exitBS = hasIP(brokenServers,brokenServers.size(),temp.address,bsNum);
            if(temp.timeout == true){                                                                   //読み込んだIPがタイムアウトしている場合
                if(exitTS == false && exitBS == false){                                                 //vectorに同じIPデータがない
                    temp.timeoutNum = 1;                                                                //初回のタイムアウト
                    timeoutServers.push_back(temp);                                                     //タイムアウトvectorに格納する
                    cout << "[Message] " << temp.address << " is pushed timeoutServers vector." << endl;
                    exitTS = hasIP(timeoutServers,timeoutServers.size(),temp.address,tsNum);            //格納されたインデックスを保存しておく
                }else if(exitTS == true && exitBS == false) timeoutServers[tsNum].timeoutNum++;         //タイムアウトサーバーvectorにのみ同じIPデータがある場合連続タイムアウト回数をインクリメントする
                
                if(timeoutServers.empty() == false && timeoutServers[tsNum].timeoutNum >= N){                                              //タイムアウトサーバーvectorに格納したデータが故障閾値を超えていた時
                    brokenServers.push_back(timeoutServers[tsNum]);
                    timeoutServers.erase(timeoutServers.begin() + tsNum);
                    cout << "[Message] " << temp.address << " is moved from timeoutServers vector to brokenServers vector." << endl;
                }                
            }else{                                                                                      //読み込んだIPがタイムアウトしていないとき
                if(exitBS == true){                                                                     //故障サーバーvectorに同じIPデータがある
                    diffSec = minusDate(temp.time,brokenServers[bsNum].time);                           //復旧までにかかった時間を計算する
                    brokenServers.erase(brokenServers.begin() + bsNum);                                 //復旧したサーバーデータを故障サーバーvectorから削除する
                    cout << "[Message] " <<temp.address << " is removed from brokenServers vector." << endl;
                    cout << "[Message] Restration time(sec) is " << diffSec << endl;
                    buf ="Restored Server's IP Address:" + temp.address + ",   Restoration time(sec):" + to_string(diffSec); //bufに出力用の文を生成する
                    outputText.push_back(buf);                                                          //出力用vectorの末尾に追加していく
                }else if(exitTS == true){                                                               //タイムアウトサーバーvectorに同じIPデータがある
                    timeoutServers.erase(timeoutServers.begin() + tsNum);                               //タイムアウトサーバーvectorから出す
                    cout << "[Message] " << temp.address << " is removed from timeoutServers vector." << endl;
                }
            }

  

            //cout << " TimeoutServers \n";
            //outputVec(timeoutServers);
            //cout << " BrokenServers \n";
            //outputVec(brokenServers);
            //outputVec(data);
    }

    //brokenServers vectorには現在故障中のサーバー情報があるので、outputTextに追加格納していく
    for(int i=0;i<brokenServers.size();i++){
        buf = "Not responding (broken) Server's IP Address:" + brokenServers[i].address;
        outputText.push_back(buf);
    }    

    //timeoutServers vectorには故障の疑いがあるサーバー情報があるので、outputTextに追加格納していく
    for(int i=0;i<timeoutServers.size();i++){
        buf = "Not responding (timeout) Server's IP Address:" + timeoutServers[i].address;
        outputText.push_back(buf); 
    }
    

    
    //outputTextをCSVファイルに書き込む処理を行う
    ofstream writeFile;
    string filename = "result.txt";
    writeFile.open(filename);
    ofstream ofs(filename);
    streambuf* oldrdbuf = cout.rdbuf(ofs.rdbuf());
    for(int i=0;i<outputText.size();i++){
        cout << outputText[i] << endl;
    }
    cout.rdbuf(oldrdbuf);       //標準出力へ戻す    

}