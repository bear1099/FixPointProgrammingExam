#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;
struct datum {                                  //CSVファイルから読み込んだ一行ごとのデータ
    long long time;                             //yyyymmddhhmmss
    string address;                             //IPアドレス
    string ping;                                //ping値
    bool timeout;                               //タイムアウトかどうか
    int timeoutNum;                             //連続タイムアウト回数
    bool overload;                              //過負荷状態かどうか
    int overloadNum;                            //連続過負荷状態回数

    datum(){
        long long time = 0;string address,ping = ""; bool timeout,overload = false; int timeoutNum,overloadNum = 0;
    }
};

struct subNet{                                  //サブネット監視用の構造体
    long long time;
    int limit;
    int count;
    int prefix;
    int hostLength;
    string address;                             //IPアドレス
    string host;                                //ホスト部
    string network;                             //ネットワーク部
    string networkWithPeriod;                    //出力用ネットワーク部
    subNet(){
        int limit,count,prefix,hostLength = 0;string address,network,host,networkWithPeriod = "";
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

//Vectorに同じIPアドレスが存在するかを調べる関数(サブネット用)
bool hasIPforSub(vector<subNet> temp,size_t dataSize,string networkPart,int& exitNum){
    bool flag = false;
    for(int i=0;i<dataSize;i++){
        if(temp.at(i).network == networkPart){
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

//10進数を2進数変換する
string toBinary(int n)
{
    string r;
    while(n!=0) {r=(n%2==0 ?"0":"1")+r; n/=2;}
    return r;
}

//構造体subNetの情報を取得する関数
void getSubnet(string IP,subNet& temp){
    vector<string> buf(4);
    int i = 0;
    int result = 0;
    string s ="";
    stringstream stream(IP);
    stringstream stream2;
    string binaryIP ="";
    string binaryTemp ="";
    string binaryPart ="";
    string withPeriod = "";
    string zeros="";

    while(getline(stream,s,'/')){
        buf[i] = s;
        i++;
    }
    
    temp.address = buf[0];                          //IPアドレス部
    temp.prefix = stoi(buf[1]);                     //プレフィックス長
    temp.hostLength = 32 - temp.prefix;             //ホスト部の長さ
    s = "";
    stream2 << temp.address;
    while(getline(stream2,s,'.')){
        //int length = (int)s.length();
        binaryTemp = toBinary(stoi(s));
        int length = (int)binaryTemp.length();
        for(int i=0;i<8-length;i++){
            zeros.push_back('0');
        }
        binaryPart = zeros + binaryTemp;
        zeros.clear();
        binaryIP += binaryPart;
    }
    temp.network = binaryIP.substr(0,temp.prefix);  //ネットワーク部のアドレス取り出し(2進数)
    temp.host = binaryIP.substr(temp.prefix);       //ホスト部のアドレス取り出し(2進数)
    temp.limit = pow(2,temp.hostLength) - 2;        //限界ホスト数の計算
    //出力用のネットワーク部を作成する
    withPeriod = temp.network;
    if(withPeriod.length() > 9){
        withPeriod.insert(8,".");
        if(withPeriod.length() > 18){
            withPeriod.insert(17,".");
            if(withPeriod.length() > 27){
                withPeriod.insert(26,".");
            }
        }
    }
    temp.networkWithPeriod = withPeriod;

    }


int main() {
    vector<datum> timeoutServers;                   //タイムアウトしたサーバーデータを格納するvector
    vector<datum> brokenServers;                    //故障したサーバーデータを格納するvector
    vector<datum> overloadServers;                  //過負荷状態の恐れがあるサーバーデータを格納するvector
    vector<datum> overloadedServers;                //過負荷状態のサーバーデータを格納する
    vector<subNet> subnets;                         //サブネットの監視情報を格納するvector
    vector<string> strBuffer(3);                    //CSVデータ読み込み用バッファ
    int lineNum = 0;                                //現在格納している行番号
    int diffSec = 0;                                //復旧までの時間
    vector<string> outputText;                      //復旧サーバー情報出力用vector
    vector<string> overloadText;                    //過負荷サーバー情報出力用vector
    vector<string> subnetText;                      //サブネット情報出力用vector
    string buf;                                     //出力用vectorのバッファ
    bool exitTS,exitBS,exitOS,exitOedS = false;              //読み込んだデータがvectorに格納されているかを示すフラグ
    bool exitSubnet = false;
    int tsNum,bsNum,osNum,oedsNum = 0;                      //読み込んだデータが既にvectorに格納されている場合のインデックス
    int subnetNum;
    int N;                                          //故障とみなす連続タイムアウト回数
    int m,t;                                        //過負荷に関する変数

    cout << "[Message] Please enter timeout limit for judgeing broken servers : ";
    cin >> N;
    cout << "[Message] Please enter count limit and ping value limit for judgeing overload" << endl;
    cout << "[Message] counter limit: "; cin >> m;
    cout << "[Message] ping value limit: "; cin >> t;
    
    ifstream ifs("data.csv");
    if(!ifs) cerr << "[ERROR] Can't open selected file." << endl;
    string line = "";
    while (getline(ifs,line)) { 
        datum temp;                                  //サーバーデータ一時保存用datum型構造体
        subNet netTemp;                              //サブネットデータ一時保存用subNet型構造体  
        string str = "";
        istringstream stream(line);
        int i = 0;
        while(getline(stream,str,',')){
            strBuffer[i] = str;
            i++;
        }
        i = 0;
        temp.time = stoll(strBuffer[0]);             //string型日付をlong long型にして構造体tempに入れる
        temp.address = strBuffer[1];                 //string型IPアドレスを構造体tempに入れる
        temp.ping = strBuffer[2];                    //string型ping値を構造体tempに入れる
        temp.timeout = false;
        temp.overload = false;
        if(temp.ping == "-") temp.timeout = true;    //ping値がタイムアウトの時はtimeoutフラグを立てる
        else if(stoi(temp.ping) > t){
            temp.overload = true;
            temp.overloadNum = 1;
        }

        outputDatum(temp);
        //vectorに既に格納されているか、されていた場合のインデックスを調べる
        exitTS = hasIP(timeoutServers,timeoutServers.size(),temp.address,tsNum);
        exitBS = hasIP(brokenServers,brokenServers.size(),temp.address,bsNum);
        exitOS = hasIP(overloadServers,overloadServers.size(),temp.address,osNum);
        exitOedS = hasIP(overloadedServers,overloadedServers.size(),temp.address,oedsNum);
        //サブネット情報を取得する
        getSubnet(temp.address,netTemp);                                                                
        netTemp.time = temp.time;
        exitSubnet = hasIPforSub(subnets,subnets.size(),netTemp.network,subnetNum);                     //すでに監視されているか調べる

        //読み込んだデータがタイムアウトしている場合    
            if(temp.timeout == true){                                                                   
                //vectorに同じデータがない場合
                if(exitTS == false && exitBS == false){                                                 
                    temp.timeoutNum = 1;                                                                //初回のタイムアウト
                    timeoutServers.push_back(temp);                                                     //タイムアウトvectorに格納する
                    cout << "[Message] " << temp.address << " is pushed timeoutServers vector." << endl;
                    exitTS = hasIP(timeoutServers,timeoutServers.size(),temp.address,tsNum);            //格納されたインデックスを保存しておく
                //タイムアウトvectorに同じデータがある場合    
                }else if(exitTS == true && exitBS == false) timeoutServers[tsNum].timeoutNum++;         //連続タイムアウト回数をインクリメントする
                
                //タイムアウトサーバーvectorに格納したデータが連続タイムアウト閾値を超えていた場合、故障と判定する
                if(timeoutServers.empty() == false && timeoutServers[tsNum].timeoutNum >= N){           
                    brokenServers.push_back(timeoutServers[tsNum]);
                    timeoutServers.erase(timeoutServers.begin() + tsNum);
                    cout << "[Message] " << temp.address << " is moved from timeoutServers vector to brokenServers vector." << endl;
                    exitBS = hasIP(brokenServers,brokenServers.size(),temp.address,bsNum);              //格納されたインデックスを保存しておく
                    //サブネットの監視処理を行う
                    if(exitSubnet == true){                                                             //既に監視されている場合
                        subnets[subnetNum].count++;                                                     //故障ネットワークカウントをインクリメントする
                        subnets[subnetNum].time = netTemp.time;                                         //時間を更新
                        cout << "[Message] " << "Subnet : " << subnets[subnetNum].networkWithPeriod << ", Count : " << subnets[subnetNum].count << ", Limit : " << subnets[subnetNum].limit << " (" << subnets[subnetNum].limit + 2 << ")" << endl; 
                    }else{                                                                              //ネットワークアドレスが格納されていない場合
                        netTemp.count = 1;
                        subnets.push_back(netTemp);
                        exitSubnet = hasIPforSub(subnets,subnets.size(),netTemp.network,subnetNum);
                        cout << "[Message] " << "Watching subnet [ " << netTemp.networkWithPeriod << " ] ..." << endl; 
                    }

                }

                //連続過負荷サーバーvectorに同じデータがある場合
                if(exitOS == true){                                 
                    overloadServers.erase(overloadServers.begin() + osNum);
                    cout << "[Message] " << temp.address << " is removed from overloadServers vector." << endl;
                }
                if(exitOedS == true){
                    diffSec = minusDate(temp.time,overloadedServers[oedsNum].time);
                    overloadedServers.erase(overloadedServers.begin() + oedsNum);
                    cout << "[Message] " << temp.address << " is removed from overloadedServers vector" << endl;
                    cout << "[Message] Overload time(sec) is " << diffSec << endl;
                    buf = "[Overloaded Server's IP Address] :" + temp.address + ", [Overload time(sec)]:" + to_string(diffSec); 
                    overloadText.push_back(buf);
                }                
                
        //読み込んだデータが正常に応答している場合
            }else{                                                                                      
                //故障サーバーvectorに同じデータがある場合
                if(exitBS == true){                                                                     
                    diffSec = minusDate(temp.time,brokenServers[bsNum].time);                           //復旧までにかかった時間を計算する
                    brokenServers.erase(brokenServers.begin() + bsNum);                                 //復旧したサーバーデータを故障サーバーvectorから削除する
                    cout << "[Message] " <<temp.address << " is removed from brokenServers vector." << endl;
                    cout << "[Message] Server Restration time(sec) is " << diffSec << endl;
                    buf ="[Restored Server's IP Address]:" + temp.address + ", [Restoration time(sec)]:" + to_string(diffSec); //bufに出力用の文を生成する
                    outputText.push_back(buf);                                                          //出力用vectorの末尾に追加していく
                    //サブネット監視vectorの故障サーバーカウントを減らす
                    subnets[subnetNum].count--;
                    if(subnets[subnetNum].count + 1 == subnets[subnetNum].limit){
                        diffSec = minusDate(temp.time,subnets[subnetNum].time);
                        cout << "[Message] Subnet : " << subnets[subnetNum].networkWithPeriod << " is restored. Count: " << subnets[subnetNum].count << endl;
                        cout << "[Message] Subnet restration time(sec) is " << diffSec << endl;
                        buf = "[Restored Subnet's Network Address(Binary)]:" + subnets[subnetNum].networkWithPeriod + ", [Restoration time(sec)]:" + to_string(diffSec);
                        subnetText.push_back(buf);                        
                    }else if(subnets[subnetNum].count == 0){
                        subnets.erase(subnets.begin() + subnetNum);
                        cout << "[Message] Subnet's Address(Binary):" << netTemp.networkWithPeriod << " is all good status." << endl;
                     }

                //タイムアウトサーバーvectorに同じデータがある場合
                }else if(exitTS == true){                                                               
                    timeoutServers.erase(timeoutServers.begin() + tsNum);                               //タイムアウトサーバーvectorから出す
                    cout << "[Message] " << temp.address << " is removed from timeoutServers vector." << endl;
                }

                //過負荷サーバーvectorに同じデータがあるため、連続過負荷状態である場合
                if(temp.overload == true && exitOS == true){                                            
                    overloadServers[osNum].overloadNum++;                                               //連続過負荷状態回数をインクリメントする
                //初回の過負荷状態である場合
                }else if(temp.overload == true && exitOS == false && exitOedS == false){                                     //高pingのIPを読み込み、高負荷サーバーvectorに同じIPデータがない場合
                    overloadServers.push_back(temp);                                                    //過負荷サーバーvectorに新規追加する
                    exitOS = hasIP(overloadServers,overloadServers.size(),temp.address,osNum);          //インデックスを保存しておく
                    cout << "[Message] " << temp.address << " is pushed overloadServers vector." << endl;
                //過負荷サーバーvectorに同じデータがあるが、正常に応答している場合
                }else if(temp.overload == false && exitOS == true){                                     
                    overloadServers.erase(overloadServers.begin() + osNum);                             //過負荷サーバーvectorからデータを削除する
                    cout << "[Message] " << temp.address << " is removed from overloadServers vector." << endl;
                }else if(temp.overload == false && exitOedS == true){                                   //低pingのIPを読み込み、過負荷状態サーバーvectorに同じIPデータがある場合
                    diffSec = minusDate(temp.time,overloadedServers[oedsNum].time);
                    overloadedServers.erase(overloadedServers.begin() + oedsNum);
                    cout << "[Message] " << temp.address << " is removed from overloadedServers vector" << endl;
                    cout << "[Message] Overload time(sec) is " << diffSec << endl;
                    buf = "[Overloaded Server's IP Address] :" + temp.address + ", [Overload time(sec)]:" + to_string(diffSec);  
                    overloadText.push_back(buf);
                }

                //読み込んだデータが、連続過負荷状態閾値を超えた時
                if(overloadServers.empty() == false && overloadServers[osNum].overloadNum >= m){        
                    overloadedServers.push_back(overloadServers[osNum]);
                    overloadServers.erase(overloadServers.begin() + osNum);
                    cout << "[Message] " << temp.address << " is moved from overloadServers to overloadedServers vector." <<endl;
                }
            }

            /*cout << "TimeoutServers \n";
            outputVec(timeoutServers);
            cout << "BrokenServers \n";
            outputVec(brokenServers);
            cout << "OverloadServers \n";
            outputVec(overloadServers);*/
    }

    //brokenServers vectorには現在故障中のサーバー情報があるので、outputTextに追加格納していく
    for(int i=0;i<brokenServers.size();i++){
        buf = "[Not responding (broken) Server's IP Address]:" + brokenServers[i].address;
        outputText.push_back(buf);
    }    

    //timeoutServers vectorには故障の疑いがあるサーバー情報があるので、outputTextに追加格納していく
    for(int i=0;i<timeoutServers.size();i++){
        buf = "[Not responding (timeout) Server's IP Address]:" + timeoutServers[i].address;
        outputText.push_back(buf); 
    }

    //overloadedServers vectorには過負荷状態から復旧していないサーバー情報があるので、overloadTextに追加格納していく
    for(int i=0;i<overloadedServers.size();i++){
        buf = "[Now overloading Server's IP Address]:" + overloadedServers[i].address;
        overloadText.push_back(buf);
    }

    //subnets vectorにはサブネットが故障したままのサブネット情報があるので、subnetTextに追加格納していく
    for(int i=0;i<subnets.size();i++){
        if(subnets[i].count >= subnets[i].limit){
            buf = "[Still not responding Subnet Address]:" + subnets[i].networkWithPeriod;
            subnetText.push_back(buf); 
        }
    }
    

    
    //outputTextをファイルに書き込む処理を行う
    streambuf* last = cout.rdbuf();
    string filename1 = "result.txt";
    string filename2 = "overload.txt";
    string filename3 = "subnet.txt";
    ofstream ofs(filename1);
    cout.rdbuf(ofs.rdbuf());
    for(int i=0;i<outputText.size();i++){
        cout << outputText[i] << endl;
    }

    ofstream ofs2(filename2);
    cout.rdbuf(ofs2.rdbuf());
    for(int i=0;i<overloadText.size();i++){
        cout << overloadText[i] << endl;
    }

    ofstream ofs3(filename3);
    cout.rdbuf(ofs3.rdbuf());
    for(int i=0;i<subnetText.size();i++){
        cout << subnetText[i] << endl;
    }
    cout.rdbuf(last);
}