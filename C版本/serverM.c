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
char g_data_fromEE[MAXDATASIZE][MAXSTRING] = { 0 };
char g_data_fromCS[MAXDATASIZE][MAXSTRING] = { 0 };
char g_data_for_multi[MAXDATASIZE][MAXSTRING] = { 0 };
unsigned short serverM_tcp_port = 25555;
unsigned short serverM_udp_port = 24666;
unsigned short serverC_udp_port = 21999;
unsigned short serverCS_udp_port = 22888;
unsigned short serverEE_udp_port = 23777;
struct sockaddr_in serverM_tcp_addr;
struct sockaddr_in serverM_udp_addr;
struct sockaddr_in serverC_udp_addr;
struct sockaddr_in serverCS_udp_addr;
struct sockaddr_in serverEE_udp_addr;

void encrypt(char* src, char* dst, int size);
char * strlwr(char * str);

char * strlwr(char * str){
	int i=0;
	while(str[i]){
		if('A'<=str[i]&&str[i]<='Z')
			str[i]='a'+ str[i]-'A';
		i++;
	}
	return str;
}

int main() {
	//fill in the address
	fill_sock_addr(&serverM_tcp_addr, serverM_tcp_port);
	fill_sock_addr(&serverM_udp_addr, serverM_udp_port);
	fill_sock_addr(&serverC_udp_addr, serverC_udp_port);
	fill_sock_addr(&serverCS_udp_addr, serverCS_udp_port);
	fill_sock_addr(&serverEE_udp_addr, serverEE_udp_port);
	//create socket
	int serverM_tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverM_tcp_sock == -1) {
		perror("can not create tcp socket");
		return -1;
	}
	int serverM_udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverM_udp_sock == -1) {
		perror("can not create udp socket");
		return -1;
	}
	//bind the port 

	if (bind(serverM_tcp_sock, (struct sockaddr*)&serverM_tcp_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("fail to bind tcp addr");
		return -1;
	}
	if (bind(serverM_udp_sock, (struct sockaddr*)&serverM_udp_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("fail to bind udp addr");
		return -1;
	}

	//listen TCP port
	if (listen(serverM_tcp_sock, 5) < 0) {
		perror("can not listen");
		return -1;
	}
	printf("The main server is up and running.\n");


	for (;;) {
		struct sockaddr_in client_tcp_addr;
		socklen_t addrlen = sizeof(struct sockaddr_in);
		int client = accept(serverM_tcp_sock, (struct sockaddr*)&client_tcp_addr, &addrlen);
		int attempts = 3;
		while (attempts-- > 0) {
			int data_size = my_recv(TCP, g_recv_buffer, g_data, client);
			if (data_size < 0)
				break; //client close the socket
			if (data_size != 2) {
				printf("data corrupted\n");
				break;
			}
			char* username = g_data[0];
			char* password = g_data[1];
			char present_user[100] = { 0 };
			strcpy(present_user, username);
			char ciper_username[100] = { 0 };
			char ciper_password[100] = { 0 };
			printf("The main server received the authentication for %s using TCP over port %d.\n", username, serverM_tcp_port);
			encrypt(username, ciper_username, strlen(username));
			encrypt(password, ciper_password, strlen(password));
			strcpy(g_data[0], ciper_username);
			strcpy(g_data[1], ciper_password);
			//forward to ServerC
			if (my_send(UDP, g_send_buffer, g_data, 2, serverM_udp_sock, &serverC_udp_addr) < 0) {
				break;
			}
			printf("The main server sent an authentication request to serverC.\n");
			//recv from serverC
			data_size = my_recv(UDP, g_recv_buffer, g_data, serverM_udp_sock);
			if (data_size < 0) {
				perror("recv from serverC");
				break;
			}
			printf("The main server received the result of the authentication request from ServerC using UDP over port %d.\n", serverM_udp_port);
	
			//forward to clinet
			if (my_send(TCP, g_send_buffer, g_data, data_size, client, NULL) < 0) {
				break;
			}
			printf("he main server sent the authentication result to the client.\n");
			if (strcmp("PASS", g_data[0]) != 0)
				continue;

			//authentication pass
			//handle the query request
			while (1)
			{
				//recv quert from client
				if ((data_size = my_recv(TCP, g_recv_buffer, g_data, client)) < 0) {
					perror("client offline");
					break;
				}
				if (strcmp("SINGLE", g_data[0]) == 0) {
					char department[10] = { 0 };
					char course_code[100] = { 0 };
					char category[100] = { 0 };
					strcpy(course_code, g_data[1]);
					strcpy(category, g_data[2]);
					memcpy(department, course_code, 2);
					printf("The main server received from %s to query course %s about %s using TCP over port %d.\n",
						present_user, course_code, strlwr(category), serverM_tcp_port);
					strcpy(g_data[0], course_code);
					strcpy(g_data[1], category);
					if (strcmp(department, "EE") == 0) {
						//code category
						my_send(UDP, g_send_buffer, g_data, 2, serverM_udp_sock, &serverEE_udp_addr);
						printf("The main server sent a request to serverEE.\n");
						data_size = my_recv(UDP, g_recv_buffer, g_data, serverM_udp_sock);
						printf("The main server received the response from serverEE using UDP over port %d.\n", serverM_udp_port);
					}
					else if(strcmp(department, "CS") == 0) {
						my_send(UDP, g_send_buffer, g_data, 2, serverM_udp_sock, &serverCS_udp_addr);
						printf("The main server sent a request to serverCS.\n");
						data_size = my_recv(UDP, g_recv_buffer, g_data, serverM_udp_sock);
						printf("The main server received the response from serverCS using UDP over port %d.\n", serverM_udp_port);

					}
					else {
						sprintf(g_data[0], "Didn't find the course: %s.\n", course_code);
						data_size = 1;
					}
					//response
					if(my_send(TCP,g_send_buffer,g_data,data_size,client,NULL)<0){
						break; //offline
					}
					printf("The main server sent the query information to the client.\n");
				}
				else if (strcmp("MULTY", g_data[0]) == 0) {
					//MULTY CS100 EE450
					printf("The main server received from %s to query course with multiple CourseCode using TCP over port %d.\n",present_user, serverM_tcp_port );
					clear_data(g_data_for_multi);
					sprintf(g_data_for_multi[0], "CourseCode: Credits, Professor, Days, Course Name\n");
					//with multiple CourseCode
					int num = data_size;
					char courses[20][100] = { 0 };
					for (int i = 1; i < num; ++i) {
						strcpy(courses[i], g_data[i]);
					}
					for (int i = 1; i < num; i++) {
						char department[10] = { 0 };
						char* course_code = courses[i];
						memcpy(department, course_code, 2);
						if (strcmp(department, "EE") == 0) {
							strcpy(g_data[0], course_code);
							strcpy(g_data[1], "ALL_CATEGORY");
							my_send(UDP, g_send_buffer, g_data, 2, serverM_udp_sock, &serverEE_udp_addr);
							printf("The main server sent a request to serverEE.\n");
							data_size = my_recv(UDP, g_recv_buffer, g_data, serverM_udp_sock);
							printf("The main server received the response from serverEE using UDP over port %d.\n", serverM_udp_port);
						}
						else if (strcmp(department, "CS") == 0) {
							strcpy(g_data[0], course_code);
							strcpy(g_data[1], "ALL_CATEGORY");
							my_send(UDP, g_send_buffer, g_data, 2, serverM_udp_sock, &serverCS_udp_addr);
							printf("The main server sent a request to serverCS.\n");
							data_size = my_recv(UDP, g_recv_buffer, g_data, serverM_udp_sock);
							printf("The main server received the response from serverCS using UDP over port %d.\n", serverM_udp_port);
						}
						else {
							sprintf(g_data[0], "Didn't find the course: %s.\n", course_code);
							data_size = 1;
						}

						//now g_data[0] is the present result
						strcpy(g_data_for_multi[i], g_data[0]);
					}
					//head item1 item2 ..... 
					if (my_send(TCP, g_send_buffer, g_data_for_multi, num, client, NULL) < 0) {
						break; //offline
					}
					printf("The main server sent the query information to the client.\n");
				}

				//send to client
			}


			break;
		}


		close(client);
		clear_data(g_data);
	}
}
void encrypt(char* src, char* dst, int size) {
	for (int i = 0; i < size; i++) {
		if ('a' <= src[i] && src[i] <= 'z') {
			dst[i] = 'a' + (src[i] - 'a' + 4) % 26;
		}
		else if ('A' <= src[i] && src[i] <= 'Z') {
			dst[i] = 'A' + (src[i] - 'A' + 4) % 26;
		}
		else if ('0' <= src[i] && src[i] <= '9') {
			dst[i] = '0' + (src[i] - '0' + 4) % 10;
		}
		else {
			dst[i] = src[i];
		}
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


