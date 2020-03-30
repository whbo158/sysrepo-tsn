#ifndef __MAC_CFG_H__
#define __MAC_CFG_H__

#include "common.h"

#define MAC_ADDR_MAX_LEN	(32)
#define BR_ADDRESS_XPATH (BRIDGE_XPATH "/address")

int mac_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
	const char *path, sr_event_t event, uint32_t id, void *private_ctx);

#endif
