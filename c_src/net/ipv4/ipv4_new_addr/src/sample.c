#include "ipv4_new_addr.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

ipv4_new_addr_cb_t ipv4_new_addr_cb_impl(int err_code,
                                         uint32_t new_addr,
                                         void *p_cookie)
{
    struct in_addr addr;
    addr.s_addr = new_addr;
    if( !err_code )
      printf("new_addr=%s\n", inet_ntoa(addr));
    else
      printf("err\n");
    return NULL;
}


int main()
{
    int fd;
    fd = ipv4_new_addr_open("eth0", (ipv4_new_addr_cb_t)ipv4_new_addr_cb_impl, 2, NULL);
    
    while( getchar() != 'x' ) ;

    ipv4_new_addr_close(fd);

    return 0;
}
