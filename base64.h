
namespace tools {
namespace base64 {
void base64e(const char *src, char *result, int length);
int base64e2_get_needbufSize(int size);
int base64e2(const char *src, int length, char *result);
int base64e2_http(const char *src, int length, char *result);

void base64d(const char *src, int src_length, char *result, int *length);
}
}
