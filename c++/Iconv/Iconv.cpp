#include"Iconv.h"
using namespace std;

Iconv::Iconv(const char* from, const char* to) {
  iconv_pt = iconv_open(to, from);
  if (iconv_pt == (iconv_t)-1) {
    cerr << "iconv_open failed: "
      << from << ", " << to << endl;
  }
  iconv(iconv_pt, NULL, NULL, NULL, NULL);
}

Iconv::~Iconv() {
  iconv_close(iconv_pt);
}

string Iconv::operator () (const char* str, const int len) {
  int ret;
  size_t len_i, len_o;
  char *sin, *sout;

  len_i = len;
  len_o = 1024;
  sin   = buff_i;
  sout  = buff_o;

  strncpy(buff_i, str, sizeof(buff_i));
  ret = iconv(iconv_pt, &sin, &len_i, &sout, &len_o);
  if (ret == -1) {
    cerr << "convert error." << endl;
    cerr << "iconv stop at: " << buff_i << endl;
  }
  return string(buff_o, 1024-len_o);
}

