VPATH += tests
tests: test-cancelq

test-queue: test-queue.c obj/queue.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

test-cancelq: test-cancelq.c obj/cancellable.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)
