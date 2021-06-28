/* SungKyul University College of Engineering - Information Telecommunication Engineering */
/* 20170910 Park Soo Young - 2nd Grade Competition : Computer Communication Programming */

#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>

#define PORT_NUM 8000
#define BUFSIZE 512

WSADATA wsaData;

// IP 주소 얻기
int getAddrHost(void)
{
  int i;
  HOSTENT *lpHost;       //  호스트 정보를 저장하는 구조체
  IN_ADDR inaddr;       // IP 주소를 저장하는 구조체
  char szIPBuf[256], szIP[16];  // 호스트 이름, IP 주소를 저장하는 배열
 
  // 에러 처리
  if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
  {
    return -1;
  }
 
  // 로컬 머신의 호스트 이름을 얻는다
  gethostname(szIPBuf, (int)sizeof(szIPBuf));
 
  // 호스트 정보 얻기
  lpHost = gethostbyname(szIPBuf);

  // IP 주소 얻기
  for(i=0; lpHost->h_addr_list[i]; i++)
  {
    memcpy(&inaddr, lpHost->h_addr_list[i], 4);
  }

  strcpy(szIP, inet_ntoa(inaddr));
  printf("웹 브라우저에 다음 URL을 입력하세요. : http://%s:%d\n", szIP, PORT_NUM);  
 
  WSACleanup();
  return 0;
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr *)&clientaddr, &addrlen);

	while(1){
		// 데이터 받기
		retval = recv(client_sock, buf, BUFSIZE, 0);
		if(retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if(retval == 0)
			break;

		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if(retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	return 0;
}

int main(int argc, char *argv[])
{
	int retval;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	BOOL bValid = 1;
	char szBuf[2048];
	char szInBuf[2048];

	// 윈속 초기화
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		return 1;
	}

	// 소켓 생성 , 실패 시 에러 번호 출력
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET)
	{
		printf("Socket Error Number : %d", WSAGetLastError());
        return 1;
	}

	// 멘트 출력
	printf("성결대학교 공과대학 정보통신공학과 - 20170910 박수영\n");
	printf("2학년 컴퓨터통신 프로그래밍 경진대회 - Program 'SooYoung's WebServer\n\n");
	getAddrHost();
	printf("인터넷 익스플로어 브라우저에서 메시지 출력을 볼 수 있습니다. \n");
	printf("웹 서버 접속 시, 접속할 때마다 아래에 사용자 접속 정보가 표시됩니다.\n\n\n");

	// 소켓 설정
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT_NUM);
	serveraddr.sin_addr.S_un.S_addr = INADDR_ANY;

	// 소켓 옵션 설정
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&bValid, sizeof(bValid));
    if(bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Bind Error Number : %d", WSAGetLastError());
        return 1;
	}
	
	// TCP클라이언트로 부터 접속 요구를 대기
    retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR)
    {
        printf("Listen Error Number : %d", WSAGetLastError());
        return 1;
    }

	// 응답용 HTTP 메세지 작성
    memset(szBuf, 0, sizeof(szBuf));
    _snprintf(szBuf, sizeof(szBuf),
                "HTTP/1.0 200 OK\r\n"
                "Content-Length : 100\r\n"
                "Content-Type : text/html\r\n"
                "\r\n"
                "성결대학교 공과대학 정보통신공학과 20170910 박수영\n");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	int addrlen;
	HANDLE hThread;

	while(1)
	{
		// TCP클라이언트로 부터 접속 요구 받기
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET)
		{
			printf("Accept Error Number : %d", WSAGetLastError());
            return 1;
		}

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		if(hThread == NULL)
		{
			printf("스레드 생성 실패");
		}
		else
		{
			CloseHandle(hThread);
		}

		memset(szInBuf, 0, sizeof(szInBuf));
		recv(client_sock, szInBuf, sizeof(szInBuf), 0);

		// 클라이언트로 받은 요구 처리 필요(여기서는 생략)
        printf("%s", szInBuf);

		// 클라이언트에 테스트용 HTTP 메세지 송신
        send(client_sock, szBuf, (int)strlen(szBuf), 0);
	}

	closesocket(listen_sock); // 소켓 닫기
	WSACleanup(); // 윈속 종료
	return 0;
}
