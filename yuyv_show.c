#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jpeglib.h>
#include<sys/mman.h>
#include<string.h>
#include<fcntl.h>
#include<pthread.h>
#include "lcd.h"
#include "queue.h"


int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
	 unsigned int pixel32 = 0;
	 unsigned char pixel[3];// = (unsigned char *)&pixel32;
	 int r, g, b;
	 r = y + (1.370705 * (v-128));
	 g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	 b = y + (1.732446 * (u-128));
	 if(r > 255) r = 255;
	 if(g > 255) g = 255;
	 if(b > 255) b = 255;
	 if(r < 0) r = 0;
	 if(g < 0) g = 0;
	 if(b < 0) b = 0;
	 pixel[0] = r * 220 / 256;
	 pixel[1] = g * 220 / 256;
	 pixel[2] = b * 220 / 256;

	 pixel32 = (pixel[0] << 16 ) | (pixel[1] << 8) | (pixel[2]);
	 return pixel32;
}


void *pthread_yuv_image(void *arr)
{
	struct pthread_data pmet = *(struct pthread_data*)arr;
	Queue *temp = pmet.snap;
	int width = pmet.len;
	int height = pmet.high;

	while(1)
		{
			if(temp->head == NULL)
				{
					continue;
				}
			else
				{
					pthread_mutex_lock(&pmet.lock);	
					unsigned char yuv[640*480*2];		//出队，显示画面；
					del_queue(temp,yuv);	
					pthread_mutex_unlock(&pmet.lock);
					
					unsigned int in, out = 0;
					unsigned int pixel_16;
					unsigned char pixel_24[3];
					unsigned int pixel32;
					int y0, u, y1, v;

					int pixel_num = 0;
					
					for(in = 0; in < width * height * 2; in += 4) 
						{

							pixel_16 =
								yuv[in + 3] << 24 |
								yuv[in + 2] << 16 |
								yuv[in + 1] <<  8 |
								yuv[in + 0];
						
							y0 = (pixel_16 & 0x000000ff);
							u  = (pixel_16 & 0x0000ff00) >>  8;
							y1 = (pixel_16 & 0x00ff0000) >> 16;
							v  = (pixel_16 & 0xff000000) >> 24;
							

							pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
							
							pixel_num ++;
							lcd_draw_point(pixel_num/width,pixel_num%width,pixel32);
							
							pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
							pixel_num ++;
							lcd_draw_point(pixel_num/width,pixel_num%width,pixel32);
						}
					pthread_mutex_lock(&pmet.lock);	
					del_queue(temp,yuv);
					del_queue(temp,yuv);
					pthread_mutex_unlock(&pmet.lock);	
				}
		}
	pthread_exit((void*)0);
}