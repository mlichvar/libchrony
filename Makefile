CC = cc
CFLAGS = -O2 -Wall -g
LDFLAGS = -lm
INSTALL = install
LIBTOOL = libtool

name = libchrony
version = 0.1

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
		-rpath $(libdir) -o $@ $^ $(LDFLAGS)

example-reports: example-reports.o $(lib)
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

fuzz: fuzz.o $(lib)
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install: $(lib)
	mkdir -p $(libdir) $(libdir)/pkgconfig $(includedir)
	$(LIBTOOL) --mode=install $(INSTALL) $(lib) $(libdir)
	$(INSTALL) -m644 $(headers) $(includedir)
	@echo "Generating $(pkgconfigdir)/$(name).pc"
	@echo "Name: $(name)" > $(pkgconfigdir)/$(name).pc
	@echo "Version: $(version)" >> $(pkgconfigdir)/$(name).pc
	@echo "Description: Library for chronyd monitoring" >> $(pkgconfigdir)/$(name).pc
	@echo "Libs: -L$(libdir) -l$(subst lib,,$(name))" >> $(pkgconfigdir)/$(name).pc
	@echo "Cflags: -I$(includedir)" >> $(pkgconfigdir)/$(name).pc

clean:
	-rm -rf $(lib) $(examples) *.o *.lo .deps .libs

.deps:
	@mkdir .deps

.deps/%.d: %.c .deps
	@$(CC) -MM $(CPPFLAGS) -MT '$(<:%.c=%.lo) $@' $< -o $@

-include $(objs:%.o=.deps/%.d)
