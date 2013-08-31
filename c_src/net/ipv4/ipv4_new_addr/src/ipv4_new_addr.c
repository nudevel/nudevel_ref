/*=============================================================================

Copyright (c) 2013, Naoto Uegaki
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/


//=============================================================================
// Includes
//=============================================================================
#include "ipv4_new_addr.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

//=============================================================================
// Local Macro/Type/Enumeration/Structure Definitions
//=============================================================================
struct ipv4_new_addr_man_t
{
    int alive;
    char ifname[IFNAMSIZ];
    pthread_t thrd;
    void *p_cookie;
    ipv4_new_addr_cb_t func;
    int polling_interval;
};


//=============================================================================
// Local Function/Variable Implementations
//=============================================================================
static void *_ipv4_new_addr_thread(struct ipv4_new_addr_man_t *p_man)
{
    int fd, err_code;
    struct ifreq ifr;
    uint32_t last_addr = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if( fd == -1 ) goto _err_end_;

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, p_man->ifname, IFNAMSIZ-1);

    while( p_man->alive ){
        uint32_t new_addr;

        err_code = ioctl(fd, SIOCGIFADDR, &ifr);
        if( err_code == -1 ) goto _err_end_;

        new_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
        if( new_addr != last_addr ){
            p_man->func( 0, new_addr, p_man->p_cookie );
            last_addr = new_addr;
        }
        sleep( p_man->polling_interval );
    }
    close(fd);

    return NULL;

  _err_end_:
    p_man->func( -1, 0, p_man->p_cookie );
    return NULL;
}


//=============================================================================
// Global Function/Variable Implementations
//=============================================================================
int ipv4_new_addr_open(const char *p_ifname,
                       const ipv4_new_addr_cb_t func,
                       int polling_interval,
                       const void *p_cookie)
{
    int err_code;
    struct ipv4_new_addr_man_t *p_man;
    pthread_attr_t  thrd_attr;

    if( func == NULL ) goto _err_end_;

    p_man = calloc(1, sizeof(struct ipv4_new_addr_man_t));
    if( p_man == NULL ) goto _err_end_;

    p_man->alive = 1;
    strncpy(p_man->ifname, p_ifname, IFNAMSIZ-1);
    p_man->func = func;
    p_man->polling_interval = polling_interval;
    p_man->p_cookie = (void*)p_cookie;

    err_code = pthread_attr_init( &thrd_attr );
    if( err_code!=0 ) goto _err_end_;
    err_code = pthread_create( &p_man->thrd, &thrd_attr, (void*)_ipv4_new_addr_thread, (void*)p_man);
    if( err_code!=0 ) goto _err_end_;
    err_code = pthread_attr_destroy( &thrd_attr );
    if( err_code!=0 ) goto _err_end_;

    return (int)p_man;
    
  _err_end_:
    if( p_man->thrd ) pthread_join( p_man->thrd, NULL );
    if( p_man ) free(p_man);

    return -1;
}

int ipv4_new_addr_close(int fd)
{
    int err_code;
    struct ipv4_new_addr_man_t *p_man = (struct ipv4_new_addr_man_t *)fd;

    p_man->alive = 0;
    err_code = pthread_join( p_man->thrd, NULL );
    if( err_code!=0 ) goto _err_end_;

    free(p_man);

    return 0;

  _err_end_:
    return -1;
}
