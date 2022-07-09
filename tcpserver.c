/*************************************************************************
    > File Name: tcpserver.c
    > Author: 
    > Mail:  
    > Created Time: 2022年07月08日 星期五 09时53分43秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_BUFF_LEN 512
#define TCP_BIND_MAX 10

int main(int argc,char *argv[])
{
	if (argc != 2)
	{
		printf("Usage:%s <port>\n",argv[0]);
		return -1;
	}

	int serv_sock,clnt_sock;
	struct sockaddr_in serv_addr,clnt_addr;
	socklen_t addrlen;
	char buff[TCP_BUFF_LEN];
	
	serv_sock = socket(AF_INET,SOCK_STREAM,0);
	if (serv_sock < 0)
	{
		perror("socket() error!");
		return -1;
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
    addrlen = sizeof(serv_addr);

	if (bind(serv_sock,(struct sockaddr*)&serv_addr,addrlen) < 0)
	{
		perror("bind() error!");
		return -1;
	}
	
	if (listen(serv_sock,TCP_BIND_MAX) < 0)
	{
		perror("bind() error!");
		return -1;
	}

	while(1)
	{
		if ((clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&addrlen)) < 0)
		{
			perror("accept() error!");
			return -1;
		}
	
		printf("client %d[%s:%d] connected.\n",clnt_sock,inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));
	
		int recvlen = 0;
		if (clnt_sock > 0)
		{
			while(1)
			{
				memset(buff,0,TCP_BUFF_LEN);
				recvlen = recv(clnt_sock,buff,TCP_BUFF_LEN,0);
				if (recvlen == 0)
				{
					printf("client %d[%s:%d] disconnected.\n",clnt_sock,inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));
					close(clnt_sock);
					break;
				}
				printf("recv [%s] from %s\n",buff,inet_ntoa(clnt_addr.sin_addr));
			}
		}	
	}

	close(serv_sock);

    return 0;
}
