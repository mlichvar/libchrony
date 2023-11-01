CC = gcc
CFLAGS = -O2 -Wall -g
LDFLAGS = -lm
INSTALL = install
LIBTOOL = libtool

lib = libchrony.la
lib_version = 0:0:0

prefix = /usr/local
libdir = $(prefix)/lib
includedir = $(prefix)/include

objs = $(patsubst %.c,%.o,$(wildcard *.c))
headers = chrony.h
examples = example-reports

all: $(lib) $(examples)

%.lo: %.c
	$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) -c $<

$(lib): client.lo message.lo socket.lo
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -version-info $(lib_version) \
		-rpath $(libdir) -o $@ $^ $(LDFLAGS)

example-reports: example-reports.o $(lib)
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install: $(lib)
	mkdir -p $(libdir) $(includedir)
	$(LIBTOOL) --mode=install $(INSTALL) $(lib) $(libdir)
	$(INSTALL) -m644 $(headers) $(includedir)

clean:
	-rm -rf $(lib) $(examples) *.o *.lo .deps .libs

.deps:
	@mkdir .deps

.deps/%.d: %.c .deps
	@$(CC) -MM $(CPPFLAGS) -MT '$(<:%.c=%.lo) $@' $< -o $@

-include $(objs:%.o=.deps/%.d)
