/*
 * Copyright (C) 2024  Miroslav Lichvar
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

#include <arpa/inet.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

static FILE *in;
static FILE *out;
static int server_fd;
static uint32_t sequence;

static int receive_request(void) {
	struct pollfd pfd = { .fd = server_fd, .events = POLLIN };
	char buf[65536];

	if (poll(&pfd, 1, 0) != 1)
		return 0;

	fprintf(stderr, "recv\n");
	if (recv(server_fd, buf, sizeof (buf), 0) < 12)
		return 1;

	sequence = *(uint32_t *)&buf[8];

	return 1;
}

static int send_responses(void) {
	char buf[65536];
	uint16_t len;
	int i;

	if (fread(&len, sizeof (len), 1, in) != 1)
		return 0;
	len = ntohs(len);
	if (fread(buf, len, 1, in) != 1)
		return 0;
	if (len >= 20 && buf[19] != 0)
		*(uint32_t *)&buf[16] = sequence;
	for (i = 0; i < 2; i++) {
		fprintf(stderr, "send\n");
		if (send(server_fd, buf, len, 0) < 0)
			return 0;
	}

	return 1;
}

static chrony_err process_responses(chrony_session *s) {
	chrony_err r;

	while (chrony_needs_response(s)) {
		if (!receive_request() || !send_responses())
			return CHRONY_RECV_FAILED;

		r = chrony_process_response(s);
		if (r != CHRONY_OK)
			return r;
	}

	return CHRONY_OK;
}

static int dump_report(chrony_session *s, int report_index) {
	const char *report_name, *str;
	chrony_field_content content;
	struct timespec ts;
	uint64_t flag;
	chrony_err r;
	int i, j;

	report_name = chrony_get_report_name(report_index);
	if (!report_name)
		return CHRONY_OK;

	fprintf(out, "request number of records\n");

	r = chrony_request_report_number_records(s, report_name);
	if (r != CHRONY_OK)
		return r;

	r = process_responses(s);
	if (r != CHRONY_OK)
		return r;

	for (i = -1; i < chrony_get_report_number_records(s) + 1; i++) {
		fprintf(out, "request record %d\n", i);
		r = chrony_request_record(s, report_name, i);
		if (r != CHRONY_OK)
			continue;

		r = process_responses(s);
		fprintf(out, "response %s\n", chrony_get_error_string(r));
		if (r != CHRONY_OK)
			continue;

		for (j = -1; j < chrony_get_record_number_fields(s) + 1; j++) {
			content = chrony_get_field_content(s, j);
			fprintf(out, "%d\n", content);

			str = chrony_get_field_name(s, j);
			if (str)
				fprintf(out, "%s\n", str);

			fprintf(out, "%d\n", chrony_get_field_type(s, j));

			fprintf(out, "%"PRIu64"\n", chrony_get_field_uinteger(s, j));

			for (flag = 1; flag != 0; flag <<= 1) {
				str = chrony_get_field_constant_name(s, j, flag);
				if (str)
					fprintf(out, "%s\n", str);
			}

			fprintf(out, "%"PRId64"\n", chrony_get_field_integer(s, j));

			fprintf(out, "%f\n", chrony_get_field_float(s, j));

			str = chrony_get_field_string(s, j);
			if (str)
				fprintf(out, "%s\n", str);

			ts = chrony_get_field_timespec(s, j);

			fprintf(out, "%"PRIu64".%09"PRIu32"\n",
				(uint64_t)ts.tv_sec, (uint32_t)ts.tv_nsec);
		}
	}

	return CHRONY_OK;
}

int main(int argc, char **argv) {
	int fd[2], report_index;
	chrony_session *s;

	in = stdin;
	if (0) {
		out = fopen("/dev/null", "w");
		if (!out) {
			perror("fopen");
			return 1;
		}
	} else {
		out = stdout;
	}

	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fd) < 0) {
		perror("socketpair");
		return 1;
	}
	server_fd = fd[1];

	if (chrony_init_session(&s, fd[0]) != CHRONY_OK)
		return 1;

	if (fread(&report_index, sizeof (report_index), 1, in) != 1)
		return 0;
	report_index = ntohl(report_index);
	fprintf(out, "%d\n", report_index);

	dump_report(s, report_index);

	chrony_deinit_session(s);

	close(fd[0]);
	close(fd[1]);
	fclose(in);
	fclose(out);

	return 0;
}
