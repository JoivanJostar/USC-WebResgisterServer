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


short UDP_serverM_port = 24666;
short UDP_serverC_port = 21999;

struct sockaddr_in udp_serverM_addr;
struct sockaddr_in udp_serverC_addr;

map<string,string> LoadAccountInfo(string filename) {
	map<string,string> account;
	ifstream infile(filename);
	if (!infile.is_open()) {
		perror("unable to open file");
		return account;
	}
	string username = "";
	string password = "";
	while (!infile.fail()) {
		getline(infile, username, ',');
		getline(infile, password, '\n');
		if (infile.fail())
			continue;
		account[username] = trim(password);
	}
	infile.close();
	return account;
}
int main() {
	map<string,string> account=LoadAccountInfo("cred.txt");
	cout << "The ServerC is up and running using UDP on port " << UDP_serverC_port << "." << endl;
	char on = 1;
	SetSockAddr(&udp_serverM_addr, "127.0.0.1", UDP_serverM_port);
	SetSockAddr(&udp_serverC_addr, "127.0.0.1", UDP_serverC_port);
	int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_sock == -1) {
		perror("create socket error");
		return -1;
	}
	//set the socket reusable for quick re-start 
	setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (bind(udp_sock, (struct sockaddr*)&udp_serverC_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("fail to bind");
		return -1;
	}
	char buf[MAXBUFSIZE] = { 0 };
	int nbytes = 0;
	while (true) {
		struct sockaddr_in from_where;
		socklen_t addr_len = sizeof(from_where);
		memset(&from_where, 0, sizeof(from_where));
		memset(buf, 0, MAXBUFSIZE);
		nbytes = recvfrom(udp_sock, buf, MAXBUFSIZE, 0, (struct sockaddr*)&from_where, &addr_len);
		cout << "The ServerC received an authentication request from the Main Server. ";
		vector<pair<string, string>> datas;
		unpack(buf, nbytes, datas);
		string username = datas[0].first;
		string password = datas[0].second;
		if (account[username] == "") {
			sendto(udp_sock, "FAIL_NO_USER", strlen("FAIL_NO_USER"), 0, (sockaddr*)&udp_serverM_addr, sizeof(udp_serverM_addr));
		}
		else {
			if(password!=account[username]){
				sendto(udp_sock, "FAIL_PASS_NO_MATCH", strlen("FAIL_PASS_NO_MATCH"), 0, (sockaddr*)&udp_serverM_addr, sizeof(udp_serverM_addr));
				}

			else{
					sendto(udp_sock, "PASS", strlen("PASS"), 0, (sockaddr*)&udp_serverM_addr, sizeof(udp_serverM_addr));
				}
				
		}
		cout << "The ServerC finished sending the response to the Main Server." << endl;
	}



}

