SMUDGE ?= /usr/bin/smudge
# This is where your smear install lives. Change it if you didn't put it in ~.
SMEAR_ROOT ?= $(HOME)/smear
SMUDGE_OPTIONS_PNG=--dot-fmt=Png
CC=gcc
CFLAGS=-Wall -Werror -Wextra -Wno-unused-parameter -ggdb3
LFLAGS=-lsmear -pthread -lrt

# This is the path to your libsmear.a. Change it if it's somewhere else.
LPATH=-L$(SMEAR_ROOT)
# This is the path to your smear.h. Change it if it's somewhere else.
IPATH=-I$(SMEAR_ROOT)/include

SMUDGE_FILES=$(basename $(wildcard *.smudge))
ALWAYS_GENERATED=.h .c _ext.h
GENERATED_EXTENSIONS=$(ALWAYS_GENERATED) _ext.c .gv .png .svg
GENERATED_FILES=$(foreach x, $(GENERATED_EXTENSIONS), $(addsuffix $(x), $(SMUDGE_FILES)))
GEN_DOC=$(foreach x, $(ALWAYS_GENERATED), $(addsuffix $(x), $(SMUDGE_FILES)))

default: pinball-tutorial.pdf

all: 00_pinball 01_pinball 02_pinball 03_pinball 04_pinball 05_pinball 06_pinball 07_pinball 08_pinball 09_pinball 10_pinball pinball-tutorial.pdf

%.png %_ext.h %.c %.h: %.smudge
	$(SMUDGE) $(SMUDGE_OPTIONS_PNG) $^

pinball-tutorial.pdf: pinball-tutorial.rst 00_pinball.png *_main.c *.smudge $(GEN_DOC)
	rst2pdf $<

%.o: %.c
	$(CC) $(CFLAGS) $(IPATH) -c -o $@ $^

%_pinball: %_pinball.o %_main.o %_pinball.h %_pinball_ext.h
	$(CC) $(LPATH) -o $@ $(filter %.o, $^) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(GENERATED_FILES) *.o *_pinball *.pdf
