/**
 * @file mac_cfg.h
 * @author hongbo wang (hongbo.wang@nxp.com)
 * @brief header file for mac_cfg.c.
 *
 * Copyright 2019-2020 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MAC_CFG_H__
#define __MAC_CFG_H__

#include "common.h"

#define MAC_ADDR_LEN	(32)
#define BR_ADDRESS_XPATH (BRIDGE_XPATH "/address")

int mac_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
	const char *path, sr_event_t event, uint32_t id, void *private_ctx);

#endif
