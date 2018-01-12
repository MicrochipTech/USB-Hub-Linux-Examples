#Simple makefile for libusb application
# this is a proper pound sign! jolly good!

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB_Linux_HSET.o

USB_Linux_HSET: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB_Linux_HSET

USB_Linux_HSET.o: USB_Linux_HSET.c
	$(CC) -c USB_Linux_HSET.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB_Linux_HSET
