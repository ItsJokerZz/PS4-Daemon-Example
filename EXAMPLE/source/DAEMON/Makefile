TITLE_ID    := DMON00000

PROJDIR     := source
INTDIR      := build

TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	CC      := clang
	CCX     := clang++
	LD      := ld.lld
	CDIR    := linux
endif

CPP        := $(wildcard $(PROJDIR)/*.cpp) $(wildcard $(PROJDIR)/libraries/*.cpp)
OBJS       := $(patsubst $(PROJDIR)/%.cpp,$(INTDIR)/%.o,$(CPP))

INCLUDES   := -Iheaders -Iheaders/libraries -Iexternal/freetype-gl/include -Iexternal/sqlite/include -Iinclude
CFLAGS     := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c \
              -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include $(INCLUDES) \
              -std=c26 -D_DEFAULT_SOURCE -DFMT_HEADER_ONLY

CXXFLAGS   := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1 -std=c++26

LIBS       := -lc -lc++ -lSceLibcInternal -lkernel -lkernel_sys

LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr \
               -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

.DEFAULT_GOAL := all

OBJDIRS := $(sort $(dir $(OBJS)))
$(shell mkdir -p $(OBJDIRS))


$(INTDIR)/%.o: $(PROJDIR)/%.cpp
	$(CCX) $(CXXFLAGS) \
	-o $@ $<

eboot.bin: $(OBJS)
	$(LD) $(OBJS) -o $(INTDIR)/daemon.elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself \
		-in=$(INTDIR)/daemon.elf \
		-out=$(INTDIR)/daemon.oelf \
		--eboot eboot.bin \
		--paid 0x3800000000000011 \
		--authinfo 000000000000000000000000001C004000FF000000000080000000000000000000000000000000000000008000400040000000000000008000000000000000080040FFFF000000F000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 eboot.bin $(INTDIR)/daemon.o

clean:
	rm -f eboot.bin $(INTDIR)/daemon.elf $(INTDIR)/daemon.oelf $(INTDIR)/*.o

all: eboot.bin

master: clean all
