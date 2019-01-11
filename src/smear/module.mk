OS ?= linux
ARCH += posix
SRC += smear.c smeartime.c
LIBS += -pthread
VPATH := $(VPATH) src/smear/$(ARCH)/ src/smear/$(OS)/
CFLAGS += -D_POSIX_C_SOURCE=199309L
SRC += smeartime-platform.c thread-utils.c
