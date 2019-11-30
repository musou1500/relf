CFLAGS = -std=gnu99

relf: main.o
	$(CC) main.o -o relf 

clean:
	rm *.o relf
