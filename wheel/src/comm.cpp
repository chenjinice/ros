#include "comm.h"
#include <iostream>
#include <stdlib.h>

Comm::Comm()
{
    iComFd = 0;
    bComConnectState = false;
}

Comm::~Comm()
{
    CloseCom();
}

int Comm::OpenCom(const char *pDevName)
{
    return iComFd = open(pDevName, O_RDWR | O_SYNC | O_NOCTTY | O_NDELAY);
}

void Comm::CloseCom()
{
    close(iComFd);
}

int Comm::SetComParameter(enum BAUDS baud,char dataBits, char stopBits, char parity)
{
    struct termios opt;

    bzero(&opt, sizeof(opt));

    if(tcgetattr(iComFd, &opt) != 0)
    {

        return -1;
    }

    tcflush(iComFd, TCIOFLUSH);
    cfsetispeed(&opt, SPRRD_LIST[baud]);
    cfsetospeed(&opt, SPRRD_LIST[baud]);
    if(tcsetattr(iComFd, TCSANOW, &opt) != 0)
    {
        return -2;
    }
    tcflush(iComFd, TCIOFLUSH);

    opt.c_cflag &= ~CRTSCTS;//不使用流控制
    opt.c_cflag |= (CLOCAL | CREAD);//一般必设标志
    opt.c_cflag &= ~CSIZE;//初始化标志位

    switch(dataBits)
    {
    case 7:
        opt.c_cflag |= CS7;
        break;
    case 8:
        opt.c_cflag |= CS8;
        break;
    default:
        printf("Error: DataBits error!!\a\n");
        return -3;
    }

//    可以用于c_iflag的宏为：

//    BRKINT：在一行中检测到中断(break)条件时产生一个中断
//    IGNBRK：在一行中忽略中断条件
//    INCRNL：将接收到的回车转换为换行
//    IGNCR：忽略接收到的因车
//    INLCR：将接收到的新行转换为回车
//    IGNPAR：忽略带有奇偶检验误差的字符
//    INPCK：在接收到的字符上执行奇偶校验
//    PARMRK：标记奇偶校验误差
//    ISTRIP：去除所有的输入字符
//    IXOFF：在输入上允许软件流控制
//    IXON：在输出上允许软件流控制

//    我们可以在c_oflag使用的标记有：

//    OPOST：打开输出处理
//    ONLCR：将输出的新行转换为回车/换行对
//    OCRNL：将输出的回车转换为新行
//    ONOCR：在第0列不输出回车
//    ONLRET：新行也需要一个回车
//    OFILL：发送填充字符来提供延时
//    OFDEL：使用DEL作为填充字符，而不是NULL
//    NLDLY：新行延时选择
//    CRDLY：回车延时选择
//    TABDLY：Tab延时选择
//    BSDLY：Backspace延时选择
//    VTDLY：垂直Tab延时选择
//    FFDLY：换页延时选择

    opt.c_cflag |= (tcflag_t)  (CLOCAL | CREAD);
    opt.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                         ISIG | IEXTEN); //|ECHOPRT

    opt.c_oflag &= (tcflag_t) ~(OPOST);
    opt.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);

    //设置校验位
    switch(parity)
    {
    case 'n':
    case 'N':   //无校验
        opt.c_cflag &= ~PARENB;
        opt.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O':   //奇校验
        opt.c_cflag |= (PARENB | PARODD);
        opt.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E':   //偶校验
        opt.c_cflag |= PARENB;
        opt.c_cflag &= ~PARODD;
        opt.c_iflag |= INPCK;
        break;
    default:
        printf("Error: parity set error!!\a\n");
        return -4;
    }

    switch(stopBits)
    {
    case 1:
        opt.c_cflag &= ~CSTOPB;
        break;
    case 2:
        opt.c_cflag |= CSTOPB;
        break;
    default:
        printf("Error: stopBits set error!!\a\n");
        return -5;
    }

    opt.c_oflag &= ~OPOST;//原始数据输出

    //设置等待时间和最小接收字符
    opt.c_cc[VTIME] = 1;
    opt.c_cc[VMIN] =  0;

    //如果发生数据溢出，接受数据但不再读取
    tcflush(iComFd, TCIFLUSH);

    //激活配置，将修改好的数据写入串口中
    if(tcsetattr(iComFd, TCSANOW, &opt) != 0)
    {
        printf("Error: Com set error!!\a\n");
        return -6;
    }

    bComConnectState = true;
    return 0;
}

int Comm::SendData(const unsigned char *pData, int iDataLength)
{
    int len = 0;

    if(!bComConnectState)
    {
        printf("Error:Wheel Comm is not Connect!!\a\n");
        return -1;
    }

    if((pData == NULL) || (iDataLength == 0))
    {
        return -1;
    }

#if 0
	int i = 0;
	for(i = 0; i < iDataLength; i++)
		printf("%02X  ",pData[i]);

	printf("\n");
#endif
    len = write(iComFd, pData, iDataLength);

	tcflush(iComFd,TCOFLUSH);

	

    if(len == iDataLength)
    {
    	//printf("[DBG CHT]: data write success!!!, all %d bytes sent!!\a\n", iDataLength);
        return 0;
    }
	else if(len > 0)
    {
        printf("Error: data write error!!\a\n");
        return -1;
    }
	else
    {
    	printf("[DBG CHT]: data write no success, only %d bytes sent!!\a\n", len);
		printf("[DBG CHT]: wheel write error: %s\n", strerror(errno));
        return len;
    }
}

int Comm::ReadData(dataDisposeCallback DataDispose)
{
    int iLen = 0;

    if(!bComConnectState)
    {
        printf("Error:Wheel Comm is not Connect!!\a\n");
        return -1;
    }

	if(NULL == DataDispose)
	{
		printf("DataDispose is NULL\a\n");
        return -1;
	}

	memset(dataBuf,0x00,BUF_MAX_SIZE + 1);

    iLen = read(iComFd, dataBuf, BUF_MAX_SIZE);

    if(iLen < 0)
    {
        printf("read error: %s\n", strerror(errno)); //转换错误码为对应的错误信息
        return -1;
    }
	else if(iLen > 0)
    {
		//printf("%d bytes read out, databuf is %s\n\n", iLen, dataBuf);
	
        DataDispose(dataBuf, iLen);
        return 1;
    }
	else if(0 == iLen)
    {
		printf("Error: no data!!\n");
        return 2;
    }
}
