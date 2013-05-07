CFLAGS = -Wall -g -std=c99 -D_GNU_SOURCE -DBUILD_POSIX_SHMFS -pthread

all: libshmdc.so shmd test

libshmdc.so: ipc_integ.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,$@ -o$@ $^ -lrt

shmd: shm_worker.o shm_worker_linux.o options.o ipc_server.o interdaemon_linux.o logging.o main.o
	$(CC) $(CFLAGS) -o$@ $^ -lrt

test: libshmdc.so test.o
	$(CC) $(CFLAGS) -o$@ $^ -L./ -lshmdc

.PHONY: clean
clean:
	rm -f *.o shmd libshmdc.so test
