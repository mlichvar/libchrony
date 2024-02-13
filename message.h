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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "chrony.h"

#define MAX_MESSAGE_LEN 1024
#define MAX_REQUESTS 2
#define MAX_RESPONSES 4

typedef enum {
	TYPE_NONE = 0,
	TYPE_UINT64,
	TYPE_UINT32,
	TYPE_UINT16,
	TYPE_UINT8,
	TYPE_INT16,
	TYPE_INT8,
	TYPE_FLOAT,
	TYPE_ADDRESS,
	TYPE_TIMESPEC,
	TYPE_STRING,

	/* Context-specific types */
	TYPE_ADDRESS_OR_UINT32_IN_ADDRESS,
} FieldType;

typedef struct {
	uint32_t value;
	const char *name;
} Constant;

typedef struct {
	const char *name;
	FieldType type;
	chrony_field_content content;
	const Constant *constants;
} Field;

typedef struct {
	uint16_t code;
	const Field *fields;
} Request;

typedef struct {
	uint16_t code;
	const Field *fields;
} Response;

typedef struct {
	const char *name;
	const Request count_requests[MAX_REQUESTS];
	const Response count_responses[MAX_RESPONSES];
	const Request record_requests[MAX_REQUESTS];
	const Response record_responses[MAX_RESPONSES];
} Report;

typedef struct {
	char msg[MAX_MESSAGE_LEN];
	int len;
	int num_fields;
	const Field *fields;
} Message;

void format_request(Message *msg, uint32_t sequence, const Request *request,
		    void **values, const Response *expected_responses);
bool is_response_valid(const Message *request, const Message *response);
chrony_err process_response(Message *response, const Response *expected_responses);

int get_field_position(const Message *msg, int field);

FieldType resolve_field_type(const Message *msg, int field);
const char *resolve_field_name(const Message *msg, int field);
chrony_field_content resolve_field_content(const Message *msg, int field);

uint64_t get_field_uinteger(const Message *msg, int field);
int64_t get_field_integer(const Message *msg, int field);
double get_field_float(const Message *msg, int field);
struct timespec get_field_timespec(const Message *msg, int field);
const char *get_field_string(const Message *msg, int field);
const char *get_field_constant_name(const Message *msg, int field, uint64_t value);

int get_report_index(const char *name);
const Report *get_report(int report);
bool is_report_fields(const char *report_name, const Field *fields);

int chrony_get_number_supported_reports(void);
const char *chrony_get_report_name(int report);

#endif
