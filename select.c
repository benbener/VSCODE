/*************************************************************************
    > File Name: select.c
    > Author: 
    > Mail:  
    > Created Time: 2022年07月09日 星期六 17时18分08秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define TCP_LINK_COUNT 10
#define TCP_BUFF_LEN 512

int main(int argc,char *argv[])
{
	if (argc != 2)
	{
		printf("Usage:%s <port>\n",argv[0]);
		return -1;
	}

	int serv_sock = socket(AF_INET,SOCK_STREAM,0);
	if (serv_sock == -1)
	{
		perror("socket() error!");
		exit(1);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	//serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_addr.s_addr = inet_addr("192.168.0.136");
	//serv_addr.sin_addr.s_addr = inet_pton(AF_INET,"192.168.0.136",&serv_addr.sin_addr.s_addr);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
	{
		perror("bind() error!");
		exit(1);
	}

	if (listen(serv_sock,TCP_LINK_COUNT) == -1)
	{
		perror("listen() error!");
		exit(1);
	}

	fd_set readfds,tmpfds;
	FD_ZERO(&readfds);
	FD_ZERO(&tmpfds);
	FD_SET(serv_sock,&readfds);
	int maxfd = serv_sock;

	int clnt_socks[TCP_LINK_COUNT];
	memset(clnt_socks,0,TCP_LINK_COUNT);

	struct sockaddr_in clnt_addrs[TCP_LINK_COUNT];
	int clnt_addr_sz = sizeof(struct sockaddr_in);
	
	int clnt_idx;
	
	char buff[TCP_BUFF_LEN];
	
	while(1)
	{
		tmpfds = readfds;
		if(select(maxfd+1,&readfds,NULL,NULL,NULL) == -1)
		{
			perror("select() error!");
			exit(1);
		}

		if(FD_ISSET(serv_sock,&tmpfds))		// 判断是否有客户端连接
		{
			for (clnt_idx=0;clnt_idx<TCP_LINK_COUNT;clnt_idx++)
			{
				if (clnt_socks[clnt_idx] == 0)
				{
					break;
				}
			}

			clnt_socks[clnt_idx] = accept(serv_sock,(struct sockaddr*)&clnt_addrs[clnt_idx],&clnt_addr_sz);
			if(clnt_socks[clnt_idx] == -1)
			{
				perror("accpet() error!");
				exit(1);
			}
			
			FD_SET(clnt_socks[clnt_idx],&readfds);
			if(clnt_socks[clnt_idx] > maxfd)
			{
				maxfd = clnt_socks[clnt_idx];
			}

			printf("client [%s:%d] connected.\n",inet_ntoa(clnt_addrs[clnt_idx].sin_addr),ntohs(clnt_addrs[clnt_idx].sin_port));
		}
		else 
		{
			for (clnt_idx=0;clnt_idx<TCP_LINK_COUNT;clnt_idx++)
			{
				if (FD_ISSET(clnt_socks[clnt_idx],&tmpfds))
				{
					while(1)
					{
						memset(buff,0,TCP_BUFF_LEN);
						int len = read(clnt_socks[clnt_idx],buff,TCP_BUFF_LEN);
						if (len == 0)
						{
							printf("client %d[%s:%d] disconnected.\n",clnt_socks[clnt_idx],inet_ntoa(clnt_addrs[clnt_idx].sin_addr),ntohs(clnt_addrs[clnt_idx].sin_port));
							FD_CLR(clnt_socks[clnt_idx],&readfds);
							close(clnt_socks[clnt_idx]);
							clnt_socks[clnt_idx] = 0;
							break;
						}
						else if (len > 0)
						{
							printf("recv %s from %s",buff,inet_ntoa(clnt_addrs[clnt_idx].sin_addr));
							write(clnt_socks[clnt_idx],buff,len);
						}
						else
						{
							perror("read() error!");
							break;
						}
					}
				}
			}
		}
	}

	close(serv_sock);

    return 0;
}
