ARCH_LINUX = -DKERN_LINUX
ARCH_SCC = -DKERN_SCC
SHMFS_POSIX = -DBUILD_POSIX_SHMFS
SHMFS_CUSTOM = -DBUILD_CUSTOM_SHMFS

MODE = $(ARCH_SCC) $(SHMFS_CUSTOM)

SHMFS_OBJECTS = shm_fs.o shm_fs_helpers.o shm_fs_fifos.o
ARCH_OBJECTS =
EXTRA_LIBS =
ifneq (,$(findstring $(ARCH_LINUX), $(MODE)))
	ARCH_OBJECTS += interdaemon_linux.o
endif

ifneq (,$(findstring $(ARCH_SCC), $(MODE)))
	ARCH_OBJECTS += interdaemon_scc.o
	EXTRA_LIBS += -I/shared/au231/rcce/include/ -I/shared/au231/iRCCE/include/ /shared/au231/rcce/bin/SCC_LINUX/libRCCE_bigflags_nongory_nopwrmgmt.a
	CC = icc -mcpu=pentium -gcc-version=340 -static
endif

ifneq (,$(findstring $(SHMFS_POSIX), $(MODE)))
	ARCH_OBJECTS += shm_worker_posix.o
endif

ifneq (,$(findstring $(SHMFS_CUSTOM), $(MODE)))
	ARCH_OBJECTS += $(SHMFS_OBJECTS) shm_worker_custom.o
endif

CFLAGS = -std=c99 -pthread -D_GNU_SOURCE $(MODE)

all: libshmdc.so shmd

%.o: %.c
	$(CC) $(CFLAGS) -c -o$@ $^ -lrt $(EXTRA_LIBS)

libshmdc.so: ipc_integ.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,$@ -o$@ $^ -lrt

shmd: $(ARCH_OBJECTS) interdaemon.o shm_worker.o options.o ipc_server.o logging.o main.o
	$(CC) $(CFLAGS) -o$@ $^ -lrt $(EXTRA_LIBS)

.PHONY: install
install:
	/usr/bin/install -m 0755 shmd /usr/local/bin/
	/usr/bin/install -m 0755 libshmdc.so /usr/local/lib/
	/sbin/ldconfig

.PHONY: clean
clean:
	rm -f *.o shmd libshmdc.so
