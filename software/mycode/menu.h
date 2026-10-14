#ifndef __MENU_H
#define __MENU_H

extern int page;

void main_page(void);
int menu1(void);
void menu2_mode(void);
int menu2_threshold_setting(void);
void menu3_threshold(int flag);	//flag为1表示更改照明阈值，为2更改湿度阈值
void menu2_date_setting(void);
void menu2_timer_setting(void);

#endif
