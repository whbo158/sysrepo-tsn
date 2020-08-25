/**
 * @file qci.c
 * @author Xiaolin He
 * @brief Implementation of Qci function based on sysrepo
 * datastore.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>

#include "common.h"
#include "main.h"
#include "qci.h"

void init_sf_list_node(struct std_qci_list *node, char *port, uint32_t sfid)
{
	node->apply_st = APPLY_NONE;
	if (!node->sf_ptr)
		return;

	snprintf(node->sf_ptr->port, IF_NAME_MAX_LEN, "%s", port);
	node->apply_st = APPLY_NONE;
	node->sf_ptr->sfconf.stream_handle_spec = -1;
	node->sf_ptr->sf_id = sfid;
	node->sf_ptr->sfconf.priority_spec = -1;
	node->sf_ptr->sfconf.stream_filter.flow_meter_instance_id = -1;
}

void init_sg_list_node(struct std_qci_list *node, char *port, uint32_t sgid)
{
	node->apply_st = APPLY_NONE;
	if (!node->sg_ptr)
		return;

	snprintf(node->fm_ptr->port, IF_NAME_MAX_LEN, "%s", port);
	node->sg_ptr->sg_id = sgid;
	node->sg_ptr->cycletime_f = false;
	node->sg_ptr->basetime_f = false;
	node->sg_ptr->sgconf.admin.init_ipv = -1;
}

void init_fm_list_node(struct std_qci_list *node, char *port, uint32_t fmid)
{
	node->apply_st = APPLY_NONE;
	if (!node->fm_ptr)
		return;

	snprintf(node->fm_ptr->port, IF_NAME_MAX_LEN, "%s", port);
	node->fm_ptr->fm_id = fmid;
}

struct std_qci_list *new_list_node(enum qci_type type, char *port,
		uint32_t id)
{
	struct std_qci_list *list;

	list = calloc(1, sizeof(struct std_qci_list));
	if (!list)
		return NULL;

	list->pre = NULL;
	list->next = NULL;

	switch (type) {
	case QCI_T_SF:
		list->sf_ptr = calloc(1, sizeof(struct std_sf));
		if (!list->sf_ptr) {
			free(list);
			return NULL;
		}

		init_sf_list_node(list, port, id);
		break;

	case QCI_T_SG:
		list->sg_ptr = calloc(1, sizeof(struct std_sg));
		if (!list->sg_ptr) {
			free(list);
			return NULL;
		}

		list->sg_ptr->sgconf.admin.gcl = malloc(MAX_ENTRY_SIZE);
		if (!list->sg_ptr->sgconf.admin.gcl) {
			free(list);
			free(list->sg_ptr);
			return NULL;
		}

		init_sg_list_node(list, port, id);
		break;

	case QCI_T_FM:
		list->fm_ptr = calloc(1, sizeof(struct std_fm));
		if (!list->fm_ptr) {
			free(list);
			return NULL;
		}

		init_fm_list_node(list, port, id);
		break;

	default:
		break;
	}

	return list;
}

void del_list_node(struct std_qci_list *node, enum qci_type type)
{
	if (!node)
		return;

	if (node->pre)
		node->pre->next = node->next;

	switch (type) {
	case QCI_T_SF:
		if (node->sf_ptr)
			free(node->sf_ptr);
		break;

	case QCI_T_SG:
		if (node->sg_ptr) {
			if (node->sg_ptr->sgconf.admin.gcl)
				free(node->sg_ptr->sgconf.admin.gcl);
			free(node->sg_ptr);
		}
		break;

	case QCI_T_FM:
		if (node->fm_ptr)
			free(node->fm_ptr);
		break;

	default:
		break;
	}

	free(node);
}

void free_list(struct std_qci_list *l_head, enum qci_type type)
{
	if (!l_head)
		return;

	if (l_head->next)
		free_list(l_head->next, type);

	del_list_node(l_head, type);
}

struct std_qci_list *is_node_in_list(struct std_qci_list *list,
		char *port, uint32_t id, enum qci_type type)
{
	struct std_qci_list *node = list;

	switch (type) {
	case QCI_T_SF:
		while (node) {
			if (!strncmp(port, node->sf_ptr->port, IF_NAME_MAX_LEN)
			    && (node->sf_ptr->sf_id == id))
				goto out;
			else
				node = node->next;
		}
		break;

	case QCI_T_SG:
		while (node) {
			if (!strncmp(port, node->sg_ptr->port, IF_NAME_MAX_LEN)
			    && (node->sg_ptr->sg_id == id))
				goto out;
			else
				node = node->next;
		}
		break;

	case QCI_T_FM:
		while (node) {
			if (!strncmp(port, node->fm_ptr->port, IF_NAME_MAX_LEN)
			    && (node->fm_ptr->fm_id == id))
				goto out;
			else
				node = node->next;
		}
		break;

	default:
		break;
	}
out:
	return node;
}

void add_node2list(struct std_qci_list *list, struct std_qci_list *node)
{
	struct std_qci_list *last = list;

	if (!list) {
		list = node;
		return;
	}

	while (last->next)
		last = last->next;

	last->next = node;
}

static char cmd_buf[MAX_CMD_LEN * 4];
static char st_buf[MAX_CMD_LEN];
static char fm_buf[MAX_CMD_LEN];
static char sg_buf[MAX_CMD_LEN];

static void *qci_monitor_thread(void *arg)
{
	int st_ret = 0;
	int fm_ret = 0;
	int sg_ret = 0;

	cb_streamid_clear_para();
	qci_fm_clear_para();

	while (true) {
		if (st_ret == 0)
			st_ret = cb_streamid_get_para(st_buf, sizeof(st_buf));

		if (sg_ret == 0)
			sg_ret = qci_sg_get_para(sg_buf, sizeof(sg_buf));

		if (fm_ret == 0)
			fm_ret = qci_fm_get_para(fm_buf, sizeof(fm_buf));

		if (!st_ret || (!fm_ret && !sg_ret))
			goto loop_tag;

		memset(cmd_buf, 0, sizeof(cmd_buf));

		if (st_ret > 0)
			strncat(cmd_buf, st_buf, sizeof(cmd_buf) - 1 - strlen(cmd_buf));

		if (sg_ret > 0)
			strncat(cmd_buf, sg_buf, sizeof(cmd_buf) - 1 - strlen(cmd_buf));

		if (fm_ret > 0)
			strncat(cmd_buf, fm_buf, sizeof(cmd_buf) - 1 - strlen(cmd_buf));

		printf("cmd:%s\n", cmd_buf);
		printf("qci thread ok!\n");

		fm_ret = 0;
		sg_ret = 0;
		st_ret = 0;

		qci_fm_clear_para();
		qci_sg_clear_para();
		cb_streamid_clear_para();

loop_tag:
		sleep(1);
	}

	return NULL;
}

int qci_init_thread(void)
{
	int ret;
	pthread_t qci_thread;

	ret = pthread_create(&qci_thread, NULL, qci_monitor_thread, NULL);
	if (ret != 0)
		printf("Unable to create qci monitor thread\n");

	pthread_detach(qci_thread);

	return ret;
}
