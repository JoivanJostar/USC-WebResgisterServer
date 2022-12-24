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

unsigned short serverM_tcp_port = 25555;
struct sockaddr_in serverM_tcp_addr;

int main() {

	
	unsigned short dynamic_port = 0;
	fill_sock_addr(&serverM_tcp_addr, serverM_tcp_port);
	//create socket
	int connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == -1) {
		perror("can not create tcp socket");
		return -1;
	}
	//connect to serverM
	if (connect(connection, (struct sockaddr*)&serverM_tcp_addr, sizeof(serverM_tcp_addr)) < 0) {
		perror("can not connect serverM");
		return -1;
	}
	//get dynamic port of client
	struct sockaddr_in client_addr;
	socklen_t namelen = sizeof(client_addr);
	int getsock_check = getsockname(connection, (struct sockaddr*)&client_addr, &namelen);
	if (getsock_check == -1) {
		perror("getsockname");
		exit(1);
	}
	dynamic_port = ntohs(client_addr.sin_port);


	printf("The client is up and running.\n");

	int attempts = 3;
	char username[100] = { 0 };
	char password[100] = { 0 };
	while (attempts-- > 0) {
		printf("Please enter the username: ");
		scanf("%s", username);
		printf("Please enter the password: ");
		scanf("%s", password);

		clear_data(g_data);
		strcpy(g_data[0], username);
		strcpy(g_data[1], password);
		if (my_send(TCP, g_send_buffer, g_data, 2, connection, NULL) <= 0) {
			perror("send username and password");
			break;
		}
		printf("%s sent an authentication request to the main server.\n", username);
		int data_size = my_recv(TCP, g_recv_buffer, g_data, connection);
		if (data_size < 0) {
			printf("disconnect with server\n");
			break;
		}
		printf("%s received the result of authentication using TCP over port %d. ", username, dynamic_port);
		if (strcmp("NO_USER", g_data[0]) == 0) {
			printf("Authentication failed: Username Does not exist\n");
			printf("Attempts remaining: %d\n", attempts);
			continue;
		}
		else if (strcmp("NO_MATCH", g_data[0]) == 0) {
			printf("Authentication failed: Password does not match\n");
			printf("Attempts remaining: %d\n", attempts);
			continue;
		}
		else if (strcmp("PASS", g_data[0]) == 0) {;
			printf("Authentication is successful\n");
		}

		//Authentication pass
		getchar();
		while (1) {

			printf("Please enter the course code to query: ");
			char line[100] ="\n";
			while (strcmp(line,"\n") == 0) {
				fgets(line,100,stdin);
			}
			line[strlen(line)-1]=0;		
			int num = split(line, strlen(line), g_data, ' ');
			if (num == 1) {
				char course_code[100] = { 0 };
				char category[100] = { 0 };
				strcpy(course_code, g_data[0]);
				printf("Please enter the category (Credit / Professor / Days / CourseName): ");
				scanf("%s", category);
				getchar();
				strcpy(g_data[0],"SINGLE");
				strcpy(g_data[1], course_code);
				strcpy(g_data[2], category);
				if (my_send(TCP, g_send_buffer, g_data, 3, connection, NULL) < 0) {
					perror("disconnet with serverM");
					break;
				}
				printf("%s sent a request to the main server.\n", username);
				if ( my_recv(TCP,g_recv_buffer,g_data,connection) < 0) {
					perror("disconnet with serverM");
					break;
				}
				printf("The client received the response from the Main server using TCP over port %d.\n", dynamic_port);
				printf("%s", g_data[0]);
			}
			else if(num>1){
				//MULTY course1 course2 ....course_num
				for (int i = num; i >=1; i--) {
					strcpy(g_data[i], g_data[i-1]);
				}
				strcpy(g_data[0], "MULTY");
				if (my_send(TCP, g_send_buffer, g_data, num+1, connection, NULL) < 0) {
					perror("disconnet with serverM");
					break;
				}
				printf("%s sent a request with multiple CourseCode to the main server.\n", username);
				if ((data_size=my_recv(TCP, g_recv_buffer, g_data, connection) ) < 0) {
					perror("disconnet with serverM");
					break;
				}
				printf("The client received the response from the Main server using TCP over port %d.\n", dynamic_port);
				for (int i = 0; i < data_size; i++) {
					printf("%s", g_data[i]);
				}
				
			}
			printf("-----Start a new request-----\n");
		}
		break;
	}
	if (attempts <= 0) {
		printf("Authentication Failed for 3 attempts. Client will shut down.\n");
	}
	close(connection);
}


void fill_sock_addr(struct sockaddr_in* sock_addr, short port) {
	memset(sock_addr, 0, sizeof(struct sockaddr_in));
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_port = htons(port);
	inet_aton("127.0.0.1", &(sock_addr->sin_addr));
}

int split( char* src, int src_size, char dst[][MAXSTRING], char delim) {
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
		if ((n = recvfrom(socket, recv_buffer, MAXSIZE, 0, (struct sockaddr*)&peer, &addr_len) )<= 0) {
			return -1;
		}
		recv_buffer[n] = 0;
	}
	return 	msg2data(recv_buffer, n, data);//	msg2data(recv_buffer, n+1, data);
}


