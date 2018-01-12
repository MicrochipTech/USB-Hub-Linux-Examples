#Simple makefile for libusb application
# this is a proper pound sign! jolly good!

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB491x_EndpointReflector.o

USB491x_EndpointReflector: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB491x_EndpointReflector

USB491x_EndpointReflector.o: USB491x_EndpointReflector.c
	$(CC) -c USB491x_EndpointReflector.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB491x_EndpointReflector
