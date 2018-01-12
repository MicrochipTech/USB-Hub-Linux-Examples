/*
©  [2018] Microchip Technology Inc. and its subsidiaries.

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

USB471x_USB49xx_FlexConnect.c - Example code on how to issue the FlexConnect Command to the
USB471x and USB49xx hubs
Author: Connor Chilton <connor.chilton@microchip.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <libusb.h>
#include <semaphore.h>
#include <errno.h>

/* the buffer sizes can exceed the USB MTU */
#define MAX_CTL_XFER	64
#define MAX_BULK_XFER	512

/*
 * VENDOR_ID is always 0x0424 for Microchip/SMSC Hubs
 * Note: FlexConnect Command is not issued to Hub but rather the Hub Feature Controller
*/

#define VENDOR_ID	0x0424

/**
 * struct my_usb_device - struct that ties USB related stuff
 * @dev: pointer libusb devic
 * @dev_handle: device handle for USB devices
 * @ctx: context of the current session
 * @device_desc: structure that holds the device descriptor fields
 * @inbuf: buffer for USB IN communication
 * @outbut: buffer for USB OUT communication
 */

struct libusb_session {
	libusb_device **dev;
	libusb_device_handle *dev_handle;
	libusb_context *ctx;
	struct libusb_device_descriptor device_desc;
	unsigned char inbuf[MAX_CTL_XFER];
	unsigned char outbuf[MAX_BULK_XFER];
	uint16_t wValue;
	int port_num;
};

static struct libusb_session session;


int main(int argc, char **argv)
{
	int r,i,z;
	char yesno[] = "";
	int timeout_time;
	char port_select[] = "";
	ssize_t cnt;
	int port;
	int err = 0;
	/////////////////////////////////////////////////////////////////////////////
	/* List of 49xx HFC PIDs possible */
	unsigned int PRODUCT_ID[6] = {0x4940, 0x494A, 0x494B, 0x494C, 0x494E, 0x494F};
	/////////////////////////////////////////////////////////////////////////////

	printf("This demo will show off the Flexconnect command.\n");

	printf("Setup the Flexconnect Command....\n");
	printf("If smartphone does not enumerate NCM device within 'X' time, the hub will revert to deault.\n");
	TIMEOUT:
	printf("0-No Timeout\n1-10ms\n2-100ms\n3-500ms\n4-1s\n5-5s\n6-10s\n7-20s\n");
	scanf("%i", &timeout_time);
	switch(timeout_time){
		case 0:
			printf("No timeout selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0031;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0032;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0033;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0034;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 1:
			printf("Timeout of 10ms selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0131;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0132;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0133;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0134;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 2:
			printf("Timeout of 100ms selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0231;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0232;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0233;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0234;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 3:
			printf("Timeout of 500ms selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0331;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0332;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0333;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0334;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 4:
			printf("Timeout of 1s selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0431;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0432;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0433;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0434;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 5:
			printf("Timeout of 5s selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0531;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0532;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0533;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0534;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 6:
			printf("Timeout of 10s selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0631;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0632;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0633;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0634;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		case 7:
			printf("Timeout of 20s selected\n");
			printf("Which port will the command be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wValue = 0x0731;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wValue = 0x0732;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wValue = 0x0733;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wValue = 0x0734;
			}
			else
			{
				printf("Invalid Port Selected\n");
				while(getchar()!='\n');
				goto TIMEOUT;
			}
			break;

		default:
			printf("Invalid Response. Please choose 0-7.\n");
			while(getchar()!='\n');
			goto TIMEOUT;
	}

	/* The Control Xfer is Configured and Initiated Below
	* Only the wValue parameter should be modified, all others should remain the same.
	*
	* The wValue in the Control Xfer below is configured as:
	* Bit 15:11	Reserved (Always '0')
	* Bit 10:8	ENUM_TIMEOUT: If smartphone does not enumerate NCM device within "X" time, hub will revert to default
	*		000 = No Timeout
	*		001 = 10ms
	*		010 = 100ms
	*		011 = 500ms
	*		100 = 1 second
	*		101 = 5 seconds
	*		110 = 10 seconds
	*		111 = 20 seconds
	* Bit 7	Reserved (Always '0')
	* Bit 6:5	ROLE_SWITCH_TYPE: Always '01' for Flexconnect Mode
	* Bit 4	ROLE_SWITCH_STATE:
	*		1 - Initiates Flexconnect Mode
	*		0 - Terminates Flexconnect Mode
	* Bit 3:0	PORT: Selects which port to initiate/terminate Flexconnect Mode
	*		0001 = Port 1
	*		0010 = Port 2
	*		0011 = Port 3
	*		0100 = Port 4
	*		All Others = Invalid
	*
	* EXAMPLE: Initiate Flexconnect Mode on Port 1 with 1 second timeout. wValue = 0x0411
	* EXAMPLE: Initiate Flexconnect Mode on Port 2 with 1 second timeout. wValue = 0x0412
	* EXAMPLE: Initiate Flexconnect Mode on Port 3 with 1 second timeout. wValue = 0x0413
	* EXAMPLE: Initiate Flexconnect Mode on Port 4 with 1 second timeout. wValue = 0x0414
	*/
	/* initialize the library for the session we just declared */
	r = libusb_init(&session.ctx);
	if (r < 0) {
		printf("Init Error %i occourred.\n", r);
		return -EIO;
	}

	/* set verbosity level to 3, as suggested in the documentation */
	libusb_set_debug(session.ctx, 3);
	HFC:
	printf("Searching USB devices...");
	cnt = libusb_get_device_list(session.ctx, &session.dev);
	if (cnt < 0) {
		printf("no device found\n");
		libusb_exit(session.ctx);
		return -ENODEV;
	}

	for (i=0; i<6; i++){
		session.dev_handle = libusb_open_device_with_vid_pid(session.ctx, VENDOR_ID, PRODUCT_ID[i]);
		if (session.dev_handle){
			printf("Opened HFC with PID 0x%04x\n", PRODUCT_ID[i]);
			break;
		}
	}
	if (!session.dev_handle){
		printf("Failed to open HFC\n");
		libusb_exit(session.ctx);
		return -ENODEV;
	}
	printf("ok\n");

	/* free the list, unref the devices in it */
	libusb_free_device_list(session.dev, 1);

	/* find out if a kernel driver is attached */
	if (libusb_kernel_driver_active(session.dev_handle, 0) == 1) {
		printf("Device has kernel driver attached.\n");
		/* detach it */
		if (!libusb_detach_kernel_driver(session.dev_handle, 0))
			printf("Kernel Driver Detached!\n");
	}

	/* claim interface 0 (the first) of device (mine had jsut 1) */
	printf("Claiming interface 0...");
	r = libusb_claim_interface(session.dev_handle, 0);
	if (r < 0) {
		printf("failed\n");
		libusb_close(session.dev_handle);
		libusb_exit(session.ctx);
		return -EIO;
	}
	printf("ok\n");

	int len;
	int transferred;
	uint8_t bmRequestType = 0x41;		/* Always 0x41					*/
	uint8_t bRequest = 0x90;		/* Always 0x90 for SET_ROLE_SWITCH 		*/
	//uint16_t wValue = 0x0011;		/* See above Note for Configuring this value 	*/
	uint16_t wIndex = 0x0000;		/* Always 0x0000				*/
	unsigned char *data = 0;		/* Always zero data length control xfer		*/
	uint16_t wLength = 0x0000;		/* Always 0x0000				*/
	unsigned int timeout_ = 50000000;

	/* Send Flexconnect control transfer */
	r = libusb_control_transfer(session.dev_handle,
				    bmRequestType,
				    bRequest,
				    session.wValue,
				    wIndex,
				    data,
				    wLength,
				    timeout_);
	if (!r){
		printf("Flexconnect Control transfer successful!\n");
	}
	else{
		printf("Flexconnect Control transfer failed. Error: %d\n", r);
	}


	/* release interface */
	printf("Releasing interface...");
	r = libusb_release_interface(session.dev_handle, 0);
	if (r) {
		printf("failed\n");
		return -EIO;
	}
	printf("ok\n");

	/* close the device we opened */
	libusb_close(session.dev_handle);
	libusb_exit(session.ctx);
	return 0;
}
