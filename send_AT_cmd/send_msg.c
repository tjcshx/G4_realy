/*********************************************************************************
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  send_msg.c
 *    Description:  This file is using at cmd to send_message.
 *                 
 *        Version:  1.0.0(14/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "14/07/20 07:16:29"
 *                 
 ********************************************************************************/

#include "send_msg.h"
#include "comport.h"
#include "log.h"

static void sig_handler(int signum);
static void  install_signal();
static int g_stop = 0;
static int check_sim(int fd);
static int check_send_cmd(int fd, char *cmd, int cmd_len);
int main (int argc, char **argv)
{
    int        fd;
    char       select;
    char       send_msg[BUF_SIZE] = {0};
    fd_set     rset;
    comport_t  comport;
    int        rv = -1;


    log_open();
    install_signal();
    rv = init_uart_agcs(argc, argv, &comport);
    if (rv < 0)
    {
        log_error("init uart arguments error\n");
        return -1;
    }
    else
        log_info("init uart arguments done.\n");

    rv = open_uart(&comport);
    if (rv < 0)
    {
        log_error("open device failed:%s.\n", strerror(errno));
        return -1;
    }
    else
        log_info("Open uart[%s] done.\n", comport.device_name);

    /*  初始化串口属性 */
    rv = set_uart_attr(&comport);
    if (rv < 0)
    {
        log_error("set opt failed\n");
        return -1;
    }
    else
        log_info("set uart attr success.\n");

    while(!g_stop)
    {
        printf("enter your select:  's' is send message,  'c' is call number.\n");

        select=getchar();/* 去掉回车符*/
        switch(select)
        {
            case 's':
                check_sim(comport.fd);
                send_message(comport.fd);
                break;
            case 'c':
                check_sim(comport.fd);
                call_number(comport.fd);
                break;
            default:
                break;
        }
    }


    close_uart(&comport);

    return 0;
}
static int check_sim(int fd)
{
    char csq[64] = "at+csq\r";
    char *cpin = "at+cpin?\r";
    char *creg = "at+creg?\r";
    int  rv = -1;

    rv = check_send_cmd(fd, cpin, strlen(cpin));
    if (rv < 0)
    {
        printf("send cpin cmd error:%s\n", strerror(errno));
    }

    rv = check_send_cmd(fd, creg, strlen(creg));
    if (rv < 0)
    {
        printf("send cpin cmd error:%s\n", strerror(errno));
    }

    rv = check_send_cmd(fd, csq, strlen(csq));
    if (rv < 0)
    {
        printf("send cpin cmd error:%s\n", strerror(errno));
    }
}
static int check_send_cmd(int fd, char *cmd, int cmd_len)
{
    if (fd < 0 || NULL == cmd)
        return -1;

    char read_buf[64] = {0};
    int  rv = -1;

    rv = write(fd, cmd, strlen(cmd));
    if (rv < 0)
    {
        return -1;
    }

    sleep(1);

    memset(read_buf, 0, 64);
    rv = read(fd, read_buf, 64);
    if (rv < 0)
    {
        return -1;
    }
    
    if (strpbrk("OK", read_buf))
    {
        printf("%s\n", read_buf);
        return 0;
    }
    else 
        return -1;
}

/* *************************************************************************************
 * 功能：sim7110模块打电话
 * 实现：ATD+号码+;
 * *************************************************************************************/
int call_number (int fd)
{
    getchar();   /* 将缓冲区回车吃掉,输入c后的回车*/

    int count=0;
    char call[20]="atd";    /* 打电话的AT指令 */
    char number[20];
    char reply[128];

    printf("enter you call number\n");
    if(NULL==fgets(number,20,stdin))      /* 输入电话号码，其中fgets在读入一个字符串后在字符串尾端默认加入\n字符*/
        return -1;           /* 这个语句的功能可以用gets实现，区别在于 fgets 读入的含 "\n"（最后一个字符），gets 不含 "\n"*/

    while(strlen(number)!=12)           
    {
        printf("please again number\n");
        if(NULL==fgets(number,20,stdin))
            return -1;
        if(count==3)
            return -1;
        count++;
    }

    number[strlen(number)-1]='\0'; /* 将刚才fgets读入的字符串尾端去除\n字符*/
    strcat(call,number);     
    strcat(call,";\r");            /*  \r是换行字符[atd电话号码;\r]*/

    write(fd,call,strlen(call));   /* 向串口拨打号码*/
    printf("write %s\n",call);
    sleep(3);  

    memset(reply,0,sizeof(reply));
    read(fd,reply,sizeof(reply));   /* AT指令*/
    printf("%s\n",reply);

    printf("number is calling,please press 'a' hang up \n");

    while('a'!=getchar())
        printf("please again input 'a' to hung up \n");

    memset(call,0,sizeof(call));
    strcpy(call,"ATH\r"); 
    write(fd,call,strlen(call)); /* 挂断电话 */
    sleep(1);
    memset(reply,0,sizeof(reply));
    read(fd,reply,sizeof(reply)); /* 数据*/
    printf("%s\n",reply);

    return 0;
}

/* ***************************************
 * 功能: EC20模块发短信函数
 * 实现: AT+CMGF=1 (回车)                
 *       AT+CMGS="号码" (回车)    
 *       >   (Ctrl + z)
 * ****************************************/

int send_message (int fd)
{
    int   count = 0;
    char  cmgf[]= "at+cmgf=1\r";    /* 设置短信发送模式为text,只能发送英文短信，PDU发送中文短信 （0-PDU;1-text）*/
    char  cmgs[128]="at+cmgs=\"";   /* AT+CMGS="号码"  \:连接字符串*/
    char  send_number[16];          /* 电话号码*/
    char  message[128]; 
    char  reply[128];

    getchar();   /* 吃掉缓冲区回车,输入s后的回车*/
    printf("enter send_message number :\n");
    if(NULL==fgets(send_number,16,stdin))
        exit(0);
    while(12!=strlen(send_number))
    {
        getchar();
        printf("please again input number\n");
        if(NULL==fgets(send_number,16,stdin))
            return -1;
        if(count==3)
            return -1;
        count++;
    }

    send_number[strlen(send_number)-1]='\0';   /* 去除字符串末端读入的换行符\n;*/
    strcat(cmgs,send_number);
    strcat(cmgs,"\"\r");    /* cmgs:at+cmgs="号码"*/

    printf("enter send_message :\n");
    if(NULL==fgets(message,128,stdin))
        exit(0);

    message[strlen(message)-1]='\0';
    /* "\x1a对应的控制字符为:SUB:意为后面只跟可打印输出的字符*/
    strcat(message,"\x1a"); /* 可以打印的*/

    /*  write 模式 */

    write(fd,cmgf,strlen(cmgf));
    printf("write %s\n",cmgf);
    sleep(2);

    memset(reply,0,sizeof(reply));
    read(fd,reply,sizeof(reply));
    printf("%s\n",reply);

    /*  write 命令 */
    write(fd,cmgs,strlen(cmgs));
    printf("write %s\n",cmgs);
    sleep(5);

    /* write 信息*/
    write(fd,message,strlen(message));
    printf("write %s\n",message);
    sleep(4);
    memset(reply,0,sizeof(reply));
    read(fd,reply,sizeof(reply));
    printf("%s\n",reply);

    return 0;
}
static void  install_signal()  
{      
    struct sigaction act, oldact; 
    act.sa_handler = sig_handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGINT|SIGTERM);
    act.sa_flags = 0; 
    printf("The programe statr running now...\n");
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTERM, &act, 0);
}      

static void sig_handler(int signum)      
{                                  
    if (signum == SIGINT || SIGTERM)          
    {  
        g_stop = 1;                
        printf("Get singal SIGINT/SIGTERM singal,and the porgram will exit.\n");
        sleep(1);
    }  
}          
