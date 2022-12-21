// 文字列を受信して表示するサーバ

#include <iostream>		// 入出力用
#include <WinSock2.h>	// WinSock用
#include <ws2tcpip.h>	// WinSock用
#include <vector>
#include <string.h>

#pragma comment( lib, "ws2_32.lib" )

const unsigned short PORT = 8080;	//サーバのポート番号

//位置
struct Position
{
    float x;
    float y;
    float z;
};

//送る情報
struct SendInfo
{
    Position pos;
    float axisAngle;
    bool isShot;
    bool isDead;
};

//情報(アドレス、ポート)
struct Info
{
    char ipAddr[256];  //アドレス保存
    char portstr[256]; //ポート保存
    int  id;           //ID番号
};

//クライアントの情報 第一引数 : クライアントのソケットアドレス
std::vector<std::pair<Info,SOCKADDR_IN>> ClientInfo;

//プレイ人数
int PlayerMaxNumber;

//前方宣言
BOOL Receiving(SOCKET sock);
BOOL PlaySending(SOCKET sock,SendInfo data, char ipAddr[], char portstr[]);
BOOL LobySending(SOCKET sock,char ipAddr[], char portstr[]);



int main()
{
    std::cout << "Server\n";

    // WinSockの初期化
    WSADATA  wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // エラー処理
    }

    // UDPソケットの作成	socket()
    SOCKET  sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        // エラー
    }

    // ポート番号割り当て
    SOCKADDR_IN  bindAddr;
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(PORT);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (SOCKADDR*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
    {
        // エラー処理
    }


    while (true)
    {
        //受信
        Receiving(sock);
    }

    // UDPソケット破棄		closesocket()
    if (closesocket(sock) == SOCKET_ERROR)
    {
        // エラー
    }

    // WinSock終了
    if (WSACleanup() != 0)
    {
        // エラー処理
    }

    return 0;
}

//受信
BOOL Receiving(SOCKET sock)
{
    // クライアントのソケットアドレス
    SOCKADDR_IN fromAddr;
    int fromlen = sizeof(fromAddr);

    // 構造体受信した格納領域
    SendInfo info;	

    int ret = recvfrom(sock, (char*)&info, sizeof(info), 0, (SOCKADDR*)&fromAddr, &fromlen);
    if (ret == SOCKET_ERROR)
    {
        // エラー処理
    }

    char ipAddr[256];  //アドレス
    char portstr[256]; //ポート

    //受信データがあればここでその時の処理を書く
    inet_ntop(AF_INET, &fromAddr.sin_addr, ipAddr, sizeof(ipAddr));
    sprintf_s(portstr, "%d", ntohs(fromAddr.sin_port));

    //追加するかどうか
    bool IsAdd = false;

    //保存しているクライアントの情報とかぶっていなければ情報保存
    for (auto i = ClientInfo.begin(); i != ClientInfo.end(); i++)
    {
        //もしアドレスとポート番号が違うのなら
        if ((*i).first.ipAddr != ipAddr && (*i).first.portstr != portstr)
        {
            IsAdd = true;
        }
    }

    //もしベクターの中身が空ならば
    if(ClientInfo.empty())
        IsAdd = true;

    //もし追加するのなら
    if (IsAdd)
    {
        //ベクターに保存する用の変数を用意
        std::pair<Info, SOCKADDR_IN> a;

        strcpy_s(a.first.ipAddr,ipAddr);      //アドレス設定  
        strcpy_s(a.first.portstr,portstr);    //ポート設定
        a.second = fromAddr;                //クライアントのソケットアドレス設定

        //vector設定
        ClientInfo.push_back(a);        

        //送信
        LobySending(sock,ipAddr, portstr);
    }
    //もしすでに追加されているのなら
    else
    {
        //変換した構造体を入れるようの変数
        SendInfo data;

        //実際に使うためにバイトオーダーを変換
        data.pos.x = ntohl(info.pos.x);
        data.pos.y = ntohl(info.pos.y);
        data.pos.z = ntohl(info.pos.z);
        data.axisAngle = ntohl(info.axisAngle);
        data.isDead = ntohl(info.isDead);
        data.isShot = ntohl(info.isShot);

        //他の人に向けて
        PlaySending(sock, data, ipAddr, portstr);
    }

    return TRUE;
}

//Playしている時の送信
BOOL PlaySending(SOCKET sock,SendInfo data, char ipAddr[], char portstr[])
{
    for (auto i = ClientInfo.begin(); i != ClientInfo.end(); i++)
    {
        //もしアドレスとポート番号が違うのなら
        if (strcmp((*i).first.ipAddr, ipAddr) != 0 && strcmp((*i).first.portstr, portstr) != 0)
        {
            //変換した構造体を入れるようの変数
            SendInfo sendData;

            //バイトオーダー変換
            sendData.pos.x = htonl(data.pos.x);
            sendData.pos.y = htonl(data.pos.y);
            sendData.pos.z = htonl(data.pos.z);
            sendData.axisAngle = htonl(data.axisAngle);
            sendData.isDead = htonl(data.isDead);
            sendData.isShot = htonl(data.isShot);

            //送信
            sendto(sock, (char*)&sendData, sizeof(sendData), 0, (SOCKADDR*)&(*i).second, sizeof((*i).second));
        }
    }

    return TRUE;
}

//ロビー時の送信
BOOL LobySending(SOCKET sock,char ipAddr[], char portstr[])
{
    //送るID
    int ID = 0;

    for (auto i = ClientInfo.begin(); i != ClientInfo.end(); i++)
    {
        //もしアドレスとポート番号が一緒なら
        if (strcmp((*i).first.ipAddr,ipAddr) == 0 && strcmp((*i).first.portstr,portstr) == 0)
        {
            //IDの設定
            (*i).first.id = ID + 1;
            break;
        }

        //IDの番号増やす
        ID++;
    }


    std::cout << ID + 1;

    //送る相手
    auto toAddr = *(ClientInfo.begin() + ID);

    //バイトオーダー
    ID = htonl(ID) + 1;

    //送信
    sendto(sock, (char*)&ID, sizeof(ID), 0, (SOCKADDR*)&toAddr.second, sizeof(toAddr.second));

    return TRUE;
}