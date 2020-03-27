#ifndef __VLAN_CFG_H__
#define __VLAN_CFG_H__

#include "common.h"

#define BR_VLAN_XPATH ("/bridge-vlan")
#define MAX_VLAN_LEN (16)

int vlan_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
	const char *path, sr_event_t event, uint32_t id, void *private_ctx);

#endif
