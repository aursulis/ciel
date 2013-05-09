ARCH_LINUX = -DKERN_LINUX
SHMFS_CUSTOM = -DBUILD_CUSTOM_SHMFS
MODE = $(ARCH_LINUX) $(SHMFS_CUSTOM)

SHMFS_OBJECTS = ../shm_fs.o ../shm_fs_helpers.o
CFLAGS = -Wall -std=c99 -pthread -D_GNU_SOURCE $(MODE)

shmls: shmls.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

.PHONY: clean
clean:
	rm -f *.o shmd libshmdc.so test
