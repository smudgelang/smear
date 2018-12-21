OS ?= linux
SRC += smear.c smeartime.c
LIBS += -pthread
VPATH := $(VPATH) src/smear/$(OS)/
CFLAGS += -D_POSIX_C_SOURCE=199309L
SRC += smeartime-platform.c
