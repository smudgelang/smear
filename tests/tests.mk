VPATH += tests
tests: test-queue

test-queue: test-queue.c libsmear.a
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^
