#ifndef WHEEL_CONTROL_H
#define WHEEL_CONTROL_H

#include "comm.h"

extern "C"
{
#include "sys/time.h"
#include "signal.h"
}

typedef struct
{
    int leftSpeed;
    int rightSpeed;
}speeddatatype;

#define BUF_SIZE 300

#define RECEIVE_DATA_LENTH 10
#define SEND_DATA_LENTH 8

#define RECEIVE_DATA_HEAD 0x05
#define SEND_DATA_HEAD 0x04

#define UNTREATED_BUF_SIZE 300





class Wheel_Control
{
public:
    Wheel_Control();
    ~Wheel_Control();
    int OpenCom(const char* devPath);
	int CloseCom();
	short GetLSpeed();
	short GetRSpeed();
	//void TimeOutDispose(int sig);
    int Start(int uSec);
    void SetSpeedData(short leftSpeed, short rightSpeed); //�����������ٶȣ��������������ͣ������ɶ�ʱ���Զ����·��ͣ�
    int ReadSpeedData(speeddatatype* pSpeedData);//���ؾ��ϴθ��º�����ݴ���

private:
    struct itimerval timeValue, reloadTimeValue; 
	volatile short LeftSpeed;
	volatile short RightSpeed; 
};

int wheel_control_init();
int wheel_control_deInit();
Wheel_Control *wheel_control_get_handler();


#endif // WHEEL_CONTROL_H
