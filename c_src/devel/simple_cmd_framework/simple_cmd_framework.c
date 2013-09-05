#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

typedef enum {
    // for api
    cmd_no1_req = 0xbeaf0001,
    cmd_no1_ack = 0xdead0001,

    cmd_no2_req = 0xbeaf0002,
    cmd_no2_ack = 0xdead0002,

    // for eventing
    cmd_no6_req = 0xdead0006,
    cmd_no6_ack = 0xbeaf0006,
} cmd_e;

typedef struct {
    char x[10];
} cmd_no1_req_t;
typedef struct {
    char x[10];
} cmd_no1_ack_t;

typedef struct {
    char x[11];
} cmd_no2_req_t;
typedef struct {
    char x[11];
} cmd_no2_ack_t;

typedef struct {
    char x[12];
} cmd_no6_req_t;
typedef struct {
    char x[12];
} cmd_no6_ack_t;


typedef struct {
    cmd_e cmd;
    int fdw;
    int fdr;
    int len;
} cmd_conf_t;

typedef int (*event_handler_t)(cmd_e cmd_req, void *p_arg_req, cmd_e *p_cmd_ack, void **pp_arg_ack);

event_handler_t event_handler;

#define CMD_CONF_LIST_SIZE (6)
cmd_conf_t cmd_conf_list[CMD_CONF_LIST_SIZE] = {
    { cmd_no1_req, 0, 0, sizeof(cmd_no1_req_t) },
    { cmd_no1_ack, 0, 0, sizeof(cmd_no1_ack_t) },
    { cmd_no2_ack, 0, 0, sizeof(cmd_no2_ack_t) },
    { cmd_no2_req, 0, 0, sizeof(cmd_no2_req_t) },

    { cmd_no6_req, 0, 0, sizeof(cmd_no6_req_t) },
    { cmd_no6_ack, 0, 0, sizeof(cmd_no6_ack_t) },
};

int dummy_fdr, dummy_fdw;

int is_event(cmd_e cmd)
{
    switch(cmd){
      case cmd_no6_req:
        return 0;
      default:
        return -1;
    }
}

int open_fd(void)
{
    int i;
    for(i=0; i<CMD_CONF_LIST_SIZE; ++i){
        int pipefd[2], ret;

        ret = pipe(pipefd);
        if( ret != 0 ){
            printf("ERR %d\n", __LINE__);
            return -1;
        }
        
        cmd_conf_list[i].fdr = pipefd[0];
        cmd_conf_list[i].fdw = pipefd[1];
    }

    return 0;
}

int get_fdr(cmd_e cmd)
{
    int i;
    for(i=0; i<CMD_CONF_LIST_SIZE; ++i)
      if( cmd_conf_list[i].cmd == cmd )
        return cmd_conf_list[i].fdr;
    return 0;
}

int get_fdw(cmd_e cmd)
{
    int i;
    for(i=0; i<CMD_CONF_LIST_SIZE; ++i)
      if( cmd_conf_list[i].cmd == cmd )
        return cmd_conf_list[i].fdw;
    return 0;
}

int get_len(cmd_e cmd)
{
    int i;
    for(i=0; i<CMD_CONF_LIST_SIZE; ++i)
      if( cmd_conf_list[i].cmd == cmd )
        return cmd_conf_list[i].len;
    return 0;
}

int send_cmd(cmd_e cmd, void* p_arg)
{
    int ret;

    switch(cmd){
      case cmd_no1_req:
        cmd = cmd_no1_ack;
        break;
      case cmd_no2_req:
        cmd = cmd_no2_ack;
        break;
      default:
        break;
    }
    
    ret = write(dummy_fdw, &cmd, sizeof(cmd));
    if( ret != sizeof(cmd) ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }

    ret = write(dummy_fdw, p_arg, get_len(cmd));
    if( ret != get_len(cmd) ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }

    return 0;
}

int recv_cmd(cmd_e cmd, void* p_arg)
{
    char *p_buf = (char*)p_arg;
    int remain = get_len(cmd);

    while(1){
        int ret;
        
        ret = read( get_fdr(cmd),  p_buf, remain );
        if( ret > 0 ){
            remain -= ret;
            p_buf  += ret;
            if( remain == 0 ){
                break;
            } else if( remain < 0 ){
                printf("ERR %d\n", __LINE__);
                return -1;
            } else {
                /* nothing to do.  */;
            }
        } else {
            printf("ERR %d\n", __LINE__);
            return -1;
        }

    }
    return 0;
}

void* recv_thread(void *p_arg)
{
    while(1){
        cmd_e cmd;
        char buff[256];
        int ret;

        ret = read( dummy_fdr, &cmd, sizeof(cmd) );
        if( ret != sizeof(cmd) ){
            printf("ERR %d\n", __LINE__);
            continue;
        }
        ret = read( dummy_fdr, buff, get_len(cmd) );
        if( ret != get_len(cmd) ){
            printf("ERR %d\n", __LINE__);
            continue;
        }

        if( is_event(cmd) == 0 ){
            if( event_handler ){
                void *p_buff_ack;
                cmd_e cmd_ack;
                event_handler(cmd, (void*)buff, &cmd_ack,  &p_buff_ack);
                if( cmd_ack != 0 ){
                    // send_cmd(cmd_ack, p_buff_ack);
                }
            }
        } else {
            ret = write( get_fdw(cmd),  buff, get_len(cmd) );
            if( ret != get_len(cmd) ){
                printf("ERR %d\n", __LINE__);
                continue;
            }
        }
    }

    return NULL;
}

int api_no1(const char *p_in, char *p_out)
{
    int ret;
    cmd_no1_req_t req;
    cmd_no1_ack_t ack;

    strncpy(req.x, p_in, 10);
    ret =  send_cmd(cmd_no1_req, (void*)&req);
    if( ret != 0 ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }

    ret = recv_cmd(cmd_no1_ack, (void*)&ack);
    if( ret != 0 ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }
    strncpy(p_out, ack.x, 10);

    return 0;
}

event_handler_t event_handler_impl(cmd_e cmd_req, void *p_arg_req, cmd_e *p_cmd_ack, void **pp_arg_ack)
{
    switch(cmd_req){
      case cmd_no6_req:
        {
            cmd_no6_req_t *p_req = (cmd_no6_req_t *)p_arg_req;
            cmd_no6_ack_t ack;
            strcpy(ack.x, "test");
            *p_cmd_ack  = cmd_no6_ack;
            *pp_arg_ack = (void*)&ack;
        }
        break;
      default:
        break;
    }
    return NULL;
}



int main()
{
    int pipefd[2], ret;
    pthread_t thrd;

    ret = pipe(pipefd);
    if( ret != 0 ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }
    dummy_fdr = pipefd[0];
    dummy_fdw = pipefd[1];

    ret = open_fd();
    if( ret != 0 ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }

    ret = pthread_create(&thrd, NULL, recv_thread, NULL);
    if( ret != 0 ){
        printf("ERR %d\n", __LINE__);
        return -1;
    }

    event_handler = (event_handler_t)event_handler_impl;
    
    {
        char out[10];
        ret = api_no1("test", out);
        if( ret != 0 ){
            printf("ERR %d\n", __LINE__);
            return -1;
        }
        printf("%s\n", out);
    }
    
    return 0;
}
