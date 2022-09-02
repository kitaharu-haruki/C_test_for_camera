#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include "lcd.h"
#include "yuyv_show.h"
#include "queue.h"



int main(int argc,char*argv[])
{
	lcd_init();
	
	Queue *mplayer_inter = create_queue(); 
	
	pthread_mutex_t mutex_cli;

	int ret = pthread_mutex_init(&mutex_cli,NULL);
	if(ret != 0)
		{
			perror("mutex_inter_init error");
			lcd_uninit();
			return -1;
		}
					
	struct pthread_data inter_temp;

	inter_temp.snap = mplayer_inter;
	inter_temp.lock = mutex_cli;
	inter_temp.len = 640;
	inter_temp.high = 480;
	
	pthread_t pthread_id_local;
	ret = pthread_create(&pthread_id_local,NULL,pthread_yuv_image,(void*)&inter_temp);
	if(ret < 0)
		{
			perror("create colal_pthread error");
			return -1;
		}
	
	
	//第一步：创建套接字 
	int client_sock_fd = socket(AF_INET,SOCK_STREAM,0);
	//第二步：请求链接 
	struct sockaddr_in dest_addr;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(atoi(argv[2]));
	dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
	ret = connect(client_sock_fd,(struct sockaddr*)&dest_addr,sizeof(dest_addr));

	
	
	//第三步：收发数据 
	while(1)
	{		
		int ret = send(client_sock_fd,"video",5,0);
		
		unsigned char str[640*480*2] = {0};
		ret = recv(client_sock_fd,str,640*480*2,MSG_WAITALL);
		
		pthread_mutex_lock(&mutex_cli);	
		printf("str size = %d\n",strlen(str));
		if(strlen(str) != 0)
			Enqueue(mplayer_inter,str);	
		pthread_mutex_unlock(&mutex_cli);	
		
		printf("join queue success\n");	
		printf("queue mplayer's number = %d\n",mplayer_inter->num);
	}

	//第四步：关闭套接字 
	destory_Queue(mplayer_inter);
	lcd_uninit();
	close(client_sock_fd);
}



