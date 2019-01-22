
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "netlink.h"

struct veth_request {
    struct nlmsghdr     header;
    char                buf[1024]; /* Ĭ��1024�㹻 */
};

static int __veth_addattr(struct rtattr *rta, int type, const void *data, int alen)
{
    int len = RTA_LENGTH(alen);// 4�ֽڶ��� len����ͷ��
    rta->rta_type = type;
    rta->rta_len = len;
    if (alen)
        memcpy(RTA_DATA(rta), data, alen);
    return 0;
}

/**
 * ����veth paire Ĭ��up״̬
 */
void veth_create() 
{
    struct netlink_handle handle;
    struct veth_request request;
    char *start = NULL;
    struct rtattr *rta = NULL;
    struct rtattr *rta_linkinfo = NULL;
    struct rtattr *rta_infodata = NULL;    
    struct veth_request req = {
        .header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
        .header.nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_EXCL,
        .header.nlmsg_type = RTM_NEWLINK
    };

    netlink_open(&handle, 0);

    start = req.buf;
    /* netlink msg Ҫ��4�ֽڶ��� */    
    struct ifinfomsg *ifmsg_header = (struct ifinfomsg *)start;
    ifmsg_header->ifi_family = AF_UNSPEC;
    ifmsg_header->ifi_flags = IFF_UP; // up
    ifmsg_header->ifi_change = IFF_UP; // up
    start += RTA_ALIGN(sizeof(struct ifinfomsg));

    /* ��������: ���ñ���veth �˿����� */
    struct rtattr *rta = (struct rtattr *)start;
    __veth_addattr(rta, IFLA_IFNAME, "cveth0", strlen("cveth0"));
    start += rta->rta_len;

    /* ��������: ����link info��Ϣ */
    rta_linkinfo = (struct rtattr *)start;
    rta_linkinfo->rta_type = IFLA_LINKINFO;

    /* ��������: ����kind */
    struct rtattr *rta = (struct rtattr *)start;
    __veth_addattr(rta, IFLA_INFO_KIND, "veth", strlen("veth"));
    start += rta->rta_len;

    /* ��������: ����link info��Ϣ */
    rta_infodata = (struct rtattr *)start;
    rta_infodata->rta_type = IFLA_INFO_DATA;



    rta_infodata->rta_rta_len = start - (char*)rta_infodata;


    rta_linkinfo->rta_rta_len = start - (char*)rta_linkinfo;
    
    netlink_send(struct netlink_handle *handle, struct nlmsghdr *buffer);
    
    netlink_close(&handle);
}

void veth_up() 
{
    
}

void veth_config() 
{
    
}

void veth_network_namespace() 
{
    
}

