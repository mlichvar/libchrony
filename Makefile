CC = cc
CFLAGS = -O2 -Wall -g
LDFLAGS =
INSTALL = install
LIBTOOL = libtool

name = libchrony
version = 0.1

libs = -lm

lib = $(name).la
lib_version = 0:0:0

prefix = /usr/local
libdir = $(prefix)/lib
pkgconfigdir = $(libdir)/pkgconfig
includedir = $(prefix)/include

objs = $(patsubst %.c,%.o,$(wildcard *.c))
headers = chrony.h
examples = example-reports

all: $(lib) $(examples)

%.lo: %.c
	$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) -c $<

$(lib): client.lo message.lo socket.lo
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -version-info $(lib_version) \
		-rpath $(libdir) -o $@ $^ $(LDFLAGS) $(libs)

example-reports: example-reports.o $(lib)
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(libs)

fuzz: fuzz.o $(lib)
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(libs)

install: $(lib)
	mkdir -p $(DESTDIR)$(libdir)/pkgconfig $(DESTDIR)$(includedir)
	$(LIBTOOL) --mode=install $(INSTALL) $(lib) $(DESTDIR)$(libdir)
	$(INSTALL) -m644 $(headers) $(DESTDIR)$(includedir)
	@echo "Generating $(DESTDIR)$(pkgconfigdir)/$(name).pc"
	@echo "Name: $(name)" > $(DESTDIR)$(pkgconfigdir)/$(name).pc
	@echo "Version: $(version)" >> $(DESTDIR)$(pkgconfigdir)/$(name).pc
	@echo "Description: Library for chronyd monitoring" >> \
		$(DESTDIR)$(pkgconfigdir)/$(name).pc
	@echo "Libs: -L$(libdir) -l$(subst lib,,$(name))" >> \
		$(DESTDIR)$(pkgconfigdir)/$(name).pc
	@echo "Cflags: -I$(includedir)" >> $(DESTDIR)$(pkgconfigdir)/$(name).pc

clean:
	-rm -rf $(lib) $(examples) *.o *.lo .deps .libs

.deps:
	@mkdir .deps

.deps/%.d: %.c .deps
	@$(CC) -MM $(CPPFLAGS) -MT '$(<:%.c=%.lo) $@' $< -o $@

-include $(objs:%.o=.deps/%.d)
