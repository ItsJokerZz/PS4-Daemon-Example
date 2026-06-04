LIBPS4      := $(PS4SDK)

CC          := gcc
OBJCOPY     := objcopy
ODIR        := build
SDIR        := source
IDIRS       := -I$(LIBPS4)/include -Iinclude
LDIRS       := -L$(LIBPS4)
MAPFILE     := $(ODIR)/DaemonLauncher.map

CFLAGS      := $(IDIRS) -Os -std=c11 -ffunction-sections -fdata-sections \
               -fno-builtin -nostartfiles -nostdlib -Wall -Wextra \
               -masm=intel -march=btver2 -mtune=btver2 -m64 -mabi=sysv \
               -mcmodel=small -fpie -fPIC -Wno-unused-parameter

LFLAGS      := $(LDIRS) -Xlinker -T $(LIBPS4)/linker.x -Xlinker -Map="$(MAPFILE)" \
               -Wl,--build-id=none -Wl,--gc-sections

CFILES      := $(wildcard $(SDIR)/*.c)
OBJS        := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES)) 

LIBS        := -lPS4
TARGET      := DaemonLauncher.bin

$(TARGET): $(ODIR) $(OBJS)
	$(CC) $(LIBPS4)/crt0.s $(ODIR)/*.o -o temp.t $(CFLAGS) $(LFLAGS) $(LIBS)
	$(OBJCOPY) -O binary temp.t "$(TARGET)"
	rm -f temp.t

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR):
	@mkdir $@

.DEFAULT_GOAL := all

clean:
	rm -rf "$(TARGET)" "$(MAPFILE)" $(ODIR)

all: $(TARGET)

master: clean $(TARGET)
