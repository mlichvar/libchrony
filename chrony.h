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

/** \file
 * Public header file of libchrony.
 */

#ifndef CHRONY_H
#define CHRONY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/**
 * Open a client UDP or Unix domain socket connected to chronyd.
 * @param address	Address of the server socket as a string. If it starts
 * 			with '/', it is interpreted as an absolute path to the
 * 			Unix domain socket, otherwise it is an IPv4 or IPv6
 * 			address, optionally including the port number (default
 * 			is 323). If NULL or empty string, the function will try
 * 			to connect to /var/run/chrony/chronyd.sock, 127.0.0.1,
 * 			and ::1. The address is specified in one of the
 * 			following forms:
 * 			- /path/to/unix.socket
 * 			- 192.0.2.1
 * 			- 192.0.2.1:323
 * 			- 2001:db8::1
 * 			- [2001:db8::1]:323
 * @return		File descriptor of the socket, or a negative value on
 * 			error.
 */
int chrony_open_socket(const char *address);
/**
 * Close the client socket when no longer needed.
 * @param fd		Socket returned by chrony_open_socket().
 */
void chrony_close_socket(int fd);

/**
 * Enum for error codes.
 */
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

/**
 * Get a string describing a libchrony error code.
 * @param e		Error code.
 * @return		Description of the error.
 */
const char *chrony_get_error_string(chrony_err e);

/**
 * Type for a client-server session.
 */
typedef struct chrony_session_t chrony_session;

/**
 * Create a new client-server session.
 * @param s		Pointer to pointer where the new session should
 * 			be saved.
 * @param fd		Socket returned by chrony_open_socket().
 * @return		Error code (CHRONY_OK on success).
 */
chrony_err chrony_init_session(chrony_session **s, int fd);
/**
 * Destroy the session.
 * @param s		Session.
 */
void chrony_deinit_session(chrony_session *s);

/**
 * Get the socket used by the session.
 * @param s		Session.
 * @return		Socket which was provided to chrony_init_session().
 */
int chrony_get_fd(chrony_session *s);

/**
 * Check if the session is waiting for a server response after sending a
 * request, i.e. when the application should wait for a timeout or read event
 * on the socket and then call chrony_process_response(). Multiple responses
 * might need to be processed before the requested information is available.
 * @param s		Session.
 * @return		true if waiting for the response, false otherwise.
 */
bool chrony_needs_response(chrony_session *s);
/**
 * Process a server response waiting to be received from the socket and send
 * another request if needed. This function should be called only when
 * chrony_needs_response() returns true.
 * @param s		Session.
 * @return		Error code (CHRONY_OK on success).
 */
chrony_err chrony_process_response(chrony_session *s);

/**
 * Get the number of reports supported by the client. A report contains
 * a number of records, each containing a number of fields. Some reports are
 * available only over the Unix domain socket (depending on the chronyd
 * configuration).
 * @return		Number of supported reports.
 */
int chrony_get_number_supported_reports(void);
/**
 * Get the name of a report supported by the client.
 * @param report	Index of the report (starting at 0 and ending at
 * 			returned by chrony_get_number_supported_reports()-1).
 * @return		Name of the report (e.g. sources, tracking).
 */
const char *chrony_get_report_name(int report);

/**
 * Send a request to the server to get the number of records available for
 * a given report. The number will be available after chrony_process_response()
 * returns success.
 * @param s		Session.
 * @param report_name	Name of the report.
 * @return		Error code (CHRONY_OK on success).
 */
chrony_err chrony_request_report_number_records(chrony_session *s, const char *report_name);
/**
 * Get the number of available records requested by
 * chrony_request_report_number_records(). Some reports always have one record,
 * other reports have a variable number of records (e.g. equal to the number of
 * configured time sources).
 * @param s		Session.
 * @return		Number of records.
 */
int chrony_get_report_number_records(chrony_session *s);

/**
 * Send a request to the server to get a record of a report. The number
 * of fields in the record and their values will be available after
 * chrony_process_response() returns success.
 * @param s		Session.
 * @param report_name	Name of the report.
 * @param record	Index of the record (starting at 0).
 * @return		Error code (CHRONY_OK on success).
 */
chrony_err chrony_request_record(chrony_session *s, const char *report_name, int record);

/**
 * Enum for record field data types.
 */
typedef enum {
	CHRONY_TYPE_NONE = 0,
	CHRONY_TYPE_UINTEGER,
	CHRONY_TYPE_INTEGER,
	CHRONY_TYPE_FLOAT,
	CHRONY_TYPE_TIMESPEC,
	CHRONY_TYPE_STRING,
} chrony_field_type;

/**
 * Enum for record field content types (e.g. units).
 */
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

/**
 * Get the number of fields in the requested record. The number and order of
 * the fields may depend on the server version.
 * @param s		Session.
 * @return		Number of fields.
 */
int chrony_get_record_number_fields(chrony_session *s);
/**
 * Get the name of a field in the requested record.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Pointer to the name of the field, or NULL if the index
 * 			is not valid. The pointer is valid until the session is
 * 			destroyed.
 */
const char *chrony_get_field_name(chrony_session *s, int field);
/**
 * Get the index of a field given by its name.
 * @param s		Session.
 * @param name		Name of the field.
 * @return		Index of the field, or a negative value if not present.
 */
int chrony_get_field_index(chrony_session *s, const char *name);
/**
 * Get the type of a field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Type of the field (CHRONY_TYPE_NONE if the field
 * 			is missing).
 */
chrony_field_type chrony_get_field_type(chrony_session *s, int field);
/**
 * Get the content type of a field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Content type of the field (CHRONY_CONTENT_NONE if
 * 			the field is missing).
 */
chrony_field_content chrony_get_field_content(chrony_session *s, int field);

/**
 * Get the value of an unsigned integer field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Value of the field as uint64_t.
 */
uint64_t chrony_get_field_uinteger(chrony_session *s, int field);
/**
 * Get the value of a signed integer field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Value of the field as int64_t.
 */
int64_t chrony_get_field_integer(chrony_session *s, int field);
/**
 * Get the value of a floating-point field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Value of the field as double.
 */
double chrony_get_field_float(chrony_session *s, int field);
/**
 * Get the value of a timespec field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Value of the field as struct timespec.
 */
struct timespec chrony_get_field_timespec(chrony_session *s, int field);
/**
 * Get the pointer to a string field.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @return		Pointer to the string, or NULL if the index is not
 *			valid or the field does not have the CHRONY_TYPE_STRING
 *			type. The pointer is valid until another libchrony
 *			function is called.
 */
const char *chrony_get_field_string(chrony_session *s, int field);
/**
 * Get the name of an enum or flag value in a CHRONY_CONTENT_ENUM or
 * CHRONY_CONTENT_FLAGS field respectively.
 * @param s		Session.
 * @param field		Index of the field in the record (starting at 0).
 * @param value		Enum or flag value.
 * @return		Pointer to the name. It is valid until the session is
 * 			destroyed.
 */
const char *chrony_get_field_constant_name(chrony_session *s, int field, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif
