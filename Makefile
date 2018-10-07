ifeq ($(OS),Windows_NT)
PKGEXT=zip
PLATFORM=windows
else
PKGEXT=tgz deb
PLATFORM=$(shell dpkg --print-architecture)
endif
OBJDIR := obj
SRCDIR := src
MODULES := smear cancelq number
LIBS :=
SRC := 
CC := gcc
CFLAGS := -ggdb3 -std=c99 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -fvisibility=hidden -O3 -pedantic
INCLUDE := -Iinclude $(foreach mod, $(MODULES), -Isrc/$(mod))
VPATH := $(foreach mod, $(MODULES), src/$(mod)) include
SMEAR_RELEASE_SUBDIR=smear
SMEAR_RELEASE_STAGE_DIR=$(OBJDIR)/$(SMEAR_RELEASE_SUBDIR)
SMEAR_VERSION=$(shell grep "\bSMEAR_VERSION\b" include/smear/version.h | cut -f 2 -d '"')

default: all

include tests/tests.mk
include $(patsubst %,$(SRCDIR)/%/module.mk, $(MODULES))
OBJ := $(SRC:%.c=$(OBJDIR)/%.o)

LIBS := $(sort $(LIBS))

.PHONY: clean default all tests \
        package zip tgz deb


debug:
	@echo inc $(INCLUDE)
	@echo src $(SRC)
	@echo libs $(LIBS)
	@echo obj $(OBJ)
	@echo arch $(ARCH)
	@echo os $(OS)
	@echo vpath $(VPATH)

all: libsmear.a libsmear.dmp tests obj/libsmear.a


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

obj/libsmear.a: libsmear.a
	strip -o $@ $<

obj/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -c -o $@ $<

stage: libsmear.a
	rm -rf $(SMEAR_RELEASE_STAGE_DIR)
	mkdir -p $(SMEAR_RELEASE_STAGE_DIR)
	cp $< $(SMEAR_RELEASE_STAGE_DIR)
	cp -r include $(SMEAR_RELEASE_STAGE_DIR)
	cp CHANGES $(SMEAR_RELEASE_STAGE_DIR)
	cp LICENSE $(SMEAR_RELEASE_STAGE_DIR)
	cp README.md $(SMEAR_RELEASE_STAGE_DIR)

package: $(foreach EXT,$(PKGEXT),libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).$(EXT))

zip: libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).zip
libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).zip: stage
	cd $(OBJDIR) && \
	if type zip >/dev/null 2>&1; then \
	    zip -r $@ $(SMEAR_RELEASE_SUBDIR); \
	elif type 7z >/dev/null 2>&1; then \
	    7z a $@ $(SMEAR_RELEASE_SUBDIR); \
	fi
	mv $(OBJDIR)/$@ .

tgz: libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).tgz
libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).tgz: stage
	cd $(OBJDIR) && \
	fakeroot tar -czf $@ $(SMEAR_RELEASE_SUBDIR)
	mv $(OBJDIR)/$@ .

deb: libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).deb
libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).deb: libsmear.a
	debuild -i -us -uc -nc -b
	mv ../libsmear-dev_$(SMEAR_VERSION)_$(PLATFORM).deb .
	mv ../libsmear_$(SMEAR_VERSION)_$(PLATFORM).* .

clean:
	rm -rf debian/libsmear-dev
	rm -rf debian/.debhelper
	rm -rf obj/* *.a test-* *.dmp *.deb *.tgz *.zip *.build *.buildinfo *.changes
	rm -f debian/files debian/libsmear-dev.substvars
	rm -f debian/debhelper-build-stamp
