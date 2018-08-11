VPATH += tests
tests: test-cancelq-fill-then-cancel test-cancelq-fill-then-drain-all \
	  test-cancelq-cancel-some-drain-some test-cancelq-not-cancellable \
	  test-cancelq-threads

test-cancelq-%: test-cancelq-%.c obj/cancellable.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

test-queue: test-queue.c obj/queue.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

