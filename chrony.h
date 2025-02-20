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

#ifndef CHRONY_H
#define CHRONY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

int chrony_open_socket(const char *address);
void chrony_close_socket(int fd);

typedef enum {
	CHRONY_OK = 0,
	CHRONY_NO_MEMORY,
	CHRONY_NO_RANDOM,
	CHRONY_UNKNOWN_REPORT,
	CHRONY_RANDOM_FAILED,
	CHRONY_SEND_FAILED,
	CHRONY_RECV_FAILED,
	CHRONY_INVALID_ARGUMENT,
	CHRONY_UNEXPECTED_CALL,
	CHRONY_UNAUTHORIZED,
	CHRONY_DISABLED,
	CHRONY_UNEXPECTED_STATUS,
	CHRONY_OLD_SERVER,
	CHRONY_NEW_SERVER,
	CHRONY_INVALID_RESPONSE,
} chrony_err;

typedef struct chrony_session_t chrony_session;

const char *chrony_get_error_string(chrony_err);

chrony_err chrony_init_session(chrony_session **s, int fd);
void chrony_deinit_session(chrony_session *s);

int chrony_get_fd(chrony_session *s);

bool chrony_needs_response(chrony_session *s);
chrony_err chrony_process_response(chrony_session *s);
chrony_err chrony_timeout(chrony_session *s);

int chrony_get_number_supported_reports(void);
const char *chrony_get_report_name(int report);

chrony_err chrony_request_report_number_records(chrony_session *s, const char *report_name);
int chrony_get_report_number_records(chrony_session *s);

chrony_err chrony_request_record(chrony_session *s, const char *report_name, int record);

typedef enum {
	CHRONY_TYPE_NONE = 0,
	CHRONY_TYPE_UINTEGER,
	CHRONY_TYPE_INTEGER,
	CHRONY_TYPE_FLOAT,
	CHRONY_TYPE_TIMESPEC,
	CHRONY_TYPE_STRING,
} chrony_field_type;

typedef enum {
	CHRONY_CONTENT_NONE = 0,
	CHRONY_CONTENT_COUNT,
	CHRONY_CONTENT_TIME,
	CHRONY_CONTENT_INTERVAL_LOG2_SECONDS,
	CHRONY_CONTENT_INTERVAL_SECONDS,
	CHRONY_CONTENT_OFFSET_SECONDS,
	CHRONY_CONTENT_MEASURE_SECONDS,
	CHRONY_CONTENT_OFFSET_PPM,
	CHRONY_CONTENT_MEASURE_PPM,
	CHRONY_CONTENT_OFFSET_PPM_PER_SECOND,
	CHRONY_CONTENT_RATIO,
	CHRONY_CONTENT_REFERENCE_ID,
	CHRONY_CONTENT_ENUM,
	CHRONY_CONTENT_BITS,
	CHRONY_CONTENT_FLAGS,
	CHRONY_CONTENT_ADDRESS,
	CHRONY_CONTENT_PORT,
	CHRONY_CONTENT_INDEX,
	CHRONY_CONTENT_LENGTH_BITS,
	CHRONY_CONTENT_LENGTH_BYTES,
	CHRONY_CONTENT_BOOLEAN,
} chrony_field_content;

int chrony_get_record_number_fields(chrony_session *s);
const char *chrony_get_field_name(chrony_session *s, int field);
int chrony_get_field_index(chrony_session *s, const char *name);
chrony_field_type chrony_get_field_type(chrony_session *s, int field);
chrony_field_content chrony_get_field_content(chrony_session *s, int field);

uint64_t chrony_get_field_uinteger(chrony_session *s, int field);
int64_t chrony_get_field_integer(chrony_session *s, int field);
double chrony_get_field_float(chrony_session *s, int field);
struct timespec chrony_get_field_timespec(chrony_session *s, int field);
const char *chrony_get_field_string(chrony_session *s, int field);
const char *chrony_get_field_constant_name(chrony_session *s, int field, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif
