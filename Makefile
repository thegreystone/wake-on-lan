CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
TARGET   = wol

$(TARGET): wol.c
	$(CC) $(CFLAGS) -o $(TARGET) wol.c

static: wol.c
	$(CC) $(CFLAGS) -static -o $(TARGET) wol.c

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: static install clean
