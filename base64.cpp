#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace tools {
namespace base64 {

static const char MimeBase64[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static const char MimeBase64_http[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '-', '/'
};

 
static int DecodeMimeBase64[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};
 
typedef union{
    struct{
        unsigned char c1,c2,c3;
    };
    struct{
        unsigned int e1:6,e2:6,e3:6,e4:6;
    };
} BF;
 
void base64e(const char *src, char *result, int length){
    int i, j = 0;
    BF temp;
 
    for(i = 0 ; i < length ; i = i+3, j = j+4){
        temp.c3 = src[i];
        if((i+1) > length) temp.c2 = 0;
          else temp.c2 = src[i+1];
        if((i+2) > length) temp.c1 = 0;
          else temp.c1 = src[i+2];
 
        result[j]   = MimeBase64[temp.e4];
        result[j+1] = MimeBase64[temp.e3];
        result[j+2] = MimeBase64[temp.e2];
        result[j+3] = MimeBase64[temp.e1];
        //printf("%x,%x,%x->%c%c%c%c\n", temp.c3, temp.c2, temp.c1, MimeBase64[temp.e4], MimeBase64[temp.e3], MimeBase64[temp.e2], MimeBase64[temp.e1]);
        if((i+2) > length) result[j+2] = '=';
        if((i+3) > length) result[j+3] = '=';
    }
}
 
void base64d(const char *src, int src_length, char *result, int *length){
    int i, j = 0, blank = 0;
    BF temp;

    if(src_length < 0)
      src_length = strlen(src);
    
    for(i = 0 ; i < src_length ; i = i+4, j = j+3){
        temp.e4 = DecodeMimeBase64[src[i]];
        temp.e3 = DecodeMimeBase64[src[i+1]];
        if(src[i+2] == '='){
            temp.e2 = 0;
            blank++;
        } else temp.e2 = DecodeMimeBase64[src[i+2]];
        if(src[i+3] == '='){
            temp.e1 = 0;
            blank++;
        } else temp.e1 = DecodeMimeBase64[src[i+3]];
 
        result[j]   = temp.c3;
        result[j+1] = temp.c2;
        result[j+2] = temp.c1;
    }
    *length = j-blank;
}

struct BF2{
    unsigned char _c0_2:2,c0_6:6;
    unsigned char _c1_4:4,c1_4:4;
    unsigned char _c2_6:6,c2_2:2;
};// __attribute__((packed));

int base64e2_get_needbufSize(int size)
{ 
  int t = size / 3 * 4; 
  return ((size % 3)? t + 4: t) + 1; 
}
int base64e2(const char *src, int length, char *result){
    int i, j = 0;
    int sz = length / 3;
    BF2* p = (BF2*)src;
    char * o = result;;
    //cout << sizeof(BF2) << endl;
    //cout << &p->e0 << ":" << &p->e1 << endl;
    
    for(i = 0 ; i < sz ; i++){
      //printf("%x: %x: %x\n", p->c1 ,p->c2, p->c3);
      *o++ = MimeBase64[p->c0_6];
      //printf("%x\n", p->c0_6);
      *o++ = MimeBase64[(p->_c0_2 << 4) + p->c1_4];
      //printf("%x:%x\n", p->_c0_2, p->c1_4);
      *o++ = MimeBase64[(p->_c1_4 << 2) + p->c2_2];
      //printf("%x:%x\n", p->_c1_4, p->c2_2);
      *o++ = MimeBase64[p->_c2_6];
      //printf("%x\n", p->_c2_6);

      p++;
    }

    int mod = length % 3;
    
    if(mod == 1){
      *o++ = MimeBase64[p->c0_6];
      *o++ = MimeBase64[(p->_c0_2 << 4)];
      *o++ = '=';
      *o++ = '=';
    }
    else if(mod ==2){
      *o++ = MimeBase64[p->c0_6];
      *o++ = MimeBase64[(p->_c0_2 << 4) + p->c1_4];
      *o++ = MimeBase64[(p->_c1_4 << 2)];
      *o++ = '=';
    }
    *o = '\0';
    return (o - result);
}

int base64e2_http(const char *src, int length, char *result){
  int i, j = 0;
  int sz = length / 3;
  BF2* p = (BF2*)src;
  char * o = result;;
  //cout << sizeof(BF2) << endl;
  //cout << &p->e0 << ":" << &p->e1 << endl;
  
  for(i = 0 ; i < sz ; i++){
    //printf("%x: %x: %x\n", p->c1 ,p->c2, p->c3);
    *o++ = MimeBase64_http[p->c0_6];
    //printf("%x\n", p->c0_6);
    *o++ = MimeBase64_http[(p->_c0_2 << 4) + p->c1_4];
    //printf("%x:%x\n", p->_c0_2, p->c1_4);
    *o++ = MimeBase64_http[(p->_c1_4 << 2) + p->c2_2];
    //printf("%x:%x\n", p->_c1_4, p->c2_2);
    *o++ = MimeBase64_http[p->_c2_6];
    //printf("%x\n", p->_c2_6);
  
    p++;
  }
  
  int mod = length % 3;
  
  if(mod == 1){
    *o++ = MimeBase64_http[p->c0_6];
    *o++ = MimeBase64_http[(p->_c0_2 << 4)];
    *o++ = '=';
    *o++ = '=';
  }
  else if(mod ==2){
    *o++ = MimeBase64_http[p->c0_6];
    *o++ = MimeBase64_http[(p->_c0_2 << 4) + p->c1_4];
    *o++ = MimeBase64_http[(p->_c1_4 << 2)];
    *o++ = '=';
  }
  *o = '\0';
  return (o - result);
}

}}
