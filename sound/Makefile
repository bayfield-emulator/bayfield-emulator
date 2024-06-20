OBJS = controller.o noise.o envelope.o wave.o square.o
CFLAGS += -O2 -Wall -DDEBUG

ifeq ($(ASAN),1)
	CFLAGS += -fsanitize=address
endif

all: libsound.a

.phony: clean

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libsound.a: $(OBJS)
	ar -rcs $@ $^

sndtest: sndtest.o libsound.a
	$(CC) -o $@ ${CFLAGS} ${LDFLAGS} $^

clean:
	rm -f libsound.a
	rm -f *.o
	rm -f emumem.dmp
