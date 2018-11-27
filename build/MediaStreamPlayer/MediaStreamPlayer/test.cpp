#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
using namespace std;

sem_t g_sem;

bool isRunning = false;

void handleTermSig(int snum) {
	isRunning = false;
}


void* threadFunc(void* arg)
{
	pthread_t threadId = pthread_self();
	int count = 0;
	while(isRunning)
	{
		sem_wait(&g_sem);
		std::cout << "thread ID: " << threadId << ", count: " << count++ << std::endl;
	}
	std::cout << "thread ID " << threadId << ", exited" << std::endl;
	return (void*)0;
}

int main()
{
	int ret = -1;
	pthread_t thread;
	int count = 0;
	
	isRunning = true;
	
	signal(SIGABRT, &handleTermSig);
	signal(SIGINT, &handleTermSig);
	signal(SIGTERM, &handleTermSig);
	ret = sem_init(&g_sem, 0, 0);
	if(ret == -1)
	{
		perror("semaphore intitialization failed\n");
		return -1;
	}
	
	ret = pthread_create(&thread, NULL, threadFunc, NULL);
	if(ret != 0)
	{
		perror("pthread_create failed\n");
		return -1;
	}
	//等待线程跑起来
	sleep(1);
	while(isRunning)
	{
		std::cout << "main sem_post cout: " << count << std::endl;
		if(count++ > 10)
			break;
		sleep(3);
		sem_post(&g_sem);
	}
	isRunning = false;
	
	ret = pthread_join(thread, NULL);
	if(ret != 0)
	{
		perror("pthread_join failed\n");
		return -1;
	}
	printf("Thread joined\n");

	sem_destroy(&g_sem);
	return 0;
}
