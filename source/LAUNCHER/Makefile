TITLE       := Daemon Example
VERSION     := 13.37
TITLE_ID    := DMON00001
CONTENT_ID  := IV0000-DMON00001_00-DAEMON_EXAMPLE00

LIBMODULES  := $(wildcard sce_module/*)

TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	CC      := clang
	CCX     := clang++
	LD      := ld.lld
	CDIR    := linux
endif

PROJDIR     := $(shell basename $(CURDIR))
SRCDIR      := source
INTDIR      := build

DAEMON_OBJ  := ../DAEMON/build/daemon.o

CPP         := $(shell find $(SRCDIR) -name '*.cpp')
OBJS        := $(patsubst $(SRCDIR)/%.cpp,$(INTDIR)/%.o,$(CPP)) $(DAEMON_OBJ)

CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c $(EXTRAFLAGS) \
               -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include

CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1

           
LIBS        := -lc -lc++ -lSceLibcInternal -lkernel -ljbc -lSceSysmodule -lSceSystemService 

LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr \
               -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

OBJDIRS := $(sort $(dir $(OBJS)))
$(shell mkdir -p $(OBJDIRS))

.DEFAULT_GOAL := all

$(CONTENT_ID).pkg: pkg.gp4
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core pkg_build $< .

pkg.gp4: eboot.bin sce_sys/param.sfo sce_sys/icon0.png $(LIBMODULES)
	$(TOOLCHAIN)/bin/$(CDIR)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "$^"

sce_sys/param.sfo: Makefile
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_new $@
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1 
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 32  
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value 'gde'  
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0 
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0  
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(TOOLCHAIN)/bin/$(CDIR)/PkgTool.Core sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

eboot.bin: $(OBJS)
	$(LD) $(OBJS) -o $(INTDIR)/$(PROJDIR).elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(INTDIR)/$(PROJDIR).elf \
	-out=$(INTDIR)/$(PROJDIR).oelf --eboot "eboot.bin" --paid 0x3800000000000011 \
	--authinfo 000000000000000000000000001C004000FF000000000080000000000000000000000000000000000000008000400040000000000000008000000000000000080040FFFF000000F000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

$(INTDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(CONTENT_ID).pkg pkg.gp4 pkg/sce_sys/param.sfo eboot.bin \
	$(INTDIR)/$(PROJDIR).elf $(INTDIR)/$(PROJDIR).oelf $(INTDIR)/*.o

all: $(CONTENT_ID).pkg

master: clean all
