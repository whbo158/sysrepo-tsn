#include "common.h"

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

bool is_del_oper(sr_session_ctx_t *session, char *path)
{
	int rc = SR_ERR_OK;
	bool ret = false;
	sr_change_oper_t oper;
	sr_val_t *old_value;
	sr_val_t *new_value;
	sr_change_iter_t *it;
	char err_msg[MSG_MAX_LEN] = {0};

	rc = sr_get_changes_iter(session, path, &it);
	if (rc != SR_ERR_OK) {
		snprintf(err_msg, MSG_MAX_LEN, "Get changes from %s failed",
			 path);
		sr_set_error(session, err_msg, path);
		printf("ERROR: Get changes from %s failed\n", path);
		return false;
	}

	rc = sr_get_change_next(session, it, &oper, &old_value, &new_value);
	if (rc == SR_ERR_NOT_FOUND)
		ret = false;
	else if (oper == SR_OP_DELETED)
		ret = true;
	return ret;
}
