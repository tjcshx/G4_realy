/*********************************************************************************
 *
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  ppp.c
 *    Description:  This file is ppp c file.
 *                 
 *        Version:  1.0.0(12/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "12/07/20 05:31:07"
 *                 
 ********************************************************************************/
#include "ppp.h"
#include "proc.h"
static int      g_pa_run   = 0;
static int      g_ch_stop  = 0;
static inline void child_handle(int signum);
static inline void parent_handle(int signum);

/**************************************************************************************
 *  Description: To check whether wifi and cable can surf the Internet successfully.
 *   Input Args: 
 *               ifaec:      The interface need to check ,such as WIFI is wlan0, cable is eth0, ppp is ppp.
 *               ping_addr:  Ping is used to check whether the network is routed ip addr,such as 4.2.2.2
 *  Output Args: No.
 * Return Value: -1:error. 0:success.
 *************************************************************************************/
static int ping_test(const char * iface,const char *ping_addr);

/**************************************************************************************
 *  Description: If the wifi and cable all can not surf the Internet, will use ppp to surf the Internet.
 *   Input Args: NO.
 *  Output Args: NO.
 * Return Value: -1: ERROR; 0: SUCCESS.
 *************************************************************************************/
static int ppp_start();

#ifdef MAIN
int main (int argc, char *argv[])
{
    nic_ping_test();

    return 0;
} 
#endif 

/**************************************************************************************
 *  Description: To check whether wifi and cable can surf the Internet successfully. 
 *   Input Args: No.
 *  Output Args: No.
 * Return Value: -1:error. 0:success.
 *************************************************************************************/

int nic_ping_test()
{
    int     rv = -1;
    while(1)
    {
        rv = ping_test("wlan0", "4.2.2.2");
        if (rv)
        {
            printf("Wifi doesn't work, and will check cable can surf the Internet...\n");
            continue;
        }
        else
        {
            printf("Wifi si working now..\n");
            return 0;
            break;
        }

        rv = ping_test("eth0", "4.2.2.2");
        if (rv)
        {
            printf("Cable doesn't work, and will use PPP to surf the Internet...\n");
            continue;
        }  
        else
        {  
            printf("Cable si working now..\n");
            return 0;
            break;
        }

        rv = ppp_start();
        if (rv)
        {
            printf("PPP also doesn't work, and will sleep 1 second continue to check wifi、cable and ppp.\n");
            sleep(1);
            continue;
        }
        else 
        {
            printf("PPP si working now..\n");                                           
            return 0;
            break;   
        }
    }
} /* ----- End of nic_ping_test()  ----- */


/**************************************************************************************
 *  Description: To check whether wifi and cable can surf the Internet successfully.
 *   Input Args: 
 *               ifaec:      The interface need to check ,such as WIFI is wlan0, cable is eth0, ppp is ppp.
 *               ping_addr:  Ping is used to check whether the network is routed ip addr,such as 4.2.2.2
 *  Output Args: No.
 * Return Value: -1:error. 0:success.
 *************************************************************************************/

static int ping_test(const char *iface, const char *ping_addr)
{
    if (NULL == iface || NULL == ping_addr )
    {
        printf("Illegal argument.");
        return -1;
    }

    FILE    *pPipe = NULL;
    char    buf[BUF_SIZE] = {0};
    char    info[BUF_SIZE] = "ping ";
    char    *tmp = NULL;
    int     loss = 0;
    strncat(info, ping_addr, strlen(ping_addr));
    strncat(info, " -I ", strlen("-I "));
    strncat(info, iface, strlen(iface));
    strncat(info, " -c 4 ", strlen("-c 4 "));


    pPipe = popen(info, "r" );


    memset(buf, 0, BUF_SIZE);
    fgets(buf, BUF_SIZE - 1, pPipe);

    tmp = strpbrk("received,", buf);
    if (tmp != NULL)
    {
        char *demli = "%";
        char * p = strtok(tmp, demli);
        loss = atoi(p + 1);

        if (loss < 80)
        {
            return 0;
        }
        else 
            return -1;
    }
    else
        return -1;
} /* ----- End of ping_test()  ----- */

/**************************************************************************************
 *  Description: If the wifi and cable all can not surf the Internet, will use ppp to surf the Internet.
 *   Input Args: NO.
 *  Output Args: NO.
 * Return Value: -1: ERROR; 0: SUCCESS.
 *************************************************************************************/
/*
 * 用信号量实现父子间进程的通信;
 * 父进程:主要检测ppp拨号是否可以上网；
 * 子进程:如果父进程检测到ppp拨号可以上网，则子进程进行ppp拨号上网.
 *
 */
static int ppp_start( )
{
    pid_t          pid;
    char           pid_file[64] = { 0 };
    int            child_status;

    signal(SIGUSR2, child_handle); //注册信号函数
    signal(SIGUSR1, parent_handle); //注册信号函数

    pid = fork();
    if (pid < 0)
    {
        printf("fork error:%s\n", strerror(errno));
        return -1;
    }
    else if (pid > 0)
    {

        while (!g_pa_run) 
        {
            sleep(1);
        }

        printf("The parent  statr to check ppp is ready...\n");
        while(1)
        {
            snprintf(pid_file, sizeof(pid_file), "/var/run/%s.pid", "ppp0");
            if( check_daemon_running(pid_file) )
            {
                printf("Programe already running\n");
                if (ping_test(" ppp0", "4.2.2.2"))
                {
                    printf("ppp erroe:%s\n", strerror(errno));
                    return -1;
                }
                else 
                {
                    printf("ppp is ready.\n");
                    break;
                }
            }
            else 
            {
                if (system("sudo pppd call rasppp > ppp_log&"))
                {
                    printf("system error:%s\n", strerror(errno));
                    return -1;
                }
                sleep(3);
                if (ping_test(" ppp0", "4.2.2.2"))
                {
                    printf("ppp erroe:%s\n", strerror(errno));
                    return -1;
                }
                else 
                {
                    printf("ppp is ready.\n");
                    break;
                }
            }
        }

        kill(pid, SIGUSR1);
        wait(&child_status);
    }
    else if (pid == 0)
    {
        kill(getppid(), SIGUSR2);

        while (!g_ch_stop)
        {
            sleep(3);
        }

        printf("The child will use ppp to surf the Internet.\n");
        snprintf(pid_file, sizeof(pid_file), "/var/run/%s.pid", "ppp0");
        if( 0 == check_daemon_running(pid_file) )
        {
            printf("Programe already running.\n");

            if (ping_test("ppp0", "4.2.2.2"))
            {
                return -1;
            }
            else 
            {
                printf("ppp is used to surf the Internet...");
                printf("\n");
            }
        }
        else 
        {
            system("sudo pppd call rasppp>ppp_log&");
            if (ping_test("ppp0", "4.2.2.2"))
            {
                return -1;
            }
            else 
            {

                printf("ppp is  used to surf the Internet...\n");
                printf("\n");
            }
        }
    }

    return 0;

} /* ----- End of ppp_start()  ----- */

static inline void  parent_handle(int signum)
{
    if (signum == SIGUSR1)
        g_ch_stop = 1;

}
static inline void child_handle(int signum)
{
    if (signum == SIGUSR2)
        g_pa_run = 1;
}
