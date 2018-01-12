/*******************************************************************************
  Protouch2 SDK header file

  Company:
    Microchip Technology Inc.

  File Name:
    SpiFlash.h

  Summary:
    Protouch2 SDK Header File for SPI Flash APIs

  Description:
    Protouch2 SDK LIB Header file for SPI Flash APIs
*******************************************************************************/
#pragma once

#include "typedef.h"

#define CMD_SPI_PASSTHRU_ENTER				0x60
#define CMD_SPI_PASSTHRU_EXIT				0x62

#define MAX_FW_SIZE					(128 * 1024)

/*Get_hub _Info
 * API used to get Hub details such as
 * Firmware type
 * ASIC Type
 * Device revision*
 * Featuresflag - Boots from ROM/SPI*/
int Get_Hub_Info(HANDLE handle, uint8_t *data);

/* AthensReadSPIFlash
 * API used to read SPI Flash content */
BOOL AthensReadSPIFlash(int DevID,BYTE *SPIUploadData, DWORD dstart_address, DWORD dend_address_ul);

/* AthensReadSPICodeSPIROM
 * API used to read SPI Flash content if device boots from ROM */
BOOL AthensReadSPICodeSPIROM(int hub_id,BYTE *SPIUploadData, DWORD dstart_address, DWORD dend_address_ul);

/*AthensReadSPICodeSPIROM
 * API used to read SPI Flash content if device boots from SPI */
BOOL AthensReadSPICodeIntROM(int hub_id,BYTE *SPIUploadData, DWORD dstart_address, DWORD dend_address_ul);

/*WriteSPIFlash
 * API used to write content to SPI Flash*/
BOOL WriteSPIFlash(HANDLE DevID,BYTE *byWriteData,WORD iRestartDelay,BOOL bErasePseudoOTP,DWORD dwStartAddr,DWORD dwSize );

/*SPI_GetManufactureID
 * API used to get Manufacturer id of SPI Flash used*/
DWORD SPI_GetManufactureID (int DevID, char *szBuffer, BOOL bSupportAll);

/*LoadBinary
 * API used to write content to SPI Flash*/
DWORD LoadBinary (int DevID,bool bErasePgmArea, BYTE *abyBuffer_SPI, DWORD dstart_address, DWORD dwActualSize);

/*SPI_SignatureErase
 * API used to erase SPI content*/
DWORD SPI_SignatureErase(unsigned int hub_index);
