/*************************************************************************
    > File Name: thread.c
    > Author: 
    > Mail:  
    > Created Time: 2022年07月09日 星期六 10时48分05秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define THREAD_COUNT 10

// 查看线程命令: ps -efL | grep program
// 查看线程内存命令: ps -e -o 'pid,comm,args,pcpu,rsz,vsz,stime,user,uid' | grep program

void* func(void *param)
{
	//pthread_t tid = pthread_self();
	for (int i=0;i<10;i++)
	{
		printf("thread_%d running %d\n",(int)param,i);
		sleep(1);
	}

	pthread_exit(0);
}

int main(int argc,char *argv[])
{
	pthread_t tid[THREAD_COUNT];
	for (int i=0;i<THREAD_COUNT;i++)
	{
		int ret = pthread_create(&tid[i],NULL,func,i);
		if (ret < 0)
		{
			printf("pthread_create() error!\n");
			return -1;
		}
		pthread_detach(tid[i]);
	}

	sleep(15);

	//for (int i=0;i<THREAD_COUNT;i++)
	//{
	//	//pthread_cancel(tid[i]);		// 被动退出
	//	pthread_join(tid[i],NULL);
	//}


    return 0;
}
