/*********************************************************************************
 *      Copyright:  (C) 2020 tianjincheng<473892093@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  proc.c
 *    Description:  This file is process API.
 *                 
 *        Version:  1.0.0(20/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "20/07/20 04:26:06"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>


#include "proc.h"

proc_signal_t     g_signal={0};

void proc_sighandler(int sig)
{
    switch(sig)
    {
        case SIGINT:
            g_signal.stop = 1;
            break;

        case SIGTERM:
            g_signal.stop = 1;
            break;
        case SIGSEGV:
            break;

        case SIGPIPE:
            break;

        default:
            break;
    }
}


void install_proc_signal(void)
{
    struct sigaction sigact, sigign;


    /*   Initialize the catch signal structure. */
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = proc_sighandler;

    /*   Setup the ignore signal. */
    sigemptyset(&sigign.sa_mask);
    sigign.sa_flags = 0;
    sigign.sa_handler = SIG_IGN;

    sigaction(SIGTERM, &sigact, 0); /*   catch terminate signal "kill" command */
    sigaction(SIGINT,  &sigact, 0); /*   catch interrupt signal CTRL+C */
    sigaction(SIGPIPE, &sigact, 0); /*   catch broken pipe */
}


/*  ****************************************************************************
 *  FunctionName: daemonize
 *  Description : Set the programe runs as daemon in background
 *  Inputs      : nodir: DON'T change the work directory to / :  1:NoChange 0:Change 
 *  noclose: close the opened file descrtipion or not 1:Noclose 0:Close
 *  Output      : NONE
 *  Return      : NONE
 *  *****************************************************************************/
void daemonize(int nochdir, int noclose)
{ 
    int retval, fd; 
    int i; 

    /*   already a daemon */ 
    if (1 == getppid()) 
        return; 

    /*   fork error */
    retval = fork(); 
    if (retval < 0) exit(1); 

    /*   parent process exit */
    if (retval > 0)
        exit(0); 

    /*   obtain a new process session group */
    setsid(); 

    if (!noclose)
    {
        /*   close all descriptors */
        for (i = getdtablesize(); i >= 0; --i)
        {

            close(i);
        } 

        /*   Redirect Standard input [0] to /dev/null */
        fd = open("/dev/null", O_RDWR); 

        /*  Redirect Standard output [1] to /dev/null */
        dup(fd);  

        /*  Redirect Standard error [2] to /dev/null */
        dup(fd);  
    } 

    umask(0); 

    if (!nochdir)
        chdir("/"); 

    return;
}

/*  ****************************************************************************
 *  FunctionName: record_daemon_pid
 *  Description : Record the running daemon program PID to the file "pid_file"
 *  Inputs      : pid_file:The record PID file path
 *  Output      : NONE
 *  Return      : 0: Record successfully  Else: Failure
 *  *****************************************************************************/
int record_daemon_pid(const char *pid_file)
{ 
    struct stat fStatBuf; 
    int fd = -1; 
    int mode = S_IROTH | S_IXOTH | S_IRGRP | S_IXGRP | S_IRWXU;
    char ipc_dir[64] = { 0 }; 

    strncpy(ipc_dir, pid_file, 64); 

    /*  dirname() will modify ipc_dir and save the result */ 
    dirname(ipc_dir);  

    /*  If folder pid_file PATH doesnot exist, then we will create it" */
    if (stat(ipc_dir, &fStatBuf) < 0) 
    { 
        if (mkdir(ipc_dir, mode) < 0) 
        { 
            return -1; 
        } 

        (void)chmod(ipc_dir, mode); 
    } 

    /*   Create the process running PID file */ 
    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if ((fd = open(pid_file, O_RDWR | O_CREAT | O_TRUNC, mode)) >= 0)
    {
        char pid[PID_ASCII_SIZE]; 
        snprintf(pid, sizeof(pid), "%u\n", (unsigned)getpid()); 
        write(fd, pid, strlen(pid)); 
        close(fd); 

    } 
    else 
    {
        return -1;
    }

    return 0;
}

/*  ****************************************************************************
 *  FunctionName: get_daemon_pid
 *  Description : Get the daemon process PID from the PID record file "pid_file"
 *  Inputs      : pid_file: the PID record file
 *  Output      : NONE
 *  Return      : pid_t: The daemon process PID number
 *  *****************************************************************************/
pid_t get_daemon_pid(const char *pid_file)
{ 
    FILE *f; 
    pid_t pid; 

    if ((f = fopen(pid_file, "rb")) != NULL)
    { 
        char pid_ascii[PID_ASCII_SIZE]; 
        (void)fgets(pid_ascii, PID_ASCII_SIZE, f); 
        (void)fclose(f); 
        pid = atoi(pid_ascii); 
    } 
    else
    {
        return -1;
    } 
    return pid;
}     

/*  ****************************************************************************
 *  FunctionName: check_daemon_running
 *  Description : Check the daemon program already running or not
 *  Inputs      : pid_file: The record running daemon program PID
 *  Output      : NONE
 *  Return      : 1: The daemon program alread running   0: Not running
 *  *****************************************************************************/
int check_daemon_running(const char *pid_file)
{
    int retVal = -1; 
    struct stat fStatBuf;

    retVal = stat(pid_file, &fStatBuf); 
    if (0 == retVal) 
    { 
        pid_t pid = -1; 

        pid = get_daemon_pid(pid_file);
        if (pid > 0)  /*   Process pid exist */
        { 
            if ((retVal = kill(pid, 0)) == 0) 
            { 
                return 1; 
            } 
            else   /*  Send signal to the old process get no reply. */ 
            { 
                remove(pid_file); 
                return 0; 
            } 
        } 
        else if (0 == pid) 
        { 
            remove(pid_file); 
            return 0; 
        } 
        else  /*  Read pid from file "pid_file" failure */
        { 
            return 1; 
        } 
    } 

    return 0;
}

/*  ****************************************************************************
 *  FunctionName: stop_daemon_running
 *  Description : Stop the daemon program running
 *  Inputs      : pid_file: The record running daemon program PID
 *  Output      : NONE
 *  Return      : 1: The daemon program alread running   0: Not running
 *  *****************************************************************************/
int stop_daemon_running(const char *pid_file)
{
    pid_t            pid = -1; 
    struct stat      fStatBuf;

    if ( stat(pid_file, &fStatBuf) < 0)
        return 0;

    pid = get_daemon_pid(pid_file);
    if (pid > 0)  /*   Process pid exist */
    { 
        while ( (kill(pid, 0) ) == 0) 
        { 
            kill(pid, SIGTERM);
            sleep(1);
        } 

        remove(pid_file); 
    } 

    return 0;
}



/*  ****************************************************************************
 *  FunctionName: set_daemon_running
 *  Description : Set the programe running as daemon if it's not running and record its PID to the pid_file.
 *  Inputs      : pid_file: The record running daemon program PID
 *  Output      : NONE
 *  Return      : 0: Successfully. 1: Failure
 *  *****************************************************************************/
int set_daemon_running(const char *pid_file)
{ 
    daemonize(1, 1); 

    if (record_daemon_pid(pid_file) < 0) 
    { 
        return -2;
    }

    return 0;
}


void thread_stop(char *prompt)
{
    if(prompt)

    g_signal.threads --;
    pthread_exit(NULL);
}

int thread_start(pthread_t * thread_id, thread_body worker_func, void *thread_arg)
{
    int        retval = 0;

    pthread_attr_t thread_attr; 

    /*  Initialize the thread  attribute */
    retval = pthread_attr_init(&thread_attr); 
    if(retval)
        return -1;

    /*  Set the stack size of the thread */
    retval = pthread_attr_setstacksize(&thread_attr, 120 * 1024); 
    if(retval)
        goto CleanUp;

    /*  Set thread to detached state:Don`t need pthread_join */
    retval = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED); 
    if(retval)
        goto CleanUp;

    /*  Create the thread */
    retval = pthread_create(thread_id, &thread_attr, worker_func, thread_arg);
    if(retval)
        goto CleanUp;


    g_signal.threads++;

CleanUp:
    /*  Destroy the  attributes  of  thread */
    pthread_attr_destroy(&thread_attr); 
    return retval;
}


void exec_system_cmd(const char *format, ...)
{
    char                cmd[256];
    va_list             args;
    memset(cmd, 0, sizeof(cmd)); 

    va_start(args, format); 
    va_end(args); 

    system(cmd);
}
