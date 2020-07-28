/********************************************************************************
 *      Copyright:  (C) 2020 shx
 *                  All rights reserved.
 *
 *       Filename:  relay.h
 *    Description:  This head file relay c file.
 *
 *        Version:  1.0.0(07/07/2020)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "07/07/2020 11:28:09 AM"
 *                 
 ********************************************************************************/
#ifndef  _RELAY_H_
#define  _RELAY_H_
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <wiringPi.h>
#include <unistd.h>
#include <errno.h>

#define time_out 5   //dealy time 
#define RELAY    28  //使用的树莓派管脚
enum
{
        OFF,   //低电平，灯灭
        ON     //高电平，灯亮
};

extern int realy(int cmd);
#endif   /* ----- #ifndef _RELAY_H_  ----- */

