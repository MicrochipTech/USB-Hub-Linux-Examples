/*******************************************************************************
  Protouch2 SDK header file

  Company:
    Microchip Technology Inc.

  File Name:
    USBHubAbstraction.h

  Summary:
    Protouch2 SDK Header File for file handling and data transfer APIs

  Description:
    Protouch2 SDK LIB Header file for file handling and data transfer APIs
*******************************************************************************/
#pragma once

#include "typedef.h"
#include "USBHubAbstraction.h"



/*usb_HCE_read_data
 * API does libusb control transfer for Read operation*/
int  usb_HCE_read_data(PUSB_CTL_PKT pUsbCtlPkt);

/*usb_HCE_write_data
 * API does libusb control transfer for write operation*/
int  usb_HCE_write_data(PUSB_CTL_PKT pUsbCtlPkt);


