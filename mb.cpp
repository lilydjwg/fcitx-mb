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

// FIXME  �������˴��󣬵������ĩβ����һ������Ϊ4�ֽ�������� 2010��2��13��

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
    //��������ƴ���ʿ� /usr/share/fcitx/data/pyphrase.mb
    //�� /usr/share/fcitx/data/pybase.mb
    //����������Ϊ���ǲ�����ȷ�������

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

    //�ȶ�ȡ������Ϣ
    fread (&iTemp, sizeof (unsigned int), 1, fpDict);//�汾ָʾ
    if (iTemp == 0) {
	fread (&version, sizeof (char), 1, fpDict);//�汾��
	// printf (";fcitx �汾 0x%02x ����ļ�\n", version);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);//�����ִ��ĳ���
    }
    else
	version = '\x02';
	// printf (";fcitx �汾 0x02 ����ļ�\n");

    fread (strTemp, sizeof (char), iTemp + 1, fpDict);
    code = strTemp;
    // printf ("����=%s\n", strTemp);
    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
    codeLen = cTemp;
    // printf ("�볤=%d\n", iTemp);

    if (version) {
	fread (&cTemp, sizeof (unsigned char), 1, fpDict);
	pyLen = cTemp;
	// if (cTemp) {
	    // printf ("ƴ��=@\n");
	    // printf ("ƴ������=%d\n", iPYLen);
	// }
    }

    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    fread (strTemp, sizeof (char), iTemp + 1, fpDict);
    notCode = strTemp;
    // if (iTemp)
	// printf ("����ַ�=%s\n", strTemp);

    unsigned int i;
    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
    if (cTemp) {		//��ʾ����ʹ���
	// printf ("[��ʹ���]\n");
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

    // printf ("[����]\n");
    unsigned int j;//�ʵ�����
    unsigned int k;
    unsigned int l;
    char strHZ[STR_MAX];
    RECORD record;
    fread (&j, sizeof (unsigned int), 1, fpDict);

    if (version)
	iTemp = pyLen;

    for (i = 0; i < j; i++) {
	fread (strTemp, sizeof (char), iTemp + 1, fpDict);//��ȡ����
	fread (&k, sizeof (unsigned int), 1, fpDict);//���ֵ��ֽ���+1
	fread (strHZ, sizeof (unsigned char), k, fpDict);

	if (version) {
	    fread (&cTemp, sizeof (unsigned char), 1, fpDict);
	    //���������ָʾ�Ƿ�Ϊƴ����
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
    printf (";fcitx �汾 0x%02x ����ļ�\n", version);
    cout<<"����="<<code<<endl;
    cout<<"�볤="<<(int)codeLen<<endl;
    if(pyLen){
	printf ("ƴ��=@\n");
	printf ("ƴ������=%d\n", pyLen);
    }
    if(notCode != "")
	cout<<"����ַ�="<<notCode<<endl;
    unsigned int i;
    if(wordsRule.size()){
	cout<<"[��ʹ���]"<<endl;
	for(i=0; i<wordsRule.size(); i++)
	    cout<<wordsRule[i]<<endl;
    }

    cout<<"[����]"<<endl;
    for(i=0;i<records.size();i++){
	if(records[i].ispy)
	    cout<<'@';
	cout<<records[i].code<<" "<<records[i].hz<<endl;
    }
}

void TABLE::vprint(){
    printf (";fcitx �汾 0x%02x ����ļ�\n", version);
    cout<<"����="<<code<<endl;
    cout<<"�볤="<<(int)codeLen<<endl;
    if(pyLen){
	printf ("ƴ��=@\n");
	printf ("ƴ������=%d\n", pyLen);
    }
    if(notCode != "")
	cout<<"����ַ�="<<notCode<<endl;
    unsigned int i;
    if(wordsRule.size()){
	cout<<"[��ʹ���]"<<endl;
	for(i=0; i<wordsRule.size(); i++)
	    cout<<wordsRule[i]<<endl;
    }

    cout<<"[����]"<<endl;
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

  //��������
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

  if(middle == records.size()-1 && from != records[middle].code){//�ҵ����û���ҵ�
      return results;
  }

  //��ǰ����
  while(middle >= 0 && from == records[middle].code){
    middle--;
  }
  middle++;

  //������
  while(records[middle].code <= to){
    results.push_back(records[middle]);
    middle++;
    if(middle > records.size()-1)//�Ѿ��ҵ����һ����
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
  //�ȿ��������Ƿ��Ѿ�����
  vector<RECORD> alreadyCode = search_code(record.code);
  for(int i=0;i<alreadyCode.size();i++){
      if(alreadyCode[i].hz == record.hz)//�Ѿ�����
	  return 1;
  }

  //��������Ӧ�����λ��
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
  //�����ڱ���պñ����ĺ��棬���������������ģ��������ȣ���ô���п��ܴ�
  //Ҳ�п���С
  if (record.code>records[middle].code) middle++;

  vector<RECORD>::iterator pos = records.begin() + middle;
  records.insert(pos, record);

  return 0;
}

int TABLE::del(RECORD record){
  int count = 0;//����

  //��������Ӧɾ����λ��
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

  if(middle == records.size()-1 && record.code != records[middle].code)//�ҵ����û���ҵ�
      return count;

  //��ǰ����
  while(middle >= 0 && record.code == records[middle].code){
    middle--;
  }
  middle++;

  //������
  while(records[middle].code == record.code){
    if(records[middle].hz == record.hz){
      vector<RECORD>::iterator pos = records.begin() + middle;
      records.erase(pos);
      count++;
    }
    middle++;
    if(middle > records.size()-1)//�Ѿ��ҵ����һ����
	break;
  }

  return count;
}

void TABLE::write(string fname){
  ofstream thisprint(TMP_PRINT);

  thisprint<<";fcitx �汾 0x";
  thisprint.fill('0');
  thisprint.width(2);
  thisprint<<(int)version;
  thisprint.fill(' ');
  thisprint.width(0);
  thisprint<<" ����ļ�"<<endl;
  thisprint<<"����="<<code<<endl;
  thisprint<<"�볤="<<(int)codeLen<<endl;
  if(pyLen){
    thisprint<<"ƴ��=@"<<endl;
    thisprint<<"ƴ������="<<(int)pyLen<<endl;
  }
  if(notCode != "")
    thisprint<<"����ַ�="<<notCode<<endl;
  unsigned int i;
  if(wordsRule.size()){
    thisprint<<"[��ʹ���]"<<endl;
    for(i=0; i<wordsRule.size(); i++)
      thisprint<<wordsRule[i]<<endl;
  }

  thisprint<<"[����]"<<endl;
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
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);//�汾ָʾ
    cTemp = version;
    fwrite(&cTemp, sizeof (char), 1, fpDict);//�汾��
    iTemp = code.length();
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);//�����ִ��ĳ���
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

    if(!wordsRule.empty()){//��ʹ���
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

    //����
    iTemp = records.size();
    cTemp = 0;
    fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
    for(int i=0; i<records.size(); i++){
	//������볤�Ȱ� pyLen ��
	strTemp = (char*)realloc(strTemp, sizeof(char) * pyLen + 1);
	strcpy(strTemp, records[i].code.c_str());
	fwrite(strTemp, sizeof (char) * pyLen + 1, 1, fpDict);
	iTemp = records[i].hz.length() +1;
	fwrite(&iTemp, sizeof (unsigned int), 1, fpDict);
	strTemp = (char*)realloc(strTemp, sizeof(char) * iTemp);
	strcpy(strTemp, records[i].hz.c_str());
	fwrite(strTemp, sizeof (char) * iTemp, 1, fpDict);
	//���������ָʾ�Ƿ�Ϊƴ����
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
    //XXX �������ƴ�������
    //��Ҳû�б�˳�򲻶�
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
