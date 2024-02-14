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

#include "chrony.h"

#include <inttypes.h>
#include <poll.h>
#include <stdio.h>

static chrony_err process_responses(chrony_session *s) {
	struct pollfd pfd = { .fd = chrony_get_fd(s), .events = POLLIN };
	struct timespec ts1, ts2;
	int n, timeout;
	chrony_err r;

	if (clock_gettime(CLOCK_MONOTONIC, &ts1) < 0) {
		fprintf(stderr, "Could not read monotonic clock\n");
		return -1;
	}

	timeout = 1000;

	while (chrony_needs_response(s)) {
		n = poll(&pfd, 1, timeout);
		if (n < 0) {
			perror("poll");
			return -1;
		} else if (n == 0) {
			fprintf(stderr, "No valid response received\n");
			return -1;
		}
		r = chrony_process_response(s);
		if (r != CHRONY_OK)
			return r;

		if (clock_gettime(CLOCK_MONOTONIC, &ts2) < 0) {
			fprintf(stderr, "Could not read monotonic clock\n");
			return -1;
		}
		timeout -= (ts2.tv_sec - ts1.tv_sec) * 1000 +
			((int32_t)ts2.tv_nsec - (int32_t)ts1.tv_nsec) / 1000000;
		if (timeout < 0)
			timeout = 0;
		ts1 = ts2;
	}

	return CHRONY_OK;
}

static int print_report(chrony_session *s, int report_index) {
	chrony_field_content content;
	const char *report_name, *str;
	uint64_t uval, flag;
	struct timespec ts;
	chrony_err r;
	int i, j;

	report_name = chrony_get_report_name(report_index);
	printf("%s:\n", report_name);

	r = chrony_request_report_number_records(s, report_name);
	if (r != CHRONY_OK)
		return r;

	r = process_responses(s);
	if (r != CHRONY_OK)
		return r;

	for (i = 0; i < chrony_get_report_number_records(s); i++) {
		printf("  Record #%d:\n", i + 1);

		r = chrony_request_record(s, report_name, i);
		if (r != CHRONY_OK)
			return r;

		r = process_responses(s);
		if (r != CHRONY_OK)
			return r;

		for (j = 0; j < chrony_get_record_number_fields(s); j++) {
			content = chrony_get_field_content(s, j);
			if (content == CHRONY_CONTENT_NONE)
				continue;

			printf("    %s: ", chrony_get_field_name(s, j));

			switch (chrony_get_field_type(s, j)) {
			case CHRONY_TYPE_UINTEGER:
				uval = chrony_get_field_uinteger(s, j);
				switch (content) {
				case CHRONY_CONTENT_REFERENCE_ID:
					printf("%08"PRIX64, uval);
					break;
				case CHRONY_CONTENT_ENUM:
					str = chrony_get_field_constant_name(s, j, uval);
					if (str)
						printf("%s", str);
					break;
				case CHRONY_CONTENT_FLAGS:
					for (flag = 1; flag != 0; flag <<= 1) {
						if ((uval & flag) == 0)
							continue;
						str = chrony_get_field_constant_name(s, j, flag);
						if (str)
							printf("%s ", str);
					}
					break;
				case CHRONY_CONTENT_BOOLEAN:
					printf(uval ? "Yes" : "No");
					break;
				default:
					printf("%"PRIu64, uval);
				}
				break;
			case CHRONY_TYPE_INTEGER:
				printf("%"PRId64, chrony_get_field_integer(s, j));
				break;
			case CHRONY_TYPE_FLOAT:
				printf("%f", chrony_get_field_float(s, j));
				break;
			case CHRONY_TYPE_STRING:
				str = chrony_get_field_string(s, j);
				if (str)
					printf("%s", str);
				break;
			case CHRONY_TYPE_TIMESPEC:
				ts = chrony_get_field_timespec(s, j);
				printf("%"PRIu64".%09"PRIu32, (uint64_t)ts.tv_sec, (uint32_t)ts.tv_nsec);
				break;
			default:
				printf("?");
				break;
			}

			switch (content) {
			case CHRONY_CONTENT_INTERVAL_LOG2_SECONDS:
				printf(" log2(seconds)");
				break;
			case CHRONY_CONTENT_INTERVAL_SECONDS:
			case CHRONY_CONTENT_OFFSET_SECONDS:
			case CHRONY_CONTENT_MEASURE_SECONDS:
				printf(" seconds");
				break;
			case CHRONY_CONTENT_OFFSET_PPM:
			case CHRONY_CONTENT_MEASURE_PPM:
				printf(" ppm");
				break;
			case CHRONY_CONTENT_OFFSET_PPM_PER_SECOND:
				printf(" ppm per second");
				break;
			case CHRONY_CONTENT_LENGTH_BITS:
				printf(" bits");
				break;
			case CHRONY_CONTENT_LENGTH_BYTES:
				printf(" bytes");
				break;
			default:
				break;
			}
			printf("\n");
		}
	}

	return CHRONY_OK;
}

static void print_all_reports(chrony_session *s) {
	chrony_err r;
	int i;

	for (i = 0; i < chrony_get_number_supported_reports(); i++) {
		r = print_report(s, i);
		if (r != CHRONY_OK && r != -1)
			printf("%s\n", chrony_get_error_string(r));
	}
}

int main(int argc, char **argv) {
	chrony_session *s;
	int fd, r = 0;

	fd = chrony_open_socket(argc > 1 ? argv[1] : NULL);
	if (fd < 0) {
		perror("Could not open socket");
		return 1;
	}

	if (chrony_init_session(&s, fd) == CHRONY_OK) {
		print_all_reports(s);
		chrony_deinit_session(s);
	} else {
		r = 1;
	}

	chrony_close_socket(fd);

	return r;
}
