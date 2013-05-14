#
# Makefile for busy-wait IO tests
#

ifeq ($(ARCH), x86)
	include x86.mk
else
	include arm.mk
endif

OBJECTS = io.o ring_buffer.o timer.o ui.o train.o utility.o

all: a0.elf 

.SUFFIXES:
	

.SECONDARY:
	

%.s : %.c  
	$(CC) -S $(CFLAGS) $< 

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $< 

a0.elf : main.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
test : test.o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
	
clean:
	-rm -f *.elf *.s *.o a0.map test
