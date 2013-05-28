#
# Makefile for busy-wait IO tests
#

ifeq ($(ARCH), x86)
	include x86.mk
else
	include arm.mk
endif

OBJECTS = bwio.o scheduling.o circular_queue.o task.o messaging.o memory.o ksyscalls.o

all: kernel.elf 

tests: test circular_queue_tests messaging_tests ksyscalls_tests

.SUFFIXES:

.SECONDARY:

DEPS = $(OBJECTS:.o=.depends)

%.s : %.c
	$(CC) -S $(CFLAGS) $<

%.depends: %.c
	$(CC) -M $(CFLAGS) $< > $@

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $< 

kernel.elf : kernel.o context_switch.o syscall.o user.o time.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
test : test.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

circular_queue_tests : circular_queue_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

messaging_tests: messaging_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

ksyscalls_tests: ksyscalls_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
clean:
	-rm -f *.elf *.o kernel.map test `find ./*.s ! -name context_switch.s`

install: kernel.elf
	-cp kernel.elf /u/cs452/tftp/ARM/ai/

-include $(DEPS)
