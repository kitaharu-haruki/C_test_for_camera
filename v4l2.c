#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "lcd.h"
#include "yuyv_show.h"
#include "queue.h"
#include "epoll_TCP_ser.h"


struct usebuf
{
	void *start; 
	int length; 
};
struct usebuf ub[16] ={0}; 


int main(int argc,char*argv[])
{
	int fd;
	int r;
	int num = 16;		//缓冲区的数量;
	lcd_init();
	
	Queue *mplayer = create_queue(); 
	Queue *inter = create_queue();
	
	pthread_mutex_t mutex_local,mutex_inter;
	int ret = pthread_mutex_init(&mutex_local,NULL);
	if(ret != 0)
		{
			perror("mutex_local_init error");
			close(fd);
			lcd_uninit();
			return -1;
		}
	ret = pthread_mutex_init(&mutex_inter,NULL);
	if(ret != 0)
		{
			perror("mutex_inter_init error");
			close(fd);
			lcd_uninit();
			return -1;
		}
					
	struct pthread_data temp,inter_temp;
	temp.snap = mplayer;
	temp.lock = mutex_local;
	temp.len = 640;
	temp.high = 480;

	inter_temp.snap = inter;
	inter_temp.lock = mutex_inter;
	inter_temp.len = 640;
	inter_temp.high = 480;
	
	pthread_t pthread_id_local,pthread_id_inter;
	ret = pthread_create(&pthread_id_local,NULL,pthread_yuv_image,(void*)&temp);
	if(ret < 0)
		{
			perror("create colal_pthread error");
			return -1;
		}
	
	ret = pthread_create(&pthread_id_inter,NULL,epoll_tcp_cli,(void*)&inter_temp);
	if(ret < 0)
		{
			perror("create inter_pthread error");
			return -1;
		}
/*
	打开摄像头；
*/
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
		{
			perror("open error:");
			return -1;
		}

	
	struct v4l2_capability cap;
	ret = ioctl(fd,VIDIOC_QUERYCAP,&cap); 
	if(ret < 0)
		{
			perror("ioctl VIDIOC_QUERYCAP error");
			return -1;
		}
	printf("cap.capabilities = %d",cap.capabilities);
	if(cap.capabilities == V4L2_CAP_VIDEO_CAPTURE)
		{
			printf("YES\n");
		}



	struct v4l2_fmtdesc check;
	check.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	check.index = 0;
	
	ret = ioctl(fd,VIDIOC_ENUM_FMT,&check);
	if(ret < 0)
		{
			perror("ioctl VIDIOC_ENUM_FMT error");
			return -1;
		}
	printf(" format:%s\n",check.description);

	


	struct v4l2_format s_fmt;
	s_fmt.fmt.pix.width = 640; 
	s_fmt.fmt.pix.height = 480; 
	s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	ret = ioctl(fd,VIDIOC_S_FMT,&s_fmt);
	if(ret < 0)
		{
			perror("ioctl VIDIOC_S_FMT error");
			return -1;
		}
	printf("pix.field = %d\n",s_fmt.fmt.pix.field);
	

	/*
		step 5: 申请视频缓冲
	*/
	struct v4l2_requestbuffers reqbuf;
	memset(&reqbuf,0,sizeof(reqbuf));
	
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.count = num;		//申请缓冲区的数量
	reqbuf.memory = V4L2_MEMORY_MMAP;
	
	ret = ioctl(fd,VIDIOC_REQBUFS,&reqbuf);
	if(ret < 0)
		{
			perror("ioctl VIDIOC_REQBUFS error");
			return -1;
		}
	else 
		{
			printf("request buf success\n");
		}

	
	/*
		step 6: 获取每个缓冲区的信息，并映射，再把它加入到
			采集队列中去。

	*/
	struct v4l2_buffer buf;
	int i = 0;
	for(i=0;i<num;i++)
		{
			memset(&buf,0,sizeof(buf));
			
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i; 
			ret = ioctl(fd,VIDIOC_QUERYBUF,&buf);
			if(ret <0)
				{
					perror("ioctl VIDIOC_QUERYBUF error");
					return -1;
				}
			else 
				{
					printf("ioctl VIDIOC_QUERYBUF success\n");
				}
			/*映射缓冲区；*/
			ub[i].length = buf.length;
			ub[i].start = mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
			if(ub[i].start == NULL)
				{
					perror("mmap ub[i] error\n");
					return -1;
				}
			else 
				{
					printf("this is test ===================\n");
					printf("ub[%d].length = %d\n",i,ub[i].length);
					printf("this is test ===================\n");
				}
			/*把申请到的缓冲区加入到采集队列上去*/
			ret=ioctl(fd,VIDIOC_QBUF,&buf);
			if(ret <0)
				
				{
					perror("ioctl VIDIOC_QBUF error");
					return -1;
				}
			else 
				{
					printf("ioctl VIDIOC_QBUF success\n");
				}
		}
	
	
	
	
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd,VIDIOC_STREAMON,&type);
	if(ret < 0)
		{
			perror("ioctl VIDIOC_STREAMON error");
			return -1;
		}
	else 
		{
			printf("ioctl VIDIOC_STREAMON success\n");
		}
	
	
	fd_set rfds;  
			
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds); 
	while (1)
		{
			r = select(fd + 1, &rfds, NULL, NULL,NULL);
			if (r <= 0)
				{
					continue;
				}
			else
				{
					if (FD_ISSET(fd, &rfds)) 
						{
							struct v4l2_buffer  vbuf;
							bzero(&vbuf,sizeof(vbuf));
							vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
							vbuf.memory = V4L2_MEMORY_MMAP;


							
							r = ioctl(fd, VIDIOC_DQBUF, &vbuf);
							if (r == -1)
								{
									perror("VIDIOC_DQBUF error:");
									break;
								}
							
							
							pthread_mutex_lock(&mutex_local);	
							Enqueue(mplayer,ub[vbuf.index].start);
							pthread_mutex_unlock(&mutex_local);	

							
							pthread_mutex_lock(&mutex_inter);	
							Enqueue(inter,ub[vbuf.index].start);
							pthread_mutex_unlock(&mutex_inter); 

							r = ioctl(fd, VIDIOC_QBUF, &vbuf);
							if (r == -1)
								{
									perror("VIDIOC_QBUF error:");
									continue;
								}
						}
				}
		}
	
	

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd,VIDIOC_STREAMOFF,&type);
	if(ret < 0)
		{
			perror("ioctl VIDIOC_STREAMOD error");
			return -1;
		}
	else 
		{
			printf("ioctl VIDIOC_STREAMOD success\n");
		}
	

	for(i = 0;i< num;i++)
		{
			munmap(ub[i].start,ub[i].length);
		}
	
	close(fd);
	destory_Queue(mplayer);
	destory_Queue(inter);
	lcd_uninit();
	
}

