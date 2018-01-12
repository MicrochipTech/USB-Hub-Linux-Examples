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
*  register_rw.cpp
*   This file gives the sample code/ test code for using MchpUSB API
*	Interface.
**********************************************************************************
*  $Revision: #1.1 $  $DateTime: 2015/09/21  18:04:00 $  $    $
*  Description: sample code for Register Read/write
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
//  SDK API Header file
#include "MchpUSBInterface.h"

using namespace std;

int main (int argc, char* argv[])
{
	CHAR sztext[2048];
	CHAR chText[256];
	DWORD dwError = 0;
	WORD vendor_id = 0x424 ,product_id= 0x4916;
	DWORD dwAddr = 0xBF803004;
	BYTE byReadData[256] = "\0", byWriteData[256] = "\0";
	BYTE byOp = 0x00;
	int iLen =2,i=0;
	HANDLE hDevice =  INVALID_HANDLE_VALUE;
	BOOL bRet = TRUE;


	memset(sztext,0,2048);

	printf("Register read and write Demo\n");

	if(argc < 2)
	{
		printf("PID and VID are not mentioned.\n");
		printf("Use --help option for further details \n");
		byOp = 0x01; //Redirect to register write and read.

		//Getting data for write.
		for(i=0; i<iLen; i++)
		{
			byWriteData[i] = 0x01;
		}
	}
	else if((0 == strcmp(argv[1],"--help")) || (0 == strcmp(argv[1],"/?"))) //Help
	{
		printf("Op	: Write \n");
		printf("Usage	: ./register_rw VID(Hex) PID(Hex) Operation(0x01) XDATAAddr Length Data \n");
		printf("Example	: ./register_rw 0x0424 0x4916 0x01 0xBF803004 2 0x12 0x34 \n\n");
		printf("Op	: Read \n");
		printf("Usage	: ./register_rw VID(Hex) PID(Hex) Operation(0x00) XDATAAddr Length\n");
		printf("Example	: ./register_rw 0x0424 0x4916 0x00 0xBF803004 2 \n");
		printf("Op	: Write and Read \n");
		printf("Usage	: ./register_rw \n");
		printf("Default values will be taken as VID - 0x424,PID - 0x4916,XDATAAddr - 0xBF803004, Length - 2");
		return -1;
	}
	else
	{
		vendor_id  =  strtol (argv[1], NULL, 0) ;           //Getting vid and Pid from commandline arguments.
		product_id =  strtol (argv[2], NULL, 0) ;
		byOp  	   =  strtol (argv[3], NULL, 0) ;
		dwAddr	   =  strtoul (argv[4], NULL, 0) ;
		iLen	   =  strtol (argv[5], NULL, 0) ;
		if(0x01 == byOp)
		{
			for(i=0; i<iLen; i++)
			{
				byWriteData[i] = strtol (argv[6+i], NULL, 0) ;
			}
		}
	}

	//Return handle to the first instance of VendorID and ProductID matched device.
	hDevice = MchpUsbOpenID(vendor_id,product_id);

	if(INVALID_HANDLE_VALUE == hDevice)
	{
		printf ("\nError: MchpUsbOpenID Failed:\n");
		return -1;
	}

	printf ("MchpUsbOpenID successful...\n");

	if(0x00 == byOp)
	{
		//Read the XDATA register(s) in the XDATA space of internal registers
		cout << "Register Read operation \n";
		if (FALSE ==MchpUsbRegisterRead(hDevice,dwAddr,iLen, (UINT8 *)&byReadData))
		{
			dwError = MchpUsbGetLastErr(hDevice);
			printf ("Error,%04x\n",(unsigned int)dwError);
            goto EXIT_APP;
		}
		cout << "Success : Register Read operation \n";
		for(i =0; i< iLen; i++)
		{
			sprintf(chText,"0x%02x \t",byReadData[i] );
			strcat(sztext,chText);
		}
		printf("%s \n",sztext);
		printf("\nRegister Read value is %s from 0x%08x \n",sztext,dwAddr);
	}
	else
	{
		//Write to the XDATA register (s) in the XDATA space of internal registers.
		printf("Register Write operation \n");
		if (FALSE ==MchpUsbRegisterWrite(hDevice,dwAddr,iLen,(UINT8 *) &byWriteData))
		{
			dwError = MchpUsbGetLastErr(hDevice);
			printf ("Failed to open the device Error,%08x\n",(unsigned int)dwError);
			goto EXIT_APP;
		}
		cout << "\nSuccess :Register Write operation\n";
		sleep(2);
		cout << "Register Read operation\n";
		if (FALSE ==MchpUsbRegisterRead(hDevice,dwAddr,iLen, (UINT8 *)&byReadData))
		{
			dwError = MchpUsbGetLastErr(hDevice);
			printf ("Error,%04xn",(unsigned int)dwError);
			goto EXIT_APP;
		}
		cout << "\nSuccess : Register Read operation\n";

		//Compare read data with written data.
		for(i =0; i< iLen; i++)
		{
			if(byReadData [i]==byWriteData[i])
			{
				bRet &= TRUE;
			}
			else
			{
				bRet &= FALSE;
			}
		}
		if(bRet == TRUE)
		{
			cout << "Success : Register Read/write operation\n";
		}
		else
		{
			cout << "failed : Register Read/write operation\n";
		}
	}
EXIT_APP:
	//Close device handle
	if(FALSE == MchpUsbClose(hDevice))
	{
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("MchpUsbClose:Error Code,%04x\n",(unsigned int)dwError);
		return -1;
	}

	return 0;
}
