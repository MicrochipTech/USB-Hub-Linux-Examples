#Simple makefile for libusb application

CC=gcc
INC=/usr/local/include/libusb-1.0/
OBJ=USB49xx_USB47xx_DynamicBatteryCharging.o

USB49xx_USB47xx_DynamicBatteryCharging: $(OBJ)
	$(CC) $(OBJ) -lusb-1.0 -o USB49xx_USB47xx_DynamicBatteryCharging

USB49xx_USB47xx_DynamicBatteryCharging.o: USB49xx_USB47xx_DynamicBatteryCharging.c
	$(CC) -c USB49xx_USB47xx_DynamicBatteryCharging.c -I$(INC)

clean :
	-rm *.o $(objects) *.exe USB49xx_USB47xx_DynamicBatteryCharging
