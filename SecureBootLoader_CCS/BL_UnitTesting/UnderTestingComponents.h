#ifndef UNDER_TESTING_COMPONENTS_H
#define UNDER_TESTING_COMPONENTS_H
#include "bootloader.h"
void BL_Print_Message_UnderTest(char *format ,...);
BL_Status BL_UART_Fetch_Host_Command_UnderTest();
uint8_t Bootloader_CRC_Verify_UnderTesting(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
void Bootloader_Send_Data_To_Host_UnderTesting(uint8_t *Host_Buffer, uint32_t Data_Len);
void Bootloader_Get_Version_UnderTesting(uint8_t *Host_Buffer);
uint8_t Host_Address_Verification_UnderTesting (uint32_t Jump_Address);
void Bootloader_Jump_To_Address_UnderTesting(uint8_t *Host_Buffer);
 uint8_t Perform_Flash_Erase_UnderTesting(uint8_t Sector_Number, uint8_t Number_Of_Sectors);
#endif
