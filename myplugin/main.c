#include "common.h" 
#include "sysrepo.h"
#include "sysrepo/xpath.h"

#include "ip_cfg.h"
#include "vlan_cfg.h"
#include "mac_cfg.h"
#include "brtc_cfg.h"

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

static uint8_t exit_application;

static void sigint_handler(int signum)
{
	exit_application = 1;
}

int main(int argc, char **argv)
{
	int rc = SR_ERR_OK;
	char path[XPATH_MAX_LEN];
	sr_subscr_options_t opts;
	sr_conn_ctx_t *connection = NULL;
	sr_session_ctx_t *session = NULL;
	sr_subscription_ctx_t *ip_subscription = NULL;

	exit_application = 0;

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

	opts = SR_SUBSCR_DEFAULT | SR_SUBSCR_CTX_REUSE;

	/* Subscribe to IP_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", IF_XPATH);
	strncat(path, IPV4_XPATH, XPATH_MAX_LEN - 1 - strlen(path));
	rc = sr_module_change_subscribe(session, "ietf-interfaces", path,
					ip_subtree_change_cb, NULL, 0,
					opts, &ip_subscription);
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
					opts, &ip_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}

	/* Subscribe to MAC_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BR_ADDRESS_XPATH);
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					mac_subtree_change_cb, NULL, 0,
					opts, &ip_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}
#if 1
	/* Subscribe to BR_TC_CFG subtree */
	snprintf(path, XPATH_MAX_LEN, "%s", BRIDGE_COMPONENT_XPATH);
	strncat(path, BR_TC_XPATH, XPATH_MAX_LEN - 1 - strlen(path));
	rc = sr_module_change_subscribe(session, "ieee802-dot1q-bridge", path,
					brtc_subtree_change_cb, NULL, 0,
					opts, &ip_subscription);
	if (rc != SR_ERR_OK) {
		fprintf(stderr, "Error by sr_module_change_subscribe: %s\n",
			sr_strerror(rc));
		goto cleanup;
	}
#endif
	/* Loop until ctrl-c is pressed / SIGINT is received */
	signal(SIGINT, sigint_handler);
	signal(SIGPIPE, SIG_IGN);
	while (!exit_application)
		sleep(1000);  /* Or do some more useful work... */

cleanup:
	if (ip_subscription)
		sr_unsubscribe(ip_subscription);

	if (session)
		sr_session_stop(session);
	if (connection)
		sr_disconnect(connection);

	return 0;
}
