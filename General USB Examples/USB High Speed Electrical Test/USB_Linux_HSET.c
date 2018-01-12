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
	uint16_t wIndex;
	int port_num;
};

static struct libusb_session session;


int main(int argc, char **argv)
{
	int r,i,z;
	char yesno[] = "";
	int timeout_time;
	char port_select[] = "";
	char test_select[] = "";
	ssize_t cnt;
	int port;
	int err = 0;
	int VENDOR_ID = 0x0424;
	int PRODUCT_ID = 0;
	char PRODUCT_ID1[] = "";

	INPUT:
	printf("Please enter the product ID of the hub under test\n0x");
	scanf("%s", PRODUCT_ID1);
	if(strlen(PRODUCT_ID1) != 4){
		printf("Please enter 4 digit hex only\n");
		goto INPUT;
	}
	int count;
	char hexset[] = "0123456789ABCDEFabcdef";
	count = strspn(PRODUCT_ID1, hexset);
	if (count != 4){
		printf("Please enter 4 digit hex only\n");
		goto INPUT;
	}
	PRODUCT_ID = (int)strtol(PRODUCT_ID1, NULL, 16);


	printf("This demo will iniate a test mode on a port.\n");

	TEST_SEL:
		printf("Press '1' for Test_J\n");
		printf("Press '2' for Test_K\n");
		printf("Press '3' for Test_SE0_NAK\n");
		printf("Press '4' for Test_Packet\n");
		printf("Press '5' for Test_Force_Enable\n");
		printf("Press 'q' or 'Q' or CTL+C to quit\n");
		scanf("%s", test_select);
		if (strcmp(test_select, "1")==0){
		PORT_SEL1:
			printf("Which port will Test_J be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wIndex = 0x0101;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wIndex = 0x0102;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wIndex = 0x0103;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wIndex = 0x0104;
			}
			else
			{
				printf("Invalid Port Selected. Please select a valid port # or Ctrl+C to quit\n");
				goto PORT_SEL1;
			}
		}
		else if (strcmp(test_select, "2")==0){
		PORT_SEL2:
			printf("Which port will Test_K be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wIndex = 0x0201;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wIndex = 0x0202;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wIndex = 0x0203;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wIndex = 0x0204;
			}
			else
			{
				printf("Invalid Port Selected. Please select a valid port # or Ctrl+C to quit\n");
				goto PORT_SEL2;
			}
		}
		else if (strcmp(test_select, "3")==0){
		PORT_SEL3:
			printf("Which port will Test_SE0_NAK be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wIndex = 0x0301;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wIndex = 0x0302;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wIndex = 0x0303;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wIndex = 0x0304;
			}
			else
			{
				printf("Invalid Port Selected. Please select a valid port # or Ctrl+C to quit\n");
				goto PORT_SEL3;
			}
		}
		else if (strcmp(test_select, "4")==0){
		PORT_SEL4:
			printf("Which port will Test_Packet be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wIndex = 0x0401;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wIndex = 0x0402;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wIndex = 0x0403;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wIndex = 0x0404;
			}
			else
			{
				printf("Invalid Port Selected. Please select a valid port # or Ctrl+C to quit\n");
				goto PORT_SEL4;
			}
		}

		else if (strcmp(test_select, "5")==0){
		PORT_SEL5:
			printf("Which port will Test_Force_Enable be sent to?\n");
			scanf("%s", port_select);
			if (strcmp(port_select, "1") == 0)
			{
				printf("Port 1 selected\n");
				session.port_num = 1;
				session.wIndex = 0x0501;
			}
			else if(strcmp(port_select, "2") == 0)
			{
				printf("Port 2 selected\n");
				session.port_num = 2;
				session.wIndex = 0x0502;
			}
			else if(strcmp(port_select, "3") == 0)
			{
				printf("Port 3 selected\n");
				session.port_num = 3;
				session.wIndex = 0x0503;
			}
			else if(strcmp(port_select, "4") == 0)
			{
				printf("Port 4 selected\n");
				session.port_num = 4;
				session.wIndex = 0x0504;
			}
			else
			{
				printf("Invalid Port Selected. Please select a valid port # or Ctrl+C to quit\n");
				goto PORT_SEL5;
			}
		}
		else if (strcmp(test_select, "q")==0 || strcmp(test_select, "Q")==0){
			return 0;
		}
		else{
			printf("Invalid Test Selected or invalid character.\n");
			goto TEST_SEL;
		}


	r = libusb_init(&session.ctx);
	if (r < 0) {
		printf("Init Error %i occourred.\n", r);
		return -EIO;
	}

	/* set verbosity level to 3, as suggested in the documentation */
	libusb_set_debug(session.ctx, 3);


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

		int len;
		int transferred;
		uint8_t bmRequestType = 0x23;
		uint8_t bRequest = 0x03;
		uint16_t wValue = 0x0015;
		//uint16_t wIndex = 0x0000;
		unsigned char *data = 0;
		uint16_t wLength = 0x0000;
		unsigned int timeout_ = 50000000;

		/* Send Endpoint Reflector control transfer */
		r = libusb_control_transfer(session.dev_handle,
							bmRequestType,
							bRequest,
							wValue,
							session.wIndex,
							data,
							wLength,
							timeout_);
		if (!r){
			printf("Port now in test mode!\n");
		}
		else{
			printf("Control transfer failed. Error: %d\n", r);
		}

		/* close the device we opened */
		libusb_close(session.dev_handle);
		libusb_exit(session.ctx);
		return 0;

}
