ARCH_LINUX = -DKERN_LINUX
SHMFS_CUSTOM = -DBUILD_CUSTOM_SHMFS
MODE = $(ARCH_LINUX) $(SHMFS_CUSTOM)

SHMFS_OBJECTS = ../shm_fs.o ../shm_fs_helpers.o
CFLAGS = -O2 -Wall -std=c99 -pthread -D_GNU_SOURCE -DSHMFS_STANDALONE $(MODE)

all: shmls shmln shmld shmst shminit shmnull

shmls: shmls.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

shmln: shmln.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

shmld: shmld.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

shmst: shmst.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

shminit: shminit.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

shmnull: shmnull.c $(SHMFS_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ -I../ -lrt

.PHONY: clean
clean:
	rm -f *.o shmls shmln shmld shmst shminit shmnull
