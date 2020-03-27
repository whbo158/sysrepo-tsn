#ifndef __IP_CFG_H__
#define __IP_CFG_H__

#include "common.h"

#define IPV4_XPATH ("/ietf-ip:ipv4")
#define IP_MODULE_NAME ("ietf-ip")

struct inet_cfg
{
	struct in_addr ip;
	struct in_addr mask;
	char ifname[IF_NAME_MAX_LEN];
};

static struct inet_cfg sinet_conf;


int ip_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
	const char *path, sr_event_t event, uint32_t id, void *private_ctx);

#endif
