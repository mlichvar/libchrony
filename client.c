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

#include <arpa/inet.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	STATE_IDLE,
	STATE_REQUEST_SENT,
	STATE_RESPONSE_RECEIVED,
	STATE_RESPONSE_ACCEPTED,
} State;

struct chrony_session_t {
	State state;
	int fd;
	int num_expected_responses;
	bool count_requested;
	int requested_record;
	const Response *expected_responses;
	Message request_msg;
	Message response_msg;
	int num_records;
	const char *follow_report;
	FILE *urandom;
};

const char *chrony_get_error_string(chrony_err e) {
	static const char *strings[] = {
		"CHRONY_OK",
		"CHRONY_NO_MEMORY",
		"CHRONY_NO_RANDOM",
		"CHRONY_UNKNOWN_REPORT",
		"CHRONY_RANDOM_FAILED",
		"CHRONY_SEND_FAILED",
		"CHRONY_RECV_FAILED",
		"CHRONY_INVALID_ARGUMENT",
		"CHRONY_UNEXPECTED_CALL",
		"CHRONY_UNAUTHORIZED",
		"CHRONY_UNEXPECTED_STATUS",
		"CHRONY_UNSUPPORTED_RESPONSE",
	};
	assert(CHRONY_UNSUPPORTED_RESPONSE == 11);

	if (e < 0 || e >= sizeof (strings) / sizeof (strings[0]))
		return "Unknown error";

	return strings[e];
}

chrony_err chrony_session_init(chrony_session **s, int fd) {
	chrony_session *session;

	session = malloc(sizeof (*session));
	if (!session)
		return CHRONY_NO_MEMORY;

	memset(session, 0, sizeof (*session));
	session->state = STATE_IDLE;
	session->fd = fd;
	session->urandom = fopen("/dev/urandom", "r");
	if (!session->urandom) {
		free(session);
		return CHRONY_NO_RANDOM;
	}

	*s = session;

	return CHRONY_OK;
}

void chrony_session_deinit(chrony_session *s) {
	fclose(s->urandom);
	free(s);
}

int chrony_get_fd(chrony_session *s) {
	return s->fd;
}

bool chrony_needs_response(chrony_session *s) {
	return s->state == STATE_REQUEST_SENT;
}

chrony_err chrony_process_response(chrony_session *s) {
	chrony_err r;
	int len;

	if (s->state != STATE_REQUEST_SENT)
		return CHRONY_UNEXPECTED_CALL;

	memset(&s->response_msg, 0, sizeof (s->response_msg));

	len = recv(s->fd, s->response_msg.msg, sizeof (s->response_msg.msg), 0);
	if (len < 0)
		return CHRONY_RECV_FAILED;

	s->response_msg.len = len;

	if (!is_response_valid(&s->request_msg, &s->response_msg))
		/* Ignore the response */
		return CHRONY_OK;

	s->state = STATE_RESPONSE_RECEIVED;

	r = process_response(&s->response_msg, s->expected_responses);
	if (r != CHRONY_OK)
		return r;

	s->state = STATE_RESPONSE_ACCEPTED;

	if (s->count_requested) {
		assert(s->response_msg.fields[0].type == TYPE_UINT32);
		s->num_records = get_field_uinteger(&s->response_msg, 0);
	}

	if (s->follow_report)
		return chrony_request_record(s, s->follow_report, s->requested_record);

	return CHRONY_OK;
}

static chrony_err send_request(chrony_session *s, const Request *request,
			       void **values, const Response *expected_responses) {
	uint32_t sequence;

	if (fread(&sequence, sizeof (sequence), 1, s->urandom) != 1) {
		s->state = STATE_IDLE;
		return CHRONY_RANDOM_FAILED;
	}

	format_request(&s->request_msg, random(), request, values, expected_responses);

	if (send(s->fd, s->request_msg.msg, s->request_msg.len, 0) < 0) {
		s->state = STATE_IDLE;
		return CHRONY_SEND_FAILED;
	}

	s->state = STATE_REQUEST_SENT;

	return CHRONY_OK;
}

chrony_err chrony_request_report_number_records(chrony_session *s, const char *report_name) {
	const Report *report;
	chrony_err r;

	report = get_report(get_report_index(report_name));
	if (!report)
		return CHRONY_UNKNOWN_REPORT;

	if (report->count_requests[0].code == 0) {
		s->num_records = 1;
		return CHRONY_OK;
	}

	r = send_request(s, &report->count_requests[0], NULL, report->count_responses);
	if (r != CHRONY_OK)
		return r;

	s->num_expected_responses = 1;
	s->expected_responses = report->count_responses;
	s->count_requested = true;

	s->num_records = 0;

	return CHRONY_OK;
}

int chrony_get_report_number_records(chrony_session *s) {
	return s->num_records;
}

chrony_err chrony_request_record(chrony_session *s, const char *report_name, int record) {
	uint32_t index = record;
	const Report *report;
	const Field *fields;
	chrony_err r;
	void *args[1];

	s->follow_report = NULL;
again:
	report = get_report(get_report_index(report_name));
	if (!report)
		return CHRONY_UNKNOWN_REPORT;

	fields = report->record_requests[0].fields;

	if (fields) {
		switch (fields[0].type) {
		case TYPE_ADDRESS:
			/* Get the address from sourcestats report */
			if (s->state != STATE_RESPONSE_ACCEPTED ||
			    !is_report_fields("sourcestats", s->response_msg.fields) ||
			    s->requested_record != record) {
				s->follow_report = report_name;
				report_name = "sourcestats";
				goto again;
			}
			args[0] = s->response_msg.msg + get_field_position(&s->response_msg, 1);
			/* Unspecified address is a reference clock */
			if (!get_field_string(&s->response_msg, 1)) {
				s->response_msg.num_fields = 0;
				return CHRONY_OK;
			}
			break;
		case TYPE_UINT32:
			args[0] = &index;
			break;
		default:
			assert(0);
		}
		assert(fields[1].type == TYPE_NONE);
	} else {
		if (record != 0)
			return CHRONY_INVALID_ARGUMENT;
	}

	r = send_request(s, &report->record_requests[0], args, report->record_responses);
	if (r != CHRONY_OK)
		return r;

	s->num_expected_responses = 1;
	s->expected_responses = report->record_responses;
	s->count_requested = false;
	s->requested_record = record;

	return CHRONY_OK;
}

int chrony_get_record_number_fields(chrony_session *s) {
	return s->response_msg.num_fields;
}

const char *chrony_get_field_name(chrony_session *s, int field) {
	return resolve_field_name(&s->response_msg, field);
}

chrony_field_type chrony_get_field_type(chrony_session *s, int field) {
	switch (resolve_field_type(&s->response_msg, field)) {
	case TYPE_UINT32:
	case TYPE_UINT16:
	case TYPE_UINT8:
		return CHRONY_TYPE_UINTEGER;
	case TYPE_INT16:
	case TYPE_INT8:
		return CHRONY_TYPE_INTEGER;
	case TYPE_FLOAT:
		return CHRONY_TYPE_FLOAT;
	case TYPE_ADDRESS:
		return CHRONY_TYPE_STRING;
	case TYPE_TIMESPEC:
		return CHRONY_TYPE_TIMESPEC;
	case TYPE_NONE:
		return CHRONY_TYPE_NONE;
	default:
		assert(0);
		return CHRONY_TYPE_NONE;
	}
}

chrony_field_content chrony_get_field_content(chrony_session *s, int field) {
	return resolve_field_content(&s->response_msg, field);
}

uint64_t chrony_get_field_uinteger(chrony_session *s, int field) {
	return get_field_uinteger(&s->response_msg, field);
}

int64_t chrony_get_field_integer(chrony_session *s, int field) {
	return get_field_integer(&s->response_msg, field);
}

double chrony_get_field_float(chrony_session *s, int field) {
	return get_field_float(&s->response_msg, field);
}

struct timespec chrony_get_field_timespec(chrony_session *s, int field) {
	return get_field_timespec(&s->response_msg, field);
}

const char *chrony_get_field_string(chrony_session *s, int field) {
	return get_field_string(&s->response_msg, field);
}

const char *chrony_get_field_constant_name(chrony_session *s, int field, uint64_t value) {
	return get_field_constant_name(&s->response_msg, field, value);
}
