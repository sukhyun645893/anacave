#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <random>

//UUID 라이크러리
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>


#pragma comment(lib, "ws2_32.lib")

using namespace std;


//서사 타입
struct Narrativepacket {
    char uniqueid[37];
    char state[64];
    char title[64];
    char connectkey[64];
    char narrative[512];
    char parentchannel[64];
    char chapter[64];
    char username[64];
    char userid[64];
    char userpassword[64];
};




//댓글 타입
struct commentpacket {
    char narrativeuniqueid[37];
    char state[64]; // comment
    char commentid[64];
    char commentstring[512];
    char parentchannel[64];
    char chapter[64];
    char Narrativeconnectkey[64];
    char parentcommentuniquekey[64]; //부모의 댓글 아이디
    char commentuniquekey[37]; //댓글의 고유 아이디
    char username[64]; // 유저 닉네임
    char userid[64]; //유저 아이디 , 토큰
    char userpassword[64]; //유저 로그인시 패스워드까지
};



//유저 타입
struct User {
    string nickname;
    string id;
    string password;

    //로그인 상태
    bool islogin;
};



//타입 : 아나콘 하나
struct anacontype {
    string path;
};


//타입 : 아나콘 세트
struct anaconset {
    string name;
    vector<string> anaconpaths;
};


enum PacketType {
    FETCH_NARRATIVE = 0,
    SUBMIT_NARRATIVE = 1,
    FETCH_COMMENT = 2,
    SUBMIT_COMMENT = 3,
    USER_TOKEN = 4,
    USERCONNECt = 5,
    SUBMITANACON = 6
};



struct PacketHeader {
    int type;
};



//아나케이브 서버 구상 설계도
class anacaveserver {
    public:


    //유저 구조체
    User user;
    User userbox;


    //유저 관련
    struct userinform
    {


        //비로그인 유저 토큰 보내기
        void usertoken(SOCKET& clientsocket) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<int> dist(0, 100);


            boost::uuids::uuid id = boost::uuids::random_generator()();
            string uuid_str = boost::uuids::to_string(id);
            
            send(clientsocket, (char*)uuid_str.c_str(), sizeof(uuid_str), 0);
            
            cout << "uuid_str이 어떤식으로 나오는지" << uuid_str.c_str() << endl;
        };

};



    //서사 관련
    struct Narrative {

    //서사 저장
    void saveNarrative(Narrativepacket& packet) {
        ofstream file("narratives.data", ios::binary | ios::app);

        //랜덤 생성
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, 100);

        boost::uuids::uuid id = boost::uuids::random_generator()();
        string uuid_str = boost::uuids::to_string(id);

        //랜덤아이디 붙여넣기
        strncpy(packet.uniqueid, (char*)uuid_str.c_str(), sizeof(packet.uniqueid)-1);
            

    if (file.is_open()) {
        file.write((const char*)&packet, sizeof(packet));
        cout << ">> 서사 저장 완료----------!!!!" << endl;
        cout << "narrative의 서사 본연의 키" << packet.uniqueid << endl;
        cout << "narrative를 쓴 자의 아이디" << packet.userid << endl;
        }
    };



    //서사 보내기
    void sendNarratives(SOCKET client) {

        ifstream file("narratives.data", ios::binary);

        if (!file.is_open()) return;

        Narrativepacket packet;

        while (file.read((char*)&packet, sizeof(packet))) {
            send(client, (char*)&packet, sizeof(packet), 0);
        }

        cout << ">> 서사 전송 완료" << endl;
    }



    //서사 전부 지우는 함수
void narrativeclear() {
    ofstream File("narratives.data", ios::binary | ios::trunc);
}

};


    //댓글 관련
    struct comment {


        //댓글 저장
        void savecomment(commentpacket& commentpacket) {
            ofstream outFile("comments.data", ios::binary | ios::app);

            cout << "comment nick name은 어떻게 오는지" << commentpacket.username << endl;

            if(outFile.is_open()) {
                outFile.write((const char*)&commentpacket, sizeof(commentpacket));
                cout << "댓글 저장 완료" << endl;
            }
        }

        //댓글 보내기
        void snedcomment(SOCKET& clientsocket) {
            ifstream File("comments.data", ios::binary);

            if(File.is_open()) {
                commentpacket commentpacket;

                while (File.read((char*)&commentpacket, sizeof(commentpacket)))
                {
                     send(clientsocket, (char*)&commentpacket, sizeof(commentpacket), 0);
                     cout << "댓글 전송 완료" << endl;
                }
            }
        }

        //댓글 전부 지우는 함수
        void commentclear() {
            ofstream inFile("comments.data", ios::binary | ios::trunc);
        };
        };




userinform userinform;
Narrative Narrative;
comment Comment;
};









//메인
int main() {
    

    anacaveserver anacaveserver;

    anacaveserver.Narrative.narrativeclear();
    anacaveserver.Comment.commentclear();


    system("chcp 65001");

    WSAData wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    //서버 창구 개설
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //주소 개설
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(serverSocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "bind 실패" << endl;
        return 1;
    }

    listen(serverSocket, SOMAXCONN);


    cout << ">> 서버가 8080 포트에서 작동중" << endl;



    while (true) {

        sockaddr_in clientaddr;
        int clientsize = sizeof(clientaddr);


        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientaddr, &clientsize);


        if (clientSocket == INVALID_SOCKET) continue;

        PacketHeader header;

        int recvBytes = recv(clientSocket, (char*)&header, sizeof(header), 0);


        if (recvBytes <= 0) {
            closesocket(clientSocket);
            continue;
        }


        switch (header.type) {


        case FETCH_NARRATIVE:
            anacaveserver.Narrative.sendNarratives(clientSocket);
            break;

            //서사 저장
        case SUBMIT_NARRATIVE: {

            Narrativepacket packet;

            recv(clientSocket, (char*)&packet, sizeof(packet), 0); 

            anacaveserver.Narrative.saveNarrative(packet);
            break;
        }



        case FETCH_COMMENT:
            anacaveserver.Comment.snedcomment(clientSocket);
            break;



        case SUBMIT_COMMENT: {

            commentpacket packet;

            recv(clientSocket, (char*)&packet, sizeof(packet), 0);

            cout << "packet comment가 어떤식으로 오는지" << packet.commentstring << endl;
            cout << "parentchannel이 잘오는지" << packet.parentchannel << endl;


            anacaveserver.Comment.savecomment(packet);
            break;
        };


        //처음 들어올시 토큰 발행
        case USER_TOKEN: {
            anacaveserver.userinform.usertoken(clientSocket);
            break;
        };


        default:
            cout << "알 수 없는 패킷 타입" << endl;
            break;
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
