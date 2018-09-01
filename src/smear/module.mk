OS ?= linux
SRC += smear.c
LIBS += -pthread
VPATH := $(VPATH) src/smear/$(OS)/
CFLAGS += -D_POSIX_C_SOURCE=199309L
SRC += smeartime.c
