CC = arm-linux-gnueabihf-gcc

default: libmax7219mat.a

libmax7219mat.a: libmax7219mat.o
	ar rcs $@ $^

max7219mat.o: libmax7219mat.c libmax7219mat.h
	$(CC) -c $<

clean:
	rm *.o *.a
