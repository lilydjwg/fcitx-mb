#include<string>
#include<vector>
#include<algorithm>
#include<cstdlib>
#include<cstring>
#include<fstream>
#include<sstream>
using namespace std;

#ifndef MAIN_HEADER
#define MAIN_HEADER

#define SYNTAX_ERROR	1
#define FILE_ERROR	2

#define STR_MAX		100

class RECORD {
    public:
	string		 code;				//编码
	string		 hz;				//汉字
	unsigned int	 hit;
	unsigned int	 index;
	bool             ispy;                          //拼音指示

			 RECORD(string, string);	//构造函数
			 RECORD();			//构造函数
	string		 toString();
};

class TABLE {
    private:
	char		 version;			//版本号
	string		 code;				//键码
	unsigned char	 codeLen;			//码长
	unsigned char	 pyLen;				//拼音长度
	string		 notCode;			//规避字符
	vector<string>	 wordsRule;			//组词规则
	vector<RECORD>	 records;			//数据

    public:
	int		ins(RECORD);			//插入数据
	int		del(RECORD);			//删除数据
	vector<string>	search_hz(string);		//搜索汉字
	vector<RECORD>	search_code(string,string="");	//搜索编码

	unsigned int	loadTable(char *fname);		//载入码表
	void		print();			//输出码表
	void		vprint();			//输出码表的全部信息
	void		write(string);			//将码表写到文件
	int		vwrite(string);			//将码表写到文件（保留词频信息）

	int		getSize();			//词的条数
	bool		checkCode(string);		//检查编码的合法性
	bool		isSorted();
	void		sortTable();				//排序
};

bool operator<(const RECORD&, const RECORD&);
#endif

