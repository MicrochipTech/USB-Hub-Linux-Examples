/*
**********************************************************************************
* $File:  SpiFlash.cpp
* Description : File holds API for SPI Flash Write and Read
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

// Microchip Default VID and PID
#define MCHP_ROM_VID						0x0424
#define MCHP_ROM_PID						0x4504

//SPI
#define CMD_SPI_PASSTRHU_WRITE				0x61
#define CMD_SPI_BYTEFLASH_PGM				0x64
#define CMD_GET_HUB_INFO					0x09

#define SPI_READ_BUF_SIZE					0x400
#define SRAM_CODE_LOC						0x4800
#define SRAM_OVERLAY_REG_PAGE2				0x0844
#define USB2534_CALLFUNCTION_ADDR			0x8000
#define SPI_READ_BUF_LOCATION 				0x4c00
#define USB2534_CALLFUNCTION_PARAMS			0x4032
#define CHIP_ERASE_SST26VF064B				0xC7

#define JEDEC_ID							0x9f
#define	WRENABLE							0x06
#define	RDSR								0x05
#define	WRSR								0x01
#define ULBPR								0x98
#define	BLOCK_ERASE_64K						0xD8
#define CHIP_ERASE							0x60

#define MFR_ID_ATMEL						0x1F
#define MFR_ID_SST							0xBF
#define MFR_ID_WINBOND						0xEF
#define MFR_ID_ENDOFLIST					0xFF


#define BYTE_PROG_INDEX 					25
#define UPLOAD_FW_INDEX 					5
#define MAX_READ_SIZE						0x205

#define	WINDBOND_MEMORY_TYPE				0x30
#define	WINBOND_CAPACITY_1					0x11
#define	WINBOND_CAPACITY_2					0x12
#define	WINBOND_CAPACITY_3					0x13
#define	WINBOND_CAPACITY_4					0x14

#define AT25DF161_DEVICE_ID_1				0x46
#define AT25DF161_DEVICE_ID_2				0x02
#define AT26DF081A_DEVICE_ID_1				0x45
#define AT26DF081A_DEVICE_ID_2				0x01

// Device IDs of SST
#define	MEMORY_TYPE_SST_SPIFLASH 			0x25
#define MEMORY_TYPE_SST26_SPIFLASH  		0x26
#define DEVICE_ID_SST25VF064C	 			0x4B
#define DEVICE_ID_SST26VF064B	 			0x43
#define	DEVICE_ID_SST25VF020B	 			0x8C
#define DEVICE_ID_SST26VF016B	 			0x41

#define SCRATCHPAD_OFFSET_XDATA				0x4800
#define SPI_RESP_BUF_OFFSET					0x0210

#define SIZE_1KB							1024
#define SIZE_64KB							(64*1024)
#define SIZE_130KB							(130*1024)
#define ROM_A0								0x0108

//SPI Flash Opcodes
#define BYTE_PGM_OPCODE						0xAD
#define PAGE_PGM_OPCODE						0X02

#define CMD_CALL_TO_SRAM					0x05
#define MAP_TO_CODE_ADDR					0x8000
#define SRAM_PAGE2							0x844

#pragma pack(1)

typedef struct tagSPICmdInfo {
	BYTE bDirection;
	PBYTE pbyBuffer;
	WORD wDataLen;
	WORD wTotalLength;
} SPI_CMDINFO, *PSPI_CMDINFO;

const BYTE abySPISignEraseData[474] ={0x02, 0x81, 0xd7, 0x90, 0x08, 0x0a, 0xe0, 0x44, 0x04, 0xf0, 0x75, 0xf9, 0x10, 0xe0, 0x54, 0xfb,
0xf0, 0x90, 0x24, 0x08, 0x74, 0x06, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x01, 0xf0, 0x90, 0x24, 0x00,
0xe0, 0x44, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x12, 0x80, 0xea, 0x12, 0x81,
0x71, 0xef, 0x60, 0x38, 0x90, 0x24, 0x08, 0x74, 0x98, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x01, 0xf0,
0x90, 0x24, 0x00, 0xe0, 0x44, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x12, 0x80,
0xea, 0xe4, 0xff, 0x12, 0x81, 0x2e, 0x7f, 0x20, 0x12, 0x81, 0x2e, 0x7f, 0x40, 0x12, 0x81, 0x2e,
0x7f, 0x60, 0x12, 0x81, 0x2e, 0x7f, 0x80, 0x12, 0x81, 0x2e, 0x80, 0x61, 0x90, 0x24, 0x08, 0x74,
0x01, 0xf0, 0xe4, 0xa3, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x02, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x44,
0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x12, 0x80, 0xea, 0x90, 0x24, 0x08, 0x74,
0x06, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x44, 0x01, 0xf0, 0x90,
0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x12, 0x80, 0xea, 0x90, 0x24, 0x08, 0x74, 0xd8, 0xf0, 0xe4,
0xa3, 0xf0, 0xa3, 0xf0, 0xa3, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x04, 0xf0, 0x90, 0x24, 0x00, 0xe0,
0x44, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x12, 0x80, 0xea, 0x12, 0x81, 0xa2,
0x90, 0x40, 0x38, 0xe4, 0xf0, 0xa3, 0x74, 0x02, 0xf0, 0x90, 0x08, 0x04, 0xe0, 0x44, 0x04, 0xf0,
0x12, 0x81, 0xa2, 0x90, 0x08, 0x0a, 0x74, 0x40, 0xf0, 0x22, 0x90, 0x24, 0x08, 0x74, 0x05, 0xf0,
0xe4, 0xa3, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x02, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x44, 0x01, 0xf0,
0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x90, 0x24, 0x11, 0xe0, 0x30, 0xe0, 0x1f, 0x90, 0x24,
0x08, 0x74, 0x05, 0xf0, 0xe4, 0xa3, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x02, 0xf0, 0x90, 0x24, 0x00,
0xe0, 0x44, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x30, 0xe0, 0xdc, 0x80, 0xf7, 0x22, 0x90, 0x24,
0x08, 0x74, 0x06, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x44, 0x01,
0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x12, 0x80, 0xea, 0x90, 0x24, 0x08, 0x74, 0xd8,
0xf0, 0xe4, 0xa3, 0xf0, 0xa3, 0xef, 0xf0, 0xe4, 0xa3, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x04, 0xf0,
0x90, 0x24, 0x00, 0xe0, 0x44, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x02, 0x80,
0xea, 0x90, 0x24, 0x08, 0x74, 0x9f, 0xf0, 0x90, 0x24, 0x01, 0x74, 0x04, 0xf0, 0x90, 0x24, 0x00,
0xe0, 0x44, 0x01, 0xf0, 0x90, 0x24, 0x00, 0xe0, 0x20, 0xe0, 0xf9, 0x90, 0x24, 0x11, 0xe0, 0xb4,
0xbf, 0x0d, 0xa3, 0xe0, 0xb4, 0x26, 0x08, 0xa3, 0xe0, 0xb4, 0x43, 0x03, 0x7f, 0x01, 0x22, 0x7f,
0x00, 0x22, 0x7b, 0x78, 0x7a, 0xec, 0x7c, 0x03, 0x7d, 0xe8, 0xc2, 0xab, 0x53, 0x89, 0x8f, 0x43,
0x89, 0x10, 0xc2, 0x8e, 0xed, 0x1d, 0xae, 0x04, 0x70, 0x01, 0x1c, 0x4e, 0x60, 0x10, 0xc2, 0x8f,
0xaf, 0x03, 0xae, 0x02, 0x12, 0x81, 0xd1, 0xd2, 0x8e, 0x20, 0x8f, 0xe8, 0x80, 0xfb, 0xc2, 0x8f,
0x22, 0xef, 0xf5, 0x8b, 0x8e, 0x8d, 0x22, 0x02, 0x80, 0x03};

const BYTE abySPIUploadArea[] = {
 0x02 ,0x80 ,0xa6 ,0x90 ,0x40 ,0x3a ,0xe0 ,0xf5 ,0xf9 ,0x90 ,0x40 ,0x32 ,0xe0 ,0xfc ,0xa3 ,0xe0
,0xfd ,0xa3 ,0xe0 ,0xfe ,0xa3 ,0xe0 ,0xff ,0x90 ,0x40 ,0x3b ,0xe0 ,0xf8 ,0xa3 ,0xe0 ,0xf9 ,0xa3
,0xe0 ,0xfa ,0xa3 ,0xe0 ,0xfb ,0xc3 ,0xef ,0x9b ,0xff ,0xee ,0x9a ,0xfe ,0xed ,0x99 ,0xfd ,0xec
,0x98 ,0xfc ,0x90 ,0x40 ,0x32 ,0x12 ,0x80 ,0x9a ,0x90 ,0x40 ,0x32 ,0xa3 ,0xa3 ,0xe0 ,0xfe ,0xa3
,0xe0 ,0xfb ,0xaa ,0x06 ,0x90 ,0x40 ,0x32 ,0xa3 ,0xa3 ,0xe0 ,0xfe ,0xa3 ,0xe0 ,0xff ,0x90 ,0x40
,0x39 ,0xe0 ,0x2f ,0xff ,0x90 ,0x40 ,0x38 ,0xe0 ,0x3e ,0xfe ,0xc3 ,0xeb ,0x9f ,0xea ,0x9e ,0x50
,0x1f ,0x8b ,0x82 ,0x8a ,0x83 ,0xe4 ,0x93 ,0xff ,0x90 ,0x40 ,0x36 ,0xe4 ,0x75 ,0xf0 ,0x01 ,0x12
,0x80 ,0x84 ,0x85 ,0xf0 ,0x82 ,0xf5 ,0x83 ,0xef ,0xf0 ,0x0b ,0xbb ,0x00 ,0x01 ,0x0a ,0x80 ,0xc4
,0x75 ,0xf9 ,0x98 ,0x22 ,0xa3 ,0xf8 ,0xe0 ,0xc5 ,0xf0 ,0x25 ,0xf0 ,0xf0 ,0xe5 ,0x82 ,0x15 ,0x82
,0x70 ,0x02 ,0x15 ,0x83 ,0xe0 ,0xc8 ,0x38 ,0xf0 ,0xe8 ,0x22 ,0xec ,0xf0 ,0xa3 ,0xed ,0xf0 ,0xa3
,0xee ,0xf0 ,0xa3 ,0xef ,0xf0 ,0x22 ,0x02 ,0x80 ,0x03
};
/****************************************************
* Get_Hub_Info : Used to get Hub information
*					Internal Firmware Version
*					Device Revision
*					ASIC Type
*					Features Flag
*****************************************************/
int Get_Hub_Info(HANDLE handle, uint8_t *data)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest = 0x09;
	UsbCtlPkt.wValue 	= 0;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= 6;
	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	if(bRetVal< 0)
	{
		DEBUGPRINT("Execute HubInfo command failed %d\n",bRetVal);
		return bRetVal;
	}
	return bRetVal;
}
BOOL AthensReadSPIFlash(int DevID,BYTE *SPIUploadData, DWORD dstart_address, DWORD dend_address_ul)
{
	//find whether device boots from SPI or ROM
	Get_Hub_Info(DevID, (uint8_t *)&gasHubInfo[DevID].sHubInfo);

	//boots from SPI
	if(gasHubInfo[DevID].sHubInfo.byFeaturesFlag & 0x01)
	{
		if(FALSE == AthensReadSPICodeSPIROM(DevID, SPIUploadData,dstart_address, dend_address_ul))
		{
			DEBUGPRINT("Failed to Read SPI Flash \n");
			return FALSE;
		}
		else
		{
			DEBUGPRINT("Read SPI Flash success \n");
			return TRUE;
		}
	}
	else //boots from ROM
	{
		if(FALSE == AthensReadSPICodeIntROM(DevID,SPIUploadData,dstart_address,dend_address_ul))
		{
			DEBUGPRINT("Failed to Read SPI Flash \n");
			return FALSE;
		}
		else
		{
			DEBUGPRINT("Read SPI Flash success \n");
			return TRUE;
		}
	}
}
BOOL AthensReadSPICodeSPIROM(int hub_index,BYTE *SPIUploadData, DWORD dstart_address, DWORD dend_address_ul)
{
	DWORD i, j;
	BYTE Buf_Temp[13], abyRecBuf[SPI_READ_BUF_SIZE], byTemp[2],abyUploadData[SPI_READ_BUF_SIZE];
	WORD  wCallAddress=0;
	BOOL bRet = TRUE;
	int bRetVal = FALSE;

	do
	{
		if (dend_address_ul < dstart_address)
		{
			DEBUGPRINT("ReadSPIFlash : Start addr is greater than the end addr \n");
			bRet = FALSE;
			break;
		}
		memcpy(abyUploadData,abySPIUploadArea,sizeof(abySPIUploadArea));

		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0x41,0x03,SRAM_CODE_LOC, 0,(unsigned char *)&abyUploadData[0],
								sizeof(abySPIUploadArea), CTRL_TIMEOUT);

		if(bRetVal < 0)
		{
			DEBUGPRINT("SRAM Programming failed \n");
			bRet = FALSE;
			break;
		}


		memset (abyRecBuf, 0xCC, SPI_READ_BUF_SIZE);

		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0xC1,0x04,SRAM_CODE_LOC, 0,(unsigned char *)&abyRecBuf[0],
								sizeof(abySPIUploadArea), CTRL_TIMEOUT);


		if(bRetVal < 0)
		{
			DEBUGPRINT("SRAM Programming failed \n");
			bRet = FALSE;
			break;
		}
		if (!(0 == memcmp (abyRecBuf, abySPIUploadArea, bRetVal)))
		{
			DEBUGPRINT("SRAM Programming failed \n");
			bRet = FALSE;
			break;
		}

		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0xC1,0x04,SRAM_OVERLAY_REG_PAGE2, 0,(unsigned char*)&byTemp[0],
								2, CTRL_TIMEOUT);

		if(bRetVal < 0)
		{
			DEBUGPRINT("SRAM Programming failed \n");
			bRet = FALSE;
			break;
		}
		// write sram overlay registers
		Buf_Temp[0] = 0x10;
		Buf_Temp[1] = 0x02;

		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0x41,0x03,SRAM_OVERLAY_REG_PAGE2, 0,(unsigned char *)&Buf_Temp[0],
								2, CTRL_TIMEOUT);
		if(bRetVal < 0)
		{
			DEBUGPRINT("SRAM Programming failed \n");
			bRet = FALSE;
			break;
		}
		//set sram call address at 0x08000
		wCallAddress = USB2534_CALLFUNCTION_ADDR;

		for(i = dstart_address, j = 0; i < dend_address_ul; i+= SPI_READ_BUF_SIZE, j+= SPI_READ_BUF_SIZE)
		{
			Buf_Temp[0] = (BYTE)((i >> 24) & 0x00FF);
			Buf_Temp[1] = (BYTE)((i >> 16) & 0x00FF);
			Buf_Temp[2] = (BYTE)((i >> 8) & 0x00FF);
			Buf_Temp[3] = (BYTE)(i & 0x00FF); //spi address
			Buf_Temp[4] = 0x4c; // buffer location
			Buf_Temp[5] = 0x00;
			Buf_Temp[6] = 0x04; // size to be read (limited based on buffer size)
			Buf_Temp[7] = 0x00;
			Buf_Temp[8] = 0x98; // ROM_MAP1 value
			Buf_Temp[9] = 0x00;	// address offset in the mapped page
			Buf_Temp[10] = 0x00;
			Buf_Temp[11] = 0x00;
			Buf_Temp[12] = 0x00;

			if(i >= 0x8000)
			{
				if(i >= 0x10000)
				{
					if(i >= 0x18000)
					{
						// mapping SPI ROM page 0x18000 to 0x1ffff to rom c:0 to c:0x7FFF
						Buf_Temp[8] = 0xFE;
						Buf_Temp[9] = 0x00;
						Buf_Temp[10] = 0x01;
						Buf_Temp[11] = 0x80;
						Buf_Temp[12] = 0x00;
					}
					else
					{
						// mapping SPI ROM page 0x10000 to 0x17fff to rom c:0 to c:0x7FFF
						Buf_Temp[8] = 0xDC;
						Buf_Temp[9] = 0x00;
						Buf_Temp[10] = 0x01;
						Buf_Temp[11] = 0x00;
						Buf_Temp[12] = 0x00;
					}
				}
				else
				{
					// mapping SPI ROM page 0x08000 to 0x0ffff to rom c:0 to c:0x7FFF
					Buf_Temp[8] = 0xBA;
					Buf_Temp[9] = 0x00;
					Buf_Temp[10] = 0x00;
					Buf_Temp[11] = 0x80;
					Buf_Temp[12] = 0x00;
				}
			}
			// write parameters for the sram call fnc
			bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0x41,0x03, USB2534_CALLFUNCTION_PARAMS,
								0, (unsigned char *)&Buf_Temp[0],13, CTRL_TIMEOUT);

			if(bRetVal < 0)
			{
				DEBUGPRINT("ReadBack failed \n");
				bRet = FALSE;
				break;
			}

			// call cmd_call_to_sram
			bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0x41,0x05, wCallAddress,
								0, 0,0, CTRL_TIMEOUT);

			sleep(20);

			memset (abyRecBuf, 0xCC, SPI_READ_BUF_SIZE);

			bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0xC1,0x04, SPI_READ_BUF_LOCATION,
								0, (unsigned char*)&abyRecBuf[0],SPI_READ_BUF_SIZE, CTRL_TIMEOUT);

			if(bRetVal < 0)
			{
				DEBUGPRINT("ReadBack failed \n");
				bRet = FALSE;
				break;
			}
			memcpy(&SPIUploadData[j], &abyRecBuf[0], SPI_READ_BUF_SIZE);
		}

		//Restore the sram mapping
		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_index].handle, 0x41,0x03, SRAM_OVERLAY_REG_PAGE2,
								0, (unsigned char *)&byTemp[0],2, CTRL_TIMEOUT);

		if(bRetVal < 0)
		{
			DEBUGPRINT("Restore SRAM Mapping failed \n");
			bRet = FALSE;
			break;
		}

	}while(FALSE);
	return bRet;
}
BOOL AthensReadSPICodeIntROM(int hub_id,BYTE *SPIUploadData, DWORD dstart_address, DWORD dend_address_ul)
{
	DWORD j =0, i = 0;
	BYTE abyBuffer_read [SIZE_1KB];
	int m = 0;
	DWORD error = Error_Success;
	char szBuffer [260];
	int rc = FALSE;
	BYTE abyBuffer1 [1] = {JEDEC_ID};	//JEDEC ID

	BYTE abyBuffer2 [1] = {WRENABLE};		//WREN

	BYTE abyBuffer4 [2] = {WRSR, 0x00};		// WRSR Unprotect all
	BYTE abyBuffer4_SST26VF064B [1] = {ULBPR};		// ULBPR Unprotect all
	BYTE abyBuffer7 [5] = {0x3B, 0x00, 0x00, 0x00, 0x00};
	BOOL bRet = TRUE;

  	char szCompareStringSST26VF064B[16] = "SST26VF064B";
	SPI_CMDINFO gasSPICmdList_Upload [] =
	{

			// DIR,	BUFFER,		DATALEN, TOTLALENGTH

			//READ JEDEC ID
			{0,	abyBuffer1,	sizeof (abyBuffer1), 4},
			{1, abyBuffer_read, 4, 4 },

			//WREN
			{0, abyBuffer2, sizeof (abyBuffer2), 1},
			{1, abyBuffer_read, 1, 1 },

			//WRSR
			{0, abyBuffer4, sizeof (abyBuffer4), 1},

			//PAGE Read
			{0, abyBuffer7, sizeof (abyBuffer7), 0x0205},

			//XDATA read response
			{1, abyBuffer_read, sizeof (abyBuffer_read), 0x0205 },

			{0xFF, 0, 0, 0}
	};
	// SPI upload command sequence for SST26VF064B
	SPI_CMDINFO gasSPICmdList_Upload_SST26VF064B [] =
	{

			// DIR,	BUFFER,		DATALEN, TOTLALENGTH

			//READ JEDEC ID
			{0,	abyBuffer1,	sizeof (abyBuffer1), 4},
			{1, abyBuffer_read, 4, 4 },

			//WREN
			{0, abyBuffer2, sizeof (abyBuffer2), 1},
			{1, abyBuffer_read, 1, 1 },

			//ULBPR
			{0, abyBuffer4_SST26VF064B, sizeof (abyBuffer4_SST26VF064B), 1},

			//PAGE Read
			{0, abyBuffer7, sizeof (abyBuffer7), 0x0205},

			//XDATA read response
			{1, abyBuffer_read, sizeof (abyBuffer_read), 0x0205 },

			{0xFF, 0 , 0, 0}
	};
	do
	{
		if (dend_address_ul < dstart_address)
		{
			DEBUGPRINT("End Address Greater than Start Address \n");
			bRet = FALSE;
			break;
		}
		SPI_CMDINFO *gpSPICmdList_Upload = NULL;
		if (FALSE == MchpUsbSpiSetConfig ( hub_id, TRUE))
		{
			DEBUGPRINT("SPI pass thru enter failed \n");
			bRet = FALSE;
		}
		error = SPI_GetManufactureID(hub_id,&szBuffer[0],TRUE);

		if(error != 0x0000)
		{
			if(error == SpiNoDevice)
			{
				DEBUGPRINT("No SPI Flash Present \n");
				return FALSE;
			}
			else if(error == SpiFlashWrongDeviceID)
			{
				DEBUGPRINT("Unsupported SPI Interface/ No Flash Present \n");
				return FALSE;
			}
			break;
		}
		else
		{
			if(!memcmp (&szCompareStringSST26VF064B[0] , &szBuffer[0], 0x07))
			{
				gpSPICmdList_Upload = gasSPICmdList_Upload_SST26VF064B;
			}
			else
			{
				gpSPICmdList_Upload = gasSPICmdList_Upload;
			}
		}
		for (m = 0; gpSPICmdList_Upload [m].bDirection != 0xFF; m++)
		{

			if(m != UPLOAD_FW_INDEX)
			{
				if( gpSPICmdList_Upload [m].bDirection) //Read
				{
					rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList_Upload [m].pbyBuffer,
									(uint16_t)gpSPICmdList_Upload [m].wTotalLength,CTRL_TIMEOUT);
				}
				else //Write
				{
					rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
						(uint16_t)gpSPICmdList_Upload [m].wTotalLength,0,gpSPICmdList_Upload [m].pbyBuffer,
									(uint16_t)gpSPICmdList_Upload [m].wDataLen,CTRL_TIMEOUT);
				}

				// 0x05 is the command for RDSR; Once RDSR is sent the SPI bitwell bit has to be checked which
				// located at buffer[1] bit0 which is the BUSY bit

				if(RDSR == gpSPICmdList_Upload [m].pbyBuffer[0])
				{
					if( gpSPICmdList_Upload [m].bDirection) //Read
					{
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList_Upload [m].pbyBuffer,
									(uint16_t)gpSPICmdList_Upload [m].wTotalLength,CTRL_TIMEOUT);
					}
					else //Write
					{
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
						(uint16_t)gpSPICmdList_Upload [m].wTotalLength,0,gpSPICmdList_Upload [m].pbyBuffer,
									(uint16_t)gpSPICmdList_Upload [m].wDataLen,CTRL_TIMEOUT);
					}
					rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,
									(uint16_t)gpSPICmdList_Upload [m].wTotalLength,CTRL_TIMEOUT);

					while (abyBuffer_read [1] & 0x01)
					{
						if( gpSPICmdList_Upload [m].bDirection) //Read
						{
							rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
								(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList_Upload [m].pbyBuffer,
									(uint16_t)gpSPICmdList_Upload [m].wTotalLength,CTRL_TIMEOUT);
						}
						else //Write
						{
							rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0x41,
								 CMD_SPI_PASSTRHU_WRITE,(uint16_t)gpSPICmdList_Upload [m].wTotalLength,0,
									gpSPICmdList_Upload [m].pbyBuffer,(uint16_t)gpSPICmdList_Upload [m].wDataLen,CTRL_TIMEOUT);
						}
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
								(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,
									(uint16_t)gpSPICmdList_Upload [m].wTotalLength,CTRL_TIMEOUT);
					}
				}
			}
			else
			{

				for(i = dstart_address, j = 0; i < (dstart_address + (dend_address_ul - dstart_address)); i+= 512, j+= 512)
				{

					gpSPICmdList_Upload [UPLOAD_FW_INDEX].pbyBuffer[0] = 0x0B;

					gpSPICmdList_Upload [UPLOAD_FW_INDEX].pbyBuffer[1] = (BYTE)(i>>16);

					gpSPICmdList_Upload [UPLOAD_FW_INDEX].pbyBuffer[2] = HIBYTE((WORD)i);

					gpSPICmdList_Upload [UPLOAD_FW_INDEX].pbyBuffer[3] = LOBYTE((WORD)i);

					if( gpSPICmdList_Upload [UPLOAD_FW_INDEX].bDirection) //Read
					{
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
							(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList_Upload [UPLOAD_FW_INDEX].pbyBuffer,
							(uint16_t)gpSPICmdList_Upload [UPLOAD_FW_INDEX].wTotalLength,CTRL_TIMEOUT);
					}
					else //Write
					{
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
						(uint16_t)gpSPICmdList_Upload [UPLOAD_FW_INDEX].wTotalLength,0,
						gpSPICmdList_Upload [UPLOAD_FW_INDEX].pbyBuffer,
						(uint16_t)gpSPICmdList_Upload [UPLOAD_FW_INDEX].wDataLen,CTRL_TIMEOUT);
					}

					//xdata read back
					rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[hub_id].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,MAX_READ_SIZE,CTRL_TIMEOUT);

					// 5 Bytes header is subtracting as it will be junk in the response
					memcpy(&SPIUploadData[j], &abyBuffer_read[5], (MAX_READ_SIZE - 0x005));

				}
			}
			if(rc < 0)
			{
				DEBUGPRINT("SPI Pass Through write failed \n");
				bRet = FALSE;
				break;
			}
		}
	}while(FALSE);
	return bRet;
}

BOOL WriteSPIFlash(HANDLE DevID,BYTE *byWriteData,WORD iRestartDelay,BOOL bErasePseudoOTP,DWORD dwStartAddr,DWORD dwSize )
{
	BOOL bReturn = FALSE;
	char szBuffer [260];
	DWORD error;
	HANDLE hDevice =  INVALID_HANDLE_VALUE;
	//find whether device boots from SPI or ROM
	Get_Hub_Info(DevID, (uint8_t *)&gasHubInfo[DevID].sHubInfo);

	//boots from SPI
	if(gasHubInfo[DevID].sHubInfo.byFeaturesFlag & 0x01)
	{
		DEBUGPRINT("Code is running from SPI \n");
		error = SPI_SignatureErase(DevID);
		if(error != Error_Success)
		{
			DEBUGPRINT("SPI Erase Failed \n");
			return bReturn;
		}
		/// to be determined
		bReturn = MchpUsbClose(DevID);
		sleep(iRestartDelay);
		hDevice = MchpUsbOpenID(MCHP_ROM_VID, MCHP_ROM_PID);
		DevID = hDevice;

	}
	//Pass thru enter
	if (FALSE == MchpUsbSpiSetConfig ( DevID, TRUE))
	{
		DEBUGPRINT("SPI pass thru enter failed \n");
		return FALSE;
	}
	error = SPI_GetManufactureID(DevID,&szBuffer[0], TRUE);
	if(error != 0x0000)
	{
		if(error == SpiNoDevice)
		{
			DEBUGPRINT("No SPI Flash Present \n");
			return FALSE;
		}
		else if(error == SpiFlashWrongDeviceID)
		{
			DEBUGPRINT("Unsupported SPI Interface/ No Flash Present \n");
			return FALSE;
		}

	}
	if(FALSE == MchpUsbSpiSetConfig ( DevID, FALSE))
	{
		DEBUGPRINT("SPI pass thru exit failed \n");
		return FALSE;
	}
	error = LoadBinary(DevID, bErasePseudoOTP?true:false,byWriteData, dwStartAddr, dwSize);

	if(error != Error_Success)
	{
		DEBUGPRINT("SPI Flash Write Failed %x \n",(unsigned int)error);
		bReturn = FALSE;
	}
	else
	{
		DEBUGPRINT("SPI Flash Programming Pass \n");
		bReturn = TRUE;
	}
	return bReturn;
}
DWORD SPI_GetManufactureID (int DevID, char *szBuffer, BOOL bSupportAll)
{
	BYTE abyjedecid_cmd[]={JEDEC_ID},abyjedecid_rsp[10],i;
	int rc = FALSE;
	DWORD error = Error_Success;

	typedef struct tagJEDECID
	{
		BYTE byManufacturer;
		BYTE byMemoryType;
		BYTE byDeviceId;
		char szPartNumber [20];

		BYTE byValidationMask;

	} JEDECID;


	JEDECID asJedecId [] = {
		{MFR_ID_ATMEL, AT26DF081A_DEVICE_ID_1, AT26DF081A_DEVICE_ID_2, "AT26DF081A", 0x07},
		{MFR_ID_ATMEL, AT25DF161_DEVICE_ID_1, AT25DF161_DEVICE_ID_2, "AT25DF161", 0x07},

		{MFR_ID_WINBOND, WINDBOND_MEMORY_TYPE, WINBOND_CAPACITY_1, "W25X10", 0x07},
		{MFR_ID_WINBOND, WINDBOND_MEMORY_TYPE, WINBOND_CAPACITY_2, "W25X20", 0x07},
		{MFR_ID_WINBOND, WINDBOND_MEMORY_TYPE, WINBOND_CAPACITY_3, "W25X30", 0x07},
		{MFR_ID_WINBOND, WINDBOND_MEMORY_TYPE, WINBOND_CAPACITY_4, "W25X40", 0x07},

		{MFR_ID_SST, MEMORY_TYPE_SST_SPIFLASH, DEVICE_ID_SST25VF064C, "SST25VF064C", 0x07},
		{MFR_ID_SST, MEMORY_TYPE_SST26_SPIFLASH, DEVICE_ID_SST26VF064B, "SST26VFXXXX", 0x07},
		{MFR_ID_SST, MEMORY_TYPE_SST_SPIFLASH, DEVICE_ID_SST25VF020B, "SST25VF020B", 0x07},
		{MFR_ID_ENDOFLIST, 0xFF, 0xFF, "Unsupported Flash", 0xFF}
	};
	do
	{
		//read jedec id command via pass thru write
		rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle, 0x41,0x61,4,0, &abyjedecid_cmd[0],1,CTRL_TIMEOUT);
		if(rc < 0)
		{
			DEBUGPRINT("Failed to Write \n");
			break;
		}
		// read the jedec id from the response buffer using paa thru read
		rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle, 0xC1,0x04,(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,
						&abyjedecid_rsp[0],4,CTRL_TIMEOUT);
		if(rc < 0)
		{
			DEBUGPRINT("Failed to Read \n");
			break;
		}

		// byte 0 in this buffer is response for opcode 0x9F. So ignore.
		for (i=0; asJedecId [i].byManufacturer != MFR_ID_ENDOFLIST; i++)
		{
			if (asJedecId [i].byManufacturer != abyjedecid_rsp [1])
			{
				continue;
			}

			if (asJedecId [i].byValidationMask & 0x02)
			{
				if (asJedecId [i].byMemoryType != abyjedecid_rsp [2])
				{
					continue;
				}
			}
			if (asJedecId [i].byValidationMask & 0x04)
			{
				//Skip device id checking for SST flash , so that we can apply common commands

				if ((asJedecId [i].byManufacturer == MFR_ID_SST) && (asJedecId [i].byMemoryType == MEMORY_TYPE_SST26_SPIFLASH))
				{
					sprintf(szBuffer,"%s",asJedecId [i].szPartNumber);
					DEBUGPRINT("SPI flash chip identified as %s\n", asJedecId [i].szPartNumber);
					break;
				}

				if (asJedecId [i].byDeviceId != abyjedecid_rsp [3])
				{
					continue;
				}
			}
			/*
			* Here, assuming ganufacturer id string no longer than MAX path but
			* better to pass the size as parameter
			*/
			sprintf(szBuffer,"%s",asJedecId [i].szPartNumber);
			DEBUGPRINT ("SPI flash chip identified as %s\n", asJedecId [i].szPartNumber);
			break;

		}
		if (MFR_ID_ENDOFLIST == asJedecId [i].byManufacturer)
		{
			if((abyjedecid_rsp[1] == 0) & (abyjedecid_rsp[2] == 0) & (abyjedecid_rsp[3] == 0))
			{
				error = SpiNoDevice;
				DEBUGPRINT( "SPI Flash is absent \n");
				break;
			}
			if(bSupportAll)
			{
				error = Error_Success;
				sprintf(szBuffer,"JEDEC ID read as %02X, %02X, %02X\n", abyjedecid_rsp [1], abyjedecid_rsp[2], abyjedecid_rsp[3]);
				DEBUGPRINT("%s",szBuffer);
				break;
			}
			else
			{
				error = SpiFlashWrongDeviceID;
				sprintf(szBuffer,"SPI flash chip identified as unsupported, JEDEC ID read as %02X, %02X, %02X", abyjedecid_rsp [1], abyjedecid_rsp[2], 							abyjedecid_rsp[3]);
				DEBUGPRINT("%s",szBuffer);
				break;
			}
		}
	}while(FALSE);
	return error;
}
DWORD LoadBinary (int DevID,bool bErasePgmArea, BYTE *abyBuffer_SPI, DWORD dstart_address, DWORD dwActualSize)
{
	DWORD error = Error_Success;
	int rc = FALSE;

	DWORD j =0, i = 0, MAX_DOWNLOAD_SIZE;
	WORD wSize = 0;
	BYTE SPIUploadData_verify [SIZE_130KB] , abyBuffer_read [SIZE_1KB];
	int m = 0;
	BOOL bBytepgmSupported = FALSE;
	BOOL bFlashsst26vf064b = 0;

	char szFlashType[260], szCompareStringSST26VF064B[16] = "SST26VF064B";

	// Intialize before every download; as last state would be false
	BOOL bCheck_flag = TRUE;

	BYTE abyBuffer_byte_prog[256 + 4] = {0x02, 0x00, 0x00, 0x00}; //BYTE PROG
	BYTE abyBuffer1 [1] = {JEDEC_ID};		//JEDEC ID read
	BYTE abyBuffer2 [1] = {WRENABLE};		//WREN
	BYTE abyBuffer3 [2] = {RDSR, 0x00};		//RDSR
	BYTE abyBuffer4 [2] = {WRSR, 0x00};		// WRSR Unprotect all
	BYTE abyBuffer4_SST26VF064B [1] = {ULBPR};		// ULBPR Unprotect all
	BYTE abyBuffer5 [4] = {0x00, 0x00, 0x00, 0x00};	// 64k block erase
	BYTE abyBuffer5a [4]= {0x00, 0x00, 0x00, 0x00};	 // 64k block erase
	BYTE abyBuffer5_SST26VF064B [4]= {0x00, 0x00, 0x00, 0x00};	// 1st 8k block erase for SST26VF064B
	BYTE abyBuffer5_SST26VF064B_1 [4]= {0x00, 0x00, 0x00, 0x00}; // 2nd 8k block erase for SST26VF064B
	BYTE abyBuffer5_SST26VF064B_2 [4]= {0x00, 0x00, 0x00, 0x00}; // 3rd 8k block erase for SST26VF064B
	BYTE abyBuffer5_SST26VF064B_3 [4]= {0x00, 0x00, 0x00, 0x00}; // 4th  8k block erase for SST26VF064B
	BYTE abyBuffer5_SST26VF064B_4 [4]= {0x00, 0x00, 0x00, 0x00}; // 1st 32k block erase for SST26VF064B
	BYTE abyBuffer5_SST26VF064B_5 [4]= {0x00, 0x00, 0x00, 0x00}; // 1st 64k block erase for SST26VF064B

	//select which 64KB block to be erased if the sector is less than 128KB to optimize time
	if(dstart_address > 0x0FFFF)
	{
		if((dwActualSize + dstart_address) > SIZE_64KB)
		{
			abyBuffer5a[0] = BLOCK_ERASE_64K;
			abyBuffer5a[1] = 0x01;

			// erase 1st 64k block
			abyBuffer5_SST26VF064B_5[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_5[1] = 0x01;
			abyBuffer5_SST26VF064B_5[2] = 0x00;
		}
		else
		{
			abyBuffer5a[0] = BLOCK_ERASE_64K;
			abyBuffer5a[1] = 0x01;

			// erase 1st 64k block
			abyBuffer5_SST26VF064B_5[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_5[1] = 0x01;
			abyBuffer5_SST26VF064B_5[2] = 0x00;
		}
	}
	else
	{
		if((dwActualSize + dstart_address) > SIZE_64KB)
		{
			abyBuffer5a[0] = BLOCK_ERASE_64K;
			abyBuffer5a[1] = 0x01;

			abyBuffer5[0] = BLOCK_ERASE_64K;
			abyBuffer5[1] = 0x00;

			// erase 1st 8k block
			abyBuffer5_SST26VF064B[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B[1] = 0x00;
			abyBuffer5_SST26VF064B[2] = 0x00;

			// erase 2nd 8k block
			abyBuffer5_SST26VF064B_1[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_1[1] = 0x00;
			abyBuffer5_SST26VF064B_1[2] = 0x20;

			// erase 3rd 8k block
			abyBuffer5_SST26VF064B_2[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_2[1] = 0x00;
			abyBuffer5_SST26VF064B_2[2] = 0x40;

			// erase 4th 8k block
			abyBuffer5_SST26VF064B_3[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_3[1] = 0x00;
			abyBuffer5_SST26VF064B_3[2] = 0x60;

			// erase 1st 32k block
			abyBuffer5_SST26VF064B_4[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_4[1] = 0x00;
			abyBuffer5_SST26VF064B_4[2] = 0x80;

			// erase 1st 64k block
			abyBuffer5_SST26VF064B_5[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_5[1] = 0x01;
			abyBuffer5_SST26VF064B_5[2] = 0x00;

		}
		else
		{
			//Erase configuration data
			if (bErasePgmArea)
			{
				abyBuffer5a[0] = BLOCK_ERASE_64K;
				abyBuffer5a[1] = 0x01;

				// erase 1st 64k block
				abyBuffer5_SST26VF064B_5[0] = BLOCK_ERASE_64K;
				abyBuffer5_SST26VF064B_5[1] = 0x01;
				abyBuffer5_SST26VF064B_5[2] = 0x00;
			}


			abyBuffer5[0] = BLOCK_ERASE_64K;
			abyBuffer5[1] = 0x00;

			// erase 1st 8k block
			abyBuffer5_SST26VF064B[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B[1] = 0x00;
			abyBuffer5_SST26VF064B[2] = 0x00;

			// erase 2nd 8k block
			abyBuffer5_SST26VF064B_1[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_1[1] = 0x00;
			abyBuffer5_SST26VF064B_1[2] = 0x20;

			// erase 3rd 8k block
			abyBuffer5_SST26VF064B_2[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_2[1] = 0x00;
			abyBuffer5_SST26VF064B_2[2] = 0x40;

			// erase 4th 8k block
			abyBuffer5_SST26VF064B_3[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_3[1] = 0x00;
			abyBuffer5_SST26VF064B_3[2] = 0x60;

			// erase 1st 32k block
			abyBuffer5_SST26VF064B_4[0] = BLOCK_ERASE_64K;
			abyBuffer5_SST26VF064B_4[1] = 0x00;
			abyBuffer5_SST26VF064B_4[2] = 0x80;
		}
	}
	//perform chip erase if the sector is beyond 128KB
	if((dwActualSize + dstart_address) > (MAX_FW_SIZE))
	{
		    abyBuffer5_SST26VF064B[0] = CHIP_ERASE_SST26VF064B;

		    abyBuffer5[0] = CHIP_ERASE;
	}
	//SPI programming command sequence
	SPI_CMDINFO gasSPICmdList [] =
	{

		// DIR,	BUFFER,		DATALEN, TOTLALENGTH

		//READ JEDEC ID
		{0,	abyBuffer1,	sizeof (abyBuffer1), 4},
		{1, abyBuffer_read, 4, 4 },

		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//WRSR
		{0, abyBuffer4, sizeof (abyBuffer4), 2},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK1 ERASE
		{0,	abyBuffer5,	sizeof (abyBuffer5),sizeof (abyBuffer5) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK2 ERASE
		{0,	abyBuffer5a,sizeof (abyBuffer5a),sizeof (abyBuffer5a) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		// PAGE PROGRAM
		{0,	abyBuffer_byte_prog,	sizeof (abyBuffer_byte_prog),sizeof (abyBuffer_byte_prog)},

		// RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 3},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		{0xff, 0 , 0, 0}
	};
	//SPI programming command sequence for SST26VF064B
	SPI_CMDINFO gasSPICmdList_SST26VF064B [] =
	{

		// DIR,	BUFFER,		DATALEN, TOTLALENGTH

		//READ JEDEC ID
		{0,	abyBuffer1,	sizeof (abyBuffer1), 4},
		{1, abyBuffer_read, 4, 4 },

		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//ULBPR
		{0, abyBuffer4_SST26VF064B, sizeof (abyBuffer4_SST26VF064B), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },



		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK1 8k ERASE
		{0,	abyBuffer5_SST26VF064B,	sizeof (abyBuffer5_SST26VF064B),sizeof (abyBuffer5_SST26VF064B) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },


		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK2 8k ERASE
		{0,	abyBuffer5_SST26VF064B_1,	sizeof (abyBuffer5_SST26VF064B_1),sizeof (abyBuffer5_SST26VF064B_1) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },


		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK3 8k ERASE
		{0,	abyBuffer5_SST26VF064B_2,	sizeof (abyBuffer5_SST26VF064B_2),sizeof (abyBuffer5_SST26VF064B_2) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },


		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK4 8k ERASE
		{0,	abyBuffer5_SST26VF064B_3,	sizeof (abyBuffer5_SST26VF064B_3),sizeof (abyBuffer5_SST26VF064B_3) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },


		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK1 32k ERASE
		{0,	abyBuffer5_SST26VF064B_4,	sizeof (abyBuffer5_SST26VF064B_4),sizeof (abyBuffer5_SST26VF064B_4) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },


		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//BLK1 64k ERASE
		{0,	abyBuffer5_SST26VF064B_5,sizeof (abyBuffer5_SST26VF064B_5),sizeof (abyBuffer5_SST26VF064B_5) },

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },


		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		//WREN
		{0, abyBuffer2, sizeof (abyBuffer2), 1},

		//RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 2},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		// PAGE PROGRAM
		{0,	abyBuffer_byte_prog,	sizeof (abyBuffer_byte_prog),sizeof (abyBuffer_byte_prog)},

		// RDSR
		{0, abyBuffer3, sizeof (abyBuffer3), 3},
		{1, abyBuffer_read, sizeof (abyBuffer_read), 2 },

		{0xff, 0 , 0, 0}
	};
	do
	{
		error = Error_Success;
		//Pass thru enter
		if (FALSE == MchpUsbSpiSetConfig ( DevID, TRUE))
		{
			DEBUGPRINT("SPI pass thru enter failed \n");
			error = SpiPassThruEnterFailed;
			break;
		}
		rc =Get_Hub_Info(DevID, (uint8_t *)&gasHubInfo[DevID].sHubInfo);
		if (ROM_A0 == gasHubInfo[DevID].sHubInfo.wInternalFWRevision)
		{
			MAX_DOWNLOAD_SIZE = 128;
		}
		else
		{
			MAX_DOWNLOAD_SIZE = 256;
		}
		SPI_CMDINFO *gpSPICmdList = NULL;

		if (Error_Success == (error = SPI_GetManufactureID (DevID,&szFlashType[0], 1)))
		{
			if(!memcmp (&szCompareStringSST26VF064B[0] , &szFlashType[0], 0x07))
			{
				gpSPICmdList = gasSPICmdList_SST26VF064B;
				bFlashsst26vf064b = 1;
			}
			else
			{
				gpSPICmdList = gasSPICmdList;
			}

		}
		else
		{
			break;
		}
		for (m = 0; gpSPICmdList [m].bDirection != 0xFF; m++)
		{
			//m == 25 is the BYTE programme command and it requires n iterations depending on the file size
			// remember to change the byte prog index #define whenever there is any change in the prog sequence array
			if(((bFlashsst26vf064b == 0) && (m != BYTE_PROG_INDEX))
				|| ((bFlashsst26vf064b == 1) && (m != 49)))
			{
				//chip erase command - modifiy length to 1
				if((CHIP_ERASE == gpSPICmdList [m].pbyBuffer[0]) || (CHIP_ERASE_SST26VF064B == gpSPICmdList [m].pbyBuffer[0]))
				{
					gpSPICmdList [m].wDataLen = 1;
					gpSPICmdList [m].wTotalLength = 1;
				}
				if(gpSPICmdList [m].bDirection) //Read
				{
					rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList [m].pbyBuffer,
						(uint16_t)gpSPICmdList [m].wTotalLength,CTRL_TIMEOUT);
				}
				else //Write
				{
					rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
						(uint16_t)gpSPICmdList [m].wTotalLength,0,gpSPICmdList [m].pbyBuffer,(uint16_t)gpSPICmdList [m].wDataLen,CTRL_TIMEOUT);
				}

				// 0x05 is the command for RDSR; Once RDSR is sent the SPI bitwell bit has to be polled which
				// located at buffer[1] bit0 which is the BUSY bit

				if(RDSR == gpSPICmdList [m].pbyBuffer[0])
				{
					if(gpSPICmdList [m].bDirection) //Read
					{
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
							(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList [m].pbyBuffer,
							(uint16_t)gpSPICmdList [m].wTotalLength,CTRL_TIMEOUT);
					}
					else //Write
					{
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
							(uint16_t)gpSPICmdList [m].wTotalLength,0,gpSPICmdList [m].pbyBuffer,
										(uint16_t)gpSPICmdList [m].wDataLen,CTRL_TIMEOUT);
					}

					// Read response for RDSR sent earlier
					rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,
						(uint16_t)gpSPICmdList [m].wTotalLength,CTRL_TIMEOUT);

					//wait until BUSY bit is cleared
					while (abyBuffer_read [1] & 0x01)
					{
						if(gpSPICmdList [m].bDirection) //Read
						{
							rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
								(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList [m].pbyBuffer,
											(uint16_t)gpSPICmdList [m].wTotalLength,CTRL_TIMEOUT);
						}
						else //Write
						{
							rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
								(uint16_t)gpSPICmdList [m].wTotalLength,0,gpSPICmdList [m].pbyBuffer,
											(uint16_t)gpSPICmdList [m].wDataLen,CTRL_TIMEOUT);
						}

						// Read response for RDSR sent earlier
						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
							(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,
											(uint16_t)gpSPICmdList [m].wTotalLength,CTRL_TIMEOUT);
					}
				}
			}
			else
			{
				for(i = dstart_address; TRUE == bCheck_flag; i+= (MAX_DOWNLOAD_SIZE))
				{
					if(i < (dwActualSize + dstart_address))
					{
						bCheck_flag = TRUE;
						j = i;
						wSize = (WORD)MAX_DOWNLOAD_SIZE;
						gpSPICmdList [m].wDataLen = wSize + 4;
						gpSPICmdList [m].wTotalLength = wSize + 4;

					}

					else
					{
						DWORD dwcond = i- (dwActualSize + dstart_address);
						if((dwcond <MAX_DOWNLOAD_SIZE) && (dwcond != 0))
						{
							bCheck_flag = TRUE;
							wSize = (WORD)(i- (dwActualSize + dstart_address));
							j = (i- MAX_DOWNLOAD_SIZE) + (i- (dwActualSize + dstart_address));
							gpSPICmdList [m].wDataLen = wSize + 4;
							gpSPICmdList [m].wTotalLength = wSize + 4;
						}

						else
							bCheck_flag = FALSE;
					}
					if(bCheck_flag)
					{
						abyBuffer_byte_prog[0] = bBytepgmSupported? BYTE_PGM_OPCODE: PAGE_PGM_OPCODE;
						abyBuffer_byte_prog[1] = LOBYTE(HIWORD(j));
						abyBuffer_byte_prog[2] = HIBYTE((WORD)j);
						abyBuffer_byte_prog[3] = LOBYTE((WORD)j);

						memcpy(&abyBuffer_byte_prog[4], &abyBuffer_SPI[j-dstart_address], (wSize));

						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
								sizeof(abyBuffer3), 0,abyBuffer3,2,CTRL_TIMEOUT);

						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
							(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,2,CTRL_TIMEOUT);

						//check byte programming status before passing next command
						while (abyBuffer_read [1] & (0x01| 0x0C |0x20))
						{
							rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
								sizeof(abyBuffer3),0,abyBuffer3,2,CTRL_TIMEOUT);

							rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
								(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,2,CTRL_TIMEOUT);
						}

						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
							sizeof(abyBuffer2) ,0,abyBuffer2,1,CTRL_TIMEOUT);

						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, CMD_SPI_PASSTRHU_WRITE,
							sizeof(abyBuffer3) ,0,abyBuffer3,2,CTRL_TIMEOUT);

						rc &= libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
						(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,abyBuffer_read,2,CTRL_TIMEOUT);

						if (!bBytepgmSupported)
						{
							if(gpSPICmdList [m].bDirection) //Read
							{
								rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xc1, 0x04,
									(SCRATCHPAD_OFFSET_XDATA + SPI_RESP_BUF_OFFSET),0,gpSPICmdList [m].pbyBuffer,
												(uint16_t)gpSPICmdList [m].wTotalLength,CTRL_TIMEOUT);
							}
							else //Write
							{
								rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, 									CMD_SPI_PASSTRHU_WRITE,(uint16_t)gpSPICmdList [m].wTotalLength,0,gpSPICmdList [m].pbyBuffer,
								(uint16_t)gpSPICmdList [m].wDataLen,CTRL_TIMEOUT);
							}
						}
						else
						{
							rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41, 								CMD_SPI_BYTEFLASH_PGM,(uint16_t)gpSPICmdList [m].wTotalLength,0,
							gpSPICmdList [m].pbyBuffer,(uint16_t)gpSPICmdList [m].wDataLen,CTRL_TIMEOUT);
						}

					}
				}

				if(rc < 0)
				{
					error = SpiPassThruWriteFailed;
					break;
				}
				memset(SPIUploadData_verify, 0xCC, (SIZE_130KB));
				if(AthensReadSPIFlash(DevID,&SPIUploadData_verify[0],dstart_address,(dwActualSize + dstart_address)))
				{
					if(!memcmp(SPIUploadData_verify, abyBuffer_SPI, (dwActualSize)))
					{
						error = Error_Success;
					}
					else
					{
						error = SpiFWCompareFailed;
					}
				}
				else
				{
					error = SpiFWCompareFailed;
				}

			}
		}
	}while(FALSE);
	return error;
}
DWORD SPI_SignatureErase(unsigned int DevID)
{
	DWORD dwActualSize = 0;
	BYTE abyBuffer_Erase[SPI_READ_BUF_SIZE], Buf_Temp[2], abyRecBuf[SPI_READ_BUF_SIZE];
	BYTE byData = 0;
	DWORD error = Error_Success;
	int rc = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	memcpy(abyBuffer_Erase,abySPISignEraseData,sizeof(abyBuffer_Erase));

	do
	{
		dwActualSize = sizeof(abySPISignEraseData);

		UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
		UsbCtlPkt.byRequest 	= 0x03;
		UsbCtlPkt.wValue 	= SRAM_CODE_LOC;
		UsbCtlPkt.wIndex 	= 0;
		UsbCtlPkt.byBuffer 	= &abyBuffer_Erase[0];
		UsbCtlPkt.wLength 	= (WORD)dwActualSize;

		rc = usb_HCE_write_data (&UsbCtlPkt);
		if(rc < 0)
		{
			DEBUGPRINT("Download SRAM code failed \n");
			error = ReadBackFailed;
			break;
		}
		sleep(1);
		memset (abyRecBuf, 0xCC, SPI_READ_BUF_SIZE);

		UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
		UsbCtlPkt.byRequest 	= 0x04;
		UsbCtlPkt.wValue 	= SRAM_CODE_LOC;
		UsbCtlPkt.wIndex 	= 0;
		UsbCtlPkt.byBuffer 	= &abyRecBuf[0];
		UsbCtlPkt.wLength 	= (WORD)dwActualSize;

		rc = usb_HCE_read_data (&UsbCtlPkt);
		if(rc < 0)
		{
			DEBUGPRINT("XData read failed: ReadBack failure \n");
			error = ReadBackFailed;
			break;
		}
		sleep(1);

		if (!(0 == memcmp (abyRecBuf, abyBuffer_Erase, dwActualSize)))
		{
			error = SPISRAMProgFailed;
			DEBUGPRINT("SRAM Programming failed \n");
			break;
		}
		// write sram overlay registers to c:0x8000
		Buf_Temp[0] = 0x10;
		Buf_Temp[1] = 0x02;

		UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
		UsbCtlPkt.byRequest = 0x03;
		UsbCtlPkt.wValue 	= SRAM_PAGE2;
		UsbCtlPkt.wIndex 	= 0;
		UsbCtlPkt.byBuffer 	= &Buf_Temp[0];
		UsbCtlPkt.wLength 	= 2;

		rc = usb_HCE_write_data (&UsbCtlPkt);
		if(rc < 0)
		{
			error = SPISRAMProgFailed;
			DEBUGPRINT("Write sram overlay registers failed \n");
			break;
		}
		// call cmd_call_to_sram
		UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
		UsbCtlPkt.byRequest = CMD_CALL_TO_SRAM;
		UsbCtlPkt.wValue 	= MAP_TO_CODE_ADDR;
		UsbCtlPkt.wIndex 	= 0;
		UsbCtlPkt.byBuffer 	= 0;
		UsbCtlPkt.wLength 	= 0;

		rc = usb_HCE_no_data(&UsbCtlPkt);

		UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
		UsbCtlPkt.byRequest = 0x04;
		UsbCtlPkt.wValue 	= 0x804;
		UsbCtlPkt.wIndex 	= 0;
		UsbCtlPkt.byBuffer 	= (BYTE*)&byData;
		UsbCtlPkt.wLength 	= 1;

		rc = usb_HCE_read_data (&UsbCtlPkt);
		if(rc  > 0)
		{
			BYTE ResetRegValue = 0x40;
			byData |= 0x04;
			UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
			UsbCtlPkt.byRequest = 0x03;
			UsbCtlPkt.wValue 	= 0x804;
			UsbCtlPkt.wIndex 	= 0;
			UsbCtlPkt.byBuffer 	= (BYTE*)&byData;
			UsbCtlPkt.wLength 	= 1;

			rc = usb_HCE_write_data (&UsbCtlPkt);

			UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[DevID].handle;
			UsbCtlPkt.byRequest = 0x03;
			UsbCtlPkt.wValue 	= 0x80A;
			UsbCtlPkt.wIndex 	= 0;
			UsbCtlPkt.byBuffer 	= &ResetRegValue;
			UsbCtlPkt.wLength 	= 1;

			rc = usb_HCE_write_data (&UsbCtlPkt);
		}


	}while(FALSE);
	return error;
}
