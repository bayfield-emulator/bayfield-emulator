OBJS = cpu.o mem.o insn.o timer.o insn_extended.o mbc.o
CFLAGS += -DDEBUG -DSIMULATE_PIPELINE=0 -Wall -g -O3

all: cputest libemucore.a

.phony: clean dotests

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libemucore.a: $(OBJS)
	ar -rc $@ $^
	ranlib -s $@

cputest: cputest.o libemucore.a
	$(CC) -o $@ ${CFLAGS} ${LDFLAGS} $^

dotests: cputest
	@./test_driver.sh roms_for_test | tee last_test.log

clean:
	rm -f libemucore.a
	rm -f *.o
	rm -f emumem.dmp