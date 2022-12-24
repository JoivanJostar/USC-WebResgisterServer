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
unsigned short serverC_udp_port = 21999;
struct sockaddr_in serverM_udp_addr;
struct sockaddr_in serverC_udp_addr;

int Authenticae(char* username, char* password);

int main() {
	fill_sock_addr(&serverC_udp_addr, serverC_udp_port);
	fill_sock_addr(&serverM_udp_addr, serverM_udp_port);

	int serverC_udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverC_udp_sock == -1) {
		perror("can not create socket");
		return -1;
	}

	if (bind(serverC_udp_sock, (struct sockaddr*)&serverC_udp_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("can not bind port");
		return -1;
	}
	printf("The ServerC is up and running using UDP on port %d.\n", serverC_udp_port);
	while (1) {

		int data_size = my_recv(UDP, g_recv_buffer, g_data, serverC_udp_sock);
		if (data_size < 0) {
			perror("recv from serverM");

			break;
		}
		printf("The ServerC received an authentication request from the Main Server.\n");
		char username[100] = { 0 };
		char password[100] = { 0 };
		strcpy(username, g_data[0]);
		strcpy(password, g_data[1]);
		int result=Authenticae(username, password);
		int len = 0;
		switch (result)
		{
		case 0:
			strcpy(g_data[0], "NO_USER");
			break;
		case 1:
			strcpy(g_data[0], "NO_MATCH");
			break;
		case 2:
			strcpy(g_data[0], "PASS");
			break;
		default:
			break;
		}
		len = my_send(UDP, g_send_buffer, g_data, 1, serverC_udp_sock, &serverM_udp_addr);
		if (len < 0) {
			perror("send to serverM fail");
			break;
		}
		printf("The ServerC finished sending the response to the Main Server.\n");
	}
	close(serverC_udp_sock);

}
int Authenticae(char * username, char * password) {
	FILE* cred=fopen("cred.txt", "r");
	if (cred == NULL) {
		perror("open cred.txt");
		return 0;
	}
	int flag = 0;
	char line[200] = { 0 };
	//char *fgets(char *str, int n, FILE *stream)
	while (fgets(line, 200, cred)) {
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;
		if (line[strlen(line) - 1] == '\r')
			line[strlen(line) - 1] = 0;
		char splited[2][100];
		split(line, strlen(line), splited, ',');
		char* name = splited[0];
		char * passwd = splited[1];
		if (strcmp(name, username) == 0) {
			flag = 1;
			if (strcmp(passwd, password) == 0) {
				flag = 2;
			}
			break;
		}
	}
	fclose(cred);
	return flag;
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
	for (int i = 0; i < MAXDATASIZE;i++)
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


