/*********************************************************************************
 *      Copyright:  (C) 2020 shx
 *                  All rights reserved.
 *
 *       Filename:  comport.c
 *    Description:  This file comport c file.
 *                 
 *        Version:  1.0.0(07/07/2020)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "07/07/2020 02:54:51 PM"
 *                 
 ********************************************************************************/
#include "comport.h"
#include "log.h"

static int g_stop = 0;

static inline void  install_signal();
int   sigaction(int signum, const struct sigaction *act,struct sigaction *oldact);
static inline void  sig_handler(int signum);
static inline void  print_usage(char *paogname);
static inline void  adjust_buf(char *buf);

#ifdef MAIN
int main(int argc, char **argv)
{
    int       rv = -1;
    char      send_msg[BUF_SIZE] = {0};
    fd_set    rset;
    comport_t comport;

    install_signal();
    log_open();

    memset(&comport, 0, sizeof(comport_t));
    /* 初始化结构体参数 */
    rv  = init_uart_agcs(argc, argv, &comport);
    if (rv < 0)
    {
        log_error("init uart arguments error\n");
        return -1;
    }
    else 
        log_info("init uart arguments done.\n");


    /* 打开串口 */
    rv = open_uart(&comport);
    if (rv < 0)
    {
        log_error("open device failed:%s.\n", strerror(errno));
        return -1;
    }
    else 
        log_info("Open uart[%s] done.\n", comport.device_name);

    /* 初始化串口属性 */
    rv = set_uart_attr(&comport);
    if (rv < 0)
    {
        log_error("set opt failed\n");
        return -1;
    }
    else 
        log_info("set uart attr success.\n");


    while (!g_stop)
    {
#if 0
        /* 使用多进程 */
        pid_t pid = -1;
        pid = fork();
        if(pid == 0)
        {
            memset(send_msg,0,sizeof(send_msg));

            /*  从标准输入读取命令 */
            fgets(send_msg,sizeof(send_msg),stdin);

            adjust_buf(send_msg);
            comport.len = strlen(send_msg);
            memcpy(&comport.send_data, send_msg, BUF_SIZE);

            if(send_data(&comport) < 0)
            {
                log_error("Write failed.\n");
                goto CLEAN;
            }
            else 
                log_info("send data done.\n");

            fflush(stdin);
        }
        else if (pid > 0)
        {
            sleep(1);
            rv = recv_data(&comport);
            if(rv < 0)
            {
                log_error("Read failed:%s\n",strerror(errno));
                break;
            }

            fflush(stdout);
        }
        else 
        {
            log_error("fork eror:%s\n", strerror(errno));
            break;
        }
#endif 
        /* 使用select */
        FD_ZERO(&rset);
        FD_SET(comport.fd, &rset);
        FD_SET(STDIN_FILENO, &rset);

        rv = select(comport.fd + 1, &rset, NULL, NULL, NULL);
        if (rv < 0)
        {
            log_error("select error:%s\n", strerror(errno));
            goto CLEAN;
        }
        else if (0 == rv)
        {
            log_info("select timeout\n");
        }

        if (FD_ISSET(STDIN_FILENO,&rset))
        {
            memset(send_msg,0,sizeof(send_msg));

            /*  从标准输入读取命令 */
            fgets(send_msg,sizeof(send_msg),stdin);

            adjust_buf(send_msg);
            comport.len = strlen(send_msg);
            memcpy(&comport.send_data, send_msg, BUF_SIZE);

            if(send_data(&comport) < 0)
            {
                log_error("Write failed.\n");
                goto CLEAN;
            }
            else 
                log_info("send data done.\n");

            fflush(stdin);
        }

        if (FD_ISSET(comport.fd, &rset))
        {


            rv = recv_data(&comport);
            if(rv < 0)
            {
                log_error("Read failed:%s\n",strerror(errno));
                break;
            }

            fflush(stdout);
        }

    }

    close_uart(&comport);
    FD_CLR(STDIN_FILENO, &rset);
    FD_CLR(comport.fd, &rset);

CLEAN:
    close_uart(&comport);
    FD_CLR(STDIN_FILENO, &rset);
    FD_CLR(comport.fd, &rset);

    return 0;
}
#endif 

int init_uart_agcs(int argc, char **argv, comport_t *comport)
{   
    if (NULL == comport)
    {
        log_error("Illegal input arguments.\n");
        return -1;
    }

    int   ch = -1;
    struct option        opts[] = {
        {"device",   required_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},          
        {NULL, 0, NULL, 0}
    };

    while((ch=getopt_long(argc, argv, "d:h", opts, NULL)) != -1 )    
    {
        switch(ch)
        {
            case 'd':   
                comport->device_name = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
        }           
    }
    if(!comport->device_name)        
    {
        print_usage(argv[0]);
        return 0;        
    }

    comport->nSpeed = 115200;
    comport->nBits  = 8;
    comport->nEvent = 'N';
    comport->nStop = 1;
}

static inline void adjust_buf(char *buf)
{
    int i = strlen(buf);
    strcpy(&buf[i-1],"\r");
}

static inline void  print_usage(char *paogname)
{
    printf("%s usage : \n", paogname);
    printf("-d(--device_name):Input the device name you want to using..\n");
    printf("-h(--help): print this help information.\n");

    return ;
}



/* 初始化串口 */
int set_uart_attr(comport_t *comport)
{
    if (NULL == comport)
    {
        log_error("Illegal input agruments.\n");
        return -1;
    }

    struct termios newtio;
    struct termios oldtio;

    memset(&newtio, 0, sizeof(newtio));
    memset(&oldtio, 0, sizeof(oldtio));

    if (tcgetattr(comport->fd,&oldtio))
    {
        log_error("SetupSerial oldtio error:%s\n", strerror(errno));
        close(comport->fd);
        return -1;
    }

    if (tcgetattr(comport->fd,&newtio))
    {
        log_error("SetupSerial newtio error:%s\n", strerror(errno));
        close(comport->fd);
        return -1;
    }



    /*  修改控制模式，保证程序不会占用串口，及启动接收器，能够从串口中读取输入数据 */
    newtio.c_cflag |= CLOCAL | CREAD;

    /*   CSIZE字符大小掩码，将与设置databits相关的标致位置零  */
    newtio.c_cflag &= ~CSIZE;

    /*   
     * ICANON: 标准模式
     *  ECHO: 回显所输入的字符
     * ECHOE: 如果同时设置了ICANON标志，ERASE字符删除前一个所输入的字符，WERASE删除前一个输入的单词
     * ISIG: 当接收到INTR/QUIT/SUSP/DSUSP字符，生成一个相应的信号
     *
     * */
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    newtio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /*   
     * BRKINT: BREAK将会丢弃输入和输出队列中的数据(flush)，并且如果终端为前台进程组的控制终端，则BREAK将会产生一个SIGINT信号发送>到这个前台进程组
     * ICRNL: 将输入中的CR转换为NL
     * INPCK: 允许奇偶校验
     * ISTRIP: 剥离第8个bits
     * IXON: 允许输出端的XON/XOF流控
     **/

    /*  OPOST: 表示处理后输出，按照原始数据输出 */ 
    newtio.c_oflag &= ~(OPOST);

    /* 第二部：设置数据位数 */

    switch(comport->nBits)

    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    /* 第三步：设置奇偶校验位 */
    switch(comport->nEvent)
    {
        case 'O':    //奇校验
            newtio.c_cflag |= PARENB;            
            newtio.c_cflag |= PARODD;
            newtio.c_cflag |= (INPCK | ISTRIP);
        case'E':     //偶校验
            newtio.c_cflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':    //无奇偶校验位
            newtio.c_cflag &= ~PARENB;
            break;
    }

    /* 第四步：设置波特率 */
    switch (comport->nSpeed)
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;

    }

    /* 第五步：设置停止位 */
    if (comport->nStop == 1)
    {
        newtio.c_cflag &= ~CSTOPB;
    }
    else if (comport->nStop == 2)
    {
        newtio.c_cflag |= CSTOPB;
    }

    /* 第六步：设置等待时间和最小接收字符*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN]  = 0;

    /* 处理未接收字符*/
    tcflush(comport->fd, TCIFLUSH);

    /* 激活新配置*/
    if ((tcsetattr(comport->fd, TCSANOW, &newtio)) != 0)
    {
        log_error("com set error:%s\n", strerror(errno));
        return -1;
    }
    log_info("set uart attr done!\n");

    return 0;


}

/* 打开串口 */
int open_uart(comport_t *comport)
{
    int rv = -1;

    if ((comport->fd = open(comport->device_name, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    {
        log_error("Open device failure:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int recv_data(comport_t *comport)
{
    char  buff[BUF_SIZE] = {0};
    int   rv = -1;
    short count = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(comport->fd, &fds);

    rv  = select(comport->fd + 1, &fds, NULL, NULL, NULL);
    if (rv < 0)
    {
        log_error("Rcev data error:%s\n", strerror(errno));
    }
    rv  = read(comport->fd, buff, BUF_SIZE);
    if (rv < 0)
    {
        log_error("read error:%s.\n", strerror(errno));
    }

    printf("%s",buff);


    return 0;
}

int send_data(comport_t *comport, char *send_data, int data_len)
{
    if (NULL == comport)
    {
        log_error("Illegal input arguments.\n");
    }

    int  rv;

    rv = write(comport->fd, comport->recv_data, comport->len);
    if (rv == data_len)
    {
        log_info("send %s doen.", send_data);
    }
    else if (rv < 0)
    {
        log_error("write error\n");
        return -1;
    }

    return 0;
}

void  install_signal()  
{
    struct sigaction act, oldact; 
    act.sa_handler = sig_handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGINT|SIGTERM);
    act.sa_flags = 0; 
    printf("The programe statr running,and will print mssage into[%s].\n",  LOGFILENAME);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTERM, &act, 0);
}

void sig_handler(int signum)      
{                                  
    if (signum == SIGINT || SIGTERM)          
    {
        g_stop = 1;                
        log_info("Get singal SIGINT/SIGTERM singal,and the porgram will exit.\n");
        sleep(1);
    }
}          

void close_uart(comport_t *comport)
{
    close(comport->fd);
}
