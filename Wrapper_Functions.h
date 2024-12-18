#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart.h"
#include "hw_memmap.h"
#include "crc.h"
#include "flash.h"
typedef enum 
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;


typedef enum {
		Hello 
}UART_HandleTypeDef ;
typedef enum {
		HelloO 
}CRC_HandleTypeDef ;

typedef struct
{
  uint32_t TypeErase;   /*!< Mass erase or sector Erase.
                             This parameter can be a value of @ref FLASHEx_Type_Erase */

  uint32_t Banks;       /*!< Select banks to erase when Mass erase is enabled.
                             This parameter must be a value of @ref FLASHEx_Banks */

  uint32_t Sector;      /*!< Initial FLASH sector to erase when Mass erase is disabled
                             This parameter must be a value of @ref FLASHEx_Sectors */

  uint32_t NbSectors;   /*!< Number of sectors to be erased.
                             This parameter must be a value between 1 and (max number of sectors - value of Initial sector)*/

  uint32_t VoltageRange;/*!< The device voltage range which defines the erase parallelism
                             This parameter must be a value of @ref FLASHEx_Voltage_Range */

} FLASH_EraseInitTypeDef;


#define FLASH_BANK_1   0
#define FLASH_VOLTAGE_RANGE_3 3
void CRC_init();
#define FLASH_TYPEERASE_SECTORS         0x00000000U  
#define FLASH_TYPEERASE_MASSERASE       0x00000001U 
#define FLASH_NUMBER_OF_BLOCKS					256
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *j, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout) ;

uint32_t HAL_CRC_Accumulate(CRC_HandleTypeDef *hcrc,
														uint32_t pBuffer[],
														uint32_t BufferLength) ;
void __HAL_CRC_DR_RESET(CRC_HandleTypeDef * ui32Base);

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef  *pEraseInit , uint32_t *SectorError )  ;
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)         ;
