CFLAGS=$(shell pkg-config --libs libusb-1.0)

main: main.o $(LIB_OBJ)
	$(CC) main.o $(LIB_OBJ) -lcamlib $(CFLAGS) -o main

clean:
	$(RM) *.o $(LIB_OBJ) main
