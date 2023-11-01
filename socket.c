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

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#define MAX_UNIX_SOCKET_INDEX 1000

static void remove_unix_socket(int fd) {
	struct sockaddr_un addr;
	socklen_t len;

	len = sizeof (addr);
	if (getsockname(fd, (struct sockaddr *)&addr, &len) == 0 &&
	    addr.sun_family == AF_UNIX && len <= sizeof (addr)) {
		unlink(addr.sun_path);
	}
}

static int open_unix_socket(const char *path) {
	struct sockaddr_un sa;
	char *s, dir[256];
	int i, fd, fd2;

	if (snprintf(dir, sizeof (dir), "%s", path) >= sizeof (dir)) {
		errno = EINVAL;
		return -1;
	}

	s = strrchr(dir, '/');
	if (!s || s == dir) {
		errno = EINVAL;
		return -1;
	}
	*s = '\0';

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	sa.sun_family = AF_UNIX;

	for (i = 1; i <= MAX_UNIX_SOCKET_INDEX; i++) {
		if (snprintf(sa.sun_path, sizeof (sa.sun_path),
			     "%s/libchrony.%d", dir, i) >= sizeof (sa.sun_path)) {
			close(fd);
			errno = EINVAL;
			return -1;
		}

		if (bind(fd, (struct sockaddr *)&sa, sizeof (sa)) == 0)
			break;

		if (errno != EADDRINUSE) {
			close(fd);
			return -1;
		}

		fd2 = socket(AF_UNIX, SOCK_DGRAM, 0);
		if (fd2 < 0) {
			close(fd);
			return -1;
		}

		/* Remove the conflicting socket if it is no longer used */
		if (connect(fd2, (struct sockaddr *)&sa, sizeof (sa)) < 0 &&
		    errno == ECONNREFUSED && unlink(sa.sun_path) == 0)
			i--;
		close(fd2);
	}

	if (i > MAX_UNIX_SOCKET_INDEX) {
		close(fd);
		errno = EMFILE;
		return -1;
	}

	/* Allow chronyd running under a different user to send responses to
	   our socket. Access to the socket is protected by the directory. */
	chmod(sa.sun_path, 0666);

	if (snprintf(sa.sun_path, sizeof (sa.sun_path), "%s", path) >= sizeof (sa.sun_path)) {
		remove_unix_socket(fd);
		close(fd);
		errno = EINVAL;
		return -1;
	}

	if (connect(fd, (struct sockaddr *)&sa, sizeof (sa)) < 0) {
		remove_unix_socket(fd);
		close(fd);
		return -1;
	}

	return fd;
}

static int open_inet_socket(const char *address) {
	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
		struct sockaddr sa;
	} sa;
	char buf[256], *addr, *s;
	int i, fd, port, colons;

	if (snprintf(buf, sizeof (buf), "%s", address) >= sizeof (buf)) {
		errno = EINVAL;
		return -1;
	}

	addr = buf;
	port = 323;

	for (i = colons = 0; buf[i] != '\0'; i++) {
		if (buf[i] == ':')
			colons++;
	}
	if (colons == 1 || (colons >= 3 && buf[0] == '[')) {
		s = strrchr(buf, ':');
		if (!s || s == buf || s[1] == '\0') {
			errno = EINVAL;
			return -1;
		}
		if (colons >= 3 && s[-1] == ']') {
			addr = buf + 1;
			s[-1] = '\0';
		}
		s[0] = '\0';
		port = atoi(s + 1);
	}

	memset(&sa, 0, sizeof (sa));

	if (inet_pton(AF_INET, addr, &sa.in4.sin_addr.s_addr) == 1) {
		sa.in4.sin_port = htons(port);
		sa.in4.sin_family = AF_INET;
	} else if (inet_pton(AF_INET6, addr, &sa.in6.sin6_addr.s6_addr) == 1) {
		sa.in6.sin6_port = htons(port);
		sa.in6.sin6_family = AF_INET6;
	} else {
		errno = EINVAL;
		return -1;
	}

	fd = socket(sa.sa.sa_family, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	if (connect(fd, (struct sockaddr *)&sa, sizeof (sa)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

int chrony_open_socket(const char *address) {
	int fd;

	if (!address || address[0] == '\0') {
		fd = open_unix_socket("/var/run/chrony/chronyd.sock");
		if (fd >= 0)
			return fd;
		fd = open_inet_socket("127.0.0.1:323");
		if (fd >= 0)
			return fd;
		fd = open_inet_socket("[::1]:323");
		return fd;
	}

	if (address[0] == '/')
		return open_unix_socket(address);
	else
		return open_inet_socket(address);
}

void chrony_close_socket(int fd) {
	remove_unix_socket(fd);
	close(fd);
}
