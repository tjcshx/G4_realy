/********************************************************************************
 *      Copyright:  (C) 2020 shx
 *                  All rights reserved.
 *
 *       Filename:  comport.h
 *    Description:  This head file is about c comport.
 *
 *        Version:  1.0.0(07/07/2020)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "07/07/2020 02:55:31 PM"
 *                 
 ********************************************************************************/
#ifndef  _COMPORT_H_
#define  _COMPORT_H_ 

#include <termios.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
/* 
 *
 *|起始位|7/8位数据位|奇偶校验位|停止位|
 *
 *
 * */

#define   BUF_SIZE   1024
#define   MAIN

typedef struct st_comport 
{

    int          fd;
    const char   *device_name; //设备名称
    int          nBits;        //串口工作的位数7/8
    char         nEvent;       //奇偶校验位
    int          nStop;        //停止位
    int          nSpeed;
    char         send_data[BUF_SIZE];
    char         recv_data[BUF_SIZE];
    int          len;


} comport_t;
extern int  set_uart_attr(comport_t *comport);
extern int  open_uart(comport_t *comport);
extern int  send_data(comport_t *comport);
extern int  recv_data(comport_t *comport);
extern void close_uart(comport_t *comport);
#endif   /* ----- #ifndef _COMPORT_H_  ----- */
