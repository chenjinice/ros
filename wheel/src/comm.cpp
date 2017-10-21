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

    opt.c_cflag &= ~CRTSCTS;//��ʹ��������
    opt.c_cflag |= (CLOCAL | CREAD);//һ������־
    opt.c_cflag &= ~CSIZE;//��ʼ����־λ

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

//    ��������c_iflag�ĺ�Ϊ��

//    BRKINT����һ���м�⵽�ж�(break)����ʱ����һ���ж�
//    IGNBRK����һ���к����ж�����
//    INCRNL�������յ��Ļس�ת��Ϊ����
//    IGNCR�����Խ��յ�����
//    INLCR�������յ�������ת��Ϊ�س�
//    IGNPAR�����Դ�����ż���������ַ�
//    INPCK���ڽ��յ����ַ���ִ����żУ��
//    PARMRK�������żУ�����
//    ISTRIP��ȥ�����е������ַ�
//    IXOFF�����������������������
//    IXON����������������������

//    ���ǿ�����c_oflagʹ�õı���У�

//    OPOST�����������
//    ONLCR�������������ת��Ϊ�س�/���ж�
//    OCRNL��������Ļس�ת��Ϊ����
//    ONOCR���ڵ�0�в�����س�
//    ONLRET������Ҳ��Ҫһ���س�
//    OFILL����������ַ����ṩ��ʱ
//    OFDEL��ʹ��DEL��Ϊ����ַ���������NULL
//    NLDLY��������ʱѡ��
//    CRDLY���س���ʱѡ��
//    TABDLY��Tab��ʱѡ��
//    BSDLY��Backspace��ʱѡ��
//    VTDLY����ֱTab��ʱѡ��
//    FFDLY����ҳ��ʱѡ��

    opt.c_cflag |= (tcflag_t)  (CLOCAL | CREAD);
    opt.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                         ISIG | IEXTEN); //|ECHOPRT

    opt.c_oflag &= (tcflag_t) ~(OPOST);
    opt.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);

    //����У��λ
    switch(parity)
    {
    case 'n':
    case 'N':   //��У��
        opt.c_cflag &= ~PARENB;
        opt.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O':   //��У��
        opt.c_cflag |= (PARENB | PARODD);
        opt.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E':   //żУ��
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

    opt.c_oflag &= ~OPOST;//ԭʼ�������

    //���õȴ�ʱ�����С�����ַ�
    opt.c_cc[VTIME] = 1;
    opt.c_cc[VMIN] =  0;

    //�����������������������ݵ����ٶ�ȡ
    tcflush(iComFd, TCIFLUSH);

    //�������ã����޸ĺõ�����д�봮����
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
        printf("read error: %s\n", strerror(errno)); //ת��������Ϊ��Ӧ�Ĵ�����Ϣ
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
