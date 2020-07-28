/*********************************************************************************
 *      Copyright:  (C) 2020 ysn
 *                  All rights reserved.
 *
 *       Filename:  moquito.c
 *    Description:  This file is mosquit API. 
 *                 
 *        Version:  1.0.0(27/07/20)
 *         Author:  tianjincheng <473892093@qq.com>
 *      ChangeLog:  1, Release initial version on "27/07/20 08:06:42"
 *                 
 ********************************************************************************/

#include "mosquitto_sub.h"
#include "mosquitto.h"
static int g_stop = 0;

static void print_usage (char *progname);
static void connect_callback (struct mosquitto *mosqut, void *obj, int rc);
static void disconnect_callback (struct mosquitto *mosqut, void *obj, int rc);
static void message_callback (struct mosquitto *mosqut,void *obj, const struct mosquitto_message *msg);
static void subscribe_callback (struct mosquitto *mosqut, void *obj, int mid, int qos_count, const int *granted_qos);
static void mosquitto_sub_clean (struct mosquitto *mosqut, void (disconnect_callback)(struct mosquitto *mosqut, void *, int));

/**************************************************************************************
 *  Description: Start to mosquitto subscribe func.
 *   Input Args: 
 *               mosqut: The mosquitto lib provide the struct of mosquitto.
 *               mosquitto :The uesr provide arguments.
 *  Output Args: No.
 * Return Value: 0:SUCCESS; other: ERROR.
 *************************************************************************************/
int mosquitto_sub_start (struct mosquitto *mosqut, mosquitto_t *mosquitto)
{
    int               rv = 1;

    rv = mosquitto_lib_init();
    if (rv)
    {
        printf("Init mosquitto lib error:%s\n", strerror(errno));
        return rv;
    }

    mosqut = mosquitto_new(mosquitto->client_id, true, (void *)mosquitto);
    if (!mosqut)
    {
        printf("Mosquitto new error:%s\n", strerror(errno));
        return rv;
    }
    else
        printf("Mosquitto new success.\n ");

    mosquitto_connect_callback_set(mosqut, connect_callback);
    mosquitto_message_callback_set(mosqut, message_callback);
    mosquitto_username_pw_set(mosqut, mosquitto->user, mosquitto->password);
    mosquitto_subscribe_callback_set(mosqut, subscribe_callback);

    while (!g_stop)
    {
        rv = mosquitto_connect(mosqut, mosquitto->host, mosquitto->port, mosquitto->keepalive);
        if (rv)
        {
            printf("Connect server error:%s\n", strerror(errno));
            sleep(1);
            continue;
        }
        else
            printf("mosquitto connect success.\n");

        mosquitto_loop_forever(mosqut, -1, 1);
        if( rv != MOSQ_ERR_SUCCESS ) 
        {
            printf("%s\n", strerror(errno));
            mosquitto_sub_clean(mosqut, disconnect_callback);
        }
    }

    return 0;
} /* ----- End of mosquitto_sub_start()  ----- */

/**************************************************************************************
 *  Description: To clean after mosquitto subscribe get disconnected.
 *   Input Args: 
 *               mosqut: The mosquitto lib provide the struct of mosquitto.
 *               disconnect_callback: The func pointer which is point disconnect_callback func.
 *  Output Args: No.
 * Return Value: No.
 *************************************************************************************/
static void mosquitto_sub_clean (struct mosquitto *mosqut, void (disconnect_callback)(struct mosquitto *mosqut, void *, int))
{
    mosquitto_disconnect_callback_set(mosqut, disconnect_callback);
    mosquitto_destroy(mosqut);
    mosquitto_lib_cleanup();
} /* ----- End of mosquitto_sub_clean()  ----- */

#ifdef MAIN 

int main (int argc, char **argv)
{
    int               rv = -1;
    mosquitto_t       mosquitto;
    struct mosquitto  *mosqut;

    memset(&mosquitto, 0, sizeof(mosquitto_t));

    rv = init_mosquitto_argc(argc, argv, &mosquitto);
    if (rv)
    {
        printf("Init mosquitto arguments error:%s\n", strerror(errno));
    }

    rv = mosquitto_sub_start(mosqut, &mosquitto);
    if (rv)
    {
        printf("mosquitto subscribe start error:%s\n", strerror(errno));
    }
    
    mosquitto_sub_clean(mosqut, disconnect_callback);
    return 0;
} /* ----- End of main() ----- */
#endif 

/**************************************************************************************
 *  Description: Init mosquitto argcs.
 *   Input Args: agrc & argv : form main func, mosquitto: the struct of mosquitto need init argc.
 *  Output Args: NO.
 * Return Value: -1: ERROR, 0: SUCCESS.
 *************************************************************************************/
int init_mosquitto_argc (int argc, char **argv, mosquitto_t *mosquitto)
{
    if (argc < 0 )
        return -1;

    int   ch = -1; 

    struct option        opts[] = { 
        {"--host",      required_argument, NULL, 'H'},
        {"--port",      required_argument, NULL, 'p'},
        {"--topic",     required_argument, NULL, 't'},
        {"--client_id", required_argument, NULL, 'i'},
        {"--user",      required_argument, NULL, 'u'},
        {"--password",  required_argument, NULL, 'P'},
        {"--keepalive", required_argument, NULL, 'l'},
        {"help",        no_argument,       NULL, 'h'},    
        {NULL,          0,                 NULL,  0 }
    };  

    while((ch=getopt_long(argc, argv, "H:p:t:i:u:P:l:h", opts, NULL)) != -1 )    
    {   
        switch(ch)
        {   
            case 'H':   
                mosquitto->host = optarg;
                break;
            case 'p':
                mosquitto->port = atoi(optarg);
                break;
            case 't':
                mosquitto->topic = optarg;
                break;
            case 'i':
                mosquitto->client_id = optarg;
                break;
            case 'u':
                mosquitto->user = optarg;
                break;
            case 'P':
                mosquitto->password = optarg;
                break;
            case 'l':
                mosquitto->keepalive = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return -1;
        }    
    }   

    mosquitto->recv_message = NULL;

    if (NULL == mosquitto->host || !mosquitto->keepalive ||!mosquitto->port || NULL == mosquitto->client_id || NULL == mosquitto->password || NULL == mosquitto->user || NULL == mosquitto->topic)
    {
        print_usage(argv[0]);
        return -1;
    }

    return 0;
} /* ----- End of inin_mosquitto_argc()  ----- */


/**************************************************************************************
 *  Description: Print usage help information.
 *   Input Args: The program name.
 *  Output Args: NO.
 * Return Value: NULL.
 *************************************************************************************/
static void print_usage (char *progname)
{
    printf("%s usage : \n", progname);
    printf("-H(--host):Input the Aliyun equioment host name.\n");
    printf("-p(--port):Input the Aliyun equioment host port.\n");
    printf("-i(--client_id):Input the Aliyun equioment client id.\n");
    printf("-t(--topic):Input the Aliyun equioment subscribe topic.\n");
    printf("-u(--user):Input the Aliyun equioment user name.\n");
    printf("-P(--password):Input the Aliyun equioment user's password name.\n");
    printf("-l(--keepalive):Input the keepalive.\n");
    printf("-h(--help): print this help information.\n");

    return ;
} /* ----- End of print_usage()  ----- */

/**************************************************************************************
 *  Description: mosquitto subscribe connect call back func.
 *   Input Args: 
 *               mosquitto: the struct of mosquitto argc;
 *               obj: mosquitto_new provide argument; 
 *               rc: 0: connect success, else connect error.
 *  Output Args: NO.
 * Return Value: -1: ERROR, 1: SUCCESS.
 *************************************************************************************/
static void connect_callback (struct mosquitto *mosqut, void *obj, int rc)
{
    if (NULL == obj)
        return ;

    mosquitto_t    *mosquitto;
    int            mid;
    mosquitto = (mosquitto_t *)obj;

    if (!rc)
    {
        if (mosquitto_subscribe(mosqut, &mid, mosquitto->topic, 0) != MOSQ_ERR_SUCCESS)
        {
            printf("Set the topic error:%s\n", strerror(errno));
            return ;
        }
        else 
            printf("subscribe topic:%s success.\n", mosquitto->topic);
    }
    else
        printf("connect_callback error\n");

} /* ----- End of connect_callback()  ----- */


/**************************************************************************************
 *  Description: mosquitto subscribe disconnect call back func.
 *   Input Args: 
 *               mosqut: the struct of mosquitto argc;
 *               obj: mosquitto_new provide argument;
 *               rc:  0: connect success, else connect error.
 *  Output Args: NO.
 * Return Value: -1: ERROR, 0: SUCCESS.
 *************************************************************************************/
static void disconnect_callback (struct mosquitto *mosqut, void *obj, int rc)
{
    g_stop = 1;
} /* ----- End of disconnect_callback()  ----- */


/**************************************************************************************
 *  Description: The subscribe callback func.
 *   Input Args: The system provide agrcs.
 *  Output Args: No;
 * Return Value: No.
 *************************************************************************************/
static void subscribe_callback (struct mosquitto *mosqut, void *obj, int mid, int qos_count, const int *granted_qos)
{
    printf("Call the function: on_subscribe\n");
} /* ----- End of subscribe_callback()  ----- */


/**************************************************************************************
 *  Description: The message callback func.
 *   Input Args: 
 *               mosqut: the struct of mosquitto;
 *               obj:    the subscribe new provide user argument;
 *               msg:    the recv broker informations.
 *  Output Args: No.
 * Return Value: No.
 *************************************************************************************/
static void message_callback (struct mosquitto *mosqut,void *obj, const struct mosquitto_message *msg)
{
    mosquitto_t *mosquitto;
    mosquitto = (mosquitto_t *)obj;
    memcpy(&mosquitto-> recv_message, (char *)&msg->payload, strlen((char *)msg->payload));
    printf("recv_message: %s\n", mosquitto->recv_message);

    if (0 == strcmp(msg->payload, "quit"))
        mosquitto_disconnect(mosqut);

} /* ----- End of message_callback()  ----- */


