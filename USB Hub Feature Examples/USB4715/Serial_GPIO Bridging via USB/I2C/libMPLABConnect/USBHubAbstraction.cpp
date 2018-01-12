/*
**********************************************************************************
* $File:  USBHubAbstraction.cpp
* Description : File holds API for used for file handling and data transfer
**********************************************************************************/

#include <stdio.h>
#include <libusb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include "SpiFlash.h"
#include "MchpUSBInterface.h"
#include "USBHubAbstraction.h"

FILE *fp;
uint16_t ReadBinfile(char const *name, uint8_t *buffer)
{
	FILE *file;
	unsigned long fileLen;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s\n", name);
		return 0;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);
	return ((uint16_t)fileLen);
}
uint16_t writeBinfile(char const *name, uint8_t *buffer, unsigned long fileLen)
{
	FILE *file;


	//Open file
	file = fopen(name, "wb+");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		return 0;
	}

	//Read file contents into buffer
	fwrite(buffer, 1, fileLen, file);
	fclose(file);
	return (0);
}
int  usb_HCE_read_data(PUSB_CTL_PKT pUsbCtlPkt)
{
	int rc = 0;



	rc = libusb_control_transfer(	pUsbCtlPkt->handle,
									LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE,
									pUsbCtlPkt->byRequest,
									pUsbCtlPkt->wValue,
									pUsbCtlPkt->wIndex,
									pUsbCtlPkt->byBuffer,
									pUsbCtlPkt->wLength,
									CTRL_TIMEOUT
								);

	return rc;
}
int  usb_HCE_write_data(PUSB_CTL_PKT pUsbCtlPkt)
{
	int rc = 0;

	rc = libusb_control_transfer(	pUsbCtlPkt->handle,
									LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE,
									pUsbCtlPkt->byRequest,
									pUsbCtlPkt->wValue,
									pUsbCtlPkt->wIndex,
									pUsbCtlPkt->byBuffer,
									pUsbCtlPkt->wLength,
									CTRL_TIMEOUT
								);
	return rc;
}
int  usb_HCE_no_data(PUSB_CTL_PKT pUsbCtlPkt)
{
	int rc = 0;

	rc = libusb_control_transfer(	pUsbCtlPkt->handle,
									LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE,
									pUsbCtlPkt->byRequest,
									pUsbCtlPkt->wValue,
									pUsbCtlPkt->wIndex,
									NULL,
									0,
									CTRL_TIMEOUT
								);

	return rc;
}
