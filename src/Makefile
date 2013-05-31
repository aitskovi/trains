#
# Makefile for busy-wait IO tests
#

VPATH += libc/ test/ user/ user/rps user/libc
INCLUDES = -I. -I../include -I../include/libc -I../include/user -I../include/user/rps

ifeq ($(ARCH), x86)
	include x86.mk
else
	include arm.mk
endif

ARMOBJECTS = kernel.o context_switch.o syscall.o user.o time.o nameserver.o
OBJECTS = bwio.o scheduling.o circular_queue.o task.o messaging.o memory.o ksyscalls.o nameservice.o string.o random.o

all: kerneltimings.elf kernel2.elf

tests: test circular_queue_tests messaging_tests ksyscalls_tests

opt: CFLAGS+=-O2
opt: all

.SUFFIXES:

.SECONDARY:

DEPS = $(OBJECTS:.o=.depends)

%.s : %.c
	$(CC) -S $(CFLAGS) $(INCLUDES) $<

bwio.s : bwio.c
	$(CC) -S $(CFLAGS) $(INCLUDES) -O0 $<

%.depends: %.c
	$(CC) -M $(CFLAGS) $(INCLUDES) $< > $@

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $< 

kernel.elf : $(ARMOBJECTS) $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
kernel2.elf : kernel2.o rpsclient.o rpsserver.o $(ARMOBJECTS) $(OBJECTS) 
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
kerneltimings.elf : timings.o $(ARMOBJECTS) $(OBJECTS) 
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
scheduling_tests : scheduling_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

circular_queue_tests : circular_queue_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

messaging_tests: messaging_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

ksyscalls_tests: ksyscalls_tests.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
clean:
	-rm -f *.depends *.elf *.o kernel.map `find ./*.s ! -name context_switch.s`

install: all
	-cp kernel2.elf kerneltimings.elf /u/cs452/tftp/ARM/ai/

-include $(DEPS)
