/**
 * @file qbv.h
 * @author Xiaolin He
 * @brief header file for qbv.c.
 *
 * Copyright 2019 NXP
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

#ifndef __QBV_H_
#define __QBV_H_

#ifdef TEST_PLUGIN

#define ptptime_t int64_t

#define QBV_TC_NUM (8)
#define MAX_CMD_LEN (512)
#define MAX_ENTRY_SIZE (1024)
#define SUB_PARA_LEN (64)

/* An entry for gate control list */
struct tsn_qbv_entry {
	/* Octet represent the gate states for the corresponding traffic
	 * classes.
	 * The MS bit corresponds to traffic class 7.
	 * The LS bit to traffic class 0.
	 * A bit value of 0 indicates closed;
	 * A bit value of 1 indicates open.
	 */
	uint8_t gate_state;

	/* A TimeInterval is encoded in 4 octets as a 32-bit unsigned integer,
	 * representing a number of nanoseconds.
	 */
	uint32_t time_interval;
};

/* The administrative/operation time and gate list */
struct tsn_qbv_basic {
	/* The administrative/operation value of the GateStates parameter for
	 * the Port.
	 * The bits of the octet represent the gate states for the
	 * corresponding traffic classes; the MS bit corresponds to traffic
	 * class 7, the LS bit to traffic class 0. A bit value of 0 indicates
	 * closed; a bit value of 1 indicates open.
	 * The value of this object MUST be retained
	 * across reinitializations of the management system.
	 */
	uint8_t gate_states;

	/* The administrative/operation value of the ListMax parameter for the
	 * port. The integer value indicates the number of entries (TLVs) in
	 * the AdminControlList. The value of this object MUST be retained
	 * across reinitializations of the management system.
	 */
	uint32_t control_list_length;

	/* The administrative/operation value of the AdminCycleTime
	 * parameter for the Port. The numerator and denominator together
	 * represent the cycle time as a rational number of seconds.  The value
	 * of this object MUST be retained across reinitializations of the
	 * management system.
	 */
	uint32_t cycle_time;

	/* The administrative/operation value of the CycleTimeExtension
	 * parameter for the Port. The value is an unsigned integer number of
	 * nanoseconds.
	 * The value of this object MUST be retained across reinitializations
	 * of the management system.
	 */

	uint32_t cycle_time_extension;

	/* The administrative/operation value of the BaseTime parameter for the
	 * Port.  The value is a representation of a PTPtime value, consisting
	 * of a 48-bit integer number of seconds and a 32-bit integer number of
	 * nanoseconds.
	 * The value of this object MUST be retained across reinitializations of
	 * the management system.
	 */
	ptptime_t base_time;

	/* admin_control_list represent the AdminControlList/OperControlList.
	 * The administrative version of the gate control list for the Port.
	 */
	struct tsn_qbv_entry *control_list;
};

struct tsn_qbv_conf {
	/* The GateEnabled parameter determines whether traffic scheduling is
	 * active (true) or inactive (false).  The value of this object MUST be
	 * retained across reinitializations of the management system.
	 */
	uint8_t gate_enabled;

	/* The maxsdu parameter denoting the maximum SDU size supported by the
	 * queue.
	 */
	uint32_t maxsdu;

	/* The ConfigChange parameter signals the start of a configuration
	 * change when it is set to TRUE. This should only be done when the
	 * various administrative parameters are all set to appropriate values.
	 */
	uint8_t config_change;

	/* The admin parameter signals the admin relate cycletime, basictime,
	 * gatelist paraters.
	 */
	struct tsn_qbv_basic admin;
};

#else
#include <tsn/genl_tsn.h>
#endif
#include "common.h"

#define QBV_GATE_PARA_XPATH "/ieee802-dot1q-sched:gate-parameters"
#define QBV_MAX_SDU_XPATH "/ieee802-dot1q-sched:max-sdu-table"
#define QBV_MODULE_NAME "ieee802-dot1q-sched"


struct sr_qbv_conf {
	bool qbv_en;
	struct tsn_qbv_conf *qbvconf_ptr;
	bool cycletime_f;
	bool basetime_f;
	struct cycle_time_s cycletime;
	struct base_time_s basetime;
};

int qbv_subtree_change_cb(sr_session_ctx_t *session, const char *path,
		sr_event_t event, void *private_ctx);

#endif
