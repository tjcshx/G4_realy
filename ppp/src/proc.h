/********************************************************************************
 *      Copyright:  (C) 2020 tianjincheng<473892093@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  proc.h
 *    Description:  This head file for Linux process API
 *    
 *
 *        Version:  1.0.0(20/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "20/07/20 04:32:23"
 *                 
 ********************************************************************************/

#ifndef  _PROC_H_
#define  _PROC_H_
#include <signal.h>
 
#define PID_ASCII_SIZE  11
 
typedef struct proc_signal_s
{
        int       signal;
            unsigned  stop;     /*  0: Not term  1: Stop  */
                int       threads;  /*  threads counter */
}  proc_signal_t;
 
extern proc_signal_t     g_signal;
extern void install_proc_signal(void);
 
extern void daemonize(int nochdir, int noclose);
extern int record_daemon_pid(const char *pid_file);
extern pid_t get_daemon_pid(const char *pid_file);
extern int check_daemon_running(const char *pid_file);
extern int stop_daemon_running(const char *pid_file);
extern int set_daemon_running(const char *pid_file);
 
extern void exec_system_cmd(const char *format, ...);
 
typedef void *(* thread_body)(void *thread_arg);
extern int thread_start(pthread_t * thread_id, thread_body worker_func, void *thread_arg);
 
extern void thread_stop(char *prompt);
#endif   /* ----- #ifndef _PROC_H_  ----- */

