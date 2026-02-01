CFLAGS = -Wall -O2 -g

.PHONY: libmigdb

all: libmigdb

libmigdb:
	$(MAKE) -C src CFLAGS="$(CFLAGS)"

clean:
	$(MAKE) -C src clean
	$(MAKE) -C examples clean
	-@rm version

install:
	$(MAKE) -C src install