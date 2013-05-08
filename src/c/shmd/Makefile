ARCH_LINUX = -DKERN_LINUX
ARCH_SCC = -DKERN_SCC
SHMFS_POSIX = -DBUILD_POSIX_SHMFS
SHMFS_CUSTOM = -DBUILD_CUSTOM_SHMFS

MODE = $(ARCH_LINUX) $(SHMFS_CUSTOM)

SHMFS_OBJECTS = shm_fs.o shm_fs_helpers.o shm_fs_fifos.o
ARCH_OBJECTS =
ifneq (,$(findstring $(ARCH_LINUX), $(MODE)))
	ARCH_OBJECTS += interdaemon_linux.o
endif

ifneq (,$(findstring $(ARCH_SCC), $(MODE)))
	#ARCH_OBJECTS += interdaemon_scc.o
endif

ifneq (,$(findstring $(SHMFS_POSIX), $(MODE)))
	ARCH_OBJECTS += shm_worker_posix.o
endif

ifneq (,$(findstring $(SHMFS_CUSTOM), $(MODE)))
	ARCH_OBJECTS += $(SHMFS_OBJECTS) shm_worker_custom.o
endif

CFLAGS = -Wall -std=c99 -pthread -D_GNU_SOURCE $(MODE)

all: libshmdc.so shmd test

libshmdc.so: ipc_integ.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,$@ -o$@ $^ -lrt

shmd: $(ARCH_OBJECTS) interdaemon.o shm_worker.o options.o ipc_server.o logging.o main.o
	$(CC) $(CFLAGS) -o$@ $^ -lrt

test: libshmdc.so test.o
	$(CC) $(CFLAGS) -o$@ $^ -L./ -lshmdc

.PHONY: install
install:
	/usr/bin/install -m 0755 shmd /usr/local/bin/
	/usr/bin/install -m 0755 libshmdc.so /usr/local/lib/
	/sbin/ldconfig

.PHONY: clean
clean:
	rm -f *.o shmd libshmdc.so test
