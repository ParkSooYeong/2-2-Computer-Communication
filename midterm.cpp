/* SungKyul University College of Engineering - Information Telecommunication Engineering */
/* 20170910 Park Soo Young - 2nd Grade Competition : Computer Communication Programming */

#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include <winsock2.h>
#define PORT_NUM 8000
#define BUFSIZE 512

WSADATA wsaData;

// IP �ּ� ���
int getAddrHost(void)
{
  int i;
  HOSTENT *lpHost;       //  ȣ��Ʈ ������ �����ϴ� ����ü
  IN_ADDR inaddr;       // IP �ּҸ� �����ϴ� ����ü
  char szIPBuf[256], szIP[16];  // ȣ��Ʈ �̸�, IP �ּҸ� �����ϴ� �迭
 
  // ���� ó��
  if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
  {
    return -1;
  }
 
  // ���� �ӽ��� ȣ��Ʈ �̸��� ��´�
  gethostname(szIPBuf, (int)sizeof(szIPBuf));
 
  // ȣ��Ʈ ���� ���
  lpHost = gethostbyname(szIPBuf);

  // IP �ּ� ���
  for(i=0; lpHost->h_addr_list[i]; i++)
  {
    memcpy(&inaddr, lpHost->h_addr_list[i], 4);
  }

  strcpy(szIP, inet_ntoa(inaddr));
  printf("�� �������� ���� URL�� �Է��ϼ���. : http://%s:%d\n", szIP, PORT_NUM);  
 
  WSACleanup();
  return 0;
}

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr, clientaddrb;
	int addrlen;
	char buf[BUFSIZE+1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);

	while(1){
		// ������ �ޱ�
		retval = recv(client_sock, buf, BUFSIZE, 0);
		if(retval == SOCKET_ERROR){
			err_display("recv()");
			break;
		}
		else if(retval == 0)
			break;

		// ���� ������ ���
		//buf[retval] = '\0';
		//printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
			//ntohs(clientaddr.sin_port), buf);

		// ������ ������
		retval = send(client_sock, buf, retval, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			break;
		}
	}

	// closesocket()
	closesocket(client_sock);
	//printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		//inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	return 0;
}

int main(int argc, char *argv[])
{
	int nlen;
	SOCKET sockSS;
	struct sockaddr_in addrSockSvr;
	struct sockaddr_in addrSockclt;
	long nRet;
	BOOL bValid = 1;
	char szBuf[2048];
	char szInBuf[2048];

	// ���� �ʱ�ȭ , ���� �������� ������ ó���ϴ� ���
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

	// ���� ���� , ���� �� ���� ��ȣ ���
    SOCKET sockSvr = socket(AF_INET, SOCK_STREAM, 0);
    if(sockSvr == INVALID_SOCKET)
    {
        printf("Socket Error Number : %d", WSAGetLastError());
        return 1;
    }

	printf("������б� �������� ������Ű��а� - 20170910 �ڼ���\n");
	printf("2�г� ��ǻ����� ���α׷��� ������ȸ - Program 'SooYoung's WebServer\n\n");
	getAddrHost();
	printf("���ͳ� �ͽ��÷ξ� ���������� �޽��� ����� �� �� �ֽ��ϴ�. \n");
	printf("�� ���� ���� ��, ������ ������ �Ʒ��� ����� ���� ������ ǥ�õ˴ϴ�.\n\n\n");

    // ���� ����
    addrSockSvr.sin_family = AF_INET;
    addrSockSvr.sin_port = htons(PORT_NUM);
    addrSockSvr.sin_addr.S_un.S_addr = INADDR_ANY;

    // ���� �ɼ� ����
    setsockopt(sockSvr, SOL_SOCKET, SO_REUSEADDR, (const char *)&bValid, sizeof(bValid));
    if(bind(sockSvr, (struct sockaddr *)&addrSockSvr, sizeof(addrSockSvr)) != 0)
    {
        printf("Bind Error Number : %d", WSAGetLastError());
        return 1;
	}

    // TCPŬ���̾�Ʈ�� ���� ���� �䱸�� ���
    if(listen(sockSvr, 5) != 0)
    {
        printf("Listen Error Number : %d", WSAGetLastError());
        return 1;
    }

    // ����� HTTP �޼��� �ۼ�
    memset(szBuf, 0, sizeof(szBuf));
    _snprintf(szBuf, sizeof(szBuf),
                "HTTP/1.0 200 OK\r\n"
                "Content-Length : 100\r\n"
                "Content-Type : text/html\r\n"
                "\r\n"
                "������б� �������� ������Ű��а� 20170910 �ڼ���\n");

    while (1)
	{
		// TCPŬ���̾�Ʈ�� ���� ���� �䱸 �ޱ�
        nlen = sizeof(addrSockclt);
        sockSS = accept(sockSvr, (struct sockaddr *)&addrSockclt, &nlen);
        if(sockSS == INVALID_SOCKET)
        {
            printf("Accept Error Number : %d", WSAGetLastError());
            return 1;
        }

        memset(szInBuf, 0, sizeof(szInBuf));
        recv(sockSS, szInBuf, sizeof(szInBuf), 0);

        // Ŭ���̾�Ʈ�� ���� �䱸 ó�� �ʿ�(���⼭�� ����)
        printf("%s", szInBuf);

        // Ŭ���̾�Ʈ�� �׽�Ʈ�� HTTP �޼��� �۽�
        send(sockSS, szBuf, (int)strlen(szBuf), 0);

        closesocket(sockSS); // ���� �ݱ�
    }

    // ���� ����
    WSACleanup();
    return 0;
}