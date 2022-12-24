#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include "common.h"
using namespace std;
//这个是把一个字符串里的大写字母转为小写 返回的就是小写格式的字符串
string toLowerCase(string str) {
	string result = str;
	for (size_t i = 0; i < result.size(); ++i) {
		if ('A' <= result[i] && 'Z' >= result[i]) {
			result[i] = 'a' + result[i] - 'A';
		}
	}
	return result;
}

//这个是对一个字符串去掉末尾的 '\r' ，因为你发我的文本文件我发现是CRLF,按理说Linux是LF，这个函数
//就是为了应对CRLF的情况。因为CRLF的换行符是'\r''\n' LF的是'\n'。
std::string trim(std::string str){
	string copy=str;
	if(copy[copy.size()-1]=='\r')
		copy.pop_back();
	return copy;
}

//这个是填充Socket地址，因为所有cpp里都要大量使用这个功能，所以我就抽取出来了
void SetSockAddr(struct sockaddr_in* addr, const char* inetAddr, short port) {
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
   	inet_aton(inetAddr,&(addr->sin_addr));
	//addr->sin_addr.S_un.S_addr = inet_addr(inetAddr);

}
//这个是分割字符串,split，因为C++自带的API里面没有这个，我自己实现了一个
//s是待分割的字符串，maxsize是s的最大长度，delim是要用哪个字符为依据进行分割
//返回值就是分割后的结果
//例如 "abc de f" 按照delim=' '空格去分割，结果就是三个字符串 "abc" "de" "f"
vector<string> StringSplit(const char* s, size_t maxsize, char delim) {
	istringstream iss(string(s, s + maxsize));
	string item;
	vector<string> elems;
	while (getline(iss, item, delim)) {
		if (!item.empty()) {
			elems.push_back(item);
		}
	}
	return elems;
}

//这个是对datas数据进行打包，填充到buf缓冲区,用于将来发送。返回值是填充后的报文长度
int pack(char* buf, int maxsize, int type, vector<pair<string, string>> datas) {

	char* p = buf;
	string head;
	int length = 8;
	if (LOGIN == type) {
		head = "LOGIN";
	}
	else if (QUERY == type)
	{
		head = "QUERY";
	}
	else {
		return 0;
	}
	memset(buf, 0, maxsize);
	sprintf(p, "%s", head.c_str());
	p = buf + length;
	//8bytes:HEAD | username\0password\0 | coursecode\0category\0coursecode\0category\0
	string msg = "";
	for (size_t i = 0; i < datas.size(); ++i) {
		msg = msg + datas[i].first;
		msg.append(1, '\0');
		msg = msg + datas[i].second;
		msg.append(1, '\0');
	}
	memcpy(p, msg.data(), msg.size());

	length += msg.size();
	return length;
}
//这个是对报文进行解包，解包结果存在datas里面,返回值是服务类型，用于Server去判断是何种服务
int unpack(const char* buf, size_t length, vector < pair<string, string>>& datas) {
	datas.clear();
	const char* p = buf;
	char head[8] = { 0 };
	int type = 0;
	sscanf(buf, "%s", head);
	if (0 == strcmp(head, "LOGIN"))
		type = LOGIN;
	else if (0 == strcmp(head, "QUERY"))
		type = QUERY;
	else
		return 0;

	p += 8;
	vector<string> strings = StringSplit(p, length - 8, '\0');
	if (strings.size() % 2 != 0)
		return -1;
	if (strings.size() < 2)
		return -1;
	for (size_t i = 0; i < strings.size(); i += 2) {
		pair<string, string> unit(strings[i], strings[i + 1]);
		datas.push_back(unit);
	}
	return type;
}



