#Simple makefile for libusb application

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB4715_FlexConnect.o

USB4715_FlexConnect: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB4715_FlexConnect

USB4715_FlexConnect.o: USB4715_FlexConnect.c
	$(CC) -c USB4715_FlexConnect.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB4715_FlexConnect
