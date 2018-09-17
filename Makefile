OBJDIR := obj
SRCDIR := src
MODULES := smear cancelq number
LIBS :=
SRC := 
CC := /usr/bin/gcc
CFLAGS := -ggdb3 -std=c99 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -fvisibility=hidden -O3 -pedantic
INCLUDE := -Iinclude $(foreach mod, $(MODULES), -Isrc/$(mod))
VPATH := $(foreach mod, $(MODULES), src/$(mod)) include

default: all

include tests/tests.mk
include $(patsubst %,$(SRCDIR)/%/module.mk, $(MODULES))
OBJ := $(SRC:%.c=$(OBJDIR)/%.o)

LIBS := $(sort $(LIBS))

.PHONY: clean default all tests


debug:
	@echo inc $(INCLUDE)
	@echo src $(SRC)
	@echo libs $(LIBS)
	@echo obj $(OBJ)
	@echo arch $(ARCH)
	@echo os $(OS)
	@echo vpath $(VPATH)

all: libsmear.a libsmear.dmp tests


-include $(OBJ:.o=.d)

%.dmp: %.a
	objdump -dSt $< > $@

obj/%.d: %.c # Slightly modified from GNU Make tutorial
	@set -e; rm -f $@; \
	  $(CC) -MM -MG $(CFLAGS) $< > $@.$$$$; \
	  sed 's,\($*\)\.o[ :]*,obj/\1.o $@ : ,g' < $@.$$$$ > $@; \
	  rm -f $@.$$$$

libsmear.a: $(OBJ)
	$(LD) -r -o $@ $^
	objcopy --localize-hidden $@

obj/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -c -o $@ $<

clean:
	rm -f obj/* *.a test-* *.dmp
