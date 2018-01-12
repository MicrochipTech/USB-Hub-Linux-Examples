#Simple makefile for libusb application

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB70xx_FlexConnect.o

USB70xx_FlexConnect: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB70xx_FlexConnect

USB70xx_FlexConnect.o: USB70xx_FlexConnect.c
	$(CC) -c USB70xx_FlexConnect.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB70xx_FlexConnect
