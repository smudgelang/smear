VPATH += tests
tests: test-queue

test-queue: test-queue.c obj/queue.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^
