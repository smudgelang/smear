ARCH ?= posix
LIBS += -pthread
VPATH := $(VPATH) src/cancelq/$(ARCH)/
SRC += cancellable.c

# Source includes are weird. Here's an explicit dependency.
obj/cancellable.o: heap.c
