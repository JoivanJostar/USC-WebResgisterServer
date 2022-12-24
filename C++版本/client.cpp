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


//global var:
struct sockaddr_in server_addr;
struct sockaddr_in my_addr;
short TCP_serverM_port = 25555;
unsigned short my_port = 0;


string handleResult(char* buf, int len, vector<pair<string, string> > datas) {
	if (len <= 0)
		return "\n";
	string copy(buf, buf + len);
	istringstream iss(copy);
	string result;
	string table_head;
	getline(iss, table_head, '\n');
	result += table_head + "\n";
	string table_item;
	vector<string> table_contents;
	while (!iss.fail()) {
		getline(iss, table_item, '\n');
		if (iss.fail())
			continue;
		table_contents.push_back(table_item);
	}
	for (size_t i = 0; i < datas.size(); ++i) {
		string courseCode = datas[i].first;
		//for()
		for (size_t j = 0; j < table_contents.size(); ++j) {
			if (table_contents[j].find(courseCode, 0) != string::npos) {
				result += table_contents[j] + "\n";
				break;
			}
		}
	}
	return result;
	
}

int main() {

	cout << "The client is up and running." << endl;
	char buf[MAXBUFSIZE] = { 0 };
	int nbytes = 0;
	SetSockAddr(&server_addr, "127.0.0.1", TCP_serverM_port);
	int times = 3;
	string username, password;
	while(times-->0){

		cout << "Please enter the username: ";
		cin >> username;
		cout << "Please enter the password: ";
		cin >> password;
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			perror("create socket error");
			return -1;
		}
		if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
			perror("fail to connect serverM");
			return -1;
		}
		socklen_t addrlen=sizeof(my_addr);
		int getsock_check = getsockname(sockfd, (struct sockaddr*)&my_addr, &addrlen);
		//Error checking
		if (getsock_check == -1) {
			perror("getsockname");
			exit(1);
		}
		my_port = ntohs(my_addr.sin_port);
		//send the username
		vector<pair<string, string>> datas;
		datas.push_back(pair<string, string>(username, password));
		int length = pack(buf, MAXBUFSIZE, LOGIN, datas);

		if ((nbytes = send(sockfd, buf, length, 0)) == -1) {
			perror("send authentication infomation error");
			return -1;
		}
		cout << username << " sent an authentication request to the main server." << endl;
		if ((nbytes = recv(sockfd, buf, MAXBUFSIZE, 0)) <= 0) {
			perror("recv authentication infomation error");
			return -1;
		}
		buf[nbytes] = 0;
		if (0 == strcmp(buf, "PASS")) {
			cout << username << " received the result of authentication using TCP over port " << my_port << ".Authentication is successful" << endl;
			close(sockfd);
			break;
		}
		else if (0 == strcmp(buf, "FAIL_NO_USER")) {
			//<username> received the result of authentication using TCP over port <port number>.Authentication failed : Username Does not exist
				//Attempts remaining : <n>
			cout << username << " received the result of authentication using TCP over port " << my_port
				<< ". Authentication failed: Username Does not exist" << endl;
			
		}else if(0 == strcmp(buf, "FAIL_PASS_NO_MATCH")){
			cout << username << " received the result of authentication using TCP over port " << my_port
				<< ". Authentication failed: Password does not match" << endl;
		}
		cout << "Attempts remaining:" << times << endl;
		close(sockfd);
	}

	if (times <= 0) {
		cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
		return 1;
	}

	//

	getchar();//read the remaining \n

	while (true) {
		string courseCode;
		string category;
		string line;
	
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			perror("create socket error");
			return -1;
		}
		if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
			perror("fail to connect serverM");
			return -1;
		}
		socklen_t addrlen = sizeof(my_addr);
		int getsock_check = getsockname(sockfd, (struct sockaddr*)&my_addr, &addrlen);
		//Error checking
		if (getsock_check == -1) {
			perror("getsockname");
			exit(1);
		}
		my_port = ntohs(my_addr.sin_port);

		cout << "Please enter the course code to query: ";
		line.clear();
		while (line.empty()) {
			getline(cin, line, '\n');
		}
	
		vector<string> strings=StringSplit(line.c_str(), line.size(), ' ');
		vector<pair<string, string>> datas;
		datas.push_back(pair<string, string>(username, "N/A"));

		if (strings.size() > 1) {
	
			for (size_t i = 0; i < strings.size(); ++i) {
				datas.push_back(pair<string, string>(strings[i], "ALL"));
			}
			int length = pack(buf, MAXBUFSIZE, QUERY, datas);
			send(sockfd, buf, length, 0);
			cout << username << " sent a request with multiple CourseCode to the main server." << endl;
			memset(buf, 0, MAXBUFSIZE);
			nbytes = recv(sockfd, buf, MAXBUFSIZE, 0);
			buf[nbytes] = 0;
			string result = handleResult(buf, nbytes, datas);
			cout << result;
		}
		else {
			courseCode = strings[0];
			cout << "Please enter the category (Credit / Professor / Days / CourseName): ";
			cin >> category;
			if (category == "CourseName")
				category = "Course Name";
			getchar();//read the remaining '\n'
			datas.push_back(pair<string, string>(courseCode, category));
			int length = pack(buf, MAXBUFSIZE, QUERY, datas);
			send(sockfd, buf, length, 0);
			cout << username << " sent a request to the main server." << endl;
			memset(buf, 0, MAXBUFSIZE);
			nbytes=recv(sockfd, buf, MAXBUFSIZE, 0);
			buf[nbytes] = 0;
			cout << "The client received the response from the Main server using TCP over port " << my_port << ". ";
			cout << buf;
		}
		close(sockfd);
		cout << "-----Start a new request-----" << endl;
	}

}
