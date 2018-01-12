/*
**********************************************************************************
©  [2017] Microchip Technology Inc. and its subsidiaries. 

Subject to your compliance with these terms, you may use Microchip software and
any derivatives exclusively with Microchip products. It is your responsibility
to comply with third party license terms applicable to your use of third party
software (including open source software) that may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS,
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE. IN
NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN
ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST
EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU
HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

**********************************************************************************
*  $Revision: #1.1 $  $DateTime: 2015/09/21 04:18:20 $  $    $
*  Description: This version supports SPI,I2C,UART Bridging and Programming Config file
**********************************************************************************
* $File:  MchpUSBINterface.cpp
*/

#include <stdio.h>
#include <libusb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include "MchpUSBInterface.h"
#include "USBHubAbstraction.h"

#define FALSE								0
#define TRUE								1
#define min(a,b)							(((a) < (b)) ? (a) : (b))
#define MICROCHIP_HUB_VID						0x424



#define VID_MICROCHIP							0x0424
#define PID_HCE_DEVICE							0x4940

#define PT2_LIB_VER							"1.01.00"

#define CTRL_RETRIES 							2
#define HUB_STATUS_BYTELEN						3 /* max 3 bytes status = hub + 23 ports */
#define HUB_DESC_NUM_PORTS_OFFSET					2


#define logprint(x, ...) do { \
		printf(__VA_ARGS__); \
		fprintf(x,  __VA_ARGS__); \


/*-----------------------Helper functions --------------------------*/
int usb_enable_HCE_device(uint8_t hub_index);
static int compare_hubs(const void *p1, const void *p2);
static int usb_get_hubs(PHINFO pHubInfoList);
static int usb_open_HCE_device(uint8_t hub_index);
int  usb_send_vsm_command(struct libusb_device_handle *handle, uint8_t * byValue) ;

 // Global variable for tracking the list of hubs
HINFO gasHubInfo [MAX_HUBS];
/* Context Variable used for initializing LibUSB session */
libusb_context *ctx = NULL;
WORD gwPID = 0x4916;;
/*-----------------------API functions --------------------------*/
UINT32 MchpUsbGetLastErr (HANDLE DevID)
{
	return errno;
}

HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID)
{
	int error = 0, hub_cnt=0, hub_index=0;
	int restart_count=5;
	bool bhub_found = false;
	gwPID = wPID;
	hub_cnt = usb_get_hubs(&gasHubInfo[0]);

	do
	{
		if((gasHubInfo[hub_index].wVID == wVID) && (gasHubInfo[hub_index].wPID == wPID))
		{
			bhub_found = true;
		}

	}
	while(hub_index++ < hub_cnt);

	if(false == bhub_found)
	{
		DEBUGPRINT("MCHP_Error_Device_Not_Found \n");
		return INVALID_HANDLE_VALUE;
	}

	error = usb_open_HCE_device(hub_index);
	if(error < 0)
	{

		//enable 5th Endpoit
		error = usb_enable_HCE_device(hub_index);

		if(error < 0)
		{
			DEBUGPRINT("MCHP_Error_Invalid_Device_Handle: Failed to Enable the device \n");
			return INVALID_HANDLE_VALUE;
		}
		do
		{
			sleep(2);
			error = usb_open_HCE_device(hub_index);
			if(error == 0)
			{
				return hub_index;
			}

		}while(restart_count--);

		DEBUGPRINT("MCHP_Error_Invalid_Device_Handle: Failed to open the device error:%d\n",error);
		return INVALID_HANDLE_VALUE;
	}


	return hub_index;
}


BOOL MchpUsbClose(HANDLE DevID)
{
	if(gasHubInfo[DevID].handle != NULL)
	{
		libusb_close((libusb_device_handle*)gasHubInfo[DevID].handle);
		gasHubInfo[DevID].dev = NULL;
		gasHubInfo[DevID].handle = NULL;
	}
	else
	{
		printf("unknown hub index%d\n", DevID);
		return false;
	}
	libusb_exit(ctx);
	return true;
}

BOOL  MchpUsbRegisterRead ( HANDLE DevID, UINT32 RegisterAddress, UINT16 BytesToRead, UINT8* InputData )
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	/***************************USB57X4*******************************/
	bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle, 0x41,CMD_SET_MEM_TYPE,MEMTYPE_XDATA,0,0,0,CTRL_TIMEOUT);
	/*****************************************************************/

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
	UsbCtlPkt.byRequest = 0x04;
	UsbCtlPkt.wValue 	= (RegisterAddress & 0xFFFF);
	UsbCtlPkt.wIndex 	= ((RegisterAddress & 0xFFFF0000) >> 16);
	UsbCtlPkt.byBuffer 	= InputData;
	UsbCtlPkt.wLength 	= BytesToRead;

	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	if(bRetVal < 0)
	{
		return false;
	}
	return true;
}

BOOL  MchpUsbRegisterWrite( HANDLE DevID, UINT32 RegisterAddress,UINT16 BytesToWrite, UINT8* OutputData)
{
	int bRetVal = FALSE;
	if(nullptr == OutputData)
	{
		DEBUGPRINT("Register Write Failed: NULL Pointer\n");
		return false;
	}
	USB_CTL_PKT UsbCtlPkt;
	/***************************USB57X4*******************************/
	bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle, 0x41,CMD_SET_MEM_TYPE,MEMTYPE_XDATA,0,0,0,CTRL_TIMEOUT);
	/*****************************************************************/

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
	UsbCtlPkt.byRequest = 0x03;
	UsbCtlPkt.wValue 	= (RegisterAddress & 0xFFFF);
	UsbCtlPkt.wIndex 	= ((RegisterAddress & 0xFFFF0000) >> 16);
	UsbCtlPkt.byBuffer 	= OutputData;
	UsbCtlPkt.wLength 	= BytesToWrite;

	bRetVal = usb_HCE_write_data (&UsbCtlPkt);
	if(bRetVal < 0)
	{
		return false;
	}
	return true;
}

/*----------------------- Helper functions -----------------------------------*/
static int usb_get_hubs(PHINFO pHubInfoList)
{
	int cnt = 0, hubcnt = 0, i = 0, error=0;
	libusb_device **devs;
	libusb_device_descriptor desc;
	libusb_device_handle *handle;
	PHINFO pHubListHead = pHubInfoList;	// Pointer to head of the list

	error = libusb_init(&ctx);
	if(error < 0)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Initialization LibUSB failed\n");
		return -1;
	}

	cnt = (int)(libusb_get_device_list(ctx, &devs));
	if(cnt < 0)
	{
		DEBUGPRINT("Failed to get the device list \n");
		return -1;
	}


	for (i = 0; i < cnt; i++)
	{
		int error = 0;
		int value = 0;

		libusb_device *device = devs[i];

		error = libusb_get_device_descriptor(device, &desc);
		if(error != 0)
		{
			DEBUGPRINT("LIBUSB_ERROR: Failed to retrieve device descriptor for device[%d] \n", i);
		}


		if((error ==  0) && (desc.bDeviceClass == LIBUSB_CLASS_HUB))
		{
			uint8_t hub_desc[ 7 /* base descriptor */
							+ 2 /* bitmasks */ * HUB_STATUS_BYTELEN];


		  	error = libusb_open(device, &handle);
			if(error < 0)
			{
				DEBUGPRINT("Cannot open device[%d] \t", i);
				switch(error)
				{
					case LIBUSB_ERROR_NO_MEM:
						DEBUGPRINT("LIBUSB_ERROR_NO_MEM \n");
					break;
					case LIBUSB_ERROR_ACCESS:
						DEBUGPRINT("LIBUSB_ERROR_ACCESS \n");
					break;
					case LIBUSB_ERROR_NO_DEVICE:
						DEBUGPRINT("LIBUSB_ERROR_NO_DEVICE \n");
					break;
					default:
						DEBUGPRINT("UNKNOWN_LIBUSB_ERROR \n");
					break;
				}
				continue;
			}

		 	memset(hub_desc, 0, 9);

			if(desc.bcdUSB == 0x0300)
			{
				value = 0x2A;
			}
		 	else
			{
		  		value = 0x29;
			}

			error = libusb_control_transfer(handle,
											LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE,
											LIBUSB_REQUEST_GET_DESCRIPTOR,
											value << 8, 0, hub_desc, sizeof hub_desc, CTRL_TIMEOUT
											);

		  	if(error < 0)
			{
				DEBUGPRINT("libusb_control_transfer device[%d]: \t", i);
				switch(error)
				{
					case LIBUSB_ERROR_TIMEOUT:
						DEBUGPRINT("LIBUSB_ERROR_TIMEOUT \n");
					break;
					case LIBUSB_ERROR_PIPE:
						DEBUGPRINT("LIBUSB_ERROR_PIPE \n");
					break;
					case LIBUSB_ERROR_NO_DEVICE:
						DEBUGPRINT("LIBUSB_ERROR_NO_DEVICE \n");
					break;
					default:
						DEBUGPRINT("UNKNOWN_LIBUSB_ERROR \n");
					break;
				}
				continue;
			}

			pHubInfoList->port_max = libusb_get_port_numbers(device, pHubInfoList->port_list, 7);

		  	if(pHubInfoList->port_max <= 0)
			{
				continue;
			}

			pHubInfoList->byPorts	= hub_desc[3];
			libusb_close(handle);

			pHubInfoList->wVID	 	= desc.idVendor;
			pHubInfoList->wPID 		= desc.idProduct;

			pHubInfoList++;
			hubcnt++;
		}
	}

	libusb_free_device_list(devs, 1);

	qsort(pHubListHead, hubcnt, sizeof(HINFO), compare_hubs);

	return hubcnt;
}

static int compare_hubs(const void *p1, const void *p2)
{
	PHINFO pHub1, pHub2;

	pHub1 = (PHINFO) p1;
	pHub2 = (PHINFO) p2;


	if((VID_MICROCHIP == pHub1->wVID) && (VID_MICROCHIP == pHub2->wVID))
	{
		return 0; 	//Both Microchip hubs
	}
	else if (VID_MICROCHIP == pHub1->wVID)
	{
		return -1;	//Hub 1 is MCHP
	}
	else if (VID_MICROCHIP == pHub2->wVID)
	{
		return 1;  //Hub 2 is MCHP
	}

	return 0;
}

static int usb_open_HCE_device(uint8_t hub_index)
{
	libusb_device_handle *handle= NULL;
	libusb_device **devices;
	libusb_device *dev;
	libusb_device_descriptor desc;

	int dRetval = 0;
	ssize_t devCnt = 0, port_cnt = 0;
	ssize_t i = 0;
	uint8_t port_list[7];

	devCnt = libusb_get_device_list(ctx, &devices);
	if(devCnt < 0)
	{
		DEBUGPRINT("Enumeration failed \n");
		return -1;
	}


	for (i = 0; i < devCnt; i++)
	{
		dev = devices[i];

		dRetval = libusb_get_device_descriptor(dev, &desc);
		if(dRetval < 0)
		{
			DEBUGPRINT("Cannot get the device descriptor \n");
			libusb_free_device_list(devices, 1);
			return -1;
		}

		if(PID_HCE_DEVICE == desc.idProduct)
		{
			dRetval = libusb_open(dev, &handle);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device open failed \n");
				return -1;
			}

			port_cnt = libusb_get_port_numbers(dev, port_list, 7);
			if(port_cnt <= 1)
			{
				DEBUGPRINT("Retrieving port numbers failed \n");
				break;
			}

			//Match with the hub port list
			for(i = 0; i < gasHubInfo[hub_index].port_max; i++)
			{
				if(gasHubInfo[hub_index].port_list[i] != port_list[i])
				{
					DEBUGPRINT("Hub port match failed \n");
					dRetval = -1;
					break;
				}
			}

			if(dRetval == -1)
			{
				break;
			}

			printf("Enabled VID:PID = %04x:%04x ", desc.idVendor, desc.idProduct);
			for(i = 0; i < port_cnt; i++)
			{
				printf(":%d", (unsigned int)(port_list[i]));
			}
			printf("\n");


			if(libusb_kernel_driver_active(handle, 0) == 1)
			{
				//DEBUGPRINT("Kernel has attached a driver, detaching it \n");
				if(libusb_detach_kernel_driver(handle, 0) != 0)
				{
					DEBUGPRINT("Cannot detach kerenl driver. USB device may not respond \n");
					break;
				}
			}

			dRetval = libusb_claim_interface(handle, 0);

			if(dRetval < 0)
			{
				DEBUGPRINT("cannot claim intterface \n");
				dRetval = -1;
				break;
			}

			gasHubInfo[hub_index].dev = devices[i];
			gasHubInfo[hub_index].handle = handle;
			gasHubInfo[hub_index].byHubIndex = hub_index;

			return dRetval;

		}
	}
    if(handle)
    {
        libusb_close(handle);

    }



	libusb_free_device_list(devices, 1);
	return -1;
}

int usb_enable_HCE_device(uint8_t hub_index)
{
	libusb_device_handle *handle;
	libusb_device **devices;
	libusb_device *dev;
	libusb_device_descriptor desc;

	int dRetval = 0;
	ssize_t devCnt = 0;
	ssize_t i = 0;

	devCnt = libusb_get_device_list(ctx, &devices);
	if(devCnt <= 0)
	{
		DEBUGPRINT("Enumeration failed \n");
		return -1;
	}

	for (i = 0; i < devCnt; i++)
	{
		dev = devices[i];

		dRetval = libusb_get_device_descriptor(dev, &desc);
		if(dRetval < 0)
		{
			DEBUGPRINT("Cannot get the device descriptor \n");
			libusb_free_device_list(devices, 1);
			return -1;
		}

		if(MICROCHIP_HUB_VID == desc.idVendor)
		{
			dRetval = libusb_open(dev, &handle);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device open failed \n");
				return -1;
			}

			if(libusb_kernel_driver_active(handle, 0) == 1)
			{

				if(libusb_detach_kernel_driver(handle, 0) != 0)
				{
					DEBUGPRINT("Cannot detach kerenl driver. USB device may not respond \n");
					libusb_close(handle);
					break;
				}
			}

			/*dRetval = libusb_claim_interface(handle, 0);

			if(dRetval < 0)
			{
				DEBUGPRINT("cannot claim intterface \n");
				libusb_close(handle);
				break;
			}*/

			uint16_t val = 0x0001;
			dRetval = usb_send_vsm_command(handle,(uint8_t*)&val);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device: VSM command 0x0001 failed \n");
				libusb_close(handle);
				break;
			}

			val = 0x0201;
			dRetval = usb_send_vsm_command(handle,(uint8_t*)&val);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device: VSM command 0x0201 failed \n");
			}
			sleep(2);

			libusb_close(handle);
			libusb_free_device_list(devices, 1);
			return dRetval;
		}
	}



	libusb_free_device_list(devices, 1);
	return -1;
}

int  usb_send_vsm_command(struct libusb_device_handle *handle, uint8_t * byValue)
{
	int rc = 0;

	rc = libusb_control_transfer(	handle,
					0x40,
					0x02,
					0,
					0,
					byValue,
					2,
					CTRL_TIMEOUT
				);
	return rc;

}
