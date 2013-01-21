%.o : %.c
	$(CC) -fPIC $^ -c

libcielshm.so : file_xfer.o file_xfer_client.o file_xfer_server.o shm_object.o usecases.o
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ -lrt

.PHONY: clean
clean:
	rm -f *.o *.so
