ifeq ($(shell uname), Darwin)
	APPLE_CCFLAGS = -m64
	APPLE_ASFLAGS = -arch x86_64
endif
all: gtmain gttest gttest2

CFLAGS = $(APPLE_CCFLAGS) -g -Wall

gtmain: main.o gtswtch.o
	$(CC) $(APPLE_CCFLAGS) -o $@ $^

gttest: test1.o gtswtch.o
	$(CC) $(APPLE_CCFLAGS) -o $@ $^

gttest2: test2.o gtswtch.o
	$(CC) $(APPLE_CCFLAGS) -o $@ $^
.S.o:
	as $(APPLE_ASFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o gttest gttest2 gtmain
