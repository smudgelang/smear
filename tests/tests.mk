VPATH += tests
TESTS = test-cancelq-fill-then-cancel test-cancelq-fill-then-drain-all \
        test-cancelq-cancel-some-drain-some test-cancelq-not-cancellable \
        test-cancelq-threads


test-cancelq-%: test-cancelq-%.c obj/cancellable.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

test-queue: test-queue.c obj/queue.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

.PHONY: runtests

%.log: %
	rm -f $@
	./$< 2>&1 | tee -a $@
	valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./$< 2>&1 | tee -a $@
	valgrind --tool=helgrind --error-exitcode=1 ./$< 2>&1| tee -a $@

runtests: $(foreach t, $(TESTS), $(t).log)
