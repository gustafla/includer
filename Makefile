DEBUG?=1

# basic configuration
EXECUTABLE:=includer
PREFIX:=~/.local
CC:=gcc
CFLAGS:=-std=c99 -Wall -Wextra -Wpedantic -Ilib/uthash/include

# library packages for pkg-config
PKGS:=

# debug and release build flags
ifeq ($(DEBUG), 0)
BUILDDIR:=release
CFLAGS+=-O2 -s -fno-plt -Wl,-O2,--sort-common,--as-needed,-z,relro,-z,now
else
BUILDDIR:=debug
CFLAGS+=-Og -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
endif

# add pkg-config libs and cflags
ifneq ($(strip $(PKGS)),)
CFLAGS+=$(shell pkg-config --cflags $(PKGS))
LDLIBS+=$(shell pkg-config --libs $(PKGS))
endif

TARGET:=$(BUILDDIR)/$(EXECUTABLE)
SOURCES:=$(wildcard src/*.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES:src/%=$(BUILDDIR)/%))

$(TARGET): $(OBJS)
	$(info Linking $@)
	@mkdir -p $(@D)
	@$(CC) -o $(TARGET) $(CFLAGS) $(OBJS) $(LDLIBS)

$(BUILDDIR)/%.o: src/%.c
	$(info Compiling $<)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean install

clean:
	rm -rf $(BUILDDIR)

install: $(TARGET)
	cp $(TARGET) $(PREFIX)/bin
