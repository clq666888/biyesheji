#ifndef __MODE_H
#define __MODE_H

enum mode_status        //窗帘开关状态
{
	open_state,
	close_state,
};

extern enum mode_status servo_state;	//0Ϊ�رգ�1Ϊ����
extern int mode_flag;		//1Ϊ�ֶ�ģʽ��2Ϊ�Զ�ģʽ��3Ϊ��ʱģʽ
extern int timer_start_hour,timer_start_minute,timer_end_hour,timer_end_minute;

void mode(void);

#endif
