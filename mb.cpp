/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// FIXME  曾出现了错误，导致码表末尾出现一个编码为4字节乱码的项 2010年2月13日

#include<iostream>
#include"mb.h"

const char *TMP_PRINT = "/tmp/fcitx_mb";
using namespace std;

RECORD::RECORD(){
  hit = index = 0;
  ispy = false;
}

RECORD::RECORD(string Code, string Hz){
  code = Code;
  hz = Hz;
  hit = index = 0;
  ispy = false;
}

string RECORD::toString(){
  return "["+code+":"+hz+"]";
}

bool operator<(const RECORD& a, const RECORD& b){
    return a.code < b.code;
}

unsigned int TABLE::loadTable (char *fname) {
    //不能载入拼音词库 /usr/share/fcitx/data/pyphrase.mb
    //和 /usr/share/fcitx/data/pybase.mb
    //后来发现因为它们不是正确的码表！！

    FILE		*fpDict;
    unsigned int	 iTemp;
    string		 sTemp;
    unsigned char	 cTemp;
    char		 strTemp[STR_MAX];

    version = 0;
    notCode = "";

    fpDict = fopen (fname, "rb");
    if (!fpDict) {
	fprintf (stderr, "\nCan not read source file `%s'!\n\n", fname);
	return FILE_ERROR;
    }

    //先读取码表的信息
    fread (&iTemp, sizeof (unsigned int), 1, fpDict);//版本指示
    if (iTemp == 0) {
	fread (&version, sizeof (char), 1, fpDict);//版本号
	// printf (";fcitx 版本 0x%02x 码表文件\n", version);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);//键码字串的长度
    }
    else
	version = '\x02';
	// printf (";fcitx 版本 0x02 码表文件\n");

    fread (strTemp, sizeof (char), iTemp + 1, fpDict);
    code = strTemp;
    // printf ("键码=%s\n", strTemp);
    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
    codeLen = cTemp;
    // printf ("码长=%d\n", iTemp);

    if (version) {
	fread (&cTemp, sizeof (unsigned char), 1, fpDict);
	pyLen = cTemp;
	// if (cTemp) {
	    // printf ("拼音=@\n");
	    // printf ("拼音长度=%d\n", iPYLen);
	// }
    }

    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    fread (strTemp, sizeof (char), iTemp + 1, fpDict);
    notCode = strTemp;
    // if (iTemp)
	// printf ("规避字符=%s\n", strTemp);

    unsigned int i;
    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
    if (cTemp) {		//表示有组词规则
	// printf ("[组词规则]\n");
	for (i = 0; i < codeLen - 1; i++) {
	    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
	    // printf ("%c", (iRule) ? 'a' : 'e');
	    sTemp = (cTemp) ? 'a' : 'e';
	    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
	    // printf ("%d=", iRule);
	    sTemp += cTemp + 0x30;
	    sTemp += '=';
	    for (iTemp = 0; iTemp < codeLen; iTemp++) {
		fread (&cTemp, sizeof (unsigned char), 1, fpDict);
		// printf ("%c", (cTemp) ? 'p' : 'n');
		sTemp += (cTemp) ? 'p' : 'n';
		fread (&cTemp, sizeof (unsigned char), 1, fpDict);
		// printf ("%d", cTemp);
		sTemp += cTemp + 0x30;
		fread (&cTemp, sizeof (unsigned char), 1, fpDict);
		// printf ("%d", cTemp);
		sTemp += cTemp + 0x30;
		if (iTemp != (codeLen - 1))
		    // printf ("+");
		    sTemp += "+";
	    }
	    // printf ("\n");
	    wordsRule.push_back(sTemp);
	}
    }

    // printf ("[数据]\n");
    unsigned int j;//词的数量
    unsigned int k;
    unsigned int l;
    char strHZ[STR_MAX];
    RECORD record;
    fread (&j, sizeof (unsigned int), 1, fpDict);

    if (version)
	iTemp = pyLen;

    for (i = 0; i < j; i++) {
	fread (strTemp, sizeof (char), iTemp + 1, fpDict);//读取键码
	fread (&k, sizeof (unsigned int), 1, fpDict);//汉字的字节数+1
	fread (strHZ, sizeof (unsigned char), k, fpDict);

	if (version) {
	    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
	    //这个是用来指示是否为拼音的
	    if (cTemp) {
		// printf ("@%s %s\n", strCode, strHZ);
		record.ispy = true;
	    } else {
		// printf ("%s %s\n", strCode, strHZ);
		record.ispy = false;
	    }
	}
	sTemp = strTemp;

	record.code = sTemp;
	record.hz = strHZ;

	fread (&l, sizeof (unsigned int), 1, fpDict);
	record.hit = l;
	fread (&l, sizeof (unsigned int), 1, fpDict);
	record.index = l;

	records.push_back(record);
    }

    fclose (fpDict);
    return 0;
}

void TABLE::print(){
    printf (";fcitx 版本 0x%02x 码表文件\n", version);
    cout<<"键码="<<code<<endl;
    cout<<"码长="<<(int)codeLen<<endl;
    if(pyLen){
	printf ("拼音=@\n");
	printf ("拼音长度=%d\n", pyLen);
    }
    if(notCode != "")
	cout<<"规避字符="<<notCode<<endl;
    unsigned int i;
    if(wordsRule.size()){
	cout<<"[组词规则]"<<endl;
	for(i=0; i<wordsRule.size(); i++)
	    cout<<wordsRule[i]<<endl;
    }

    cout<<"[数据]"<<endl;
    for(i=0;i<records.size();i++){
	if(records[i].ispy)
	    cout<<'@';
	cout<<records[i].code<<" "<<records[i].hz<<endl;
    }
}

void TABLE::vprint(){
    printf (";fcitx 版本 0x%02x 码表文件\n", version);
    cout<<"键码="<<code<<endl;
    cout<<"码长="<<(int)codeLen<<endl;
    if(pyLen){
	printf ("拼音=@\n");
	printf ("拼音长度=%d\n", pyLen);
    }
    if(notCode != "")
	cout<<"规避字符="<<notCode<<endl;
    unsigned int i;
    if(wordsRule.size()){
	cout<<"[组词规则]"<<endl;
	for(i=0; i<wordsRule.size(); i++)
	    cout<<wordsRule[i]<<endl;
    }

    cout<<"[数据]"<<endl;
    for(i=0;i<records.size();i++){
	if(records[i].ispy)
	    cout<<'@';
	cout<<records[i].code<<" "<<records[i].hz<<" "<<records[i].hit<<" "<<records[i].index<<endl;
    }
}

bool TABLE::checkCode(string Code){
  if(Code.length() > codeLen) return false;
  for(int i = 0; i < Code.length(); i++){
    if(code.find(Code.at(i)) == string::npos)
      return false;
  }
  return true;
}

vector<RECORD> TABLE::search_code(string from, string to){
  if(to == "")
    to = from;

  //二分搜索
  int left = 0;
  int right = records.size()-1;
  int middle = -1;
  vector<RECORD> results;
  while(left<=right){
    middle = (left+right)/2;
    if (from==records[middle].code) break;
    if (from>records[middle].code) left=middle+1;
    else
      right=middle-1;
  }

  if(middle == records.size()-1 && from != records[middle].code){//找到最后都没有找到
      return results;
  }

  //向前回溯
  while(middle >= 0 && from == records[middle].code){
    middle--;
  }
  middle++;

  //向后查找
  while(records[middle].code <= to){
    results.push_back(records[middle]);
    middle++;
    if(middle > records.size()-1)//已经找到最后一个了
	break;
  }

  return results;
}

vector<string> TABLE::search_hz(string tgt){
  vector<string> results;
  for(int i=0;i<records.size();i++){
      if(records[i].hz == tgt)
	  results.push_back(records[i].code);
  }

  return results;
}

int TABLE::ins(RECORD record){
  //先看看数据是否已经存在
  vector<RECORD> alreadyCode = search_code(record.code);
  for(int i=0;i<alreadyCode.size();i++){
      if(alreadyCode[i].hz == record.hz)//已经有了
	  return 1;
  }

  //二分搜索应插入的位置
  int left = 0;
  int right = records.size()-1;
  int middle = -1;

  while(left<=right){
    middle = (left+right)/2;
    if (record.code==records[middle].code) break;
    if (record.code>records[middle].code) left=middle+1;
    else
      right=middle-1;
  }
  //插入在编码刚好比其大的后面，但二分搜索出来的，如果不相等，那么既有可能大，
  //也有可能小
  if (record.code>records[middle].code) middle++;

  vector<RECORD>::iterator pos = records.begin() + middle;
  records.insert(pos, record);

  return 0;
}

int TABLE::del(RECORD record){
  int count = 0;//计数

  //二分搜索应删除的位置
  int left = 0;
  int right = records.size()-1;
  int middle = -1;

  while(left<=right){
    middle = (left+right)/2;
    if (record.code==records[middle].code) break;
    if (record.code>records[middle].code) left=middle+1;
    else
      right=middle-1;
  }

  if(middle == records.size()-1 && record.code != records[middle].code)//找到最后都没有找到
      return count;

  //向前回溯
  while(middle >= 0 && record.code == records[middle].code){
    middle--;
  }
  middle++;

  //向后查找
  while(records[middle].code == record.code){
    if(records[middle].hz == record.hz){
      vector<RECORD>::iterator pos = records.begin() + middle;
      records.erase(pos);
      count++;
    }
    middle++;
    if(middle > records.size()-1)//已经找到最后一个了
	break;
  }

  return count;
}

void TABLE::write(string fname){
  ofstream thisprint(TMP_PRINT);

  thisprint<<";fcitx 版本 0x";
  thisprint.fill('0');
  thisprint.width(2);
  thisprint<<(int)version;
  thisprint.fill(' ');
  thisprint.width(0);
  thisprint<<" 码表文件"<<endl;
  thisprint<<"键码="<<code<<endl;
  thisprint<<"码长="<<(int)codeLen<<endl;
  if(pyLen){
    thisprint<<"拼音=@"<<endl;
    thisprint<<"拼音长度="<<(int)pyLen<<endl;
  }
  if(notCode != "")
    thisprint<<"规避字符="<<notCode<<endl;
  unsigned int i;
  if(wordsRule.size()){
    thisprint<<"[组词规则]"<<endl;
    for(i=0; i<wordsRule.size(); i++)
      thisprint<<wordsRule[i]<<endl;
  }

  thisprint<<"[数据]"<<endl;
  for(i=0;i<records.size();i++)
    thisprint<<records[i].code<<" "<<records[i].hz<<endl;

  thisprint.close();

  string cmd = "txt2mb " + (string)TMP_PRINT + " " + fname;
  system(cmd.c_str());
}

int TABLE::vwrite(string fname){
    FILE		*fpDict;
    unsigned int	 iTemp;
    unsigned char	 cTemp;
    char		*strTemp;

    fpDict = fopen(fname.c_str(), "wb");
    if (!fpDict) {
	return FILE_ERROR;
    }

    iTemp = 0;
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);//版本指示
    cTemp = version;
    fwrite(&cTemp, sizeof (char), 1, fpDict);//版本号
    iTemp = code.length();
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);//键码字串的长度
    strTemp = (char*)malloc(sizeof(char) * iTemp + 1);
    strcpy(strTemp, code.c_str());
    fwrite(strTemp, sizeof (char) * iTemp + 1, 1, fpDict);
    cTemp = codeLen;
    fwrite(&cTemp, sizeof (unsigned char), 1, fpDict);
    if (version) {
	cTemp = pyLen;
	fwrite(&cTemp, sizeof (unsigned char), 1, fpDict);
    }
    iTemp = notCode.length();
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
    strTemp = (char*)realloc(strTemp, sizeof(char) * iTemp + 1);
    strcpy(strTemp, notCode.c_str());
    fwrite(strTemp, sizeof (char) * iTemp + 1, 1, fpDict);

    if(!wordsRule.empty()){//组词规则
	cTemp = '\x07';
	fwrite(&cTemp, sizeof (unsigned char), 1, fpDict);
	for(int i=0;i<codeLen-1;i++){
	    istringstream s(wordsRule[i]);
	    char ch;

	    s.get(ch);
	    if(ch == 'e')
		cTemp = 0;
	    else
		cTemp = 1;
	    fwrite(&cTemp, sizeof (unsigned char), 1, fpDict);
	    s.get(ch);
	    ch -= 0x30;
	    fwrite(&ch, sizeof (unsigned char), 1, fpDict);
	    s.get(ch);// '='
	    for(iTemp = 0; iTemp<codeLen; iTemp++){
		s.get(ch);
		if(ch == 'p')
		    cTemp = 1;
		else
		    cTemp = 0;
		fwrite(&cTemp, sizeof (unsigned char), 1, fpDict);
		s.get(ch);
		ch -= 0x30;
		fwrite(&ch, sizeof (unsigned char), 1, fpDict);
		s.get(ch);
		ch -= 0x30;
		fwrite(&ch, sizeof (unsigned char), 1, fpDict);
		if(iTemp != codeLen-1)
		    s.get(ch);// '+'
	    }
	}
    }

    //数据
    iTemp = records.size();
    cTemp = 0;
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
    for(int i=0; i<records.size(); i++){
	//这里编码长度按 pyLen 算
	strTemp = (char*)realloc(strTemp, sizeof(char) * pyLen + 1);
	strcpy(strTemp, records[i].code.c_str());
	fwrite(strTemp, sizeof (char) * pyLen + 1, 1, fpDict);
	iTemp = records[i].hz.length() +1;
	fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
	strTemp = (char*)realloc(strTemp, sizeof(char) * iTemp);
	strcpy(strTemp, records[i].hz.c_str());
	fwrite(strTemp, sizeof (char) * iTemp, 1, fpDict);
	//这个是用来指示是否为拼音的
	if(records[i].ispy)
	    cTemp = 1;
	else
	    cTemp = 0;
	fwrite(&cTemp, sizeof (unsigned char), 1, fpDict);
	iTemp = records[i].hit;
	fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
	iTemp = records[i].index;
	fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
    }

    free(strTemp);
    fclose(fpDict);

    return records.size();
}

int TABLE::getSize(){
    return records.size();
}

void TABLE::sortTable(){
    //XXX 如果含有拼音间隔符
    //但也没有报顺序不对
    sort(records.begin(), records.end());
}


bool TABLE::isSorted(){
    for(int i=0; i<records.size()-1; i++){
	if(records[i+1] < records[i])
	    return false;
    }
    return true;
}

// vim:sw=4:fdm=expr:fde=getline(v\:lnum+1)=~'\\v^([A-Za-z]|/)+'?0\:1
