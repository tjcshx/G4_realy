/********************************************************************************
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  ppp.h
 *    Description:  This head file is for ppp c file.
 *
 *        Version:  1.0.0(12/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "12/07/20 05:31:58"
 *                 
 ********************************************************************************/

#ifndef  _PPP_H_
#define  _PPP_H_
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>

#define BUF_SIZE 1024
/* *************************************************************************************
 *  Description: To check whether wifi and cable can surf the Internet successfully. 
 *   Input Args: No.
 *  Output Args: No.
 * Return Value: -1:error. 0:success.
 **************************************************************************************/
extern int nic_ping_test();

/* 如果使用本文件的main函数用于测试，则定义MAIN，如果只是调用本函数接口，则无需定义MAIN*/

#define MAIN

#endif   /* ----- #ifndef _PPP_H_  ----- */ 
