CC=g++
CPPFLAGS=-g
objects=main.o mb.o

mb: $(objects)
	$(CC) $(objects) Iconv/Iconv.o -o mb

clean:
	-rm *.o mb
