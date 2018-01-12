#Simple makefile for libusb application

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB492x_DUST.o

USB492x_DUST: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB492x_DUST

USB492x_DUST.o: USB492x_DUST.c
	$(CC) -c USB492x_DUST.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB492x_DUST
