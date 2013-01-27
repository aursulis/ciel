CFLAGS = -Wall -g -std=c99 -D_GNU_SOURCE

shmd: options.o main.o
	$(CC) $(CFLAGS) -o$@ $^ -lrt

.PHONY: clean
clean:
	rm -f *.o shmd
