#include "wheel_control.h"

#include   <iostream>   
using namespace std;

Comm* comPort = NULL;
unsigned int g_iUpdataCount = 0;
unsigned char dataBuf[BUF_SIZE];
static speeddatatype g_SpeedData;
static volatile short g_LeftSpeed = 0; 
static volatile short g_RightSpeed = 0;
// static FILE *spd_file_fd = 0; 

static pthread_mutex_t wheel_coder_lock;


Wheel_Control *pWheel = NULL;

int wheel_control_init()
{
	int i;

	const char portname[]= "/dev/ttyUSB-wheel"; 

/*	spd_file_fd = fopen("/home/chen/work/code/wheel_log","w");

	if(NULL == spd_file_fd)
    {
    	printf("spd_file_fd open error\n"); 
        return -1;//要返回错误代码
    }
*/
	pthread_mutex_init( &wheel_coder_lock, NULL);
	
	pWheel = new Wheel_Control();
    //i = pWheel->OpenCom("/dev/ttyUSB0");
    i = pWheel->OpenCom(portname);
    //cout << "open serial returned " << i << endl;
	printf("open serial %s returned %d!\n\n", portname, i);
	
	if(i != 0)
	{
		delete pWheel;
		return i;
	}
    pWheel->Start(10000);//10 000 us
	return 0;
}

int wheel_control_deInit()
{
	delete pWheel;
	pWheel = NULL;

	pthread_mutex_destroy( &wheel_coder_lock);
	//to do: close com
	

	return 0;
}

Wheel_Control *wheel_control_get_handler()
{
	return pWheel;
}


Wheel_Control::Wheel_Control()
{
    comPort = new Comm();
	LeftSpeed = 0;
	RightSpeed = 0;
}

Wheel_Control::~Wheel_Control()
{
    timeValue.it_value.tv_usec = 0;
    timeValue.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timeValue, &reloadTimeValue);

    delete comPort;
    comPort = NULL;
}


//comPort should be a member of Wheel_Control
int Wheel_Control::OpenCom(const char *devPath)
{
    if((NULL == devPath))
    {
        return -1;
    }
cout<<"test:"<<devPath<<endl;
    if(comPort->OpenCom(devPath) < 0)
        return -2;

    if(comPort->SetComParameter(BAUD38400, 8, 1, 'n') < 0)
        return -3;

	printf("%s opened success!\n\n", devPath);

    return 0;
}

int Wheel_Control::CloseCom()
{
    return 0;
}

int GetCoderSpeed(int *Vl, int *Vr)
{
	if( NULL == Vl || NULL == Vr )
		return -1;
//mutex needed?

	pthread_mutex_lock(&wheel_coder_lock); 
	*Vl = g_SpeedData.leftSpeed;
	*Vr = g_SpeedData.rightSpeed;
	pthread_mutex_unlock(&wheel_coder_lock); 

	return 0;
}

int SendSpeed(short leftSpeed, short rightSpeed)
{
    //dataBuf[SEND_DATA_LENTH];

	short Lspd = leftSpeed;
	short Rspd = rightSpeed;

    bzero(dataBuf, SEND_DATA_LENTH);

	//printf("send speed L = %04X, R = %04X\n", Lspd, Rspd);

    dataBuf[0] = SEND_DATA_HEAD;

    dataBuf[1] = (unsigned char)(Lspd >> 8);
    dataBuf[2] = (unsigned char)(Lspd & 0xFF);
    dataBuf[3] = (unsigned char)(Rspd >> 8);
    dataBuf[4] = (unsigned char)(Rspd & 0xFF);

    int verifyTemp = 0;
    for(int i = 0; i < (SEND_DATA_LENTH - 1); i++)
    {
        verifyTemp += dataBuf[i];
    }

    dataBuf[SEND_DATA_LENTH - 1] = (char)(verifyTemp & 0xff);

    return comPort->SendData(dataBuf, SEND_DATA_LENTH);
}

int g_iLen = 0;
char untreatedBuf[UNTREATED_BUF_SIZE];

void DataDispose(char *pData, int iDataLen)
{
   int iCountTemp = 0;//数据计数用


   memcpy(untreatedBuf + g_iLen, pData, iDataLen);//转移数据到缓存区

   g_iLen += iDataLen;
//   printf("g_iLen:%d\n", g_iLen);

   while(iCountTemp < g_iLen)
   {
       if(untreatedBuf[iCountTemp] == RECEIVE_DATA_HEAD)
       {
            //printf("find head!!\n");
           if((g_iLen - iCountTemp) >= RECEIVE_DATA_LENTH)//数据数量充足，继续处理数据
           {
                char iVerify = 0;

                for(int i = 0;i < (RECEIVE_DATA_LENTH - 1);i ++ )
                {
                    iVerify += untreatedBuf[iCountTemp + i];
//                    printf("data is:%02X\n", untreatedBuf[iCountTemp + i]);
//                    printf("verify data is:%02X\n", iVerify);
                }

//                printf("verify result:%02X\n", iVerify);
//                printf("verify result:%02X\n", untreatedBuf[iCountTemp + (RECEIVE_DATA_LENTH -1)]);
                if((iVerify & 0xff) == (untreatedBuf[iCountTemp + (RECEIVE_DATA_LENTH -1)] & 0xff))
                {
                	unsigned char status_byte5 = 0;
					unsigned char status_byte6 = 0;
					
					pthread_mutex_lock(&wheel_coder_lock); 
					
                	struct timeval tv;
					gettimeofday(&tv,0);

                    g_iUpdataCount++;

					
					
                    //解析数据
                    g_SpeedData.leftSpeed = (short)(((untreatedBuf[iCountTemp + 1] << 8) & 0xff00)
                                                    | (untreatedBuf[iCountTemp + 2] & 0x00ff));

                    g_SpeedData.rightSpeed = (short)(((untreatedBuf[iCountTemp + 3] << 8) & 0xff00)
                                                     | (untreatedBuf[iCountTemp + 4] & 0x00ff));

					status_byte5 = (unsigned char)(untreatedBuf[iCountTemp + 5]);
					status_byte6 = (unsigned char)(untreatedBuf[iCountTemp + 6]);
					
					pthread_mutex_unlock(&wheel_coder_lock);
					/*
					if(0 != status_byte5 || 0 != status_byte6)
					{
						//wheel report a error!
						printf("status_byte5: %02X, status_byte6: %02X\n", status_byte5, status_byte6 );
					}
					*/
                    //printf("I get a speed data:left  %d  right  %d\n", g_SpeedData.leftSpeed, g_SpeedData.rightSpeed);

//					fprintf(spd_file_fd,"%ld-%ld    L:%d R:%d \n", tv.tv_sec, tv.tv_usec, g_SpeedData.leftSpeed, g_SpeedData.rightSpeed);
					
                    iCountTemp += RECEIVE_DATA_LENTH;
                }
                else
                {
                    iCountTemp ++;
                }


            }
           else
           {
               char i[RECEIVE_DATA_LENTH];

               memcpy(i, untreatedBuf + iCountTemp, g_iLen - iCountTemp);//将数据暂存到
               memcpy(untreatedBuf, i, g_iLen - iCountTemp);//将未处理完的数据

               g_iLen = g_iLen - iCountTemp;//将未处理的数据长度存储

//               printf("Exit!!\n");
               break;
           }
       }
       else
       {
           iCountTemp++;
       }

   }
//   printf("iCountTemp:%d\n", iCountTemp);
   if(iCountTemp == g_iLen)
   {
       g_iLen = 0;
   }
}

static struct timeval pre_tv = {0};

void TimeOutDispose(int sig)
{
    if(comPort != NULL)
    {
        //printf("Time!!!!!\n");
//        comPort->SendData("111", 3);

		int diff = 0;

		struct timeval tv;
		gettimeofday(&tv,0);

		diff =( (tv.tv_sec - pre_tv.tv_sec) * 1000000 + (tv.tv_usec - pre_tv.tv_usec))/1000;
		
 
        comPort->ReadData(DataDispose);
		
		short LS = pWheel->GetLSpeed();
		short RS = pWheel->GetRSpeed();

		//printf("send speed L = %04X, R = %04X\n", LS, RS);
		
        SendSpeed(LS, RS);

		//printf("sec: %d usec: %d, diff ms is %d\n", tv.tv_sec, tv.tv_usec, diff);

		pre_tv = tv;
    }
}

short Wheel_Control::GetLSpeed()
{
	return LeftSpeed;
}

short Wheel_Control::GetRSpeed()
{
	return RightSpeed;
}



int Wheel_Control::Start(int uSec)
{
    signal(SIGALRM, TimeOutDispose);

    bzero(&timeValue, sizeof(timeValue));

    timeValue.it_value.tv_usec = uSec;
    timeValue.it_interval.tv_usec = uSec;
    timeValue.it_value.tv_sec = 0;
    timeValue.it_interval.tv_sec = 0;

    setitimer(ITIMER_REAL, &timeValue, &reloadTimeValue);
}

void Wheel_Control::SetSpeedData(short leftSpeed, short rightSpeed)
{
	
    g_LeftSpeed = leftSpeed;
    g_RightSpeed = rightSpeed;

	LeftSpeed = leftSpeed;
	RightSpeed = rightSpeed;

//	printf("get new speed, L = %d, R = %d\n", LeftSpeed, RightSpeed);
}

int Wheel_Control::ReadSpeedData(speeddatatype *pSpeedData)
{
    unsigned int iTemp = g_iUpdataCount;

    g_iUpdataCount = 0;

    memcpy(pSpeedData, &g_SpeedData, sizeof(g_SpeedData));

    return iTemp;
}

