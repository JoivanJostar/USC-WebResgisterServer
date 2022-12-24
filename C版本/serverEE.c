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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXSIZE 10240
#define MAXSTRING 100
#define MAXDATASIZE 100
#define TCP 1
#define UDP 2

void fill_sock_addr(struct sockaddr_in* sock_addr, short port);
int split(char* src, int src_size, char dst[][MAXSTRING], char delim);
void clear_data(char data[][MAXSTRING]);
int data2msg(char* msg, char data[][MAXSTRING], int size);
int msg2data(char* msg, int msg_len, char data[][MAXSTRING]);
int my_send(int protol, char* send_buffer, char data[][MAXSTRING], int data_size, int socket, struct sockaddr_in* dst);
int my_recv(int protol, char* recv_buffer, char data[][MAXSTRING], int socket);

char g_send_buffer[MAXSIZE];
char g_recv_buffer[MAXSIZE];
char g_data[MAXDATASIZE][MAXSTRING] = { 0 };

unsigned short serverM_udp_port = 24666;
unsigned short serverEE_udp_port = 23777;
struct sockaddr_in serverM_udp_addr;
struct sockaddr_in serverEE_udp_addr;
char * strlwr(char * str);
void query_course(char * course_code,char * category,char * result);
char * strlwr(char * str){
	int i=0;
	while(str[i]){
		if('A'<=str[i]&&str[i]<='Z')
			str[i]='a'+ str[i]-'A';
		i++;
	}
	return str;
}

void query_course(char* course_code, char* category, char* result) {
	char code[100] = { 0 };
	char credit[100] = { 0 };
	char professor[100] = { 0 };
	char days[100] = { 0 };
	char name[100] = { 0 };
	char value[100] = { 0 };
	FILE* info = fopen("ee.txt", "r");
	if (info == NULL) {
		perror("open ee.txt");
		return ;
	}
	char line[200] = { 0 };
	int flag = 0;
	while (fgets(line, 200, info)) {
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;
		if (line[strlen(line) - 1] == '\r')
			line[strlen(line) - 1] = 0;
		char splited[6][100];
		int num = split(line, strlen(line), splited, ',');
		if (num != 5) {
			perror("file context corrupt");
			return;
		}
		strcpy(code, splited[0]);
		strcpy(credit, splited[1]);
		strcpy(professor, splited[2]);
		strcpy(days, splited[3]);
		strcpy(name, splited[4]);
		if (strcmp(code, course_code) == 0) {

			flag = 1;
			strlwr(category);
			if (strcmp(category, "credit") == 0) {
				strcpy(value, credit);
			}
			else if (strcmp(category, "professor") == 0) {
				strcpy(value, professor);
			}
			else if (strcmp(category, "days") == 0) {
				strcpy(value, days);
			}
			else if (strcmp(category, "coursename") == 0) {
				strcpy(value, name);
			}
			else if (strcmp(category, "all_category") == 0) {
				sprintf(value, "%s: %s, %s, %s, %s\n", code, credit, professor, days, name);
			}
			else {
				value[0] = 0;
			}
			break;
		}
	}
	if (flag == 1) {
		if (strcmp(category, "all_category") != 0) {
			sprintf(result, "The %s of %s is %s.\n", strlwr(category), course_code, value);
			printf("The course information has been found: ");
			printf("%s", result);
		}
		else {
			strcpy(result, value);
		}
	}
	else {
		sprintf(result, "Didn't find the course: %s.\n", course_code);
		printf("%s", result);
	}
	fclose(info);
}


int main() {
	fill_sock_addr(&serverEE_udp_addr, serverEE_udp_port);
	fill_sock_addr(&serverM_udp_addr, serverM_udp_port);

	int serverEE_udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverEE_udp_sock == -1) {
		perror("can not create socket");
		return -1;
	}

	if (bind(serverEE_udp_sock, (struct sockaddr*)&serverEE_udp_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("can not bind port");
		return -1;
	}
	printf("The ServerEE is up and running using UDP on port %d.\n", serverEE_udp_port);

	while (1) {
		my_recv(UDP, g_recv_buffer, g_data, serverEE_udp_sock);

		char course_code[100] = { 0 };
		char category[100] = { 0 };
		char result[100] = { 0 };
		strcpy(course_code, g_data[0]);
		strcpy(category, g_data[1]);
		if (strcmp("ALL_CATEGORY", category) != 0) {
			printf("The ServerEE received a request from the Main Server about the %s of %s.", strlwr(category), course_code);
		}
		else {
			printf("The ServerEE received a request from the Main Server about the all information of %s.", course_code);
		}
		query_course(course_code, category, result);
		strcpy(g_data[0], result);
		my_send(UDP, g_send_buffer, g_data, 1, serverEE_udp_sock, &serverM_udp_addr);
		printf("The ServerEE finished sending the response to the Main Server.\n");
	}
}

void fill_sock_addr(struct sockaddr_in* sock_addr, short port) {
	memset(sock_addr, 0, sizeof(struct sockaddr_in));
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_port = htons(port);
	inet_aton("127.0.0.1", &(sock_addr->sin_addr));
}

int split(char* src, int src_size, char dst[][MAXSTRING], char delim) {
	int cnt = 0;
	int beg = 0;
	int i = 0;
	for (i = 0; i < src_size; ++i) {
		if (src[i] == delim) {
			src[i] = 0;
			if (i - beg > 0) {
				strcpy(dst[cnt++], src + beg);
			}
			beg = i + 1;
		}
	}
	if (i - beg > 0) {
		strcpy(dst[cnt++], src + beg);
	}
	return cnt;
}
void clear_data(char  data[][MAXSTRING]) {
	for (int i = 0; i < MAXDATASIZE; i++)
		memset(data[i], 0, MAXSTRING);
}
int data2msg(char* msg, char data[][MAXSTRING], int size) {
	char* begin = msg;
	memset(msg, 0, MAXSIZE);
	sprintf(msg, "%d", size);
	int length = 4;
	msg = begin + length;
	for (int i = 0; i < size; i++) {
		int str_size = strlen(data[i]) + 1; //contains the '\0'
		memcpy(msg, data[i], str_size);
		length = length + str_size;
		msg = begin + length;
	}
	return length;
}
int msg2data(char* msg, int msg_size, char data[][MAXSTRING]) {
	clear_data(data);
	int num = 0;
	sscanf(msg, "%d", &num);
	int data_size = split(msg + 4, msg_size - 4, data, '\0');
	if (data_size != num) {
		printf("package corrupted: require %d but %d\n", num, data_size);
	}
	return num;
}

int my_send(int protol, char* send_buffer, char data[][MAXSTRING], int data_size, int socket, struct sockaddr_in* dst) {
	int len = data2msg(send_buffer, data, data_size);
	if (protol == TCP) {
		if (send(socket, send_buffer, len, 0) <= 0) {
			return -1;
		}
	}
	else if (protol == UDP) {
		if (sendto(socket, send_buffer, len, 0, (struct sockaddr*)dst, sizeof(*dst)) <= 0)
		{
			return -1;
		}
	}
	return len;
}
int my_recv(int protol, char* recv_buffer, char data[][MAXSTRING], int socket) {
	int n = 0;
	if (protol == TCP) {
		if ((n = recv(socket, recv_buffer, MAXSIZE, 0)) <= 0) {
			return -1;
		}
		recv_buffer[n] = 0;
	}
	else if (protol == UDP) {
		struct sockaddr_in peer;
		socklen_t addr_len = sizeof(peer);
		memset(&peer, 0, sizeof(peer));
		if ((n = recvfrom(socket, recv_buffer, MAXSIZE, 0, (struct sockaddr*)&peer, &addr_len)) <= 0) {
			return -1;
		}
		recv_buffer[n] = 0;
	}
	return 	msg2data(recv_buffer, n, data);//	msg2data(recv_buffer, n+1, data);
}


