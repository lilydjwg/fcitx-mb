CC=g++
CPPFLAGS=-g
objects=main.o mb.o Iconv/Iconv.o

mb: $(objects)
	$(CC) $(objects) -o mb

clean:
	-rm *.o mb
