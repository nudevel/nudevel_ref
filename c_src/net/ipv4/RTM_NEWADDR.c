//======================================================================
// RTM_NEWADDR to detect the event of IP address changing from the kernel through RTNETLINK.
//
// This source code was copied from the below.
//   URL  : http://stackoverflow.com/questions/579783/how-to-detect-ip-address-change-programmatically-in-linux
//   Title: How to detect IP address change programmatically in Linux?
//======================================================================

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

int
main()
{
    struct sockaddr_nl addr;
    int sock, len;
    char buffer[4096];
    struct nlmsghdr *nlh;

    if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        perror("couldn't open NETLINK_ROUTE socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("couldn't bind");
        return 1;
    }

    nlh = (struct nlmsghdr *)buffer;
    while ((len = recv(sock, nlh, 4096, 0)) > 0) {
        while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE)) {
            if (nlh->nlmsg_type == RTM_NEWADDR) {
                struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
                struct rtattr *rth = IFA_RTA(ifa);
                int rtl = IFA_PAYLOAD(nlh);

                while (rtl && RTA_OK(rth, rtl)) {
                    if (rth->rta_type == IFA_LOCAL) {
                        uint32_t ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
                        char name[IFNAMSIZ];
                        if_indextoname(ifa->ifa_index, name);
                        printf("%s is now %d.%d.%d.%d\n",
                               name,
                               (ipaddr >> 24) & 0xff,
                               (ipaddr >> 16) & 0xff,
                               (ipaddr >> 8) & 0xff,
                               ipaddr & 0xff);
                    }
                    rth = RTA_NEXT(rth, rtl);
                }
            }
            nlh = NLMSG_NEXT(nlh, len);
        }
    }
    return 0;
}

