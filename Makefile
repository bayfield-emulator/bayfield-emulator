PROGRAMS = cputest

all: $(PROGRAMS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

cputest: cputest.o
	make -C emucore libemucore.a
	$(CC) -o $@ ${CFLAGS} ${LDFLAGS} $^ emucore/libemucore.a

