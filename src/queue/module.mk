ARCH ?= posix
LIBS += -pthread
VPATH := $(VPATH) src/queue/$(ARCH)/
SRC += queue.c
