.PHONY: all clean release

CFLAGS = -g -Os -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS = -s -lwavpack
EXE ?= audacity-packer
RES ?=

AUPACK_OBJS = \
	audparse.o \
	aupack.o \
	auunpack.o \
	cli.o \
	filelist.o \
	filesize.o \
	fopen_wrapper.o \
	gui.o \
	main.o \
	pack.o \
	probe.o \
	projectlist.o \
	recfind.o \
	slurp.o \
	unpack.o \
	util.o \
	winstr.o \
	yxml.o

all: $(EXE)

%.res: %.rc
	$(RC) $< -O coff -o $@

$(EXE): $(AUPACK_OBJS) $(RES)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(AUPACK_OBJS) $(EXE) audacity-packer.res

release:
	docker build -t audacity-packer .
	mkdir -p output
	docker run --rm -ti -v $(shell pwd)/output:/output audacity-packer
