# フィックスポイントプログラミング試験
静岡大学情報学部情報科学科3年の大熊崇仁です。  
この度は、プログラミング試験の受験ありがとうございました。  
設問4まで完了してあります。設問ごとにフォルダ"Task1"から"Task4"でアップロードさせていただきました。  
本プログラムはC++で作成しています。  
テストデータと実行結果は、それぞれのTaskフォルダにReport.pdfでまとめてあります。  

## プログラムの主な流れ
1. ログファイルから一行取り出します。
2. タイムアウトや、高Pingなどの情報を取得します。
3. 既に監視しているサーバーであるかを判断します。
4. 2と3で得られた情報を用いて、条件分岐を行い処理を行います。
5. 処理によって得られた結果を出力します。

## 実行方法について
1. それぞれの課題に応じたTaskフォルダにmain.cppが存在するので、対象の課題のディレクトリを移動してください。
2. 環境に応じて、C++のコンパイルコマンドを実行してください。(Win10 MinGW環境でg++コマンドを用いて動作確認しています)
3. コンパイラで生成された実行ファイルをコマンドプロンプト等で実行してください。
(課題2から)  
5. N回以上タイムアウトした場合に故障とみなすかを標準入力してください。  
(課題3から)  
6. 直近m回の平均応答時間がtミリ秒を超えた時に過負荷状態とみなすかを、mからtの順番で標準入力してください。

## 実行結果について
・読み込んだデータがどのような処理が行われているかをコマンドプロンプト等に標準出力されています。  
・故障に関する実行結果がresult.txtに出力されています。  
・過負荷に関する実行結果がoverload.txtに出力されています。  
・サブネットに関する実行結果がsubnet.txtに出力されています。  

## 細やかな仕様について
・監視ログファイルはdata.csvと想定しています。  
・故障期間等は全て秒単位で出力しています。  
・一行ごとにdata.csvファイルから読み込んでいくので、データが膨大になってもある程度対応できると予想しています。  
・最後にタイムアウトしてから、正常な応答ログなしに監視ログファイルの読み込みが終わった場合、タイムアウトしたままであることを出力します。  
・故障とみなしてから、復旧せずに監視ログファイルの読み込みが終わった場合、故障したままであることを出力します。  
・過負荷状態とみなしてから、正常な応答ログなしに監視ログファイルの読み込みが終わった場合、過負荷状態のままであることを出力します。  
・サブネットが故障とみなす際に、IPアドレスのホスト部のビット長から得られる最大ホスト数からネットワークアドレスとブロードキャストアドレスを引いた数を限界ホスト数として、  
　全てのホストが故障していることを判定しています。  
 
 ## テストデータについて
 ### 設問1,2
 設問に提示された仕様をテストすることに加えて、以下のことを確かめられるテストデータを用意しました  
・初回読み込みのデータがタイムアウトしていても、故障とみなすか  
・連続して同じサーバーが故障しているデータを読み込んだ際に、適切に処理が行えるか  
・二回以上の故障期間があるサーバーが存在する監視ログファイルを読み込んだ際に、それぞれの故障期間を出力できるか  

 ### 設問3
 設問に提示された仕様をテストすることに加えて、以下のことを確かめられるテストデータを用意しました  
 ・二回以上の過負荷期間があるサーバーが存在する監視ログファイルを読み込んだ際に、それぞれの過負荷期間を出力できるか  
 ・過負荷状態からタイムアウトした場合に、過負荷期間を出力できるか

 ### 設問4
 設問に提示された仕様をテストすることに加えて、以下のことを確かめられるテストデータを用意しました  
 ・二回以上のサブネット故障期間がある監視ログファイルを読み込んだ際に、それぞれの過負荷期間を出力できるか  

 ## プログラムの内容について
 ### 作成した構造体
 ・datum  
  CSVファイルから読み込んだ一行ごとのデータを扱う際の構造体です  
```
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
```
・subNet  
　サブネットを監視するための構造体です  
 ```
 struct subNet{                                  //サブネット監視用の構造体
    long long time;
    int limit;                                  //限界ホスト数
    int count;                                  //故障したホスト数
    int prefix;                                 //ネットワークアドレス長
    int hostLength;                             //ホストアドレス長
    string address;                             //IPアドレス
    string host;                                //ホスト部(二進数)
    string network;                             //ネットワーク部(二進数)
    string networkWithPeriod;                   //出力用ネットワーク部(ネットワーク部を見やすいようにピリオドを追加したもの)
    subNet(){
        int limit,count,prefix,hostLength = 0;string address,network,host,networkWithPeriod = "";
    }
};
```
上記の二つの構造体をvectorに格納することで扱っていきます。  

### 主な関数について  
・tm calcDate(tm a,long long time)  
　yyyymmddhhmmss形式の日付をtmが構造体に変換します。故障期間などを計算する際に扱う関数です。  
 
・double minusDate(long long afTime,long long beTime)  
  yyyymmddhhmmss形式で与えられた時間の差を計算します。上記のcalcDate関数を本関数内で呼び出します。  
  故障期間などを計算する際に扱う関数です。  
  
・bool hasIP(vector<datum> data,size_t dataSize,string IP,int& exitNum)  
  datum型構造体を格納しているvectorに同じIPアドレスのデータがないかを調べる関数です。また、データがあった場合はインデックスも調べます。  
  
・bool hasIPforSub(vector<subNet> temp,size_t dataSize,string networkPart,int& exitNum)  
  subNet型構造体を格納しているvectorに同じIPアドレスのデータがないかを調べる関数です。また、データがあった場合はインデックスも調べます。  
  
・string toBinary(int n)  
  引数によって与えられた10進数の数値を2進数に変換します。  
  
・void getSubnet(string IP,subNet& temp)  
　subNet型構造体の情報を取得します。引数によって与えられた0.0.0.0/0形式のIPアドレスを元に、限界ホスト数や、ネットワーク部とホスト部の2進表記のアドレスなどを取得します。  
 
