// websocketTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <winsock2.h>
#include <iostream>
#include <cstdio>

#define LOGV(...)  printf(__VA_ARGS__)
#define LOGD(...)  printf(__VA_ARGS__)
#define LOGE(...)  printf(__VA_ARGS__)

using namespace std;

namespace websocket_client {

	bool connect_handshaking(int sock, const char* ip, int port)
	{
#define FORMAT "GET / HTTP1.1\r\n" \
		"Host: %s:%d\r\n"	\
		"Connection: Upgrade\r\n" \
		"Upgrade: websocket\r\n" \
		"Sec-WebSocket-Version: 13\r\n" \
		"Sec-WebSocket-Key: jPOlmNeyKb8+zxB0uwOPjA==\r\n"

		SOCKADDR_IN servAddr;
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = inet_addr(ip);
		servAddr.sin_port = htons(port);

		if (::connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr)) < 0) {
			closesocket(sock);
			LOGE("connect() failed");
			return false;
		}

		char buf[1024];
		sprintf(buf, FORMAT, ip, port);
		send(sock, buf, strlen(buf), 0);
		int n = recv(sock, buf, 1024, 0);
		buf[n] = '\0';
		printf("recv[%d]:%s\n", n, buf);
		return true;
	}

	int wsend(int sock, const char* buf, int len)
	{
		unsigned char send_buf[2048];
		send_buf[0] = (0x80 | 0x01);
		send_buf[1] = 0x80;	//masked
		//mask buf
		char mask[] = { 61, 84, 35, 6 };
		int offset;
		if (len < 125) {
			send_buf[1] |= len;
			offset = 2;
		}
		else if (len < 0x10000) {
			send_buf[1] |= 126;
			send_buf[2] = (len & 0xff00) >> 8;
			send_buf[3] = len & 0xff;
			offset = 4;
		}
		else {
			printf("not support len:%d\n", len);
			return -1;
		}
		memcpy(&send_buf[offset], mask, 4);
		offset += 4;
		unsigned char* p = send_buf + offset;
		for (int i = 0; i < len; i++)
			p[i] = buf[i] ^ mask[i % 4];

		int n = send(sock, (char*)send_buf, offset + len, 0);
		return n;
	}

	//always no mask, in case of server to clent 
	int wrecv(int sock, char* buf, int buf_len, bool& bText)
	{
		int total_read = 0;
		bool bFirstFrame = true;
		while (true) {
			char header[2];
			int nRead = 0;
			while (2 - nRead) {
				int n = recv(sock, header + nRead, 2 - nRead, 0);
				nRead += n;
			}
			bool bFin = ((header[0] & 0x80) == 0x80);
			if (bFirstFrame) {
				bText = ((header[0] & 0x0f) == 1);
				bFirstFrame = false;
			}
			int len = header[1] & 0x7F;

			if (len == 126) {
				unsigned char len_buf[2];
				nRead = 0;
				while (2 - nRead) {
					int n = recv(sock, (char*)len_buf + nRead, 2 - nRead, 0);
					nRead += n;
				}
				len = len_buf[0];
				len = len << 8;
				len += len_buf[1];
				printf("len == 126; len:%d\n", len);
			}
			nRead = 0;
			char * p = buf + total_read;
			while (len - nRead) {
				int n = recv(sock, p + nRead, len - nRead, 0);
				nRead += n;
			}
			total_read += nRead;
			if (bFin)
				break;
		}
		return total_read;
	}
}

