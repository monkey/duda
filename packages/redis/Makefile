CC      = gcc
CFLAGS  =  -O2 -std=gnu99 -Wall -Wextra -fvisibility=hidden
LDFLAGS = 
DEFS    = 
INCDIR  = ../../../../src/include -I../../src
OBJECTS = duda_package.o redis.o async.o dict.o hiredis.o net.o sds.o
SOURCES = duda_package.c redis.c async.c dict.c hiredis.c net.c sds.c

all: ../redis.dpkg

../redis.dpkg: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -shared -Wl -o $@ $^ -lc

.c.o: $(SOURCES)
	$(CC) $(CFLAGS) $(DEFS) -I$(INCDIR) -fPIC -c $<

clean:
	rm -rf *~ *.o
