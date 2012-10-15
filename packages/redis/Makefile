_PATH   = $(patsubst $(monkey_root)/%, %, $(CURDIR))
CC      = @echo "  CC   $(_PATH)/$@"; gcc
CC_QUIET= @echo -n; gcc
CFLAGS  =  -O2 -std=gnu99 -Wall -Wextra -fvisibility=hidden
LDFLAGS = 
DEFS    = 
INCDIR  = ../../../../src/include -I../../src
OBJECTS = duda_package.o redis.o async.o dict.o hiredis.o net.o sds.o
SOURCES = duda_package.c redis.c async.c dict.c hiredis.c net.c sds.c

all: ../redis.dpkg

-include $(OBJECTS:.o=.d)

../redis.dpkg: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -shared -Wl -o $@ $^ -lc

.c.o: $(SOURCES)
	$(CC) $(CFLAGS) $(DEFS) -I$(INCDIR) -fPIC -c $<
	$(CC_QUIET) -MM -MP $(CFLAGS) $(DEFS) -I$(INCDIR) $*.c -o $*.d > /dev/null &2>&1

clean:
	rm -rf *~ *.o
