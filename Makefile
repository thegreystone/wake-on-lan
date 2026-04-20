CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
TARGET   = wol
VERSION := $(shell git describe --tags --always 2>/dev/null || echo dev)

$(TARGET): wol.c
	$(CC) $(CFLAGS) -DVERSION='"$(VERSION)"' -o $(TARGET) wol.c

static: wol.c
	$(CC) $(CFLAGS) -static -DVERSION='"$(VERSION)"' -o $(TARGET) wol.c

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: static install clean
