#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <fstream>
#include <sstream>
using namespace std;
struct datum {                          //CSVファイルから読み込んだ一行ごとのデータ
    long long time;                          //yyyymmddhhmmss
    string address;                          //IPアドレス
    string ping;                             //ping値
    bool timeout;                            //タイムアウトかどうか

    datum(){
        long long time = 0;string address,ping = "0"; bool timeout = false;
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
        cout << "================ vector data ===============" << "  No." << i <<  endl;
        cout <<  " time:" << data[i].time << " IP address:" << data[i].address << " ping:" << data[i].ping << endl;
        cout << "================ vector data ===============" << endl;

    }
}

//datum構造体の全メンバを出力する関数
void outputDatum(datum data){
    //cout << "================  temp ===============" << endl;
    cout <<data.time << " " << data.address << " " << data.ping << endl;
    //cout << "================ temp ===============" << "\n\n" << endl;
}



int main() {
    vector<datum> timeoutServers;                   //タイムアウトしたサーバーデータを格納するvector
    vector<string> strBuffer(3);                    //データ読み込み用バッファ
    datum temp;                                     //一時保存用datum型構造体
    int lineNum = 0;                                //現在格納している行番号
    int diffSec = 0;                                //復旧までの時間
    vector<string> outputText;                      //CSV出力用vector
    string buf;                                     //CSV出力用vectorのバッファ
    bool exitTS = false;
    int tsNum = 0;

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
        if(temp.ping == "-") temp.timeout = true;    //ping値がタイムアウトの時はtimeoutフラグを立てる
        else temp.timeout = false;

        outputDatum(temp);

        exitTS = hasIP(timeoutServers,timeoutServers.size(),temp.address,tsNum);

            if(temp.timeout == true){                                                               //読み込んだIPがタイムアウトしている場合
                if(exitTS == false){                                                                //vectorに同じIPデータがない
                    timeoutServers.push_back(temp);
                    cout << "Temp data [" << temp.time << " " << temp.address << " " <<  temp.ping << "] is pushed TimeoutServer vector." << endl;
                }else{
                    //以前もタイムアウトしている場合はvectorに格納しない
                    cout << temp.address <<" has been pushed before." << endl;
                }
            }else{                                                                                  //読み込んだIPがタイムアウトしていないとき
                if(exitTS == true){                                                                 //vectorに同じIPデータがある
                    diffSec = minusDate(temp.time,timeoutServers[tsNum].time);                    //復旧までにかかった時間を計算する
                    timeoutServers.erase(timeoutServers.begin() + tsNum);                         //復旧したデータをvectorから削除する
                    cout << temp.address << " is removed from vector." << endl;
                    cout << "Restration time(sec) is " << diffSec << endl;
                    buf ="Restored Server's IP Address:" + temp.address + ",   Restoration time:" + to_string(diffSec) + "sec"; //bufに出力用の文を生成する
                    outputText.push_back(buf);                                                      //出力用vectorの末尾に追加していく

                }else{                                                                               //vectorに同じIPデータがないとき
                    //サーバーは一度もタイムアウトしていないので何もしない
                }
            }
           //outputVec(data);
    }
    //vector dataには故障して復旧していないデータが残っているので、outputTextに追加格納していく
    for(int i=0;i<timeoutServers.size();i++){
        buf = "Not responding Server's IP Address:" + timeoutServers[i].address ;
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
    cout.rdbuf(oldrdbuf);                                                   //標準出力へ戻す    
}