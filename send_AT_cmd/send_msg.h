/********************************************************************************
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  send_msg.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(14/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "14/07/20 07:16:49"
 *                 
 ********************************************************************************/


#ifndef  _SEND_MSG_H_
#define  _SEND_MSG_H_
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


/* 设置串口参数 */
extern int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
/* 发送短信 */
extern int send_message(int fd);
/*  拨打电话 */
extern int call_number(int fd);
#endif   /* ----- #ifndef _SEND_MSG_H_  ----- */
