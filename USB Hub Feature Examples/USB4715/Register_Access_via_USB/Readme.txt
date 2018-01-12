
/**********************************************************/
				Demo
/**********************************************************/
USB47xx_RegisterRW# cd MPLABConnect/

/**********************************************************/
			Build the library
/**********************************************************/
MPLABConnect# make
=============
Compiling MchpUSBInterface.cpp
=============
Compiling USBHubAbstraction.cpp
=============
Creating MPLABConnect.a

/**********************************************************/
			Created MPLABConnect.a
/**********************************************************/



/**********************************************************/
		Go to sample application folder
/**********************************************************/
MPLABConnect# cd ../examples/register_rw/



/**********************************************************/
	Copy the libary to sample application folder
/**********************************************************/
register_rw# cp ../../MPLABConnect/MPLABConnect.a .
register_rw#

/**********************************************************/
	Build the sample application
/**********************************************************/
register_rw# make
=============
Compiling register_rw.cpp
register_rw.cpp: In function ‘int main(int, char**)’:
register_rw.cpp:132:68: warning: format ‘%x’ expects argument of type ‘unsigned int’, but argument 3 has type ‘DWORD {aka long unsigned int}’ [-Wformat=]
   printf("\nRegister Read value is %s from 0x%08x \n",sztext,dwAddr);
                                                                    ^
=============
Creating register_rw


/**********************************************************/
	Created the exe int out directory
/**********************************************************/
register_rw# cd out

/**********************************************************/
	Run exe - first try help?
/**********************************************************/
out# ./register_rw --help
Register read and write Demo
Op	: Write
Usage	: ./register_rw VID(Hex) PID(Hex) Operation(0x01) XDATAAddr Length Data
Example	: ./register_rw 0x0424 0x4916 0x01 0xBF803004 2 0x12 0x34

Op	: Read
Usage	: ./register_rw VID(Hex) PID(Hex) Operation(0x00) XDATAAddr Length
Example	: ./register_rw 0x0424 0x4916 0x00 0xBF803004 2
Op	: Write and Read
Usage	: ./register_rw
Default values will be taken as VID - 0x424,PID - 0x4916,XDATAAddr - 0xBF803004, Length - 2out#
out#

/**********************************************************/
	Check the connected devices- VID and PID of hub
/**********************************************************/
out# lsusb
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 003: ID 04ca:703c Lite-On Technology Corp.
Bus 001 Device 002: ID 8087:0a2a Intel Corp.
Bus 001 Device 005: ID 0424:4940 Standard Microsystems Corp.
Bus 001 Device 004: ID 0424:4925 Standard Microsystems Corp.
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub


/**********************************************************/
 first write- reg:0xBF803004 2 bytes - value:0x1234
/**********************************************************/
out# ./register_rw 0x424 0x4940 0x01 0xBF803004 2 0x12 0x34
Register read and write Demo
DEBUGINFO: MCHP_Error_Device_Not_Found

Error: MchpUsbOpenID Failed:
out# ./register_rw 0x424 0x4925 0x01 0xBF803004 2 0x12 0x34
Register read and write Demo
Enabled VID:PID = 0424:4940 :3:4
MchpUsbOpenID successful...
Register Write operation

Success :Register Write operation
Register Read operation

Success : Register Read operation
Success : Register Read/write operation

/**********************************************************/
Read back and see previous write is success
/**********************************************************/
out# ./register_rw 0x424 0x4925 0x00 0xBF803004 2
Register read and write Demo
Enabled VID:PID = 0424:4940 :3:4
MchpUsbOpenID successful...
Register Read operation
Success : Register Read operation
0x12 	0x34

Register Read value is 0x12 	0x34 	 from 0xbf803004
out#
/**********************************************************/
	Demo done
/**********************************************************/
