OBJS = cpu.o mem.o insn.o timer.o insn_extended.o mbc.o
CFLAGS += -O2 -Wall -DSIMULATE_PIPELINE=0 #-DDEBUG

ifeq ($(ASAN),1)
	CFLAGS += -fsanitize=address
endif

all: cputest libemucore.a

.phony: clean dotests

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libemucore.a: $(OBJS)
	ar -rcs $@ $^

cputest: cputest.o libemucore.a
	$(CC) -o $@ ${CFLAGS} ${LDFLAGS} $^

dotests: cputest
	@./test_driver.sh roms_for_test | tee last_test.log

clean:
	rm -f libemucore.a
	rm -f *.o
	rm -f emumem.dmp
