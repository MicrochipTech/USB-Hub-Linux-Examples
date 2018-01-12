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
#include "MchpUSBInterface.h"
#include "USBHubAbstraction.h"

int  usb_HCE_read_data(PUSB_CTL_PKT pUsbCtlPkt) 
{
	int rc = 0;
	
	//for tyler Delect memory type
	rc = libusb_control_transfer(pUsbCtlPkt->handle, 0x41,CMD_SET_MEM_TYPE,MEMTYPE_XDATA,0,0,0,CTRL_TIMEOUT);

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

	//for tyler Delect memory type
	rc = libusb_control_transfer(pUsbCtlPkt->handle, 0x41,CMD_SET_MEM_TYPE,MEMTYPE_XDATA,0,0,0,CTRL_TIMEOUT);

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

