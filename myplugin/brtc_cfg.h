#ifndef __BRTC_CFG_H__
#define __BRTC_CFG_H__

#include "common.h"

#define BR_VLAN_XPATH ("/bridge-vlan")
#define BR_TC_XPATH ("/nxp-bridge-vlan-tc-flower:traffic-control")
#define MAX_VLAN_LEN (16)
#define MAX_PARA_LEN (32)
#define MAX_ACTION_LEN (128)
#define MAX_CMD_LEN (512)

#define SUB_ITEM_NONE		(0)
#define SUB_ITEM_QDISC		(1)
#define SUB_ITEM_FILTER		(2)

int brtc_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
	const char *path, sr_event_t event, uint32_t id, void *private_ctx);

#endif
