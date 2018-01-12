#Simple makefile for libusb application

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB70xx_EndpointReflector.o

USB70xx_EndpointReflector: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB70xx_EndpointReflector

USB70xx_EndpointReflector.o: USB70xx_EndpointReflector.c
	$(CC) -c USB70xx_EndpointReflector.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB70xx_EndpointReflector
