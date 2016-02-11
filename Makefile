CC=clang

CFLAGS=-Wall

TEST=-Deeval_test=true
NOTEST=-Deeval_test=false

# default build
all: main.c eeval.c eeval_test.c eeval.h
	$(CC) $(CFLAGS) $(NOTEST) main.c eeval.c eeval_test.c -o eeval

# build with the test unit
with_test:
	$(CC) $(CFLAGS) $(TEST) main.c eeval.c eeval_test.c -o eeval

# build, run the test unit then delete the executable
test:
	$(CC) $(CFLAGS) $(TEST) main.c eeval.c eeval_test.c -o eeval
	./eeval -t
	rm -f eeval

# install eeval into /usr/local/bin
install:
	$(CC) $(CFLAGS) $(NOTEST) main.c eeval.c eeval_test.c -o eeval
	mv -i eeval /usr/local/bin/eeval

# removes eeval from /usr/local/bin
clean:
	rm -f /usr/local/bin/eeval

.PHONY: all with_test test install clean
