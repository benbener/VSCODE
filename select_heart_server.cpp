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

#define TCP_MAX_LINK 10
#define BUFFER_SIZE 1024
#define HEART "Heart"
#define DATA "Data"

struct PACKETHEADER
{
	char type[10];
	int length;
};

std::map<int,std::pair<std::string,int> > clnt_map;

fd_set master_fds;
fd_set working_fds;

int max_fd;

unsigned char buffer[BUFFER_SIZE];

void* heartbeat(void *argv)
{
	printf("====== heartbeat thread begin ======\n");
	while (1)
	{
		for (std::map<int,std::pair<std::string,int> >::iterator it = clnt_map.begin(); it != clnt_map.end(); )
		{
			if (it->second.second == 5)
			{
				int fd = it->first;
				close(fd);
				printf("HEARTBEAT: close client-%s:%d!\n",it->second.first.c_str(),fd);

				FD_CLR(fd,&master_fds);
				if (fd == max_fd)
				{
					while(FD_ISSET(max_fd,&master_fds) == false)
					{
						max_fd--;
					}
				}

				clnt_map.erase(it++);
			}
			else if (it->second.second < 5 && it->second.second >= 0)
			{
				it->second.second += 1;
				printf("HEARTBEAT: %s:%d beat %d!\n",it->second.first.c_str(),it->first,it->second.second);
				it++;
			}
			else
			{
				it++;
			}
		}

		sleep(3);
	}
	printf("====== heartbeat thread end ======\n");
}

int main(int argc,char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <port>\n",argv[0]);
		exit(0);
	}

	int serv_sock = socket(PF_INET,SOCK_STREAM,0);
	if (serv_sock < 0)
	{
		perror("socket() error!");
		exit(1);
	}

	int opt = 1;
	setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
	{
		perror("bind() error!");
		exit(1);
	}

	if (listen(serv_sock,TCP_MAX_LINK) == -1)
	{
		perror("listen() error!");
		exit(1);
	}

	pthread_t tid;
	if (pthread_create(&tid,NULL,heartbeat,NULL) != 0)
	{
		perror("pthread_create() error!");
		exit(1);
	}

	max_fd = serv_sock;
	
	FD_ZERO(&master_fds);
	FD_SET(serv_sock,&master_fds);

	struct timeval timeout;
	while (1)
	{
		FD_ZERO(&working_fds);
		memcpy(&working_fds,&master_fds,sizeof(master_fds));

		timeout.tv_sec = 30;
		timeout.tv_usec = 0;

		int num = select(max_fd+1,&working_fds,NULL,NULL,&timeout);
		if (num < 0)
		{
			perror("select() error!");
			// exit(1);
		}

		if (num == 0)
		{
			continue;
		}

		if (FD_ISSET(serv_sock,&working_fds))
		{
			struct sockaddr_in clnt_addr;
			socklen_t addr_len = sizeof(clnt_addr);
			int clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&addr_len);
			if (clnt_sock < 0)
			{
				perror("accept() error!");
				exit(1);
			}

			printf("client-%s:%d connected!\n",inet_ntoa(clnt_addr.sin_addr),clnt_sock);

			clnt_map.insert(std::make_pair(clnt_sock,std::make_pair(std::string(inet_ntoa(clnt_addr.sin_addr)),0)));

			FD_SET(clnt_sock,&master_fds);
			if (clnt_sock > max_fd)
			{
				max_fd = clnt_sock;
			}
				
			// clnt_map.erase(clnt_sock);
		}
		else 
		{
			for (int fd = 0;fd <= max_fd;fd++)
			{
				if (FD_ISSET(fd,&working_fds))
				{
					bool close_conn = false;

					memset(buffer,0,sizeof(buffer));
					int rcv_len = recv(fd,buffer,sizeof(buffer),0);
					if (rcv_len == 0)
					{
						printf("close client-%s:%d!\n",clnt_map[fd].first.c_str(),fd);

						close(fd);
						FD_CLR(fd,&master_fds);
						if (fd == max_fd)
						{
							while (FD_ISSET(max_fd,&master_fds))
							{
								max_fd--;
							}
						}
				
						clnt_map.erase(fd);

						continue;
					}

					PACKETHEADER header;
					memset(&header,0,sizeof(header));
					memcpy(&header,buffer,sizeof(header));
					if (strcmp(header.type,HEART) == 0)
					{
						printf("recv HEARTBEAT from client-%s:%d\n",clnt_map[fd].first.c_str(),fd);
						int snd_len = send(fd,&header,sizeof(header),0);
						if (snd_len > 0)
						{
							printf("echo HEARTBEAT to client-%s:%d\n",clnt_map[fd].first.c_str(),fd);
						}

						clnt_map[fd].second = 0;
					}
					else if (strcmp(header.type,DATA) == 0)
					{
						clnt_map[fd].second = 0;

						if (header.length > 0)
						{
							char buff[header.length] = {0};
							int len = recv(fd,buff,header.length,0);
							if (len > 0)
							{
								printf("recv %s from client-%s:%d\n",buff,clnt_map[fd].first.c_str(),fd);
							}
						}
					}
				}
			}
		}
	}

	for (int fd = 0;fd <= max_fd;fd++)
	{
		if (FD_ISSET(fd,&master_fds))
		{
			close(fd);
		}
	}

    return 0;
}
