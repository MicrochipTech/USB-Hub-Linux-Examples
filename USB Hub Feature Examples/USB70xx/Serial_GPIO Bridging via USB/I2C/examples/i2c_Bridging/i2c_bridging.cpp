/*
**********************************************************************************
* © 2017 Microchip Technology Inc. and its subsidiaries.
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
*  i2c_bridging.cpp
*   This file gives the sample code/ test code for using using I2C with the USB7002
**********************************************************************************
*  $Revision: #1.1.2 $  $DateTime: 2017/11/1  18:04:00 $  $    $
*  Description: Sample code for i2c bridging demo
**********************************************************************************
* $File:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdbool.h>

#include <cstdlib>

//MPLABConnect SDK API Header file
#include "MchpUSBInterface.h"

using namespace std;

int main (int argc, char* argv[])
{
	CHAR sztext[2048];
	CHAR chText[256];
	DWORD dwError = 0;
	WORD vendor_id = 0x424 ,product_id= 0x4002;
	BYTE bySlaveAddr,byStartAddr,byOperation;
	HANDLE hDevice =  INVALID_HANDLE_VALUE;
	INT iLength,hub_count = 0;
	BYTE byWriteData[513];
	BYTE byReadData[512];
	BOOL bStart,bStop,bNack;
	char path[20] = {0};

	if((0 == strcmp(argv[1],"--help")) || (0 == strcmp(argv[1],"/?"))) //Help
	{

		printf("SlaveAddr	: I2C Slave Address \n");
		printf("StartAddr	: Start Address of I2C Slave to Read/Write \n");
		printf("Start		: 1 - Generates Start condition\n");
		printf("		: 0 - Does not generate Start condition \n");
		printf("Stop		: 1 - Generates Stop condition\n");
		printf("		: 0 - Does not generate Stop condition \n");
		printf("Nack		: 1 - Generates NACK condition for the last byte of the transfer\n");
		printf("		: 0 - Does not generate NACK condition \n\n");
		printf("Operation 	: Write \n");
		printf("Usage		: ./i2cBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x01) SlaveAddr StartAddr Length Data \n");
		printf("Example		: ./i2cBridging 0x0424 0x4002 7:1 0x01 0x50 0x00 4 0x11 0x22 0x33 0x44 \n \n");
		printf("Operation 	: Read \n");
		printf("Usage		: ./i2cBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x00) SlaveAddr StartAddr Length \n");
		printf("Example		: ./i2cBridging 0x0424 0x4002 7:1 0x00 0x50 0x00 4 \n\n");
		printf("Operation	: Transfer: Write \n");
		printf("Usage		: ./i2cBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x03) SlaveAddr StartAddr Length Start(0/1) Stop(0/1) Nack(0/1) Data\n");
		printf("Example		: ./i2cBridging 0x0424 0x4002 7:1 0x03 0x50 0x00 4 1 1 0 0x11 0x22 0x33 0x44 \n \n");
		printf("Operation	: Transfer: Read \n");
		printf("Usage		: ./i2cBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x04) SlaveAddr StartAddr Length Start(0/1) Stop(0/1) Nack(0/1)\n");
		printf("Example		: ./i2cBridging 0x0424 0x4002 7:1 0x04 0x50 0x00 4 1 1 1\n \n");

		exit(1);

	}
	else if(argc < 8)
	{
		printf("ERROR: Invalid Usage.\n");
		printf("Use --help option for further details \n");
		exit(1);
	}
	else
	{
		vendor_id  =  strtol (argv[1], NULL, 0) ;      //Getting vid and Pid from commandline arguments.
		product_id =  strtol (argv[2], NULL, 0) ;
		strcpy(path,argv[3]);
		byOperation=  strtol (argv[4], NULL, 0) ;
		bySlaveAddr=  strtol (argv[5], NULL, 0) ;
		byStartAddr=  strtol (argv[6], NULL, 0) ;
		iLength	   =  strtol (argv[7], NULL, 0) ;
		byWriteData[0]= byStartAddr;

		if(byOperation == 0x01) //Write
		{
			for (UINT8 i=1; i <= iLength; i++)
			{
				byWriteData[i] = strtol (argv[7 + i], NULL,0);
			}
		}
		if(byOperation == 0x03 || byOperation == 0x04) //Transfer
		{
			bStart = strtol (argv[8], NULL,0) ;
			bStop  = strtol (argv[9], NULL,0) ;
			bNack  = strtol (argv[10], NULL,0) ;
			if(byOperation == 0x03 ) //Write
			{
				for (UINT8 i=1; i <= iLength; i++)
				{
					byWriteData[i] = strtol (argv[10 + i], NULL,0);
				}
			}
		}
	}

	// Get the version number of the SDK
	if (FALSE == MchpUsbGetVersion(sztext))
	{
		printf ("\nError:SDK Version cannot be obtained,Press any key to exit....");
		exit (1);
	}

	cout << "SDK Version:" <<sztext << endl;
	memset(sztext,0,2048);

	hub_count = MchpGetHubList(sztext);

	if(hub_count < 0)
	{
		printf("No USB hubs found in the system");

	}

	printf("I2C Bridging Demo\n");



	//Return handle to the first instance of VendorID,ProductID and Devicepath matched device.
	hDevice = MchpUsbOpen(vendor_id,product_id,path);

	if(INVALID_HANDLE_VALUE == hDevice)
	{
		printf ("\nError: MchpUsbOpenID Failed:\n");
		exit (1);
	}

	printf ("MchpUsbOpenID successful...\n");

	//Set I2C config parameters
	if(FALSE == MchpUsbI2CSetConfig(hDevice))
	{
		dwError = MchpUsbGetLastErr(hDevice);
		printf("Error: MchpUsbI2CSetConfig- %04x\n",(unsigned int)dwError);
		exit (1);
	}
	if(0x00 == byOperation) // Read
	{
		//I2C write through the I2C pass-through interface of USB device
		if(FALSE== MchpUsbI2CWrite(hDevice,1,(BYTE *)&byStartAddr,bySlaveAddr))
		{
			dwError = MchpUsbGetLastErr(hDevice);
			printf("Failed to write- %04x\n",(unsigned int)dwError);
			exit (1);
		}

		//I2C read through the I2C pass-through interface of USB device
		if(FALSE == MchpUsbI2CRead(hDevice,iLength,&byReadData[0],bySlaveAddr) )
		{
			dwError = MchpUsbGetLastErr(hDevice);
			printf("Failed to Read- %04x\n",(unsigned int)dwError);
			exit (1);
		}
		for(UINT8 i =0; i< iLength; i++)
		{
			sprintf(chText,"0x%02x \t",byReadData[i] );
			strcat(sztext,chText);
		}
		printf("%s \n",sztext);
	}
	else if(0x01 == byOperation)//Write
	{
		//I2C write through the I2C pass-through interface of USB device
		if(FALSE== MchpUsbI2CWrite(hDevice,(iLength+1),(BYTE *)&byWriteData,bySlaveAddr))
		{
			dwError = MchpUsbGetLastErr(hDevice);
			printf("Failed to write- %04x\n",(unsigned int)dwError);
			exit (1);
		}
	}
	else //Transfer
	{
		if(0x03 == byOperation) //Transfer Write
		{
			//I2C write through the I2C pass-through interface of USB device
			if(FALSE ==  MchpUsbI2CTransfer(hDevice,0, (BYTE *)&byWriteData,(iLength+1),bySlaveAddr,bStart,bStop,bNack))
			{
				dwError = MchpUsbGetLastErr(hDevice);
				printf("Failed to write- %04x\n",(unsigned int)dwError);
				exit (1);
			}
		}
		else if(0x04 == byOperation)
		{
			//I2C write through the I2C pass-through interface of USB device
			if(FALSE ==  MchpUsbI2CTransfer(hDevice,0, (BYTE *)&byWriteData,1,bySlaveAddr,bStart,bStop,bNack))
			{
				dwError = MchpUsbGetLastErr(hDevice);
				printf("Failed to write start addr- %04x\n",(unsigned int)dwError);
				exit (1);
			}
			//I2C read through the I2C pass-through interface of USB device
			if(FALSE ==  MchpUsbI2CTransfer(hDevice,1,&byReadData[0],iLength,bySlaveAddr,bStart,bStop,bNack))
			{
				dwError = MchpUsbGetLastErr(hDevice);
				printf("Failed to Read- %04x\n",(unsigned int)dwError);
				exit (1);
			}
			for(UINT8 i =0; i< iLength; i++)
			{
				sprintf(chText,"0x%02x \t",byReadData[i] );
				strcat(sztext,chText);
			}
			printf("%s \n",sztext);
		}
	}

	//Close device handle
	if(FALSE == MchpUsbClose(hDevice))
	{
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("MchpUsbClose:Error Code,%04x\n",(unsigned int)dwError);
		exit (1);
	}

	return 0;
}
