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
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#define MAX_UN_PATH_LENGTH (sizeof ((struct sockaddr_un *)NULL)->sun_path)

static void remove_unix_socket(int fd) {
	struct sockaddr_un addr;
	socklen_t len;
	char *s;
	int i;

	/* Remove the socket and the two directories in which it was placed */

	len = sizeof (addr);
	if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0 ||
	    addr.sun_family != AF_UNIX || len > sizeof (addr) ||
	    strnlen(addr.sun_path, sizeof (addr.sun_path)) >= sizeof (addr.sun_path))
		return;

	if (unlink(addr.sun_path) < 0)
		return;

	for (i = 0; i < 2; i++) {
		s = strrchr(addr.sun_path, '/');
		if (!s)
			return;
		*s = '\0';

		if (rmdir(addr.sun_path) < 0)
			return;
	}
}

static int generate_random_path(FILE *urandom, char *path, int length) {
	int i;

	for (i = 0; i + 1 < length; ) {
		if (fread(&path[i], 1, 1, urandom) != 1)
			return 0;
		if ((path[i] >= 'A' && path[i] <= 'Z') ||
		    (path[i] >= 'a' && path[i] <= 'z') ||
		    (path[i] >= '0' && path[i] <= '9'))
			i++;
	}
	path[i] = '\0';

	return 1;
}

static int open_unix_socket(const char *path) {
	char dir0[MAX_UN_PATH_LENGTH], dir1[MAX_UN_PATH_LENGTH], dir2[MAX_UN_PATH_LENGTH];
	char rand1[12 + 1], rand2[16 + 1], *s;
	struct sockaddr_un client_un, server_un;
	int fd = -1, dir_fd1 = -1;
	struct stat st;
	FILE *urandom;

	memset(&server_un, 0, sizeof (server_un));
	server_un.sun_family = AF_UNIX;
	memset(&client_un, 0, sizeof (client_un));
	client_un.sun_family = AF_UNIX;

	/* First, try to connect to the server socket without binding our
	   socket to see if it can actually be accessed */

	if (snprintf(server_un.sun_path, sizeof (server_un.sun_path),
		     "%s", path) >= sizeof (server_un.sun_path)) {
		errno = EINVAL;
		goto error1;
	}

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0 ||
	    connect(fd, (struct sockaddr *)&server_un, sizeof (server_un)) < 0)
		goto error1;
	close(fd);
	fd = -1;

	/* Prepare the path of the client socket and directories that will be
	   created in order to hide the socket from chronyd running under a
	   different user before the socket permissions are changed by chmod()
	   later in this function. If the chronyd process was compromised and
	   the socket path was readable or predictable, it could try to replace
	   the socket with a symlink in order for the chmod() call to change
	   permissions of something else.

	   The client socket path is constructed as follows:
		   dir0 = dirname($server_un)
		   dir1 = $dir0/libchrony.$rand1
		   dir2 = $dir0/libchrony.$rand1/$rand2
		   client_un = $dir0/libchrony.$rand1/$rand2/sock */

	if (snprintf(dir0, sizeof (dir0), "%s", path) >= sizeof (dir0)) {
		errno = EINVAL;
		goto error1;
	}

	s = strrchr(dir0, '/');
	if (!s || s == dir0) {
		errno = EINVAL;
		goto error1;
	}
	*s = '\0';

	urandom = fopen("/dev/urandom", "r");
	if (!urandom)
		goto error1;

	if (!generate_random_path(urandom, rand1, sizeof (rand1)) ||
	    !generate_random_path(urandom, rand2, sizeof (rand2))) {
		fclose(urandom);
		goto error1;
	}
	fclose(urandom);

	if (snprintf(dir1, sizeof (dir1),
		     "%s/libchrony.%s", dir0, rand1) >= sizeof (dir1) ||
	    snprintf(dir2, sizeof (dir2),
		     "%s/%s", dir1, rand2) >= sizeof (dir2) ||
	    snprintf(client_un.sun_path, sizeof (client_un.sun_path),
		     "%s/sock", dir2) >= sizeof (client_un.sun_path)) {
		errno = EINVAL;
		return -1;
	}

	/* Create the directories and bind the socket */

	if (mkdir(dir1, 0711) < 0)
		goto error1;

	dir_fd1 = open(dir1, O_RDONLY | O_NOFOLLOW);
	if (dir_fd1 < 0 ||
	    fstat(dir_fd1, &st) < 0)
		goto error2;

	if (!S_ISDIR(st.st_mode) ||
	    (st.st_mode & 0777 & ~0711) != 0 ||
	    st.st_uid != geteuid()) {
		errno = EPROTO;
		goto error2;
	}

	if (mkdirat(dir_fd1, rand2, 0711) < 0)
		goto error2;

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0 ||
	    bind(fd, (struct sockaddr *)&client_un, sizeof (client_un)) < 0)
		goto error3;

	/* Allow chronyd running under a different user to send responses to
	   our socket. Access to the socket is protected by the directory. */

	if (chmod(client_un.sun_path, 0666) < 0 ||
	    chmod(dir2, 0711) < 0 ||
	    fchmod(dir_fd1, 0711) < 0)
		goto error4;

	if (connect(fd, (struct sockaddr *)&server_un, sizeof (server_un)) < 0)
		goto error4;

	close(dir_fd1);

	return fd;

error4:
	unlink(client_un.sun_path);
error3:
	rmdir(dir2);
error2:
	rmdir(dir1);
error1:
	if (fd >= 0)
		close(fd);
	if (dir_fd1 >= 0)
		close(dir_fd1);
	return -1;
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
