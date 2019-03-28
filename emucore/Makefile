OBJS = cpu.o mem.o insn.o timer.o
CFLAGS += -DDEBUG

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libemucore.a: $(OBJS)
	libtool -static -o $@ $^