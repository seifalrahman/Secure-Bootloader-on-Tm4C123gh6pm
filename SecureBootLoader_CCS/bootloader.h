
#ifndef BOOTLOADER_H
#define BOOTLOADER_H
/*------------------Includes-------------------*/
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <stdlib.h>
#include "tinycrypt/aes.h"
#include "tinycrypt/cbc_mode.h"
#include "tinycrypt/ctr_mode.h"
#include "Wrapper_Functions.h"
#include "inc/tm4c123gh6pm.h"
#include "tinycrypt/sha256.h"
#include <tinycrypt/cmac_mode.h>
/*------------------Macro Declarations-------------------*/
#define BL_DEBUG_UART  							&huart2
#define BL_HOST_COMMUNICATION_UART  &huart2
#define CRC_ENGINE_OBJ							&hcrc

#define DEBUG_INFO_DISABLE					0
#define DEBUG_INFO_ENABLE						1
#define BL_DEBUG_ENABLE							DEBUG_INFO_DISABLE


#define SECTOR_SIZE  1024
#define BL_ENABLE_UART_DEBUG_MESSAGE 0x00
#define BL_ENABLE_SPI_DEBUG_MESSAGE  0x01
#define BL_ENABLE_CAN_DEBUG_MESSAGE	 0x02
#define BL_DEBUG_METHOD		(BL_ENABLE_UART_DEBUG_MESSAGE)
#define BL_HOST_BUFFER_RX_LENGTH		 200

#define CBL_GET_VER_CMD							0x10
//This command is used to read the bootloader version from the MCU
#define CBL_GET_HELP_CMD						0x11
//This command is used to know what are the commands supported by the bootloader
#define	CBL_GET_CID_CMD							0x12
//This command is used to read the MCU chip identification number 
#define	CBL_AES_ENCRYPT_DECRYPT_CMD			0x13
//This command is used to read the Flash Read Protection Level 
#define CBL_GO_TO_ADDR_CMD					0x14
//This command is used to jump bootloader to specified address 
#define CBL_FLASH_ERASE_CMD					0x15
//This command is used to mass erase or sector erase of the user flash 
#define CBL_MEM_WRITE_CMD						0x16
// This command is used to write data in different memories of the MCU

#define CBL_EN_R_W_PROTECT_CMD			0x17
// This command is used to enable read/write protect on different sectors of the  user flash
#define CBL_MEM_READ_CMD						0x18
// This command is used to read data from different memories of the microcontroller
#define CBL_READ_SECTOR_STATUS_CMD	0x19
// This command is used to  read all the sector protection status 
#define CBL_OTP_READ_CMD						0x20
#define	CBL_DIS_R_W_PROTECT_CMD			0x21

#define AES_ENCRYPT                 0x0
#define AES_DECRYPT                 0x1
#define AES_CBC                     0x0
#define AES_ECB                     0x1
#define AES_CTR                     0x2


#define CBL_VENDOR_ID								100
#define CBL_SW_MAJOR_VERSION				1
#define CBL_SW_MINOR_VERSION				0
#define CBL_SW_PATCH_VERSION				0

#define CRC_TYPE_SIZE_BYTE           4


#define CRC_VERIFICATION_FAILED			0
#define CRC_VERIFICATION_PASSED			1

#define CBL_SEND_ACK								0xCD
#define CBL_SEND_NACK								0xAB


#define FLASH_SECTOR2_BASE_ADDRESS  0x0010000
#define FLASH_SECTOR2_BASE_ADDRESS2  0x0020000

#define ADDRESS_IS_INVALID				0x00
#define ADDRESS_IS_VALID					0x01

#define STM32F407_SRAM1_SIZE			(112*1024)
#define STM32F407_SRAM2_SIZE			(16*1024)
#define STM32F407_SRAM3_SIZE			(64*1024)
#define STM32F407_FLASH_SIZE			(1024*1024)
#define STM32F407_SRAM1_END				(SRAM1_BASE+STM32F407_SRAM1_SIZE)
#define STM32F407_SRAM2_END				(SRAM2_BASE+STM32F407_SRAM2_SIZE)
#define STM32F407_SRAM3_END				(CCMDATARAM_BASE+STM32F407_SRAM3_SIZE)
#define TM4C123GH6PM_FLASH_END		(0x0003FFFF)


#define CBL_FLASH_MAX_SECTOR_NUMBER		255

#define INVALID_SECTOR_NUMBER					0x00
#define VALID_SECTOR_NUMBER						0x01
#define CBL_FLASH_MASS_ERASE					0xFF
#define UNSUCCESSFUL_ERASE						0x02
#define SUCCESSFUL_ERASE							0x03

#define HAL_SUCCESSFUL_ERASE					0xFFFFFFFFU
#define HAL_MAX_DELAY 								0xFFFFFFFF

#define FLASH_PAYLOAD_WRITE_FAILED		0x00
#define FLASH_PAYLOAD_WRITE_PASSED		0x01

#define FLASH_LOCK_WRITE_FAILED		0x00
#define FLASH_LOCK_WRITE_PASSED		0x01

#define APP_ADDRESS								23								
#define AES_CBC_MODE							9
#define AES_ECB_MODE							10
#define AES_CTR_MODE							11
#define AES_ENCRYPTION_MODE				AES_CBC_MODE

							

typedef void(*pMainApp)(void) ;
typedef void(*Jump_Ptr)(void) ;

/*------------------Data Type Declarations-------------------*/
typedef enum {
	BL_NACK=0,
	BL_OK
}BL_Status ;

/*------------------Software Interfaces Declarations-------------------*/
void BL_Print_Message(char *format ,...);
BL_Status BL_UART_Fetch_Host_Command (void);

#endif
