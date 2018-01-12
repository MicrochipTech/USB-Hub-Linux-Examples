/*
**********************************************************************************
* © 2015 Microchip Technology Inc. and its subsidiaries.
* You may use this software and any derivatives exclusively with
* Microchip products.
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".
* NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
* AND FITNESS FOR A PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP
* PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
* TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
* CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF
* FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
* MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
* OF THESE TERMS.
**********************************************************************************
*  $Revision: #1.2 $  $DateTime: 2015/12/17 04:18:20 $  $    $
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

//MPLABConnect SDK Header file.
#include "typedef.h"
#include "MchpUSBInterface.h"
#include "SpiFlash.h"
#include "USBHubAbstraction.h"

//DLL Exports
#define FALSE								0
#define TRUE								1
#define min(a,b)							(((a) < (b)) ? (a) : (b))
#define MICROCHIP_HUB_VID						0x424



#define VID_MICROCHIP							0x0424
#define PID_HCE_DEVICE							0x4940



#define PIO32_DIR_ADDR   						0x0830
#define PIO32_OUT_ADDR							0x0834
#define PIO32_IN_ADDR							0x0838

#define PIO64_DIR_ADDR   						0x0930
#define PIO64_OUT_ADDR							0x0934
#define PIO64_IN_ADDR							0x0938

#define PIODIR_ADDR_OFFSET   						0x30
#define PIOOUT_ADDR_OFFSET 						0x34
#define PIOIN_ADDR_OFFSET 						0x38
#define PIO64_BASE_ADDRESS						0x900
#define PIO32_BASE_ADDRESS						0x800

#define PIO_INP_EN_ADDR							0x80
#define	 LED0_PIO0_CTL_0						0x0806
#define	 LED0_PIO0_CTL_0_XNOR						0x80
#define	 LED0_PIO0_CTL_0_MODE						0x40
#define	 LED0_PIO0_CTL_0_RATE						0x3F

#define	 LED0_PIO0_CTL_1						0x0807
#define	 LED0_PIO0_CTL_1_TRAIL_OFF					0xFC
#define	 LED0_PIO0_CTL_1_LED_ON						0x02
#define	 LED0_PIO0_CTL_1_LED_PIO					0x01

#define	 LED1_PIO1_CTL_0						0x0808
#define	 LED1_PIO1_CTL_0_XNOR						0x80
#define	 LED1_PIO1_CTL_0_MODE						0x40
#define	 LED1_PIO1_CTL_0_RATE						0x3F

#define	 LED1_PIO1_CTL_1						0x0809
#define	 LED1_PIO1_CTL_1_TRAIL_OFF					0xFC
#define	 LED1_PIO1_CTL_1_LED_ON						0x02
#define	 LED1_PIO1_CTL_1_LED_PIO					0x01

#define SUSP_SEL							0x3C52
#define SUSP_SEL_SUSP_SEL						0x01

#define	 CHRG_DET_BYPASS						0x3C50
#define  CHRGEDET_BYPASS_BIT						0x80

#define  BOND_OPT0							0x0802
#define  BOND_OPT0_OPTRESEN						0x80
#define  BOND_OPT0_PKG_TYPE						0x18



#define  PORT_SEL1							0x3C00
#define  PORT_SEL2							0x3C04
#define  PORT_SEL3							0x3C08
#define  PORT_SEL4							0x3C0C
#define  VBUS_PASS_THRU							0x3C40
#define  PORT_DISABLE_GPIO_EN_VAL					0x04
#define	 VBUS_GPIO_VAL							0x02

#define  UTIL_CONFIG1_ADDR						0x080A
#define  UTIL_CONFIG1_SOFENABLE						0x80
#define  UTIL_CONFIG1_TXDSEL						0x20
#define  UTIL_CONFIG1_DEBUG_JTAG					0x10
#define  UTIL_CONFIG1_RXDSEL						0x08
#define  UTIL_CONFIG1_SPIMASTERDIS					0x04


#define I2C_ENTER_PASSTHRG						0x70
#define I2C_EXIT_PASSTHRG						0x73
#define I2C_READ							1
#define I2C_WRITE							0

#define I2C_READ_CMD							0x72
#define I2C_WRITE_CMD							0x71
#define I2CFL_SEND_STOP							0x01
#define I2CFL_SEND_START						0x02
#define I2CFL_SEND_NACK							0x04

//USB2530 I2c Flags
#define SEND_NACK							0x04
#define SEND_START							0x02
#define SEND_STOP							0x01

#define I2C_READ_BIT 							0x01
#define CMD_I2C_WRITE							0x71
#define CMD_I2C_READ							0x72

#define	 I2C_CTL_ADDR							0x0954
#define  I2C_CTL_EXTHWI2CEN						0x02
#define  I2C_CTL_INTI2CEN						0x01

//UART
#define CMD_SET_CONTROL_FLAGS						0x2A
#define CMD_CLEAR_CONTROL_FLAGS						0x2B
#define CF_UART_PASSTHRU_ENABLED					0x20

#define CRYSTAL_FREQUENCY						(60000000)
#define MAX_S0REL_VALUE							1023
#define MAX_DEVICE_UART_ERROR_PERCENTAGE				2
#define UART_SET_REGS							0x40
#define UART_DATA_DEVICE_TO_PC						0x42
#define UART_DATA_PC_TO_DEVICE						0x41

//OTP
#define OTP_DATA_START_ADDRESS						0x0002
#define BIG_ENDIAN_WORD(w) 						((((w)&0xFF)<<8) | (((w)&0xFF00) >> 8))

#define	 OCS_SEL1							 0x3c20
#define	 OCS_SEL2							 0x3c24
#define	 OCS_SEL3							 0x3c28
#define	 OCS_SEL4							 0x3c2C

#define CONVERT_ENDIAN_DWORD(w)	((((DWORD32)(w)) << 24) | (((DWORD32)(w) & 0xFF00) << 8) | \
								 (((DWORD32)(w) & 0xFF0000) >> 8) | (((DWORD32)(w) & 0xFF000000) >> 24))

#define PT2_LIB_VER							"1.23"

#define CTRL_RETRIES 							2
#define HUB_STATUS_BYTELEN						3 /* max 3 bytes status = hub + 23 ports */
#define HUB_DESC_NUM_PORTS_OFFSET					2


#define logprint(x, ...) do { \
		printf(__VA_ARGS__); \
		fprintf(x,  __VA_ARGS__); \


//UART
typedef struct
{
	BYTE bySerialPortMode;
	BYTE byBD;
	BYTE bySMOD;
	BYTE byTH1;
	WORD wS0REL;

} UART_REGS,*PUART_REGS;

//OTP
typedef struct tagOtpCfgChecksumA
{
	uint8_t	abyXORChecksum;
	uint16_t	wCfgStartOffset;
	uint16_t	wCfgLength;

}OTP_CFG_CHECKSUM;

typedef struct tagOtpCfgChecksumA1
{
	uint8_t	abySignature [3];
	OTP_CFG_CHECKSUM otpCfgChecksum;

}OTP_CFG_CHECKSUM_A1, *POTP_CFG_CHECKSUM_A1;

/*-----------------------Helper functions --------------------------*/
int usb_enable_HCE_device(uint8_t hub_index);
static int compare_hubs(const void *p1, const void *p2);
static int usb_get_hubs(PHINFO pHubInfoList);
static int usb_get_hub_list(PCHAR pHubInfoList);
static int usb_open_HCE_device(uint8_t hub_index);
int  usb_send_vsm_command(struct libusb_device_handle *handle, uint8_t * byValue) ;
bool UsbSetBitXdata(int hub_index,WORD wXDataAddress,BYTE byBitToSet);
bool UsbClearBitXdata(int hub_index,WORD wXDataAddress,BYTE byBitToClear);
int Read_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes);
int Write_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes);
int xdata_read(HANDLE handle, uint16_t wAddress, uint8_t *data, uint8_t num_bytes);
int xdata_write(HANDLE handle, uint16_t wAddress, uint8_t *data, uint8_t num_bytes);

//I2C Bridging
int USB2530API_I2C_Enter_Exit_Passthrough(int dev_handle, bool bEnable);
int USB_I2C_Transfer(int hub_index,BOOL byDir,uint8_t byAddr, uint8_t *pbyBuffer, uint16_t wLength , uint32_t *wdActualLength);
BOOL Gen_I2C_Transfer (int hub_index, BOOL bDirection, BYTE* pbyBuffer, WORD wDataLen, BYTE bySlaveAddress,BOOL bStart,BOOL bStop,BOOL bNack);

//UART Bridging
BOOL CalculateDeviceUARTRegisterSettings (uint8_t byMode, PUART_REGS pUARTRegs, uint32_t dwDesiredBaudRate);
BOOL SetDeviceUARTRegisters(int hub_index, UART_REGS *pUARTRegs);

//OTP Programming
unsigned int CalculateNumberofOnes(unsigned int UINTVar);

 // Global variable for tracking the list of hubs
HINFO gasHubInfo [MAX_HUBS];
/* Context Variable used for initializing LibUSB session */
libusb_context *ctx = NULL;

/*-----------------------API functions --------------------------*/
BOOL  MchpUsbGetVersion ( PCHAR pchVersionNo )
{
	BOOL bRet = TRUE;

	//Send command to lib to get library version
	sprintf(pchVersionNo,"%s",PT2_LIB_VER);

	return bRet;

}

// Get last error for the specific hub instance.
UINT32 MchpUsbGetLastErr (HANDLE DevID)
{
	DevID = DevID;
	return errno;
}

int MchpGetHubList(PCHAR pchHubcount )
{
	int hub_count =0;
	hub_count  = usb_get_hub_list(pchHubcount);
	return hub_count;
}

//Return handle to the first instance of VendorID &amp; ProductID matched device.
HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID)
{
	int error = 0, hub_cnt=0, hub_index=0;
	int restart_count=5;
	bool bhub_found = false;

	//Get the list of the hubs from the device.
	hub_cnt = usb_get_hubs(&gasHubInfo[0]);
	do
	{
		if((gasHubInfo[hub_index].wVID == wVID) && (gasHubInfo[hub_index].wPID == wPID))
		{
			bhub_found = true;
			break;
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

//Return handle to the first instance of VendorID &amp; ProductID &amp; port path matched device.
HANDLE  MchpUsbOpen ( UINT16 wVID, UINT16 wPID,char* cDevicePath)
{
    	int error = 0, hub_index=0;
    	int restart_count=5;
    	bool bhub_found = false;

	char *ptr;
	BYTE DeviceID[7];
	int  i=0 ;
	ptr = cDevicePath;


	while(*ptr != '\0')
	{
		if(*ptr != ':')
		{
			DeviceID[i] = *ptr - 48;
			i++;
		}
		ptr++;
	}


	if( 0 == usb_get_hubs(&gasHubInfo[0]))
	{
		DEBUGPRINT("MCHP_Error_libusb_error_no_hub \n");
		return INVALID_HANDLE_VALUE;
	}

    	for(;hub_index < 10;hub_index++)
    	{
		if((gasHubInfo[hub_index].wVID == wVID) && (gasHubInfo[hub_index].wPID == wPID) && (i == gasHubInfo[hub_index].port_max))
		{
			 for (int j=0; j<gasHubInfo[hub_index].port_max; j++)
			{
				if(gasHubInfo[hub_index].port_list[j] != DeviceID[j])
				{
					bhub_found = false;
					break;

				}
				bhub_found = true;

			}
			if(true == bhub_found)
			{
				break;
			}


		}

	}

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


//Close the device handle.
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

//Read the XDATA register(s) in the XDATA space of internal registers
BOOL  MchpUsbRegisterRead ( HANDLE DevID, UINT16 RegisterAddress, UINT16 BytesToRead, UINT8* InputData )
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
	UsbCtlPkt.byRequest = 0x04;
	UsbCtlPkt.wValue 	= RegisterAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= InputData;
	UsbCtlPkt.wLength 	= BytesToRead;

	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	if(bRetVal < 0)
	{
		return false;
	}
	return true;
}

//Write to the XDATA register (s) in the XDATA space of internal
BOOL  MchpUsbRegisterWrite( HANDLE DevID, UINT16 RegisterAddress,UINT16 BytesToWrite, UINT8* OutputData)
{
	int bRetVal = FALSE;
	if(nullptr == OutputData)
	{
		DEBUGPRINT("Register write failed: NULL POINTER \n");
		return FALSE;
	}
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
	UsbCtlPkt.byRequest = 0x03;
	UsbCtlPkt.wValue 	= RegisterAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= OutputData;
	UsbCtlPkt.wLength 	= BytesToWrite;

	bRetVal = usb_HCE_write_data (&UsbCtlPkt);
	if(bRetVal < 0)
	{
		return false;
	}
	return true;
}

//Send the Flexconnect command to device.
BOOL MchpUsbFlexConnect (HANDLE DevID, UINT16 Config)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
	UsbCtlPkt.byRequest = 0x08;
	UsbCtlPkt.wValue 	= Config;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= 0;
	UsbCtlPkt.wLength 	= 0;

	bRetVal = usb_HCE_no_data(&UsbCtlPkt);
	if(bRetVal< 0)
	{
		DEBUGPRINT("Execute Enable Flex failed %d\n",bRetVal);
		return false;
	}
	return true;
}

//Get the state of the specified GPIO pin
BOOL MchpUsbGpioGet (HANDLE DevID, UINT8 PIONumber, UINT8* Pinstate)
{
	BOOL bRet = FALSE;
	WORD wBaseRegisterAddress = 0;

	DWORD dwInputData;
	DWORD dwMask = 0;

	if(!((PIONumber <=10) || (PIONumber <=20 && PIONumber >=14) || (PIONumber >=41 && PIONumber <=45)))
	{
		DEBUGPRINT("Invalid PIO Number \n");
		return bRet;
	}
	//PIO 15
	if(PIONumber == 15)
	{
		DEBUGPRINT("Invalid PIO Number \n");
		return bRet;
	}

	//PIO 31
	if (PIONumber > 31)
	{
		dwMask = (1 << (PIONumber - 32));
	}
	else
	{
		dwMask = (1 << PIONumber);
	}

	/*The base address is 0x800 for below 32 and 0x900 for upper32*/
	if(PIONumber > 32)
	{
		wBaseRegisterAddress = 0x900;
	}
	else
	{
		wBaseRegisterAddress = 0x800;
	}
	do
	{

			//Set direction offset to input
			bRet = MchpUsbRegisterRead( DevID, wBaseRegisterAddress + PIODIR_ADDR_OFFSET, 4, (BYTE *)&dwInputData);
			if(FALSE == bRet)
			{
				DEBUGPRINT("MchpUsbGpioGet : Read direction Failed, Hubid : %d, PIONumber: %d",DevID,PIONumber);

				break;
			}
			//For input clear dir mask
			dwInputData = CONVERT_ENDIAN_DWORD (dwInputData) &~ dwMask;

			dwInputData = CONVERT_ENDIAN_DWORD (dwInputData);


			bRet = MchpUsbRegisterWrite( DevID, wBaseRegisterAddress + PIODIR_ADDR_OFFSET, 4, (BYTE *)&dwInputData);
			if(FALSE == bRet)
			{
				DEBUGPRINT("MchpUsbGpioGet : Set direction Failed, Hubid : %d, PIONumber: %d",DevID,PIONumber);
				break;
			}

			/*Read the PIO direction regsiter*/
			bRet = MchpUsbRegisterRead( DevID, (wBaseRegisterAddress + PIOIN_ADDR_OFFSET), 4, (BYTE *)&dwInputData);
			if(FALSE == bRet)
			{
				DEBUGPRINT("MchpUsbGpioGet : Write PIO direction Failed, Hubid : %d, PIONumber: %d",DevID,PIONumber);

				break;
			}

			(CONVERT_ENDIAN_DWORD (dwInputData) & dwMask)? (*Pinstate = 1) : (*Pinstate=0);
			DEBUGPRINT("MchpUsbGpioGet : Success, Hubid : %d, PIONumber: %d",DevID,PIONumber);

	}while(FALSE);


	return bRet;
}

 BOOL MchpUsbGpioSet (HANDLE DevID, UINT8 PIONumber, UINT8 Pinstate)
{
	BOOL bRet = FALSE;
	WORD wBaseRegisterAddress = 0;
	DWORD dwInputData;
	DWORD dwMask = 0;

	if(!((PIONumber <=10) || (PIONumber <=20 && PIONumber >=14) || (PIONumber >=41 && PIONumber <=45)))
	{
		DEBUGPRINT("Invalid PIO Number \n");
		return bRet;
	}
	if(PIONumber == 15)
	{
		DEBUGPRINT("Invalid PIO Number \n");
		return bRet;
	}
	if (PIONumber > 31)
	{
		dwMask = (1 << (PIONumber - 32));
	}
	else
	{
		dwMask = (1 << PIONumber);
	}

	/*The base address is 0x800 for below 32 and 0x900 for upper32*/
	if(PIONumber > 32)
	{
		wBaseRegisterAddress = 0x900;
	}
	else
	{
		wBaseRegisterAddress = 0x800;
	}


	do
	{

		//Set direction offset to ouput
		bRet = MchpUsbRegisterRead( DevID, wBaseRegisterAddress + PIODIR_ADDR_OFFSET, 4, (BYTE *)&dwInputData);
		if(FALSE == bRet)
		{
			DEBUGPRINT("MchpUsbGpioSet : Read direction Failed, ,Hubid : %d, PIONumber: %d, Pinstate: %d\n",DevID,PIONumber,Pinstate);
			break;
		}
		//For input clear dir mask
		dwInputData = CONVERT_ENDIAN_DWORD (dwInputData) | dwMask;
		dwInputData = CONVERT_ENDIAN_DWORD (dwInputData);


		bRet = MchpUsbRegisterWrite( DevID, wBaseRegisterAddress + PIODIR_ADDR_OFFSET, 4, (BYTE *)&dwInputData);
		if(FALSE == bRet)
		{
			DEBUGPRINT( "MchpUsbGpioSet : Write dir mask Failed, Hubid : %d, PIONumber: %d, Pinstate: %d\n",DevID,PIONumber,Pinstate);

			break;
		}


		/*Read the OUT register*/
		bRet = MchpUsbRegisterRead( DevID, wBaseRegisterAddress + PIOOUT_ADDR_OFFSET, 4, (BYTE *)&dwInputData);
		if(FALSE == bRet)
		{
			DEBUGPRINT( "MchpUsbGpioSet : Read OUT Reg Failed, Hubid : %d, PIONumber: %d, Pinstate: %d\n",DevID,PIONumber,Pinstate);

			break;
		}

		/*Update the PIN state*/
		if(1 == Pinstate)
		{
			dwInputData = CONVERT_ENDIAN_DWORD (dwInputData) | dwMask;
		}
		else if(0 == Pinstate)
		{
			dwInputData = CONVERT_ENDIAN_DWORD (dwInputData) &~ dwMask;
		}
		else{
			DEBUGPRINT("Invalid Pin State \n");
			return false;
		}

		dwInputData = CONVERT_ENDIAN_DWORD (dwInputData);


		bRet = MchpUsbRegisterWrite( DevID, wBaseRegisterAddress + PIOOUT_ADDR_OFFSET, 4, (BYTE *)&dwInputData);
		if(FALSE == bRet)
		{
			DEBUGPRINT( "MchpUsbGpioSet : Failed, Hubid : %d, PIONumber: %d, Pinstate: %d\n",DevID,PIONumber,Pinstate);

			break;
		}
		DEBUGPRINT( "MchpUsbGpioSet : Success, Hubid : %d, PIONumber: %d,Pinstate: %d\n",DevID,PIONumber,Pinstate);

		bRet = TRUE;
		break;


	}while(FALSE);


	return bRet;
}

//configures the specified PIO line for general purpose
BOOL MchpUsbConfigureGPIO (HANDLE DevID, UINT8 PIONumber)
{
	BOOL bRet = FALSE;

	BYTE byTemp;
	BOOL bValidGPIO = false;
	switch(PIONumber)
	{
		//PIO 0
		case 0:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, LED0_PIO0_CTL_1, LED0_PIO0_CTL_1_LED_PIO);
			if (!bRet)
			{
				break;
			}

			bRet = UsbClearBitXdata(DevID, SUSP_SEL, SUSP_SEL_SUSP_SEL);
			break;

		//PIO 1
		case 1:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, LED1_PIO1_CTL_1, LED1_PIO1_CTL_1_LED_PIO);
			if (!bRet)
			{
				break;
			}

			bRet = UsbClearBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_SOFENABLE);
			break;

		//PIO 2
		case 2:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, I2C_CTL_ADDR, I2C_CTL_INTI2CEN);
			break;

		//PIO 3
		case 3:
			// Nothing to do
			bValidGPIO = true;
			bRet = true;
			break;

		//PIO 4
		case 4:
			bValidGPIO = true;
			bRet = UsbSetBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_SPIMASTERDIS);
			break;

		//PIO 5
		case 5:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_TXDSEL);
			if (!bRet)
			{
				break;
			}

			bRet = UsbSetBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_SPIMASTERDIS);
			break;

		//PIO 6
		case 6:
			bValidGPIO = true;
			bRet = UsbSetBitXdata(DevID, CHRG_DET_BYPASS, CHRGEDET_BYPASS_BIT);
			break;

		//PIO 7
		case 7:
			bValidGPIO = true;
			bRet = UsbSetBitXdata(DevID, CHRG_DET_BYPASS, CHRGEDET_BYPASS_BIT);
			break;

		//PIO 8
		case 8:
			// Nothing to do
			bValidGPIO = true;
			bRet = true;
			break;

		//PIO 9
		case 9:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_RXDSEL);		 //rxd_sel false
			if (!bRet)
			{
				break;
			}

			bRet = UsbSetBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_SPIMASTERDIS);	 //spi_master_dis true
			break;

		//PIO 10
		case 10:
			// Nothing to do
			bValidGPIO = true;
			bRet = true;
			break;

		//PIO 14
		case 14:
			bValidGPIO = true;
			byTemp = 0xC0;
			bRet= MchpUsbRegisterWrite( DevID, 0x3C51, 1, &byTemp) ;
			if (!bRet)
			{
				break;
			}

			bRet = UsbClearBitXdata(DevID, SUSP_SEL, SUSP_SEL_SUSP_SEL);
			break;

		//PIO 16
		case 16:
			bValidGPIO = true;
			bRet = UsbSetBitXdata(DevID, VBUS_PASS_THRU, VBUS_GPIO_VAL);
			if (!bRet)
			{
				break;
			}
			// Break intentionally left out here

		//PIO 17
		case 17:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, 0x3c00, 0x80);
			if (!bRet)
			{
				break;
			}

			byTemp = 0x02;
			bRet= MchpUsbRegisterWrite( DevID, 0x3c20, 1, &byTemp) ;
			break;

		//PIO 18
		case 18:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, 0x3c04, 0x80);
			if (!bRet)
			{
				break;
			}

			byTemp = 0x02;
			bRet= MchpUsbRegisterWrite( DevID, 0x3c24, 1, &byTemp) ;
			break;

		//PIO 19
		case 19:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_RXDSEL);		//rxd_sel false
			if (!bRet)
			{
				break;
			}

			bRet = UsbClearBitXdata(DevID, 0x3c08, 0x80);
			if (!bRet)
			{
				break;
			}

			byTemp = 0x02;
			bRet= MchpUsbRegisterWrite( DevID, 0x3c28, 1, &byTemp) ;
			break;

		//PIO 20
		case 20:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, UTIL_CONFIG1_ADDR, UTIL_CONFIG1_TXDSEL);		//txd_sel false
			if (!bRet)
			{
				break;
			}

			bRet = UsbClearBitXdata(DevID, 0x3c0c, 0x80);
			if (!bRet)
			{
				break;
			}

			byTemp = 0x02;
			bRet= MchpUsbRegisterWrite( DevID, 0x3c2c, 1, &byTemp) ;
			break;

		//PIO 41
		case 41:
			bValidGPIO = true;
			byTemp = PORT_DISABLE_GPIO_EN_VAL;
			bRet= MchpUsbRegisterWrite( DevID, PORT_SEL1, 1, &byTemp) ;
			break;

		//PIO 42
		case 42:
			bValidGPIO = true;
			byTemp = PORT_DISABLE_GPIO_EN_VAL;
			bRet= MchpUsbRegisterWrite( DevID, PORT_SEL2, 1, &byTemp);
			break;

		//PIO 43
		case 43:
			bValidGPIO = true;
			byTemp = PORT_DISABLE_GPIO_EN_VAL;
			bRet= MchpUsbRegisterWrite( DevID, PORT_SEL3, 1, &byTemp) ;
			break;

		//PIO 44
		case 44:
			bValidGPIO = true;
			byTemp = PORT_DISABLE_GPIO_EN_VAL;
			bRet= MchpUsbRegisterWrite( DevID, PORT_SEL4, 1, &byTemp) ;
			break;

		//PIO 45
		case 45:
			bValidGPIO = true;
			bRet = UsbClearBitXdata(DevID, I2C_CTL_ADDR, (I2C_CTL_EXTHWI2CEN | I2C_CTL_INTI2CEN));
			break;

	}
	if(false == bValidGPIO && false == bRet)
	{
		DEBUGPRINT("Invalid Pin Number \n");
		bRet = FALSE;
	}
	return bRet;
}

//Set I2C config parameters
BOOL MchpUsbI2CSetConfig(HANDLE DevID)
{
	int bRetval = FALSE;
	do
	{
		bRetval = USB2530API_I2C_Enter_Exit_Passthrough(DevID,TRUE);
		if(bRetval < 0)
		{
			DEBUGPRINT( "I2C Enter Failed \n");
			bRetval = FALSE;
			break;
		}
		else
		{
			DEBUGPRINT( "I2C Enter SUCCESS \n");
			bRetval = TRUE;
		}

	}while(FALSE);
	return bRetval;
}

//I2C read through the I2C pass-through interface of USB device
BOOL MchpUsbI2CRead (HANDLE DevID, INT BytesToRead, UINT8* InputData, UINT8 SlaveAddress)
{
	BOOL bRet = FALSE;
	UINT32 wdActualLength = 0;

	do
	{
		bRet = USB_I2C_Transfer(DevID, I2C_READ, SlaveAddress, InputData, BytesToRead, (uint32_t *)&wdActualLength);
	}while(FALSE);
	return bRet;
}

//I2C write through the I2C pass-through interface of USB device
BOOL MchpUsbI2CWrite (HANDLE DevID, INT BytesToWrite, UINT8* OutputData, UINT8 SlaveAddress)
{
	BOOL bRet = FALSE;
	UINT32 wdActualLength = 0;
	if(nullptr == OutputData)
	{
		DEBUGPRINT("I2C Write Failed: NULL pointer\n");
		return FALSE;
	}
	do
	{
		bRet = USB_I2C_Transfer((int)DevID, I2C_WRITE, SlaveAddress, OutputData, BytesToWrite,(uint32_t *) &wdActualLength);
	}while(FALSE);
	return bRet;
}

//I2C write and read through the I2C pass-through interface of USB device
BOOL MchpUsbI2CTransfer (HANDLE DevID, BOOL bDirection, UINT8* pbyBuffer, UINT16 wDataLen, UINT8 bySlaveAddress,BOOL bStart,BOOL bStop,BOOL bNack)
{
	BOOL bRet = FALSE;
	if(nullptr == pbyBuffer)
	{
		DEBUGPRINT("I2C Transfer Failed: NULL pointer\n");
		return FALSE;
	}

	bRet = Gen_I2C_Transfer(DevID,bDirection,pbyBuffer,wDataLen,bySlaveAddress,bStart,bStop,bNack);
	if(bRet == FALSE)
	{
		DEBUGPRINT("I2C Transfer Failed \n");
	}
	else
	{
		DEBUGPRINT("I2C Transfer Success \n");
	}
	return bRet;
}

//Enables the UART device for serial communication.
BOOL MchpUsbEnableUARTBridging (HANDLE DevID, BOOL bEnable)
{
	int bRet = FALSE;

	if(bEnable)
	{
		bRet = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,CMD_SET_CONTROL_FLAGS,CF_UART_PASSTHRU_ENABLED,0,0,0,500);
	}
	else
	{
		bRet = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,CMD_CLEAR_CONTROL_FLAGS,CF_UART_PASSTHRU_ENABLED,0,0,0,500);
	}
	if(bRet<0)
	{
		DEBUGPRINT("MchpUsbEnableUARTBridging Failed\n");
		return FALSE;
	}
	else
	{
		DEBUGPRINT("MchpUsbEnableUARTBridging Success\n");
		return TRUE;
	}
}

//configures the baud rate for serial communication.
BOOL MchpUsbSetUARTBaudrate (HANDLE DevID, UINT32 BaudRate)
{
	UINT8 byMode, k;
	UART_REGS	sUARTRegs;
	BOOL bRetVal = FALSE;

	byMode = 1; // Only mode 1 is supported in passthru mode

	do
	{
		for (k=0; k < 4; k++)	// Corresponds to 4 different combinations of BD & SMOD bit, but used only in mode 1,3
		{
			memset (&sUARTRegs, 0, sizeof (UART_REGS));
			sUARTRegs.bySerialPortMode = byMode;
			sUARTRegs.bySMOD = (UINT8) (k & 0x01);
			sUARTRegs.byBD = (UINT8) ((k & 0x02) >> 1);

			bRetVal = CalculateDeviceUARTRegisterSettings (byMode, &sUARTRegs, BaudRate);

			if (FALSE == bRetVal)
			{
				continue;	// Device has error % > 2
			}
			bRetVal = SetDeviceUARTRegisters((int)DevID, &sUARTRegs);

			if(FALSE == bRetVal)
			{
				DEBUGPRINT("Error UART Set Baud Rate\n");
				bRetVal = FALSE;
			}
			else
			{
				DEBUGPRINT("UART Set Baud Rate Success\n");
				bRetVal = TRUE;
				break;
			}

		}
	}while (FALSE);

	return bRetVal;
}
BOOL MchpUsbUartWrite (HANDLE DevID, UINT32 BytesToWrite,UINT8 *OutputData)
{
	BOOL bRetVal = TRUE;
	LONG ulBytesSent;
	DWORD dwUSBSent =0, dwPendingLen;

	dwPendingLen = BytesToWrite;

	if(nullptr == OutputData)
	{
		DEBUGPRINT("UART Write failed: NULL pointer");
		return FALSE;
	}
	while (dwPendingLen && bRetVal)
	{

		ulBytesSent = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,UART_DATA_PC_TO_DEVICE,0,0,OutputData,
							min(64,dwPendingLen),500);

		if(ulBytesSent <0)
		{
			DEBUGPRINT("Mchp UART Write Failed \n");
			bRetVal = FALSE;
		}
		else
		{
			if(!ulBytesSent)
			{
				bRetVal = FALSE;
				break;
			}
			dwPendingLen -=ulBytesSent;
			dwUSBSent += ulBytesSent;
		}
	}
	return bRetVal;
}

//Transfers data through serial port to the connected serial peripheral.
BOOL MchpUsbUartRead (HANDLE DevID, UINT32 BytesToRead,UINT8 *InputData)
{
	BOOL bRet = FALSE;
	int rc = FALSE;
	rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xC1,UART_DATA_DEVICE_TO_PC,0,0,InputData,BytesToRead,500);

	if(rc < 0)
	{
		DEBUGPRINT("Mchp UART Read Failed \n");
		bRet = FALSE;
	}
	else
	{
		DEBUGPRINT("Mchp UART Read Passed \n");
		bRet = TRUE;
	}
	return bRet;
}
BOOL MchpUsbSpiSetConfig ( HANDLE DevID, INT EnterExit)
{
	int rc = FALSE;
	BOOL bRet = FALSE;
	BYTE byCmd =0;

	if(EnterExit)
	{
		byCmd = CMD_SPI_PASSTHRU_ENTER;
	}
	else
	{
		byCmd = CMD_SPI_PASSTHRU_EXIT;
	}
	rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,byCmd,0,0,0,0,500);
	if(rc < 0)
	{
		DEBUGPRINT("MchpUsbSpiSetConfig failed\n");
		bRet = FALSE;
	}
	else
	{
		DEBUGPRINT("MchpUsbSpiSetConfig Passed\n");
		bRet = TRUE;
	}
	return bRet;
}
BOOL MchpUsbSpiFlashRead(HANDLE DevID,UINT32 StartAddr,UINT8* InputData,UINT32 BytesToRead)
{
	BOOL bRet = FALSE;

	if ((StartAddr + BytesToRead) > MAX_FW_SIZE)
	{
		DEBUGPRINT("MchpUsbSpiFlashRead Failed: BytesToRead (%d) and StartAddr(0x%x) is larger than SPI memory size\n",BytesToRead,StartAddr);
		return bRet;
	}
	//boots from SPI
	bRet = AthensReadSPIFlash(DevID,InputData,StartAddr,(StartAddr + BytesToRead));

	if(FALSE == bRet)
	{
		DEBUGPRINT("MchpUsbSpiFlashRead Failed \n");
	}
	else
	{
		DEBUGPRINT("MchpUsbSpiFlashRead Passed \n");
		bRet = TRUE;
	}
	return bRet;
}
BOOL MchpUsbSpiFlashWrite(HANDLE DevID,UINT32 StartAddr,UINT8* OutputData, UINT32 BytesToWrite)
{
	BOOL bRet = FALSE;
	if(nullptr == OutputData)
	{
		DEBUGPRINT("SPI Write failed: NULL pointer");
		return FALSE;
	}

	if ((StartAddr + BytesToWrite) > MAX_FW_SIZE)
	{
		DEBUGPRINT("MchpUsbSpiFlashWrite Failed: BytesToWrite (%d) and StartAddr(0x%x) is larger than SPI memory size\n",BytesToWrite,StartAddr);
		return bRet;
	}
	bRet = WriteSPIFlash(DevID,OutputData,5000,TRUE,StartAddr,BytesToWrite);

	if(FALSE == bRet)
	{
		DEBUGPRINT("MchpUsbSpiFlashWrite failed \n");
	}
	else
	{
		DEBUGPRINT("MchpUsbSpiFlashWrite success \n");
		bRet = TRUE;
	}
	//Reset the device.
	uint8_t byData = 0;
	xdata_read(DevID, 0x804, &byData, 1);

	byData|= 0x04;
	xdata_write(DevID, 0x804, &byData, 1);

	byData = 0x40;
	xdata_write(DevID, 0x80A, &byData, 1);

	return bRet;
}
BOOL MchpUsbSpiTransfer(HANDLE DevID,INT Direction,UINT8* Buffer, UINT16 DataLength,UINT32 TotalLength)
{
	int bRetVal = FALSE;
	if(nullptr == Buffer)
	{
		DEBUGPRINT("SPI Transfer failed: NULL pointer");
		return FALSE;
	}

	if(Direction) //Read
	{
		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xC1,0x04,(0x4A10),0,Buffer,
					TotalLength,CTRL_TIMEOUT);
	}
	else //Write
	{
		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,0x61,TotalLength,0,Buffer,
					DataLength,CTRL_TIMEOUT);
	}
	if(bRetVal <0 )
	{
		DEBUGPRINT("SPI Transfer Failed \n");
		return FALSE;
	}
	else
	{
		DEBUGPRINT("SPI Transfer success \n ");
		return TRUE;
	}

}
BOOL MchpProgramFile( HANDLE DevID, PCHAR InputFileName)
{
	BOOL bRet = FALSE;
	//Read entire OTP
	uint8_t abyBuffer[2048];
	uint8_t  pbyBuffer[64];
	uint8_t wDataLength;
	uint16_t wConfigBytes;
	uint16_t wNumberOfOnes;
	wDataLength = ReadBinfile(InputFileName,pbyBuffer);
	if(0 == wDataLength)
	{
		DEBUGPRINT("Failed to Read Given Configuration File \n");
		return bRet;
	}
	bRet= Read_OTP(DevID, 0, abyBuffer, 2048);
	if(bRet < 0)
	{
		DEBUGPRINT("Failed to Read OTP Content \n");
		return bRet;
	}

	//find whether device boots from SPI or ROM
	Get_Hub_Info(DevID, (uint8_t *)&gasHubInfo[DevID].sHubInfo);

	// Update Number of configuration updated in OTP
	//Note that by default 0th byte is 0x00 and 1st byte is 0xff present in the OTP.
	//That is why xor is used below command
	if(gasHubInfo[DevID].sHubInfo.byFeaturesFlag & 0x01)
	{
		wConfigBytes = (abyBuffer[0] << 8) | (abyBuffer[1]);
		wNumberOfOnes = CalculateNumberofOnes(wConfigBytes);
		wNumberOfOnes = (16 - wNumberOfOnes);
		wConfigBytes &= ~(1 << wNumberOfOnes);

		//Update The OTP buffer
		abyBuffer[0] = (uint8_t)((wConfigBytes & 0xFF00) >> 8); //MSB
		abyBuffer[1] = (uint8_t)(wConfigBytes & 0x00FF); //LSB
	}
	else
	{
		wConfigBytes = (abyBuffer[0] << 8) |(abyBuffer[1] ^ 0xFF);
		//Calculate number of configuration present in OTP
		wNumberOfOnes = CalculateNumberofOnes(wConfigBytes);
		//Set the BitMask
		wConfigBytes = wConfigBytes | (1 << wNumberOfOnes);

		//Update the OTP buffer for indicating programming count is incremented by one.
		//First two bytes will represent the number of times the OTP is programmed.
		abyBuffer[0] = (uint8_t)((wConfigBytes & 0xFF00) >> 8); //MSB
		abyBuffer[1] = ((uint8_t)(wConfigBytes & 0x00FF) ^ 0xFF ); //LSB
	}


	//This is the logic for finding the OTP configuartion record update and data update.
	//Start from lowest index
	//By deafult, Data starts at 2 if no header found and record header will point end
	//of otp minus the last configuarion data(2048-8).
	uint16_t gwOTPDataOffset = OTP_DATA_START_ADDRESS;
	uint16_t gwOTPHdrOffset = 2048 - sizeof(OTP_CFG_CHECKSUM_A1);
	uint16_t wTmpOTPDataOffset =0, wTmpLenght=0;
	OTP_CFG_CHECKSUM_A1 *pstChecksumA1 = NULL;

	pstChecksumA1 = (OTP_CFG_CHECKSUM_A1 *) &abyBuffer[gwOTPHdrOffset];

	wTmpOTPDataOffset = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgStartOffset);
	wTmpLenght = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgLength);

	while (('I' == pstChecksumA1->abySignature [0]) && \
			('D' == pstChecksumA1->abySignature [1]) && \
			('X' == pstChecksumA1->abySignature [2]))
	{
		if ((wTmpOTPDataOffset > 0x0800) || \
			(wTmpLenght > 0x0800))
		{
			// Though signature matched, still the offset or the length field is
			// indicating OTP access more than 2K, which is invalid
			// Probably an invlid index record, where the random bytes matched "IDX" pattern.
			DEBUGPRINT("Trying to access more than 2k OTP memory\n");
			return bRet;
		}

		// Update the data offset as valid header is found
		gwOTPDataOffset = (wTmpOTPDataOffset + wTmpLenght);

		// Move to next header
		pstChecksumA1 --;
		gwOTPHdrOffset-=sizeof(OTP_CFG_CHECKSUM_A1);

		wTmpOTPDataOffset = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgStartOffset);
		wTmpLenght = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgLength);
	}
	uint16_t wTotalIndexSize = (2048 - gwOTPHdrOffset);

	if(wDataLength >= (unsigned int)(2048 - gwOTPDataOffset - wTotalIndexSize))
	{
		DEBUGPRINT("Error: No more free space available for programming OTP\n");
		return bRet;
	}
	//////////////////////////////////////////////
	// Update the OTP buffer for indicating programming count is incremented by one
	bRet = Write_OTP(DevID, 0, abyBuffer, 2);
	if(bRet < 0)
	{
		DEBUGPRINT("Failed to write OTP \n");
		return bRet;
	}
	//////////////////////////////////////////////
	// Update the otp data with new cfg block
	bRet = Write_OTP(DevID, gwOTPDataOffset, pbyBuffer, wDataLength);
	if(bRet < 0)
	{
		DEBUGPRINT ("Failed to write OTP \n");
		return bRet;
	}
	//For comparing after programming.
	memcpy (&abyBuffer[gwOTPDataOffset], pbyBuffer, wDataLength);

	uint8_t byChecksum ;
	OTP_CFG_CHECKSUM_A1 stChecksum;

	// Calculate the checksum
	for (int i = 0; i < wDataLength; i++)
	{
		byChecksum ^= pbyBuffer [i];
	}
	//OTP_CFG_CHECKSUM_A1 stChecksum;
	stChecksum.abySignature [0] = 'I';
	stChecksum.abySignature [1] = 'D';
	stChecksum.abySignature [2] = 'X';

	stChecksum.otpCfgChecksum.wCfgStartOffset = BIG_ENDIAN_WORD (gwOTPDataOffset);
	stChecksum.otpCfgChecksum.wCfgLength = BIG_ENDIAN_WORD (wDataLength);
	stChecksum.otpCfgChecksum.abyXORChecksum = byChecksum;

	//For comparing after programming.
	memcpy (&abyBuffer[gwOTPHdrOffset], &stChecksum, sizeof (OTP_CFG_CHECKSUM_A1));

	bRet = Write_OTP(DevID, gwOTPHdrOffset, (uint8_t *)&stChecksum, sizeof (OTP_CFG_CHECKSUM_A1));
	if(bRet < 0)
	{
		DEBUGPRINT("Failed to write OTP \n");
		return bRet;
	}
	sleep (2);

	//Verify OTP
	uint8_t abyVerifyBuffer[2048];
	bRet = Read_OTP(DevID, 0, abyVerifyBuffer, 2048);
	if(bRet < 0)
	{
		DEBUGPRINT("Failed to Read Config Memory \n");
		return bRet;
	}
	if(0 == memcmp(abyVerifyBuffer, abyBuffer, 2048))
	{
		printf("OTP wrote successfully\n");
		writeBinfile("actual_otp_data.bin",  abyBuffer, 2048);
		bRet = TRUE;
	}
	else
	{
		printf("Mismatch in OTP read data\n");
		writeBinfile("expected_otp_data.bin",  abyBuffer, 2048);
		writeBinfile("actual_otp_data.bin",  abyVerifyBuffer, 2048);
	}
	//Reset the device.
	uint8_t byData = 0;
	xdata_read(DevID, 0x804, &byData, 1);

	byData|= 0x04;
	xdata_write(DevID, 0x804, &byData, 1);

	byData = 0x40;
	xdata_write(DevID, 0x80A, &byData, 1);

	return bRet;
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

	cnt = libusb_get_device_list(ctx, &devs);
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
						DEBUGPRINT("UNKNOWN_LIBUSB_ERROR %x\n", error);
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
				libusb_close(handle);
				continue;
			}

			pHubInfoList->port_max = libusb_get_port_numbers(device, pHubInfoList->port_list, 7);

		  	if(pHubInfoList->port_max <= 0)
			{
				libusb_close(handle);
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

static int usb_get_hub_list(char *pHubInfoList)
{
	int cnt = 0,i = 0, error=0, port_cnt = 0, nportcnt = 0;
	libusb_device **devs;
	//libusb_device_handle *handle;
	libusb_device_descriptor desc;
//	PHINFO pHubListHead = pHubInfoList;	// Pointer to head of the list
	uint8_t port_list[7];

	*pHubInfoList = '\0';

	error = libusb_init(&ctx);

	if(error < 0)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Initialization LibUSB failed\n");
		return -1;
	}


	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0)
	{
		DEBUGPRINT("Failed to get the device list \n");
		return -1;
	}
	for (i = 0; i < cnt; i++)
	{
		int error = 0;

		libusb_device *device = devs[i];
		error = libusb_get_device_descriptor(device, &desc);

		if(error != 0)
		{
			DEBUGPRINT("LIBUSB_ERROR: Failed to retrieve device descriptor for device[%d] \n", i);
		}

		if((error ==  0) && (desc.bDeviceClass == LIBUSB_CLASS_HUB))
		{
			port_cnt = libusb_get_port_numbers(device, port_list, 7);

			if(port_cnt < 1)
			{
				continue;
			}

			nportcnt++;

			char dbgmsg[50];
			memset(dbgmsg,0,50);

			sprintf(pHubInfoList,"%sVID:PID = %04x:%04x", pHubInfoList, desc.idVendor, desc.idProduct);
			//pirintf("VID:PID = %04x:%04x", desc.idVendor, desc.idProduct);
			for(int j = 0; j < port_cnt; j++)
			{

				sprintf(dbgmsg,"%s:%d",dbgmsg,port_list[j]);

			}
			sprintf(pHubInfoList, "%s, Device Path - %s\n",pHubInfoList, dbgmsg);

		}
	}
	printf("%s\n", pHubInfoList);
	libusb_free_device_list(devs, 1);
	return nportcnt;
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
	ssize_t i = 0, j = 0;
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
			continue;
		}

		if(PID_HCE_DEVICE == desc.idProduct)
		{
			dRetval = libusb_open(dev, &handle);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device open failed \n");
				continue;
			}

			port_cnt = libusb_get_port_numbers(dev, port_list, 7);
			if(port_cnt <= 1)
			{
				DEBUGPRINT("Retrieving port numbers failed \n");
				libusb_close(handle);
				continue;
			}

			//it is comapring against hub port count not HCE
			if(gasHubInfo[hub_index].port_max != (port_cnt-1))
			{
				DEBUGPRINT("Hub port match failed with Hub the Index:%d\n",hub_index);
				libusb_close(handle);
                                continue;

			}

			//Match with the hub port list
			for(j = 0; j < gasHubInfo[hub_index].port_max; j++)
			{
				if(gasHubInfo[hub_index].port_list[j] != port_list[j])
				{
					DEBUGPRINT("Hub port match failed with Hub Index:%d\n",hub_index);
					dRetval = -1;
					break;
				}
			}

			if(dRetval == -1)
			{
				libusb_close(handle);
				continue;
			}

			printf("HCE Hub index=%d Path- ",hub_index);
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
					libusb_close(handle);
					break;
				}
			}

			dRetval = libusb_claim_interface(handle, 0);

			if(dRetval < 0)
			{
				DEBUGPRINT("cannot claim intterface \n");
				dRetval = -1;
				libusb_close(handle);
				break;
			}

			gasHubInfo[hub_index].dev = devices;
			gasHubInfo[hub_index].handle = handle;
			gasHubInfo[hub_index].byHubIndex = hub_index;
			libusb_free_device_list(devices, 1);
			return dRetval;

		}
	}

//	libusb_close(handle);

	libusb_free_device_list(devices, 1);
	return -1;
}

int usb_enable_HCE_device(uint8_t hub_index)
{
	libusb_device_handle *handle;
	libusb_device **devices;
	libusb_device *dev;
	libusb_device_descriptor desc;
	uint8_t port_list[7];


	int dRetval = 0;
	ssize_t devCnt = 0, port_cnt;
	ssize_t i = 0, j = 0;

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
				continue;
			}


			port_cnt = libusb_get_port_numbers(dev, port_list, 7);
                        if(port_cnt < 1)
                        {
                                DEBUGPRINT("Retrieving port numbers failed \n");
				libusb_close(handle);
                                continue;
                        }

			//It is comaparing agaist hub port count not HCE
			if(gasHubInfo[hub_index].port_max != (port_cnt))
                        {
                                DEBUGPRINT("Hub port match failed with Hub Index:%d\n",hub_index);
                                libusb_close(handle);
                                continue;

                        }

                        for(j = 0; j < gasHubInfo[hub_index].port_max; j++)
                        {
                                if(gasHubInfo[hub_index].port_list[j] != port_list[j])
                                {
                                        DEBUGPRINT("Hub port match failed \n");
                                        dRetval = -1;
                                        break;
                                }
                        }

                        if(dRetval == -1)
                        {
                        	libusb_close(handle);
				continue;
                        }

			printf("HCE capable hub found at Hub index=%d Path- ", hub_index);

                        for(i = 0; i < port_cnt; i++)
                        {
                                printf(":%d", (unsigned int)(port_list[i]));
                        }
                        printf("\n");


			if(libusb_kernel_driver_active(handle, 0) == 1)
			{

				if(libusb_detach_kernel_driver(handle, 0) != 0)
				{
					DEBUGPRINT("Cannot detach kerenl driver. USB device may not respond \n");
					libusb_close(handle);
					break;
				}
			}

			dRetval = libusb_claim_interface(handle, 0);

			if(dRetval < 0)
			{
				DEBUGPRINT("cannot claim intterface \n");
				libusb_close(handle);
				break;
			}

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
int Read_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest = 0x01;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	if(bRetVal< 0)
	{
		DEBUGPRINT("Read OTP failed %d\n",bRetVal);
		return bRetVal;
	}
	return bRetVal;
}
int Write_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest = 0x00;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_write_data (&UsbCtlPkt);
	if(bRetVal< 0)
	{
		DEBUGPRINT("Execute write OTP command failed %d\n",bRetVal);
		return bRetVal;
	}
	return bRetVal;
}
int xdata_read(HANDLE handle, uint16_t wAddress, uint8_t *data, uint8_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest 	= 0x04;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	return bRetVal;
}

int xdata_write(HANDLE handle, uint16_t wAddress, uint8_t *data, uint8_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest 	= 0x03;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_write_data (&UsbCtlPkt);
	return bRetVal;
}

bool UsbSetBitXdata(int hub_index,WORD wXDataAddress,BYTE byBitToSet)
{
	BYTE byData;
	bool bRet = false;
	do
	{
		bRet= MchpUsbRegisterRead ( hub_index, wXDataAddress, 1, &byData );
		if (!bRet)
		{
			break;
		}

		byData |= byBitToSet;
		bRet= MchpUsbRegisterWrite( hub_index, wXDataAddress, 1, &byData) ;
		if (!bRet)
		{
			break;
		}

	} while (FALSE);

	return bRet;
}

bool UsbClearBitXdata(int hub_index,WORD wXDataAddress,BYTE byBitToClear)
{
	BYTE byData;
	bool bRet = false;
	do
	{
		bRet= MchpUsbRegisterRead ( hub_index, wXDataAddress, 1, &byData );
		if (!bRet)
		{
			break;
		}

		byData &=~ byBitToClear;

		bRet= MchpUsbRegisterWrite( hub_index, wXDataAddress, 1, &byData) ;
		if (!bRet)
		{
			break;
		}

	} while (FALSE);

	return bRet;
}
int USB2530API_I2C_Enter_Exit_Passthrough(int hub_index, bool bEnable)
{
	int rc = 0;
	BYTE byCmd =0;
	if(bEnable)
	{
		byCmd = I2C_ENTER_PASSTHRG;
	}
	else
	{
		byCmd = I2C_EXIT_PASSTHRG;
	}
	rc= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle,0x41,byCmd,0x3131,0,0,0,20);
	return rc;
}

int USB_I2C_Transfer(int hub_index,BOOL byDir,uint8_t byAddr, uint8_t *pbyBuffer, uint16_t wLength , uint32_t *wdActualLength)
{
	BYTE byFlags, byCmd;
	int bRet = FALSE;

	WORD wAddress,temp = 0;

	do
	{
		if (wLength > 512)
		{
			break;
		}
		if (byDir)// I2c Read
		{
			byFlags = SEND_NACK | SEND_START |SEND_STOP;
		}
		else
		{
			byFlags = SEND_START |SEND_STOP;
		}
		byAddr = byAddr << 1;

		if (byDir)// I2c Read
		{
			byCmd = I2C_READ_CMD;
			byAddr |= I2C_READ_BIT;
			temp = byAddr;
			temp |= ((WORD)byFlags) << 8;

			wAddress = temp;
			bRet = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle,0xC1,byCmd,wAddress,0,&pbyBuffer[0],wLength,500);
			if(bRet <0)
			{
				DEBUGPRINT("I2C Read Failed \n");
			}
			else
			{
				DEBUGPRINT("I2C Read Success \n");
				*wdActualLength = wLength;
			}
		}
		else
		{
			byCmd = I2C_WRITE_CMD;
			temp = byAddr;
			temp |= ((WORD)byFlags) << 8;

			wAddress = temp;
			bRet = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle,0x41,byCmd,wAddress,0,&pbyBuffer[0],wLength,500);
			if(bRet <0)
			{
				DEBUGPRINT("I2C Write Failed \n");
			}
			else
			{
				DEBUGPRINT("I2C Write Success \n");
			}
		}
	}while(FALSE);
	return bRet;
}
BOOL Gen_I2C_Transfer (int hub_index, BOOL bDirection, BYTE* pbyBuffer, WORD wDataLen, BYTE bySlaveAddress,BOOL bStart,BOOL bStop,BOOL bNack)
{
	BYTE byFlags=0;
	WORD wAddress;
	BYTE byCmd, byRequestType;
	int rc = FALSE;
	do
	{
		if (wDataLen > 512)
		{
			DEBUGPRINT("Invalid Data length \n");
			break;
		}
		if (bStart)
		{
			byFlags |= I2CFL_SEND_START;
		}
		if (bStop)
		{
			byFlags |= I2CFL_SEND_STOP;
		}

		if (bNack)
		{
			byFlags |= I2CFL_SEND_NACK;
		}
		bySlaveAddress = bySlaveAddress << 1;
		if (bDirection)	// From device to USB host
		{
			bySlaveAddress	= bySlaveAddress | I2C_READ_BIT;
		}
		wAddress = MAKEWORD (bySlaveAddress, byFlags);
		if (bDirection)	// From device to USB host
		{
			byRequestType = 0xc1;
			byCmd		= CMD_I2C_READ;
		}
		else	// From USB to device
		{
			byRequestType = 0x41;
			byCmd		= CMD_I2C_WRITE;
		}
		rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle,byRequestType,byCmd,wAddress,0,
										pbyBuffer,wDataLen,CTRL_TIMEOUT);
		if(rc < 0)
		{
			DEBUGPRINT("I2C Transfer Failed \n");
			return FALSE;
		}

	}while(FALSE);
	return TRUE;
}
BOOL CalculateDeviceUARTRegisterSettings (uint8_t byMode, PUART_REGS pUARTRegs, uint32_t dwDesiredBaudRate)
{
	BYTE byTH1;
	WORD wS0REL;
	DWORD dwActualBaudRate = 0;
	ULONG ulDeviceErrorPercentage;
	BOOL bTestable = FALSE;
	DWORD dwCrystalFreq = CRYSTAL_FREQUENCY;

	switch (byMode)
	{
		case 0:
			dwActualBaudRate = CRYSTAL_FREQUENCY / 12;
			break;

		case 2:

			if (0 == pUARTRegs->bySMOD)
			{
				dwActualBaudRate = dwCrystalFreq / 64;
			}
			else
			{
				dwActualBaudRate = dwCrystalFreq / 32;
			}
			break;

		case 1:
		case 3:

			if (0 == pUARTRegs->byBD)
			{
				// When BD=0, Baud rate is based on Timer 1 overflow

				// To determine the value that must be placed in TH1 to generate a given baud rate,
				// use the following equation

				byTH1 = (BYTE ) (256 - (((DWORD) pow (2.0, pUARTRegs->bySMOD) * dwCrystalFreq) / ((DWORD) 384 * dwDesiredBaudRate)));
				dwActualBaudRate = ((DWORD)pow (2.0, pUARTRegs->bySMOD) * dwCrystalFreq) / ((DWORD)384 * (256 - byTH1));
				pUARTRegs->byTH1 = byTH1;
			}
			else
			{
				wS0REL = (BYTE )(1024 - (((DWORD)pow (2.0, pUARTRegs->bySMOD) * dwCrystalFreq) / ((DWORD)64 * dwDesiredBaudRate)));

				if (wS0REL > MAX_S0REL_VALUE)
				{
					break;
				}

				dwActualBaudRate = ((DWORD)pow (2.0, pUARTRegs->bySMOD) * dwCrystalFreq) / ((DWORD)64 * ((DWORD)1024 - wS0REL));
				pUARTRegs->wS0REL = wS0REL;
			}
			break;
	}

	do
	{
		if (dwDesiredBaudRate > dwActualBaudRate)
		{
			ulDeviceErrorPercentage = (ULONG) (100 * ((float) (dwDesiredBaudRate - dwActualBaudRate) / (float) dwDesiredBaudRate));
		}
		else
		{
			ulDeviceErrorPercentage = (ULONG) (100 * ((float) (dwActualBaudRate - dwDesiredBaudRate) / (float) dwDesiredBaudRate));
		}
		if (ulDeviceErrorPercentage > MAX_DEVICE_UART_ERROR_PERCENTAGE)
		{
			bTestable = FALSE;
			break;	// More than 2% error rate is not a good criteria to test & expect the same to pass !!!
		}

		// Now that the device error rate is < 2%, flag that we're ok to support this baud rate
		bTestable = TRUE;

	} while (FALSE);

	return bTestable;
}
BOOL SetDeviceUARTRegisters(int hub_index, UART_REGS *pUARTRegs)
{
	int bRet = FALSE;
	BOOL bReturn = FALSE;
	bRet = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle,0x41,UART_SET_REGS,0,0, (unsigned char *)pUARTRegs,
					sizeof(UART_REGS),500);

	if(bRet < 0)
	{
		DEBUGPRINT("Failed to set UART Register\n");
		bReturn = FALSE;
	}
	else
	{
		DEBUGPRINT("Set UART Register success \n");
		bReturn = TRUE;
	}
	return bReturn;
}
unsigned int CalculateNumberofOnes(unsigned int UINTVar)
{
	unsigned int N0OfOnes = 0;
	do
	{
		if(0x0000 == UINTVar) // variable if zero then return 0
			break;
		// Now counts 1's
		while(UINTVar)
		{
			N0OfOnes++;
			UINTVar &= (UINTVar -1);
		}
	}while(false);

	return N0OfOnes;
}

//Return handle to the first instance of VendorID &amp; ProductID matched device.
//VID and PID of Hub Feature Controller - by dealut vid:pid - 0x424:0x4940
//This API shouldn't use if there is multiple HUBS with HFC(vid:pid - 0x424:0x4940)
HANDLE  MchpUsbOpenHFC (UINT16 wVID, UINT16 wPID)
{
	int error = 0;
	libusb_device_handle *handle;

	error = libusb_init(&ctx);
	if(error < 0)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Initialization LibUSB failed\n");
		return -1;
	}


	handle = libusb_open_device_with_vid_pid(ctx, wVID, wPID);
	if(NULL == handle)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Failed to open the HFC vid:pid - 0x%04x:0x%04x\n", wVID, wPID);
		return -1;
	}

	/*Check if kenel driver attached*/
	if(libusb_kernel_driver_active(handle, 0))
   	{
		error = libusb_detach_kernel_driver(handle, 0); // detach driver

		if(0 > error)
		{
			DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: libusb_detach_kernel_driver failed\n");
			libusb_close(handle);
			return -1;
		}

   	}
   	error = libusb_claim_interface(handle, 0);
   	if(0 > error)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: libusb_claim_interface failed\n");
		libusb_close(handle);
		return -1;
	}

	gasHubInfo[0].handle = handle;
	gasHubInfo[0].byHubIndex = 0;

	return 0;
}
