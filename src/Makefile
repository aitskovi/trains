#
# Makefile for busy-wait IO tests
#

PARSE_TRACK = parse_track

VPATH += libc/ test/ user/ user/rps/ user/libc/ user/clock user/serial/ track/
INCLUDES = -I. -I../include -I../include/libc -I../include/user -I../include/user/rps -I../include/user/clock -I../include/user/serial -I../include/track

ifeq ($(ARCH), x86)
	include x86.mk
else
	include arm.mk
endif

ARMOBJECTS = kernel.o\
			 context_switch.o\
			 user.o\
			 fine_timer.o\
			 nameserver.o\
			 log_arm.o\
			 idle.o\
			 clock_server.o\
			 clock_notifier.o\
			 write_notifier.o\
			 write_server.o\
			 read_notifier.o\
			 read_server.o\
			 swi_arm.o\
			 train_server.o\
			 sensor_server.o\
			 switch_server.o\
			 sensor_notifier.o
			 
X86OBJECTS = log_x86.o\
	   		 swi_x86.o
	   		 
OBJECTS = bwio.o\
		  scheduling.o\
		  circular_queue.o\
		  task.o\
		  messaging.o\
		  memory.o\
		  ksyscalls.o\
		  nameservice.o\
		  string.o\
		  random.o\
		  bits.o\
		  heap_priority_queue.o\
		  waiting.o\
		  syscall.o\
		  verify.o\
		  event.o\
		  interrupt.o\
		  uart.o\
		  write_service.o\
		  read_service.o\
		  serial.o\
		  conv.o\
		  sprintf.o\
		  shell.o\
		  clock_widget.o\
		  ring_buffer.o\
		  track_data.o

all: kerneltimings.elf kernel2.elf kernel3.elf kernel4.elf

tests: unit_tests

opt: CFLAGS+=-O2
opt: all

dbg: CFLAGS+=-DDEBUG
dbg: all

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
	
track_data.c track_data.h : tracka trackb
	$(PARSE_TRACK) $^

kernel.elf : $(ARMOBJECTS) $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
kernel2.elf : kernel2.o rpsclient.o rpsserver.o $(ARMOBJECTS) $(OBJECTS) 
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

kernel3.elf : kernel3.o $(ARMOBJECTS) $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc

kernel4.elf : kernel4.o $(ARMOBJECTS) $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
kerneltimings.elf : timings.o $(ARMOBJECTS) $(OBJECTS) 
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
unit_tests: unit.o ksyscalls_tests.o messaging_tests.o circular_queue_tests.o memory_tests.o priority_queue_tests.o scheduling_tests.o $(OBJECTS) $(X86OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
clean:
	-rm -f *.depends *.elf *.o kernel.map *_tests `find ./*.s ! -name context_switch.s` track_data.c track_data.h

install: all
	-cp *.elf /u/cs452/tftp/ARM/$(USER)/

-include $(DEPS)
