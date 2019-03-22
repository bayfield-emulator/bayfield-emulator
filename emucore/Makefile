OBJS = cpu.o mem.o

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libemucore.a: $(OBJS)
	libtool -static -o $@ $^