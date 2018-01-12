#Simple makefile for libusb application

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB460x_FlexConnect.o

USB460x_FlexConnect: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB460x_FlexConnect

USB460x_FlexConnect.o: USB460x_FlexConnect.c
	$(CC) -c USB460x_FlexConnect.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB460x_FlexConnect
