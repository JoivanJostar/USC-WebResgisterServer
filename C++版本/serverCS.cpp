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
short UDP_serverCS_port = 22888;
struct sockaddr_in udp_serverM_addr;
struct sockaddr_in udp_serverCS_addr;
map<string, map<string,string> > LoadCourseInfo(string filename) {
	//Credit / Professor / Days / CourseName
	map<string, map<string, string> >   account;
	ifstream infile(filename);
	if (!infile.is_open()) {
		perror("unable to open dept file");
		return account;
	}
	string courseCode = "";
	string credit = "";
	string professor = "";
	string days = "";
	string courseName = "";
	while (!infile.fail()) {
		map<string, string> info;
		getline(infile, courseCode, ',');
		getline(infile, credit, ',');
		getline(infile, professor, ',');
		getline(infile, days, ',');
		getline(infile, courseName, '\n');
		if (infile.fail())
			continue;
		info["Credit"] = trim(credit);
		info["Professor"] = trim(professor);
		info["Days"] = trim(days);
		info["Course Name"] = trim(courseName);
		account[courseCode] = info;
	}
	infile.close();
	return account;
}
int main() {
	map<string, map<string,string>> courseTable = LoadCourseInfo("cs.txt");
	cout << "The ServerCS is up and running using UDP on port " << UDP_serverCS_port << "." << endl;
	char on = 1;
	SetSockAddr(&udp_serverM_addr, "127.0.0.1", UDP_serverM_port);
	SetSockAddr(&udp_serverCS_addr, "127.0.0.1", UDP_serverCS_port);
	int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_sock == -1) {
		perror("create socket error");
		return -1;
	}
	//set the socket reusable for quick re-start 
	setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (bind(udp_sock, (struct sockaddr*)&udp_serverCS_addr, sizeof(struct sockaddr_in)) < 0) {
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
		vector<pair<string, string>> datas;
		unpack(buf, nbytes, datas);

		if (datas.size() == 1 && datas[0].second != "ALL") {
			string courseCode = datas[0].first;
			string category = datas[0].second;
			map<string, string> info = courseTable[courseCode];
			cout << "The ServerCS received a request from the Main Server about the " << toLowerCase(category) << " of " << courseCode << ". ";
			if (info.empty()) {
				memset(buf, 0, MAXBUFSIZE);
				sprintf(buf, "Didn't find the course: %s.\n", courseCode.c_str());
				cout << "Didn't find the course: " << courseCode << ". \n";
				sendto(udp_sock, buf, strlen(buf), 0, (sockaddr*)&udp_serverM_addr, sizeof(udp_serverM_addr));
			}
			else {
				memset(buf, 0, MAXBUFSIZE);
				cout << "The course information has been found: The " << toLowerCase( category) << " of " << courseCode << " is " << info[category]<<"."<<endl;
				sprintf(buf, "The %s of %s is %s.\n", toLowerCase(category).c_str(), courseCode.c_str(), info[category].c_str());
				sendto(udp_sock, buf, strlen(buf), 0, (sockaddr*)&udp_serverM_addr, sizeof(udp_serverM_addr));
			}
		}
		else {
			//query list
			//cout << "The ServerCS received a request from the Main Server about the";
			//for (size_t i = 0; i < datas.size(); ++i) {
			//	string courseCode=datas[i].first;
			//	cout << " " << courseCode;
			//}
			//cout << "." << endl;
			memset(buf, 0, MAXBUFSIZE);
			string msg = "";
			for (size_t i = 0; i < datas.size(); ++i) {
				string courseCode = datas[i].first;

				map<string, string> info = courseTable[courseCode];
				if (info.empty()) {
					string item = "Didn't find the course: " + courseCode + ".\n";
					//cout << "Didn't find the course: " << courseCode << "." << endl;
					msg += item;
				}
				else {
					string item = courseCode + ": " + info["Credit"] + ", " + info["Professor"] + ", " + info["Days"] + ", " + info["Course Name"];
					//cout << "The course information has been found: " << item<<"."<<endl;
					item += "\n";
					msg += item;
				}

			}
			memcpy(buf, msg.data(), msg.size());
			sendto(udp_sock,buf,msg.size(),0,(sockaddr*)&udp_serverM_addr, sizeof(udp_serverM_addr));
			continue;
		}
		cout << "The ServerCS finished sending the response to the Main Server." << endl;
	}



}

