#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <stdlib.h>
#include "sha1.h"
#include "base64.h"
#include <errno.h>

#define LOGV(...)  printf(__VA_ARGS__)
#define LOGD(...)  printf(__VA_ARGS__)
#define LOGE(...)  printf(__VA_ARGS__)
#define BUFFER_SIZE 1024

#define on_error(...)             \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    fflush(stderr);               \
    exit(1);                      \
  }

using namespace std;

namespace websocket_server {

  const std::string WHITESPACE = " \n\r\t\f\v";
  
  std::string ltrim(const std::string &s)
  {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
  }

	bool handshaking(int sock)
	{
    char buf[BUFFER_SIZE];
    int read = recv(sock, buf, BUFFER_SIZE, 0);
    printf("read=%d\n", read);
    buf[read] = 0;
    if (!read)
      return false; // done reading
    if (read < 0)
      printf("Client read failed\n");
    if (strncmp(buf, "GET", 3))
      return false;
  
    printf("%s\n", buf);
    char *p = strstr(buf, "Sec-WebSocket-Key:");
    p += strlen("Sec-WebSocket-Key:");
    p = strtok(p, "\n\r");
    std::string swk = ltrim(p);
    // printf("::%s::\n", swk.c_str());
    char swka[255];
    strcpy(swka, swk.c_str());
    strcat(swka, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    SHA1 checksum;
    checksum.update(swka);
    char bin_sha1_swka[100];
    int j = 0;
    std::string sha1_swka = checksum.final();
    printf("sha1::%s\n", sha1_swka.c_str());
    for (int i = 0; i < sha1_swka.length(); i += 2)
    {
      char c = sha1_swka.at(i);
      char high = (c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10;
      c = sha1_swka.at(i + 1);
      char low = (c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10;
      bin_sha1_swka[j++] = (high << 4) + low;
      //printf("%c%c-%d,", sha1_swka.at(i), c, bin_sha1_swka[j - 1]);
    }
#if 0        
    char tt[17] = "0123456789abcdef";
    for (int i = 0; i < j; i++)
    {
      char c = bin_sha1_swka[i];
      printf("%c%c", tt[(c & 0xf0) >> 4], tt[c & 0x0f]);
    }
    putchar('\n');
#endif        
    char base64_buf[1024];
    tools::base64::base64e(bin_sha1_swka, base64_buf, j);
    printf("base64::%s\n", base64_buf);
    char send_buf[1024];
    sprintf(send_buf, "HTTP/1.1 101 Switching Protocols\r\n"
                      "Connection: Upgrade\r\n"
                      "Upgrade: websocket\r\n"
                      "Sec-WebSocket-Accept: %s\r\n\r\n",
            base64_buf);
    int err = send(sock, send_buf, strlen(send_buf), 0);
    if (err < 0){
      on_error("Client write failed\n");
      return false;
    }
    return true;
  }
  
  //always mask, in case of client to server 
  int wrecv(int sock, char* buf, int buf_len, bool& bText)
  {
    int total_read = 0;
    bool bFirstFrame = true;
    unsigned char mask[4];
    printf("wrecv++++++ space %d\n", buf_len);
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
      printf("wrecv len header %d\n", len);
      if (len == 126) {
        unsigned char len_buf[2];
        nRead = 0;
        while (2 - nRead) {
          int n = recv(sock, len_buf + nRead, 2 - nRead, 0);
          nRead += n;
        }
        len = (len_buf[0] << 8) + len_buf[1];
      }
      printf("wrecv len %d\n", len);
      //read mask // Is there the mask in first frame?
      nRead = 0;
      while (4 - nRead) {
        int n = recv(sock, mask, 4 - nRead, 0);
        nRead += n;
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
    for (int i = 0; i < total_read; ++i)
      buf[i] = (buf[i] ^ mask[i % 4]);
  
    return total_read;
  }
  
  int wsend(int sock, char* buf, int len)
  {
    struct iovec iov[2] = {0};
  
    char header[4];
    int header_len;
    header[0] = (0x80 | 0x01); //FIN | TEXT
    header[1] = 0x00; //no masked
  
    int length;
    if (len < 126) {
      header[1] |= len;
      header_len = 2;
    }
    else if(len < 0x10000){
      header[1] |= 126;
      header[2] = (len & 0xff00) >> 8;
      header[3] = len & 0xff;
      header_len = 4;
    }
  
    iov[0].iov_base = header;
    iov[0].iov_len = header_len;
    iov[1].iov_base = buf;
    iov[1].iov_len = len;
    printf("wsend %d %d\n", header_len, len);
    int n = writev(sock, iov, 2);
    if(n < 0)
      printf("error writev %d, %s\n", errno, strerror(errno));
    return n;
  }
}

