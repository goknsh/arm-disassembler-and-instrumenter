CC=cc
CFLAGS += -g -fno-stack-protector -no-pie -fno-pic -I ./include
OBJDUMP = objdump
OBJS = arm_disas.o

.PHONY = all objs clean
all: fact fact2 fact3 fib

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

fib: main.c arm_callout.S fib.o
	$(CC) $(CFLAGS) -D__FIB__  $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis

fact: main.c arm_callout.S fact.o
	$(CC) $(CFLAGS) -D__FACT__  $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis

fact2: main.c arm_callout.S fact2.o
	$(CC) $(CFLAGS)  -D__FACT2__ $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis

fact3: main.c arm_callout.S fact3.o
	$(CC) $(CFLAGS)  -D__FACT3__ $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis
objs: $(OBJS)

clean:
	rm -f fib fact fact? *.o *.dis 
