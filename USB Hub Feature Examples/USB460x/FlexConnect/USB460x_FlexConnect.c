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

USB460x_FlexConnect.c - Example code on how to issue the FlexConnect Command to the
USB460x hub
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

/* replace this with your ID pair
 * VENDOR_ID is always 0x0424 for Microchip/SMSC Hubs
 * PRODUCT_ID is 0x2530 for the USB(8)460x Family internal Hub Feature Controller Device
 * Note: Flexconnect Command is not issued to Hub.
*/

#define VENDOR_ID	0x0424
#define PRODUCT_ID	0x2530

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
	//uint16_t wValue;

	printf("This demo will show off the Flexconnect command.\n");

	printf("Setup the Flexconnect Command....\n");
	printf("*************************************\n");
	printf("           Define the wValue         \n");
	printf("Bit 0: This bit has no fucntion\n");
	printf("Bit 1: Disables Port 1 after flex\n");
	printf("Bit 2: Disables Port 2 after flex\n");
	printf("Bit 3: Disables Port 3 after flex\n");
	printf("Bit 4: Disables Port 4 after flex\n");
	printf("Bit 5: Disables Port 5 after flex\n");
	printf("Bit 6: The value of Flexconnect after hub is re-attached\n");
	printf("Bit 7: Change pin fucntion EX: VBUS_DET will have same functionality regarless of flex state\n");
	printf("Bit 8-10: Host detect timeout\n");
	printf("\t000 = No Timeout\n");
	printf("\t001 = 10ms\n");
	printf("\t010 = 100ms\n");
	printf("\t011 = 500ms\n");
	printf("\t100 = 1s\n");
	printf("\t101 = 5s\n");
	printf("\t110 = 10s\n");
	printf("\t111 = 20s\n");
	printf("Bit 11-13: These bits have no function\n");
	printf("Bit 14: Make disabled port Dedicated Charging Ports\n");
	printf("Bit 15: This bit must always be 1\n\n");
	TRYAGAIN:
	printf("Default flex wValue is 0x8040\n");
	printf("Please enter a 4 digit hex number: 0x");
	scanf("%x", (unsigned int *)&session.wValue);
	if (session.wValue < 0x8040 || session.wValue > 0xFFFF)
	{
		printf("Please enter a valid 4 digit hex number\n");
		printf("Refer to application note AN1700 for more information on wValue\n");
		while(getchar()!='\n');
		goto TRYAGAIN;
	}

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

	/* open device w/ vendorID and productID */
	printf("Opening device ID %04x:%04x...", VENDOR_ID, PRODUCT_ID);
	session.dev_handle = libusb_open_device_with_vid_pid(session.ctx, VENDOR_ID, PRODUCT_ID);
	if (!session.dev_handle) {
		printf("failed/not in list\n");
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
	/*
	Define the wValue
	Bit 0: This bit has no fucntion
	Bit 1: Disables Port 1 after flex
	Bit 2: Disables Port 2 after flex
	Bit 3: Disables Port 3 after flex
	Bit 4: Disables Port 4 after flex
	Bit 5: Disables Port 5 after flex
	Bit 6: The value of Flexconnect after hub is re-attached
	Bit 7: Change pin fucntion EX: VBUS_DET will have same functionality regarless of flex state
	Bit 8-10: Host detect timeout
			000 = No Timeout
			001 = 10ms
			010 = 100ms
			011 = 500ms
			100 = 1s
			101 = 5s
			110 = 10s
			111 = 20s
	Bit 11-13: These bits have no function
	Bit 14: Make disabled port Dedicated Charging Ports
	Bit 15: This bit must always be 1
	*/

	int len;
	int transferred;
	uint8_t bmRequestType = 0x41;		/* Always 0x41					*/
	uint8_t bRequest = 0x08;		/* Always 0x90 for SET_ROLE_SWITCH 		*/
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
