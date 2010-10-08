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

#include<iostream>
#include<cstring>
#include<cstdlib>
#include<fstream>
#include<sstream>
#include"Iconv/Iconv.h"
#include"mb.h"

#define FILE_ERROR	2

using namespace std;

void error(int);//错误处理
void do_search(string);
void do_range(string from, string to);
void do_vfind(string);
void do_set(string, string, unsigned int, unsigned int);
void do_ins(string, string);
void do_delete(string);
void do_strict_delete(string, string);
void do_delcode(string);
void do_help();
void do_version();
void do_write(string="");
void do_vwrite(string="");
//备份，文件名为原文件名后加 ~
//  目前仅在写入到 argv[1] 时调用
void do_backup(string);
string g2u(string);
string u2g(string);

TABLE mbTable;
string mbName = "";

int main (int argc, char *argv[]) {

  if (argc != 2 && argc != 3) {
    fprintf (stderr, "\nUsage: %s <Source File>\n\n", argv[0]);
    exit(SYNTAX_ERROR);
  }

  if(argc == 3){
    if(!strcmp(argv[2], "--version") || !strcmp(argv[2], "-V")){
      do_version();
      exit(0);
    }
  }

  mbName = argv[1];
  unsigned int status;

  if(status = mbTable.loadTable (argv[1]) ){
    exit(status);
  }

  if(!mbTable.isSorted())
    cout<<"注意：码表顺序有误！保存前将会排序。"<<endl;

  //不进入交互状态执行命令
  // -p: 打印输出（与 mb2txt 一致）
  // -v: 打印输出（包括 hit 和 index）
  if(argc == 3){
    if(!strcmp(argv[2], "-p")){
      mbTable.print();
      exit(0);
    }else if(!strcmp(argv[2], "-v")){
      mbTable.vprint();
      exit(0);
    }else{
      fprintf (stderr, "\nUsage: %s <Source File>\n\n", argv[0]);
      exit(SYNTAX_ERROR);
    }
  }

  //命令循环
  string cmdline, cmd, cmd2;
  unsigned int uint, uint2;
  cout<<"\t\t码表载入成功！请输入命令。help - 帮助；quit - 退出"<<endl;
  while(1){
    uint = uint2 = 0;//初始化，当set的参数不够时可用
    cout<<"mb> ";
    // 这样，只要到达 EOF 就会终止，不论输入是来自文件或者遇到了 ^D。
    if(!getline(cin, cmdline)) break;
    // getline(cin, cmdline);
    istringstream cmdstream(cmdline);
    if(!(cmdstream>>cmd))//没有输入
      continue;
    if(cmd == "quit" ||
	cmd == "exit" ||
	cmd == "q"){
      cout<<"谢谢使用！"<<endl;
      exit(0);
    }
    else if(cmd == "find" ||
	cmd == "f" ||
	cmd == "search" ||
	cmd == "lookup")
      while(cmdstream>>cmd){
	do_search(cmd);
      }
    else if(cmd == "range"){
      cmdstream>>cmd;
      cmdstream>>cmd2;
      do_range(cmd, cmd2);
    }
    else if(cmd == "insert" ||
	cmd == "ins" ||
	cmd == "add" ||
	cmd == "a" )
      for(;cmdstream>>cmd;){
	if(cmdstream>>cmd2)
	  do_ins(cmd, cmd2);
	else
	  error(SYNTAX_ERROR);
      }
    else if(cmd == "delete" ||
	cmd == "del" ||
	cmd == "d")
      while(cmdstream>>cmd)
	do_delete(cmd);
    else if(cmd == "strictdel" ||
	cmd == "sdel" ||
	cmd == "sd")
      for(;cmdstream>>cmd;){
	if(cmdstream>>cmd2)
	  do_strict_delete(cmd, cmd2);
	else
	  error(SYNTAX_ERROR);
      }
    else if(cmd == "write" ||
	cmd == "w" ||
	cmd == "save"){
      if(cmdstream>>cmd)
	do_write(cmd);
      else
	//这样就不要写回原文件了
	error(SYNTAX_ERROR);
    }
    else if(cmd == "vwrite" ||
	cmd == "vw" ||
	cmd == "vsave"){
      if(cmdstream>>cmd)
	do_vwrite(cmd);
      else{
	do_backup(argv[1]);
	do_vwrite();
      }
    }
    else if(cmd == "wq"){
      do_backup(argv[1]);
      do_vwrite();
      cout<<"谢谢使用！"<<endl;
      exit(0);
    }
    else if(cmd == "help" ||
	cmd == "h" ||
	cmd == "usage")
      do_help();
    else if(cmd == "ver")
      do_version();
    else if(cmd == "print"){
      mbTable.print();
      exit(0);
    }
    else if(cmd == "verbose" ||
	cmd == "v")
      while(cmdstream>>cmd)
	do_vfind(cmd);
    else if(cmd == "set"){
      cmdstream>>cmd;
      cmdstream>>cmd2;
      cmdstream>>uint;
      cmdstream>>uint2;
      do_set(cmd, cmd2, uint, uint2);
    }
    else if(cmd == "delcode" ||
	cmd == "dc")
      while(cmdstream>>cmd)
	do_delcode(cmd);
    else if(cmd == "size")
      cout<<"共有词组 "<<mbTable.getSize()<<" 个。"<<endl;
    else if(cmd == "#") //注释
      continue;
    else
      error(SYNTAX_ERROR);
  }

  return 0;
}

void do_search(string tgt){
  if(mbTable.checkCode(tgt)){//查找编码对应的词
    vector<RECORD> results;
    results = mbTable.search_code(tgt);
    cout<<tgt<<"\t\t";
    if(results.size() > 0){
      for(int i = 0; i < results.size()-1; i++)
	cout<<g2u(results[i].hz)<<", ";
      cout<<g2u(results[results.size()-1].hz);
      cout<<endl;
    }else{
      cout<<"(没有符合的结果)"<<endl;
    }
  }else{
    vector<string> results;
    results = mbTable.search_hz(u2g(tgt));
    cout<<tgt<<"\t\t";
    if(results.size() > 0){
      for(int i = 0; i < results.size()-1; i++)
	cout<<g2u(results[i])<<", ";
      cout<<g2u(results[results.size()-1]);
      cout<<endl;
    }else{
      cout<<"(没有符合的结果)"<<endl;
    }
  }
}

void do_range(string from, string to){
  vector<RECORD> results;
  results = mbTable.search_code(from, to);
  if(results.size() > 0){
    string curCode = results[0].code;
    cout<<curCode<<"\t\t";
    for(int i=0; i < results.size(); i++){
      if(curCode != results[i].code){
	cout<<endl;
	curCode = results[i].code;
	cout<<curCode<<"\t\t";
      }else if(i)
	cout<<", ";
      cout<<g2u(results[i].hz);
    }
    cout<<endl;
  }else{
    cout<<"(没有符合的结果)"<<endl;
  }
}

void do_vfind(string code){
  vector<RECORD> results;
  if(mbTable.checkCode(code)){
    results = mbTable.search_code(code);
    cout<<code<<":"<<endl;
    if(results.size() > 0){
      for(int i=0; i < results.size(); i++){
	cout<<"\t"<<g2u(results[i].hz);
	cout<<"\t"<<results[i].hit;
	cout<<"\t"<<results[i].index;
	if(results[i].ispy)
	  cout<<"\t"<<"ispy";
	cout<<endl;
      }
    }else{
      cout<<"\t(无此编码)"<<endl;
    }
  }else
    cout<<"（不正确的编码！）"<<endl;
}

void do_ins(string code, string hz){
  if(!mbTable.checkCode(code)){
    cout<<"（不正确的编码！）"<<endl;
    return;
  }
  RECORD newRecord(code, u2g(hz));
  int status = mbTable.ins(newRecord);
  if(!status)
    cout<<"新词组 "<<g2u(newRecord.toString())<<" 已加入！"<<endl;
  else
    cout<<"词组 "<<g2u(newRecord.toString())<<" 已经存在了！"<<endl;
}

void do_set(string code, string hz, unsigned int hit, unsigned int index){
  vector<RECORD> results;
  results = mbTable.search_code(code);
  if(results.size() > 0){
    for(int i=0; i < results.size(); i++){
      if(results[i].hz == u2g(hz)){//已经存在了
	mbTable.del(results[i]);
      }
    }
  }

  RECORD newRecord(code, u2g(hz));
  newRecord.hit = hit;
  newRecord.index = index;
  mbTable.ins(newRecord);
  cout<<"词组的信息 "<<g2u(newRecord.toString())<<" 已被设置！"<<endl;
}

void do_delete(string tgt){
  int count = 0;//计数
  vector<string> results;
  results = mbTable.search_hz(u2g(tgt));
  for(int i = 0; i < results.size(); i++){
    RECORD oldRecord(results[i], u2g(tgt));
    mbTable.del(oldRecord);
    count++;
    cout<<"词组 "<<g2u(oldRecord.toString())<<" 已删除！"<<endl;
  }
  cout<<"共删除了 "<<count<<" 个数据。"<<endl;
}

void do_delcode(string code){
  if(!mbTable.checkCode(code))
    cout<<"（不正确的编码！）"<<endl;
  else{
    int count = 0;//计数
    vector<RECORD> results;
    results = mbTable.search_code(code);
    for(int i = 0; i < results.size(); i++){
      mbTable.del(results[i]);
      count++;
      cout<<"词组 "<<g2u(results[i].toString())<<" 已删除！"<<endl;
    }
    cout<<"共删除了 "<<count<<" 个数据。"<<endl;
  }
}

void do_strict_delete(string code, string hz){
  RECORD oldRecord(code, u2g(hz));
  if(mbTable.del(oldRecord))
    cout<<"词组 "<<g2u(oldRecord.toString())<<" 已删除！"<<endl;
  else
    cout<<"没有词组被删除！"<<endl;
}

void do_write(string fname){
  if(fname == "") fname = mbName;
  mbTable.write(fname);
}

void do_vwrite(string fname){
  //虽然问题似乎已经解决了，但保险起见还是把这个留着吧
  if(!mbTable.isSorted()){
      cout<<"警告：码表顺序有误！现在将先排序再保存。"<<endl;
      mbTable.sortTable();
  }

  if(fname == "") fname = mbName;
  int status = mbTable.vwrite(fname);
  if(status<0)
    error(status);
  else
    cout<<status<<" 词已保存！"<<endl;
}

void do_backup(string fname){
  string cmd = "cp ";
  cmd += fname + " " + fname + "~";
  system(cmd.c_str());
  cout<<"原文件已备份！"<<endl;
}

void do_help(){
  cout<<"语法说明："<<endl;
  cout<<"\thelp, h, usage\t\t显示本说明"<<endl;
  cout<<"\tver\t\t\t显示版本信息"<<endl;
  cout<<endl;
  cout<<"\tfind, f, search, lookup\t查找编码对应的词或词对应的编码"<<endl;
  cout<<"\tverbose, v\t\t查看编码的详细信息"<<endl;
  cout<<"\trange\t\t\t查看介于其两个参数之间的词组"<<endl;
  cout<<"\tsize\t\t\t查看词条的数目"<<endl;
  cout<<endl;
  cout<<"\tinsert, ins, add, a\t加入新词。格式：编码 新词 [编码 新词]..."<<endl;
  cout<<"\tset\t\t\t添加/修改词条：编码 词组 hit index"<<endl;
  cout<<"\tdelete, d, del\t\t删除词组，参数为词"<<endl;
  cout<<"\tstrictdel, sd, sdel\t删除[编码:词组]对，格式同 ins"<<endl;
  cout<<"\tdelcode, dc\t\t删除编码，参数为编码"<<endl;
  cout<<endl;
  cout<<"\twrite, w, save\t\t保存到文件（无词频信息）"<<endl;
  cout<<"\tvwrite, vw, vsave\t保存到文件（有词频信息），默认为原文件"<<endl;
  cout<<"\twq\t\t\t保存并退出"<<endl;
  cout<<"\tprint\t\t\t输出并退出（同mb2txt程序）"<<endl;
  cout<<"\tquit, q, exit\t\t退出本程序"<<endl;
}

void do_version(){
  cout<<"\t\t小企鹅输入法码表操作工具 v0.7.20091115"<<endl;
}

void error(int Err){
  switch(Err){
    case SYNTAX_ERROR:
      cerr<<"语法错误！输入 help 获取更多信息。"<<endl;
      break;
    case FILE_ERROR:
      cerr<<"写入到文件时出错！"<<endl;
      perror("详细信息");
      break;
  }
}


string g2u(string gb){
  Iconv converter("gb18030", "utf-8");
  return converter(gb.c_str(), gb.length());
  // fcitx-utf-8 不需要这些了
  // return gb;
}

string u2g(string unicode){
  // 用 gb18030 才能转换∀等字符，GBK 不行
  Iconv converter("utf-8", "gb18030");
  return converter(unicode.c_str(), unicode.length());
  // return unicode;
}

// vim:sw=2:fdm=expr:fde=getline(v\:lnum+1)=~'\\v^([A-Za-z]|/)+'?0\:1
