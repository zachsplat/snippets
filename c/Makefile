CC = gcc
CFLAGS = -Wall -O2 -std=c11

test_ringbuf: ringbuf.c
	$(CC) $(CFLAGS) -DTEST_RINGBUF -o $@ $<

test_hashmap: hashmap.c
	$(CC) $(CFLAGS) -DTEST_HASHMAP -o $@ $<

clean:
	rm -f test_ringbuf test_hashmap

.PHONY: clean
