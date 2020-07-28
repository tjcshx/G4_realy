/********************************************************************************
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  mosquitto_sub.h
 *    Description:  This head file is mosquitto_sub.
 *
 *        Version:  1.0.0(28/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "28/07/20 05:27:49"
 *                 
 ********************************************************************************/

#ifndef  _MOSQUITTO_SUB_H_
#define  _MOSQUITTO_SUB_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include "mosquitto.h"

#define BUF_SIZE   128
/* 
 * if define MAIN:     will use this file's main func to test.
 * if not define MAIN: will use the mian file's mian func to test.  
 * */

#define MAIN

typedef struct mosquitto_s 
{
    char               *host;
    int                port;
    char               *topic;
    char               *client_id;
    char               *user;
    char               *password;
    int                keepalive;
    char               *recv_message;

} mosquitto_t;  /* ---  end of struct mosquitto_s  ---*/

extern int init_mosquitto_argc (int argc, char **argv, mosquitto_t *mosquitto);
extern int mosquitto_sub_init (struct mosquitto *mosqut, mosquitto_t *mosquitto);

#endif   /* ----- #ifndef _MOSQUITTO_SUB_H_  ----- */

