
namespace websocket_server {
bool handshaking(int sock);
int wrecv(int sock, char* buf, int buf_len, bool& bText);
int wsend(int sock, char* buf, int len);
}


