ARCH ?= posix
LIBS += -pthread
VPATH := $(VPATH) src/cancelq/$(ARCH)/
SRC += cancellable.c
