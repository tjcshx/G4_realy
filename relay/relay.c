/*********************************************************************************
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  relay.c
 *    Description:  This file is ctrl relay off or on.
 *                 
 *        Version:  1.0.0(07/05/2020)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "07/05/2020 10:59:29 PM"
 *                 
 ********************************************************************************/
#include "relay.h"
static inline void relay_init()
{
    wiringPiSetup();         //初始化wiringPiSetup
    pinMode(RELAY, OUTPUT);  //将管脚定义为输出管脚
}

int main()
{
    relay_init();

    while(1)
    {
        realy(ON);
        sleep(5);
        realy(OFF);
        sleep(3);

    }
}

int realy(int cmd)
{

    if (cmd == ON) 
        digitalWrite(RELAY,ON);//高电平，灯亮 
    else if(cmd == OFF)
        digitalWrite(RELAY,OFF); //高电平，灯亮

    return 0;
}

