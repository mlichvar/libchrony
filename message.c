/*
 * Copyright (C) 2023  Miroslav Lichvar
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "message.h"

#include "reports.h"

#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define REQUEST_HEADER_LEN 20
#define RESPONSE_HEADER_LEN 28

static int get_field_offset(const Field *fields, int field);

int get_response_len(const Response *response) {
	int i;

	if (response->code == 0)
		return 0;

	for (i = 0; response->fields[i].type != TYPE_NONE; i++)
		;
	return RESPONSE_HEADER_LEN + get_field_offset(response->fields, i);
}

void format_request(Message *msg, uint32_t sequence, const Request *request,
		    void **values, const Response *expected_responses) {
	int i, pos, res_len, max_res_len;

	memset(msg, 0, sizeof (*msg));

	msg->msg[0] = 6; /* Protocol version */
	msg->msg[1] = 1; /* Request type */
	*(uint16_t *)&msg->msg[4] = htons(request->code);
	*(uint32_t *)&msg->msg[8] = htonl(sequence);

	msg->fields = request->fields;
	msg->len = sizeof (msg->msg);

	for (i = 0; request->fields && request->fields[i].type != TYPE_NONE; i++) {
		msg->num_fields = i + 1;
		pos = get_field_position(msg, i);
		assert(pos > 0);

		switch (msg->fields[i].type) {
		case TYPE_UINT32:
			*(uint32_t *)(msg->msg + pos) = htonl(*(uint32_t *)values[i]);
			break;
		case TYPE_ADDRESS:
			memcpy(msg->msg + pos, values[i], 20);
			break;
		default:
			assert(0);
		}
	}

	msg->num_fields++;
	msg->len = get_field_position(msg, i);
	msg->num_fields--;

	for (i = max_res_len = 0; i < MAX_RESPONSES; i++) {
		res_len = get_response_len(&expected_responses[i]);
		if (max_res_len < res_len)
		       max_res_len = res_len;
	}

	if (msg->len < max_res_len)
		msg->len = max_res_len;
}

bool is_response_valid(const Message *request, const Message *response) {
	if (response->len < RESPONSE_HEADER_LEN ||
	    response->msg[0] != 6 ||	/* Version */
	    response->msg[1] != 2 ||	/* Response type */
	    response->msg[2] != 0 ||	/* Reserved */
	    response->msg[3] != 0 ||	/* Reserved */
	    *(uint16_t *)&response->msg[4] != *(uint16_t *)&request->msg[4] || /* Code */
	    *(uint32_t *)&response->msg[16] != *(uint32_t *)&request->msg[8])  /* Sequence */
		return false;

	return true;
}

chrony_err process_response(Message *msg, const Response *expected_responses) {
	int i, code, status;

	msg->num_fields = 0;
	msg->fields = NULL;

	code = ntohs(*(uint16_t *)&msg->msg[6]);
	status = ntohs(*(uint16_t *)&msg->msg[8]);

	switch (status) {
	case 0: /* OK */
		break;
	case 2: /* Unauthorized */
		return CHRONY_UNAUTHORIZED;
	case 3: /* Invalid */
		return CHRONY_OLD_SERVER;
	case 6: /* Not enabled */
	case 13:/* No RTC */
		return CHRONY_DISABLED;
	case 18:/* Bad packet version */
	case 19:/* Bad packet length */
		return CHRONY_NEW_SERVER;
	default:
		return CHRONY_UNEXPECTED_STATUS;
	}

	for (i = 0; i < MAX_RESPONSES && expected_responses[i].fields; i++) {
		if (code == expected_responses[i].code) {
			msg->fields = expected_responses[i].fields;
			break;
		}
	}

	if (!msg->fields)
		return CHRONY_NEW_SERVER;

	for (i = 0; msg->fields[i].type != TYPE_NONE; i++)
		;

	msg->num_fields = i + 1;
	if (msg->len < get_field_position(msg, i)) {
		msg->num_fields = 0;
		msg->fields = NULL;
		return CHRONY_INVALID_RESPONSE;
	}
	msg->num_fields--;

	return CHRONY_OK;
}

static int get_field_len(const Field *fields, int field) {
	if (!fields)
		return 0;
	switch (fields[field].type) {
	case TYPE_NONE:
		return 0;
	case TYPE_UINT64:
		return 8;
	case TYPE_UINT32:
		return 4;
	case TYPE_UINT16:
	case TYPE_INT16:
		return 2;
	case TYPE_UINT8:
	case TYPE_INT8:
		return 1;
	case TYPE_FLOAT:
		return 4;
	case TYPE_ADDRESS:
	case TYPE_ADDRESS_OR_UINT32_IN_ADDRESS:
		return 20;
	case TYPE_TIMESPEC:
		return 12;
	default:
		assert(0);
	}
}

static int get_field_offset(const Field *fields, int field) {
	int i, offset;

	for (i = 0, offset = 0; i < field; i++)
		offset += get_field_len(fields, i);
	return offset;
}

int get_field_position(const Message *msg, int field) {
	if (!msg->fields || field < 0 || field >= msg->num_fields)
		return -1;

	return (msg->msg[1] == 2 ? RESPONSE_HEADER_LEN : REQUEST_HEADER_LEN) +
		get_field_offset(msg->fields, field);
}

FieldType resolve_field_type(const Message *msg, int field) {
	if (!msg->fields || field < 0 || field >= msg->num_fields)
		return TYPE_NONE;

	if (msg->fields[field].type == TYPE_ADDRESS_OR_UINT32_IN_ADDRESS) {
		if (msg->fields == sources_report_fields)
			return get_field_uinteger(msg, 4) == 2 ? TYPE_UINT32 : TYPE_ADDRESS;
		assert(0);
	}

	return msg->fields[field].type;
}

const char *resolve_field_name(const Message *msg, int field) {
	const char *name;

	if (!msg->fields || field < 0 || field >= msg->num_fields)
		return NULL;

	name = msg->fields[field].name;

	if (msg->fields[field].type == TYPE_ADDRESS_OR_UINT32_IN_ADDRESS) {
		if (msg->fields == sources_report_fields)
			return name + (get_field_uinteger(msg, 4) == 2 ? strlen(name) + 1 : 0);
		assert(0);
	}

	return name;
}

chrony_field_content resolve_field_content(const Message *msg, int field) {
	chrony_field_content content;

	if (!msg->fields || field < 0 || field >= msg->num_fields)
		return CHRONY_CONTENT_NONE;

	content = msg->fields[field].content;

	if (msg->fields[field].type == TYPE_ADDRESS_OR_UINT32_IN_ADDRESS) {
		if (msg->fields == sources_report_fields)
			content = resolve_field_type(msg, field) == TYPE_ADDRESS ?
				CHRONY_CONTENT_ADDRESS : CHRONY_CONTENT_REFERENCE_ID;
		else
			assert(0);
	}

	if (content == CHRONY_CONTENT_ADDRESS) {
		if (!get_field_string(msg, field))
			return CHRONY_CONTENT_NONE;
	}

	return content;
}

uint64_t get_field_uinteger(const Message *msg, int field) {
	int pos = get_field_position(msg, field);

	if (pos < 0)
		return 0;

	switch (resolve_field_type(msg, field)) {
	case TYPE_UINT64:
		return ((uint64_t)ntohl(*(uint32_t *)(msg->msg + pos)) << 32) |
			ntohl(*(uint32_t *)(msg->msg + pos + 4));
	case TYPE_UINT32:
		return ntohl(*(uint32_t *)(msg->msg + pos));
	case TYPE_UINT16:
		return ntohs(*(uint16_t *)(msg->msg + pos));
	case TYPE_UINT8:
		return (uint8_t)*(msg->msg + pos);
	default:
		return 0;
	}
}

int64_t get_field_integer(const Message *msg, int field) {
	int pos = get_field_position(msg, field);

	if (pos < 0)
		return 0;

	switch (resolve_field_type(msg, field)) {
	case TYPE_INT16:
		return ntohs(*(uint16_t *)(msg->msg + pos));
	case TYPE_INT8:
		return (int8_t)*(msg->msg + pos);
	default:
		return 0;
	}
}

double get_field_float(const Message *msg, int field) {
	int pos = get_field_position(msg, field);
	int32_t exp, coef;
	uint32_t x;

	if (pos < 0)
		return FP_NAN;

	switch (resolve_field_type(msg, field)) {
	case TYPE_FLOAT:
		x = ntohl(*(uint32_t *)(msg->msg + pos));

		exp = x >> 25;
		if (exp >= 1 << 6)
			exp -= 1 << 7;

		coef = x % (1U << 25);
		if (coef >= 1 << 24)
			coef -= 1 << 25;

		return coef * pow(2.0, exp - 25);
	default:
		return FP_NAN;
	}
}

struct timespec get_field_timespec(const Message *msg, int field) {
	int pos = get_field_position(msg, field);
	struct timespec ts = { 0 };
	const char *data;

	if (pos < 0)
		return ts;

	data = msg->msg + pos;

	switch (resolve_field_type(msg, field)) {
	case TYPE_TIMESPEC:
		ts.tv_sec = (uint64_t)ntohl(*(uint32_t *)data) << 32 |
			ntohl(*(uint32_t *)(data + 4));
		ts.tv_nsec = ntohl(*(uint32_t *)(data + 8));
		break;
	default:
		break;
	}

	return ts;
}

const char *get_field_string(const Message *msg, int field) {
	int pos = get_field_position(msg, field);
	static char buf[256];
	const char *data;

	if (pos < 0)
		return NULL;

	data = msg->msg + pos;

	switch (resolve_field_type(msg, field)) {
	case TYPE_ADDRESS:
		switch (ntohs(*(uint16_t *)(data + 16))) {
		case 0:
			return NULL;
		case 1:
			return inet_ntop(AF_INET, data, buf, sizeof (buf));
		case 2:
			return inet_ntop(AF_INET6, data, buf, sizeof (buf));
		case 3:
			snprintf(buf, sizeof (buf), "ID#%010"PRIu32,
				 ntohl(*(uint32_t *)data));
			return buf;
		default:
			return "?";
		}
	default:
		return NULL;
	}
}

const char *get_field_constant_name(const Message *msg, int field, uint64_t value) {
	const Constant *c;
	int i;

	if (!msg->fields || field < 0 || field >= msg->num_fields)
		return NULL;

	c = msg->fields[field].constants;
	if (!c)
		return NULL;

	for (i = 0; c[i].name; i++) {
		if (value == c[i].value)
			return c[i].name;
	}

	return NULL;
}

int get_report_index(const char *name) {
	const char *rep_name;
	int i;

	for (i = 0; i < chrony_get_number_supported_reports(); i++) {
		rep_name = chrony_get_report_name(i);
		if (rep_name && strcmp(rep_name, name) == 0)
			return i;
	}
	return -1;
}

const Report *get_report(int report) {
	if (report < 0 || report >= chrony_get_number_supported_reports())
		return NULL;
	return &reports[report];
}

bool is_report_fields(const char *report_name, const Field *fields) {
	const Report *report = get_report(get_report_index(report_name));

	return report && report->record_responses[0].fields == fields;
}

int chrony_get_number_supported_reports(void) {
	return sizeof (reports) / sizeof (reports[0]);
}

const char *chrony_get_report_name(int report) {
	if (report < 0 || report >= chrony_get_number_supported_reports())
		return NULL;
	return reports[report].name;
}
