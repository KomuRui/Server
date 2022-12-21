// ���������M���ĕ\������T�[�o

#include <iostream>		// ���o�͗p
#include <WinSock2.h>	// WinSock�p
#include <ws2tcpip.h>	// WinSock�p
#include <vector>
#include <string.h>

#pragma comment( lib, "ws2_32.lib" )

const unsigned short PORT = 8080;	//�T�[�o�̃|�[�g�ԍ�

//�ʒu
struct Position
{
    float x;
    float y;
    float z;
};

//������
struct SendInfo
{
    Position pos;
    float axisAngle;
    bool isShot;
    bool isDead;
};

//���(�A�h���X�A�|�[�g)
struct Info
{
    char ipAddr[256];  //�A�h���X�ۑ�
    char portstr[256]; //�|�[�g�ۑ�
    int  id;           //ID�ԍ�
};

//�N���C�A���g�̏�� ������ : �N���C�A���g�̃\�P�b�g�A�h���X
std::vector<std::pair<Info,SOCKADDR_IN>> ClientInfo;

//�v���C�l��
int PlayerMaxNumber;

//�O���錾
BOOL Receiving(SOCKET sock);
BOOL PlaySending(SOCKET sock,SendInfo data, char ipAddr[], char portstr[]);
BOOL LobySending(SOCKET sock,char ipAddr[], char portstr[]);



int main()
{
    std::cout << "Server\n";

    // WinSock�̏�����
    WSADATA  wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // �G���[����
    }

    // UDP�\�P�b�g�̍쐬	socket()
    SOCKET  sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        // �G���[
    }

    // �|�[�g�ԍ����蓖��
    SOCKADDR_IN  bindAddr;
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(PORT);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (SOCKADDR*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
    {
        // �G���[����
    }


    while (true)
    {
        //��M
        Receiving(sock);
    }

    // UDP�\�P�b�g�j��		closesocket()
    if (closesocket(sock) == SOCKET_ERROR)
    {
        // �G���[
    }

    // WinSock�I��
    if (WSACleanup() != 0)
    {
        // �G���[����
    }

    return 0;
}

//��M
BOOL Receiving(SOCKET sock)
{
    // �N���C�A���g�̃\�P�b�g�A�h���X
    SOCKADDR_IN fromAddr;
    int fromlen = sizeof(fromAddr);

    // �\���̎�M�����i�[�̈�
    SendInfo info;	

    int ret = recvfrom(sock, (char*)&info, sizeof(info), 0, (SOCKADDR*)&fromAddr, &fromlen);
    if (ret == SOCKET_ERROR)
    {
        // �G���[����
    }

    char ipAddr[256];  //�A�h���X
    char portstr[256]; //�|�[�g

    //��M�f�[�^������΂����ł��̎��̏���������
    inet_ntop(AF_INET, &fromAddr.sin_addr, ipAddr, sizeof(ipAddr));
    sprintf_s(portstr, "%d", ntohs(fromAddr.sin_port));

    //�ǉ����邩�ǂ���
    bool IsAdd = false;

    //�ۑ����Ă���N���C�A���g�̏��Ƃ��Ԃ��Ă��Ȃ���Ώ��ۑ�
    for (auto i = ClientInfo.begin(); i != ClientInfo.end(); i++)
    {
        //�����A�h���X�ƃ|�[�g�ԍ����Ⴄ�̂Ȃ�
        if ((*i).first.ipAddr != ipAddr && (*i).first.portstr != portstr)
        {
            IsAdd = true;
        }
    }

    //�����x�N�^�[�̒��g����Ȃ��
    if(ClientInfo.empty())
        IsAdd = true;

    //�����ǉ�����̂Ȃ�
    if (IsAdd)
    {
        //�x�N�^�[�ɕۑ�����p�̕ϐ���p��
        std::pair<Info, SOCKADDR_IN> a;

        strcpy_s(a.first.ipAddr,ipAddr);      //�A�h���X�ݒ�  
        strcpy_s(a.first.portstr,portstr);    //�|�[�g�ݒ�
        a.second = fromAddr;                //�N���C�A���g�̃\�P�b�g�A�h���X�ݒ�

        //vector�ݒ�
        ClientInfo.push_back(a);        

        //���M
        LobySending(sock,ipAddr, portstr);
    }
    //�������łɒǉ�����Ă���̂Ȃ�
    else
    {
        //�ϊ������\���̂�����悤�̕ϐ�
        SendInfo data;

        //���ۂɎg�����߂Ƀo�C�g�I�[�_�[��ϊ�
        data.pos.x = ntohl(info.pos.x);
        data.pos.y = ntohl(info.pos.y);
        data.pos.z = ntohl(info.pos.z);
        data.axisAngle = ntohl(info.axisAngle);
        data.isDead = ntohl(info.isDead);
        data.isShot = ntohl(info.isShot);

        //���̐l�Ɍ�����
        PlaySending(sock, data, ipAddr, portstr);
    }

    return TRUE;
}

//Play���Ă��鎞�̑��M
BOOL PlaySending(SOCKET sock,SendInfo data, char ipAddr[], char portstr[])
{
    for (auto i = ClientInfo.begin(); i != ClientInfo.end(); i++)
    {
        //�����A�h���X�ƃ|�[�g�ԍ����Ⴄ�̂Ȃ�
        if (strcmp((*i).first.ipAddr, ipAddr) != 0 && strcmp((*i).first.portstr, portstr) != 0)
        {
            //�ϊ������\���̂�����悤�̕ϐ�
            SendInfo sendData;

            //�o�C�g�I�[�_�[�ϊ�
            sendData.pos.x = htonl(data.pos.x);
            sendData.pos.y = htonl(data.pos.y);
            sendData.pos.z = htonl(data.pos.z);
            sendData.axisAngle = htonl(data.axisAngle);
            sendData.isDead = htonl(data.isDead);
            sendData.isShot = htonl(data.isShot);

            //���M
            sendto(sock, (char*)&sendData, sizeof(sendData), 0, (SOCKADDR*)&(*i).second, sizeof((*i).second));
        }
    }

    return TRUE;
}

//���r�[���̑��M
BOOL LobySending(SOCKET sock,char ipAddr[], char portstr[])
{
    //����ID
    int ID = 0;

    for (auto i = ClientInfo.begin(); i != ClientInfo.end(); i++)
    {
        //�����A�h���X�ƃ|�[�g�ԍ����ꏏ�Ȃ�
        if (strcmp((*i).first.ipAddr,ipAddr) == 0 && strcmp((*i).first.portstr,portstr) == 0)
        {
            //ID�̐ݒ�
            (*i).first.id = ID + 1;
            break;
        }

        //ID�̔ԍ����₷
        ID++;
    }


    std::cout << ID + 1;

    //���鑊��
    auto toAddr = *(ClientInfo.begin() + ID);

    //�o�C�g�I�[�_�[
    ID = htonl(ID) + 1;

    //���M
    sendto(sock, (char*)&ID, sizeof(ID), 0, (SOCKADDR*)&toAddr.second, sizeof(toAddr.second));

    return TRUE;
}