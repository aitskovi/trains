#
# Makefile for busy-wait IO tests
#

ifeq ($(ARCH), x86)
	include x86.mk
else
	include arm.mk
endif

OBJECTS = bwio.o syscall.o task.o

all: kernel.elf 

.SUFFIXES:
	

.SECONDARY:
	

DEPS = $(OBJECTS:.o=.depends)

%.s : %.c
	$(CC) -S $(CFLAGS) $<

%.depends: %.c
	$(CC) -M $(CFLAGS) $< > $@

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $< 

kernel.elf : kernel.o context_switch.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
test : test.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
clean:
	-rm -f *.elf *.o kernel.map test

install: kernel.elf
	-cp kernel.elf /u/cs452/tftp/ARM/aianus/

-include $(DEPS)
