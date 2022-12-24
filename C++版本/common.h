#ifndef __COMMON_H__
#define __COMMON_H__

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#define LOGIN 1
#define QUERY 2
#define MAXBUFSIZE 1024
std::string toLowerCase(std::string str);
std::string trim(std::string str);
void SetSockAddr(struct sockaddr_in* addr, const char* inetAddr, short port);
std::vector<std::string> StringSplit(const char* s, size_t maxsize, char delim);
int pack(char* buf, int maxsize, int type, std::vector<std::pair<std::string, std::string>> datas);
int unpack(const char* buf, size_t length, std::vector < std::pair<std::string, std::string>>& datas);

#endif
