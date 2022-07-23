/*************************************************************************
	> File Name: select_heart_server.cpp
	> Author:
	> Mail:
	> Created Time: 2022年07月19日 星期二 09时36分50秒
 ************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <pthread.h>
#include <string>
#include <map>
#include <mutex>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <utility>
#include <signal.h>


#define TCP_MAX_LINK 10
#define BUFFER_SIZE 1024

#define HEART "Heart"
#define DATA "Data"

struct PACKETHEADER
{
	char type[10];
	int length;
};

unsigned char buffer[BUFFER_SIZE];
struct sockaddr_in serv_addr;

bool run_send_heart = true;

void handle_signal(int sig)
{
	printf("handle_signal %d \n",sig);
}

void* send_heartbeat(void *argv)
{
	printf("====== send heartbeat thread begin ======\n");
	int sock = *(int *)argv;

	int count = 0;
	char send_buffer[BUFFER_SIZE];
	PACKETHEADER header;

	while (run_send_heart)
	{
		memset(send_buffer, 0, sizeof(send_buffer));
		memset(&header, 0, sizeof(header));

		strcpy(header.type, HEART);

		memcpy(send_buffer, &header, sizeof(header));
		int len = send(sock, send_buffer, sizeof(send_buffer), 0);
		if (len < 0)
		{
			close(sock);
			sock = socket(AF_INET, SOCK_STREAM, 0);

			int count = 0;
			while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
			{
				if (count >= 5)
				{
					printf("reconnect 5 times fail ...\n");
					
					run_send_heart = false;

					break;
				}

				count++;
				printf("reconnect %d ...\n", count);

				sleep(3);
			}
		}
		else if (len > 0)
		{
			printf("send HeartBeat ...\n");
		}
		else 
		{
			printf("send HeartBeat(len=0) ...\n");
		}

		sleep(2);
	}
	printf("====== send heartbeat thread end ======\n");

	return 0;
}

void* recv_heartbeat(void *argv)
{
	printf("====== recv heartbeat thread begin ======\n");

	int sock = *(int *)argv;

	int count = 0;
	char recv_buffer[BUFFER_SIZE];
	PACKETHEADER header;

	while (run_send_heart)
	{
		memset(recv_buffer, 0, sizeof(recv_buffer));
		memset(&header, 0, sizeof(header));

		int len = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
		if (len == 0)
		{
			printf("recv 0 ...\n");;
			continue;
		}

		memcpy(&header, recv_buffer, sizeof(header));

		if (strcmp(header.type, HEART) == 0)
		{
			count++;
			if (count == 5)
			{
			}

			printf("recv HeartBeat ...\n");
		}
		else if (strcmp(header.type, DATA) == 0)
		{
			printf("recv Data ...\n");
		}

		sleep(3);
	}
	printf("====== recv heartbeat thread end ======\n");

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(0);
	}

	//signal(SIGPIPE,SIG_IGN);
	signal(SIGPIPE,handle_signal);

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket() error!");
		exit(1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	{
		perror("connect() error!");
		exit(1);
	}

	printf("connect %s success!\n", inet_ntoa(serv_addr.sin_addr));

	pthread_t snd_heart_tid;
	if (pthread_create(&snd_heart_tid, NULL, send_heartbeat, &sock) < 0)
	{
		perror("pthread_create() send_heartbeat error!");
		exit(1);
	}

	sleep(1);

	pthread_t rcv_heart_tid;
	if (pthread_create(&rcv_heart_tid, NULL, recv_heartbeat, &sock) < 0)
	{
		perror("pthread_create() recv_heartbeat error!");
		exit(1);
	}

	sleep(300);

	pthread_join(snd_heart_tid, NULL);
	pthread_join(rcv_heart_tid, NULL);

	close(sock);

	return 0;
}
