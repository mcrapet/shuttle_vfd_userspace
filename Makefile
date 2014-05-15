CC=gcc
CFLAGS=-Wall

OBJS=shuttle_vfd.o handler_list.o
LIBS=-lusb

all: userspace-vfd.c $(OBJS)
	$(CC) $(CFLAGS) $? $(LIBS) -o userspace-vfd

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o

remake: clean all
