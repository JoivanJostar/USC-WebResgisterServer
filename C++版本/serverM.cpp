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
#include "common.h"
using namespace std;


string encrypt(string str);
//use 24666 for UDP, use 25555 for TCP 
short UDP_serverM_port = 24666;
short TCP_serverM_port = 25555;
short UDP_serverC_port = 21999;
short UDP_serverCS_port = 22888;
short UDP_serverEE_port = 23777;
struct sockaddr_in tcp_addr;
struct sockaddr_in udp_addr;
struct sockaddr_in udp_serverC_addr;
struct sockaddr_in udp_serverCS_addr;
struct sockaddr_in udp_serverEE_addr;

string encrypt(string str) {
	string copy = str;

	for (size_t i = 0; i < copy.size(); ++i) {
		if ('0' <= copy[i] && '9' >= copy[i]) {
			copy[i] = '0' + (copy[i] - '0' + 4) % 10;
		}
		else if ('a' <= copy[i] && 'z' >= copy[i]) {
			copy[i] = 'a' + (copy[i] - 'a' + 4) % 26;
		}
		else if ('A' <= copy[i] && 'Z' >= copy[i]) {
			copy[i] = 'A' + (copy[i] - 'A' + 4) % 26;
		}
	}
	return copy;
}


void ServiceLogin(int sockfd,vector<pair<string,string> > datas,int udp_sock) {
	char* buf = new char[MAXBUFSIZE];
	int nbytes = 0;
	string username = datas[0].first;
	string password = datas[0].second;
	cout << "The main server received the authentication for " 
		<< username << " using TCP over port "<<TCP_serverM_port <<"." << endl;
	//cout << "DEBUG:username is " << username << " password is " << password << endl;
	string encrypted_username = encrypt(username);
	string encrypted_password = encrypt(password);
	//cout << "DEBUG:encrypted_username is " << encrypted_username
	//	<< " encrypted_password is " << encrypted_password << endl;
	datas.clear();
	datas.push_back(pair<string, string>(encrypted_username, encrypted_password));
	int length = pack(buf, MAXBUFSIZE,LOGIN,datas);

	sendto(udp_sock, buf, length, 0, (struct sockaddr*)&udp_serverC_addr, sizeof(udp_serverC_addr));
	cout << "The main server sent an authentication request to serverC." << endl;
	struct sockaddr_in from_where;
	socklen_t addr_len = sizeof(from_where);
	memset(&from_where, 0, sizeof(from_where));
	memset(buf, 0, MAXBUFSIZE);
	nbytes = recvfrom(udp_sock, buf, MAXBUFSIZE, 0, (struct sockaddr*)&from_where, &addr_len);
	buf[nbytes] = '\0';
	cout << "The main server received the result of the authentication request from ServerC using UDP over port " 
		<< UDP_serverM_port << "." << endl;

	if (0 == strcmp(buf, "FAIL_NO_USER") || 0 == strcmp(buf, "FAIL_PASS_NO_MATCH")
		|| 0 == strcmp(buf, "PASS")) {
		nbytes = send(sockfd, buf, strlen(buf), 0);
	}
	else {
		perror("Unknown msg from ServerC:");
		perror(buf);
		nbytes = send(sockfd, "FAIL_NO_USER", strlen("FAIL_NO_USER"), 0);
	}
	cout << "The main server sent the authentication result to the client." << endl;
	close(sockfd);
	delete[] buf;
}
void ServiceQuery(int sockfd, vector<pair<string, string> > datas,int udp_sock) {
	char* buf = new char[MAXBUFSIZE];
	memset(buf, 0, MAXBUFSIZE);
	int nbytes = 0;
	string username = datas[0].first;
	string finalResult="";
	if (datas.size() > 2) {
		//query list
		finalResult = "CourseCode: Credits, Professor, Days, Course Name\n";
		//cout << "The main server received from " << username << " to query course ";
		//for (size_t i = 1; i < datas.size(); ++i) {
		//	string courseCode = datas[i].first;
		//	cout << courseCode << " ";
		//}
		//cout << "using TCP over port " << TCP_serverM_port << "." << endl;
	}
	else {
		finalResult = "";
		string courseCode = datas[1].first;
		string category = datas[1].second;
		cout << "The main server received from " << username << " to query course "
			<< courseCode << " about " << toLowerCase( category) << " using TCP over port " << TCP_serverM_port <<"."<< endl;
	}

	vector<pair<string, string> > querysForEE;
	vector<pair<string, string> > querysForCS;
	//The main server sent a request to server<EE or CS>.
	for (size_t i = 1; i < datas.size(); ++i) {
		string courseCode = datas[i].first;
		string depart(courseCode.begin(), courseCode.begin() + 2);
		//cout << "DEBUG:depart=" << depart << endl;
		if (depart == "EE") {
			querysForEE.push_back(datas[i]);
		}
		else if (depart == "CS") {
			querysForCS.push_back(datas[i]);
		}

	}
	string resultFromEE = "";
	if (!querysForEE.empty()) {
		int length = pack(buf, MAXBUFSIZE, QUERY, querysForEE);
		sendto(udp_sock, buf, length, 0, (sockaddr*)&udp_serverEE_addr, sizeof(udp_serverEE_addr));
		cout << "The main server sent a request to serverEE." << endl;
		struct sockaddr_in from_where;
		socklen_t addr_len = sizeof(from_where);
		memset(&from_where, 0, sizeof(from_where));
		memset(buf, 0, MAXBUFSIZE);
		nbytes = recvfrom(udp_sock, buf, MAXBUFSIZE, 0, (struct sockaddr*)&from_where, &addr_len);
		buf[nbytes] = 0;
		resultFromEE.assign(buf, buf + nbytes);
		cout<<"The main server received the response from serverEE using UDP over port "<<UDP_serverM_port<<"." << endl;
	}
	string resultFromCS = "";
	if (!querysForCS.empty()) {
		int length = pack(buf, MAXBUFSIZE, QUERY, querysForCS);
		sendto(udp_sock, buf, length, 0, (sockaddr*)&udp_serverCS_addr, sizeof(udp_serverCS_addr));
		cout << "The main server sent a request to serverCS." << endl;
		struct sockaddr_in from_where;
		socklen_t addr_len = sizeof(from_where);
		memset(&from_where, 0, sizeof(from_where));
		memset(buf, 0, MAXBUFSIZE);
		nbytes = recvfrom(udp_sock, buf, MAXBUFSIZE, 0, (struct sockaddr*)&from_where, &addr_len);
		buf[nbytes] = 0;
		resultFromCS.assign(buf, buf + nbytes);
		cout << "The main server received the response from serverCS using UDP over port " << UDP_serverM_port << "." << endl;
	}
	finalResult = finalResult+resultFromEE + resultFromCS;
	memset(buf, 0, MAXBUFSIZE);
	memcpy(buf, finalResult.data(), finalResult.size());
	send(sockfd, buf, finalResult.size(), 0);
	cout << "The main server sent the query information to the client."<< endl;
	close(sockfd);
	delete[] buf;

}
int main() {

	char on = 1;
	cout << "The main server is up and running." << endl;

	SetSockAddr(&tcp_addr, "127.0.0.1", TCP_serverM_port);
	SetSockAddr(&udp_addr, "127.0.0.1", UDP_serverM_port);
	SetSockAddr(&udp_serverC_addr, "127.0.0.1", UDP_serverC_port);
	SetSockAddr(&udp_serverCS_addr, "127.0.0.1", UDP_serverCS_port);
	SetSockAddr(&udp_serverEE_addr, "127.0.0.1", UDP_serverEE_port);
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	int udp_sock= socket(AF_INET, SOCK_DGRAM, 0);
	if (listen_sock == -1 || udp_sock==-1) {
		perror("create socket error");
		return -1;
	}
	//set the socket reusable for quick re-start 
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (bind(listen_sock, (struct sockaddr*)&tcp_addr, sizeof(struct sockaddr_in)) < 0
		|| bind(udp_sock, (struct sockaddr*)&udp_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("fail to bind");
		return -1;
	}
	if (listen(listen_sock, 10) < 0) {
		perror("fail to listen\n");
		return -1;
	}

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t addrlen = sizeof(client_addr);
	char buf[MAXBUFSIZE] = { 0 };
	int nbytes = 0;
	vector<pair<string, string>> datas;
	while (true) {
		int sockfd = accept(listen_sock, (struct sockaddr*)&client_addr, &addrlen);
		datas.clear();

		if ((nbytes = recv(sockfd, buf, MAXBUFSIZE, 0)) <= 0) {
			//perror("client closed");
			close(sockfd);
			continue;
		}
		buf[nbytes] = '\0';
		int type = unpack(buf, nbytes, datas);
		switch (type)
		{
		case LOGIN:
			ServiceLogin(sockfd, datas,udp_sock);
			break;
		case QUERY:
			ServiceQuery(sockfd, datas, udp_sock);
			break;
		default:
			break;
		}

	}

}


