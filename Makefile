TARGET  := cdmenu
SCRIPTS :=
CFLAGS  ?= -Os -pipe -march=native
LDFLAGS ?= -s
CFLAGS  +=
LDFLAGS += -lcurses -lform -lmenu
PREFIX  ?= /usr/local
DESTDIR ?= bin

all: $(TARGET)

install: $(TARGET) $(SCRIPTS)
	$(foreach I, $^, install -D $(I) $(PREFIX)/$(DESTDIR)/$(I);)
