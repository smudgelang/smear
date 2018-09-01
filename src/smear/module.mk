OS ?= linux
SRC += smear.c
LIBS += -pthread
VPATH := $(VPATH) src/smear/$(OS)/
SRC += smeartime.c
