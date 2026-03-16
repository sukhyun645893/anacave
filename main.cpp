#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#define NOGDI             // Windows�쓽 Rectangle �븿�닔 李⑤떒
#define NOUSER            // Windows�쓽 CloseWindow 李⑤떒

#include <winsock2.h>
#include <windows.h>
#include "raylib.h"



typedef Rectangle Rect; 

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>

//UUID 라이크러리
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>



using namespace std;



//채널 타입
struct channel {
    string name;
    vector<const char*> scripts;
    Rect rectangle; // �씠�젣 而댄뙆�씪�윭媛� �씠 Rect瑜� �젙�솗�엳 �씤�떇�빀�땲�떎.
};



//챕터 타입
struct chapter {
    string name;
    vector<string> script;
};


//서사 추천 타입
struct recommend {
    string channelname;
    string chaptername;
    string Narrativeconnectkey;
    string narrativeuniqueid;
    int disrecommend;
};




//서사 비추천 타입
struct disrecommend {
    string channelname;
    string chaptername;
    string Narrativeconnectkey;
    string narrativeuniqueid;
    int recommed;
};



//서사 패킷 타입
struct Narrativepacket
{
    char uniqueid[37];
    char state[64]; // narrative;
    char title[64];
    char connectkey[64];
    char Narrative[512];
    char parentchannel[64];
    char chaptername[64];
    char username[64];
    char userid[64];
    char userpassword[64];
};




//댓글 패킷 타입
struct commentpacket
{
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





//헤더
enum PacketType {
    FETCH_NARRATIVE = 0,
    SUBMIT_NARRATIVE = 1,
    FETCH_COMMENT = 2,
    SUBMIT_COMMENT = 3,
    USER_TOKEN = 4,
    USERCONNECT = 5,
    SUBMITANACON = 6
};





//헤더 타입
struct PacketHeader {
    int type;
};



//타입 : 유저
struct User {
    char nickname[37];
    char id[64];
    char password[37];
};




//아나케이브 구상 설계도
class anacave {
    public:  
    enum struct anacavestate {
        MAINPAGE,
        CHANNELPAGE,
        CHAPTERPAGE,
        NARRATIVEPAGE,
        REWRITENARRATIVEPAGE,
        WRITEPAGE,
        SETTINGPAGE,
        ANACONPAGE
    };
    anacavestate current_state;
    
    //유저
    User user;
    User userbox;

    //추천 비추천
    recommend Recommend;
    disrecommend Disrecommend;


    string username;
    string userid;
    string userpassword;




    //메인 페이지(구상)
    struct mainpage {

        bool fetchtokenon = true;


        ////비 로그인시 토큰 가져오는 함수(유저 정보 만들기) //쓰기 모드는 app 붙이기
        void checkuserdata(User& userbox, User& user, bool& fetchtokenon, string& username, string& userid, string& userpassword) {
            
            ifstream File("user.data", ios::binary);

            //유저 정보가 없으면
            if(!File.is_open() || File.peek() == EOF) {
            WSAData wsadata;
            WSAStartup(MAKEWORD(2,2), &wsadata);

            //창구 개설
            SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, 0);

            //주소 개설
            sockaddr_in serveraddr;
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_port = htons(8080);
            serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");


            //연결
            if(connect(clientsocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) == 0) {
               
                PacketHeader header;
                header.type = USER_TOKEN;

                //이것은 유저토큰을 가져오는 
                send(clientsocket, (char*)&header, sizeof(header), 0);

                int bytes;

                char token[37];

                //byte가 0이상... 즉 서버가 응답하면...
                recv(clientsocket, (char*)&token, sizeof(token), 0);

                    strncpy(user.nickname, "anony", sizeof(user.nickname)-1);
                    strncpy(user.id, token, sizeof(user.id)-1);
                    strncpy(user.password, "", sizeof(user.password));


            //유저 정보 저장
            ofstream File("user.data", ios::binary);

            if(File.is_open()) {
                File.write((char*)&user, sizeof(user));
                cout << "저장 성공" << endl;
            }
    

            fetchtokenon = false;

            }

            closesocket(clientsocket);
            WSACleanup();
        } else {

            File.read((char*)&userbox, sizeof(userbox));
    
            username = userbox.nickname;
            userid = userbox.id;
            userpassword = userbox.password;

            cout << "how userbox username" << user.nickname << endl;
            cout << "how userbox userid" << user.id << endl;
            
        } 
     }


    

        //유저 정보 지우기
        void deleteuserdata(User& user) {
            ifstream file("user.data", ios::app | ios::trunc);
        };




        //설정창으로 들어가는 함수
        void Drawgosettingpage(anacavestate& current_state) {
            Rectangle box = {400, 0, 50, 50};
            DrawRectangleLinesEx(box, 3, WHITE);

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), box)) {
            current_state = anacave::anacavestate::SETTINGPAGE;
        }
    }
 };
    



    //채널
    struct channelpage {

        //채널 페이지 나가기
        void exitchannel(anacavestate& current_state) {
            Rectangle box = {5, 5, 50, 20};
            DrawRectangleLinesEx(box, 3, WHITE);
            DrawText("EXIT", 5, 5, 20, WHITE);

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), box)) {
            current_state = anacavestate::MAINPAGE;
        }
        }
    };



    //챕터
    struct chapterpage {
        vector<Narrativepacket> narrativesvector;

        
        //서사 가져오는 함수
        void fetchNarrative(string& channelname, string chaptername) {
            cout << "fetchNarrative함수 발동" << endl;
            WSAData wsadata;
            WSAStartup(MAKEWORD(2,2), &wsadata);

            SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, 0);

            //주소 개설
            sockaddr_in serveraddr;
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_port = htons(8080);
            serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

            if(connect(clientsocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) == 0) {

                //서사 가져오기
                PacketHeader header;
                header.type = FETCH_NARRATIVE;
                //헤더 먼저 보내주고
                send(clientsocket, (char*)&header, sizeof(header), 0);

                //받을 상자
                Narrativepacket narrativepacket;
                
                int bytes;

                while ((bytes = recv(clientsocket, (char*)&narrativepacket, sizeof(narrativepacket), 0)) > 0)
                {
                    cout << "수신 완료" << narrativepacket.title << endl;

                    //채널명과 챕터명이 같으면 벡터값 포함 <----- 이 부분에서 자꾸 에러가 터지네
                  
                    if(strcmp(channelname.c_str(), narrativepacket.parentchannel) == 0 &&
                    strcmp(chaptername.c_str(), narrativepacket.chaptername) == 0) {
                         narrativesvector.push_back(narrativepacket);
                    }
                
            }
        }
        closesocket(clientsocket);
        WSACleanup();
    };



    //가져온 게시글 그리기
    void Drawnarrative(float& scrollY, anacavestate& Current_state,
    string& channelname, string& chaptername, string& title, string& connectkey,
    string& narrative, string& uniqueid) {

        for(int i=0; i< narrativesvector.size(); i++) {
                   
            Rectangle box = {20, 180 + (i*100) + scrollY, 350, 80};
            DrawRectangleLinesEx(box, 3, WHITE);
            DrawText(narrativesvector[i].title, 25, 185+ (i*100)+ scrollY, 35, WHITE);


            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
            CheckCollisionPointRec(GetMousePosition(), box)) {
                channelname = narrativesvector[i].parentchannel;
                chaptername = narrativesvector[i].chaptername;
                title = narrativesvector[i].title;
                connectkey = narrativesvector[i].connectkey;
                narrative = narrativesvector[i].Narrative;
                uniqueid = narrativesvector[i].uniqueid;

                Current_state = anacavestate::NARRATIVEPAGE;
                }
        
    };

};


    //서사 지우는 함수
    void ereiser() {
        narrativesvector.clear();
    };



    //챕터 페이지 나가기
    void exitchapterpage(float& scrollY, anacavestate& current_state) {
        Rectangle box = {5, 5, 50, 25};
        DrawRectangleLinesEx(box, 3, WHITE);
        DrawText("EXIT", 6, 6, 20, WHITE);

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        CheckCollisionPointRec(GetMousePosition(), box)) {
            current_state = anacavestate::CHANNELPAGE;
        }
    }

};
    






    //서사 자세히 보기
    struct narrativepage {
        string parentchannel;
        string chaptername;
        string title;
        string connectkey;
        string Narrative; 

        //고유 서사 번호
        string uniqueid;

        //체크 연결키
        bool keyboxon = false;
        bool connectkeyon = false;

        //댓글 아이디
        bool commenton = false;
        bool commentidon = false;

    
        //댓글 구조체
        commentpacket Commentpacket;
        commentpacket Commentfetchbox;
        vector<commentpacket> commentpackets;

        //대댓글인지 아닌지...
        bool cocomment = false;
        bool fetchcommentonetime;

        //대댓글 이름
        string cocomentusername;

        //대댓글의 댓글 아이디(parentcommentid)
        string cocommentid;


        //제목
        void Drawtitle(float& scrollY) {
            DrawRectangleLines(0, 0+scrollY, 450, 50, WHITE);
            DrawText(title.c_str(), 80, 0+scrollY, 30, WHITE);
        };

        

        //서사
        void DrawNarrative(float& scrollY)  {
            DrawRectangleLines(5, 150 + scrollY, 440, 600, WHITE);
            DrawText(Narrative.c_str(), 10, 160+ scrollY, 30, WHITE);
        };
       


        //서사 연결 키
        void Drawconnectkey(float& scrollY, string& checkconnectstring, anacavestate& current_state) {
            Rectangle connectkeybox = {380, 90+scrollY, 60, 50};
            DrawText("Key", 385, 92+scrollY, 30, WHITE);

            //키박스 소환 // 배후성(서사자)명 작성
            if(CheckCollisionPointRec(GetMousePosition(), connectkeybox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                keyboxon = !keyboxon;
                connectkeyon = !connectkeyon;
            };

            //키박스 내부
            if(keyboxon&& connectkeyon) {
            Rectangle onconnectkeybox = {10, 100+scrollY, 300, 50};
            DrawRectangleLinesEx(onconnectkeybox, 3, WHITE);
    
                int key = GetCharPressed();

                //키값작성....이 안되는데...
                while (key>0) {
                    if((32<key) && (key <=125)) {
                        checkconnectstring += (char)key;
                    }
                        key = GetCharPressed();
                    };

                    DrawText(checkconnectstring.c_str(), 10, 115+scrollY, 30, WHITE);


                    if(IsKeyPressed(KEY_BACKSPACE)) {
                        checkconnectstring.pop_back(); 
                    };
                }

                if(IsKeyPressed(KEY_ENTER) && strcmp(checkconnectstring.c_str(), connectkey.c_str()) == 0) {
                    current_state = anacavestate::REWRITENARRATIVEPAGE;
                }
            };





            
            //댓글 쓰고 전송
            void Drawcomment(float& scrollY, bool& connectcommentkeyon, bool& commentwriteon,
            bool& commenton, string& commentid, string& commentstring, 
            string& channelname, string& chaptername, string& narrativeconnectkey, 
            User& user, commentpacket& commentpacket, bool& cocomment, string& cocommentusername, string& cocommentid, string& username, string& userid, string& userpassword) {

                //서사 참조& 박스
                Rectangle commentbox = {0, 740, 450, 60};
                DrawRectangleLinesEx(commentbox, 3, WHITE);
                //서사 참조&
                DrawText("comment", 20, 810+scrollY, 30, WHITE);

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), commentbox)) {
                    commenton = true;
                };

                if(commenton) {


                //댓글 취소버튼
                Rectangle closebox = {390, 680, 60, 60};
                DrawRectangleLinesEx(closebox, 3, WHITE);
                DrawLineV(Vector2{390, 680}, Vector2{450, 740}, WHITE);
                DrawLineV(Vector2{390, 740}, Vector2{450, 680}, WHITE);

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), closebox)) {
                    cocomment = false;
                    commenton = false;
                };


                Rectangle connectcommentkeybox =  {0, 680, 450, 60};
                Rectangle commentwritebox = {0, 740, 450, 60};

                DrawRectangleLinesEx(connectcommentkeybox, 3, WHITE);
                DrawRectangleLinesEx(commentwritebox, 3, GOLD);


                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    connectcommentkeyon = CheckCollisionPointRec(GetMousePosition(), connectcommentkeybox);
                    commentwriteon = CheckCollisionPointRec(GetMousePosition(), commentwritebox);
                }

                    
                //타이핑 함수
                int key = GetCharPressed();

                
                //타이핑
                while (key > 0) {
                    if((32 < key) && (key <= 125)) {
                        if(connectcommentkeyon)
                        commentid += (char)key;
                        if(commentwriteon)
                        commentstring += (char)key;
                    }
                    key = GetCharPressed();
                };
            


                if(IsKeyPressed(KEY_BACKSPACE)) {
                    if(commentidon, !commentid.empty())
                    commentid.pop_back();

                    if(commentwriteon, !commentstring.empty())
                    commentstring.pop_back();
                };


                if(IsKeyPressed(KEY_SPACE)) {
                    if(commentidon)
                    commentid += " ";
                    
                    if(commentwriteon)
                    commentstring += " ";
                };


        
                DrawText(commentid.c_str(), 20, 700, 30, WHITE);
                DrawText(commentstring.c_str(), 20, 750, 30, WHITE);
                DrawText(username.c_str(), 30, 30, 50, WHITE);


                       
                
                //대댓글일때
                if(cocomment == true) {
                    DrawText(cocommentusername.c_str(), 5, 750, 30, WHITE);
                }



                //엔터 누를시 댓글 전송
                if(IsKeyPressed(KEY_ENTER)) {
                    SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, 0);

                    sockaddr_in serveraddr;
                    serveraddr.sin_family = AF_INET;
                    serveraddr.sin_port = htons(8080);
                    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

                    if(connect(clientsocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) == 0) {

                        PacketHeader header;
                        header.type = SUBMIT_COMMENT;

                        //이것은 댓글 전송입니다
                        send(clientsocket, (char*)&header, sizeof(header), 0);

                        commentpacket = {0};


                        //대댓글일때
                        if(cocomment == true) { 
                            
                        
                        strncpy(commentpacket.commentstring, commentstring.c_str(), sizeof(commentpacket.commentstring)-1);
                        strncpy(commentpacket.commentid, commentid.c_str(), sizeof(commentpacket.commentid)-1);
                        strncpy(commentpacket.parentchannel, channelname.c_str(), sizeof(commentpacket.parentchannel)-1);
                        strncpy(commentpacket.chapter, chaptername.c_str(), sizeof(commentpacket.chapter)-1);
                        strncpy(commentpacket.Narrativeconnectkey, narrativeconnectkey.c_str(), sizeof(commentpacket.Narrativeconnectkey)-1);
                        strncpy(commentpacket.narrativeuniqueid, uniqueid.c_str(), sizeof(commentpacket.narrativeuniqueid)-1);
                        
                        //부모의 댓글
                        strncpy(commentpacket.parentcommentuniquekey, cocommentid.c_str() , sizeof(commentpacket.parentcommentuniquekey)-1);
                        strncpy(commentpacket.commentuniquekey, commentpacket.commentuniquekey, sizeof(commentpacket.commentuniquekey)-1);
                        
                        strncpy(commentpacket.username, username.c_str(), sizeof(commentpacket.username)-1);
                        strncpy(commentpacket.userid, userid.c_str(), sizeof(commentpacket.userid)-1);
                        strncpy(commentpacket.userpassword, userpassword.c_str(), sizeof(commentpacket.userpassword)-1);

                        
                        send(clientsocket, (char*)&commentpacket, sizeof(commentpacket), 0);

                        cout << "댓글 전송 완료" << endl;
                        DrawText("CLEAR",225, 400, 50, WHITE);
                        commentid.clear();
                        commentstring.clear();

                    
                    }

                    //그냥 댓글일때
                    else if(cocomment == false) {

                    
                        random_device rd;
                        mt19937 gen(rd());
                        uniform_int_distribution<int> dist(0,100);

                        boost::uuids::uuid id = boost::uuids::random_generator()();
                        string uuid_str = boost::uuids::to_string(id);

        
                        strncpy(commentpacket.commentstring, commentstring.c_str(), sizeof(commentpacket.commentstring)-1);
                        strncpy(commentpacket.commentid, commentid.c_str(), sizeof(commentpacket.commentid)-1);
                        strncpy(commentpacket.parentchannel, channelname.c_str(), sizeof(commentpacket.parentchannel)-1);
                        strncpy(commentpacket.chapter, chaptername.c_str(), sizeof(commentpacket.chapter)-1);
                        strncpy(commentpacket.Narrativeconnectkey, narrativeconnectkey.c_str(), sizeof(commentpacket.Narrativeconnectkey)-1);
                        strncpy(commentpacket.narrativeuniqueid, uniqueid.c_str(), sizeof(commentpacket.narrativeuniqueid)-1);
                        

                        strncpy(commentpacket.parentcommentuniquekey, "", sizeof(commentpacket.parentcommentuniquekey)-1);
                    
                        strncpy(commentpacket.commentuniquekey, uuid_str.c_str(), sizeof(commentpacket.commentuniquekey)-1);
                        strncpy(commentpacket.username, username.c_str(), sizeof(commentpacket.username)-1);
                        strncpy(commentpacket.userid, userid.c_str(), sizeof(commentpacket.userid)-1);
                        strncpy(commentpacket.userpassword, userpassword.c_str(), sizeof(commentpacket.userpassword)-1);
                       
                    
                        send(clientsocket, (char*)&commentpacket, sizeof(commentpacket), 0);

                        cout << "댓글 전송 완료" << endl;
                        DrawText("CLEAR", 225, 400, 50, WHITE);
                        commentid.clear();
                        commentstring.clear();
                    }
                }
                    else {
                        cout << "서버 연결 실패" << endl;
                    }

                    closesocket(clientsocket);
                }
            }; 


                Vector2 Circlecenter = Vector2{425, 775};
                float CircleRadius = 25.0f;

                DrawLineV(Vector2{420, 750}, Vector2{440, 780}, WHITE);
                DrawLineV(Vector2{420, 800}, Vector2{440, 780}, WHITE);

                DrawCircleLinesV(Circlecenter, CircleRadius, WHITE);
        };



            //댓글 가져오기
            void Drawfetchcomment(vector<commentpacket>& commentpackets, float& scrollY, commentpacket& Commentfetchbox,
            bool& cocoment) {
                WSAData wsadata;
                WSAStartup(MAKEWORD(2,2), &wsadata);

                SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, 0);

                //주소 개설
                sockaddr_in serveraddr;
                serveraddr.sin_family = AF_INET;
                serveraddr.sin_port = htons(8080);
                serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");


                //주소랑 연결
                if(connect(clientsocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) == 0) {

                    PacketHeader header;
                    header.type = FETCH_COMMENT;

                    send(clientsocket, (char*)&header, sizeof(header), 0);

                        int bytes;
                        
                        //바이트가 0이상인 동안.... 계속
                        while((bytes = recv(clientsocket, (char*)&Commentfetchbox, sizeof(Commentfetchbox), 0)) > 0)  {

                            cout << "how username" <<  Commentfetchbox.username << endl;

                            //서사 고유 번호와 겹치는 애들만
                            if(strcmp(uniqueid.c_str(), Commentfetchbox.narrativeuniqueid) == 0) {
                                 commentpackets.push_back(Commentfetchbox);
                            }
                    }
                }
            };



            //댓글 그리기
            void Drawcommentbox(vector<commentpacket>& commentpackets, float& scrollY, User& user, 
            commentpacket& commentpacket, bool& cocomment, string& cocommentusername, string& parentcommentid) {

            float yOffset = 860 + scrollY;


            for(int i = 0; i < commentpackets.size(); i++)
            {
                // 부모 댓글만 먼저 그린다
                if(strlen(commentpackets[i].parentcommentuniquekey) == 0)
                {
                    Rectangle box = {10, yOffset, 400, 110};

                    DrawRectangleLinesEx(box, 3, WHITE);

                    DrawText(commentpackets[i].username, 15, yOffset + 5, 30, WHITE);
                    DrawText(commentpackets[i].commentstring, 15, yOffset + 35, 30, WHITE);

                    // 대댓글 클릭
                    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                    CheckCollisionPointRec(GetMousePosition(), box))
                    {
                        parentcommentid = commentpackets[i].commentuniquekey;
                        cocomment = true;
                        cocommentusername = commentpackets[i].username;

                        cout << "reply to: " << cocommentusername << endl;
                    }

                    yOffset += 120;

                    // 대댓글 찾기
                    for(int j = 0; j < commentpackets.size(); j++)
                    {
                        if(strcmp(commentpackets[i].commentuniquekey,
                                commentpackets[j].parentcommentuniquekey) == 0)
                        {
                            Rectangle replybox = {30, yOffset, 380, 90};

                            DrawRectangleLinesEx(replybox, 2, GRAY);

                            DrawText(commentpackets[j].username, 40, yOffset + 5, 25, WHITE);
                            DrawText(commentpackets[j].commentstring, 40, yOffset + 30, 25, WHITE);

                            yOffset += 100;
                        }
                    }
                }
            }
            };


            //댓글 벡터 지우는 함수
            void eresiercommen() {
                commentpackets.clear();
            };

        


        //서사 자세히 보기 나가기
        void DrawExit(float& scrollY, anacavestate& Current_state) {
                Rectangle Exitbox = {0, 0+scrollY, 60, 50};
                DrawRectangleLinesEx(Exitbox, 3, WHITE);
                DrawText("EXIT",2 , 2+ scrollY, 20, WHITE);


            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(GetMousePosition(), Exitbox)) {
                Current_state = anacavestate::CHAPTERPAGE;
            }
        };
    };







    //서사 쓰기
    struct writepage {

        

    //서사 보내는 함수
    void submitNarrative(string& parentchannel, string& chaptername, string& title,
    string& narrativeconnectkey, string& narrative, User& user) {


        WSAData wsadata;
        if(WSAStartup(MAKEWORD(2,2), &wsadata) !=0) return;

        //창구 개설
        SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, 0);

        //주소 만들기
        sockaddr_in serveraddr;
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(8080);
        serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        //연결
        if(connect(clientsocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) != SOCKET_ERROR) {
            
            PacketHeader header;
            header.type = SUBMIT_NARRATIVE;

            send(clientsocket, (char*)&header, sizeof(header), 0);

            Narrativepacket narrativepacket = { 0 };

            strncpy(narrativepacket.parentchannel, parentchannel.c_str(), sizeof(narrativepacket.parentchannel)-1);
            strncpy(narrativepacket.chaptername, chaptername.c_str(), sizeof(narrativepacket.chaptername)-1);
            strncpy(narrativepacket.title, title.c_str(), sizeof(narrativepacket.title)-1);
            strncpy(narrativepacket.connectkey, narrativeconnectkey.c_str(), sizeof(narrativepacket.connectkey)-1);
            strncpy(narrativepacket.Narrative, narrative.c_str(), sizeof(narrativepacket.Narrative));
            strncpy(narrativepacket.username, user.nickname, sizeof(narrativepacket.username)-1);
            strncpy(narrativepacket.userid, user.id, sizeof(narrativepacket.userid)-1);
            strncpy(narrativepacket.userpassword, user.password, sizeof(narrativepacket.userpassword)-1);


            int result = send(clientsocket, (char*)&narrativepacket, sizeof(narrativepacket), 0);

            if(result != SOCKET_ERROR) {
            cout << "submitnarrative SUCCESS" << endl;
            } else {
                cout << "fail" << endl;
            }
        };
        closesocket(clientsocket);
        WSACleanup();
    };
};
    






    //설정
    struct settingpage {

        
        //로그인 
        void Drawlogin(float& scrollY, bool& nicknameon, bool& idon, 
        bool& passwordon, string& nicknamestring, string& idstring,
        string& passwordstring, User& userbox) {
            int key = GetCharPressed();


            //로그인 박스들
            Rectangle nicknamebox = {20, 100, 400, 50};
            Rectangle idbox = {20, 200, 400, 50};
            Rectangle passwordbox = {20, 300, 400, 50};


            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                nicknameon = CheckCollisionPointRec(GetMousePosition(), nicknamebox);
                idon = CheckCollisionPointRec(GetMousePosition(), idbox);
                passwordon = CheckCollisionPointRec(GetMousePosition(), passwordbox);
            }


            while (key > 0)
            {
                if((32 < key) && (key <= 125)) {
                    if(nicknameon)
                    nicknamestring += (char)key;
                    if(idon) 
                    idstring += (char)key;
                    if(passwordon)
                    passwordstring += (char)key;
                }
            }


            if(IsKeyPressed(KEY_BACKSPACE)) {
                if(nicknameon)
                nicknamestring.pop_back();
                if(idon)
                idstring.pop_back();
                if(passwordon)
                passwordstring.pop_back();
            }

            DrawRectangleLinesEx(nicknamebox, 3, WHITE);
            DrawRectangleLinesEx(idbox, 3, WHITE);
            DrawRectangleLinesEx(passwordbox, 3, WHITE);


            DrawText(nicknamestring.c_str(), 25, 105, 30, WHITE);
            DrawText(idstring.c_str(), 25, 205, 30, WHITE);
            DrawText(passwordstring.c_str(), 25, 305, 30, WHITE);



            //로그인 시도?
            if(IsKeyPressed(KEY_ENTER)) {
                WSAData wsadata;
                WSAStartup(MAKEWORD(2,2), &wsadata);

                //창구 개설
                SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, 0);

                //주소 개설
                sockaddr_in serveraddr;
                serveraddr.sin_family = AF_INET;
                serveraddr.sin_port = htons(8080);
                serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");


                //연결
                if(connect(clientsocket, (sockaddr*)&serveraddr, sizeof(serveraddr)) == 0) {
                    
                    PacketHeader header;
                    header.type = USERCONNECT;

                    //이것은 고닉로그인 입니다
                    send(clientsocket, (char*)&header, sizeof(header), 0);

                    strncpy(userbox.nickname, (char*)&nicknamestring, sizeof(userbox.nickname)-1);
                    strncpy(userbox.id, (char*)&idstring, sizeof(userbox.id)-1);
                    strncpy(userbox.password, (char*)&passwordstring, sizeof(passwordstring)-1);

                
                    //서버에 전송
                    send(clientsocket, (char*)&userbox, sizeof(userbox), 0);
                    
                }
            }
        }



        //배경선택
        bool selectImage;
        //제목
        void Drawtitle() {
            DrawRectangleLines(5, 5, 440, 100, WHITE);
            DrawText("SETTING", 10, 10, 80, WHITE);
        };

        //탭바
        void DrawTabbar() {
            struct tabbar {
                Rectangle tabbarractangle;
            };

            for(int i=0; i< 4; i++) {
                tabbar tabbar;
                
                tabbar.tabbarractangle = {5, 100 + (i*100), 200, 50};

                DrawRectangleLinesEx(tabbar.tabbarractangle, 3, WHITE);

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectImage = CheckCollisionPointRec(GetMousePosition(), tabbar.tabbarractangle);
                }
            }
        }


        //설정창 나가는 함수
        void Exitsettingpage(anacavestate& current_state) {
            Rectangle box = {5, 5, 50, 50};
            DrawRectangleLinesEx(box, 3, WHITE);

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), box)) {
            current_state = anacavestate::MAINPAGE;
        }
        }
    };

    mainpage mainpage;
    channelpage channelpage;
    narrativepage narrativepage;
    chapterpage chapterpage;
    writepage writepage;
    settingpage Settingpage;
    
};
        
















//메인
int main() {

    vector<channel> channelvector = {
        {"USA", {"USA", "anacave start"}, {100.0f, 220.0f, 250.0f, 40.0f}},
        {"korea", {"����븳誘쇨뎅", "anacave start"}, {100.0f, 300.0f, 250.0f, 40.0f}},
        {"japan", {"�뿥�쑍", "anacave start"}, {100.0f, 380.0f, 250.0f, 40.0f}}
    };

    //월드 베스트(모든게 한데 뒤섞인... 거대한 서사), 특이점이 온다(미래에 대한 기대와 설렘), UN정치(내가 가진정보와 반 딥스테이트 전선), "이 세상의 진실(내가 가진 정보들 공개)", "일상(평범하고 가치잇던 일상을 공유하는 갤)", "잊고 있었던것(과거의 온기에 대한 그리움)..."
    vector<chapter> chaptervector = {
        {"USA", {"World best", "singurlarity is commeing", "UNpolitical", "worldrealtruth", "losted past"}},
        {"korea", {"World best", "singurlarity is commeing", "UNpolitical", "worldrealtruth", "losted past"}},
        {"japan", {"World best", "singurlarity is commeing", "UNpolitical", "worldrealtruth", "losted past"}}
    };




    anacave anacave;
    anacave.current_state = anacave::anacavestate::MAINPAGE;


    string channelname;
    string chaptername;


    static string titlestring = "";
    static string connectstring = "";
    static string myNarrative = "";


    InitWindow(1200, 900, "ANACAVE - 추상적 추론 결과");
    SetTargetFPS(60);

    

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);



        if(IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    };



        //메인
        if (anacave.current_state == anacave::anacavestate::MAINPAGE) {
            Rectangle box = {5, 5, 440, 100};
            DrawRectangleLinesEx(box, 3, WHITE);

            DrawText("ANACAVE", 40, 5, 50, WHITE);


            Rect yourRectangle = {50.0f, 200.0f, 350.0f, 570.0f};
            DrawRectangleLinesEx(yourRectangle, 3.0f, WHITE);

            //유저 토큰 가져오는 함수
            if(anacave.mainpage.fetchtokenon == true) {
                anacave.mainpage.checkuserdata(anacave.userbox, anacave.userbox, anacave.mainpage.fetchtokenon, anacave.username, anacave.userid, anacave.userpassword);
                cout << "어떤식으로 토큰이 올까?" << anacave.user.id << endl;
            }
        

            for(int i=0; i<channelvector.size(); i++) {
                Rectangle channelbox = {channelvector[i].rectangle.x, channelvector[i].rectangle.y, 
                channelvector[i].rectangle.width, channelvector[i].rectangle.height};

                DrawRectangleLinesEx(channelbox, 3, WHITE);  

                DrawText(channelvector[i].name.c_str(), 
                channelvector[i].rectangle.x, 
                channelvector[i].rectangle.y, 
                30, 
                WHITE
            );

            //설정 페이지로 이동하는 함수
            anacave.mainpage.Drawgosettingpage(anacave.current_state);
    

            float time = GetFrameTime();

            if(IsKeyPressed(KEY_R)) {
                cout << "TIME" << time << endl;
            };
            

            if(CheckCollisionPointRec(GetMousePosition(), channelbox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                channelname = channelvector[i].name;
                anacave.current_state = anacave::anacavestate::CHANNELPAGE;
            }
        }
    }
    
    
    
    
    
    
    //채널
    else if(anacave.current_state == anacave::anacavestate::CHANNELPAGE) {
            vector<string> script;
            

            for(int i=0; i<chaptervector.size(); i++) {
                if(channelname == chaptervector[i].name) {
                    script = chaptervector[i].script;
                };
            };

        
            DrawRectangleLines(5, 5, 440, 90, WHITE);
            DrawText(channelname.c_str(), 50, 5, 50, WHITE);

            //박스들 감싸는 곳
            DrawRectangleLines(5, 130, 420, 400, WHITE);


            Rectangle box1 = {10, 140, 400, 50};
            Rectangle box2 = {10, 220, 400, 50};
            Rectangle box3 = {10, 300, 400, 50};
            Rectangle box4 = {10, 380, 400, 50};
            Rectangle box5 = {10, 460, 400, 50};


            DrawRectangleLinesEx(box1, 3, WHITE);
            DrawRectangleLinesEx(box2, 3, WHITE);
            DrawRectangleLinesEx(box3, 3, WHITE);
            DrawRectangleLinesEx(box4, 3, WHITE);
            DrawRectangleLinesEx(box5, 3, WHITE);


            DrawText(script[0].c_str(), 10, 140, 30, WHITE);
            DrawText(script[1].c_str(), 10, 220, 30, WHITE);
            DrawText(script[2].c_str(), 10, 300, 30, WHITE);
            DrawText(script[3].c_str(), 10, 380, 30, WHITE);
            DrawText(script[4].c_str(), 10, 460, 30, WHITE);


            float time = GetFrameTime();
            time ++;

            
            if(CheckCollisionPointRec(GetMousePosition(), box1) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                chaptername = script[0];
                if(time >= 1.0f)
                anacave.current_state = anacave::anacavestate::CHAPTERPAGE;
            };

            if(CheckCollisionPointRec(GetMousePosition(), box2) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                chaptername = script[1];
                if(time >= 1.0f)
                anacave.current_state = anacave::anacavestate::CHAPTERPAGE;
            };
            if(CheckCollisionPointRec(GetMousePosition(), box3) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    chaptername = script[2];
                    if(time >= 1.0f)
                    anacave.current_state = anacave::anacavestate::CHAPTERPAGE;
            };
             if(CheckCollisionPointRec(GetMousePosition(), box4) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for(int i=0; i<5; i++) {
                    if(i == 4) {
                    chaptername = script[3];
                    anacave.current_state = anacave::anacavestate::CHAPTERPAGE;
                    }
                }
            };
             if(CheckCollisionPointRec(GetMousePosition(), box4) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for(int i=0; i<5; i++) {
                    if(i == 4) {
                    chaptername = script[4];
                    anacave.current_state = anacave::anacavestate::CHAPTERPAGE;
                    }
                }
            };
            anacave.channelpage.exitchannel(anacave.current_state);

        }
        





        //챕터
        else if (anacave.current_state == anacave::anacavestate::CHAPTERPAGE) {
            
            float wheel = GetMouseWheelMove();
            static float scrollY = 0.0f;

            scrollY += wheel*10.0f;

            if(scrollY >= 50) {
                cout << "over20" << endl;
                DrawCircleV(Vector2{225.0f, 400.0f}, 5.0f, WHITE);
                scrollY = 0;
            };

            //서사 가져오고
            anacave.chapterpage.fetchNarrative(channelname, chaptername);


            //가져온 서사 그려주는 함수
            anacave.chapterpage.Drawnarrative(scrollY,
            anacave.current_state, anacave.narrativepage.parentchannel, 
            anacave.narrativepage.chaptername, anacave.narrativepage.title, anacave.narrativepage.connectkey, 
            anacave.narrativepage.Narrative, anacave.narrativepage.uniqueid); 

            //그리고 그 서사들 다 지우고... 반복
            anacave.chapterpage.ereiser();
        

            Rectangle appbar = {5, 5 + scrollY, 440, 90};
            DrawRectangleLinesEx(appbar, 3, WHITE);

            anacave.chapterpage.exitchapterpage(scrollY, anacave.current_state); 

            DrawText(chaptername.c_str(), 15, 10 + scrollY, 40, WHITE);
            Rectangle writerbox = {350, 110, 90, 60};
            DrawRectangleLinesEx(writerbox, 3, WHITE);
            DrawText("Write", 360, 125, 20, WHITE); 

            
            if(CheckCollisionPointRec(GetMousePosition(), writerbox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                anacave.current_state = anacave::anacavestate::WRITEPAGE;
            }
        }



        

        //서사 자세히 보기
        else if(anacave.current_state == anacave::anacavestate::NARRATIVEPAGE) {


            //서사 자세히 보기 체크 커넥트 키
            static string checkconnectstring = "";

            bool commenton;

            //댓글 연결키, 댓글
            bool connectcommentkeyon; //commentid
            bool commentwriteon; //commentstring

            //댓글 보내기 on
            bool submitcommenton;

            //댓글(서사참조)
            static string commentstring = "";

            //댓글 아이디
            static string commentid = "";


            float wheel = GetMouseWheelMove();
            static float scrollY = 0.0f;
            
            scrollY += wheel*10.0f;


             if(scrollY > 50) {

                
                cout << "over 50" << endl;

                scrollY = 0;
            };



            //댓글 가져오는 함수
            anacave.narrativepage.Drawfetchcomment(anacave.narrativepage.commentpackets, scrollY, anacave.narrativepage.Commentfetchbox, 
            anacave.narrativepage.cocomment);

            //댓글 그리기
            anacave.narrativepage.Drawcommentbox(anacave.narrativepage.commentpackets, scrollY, anacave.user, 
            anacave.narrativepage.Commentpacket, anacave.narrativepage.cocomment, 
            anacave.narrativepage.cocomentusername, anacave.narrativepage.cocommentid);

            //서사 벡터값 지워주는 함수
            anacave.narrativepage.eresiercommen();
            


            //제목 그려주는 함수
            anacave.narrativepage.Drawtitle(scrollY);

            //연결 키 그려주는 함수
            anacave.narrativepage.Drawconnectkey(scrollY, checkconnectstring, anacave.current_state);
            
            //서사 그려주는 함수
            anacave.narrativepage.DrawNarrative(scrollY);

            //댓글 쓰고 전송 함수
            anacave.narrativepage.Drawcomment(scrollY, connectcommentkeyon, commentwriteon, commenton, commentid, commentstring,
            channelname, chaptername, anacave.narrativepage.connectkey, anacave.user, anacave.narrativepage.Commentpacket, 
            anacave.narrativepage.cocomment, anacave.narrativepage.cocomentusername, anacave.narrativepage.cocommentid, 
            anacave.username, anacave.userid, anacave.userpassword);

            //서사 자세히 보기 나가기
            anacave.narrativepage.DrawExit(scrollY, anacave.current_state);
        }





        //서사 다시쓰기(수정)
        else if(anacave.current_state == anacave::anacavestate::REWRITENARRATIVEPAGE) {
            float wheel = GetMouseWheelMove();

            float scrollY = 0.0f;

            scrollY += wheel*10.0f;
            //제목
            Rectangle titlebox = {5, 5, 440, 90};
            DrawRectangleLinesEx(titlebox, 3, WHITE);
            DrawText(anacave.narrativepage.Narrative.c_str(), 10, 10, 30, WHITE);
            //서사
            Rectangle Narrativebox = {5, 120, 440, 500};
            DrawRectangleLinesEx(Narrativebox, 3, WHITE);
            DrawText(anacave.narrativepage.Narrative.c_str(), 10, 130, 40, WHITE);

            bool title;
            bool Narrative;

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                title = CheckCollisionPointRec(GetMousePosition(), titlebox);
                Narrative = CheckCollisionPointRec(GetMousePosition(), Narrativebox);
            };


            int key = GetCharPressed();


            while (key > 0) {
                if((32<key) && (key < 125)) {
                    if(title)
                    anacave.narrativepage.title += (char)key;

                    if(Narrative)
                    anacave.narrativepage.Narrative += (char)key;
                }
                key = GetCharPressed();
            };

            DrawText(anacave.narrativepage.title.c_str(), 5, 15, 30, WHITE);
            DrawText(anacave.narrativepage.Narrative.c_str(), 5, 150, 30, WHITE);

            if(IsKeyPressed(KEY_BACKSPACE)) {
                if(title)
                anacave.narrativepage.title.pop_back();

                if(Narrative)
                anacave.narrativepage.Narrative.pop_back();
            }
        }



        


        
        //서사쓰기
        else if (anacave.current_state == anacave::anacavestate::WRITEPAGE) {
            Vector2 mouse = GetMousePosition();

            bool titlekey;
            bool connectkey;
            bool writekey;

            Rectangle titlebox = {5, 5, 440, 90};
            Rectangle connectbox = {30, 120, 390, 50};
            Rectangle writebox = {5, 180, 440, 400};
 

            if(IsKeyPressed(KEY_G)) {
            cout << "WHAT channel...?" << channelname.c_str() << endl;
            cout << "WHAT chapter...?" << chaptername .c_str() << endl;
            };


            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                titlekey = CheckCollisionPointRec(mouse, titlebox);
                connectkey = CheckCollisionPointRec(mouse, connectbox);
                writekey = CheckCollisionPointRec(mouse, writebox);
            }


            int key = GetCharPressed();
            while (key >0)
            { 
                if((32< key) && (key <= 125)) {
                    if(titlekey)
                    titlestring += (char)key;
                    if(connectkey)
                    connectstring += (char)key;
                    if(writekey)
                    myNarrative += (char)key;
                }
                key = GetCharPressed();
            };

            if(IsKeyPressed(KEY_BACKSPACE)) {
                if(titlekey && !titlestring.empty())
                titlestring.pop_back();
                if(connectkey&& !connectstring.empty())
                connectstring.pop_back();
                if(writekey && !myNarrative.empty())
                myNarrative.pop_back();
            };
            

            DrawText(titlestring.c_str(), 5, 5, 30, WHITE);
            DrawText(connectstring.c_str(), 30, 120, 30, WHITE);
            DrawText(myNarrative.c_str(), 5, 190, 30, WHITE);


            DrawRectangleLinesEx(titlebox, 3, WHITE);
            DrawRectangleLinesEx(connectbox, 3, WHITE);
            DrawRectangleLinesEx(writebox, 3, WHITE);


            Rectangle donebox = {380, 600, 60, 30};
            DrawRectangleLinesEx(donebox, 3, WHITE);
            DrawText("DONE", 380, 600, 20, WHITE);

        
            if(CheckCollisionPointRec(mouse, donebox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                anacave.writepage.submitNarrative(channelname, chaptername, titlestring, connectstring, myNarrative, anacave.user);
                anacave.current_state = anacave::anacavestate::CHAPTERPAGE;
            }
    }




    //설정창
    else if(anacave.current_state == anacave::anacavestate::SETTINGPAGE) {

        float wheel = GetMouseWheelMove();
        float scrollY = 10*wheel;

        bool nicknameon;

        bool idon;

        bool passwordon;

        string nicknamestring = "";

        string idstring = "";

        string passwordstring = "";

        anacave.Settingpage.Drawtitle();
        anacave.Settingpage.DrawTabbar();
        anacave.Settingpage.Drawlogin(scrollY, nicknameon, idon, passwordon,
        nicknamestring, idstring, passwordstring, anacave.userbox);
    }


    EndDrawing();
 }


CloseWindow(); 

return 0;
};
