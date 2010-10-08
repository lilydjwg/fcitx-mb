#include<iostream>
#include<cstring>
#include<iconv.h>
#ifndef MY_ICONV_HEADER
#define MY_ICONV_HEADER
using namespace std;

class Iconv {
  public:
    Iconv(const char* from, const char* to);
    ~Iconv();
    string operator () (const char* str, const int len);
  private:
    char buff_i[1024];
    char buff_o[1024];
    iconv_t iconv_pt;
};
#endif
