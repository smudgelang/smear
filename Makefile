OBJDIR := obj
SRCDIR := src
MODULES := smear queue
LIBS :=
SRC := 
CC := gcc
CFLAGS := -g -std=c99 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function
INCLUDE := $(foreach mod, $(MODULES), -Isrc/$(mod))
VPATH := $(foreach mod, $(MODULES), src/$(mod))

include $(patsubst %,$(SRCDIR)/%/module.mk, $(MODULES))
OBJ := $(SRC:%.c=$(OBJDIR)/%.o)

LIBS := $(sort $(LIBS))

default: all

.PHONY: clean

debug:
	@echo src $(SRC)
	@echo libs $(LIBS)
	@echo obj $(OBJ)
	@echo arch $(ARCH)
	@echo vpath $(VPATH)

all: libsmear.a

obj/%.d: %.c # This is lifted from the GNU Make tutorial, with slight modification.
	@set -e; rm -f $@; \
	  $(CC) -MM -MG $(CFLAGS) $< > $@.$$$$; \
	  sed 's,\($*\)\.o[ :]*,obj/\1.o $@ : ,g' < $@.$$$$ > $@; \
	  rm -f $@.$$$$

-include $(OBJ:.o=.d)

libsmear.a: $(OBJ)
	ar -cvq $@ $(OBJ)

obj/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -c -o $@ $<

clean:
	rm -f obj/* *.a
