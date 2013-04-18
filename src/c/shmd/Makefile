CFLAGS = -Wall -g -std=c99 -D_GNU_SOURCE -pthread

all: libshmdc.so shmd

libshmdc.so: ipc_integ.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,$@ -o$@ $^ -lrt

shmd: shm_loader.o options.o ipc_server.o main.o
	$(CC) $(CFLAGS) -o$@ $^ -lrt

.PHONY: clean
clean:
	rm -f *.o shmd libshmdc.so
