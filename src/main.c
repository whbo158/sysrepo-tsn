/**
 * @file main.c
 * @author Xiaolin He
 * @brief Application to configure TSN function based on sysrepo datastore.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <cjson/cJSON.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"
#include "main.h"
#include "qbv.h"
#ifndef TEST_PLUGIN
#include "qbu.h"
#include "file_mon.h"
#include "cb_streamid.h"
#include "qci.h"
#endif

#include "ip_cfg.h"
#include "vlan_cfg.h"
#include "mac_cfg.h"
#include "brtc_cfg.h"

static uint8_t exit_application;

static void sigint_handler(int signum)
{
	exit_application = 1;
}
#ifndef TEST_PLUGIN
/* tsn_operation_monitor_cb()
 * file callback
 */
void tsn_operation_monitor_cb(void)
{
}

struct sr_tsn_callback file_clbks = {
	.callbacks_count = 1,
	.callbacks = {
		{
			.f_path = "/tmp/tsn-oper-record.json",
			.func = tsn_operation_monitor_cb
		},
	}
};

void check_pid_file(void)
{
	char pid_file[] = "/var/run/sysrepo-tsn.pid";
	char str[20] = { 0 };
	int ret = 0;
	int fd;

	/* open PID file */
	fd = open(pid_file, O_RDWR | O_CREAT, 0640);
	if (fd < 0) {
		printf("Unable to open sysrepo PID file '%s': %s.\n",
		       pid_file, strerror(errno));
		exit(1);
	}

	/* acquire lock on the PID file */
	if (lockf(fd, F_TLOCK, 0) < 0) {
		if (EACCES == errno || EAGAIN == errno) {
			printf("Another instance of sysrepo-tsn %s\n",
			       "daemon is running, unable to start.");
		} else {
			printf("Unable to lock sysrepo PID file '%s': %s.",
			       pid_file, strerror(errno));
		}
		exit(1);
	}

	/* write PID into the PID file */
	snprintf(str, 20, "%d\n", getpid());
	ret = write(fd, str, strlen(str));
	if (-1 == ret) {
		printf("ERR: Unable to write into sysrepo PID file '%s': %s.",
		       pid_file, strerror(errno));
		exit(1);
	}
}
#endif
int main(int argc, char **argv)
{
	int rc = SR_ERR_OK;
	sr_conn_ctx_t *connection = NULL;
	sr_session_ctx_t *session = NULL;
	sr_subscription_ctx_t *if_subscription = NULL;
	sr_subscription_ctx_t *bridge_subscription = NULL;
	sr_subscription_ctx_t *inet_subscription = NULL;
	char path[XPATH_MAX_LEN];
	sr_subscr_options_t opts;

	exit_application = 0;
#ifndef TEST_PLUGIN
	/* Check pid file */
	check_pid_file();

	/* Init file callbacks */
	sr_tsn_fcb_init();

	/* Init tsn mutex */
	init_tsn_mutex();
#endif
	/* Connect to sysrepo */
	rc = sr_connect(SR_CONN_DEFAULT, &connection);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_connect: %s\n", sr_strerror(rc));
		goto cleanup;
	}

	/* Start session */
	rc = sr_session_start(connection, SR_DS_RUNNING, &session);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_session_start: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to QBV subtree */
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE;
	snprintf(path, XPATH_MAX_LEN, "%s", IF_XPATH);
	strncat(path, QBV_GATE_PARA_XPATH, XPATH_MAX_LEN - strlen(path));
	rc = sr_module_change_subscribe(session, "ietf-interfaces", path,
					qbv_subtree_change_cb, NULL, 0,
					opts, &if_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

#ifndef TEST_PLUGIN
	/* Subscribe to QBV subtree */
	snprintf(path, XPATH_MAX_LEN, IF_XPATH);
	strncat(path, QBV_MAX_SDU_XPATH, XPATH_MAX_LEN);
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE | SR_SUBSCR_ENABLED;
	rc = sr_module_change_subscribe(session, "ietf-interfaces", path,
					qbv_subtree_change_cb, NULL, 0,
					opts, &if_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to QBU subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", IF_XPATH);
	strncat(path, QBU_XPATH, XPATH_MAX_LEN - strlen(path));
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE | SR_SUBSCR_ENABLED;
	rc = sr_module_change_subscribe(session, "ietf-interfaces", path,
					qbu_subtree_change_cb, NULL, 0,
					opts, &if_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to CB-StreamID subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, CB_STREAMID_XPATH, XPATH_MAX_LEN - strlen(path));
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE | SR_SUBSCR_ENABLED;
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					cb_streamid_subtree_change_cb,
					NULL, 0, opts, &bridge_subscription);

	/* Subscribe to QCI-Stream-Filter subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, QCISF_XPATH, XPATH_MAX_LEN - strlen(path));
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE | SR_SUBSCR_ENABLED;
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					qci_sf_subtree_change_cb,
					NULL, 0, opts, &bridge_subscription);

	/* Subscribe to QCI-Stream-Gate subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, QCISG_XPATH, XPATH_MAX_LEN - strlen(path));
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE | SR_SUBSCR_ENABLED;
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					qci_sg_subtree_change_cb,
					NULL, 0, opts, &bridge_subscription);

	/* Subscribe to QCI-Flow-Meter subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, QCIFM_XPATH, XPATH_MAX_LEN - strlen(path));
	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE | SR_SUBSCR_ENABLED;
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					 qci_fm_subtree_change_cb,
					 NULL, 0, opts, &bridge_subscription);
#endif

	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE;

	/* Subscribe to IP_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", IF_XPATH);
	strncat(path, IPV4_XPATH, XPATH_MAX_LEN - 1 - strlen(path));
	rc = sr_module_change_subscribe(session, "ietf-interfaces", path,
					ip_subtree_change_cb, NULL, 0,
					opts, &inet_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to VLAN_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, BR_VLAN_XPATH, XPATH_MAX_LEN - 1 - strlen(path));
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					vlan_subtree_change_cb, NULL, 0,
					opts, &inet_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to MAC_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_ADDR_XPATH);
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					mac_subtree_change_cb, NULL, 0,
					opts, &inet_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to BR_TC_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, BR_TC_XPATH, XPATH_MAX_LEN - 1 - strlen(path));
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					brtc_subtree_change_cb, NULL, 0,
					opts, &inet_subscription);
	if (rc != SR_ERR_OK){
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Loop until ctrl-c is pressed / SIGINT is received */
	signal(SIGINT, sigint_handler);
	signal(SIGPIPE, SIG_IGN);
	while (!exit_application)
		sleep(1000);  /* Or do some more useful work... */

cleanup:
#ifndef TEST_PLUGIN
	destroy_tsn_mutex();
#endif

	if (if_subscription)
		sr_unsubscribe(if_subscription);
	if (bridge_subscription)
		sr_unsubscribe(bridge_subscription);
	if (inet_subscription)
		sr_unsubscribe(inet_subscription);

	if (session)
		sr_session_stop(session);
	if (connection)
		sr_disconnect(connection);

	return rc;
}
