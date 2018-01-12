/*******************************************************************************
  Protouch2 SDK header file

  Company:
    Microchip Technology Inc.

  File Name:
    MchpUSBInterface.h

  Summary:
    Protouch2 SDK Header File

  Description:
    Protouch2 SDK LIB Header file
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
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
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section lists the other files that are included in this file.
*/
#pragma once

#include "typedef.h"

// *****************************************************************************
// *****************************************************************************
// Section: DLL Exports
// *****************************************************************************
// *****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif


#define MY_MAX_PATH   						1024*8
#define MAX_HUBS 						20
#define CTRL_TIMEOUT 						(5*1000) /* milliseconds */

#define CMD_SET_MEM_TYPE				0x0A
#define MEMTYPE_XDATA					0x00
#define min(a,b)					(((a) < (b)) ? (a) : (b))

#define DEBUG
#ifdef DEBUG
#define DEBUGPRINT(...) printf("DEBUGINFO: " __VA_ARGS__)
#else
#define DEBUGPRINT(...)
#endif

#pragma pack(1)
typedef struct tagControlPacket
{
	libusb_device_handle*	handle;
	uint8_t				byRequest;
	uint16_t				wValue;
	uint16_t				wIndex;
	uint8_t*				byBuffer;
	uint16_t				wLength;
} USB_CTL_PKT, *PUSB_CTL_PKT;

typedef enum tag_family
{
	HUB_FAMILY_INDEX_USB2530 = 0,
	HUB_FAMILY_INDEX_USB274X = 1,
	HUB_FAMILY_INDEX_EC2 = 2,
	HUB_FAMILY_INDEX_UNKNOWN = 3
} hub_family_index;

typedef	struct _ASIC
{
	WORD	wInternalFWRevision;
	BYTE	byDeviceRevision;
	WORD	wASICType;
	BYTE	byFeaturesFlag;

} ASICINFO;

typedef enum {SMBus, USB} IntfMode;

typedef enum
{
    USB_DEVICE_INSTANCE_NOT_INITIALIZED = 0,
    USB_DEVICE_INSTANCE_INITIALIZED,
    USB_DEVICE_INSTANCE_OPENED,
    USB_DEVICE_INSTANCE_CLOSED

}USB_DEVICE_INSTANCE_STATE;

typedef struct tagHubInfo
{
	WORD	wVID;			/*!< USB Vendor ID of the hub*/
	WORD	wPID;			/*!< USB Product ID of the hub*/
	WORD	wDID;			/*!< USB device ID of the hub*/
	BYTE	byPorts;		/*!< Number of downstream ports in the hub*/
	WORD wSKUNAME;		/*!< Get SKU name to find number of downstream port*/
	IntfMode Interface;

	BYTE	port_list[7];		/*!< Port number list from root*/
	BYTE	port_max;		/*!< Number of items in the path*/
	void*	hThisHub;	/*!< Device Handle to this hub on Windows*/
	void*	hWinUsbHandle;//usb device handle- hubhandle->usbdevice handle->winusb handle
	void*	hParentHub;	/*!< Device Handle to the parent hub on Windows*/
	int iLocation;
	char deviceName[MY_MAX_PATH];

	ASICINFO sHubInfo;

	////libusb
	void* handle;
	void* filehandle;/*Winusb Dev path handle */
	void **devs;
	void *dev;
	BYTE byHubIndex;
	bool gbUpdateSPI_OTP;
	BYTE gbyRunningfromSPI;
	hub_family_index enHubFamily;/* Family of the Hub, find from sku*/
	USB_DEVICE_INSTANCE_STATE Usb_Device_State;
	DWORD dwError; /* Error occured in last operation*/

} HINFO, DEVICE_INFO, *PHINFO, *PDEVICE_INFO;


// Global variable for tracking the list of hubs
extern HINFO gasHubInfo [MAX_HUBS];

// *****************************************************************************
#define INVALID_HANDLE_VALUE 0xFF
#define Error_Success        	0x0000
#define ReadBackFailed		0x1001



/**************************************************************************************************
  Function:
            UINT32 MchpUsbGetLastErr (HANDLE DevID);

  Summary:
    Get last error for the specific hub instance.
  Description:
    This API will get last error occurred when handling other API's in this
    library.
  Conditions:
    None.
  Input:
    DevID -  Handle to the device \- Return value of
             MchpUsbOpenID.
  Return:
    Linux Error codes.
  Example:
    <code>

	dwError = MchpUsbGetLastErr(hDevice);

    //Print error here
    cout \<\< dwError \<\< endl;
    </code>
  Remarks:
    None
  **************************************************************************************************/

 UINT32 MchpUsbGetLastErr (HANDLE DevID);

/*********************************************************************************************************
  Function:
    HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID);

 Summary:
    Open the device handle.

  Description:
    This API will return handle to the first instance of the HUB VID & PID matched device.

  Conditions:
    - None.

  Input:
    wVID -    Vendor ID(VID) of the Hub.
	wPID -    Product ID(PID) of the Hub.
  Return:
    HANDLE of the Vendor ID and Product ID matched hub - for success

    INVALID_HANDLE_VALUE (Call GetMchpUsbLastErr for more details) - for
    failure

  Example:
    <code>


    CHAR sztext[2048];

    HANDLE hDevice = INVALID_HANDLE_VALUE;

    UINT32 dwError;


    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");
    </code>
  Remarks:
    None
  *********************************************************************************************************/

 HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID);

/******************************************************************************************************
  Function:
            BOOL MchpUsbClose(HANDLE DevID);

  Summary:
    Close the device handle.
  Description:
    This API will close the handle for device specified in the call.
  Input:
    DevID -  Handle to the device \- Return value of
             MchpUsbOpenID.
  Conditions:
    MchpUsbOpenID should be called before calling this API
  Return:
    TRUE - for Success;

    FALSE - for Failure
  Example:
    <code>

    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfully\n");

    if (FALSE == MchpUsbClose(hDevice))
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04x",dwError);
        exit (1);
    }
    </code>
  Remarks:
    None
  ******************************************************************************************************/

 BOOL MchpUsbClose(HANDLE DevID);

/**************************************************************************************************************************
  Function:
            BOOL  MchpUsbRegisterRead ( HANDLE DevID, UINT32 RegisterAddress,UINT16 BytesToRead, UINT8* InputData );

  Summary:
    Read the XDATA register(s) in the XDATA space of of  internal  registers.
  Description:
    This API for Read the XDATA register(s) in the XDATA space of  internal  registers.
 This  API  also  does  the  following:  1)
Checks for the correct device handle before reading the registers 2) Checks for the proper buffer length
  Conditions:
    MchpUsbOpenID should be called before calling this API


  Input:
    DevID -            Handle to the device. Before calling this API ,the
                       caller must acquire the device handle by calling
                       appropriate API.<p />
    RegisterAddress -  Start Address(in the XDATA space) from where Read
                       operation starts<p />
    BytesToRead -      Number of bytes to be read<p />
    InputData -        Pointer to the buffer where data from XDATA registers
                       will be stored.Caller must allocate memory for the
                       buffer to accommodate the number of bytes to be read.
  Return:
    TRUE - for Success; FALSE - (Call GetMchpUsbLastErr for more details) -
    for failure
  Example:
    <code>
    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfully");

    UINT8 byData = 0x55;
    UINT32 dwAddr = 0xBF803004;
    printf("Xdata Write operation, Write 0x%02x in 0x%04xn",byData,dwAddr);
    if (FALSE ==MchpUsbRegisterWrite(hDevice,wAddr,1,&amp;byData))
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Failed to open the device Error,%04x",dwError);
        exit (1);
    }
    cout \<\< "Success :Xdata Write operation";
    byData = 0x00;

    cout \<\< "Xdata Read operation";
    if (FALSE ==MchpUsbRegisterRead(hDevice,dwAddr,1,byData))
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    cout \<\< "Success : Xdata Read operation";
    printf("Xdata Read value is %02x from 0x%08x n",byData,dwAddr);

    </code>
  Remarks:
    None
  **************************************************************************************************************************/
 BOOL  MchpUsbRegisterRead ( HANDLE DevID, UINT32 RegisterAddress, UINT16 BytesToRead, UINT8* InputData );

/**************************************************************************************************************************
  Function:
          BOOL  MchpUsbRegisterWrite( HANDLE DevID, UINT32 RegisterAddress,UINT16 BytesToWrite, UINT8* OutputData);


  Summary:
    Write to the XDATA register (s) in the XDATA space of the internal registers.
  Description:
    This API for Write to the XDATA register(s) in the internal registers.
    This  API  also  does  the  following:  1)
Checks for the correct device handle before reading the registers 2) Checks for the proper buffer length
  Conditions:

    MchpUsbOpenID should be called before calling this API

  Input:
    DevID -            Handle to the device. Before calling this API ,the
                       caller must acquire the device handle by calling
                       appropriate API.<p />
    RegisterAddress -  Start Address(in the XDATA space) from where Write
                       operation starts<p />
    BytesToWrite -     Number of bytes to be write<p />
    OutputData -       Pointer to the buffer containing data to write to
                       XDATA registers.
  Return:
    TRUE - for Success; FALSE - (Call GetMchpUsbLastErr for more details) -
    for failure
  Example:
    <code>
    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfully");

    UINT8 byData = 0x55;
    UINT32 dwAddr = 0xBF803004;
    printf("Xdata Write operation, Write 0x%02x in 0x%04xn",byData,wAddr);
    if (FALSE ==MchpUsbRegisterWrite(hDevice,dwAddr,1,byData))
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Failed to open the device Error,%04x",dwError);
       	exit (1);
    }
    cout \<\< "Success :Xdata Write operation";
    byData = 0x00;

    cout \<\< "Xdata Read operation";
    if (FALSE ==MchpUsbRegisterRead(hDevice,wAddr,1,&amp;byData))
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    cout \<\< "Success : Xdata Read operation";
    printf("Xdata Read value is %02x from 0x%08x n",byData,wAddr);

    </code>
  Remarks:
    <code>
       None
    </code>

  **************************************************************************************************************************/
 BOOL  MchpUsbRegisterWrite( HANDLE DevID, UINT32 RegisterAddress,UINT16 BytesToWrite, UINT8* OutputData);

#ifdef __cplusplus
}
#endif
