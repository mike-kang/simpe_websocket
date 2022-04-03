
namespace websocket_client {
bool connect_handshaking(int sock, const char* ip, int port);
int wsend(int sock, const char* buf, int len);
int wrecv(int sock, char* buf, int buf_len, bool& bText);
}


