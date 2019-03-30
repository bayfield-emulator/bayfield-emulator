OBJS = cpu.o mem.o insn.o timer.o insn_extended.o
CFLAGS += -DDEBUG -Wall -g

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libemucore.a: $(OBJS)
	ar -rc $@ $^
	ranlib -s $@

cputest: cputest.o libemucore.a
	$(CC) -o $@ ${CFLAGS} ${LDFLAGS} $^

.phony clean:
	rm -f libemucore.a
	rm -f *.o