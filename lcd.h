#ifndef _lcd_h_
#define _lcd_h_
int lcd_init(void);
int lcd_uninit(void);
void lcd_draw_point(int x,int y,int color);
void lcd_draw_block(int x0,int y0,int lenth,int high,int color);
void lcd_draw_full_circle(int x0,int y0,int r,int color);
void lcd_draw_empty_circle(int x0,int y0,int r,int color);
void draw_word(unsigned char *name,int x0,int y0, int len,int size,int color);
void show_yuyv_rgb(char *buf);
#endif 