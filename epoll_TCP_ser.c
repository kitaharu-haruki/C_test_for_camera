#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include "epoll_TCP_ser.h"
#include "queue.h"

#define N 256

void *epoll_tcp_cli(void *arr)
{
	struct pthread_data send_data = *(struct pthread_data*)arr;
	Queue *temp = send_data.snap;
	
	struct epoll_event arry[N];
	bzero(arry,sizeof(arry));

	int i,j; 
	int conn_sock; //connect的套接字描述符 

	char recv_buf[N] ={0};//接收数据的存储区域 


	int epoll_fd = epoll_create(1);

	//第一步：创建套接字 必须是流式套接字 
	int ser_sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(ser_sock_fd < 0)
		{
			perror("socket error");
			return (void*)-1;
		}
	//第二步：绑定地址和端口号 
	int number = 10086;
	struct sockaddr_in ser_addr;
	
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(number);
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(ser_sock_fd,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
	if(ret <0)
		{
			perror("bind error");
			return (void*)-1;
		}
	//第三步：监听

	ret = listen(ser_sock_fd,10);
	if(ret <0)
		{
			perror("listen error");
			return (void*)-1;
		}

	struct epoll_event ser;
	bzero(&ser,sizeof(ser));
	ser.data.fd = ser_sock_fd;
	ser.events = EPOLLIN|EPOLLRDHUP;
	
	ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,ser_sock_fd,&ser);
	if(ret < 0)
		{
			perror("epoll_ctl error");
			close(epoll_fd);
			close(ser_sock_fd);
			return (void*)-1;
		}
	
	int num = 0;
	while(1)
		{
			int sec_ret = epoll_wait(epoll_fd,arry,N,5000); 
			printf("sec_ret = %d\n",sec_ret);
			//第一次进来只监听一个 套接字描述符 就是服务端的套接字 
			//如果的服务端的套接字可读 一定是有连接进来了 

			//如果第二次进来还是要建立连接的 那么  i的值 一定再 集合中 并且 i应该就是服务端的套接字 
			// 同第一次 
			//如果第二次 进来的 不是 建立连接 而是客户端发来的消息 那么第一次 添加进入描述符集合的那个
			//套接字就是可读的 
			if(sec_ret == -1)
				{
					perror(" select error");
					return (void*)-1;
				}
			else if(sec_ret == 0)
				{
					printf("time out ....\n");
					continue;
				}
			else 
			{
				for(i = 0;i < sec_ret;i++) //找所有的描述符 
					{
						//从 0判断 看是不是再 我监听的描述符集合中 
						if((arry[i].events & EPOLLIN) || (arry[i].events & EPOLLRDHUP)) //如果为真 则 这个文件描述符是可读的 
							{	//成立表示在
								if(arry[i].data.fd == ser_sock_fd) //描述符 就是 服务端的套接字描述符 					//请求连接 
									{
										//第四步：接受链接 //允许客户端链接
										struct sockaddr_in sa;
										bzero(&sa,sizeof(sa));
										int len = sizeof(sa);
										
										conn_sock = accept(ser_sock_fd,(struct sockaddr*)&sa,&len);
										if(conn_sock < 0)
											{
												perror("accept error");
												return (void*)-1;
											} 
										else 
											{
												printf("accept success\n");
												struct epoll_event temp_num;
												bzero(&temp_num,sizeof(struct epoll_event));
												
												temp_num.data.fd = conn_sock;
												temp_num.events = EPOLLIN|EPOLLRDHUP;

												int ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,conn_sock,&temp_num);
												clear_queue(temp);
											}
									}
								else 
									{
										if(arry[i].events & EPOLLRDHUP)
											{
												printf("quit connect!\n");

												close(arry[i].data.fd);
												bzero(&arry[i] ,sizeof(struct epoll_event));
											}
										////第五步：收发数据 
										else
											{
												bzero(recv_buf,N);
												recv(arry[i].data.fd,recv_buf,N,0);

												
												pthread_mutex_lock(&send_data.lock);
												unsigned char p[640*480*2] = {0};
												del_queue(temp,p);
												pthread_mutex_unlock(&send_data.lock);
												printf("out queue data size = %d\n",strlen(p));
												int ret = send(arry[i].data.fd,p,640*480*2,0);
												if(ret < 0)
													{
														perror("send error");
														close(arry[i].data.fd);
														return (void*)-1;
													}
											}
										
									}
							}
					}
				}
		}
	//第六步：关闭链接 
	close(epoll_fd);
	close(ser_sock_fd);
	
	pthread_exit((void*)0);
}

