#ifndef COMM_H
#define COMM_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <memory.h>

#define BUF_MAX_SIZE 200

typedef void (*dataDisposeCallback)(char *pData, int size);

const int SPRRD_LIST[] = {B115200, B38400, B19200, B9600};

enum BAUDS{
    BAUD115200,
    BAUD38400,
    BAUD19200,
    BAUD9600
};

class Comm
{
public:
    Comm();
    ~Comm();
    int OpenCom(const char* pDevName);
    void CloseCom();
    int SetComParameter(enum BAUDS baud, char dataBits, char stopBits, char parity);
    int SendData(const unsigned char *pData, int iDataLength);
    int ReadData(dataDisposeCallback DataDispose);

private:
    int iComFd;
    bool bComConnectState;
    char dataBuf[BUF_MAX_SIZE + 1];
};

#endif // COMM_H
