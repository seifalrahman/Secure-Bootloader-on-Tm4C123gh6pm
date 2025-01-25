#include "Wrapper_Functions.h"




HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *j, uint8_t *pData, uint16_t Size, uint32_t Timeout){
	char temp ;
	int i;
	for ( i =0 ; i< Size ; i++){
		temp = UART0_ReceiveByte();
		pData[i]=temp ;
	}
	
	return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart,
    													const uint8_t *pData,
																		uint16_t Size,
																		uint32_t Timeout){

    int i ;
    for ( i=0 ; i< Size ;i++){
        UART0_SendByte(pData[i]);
	}	

	return HAL_OK;
}
void CRC_init(){

	
}

													
uint32_t HAL_CRC_Accumulate(CRC_HandleTypeDef *ui32Base, unsigned char pBuffer[], unsigned char  BufferLength)
{
	unsigned long long ulValue =0;
	ulValue = Crc32(0xFFFFFFFF, pBuffer, BufferLength);
//	ulValue = Crc32(ulValue, &pBuffer[1], 1);
	//ulValue^= 0xFFFFFFFF;

    return ulValue;
}

void __HAL_CRC_DR_RESET(CRC_HandleTypeDef * ui32Base){
	
	
	
}


HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef  *pEraseInit , uint32_t *SectorError ) 
{
		uint32_t TypeErase = pEraseInit->TypeErase;
		uint32_t NbSectors = pEraseInit->NbSectors;
		uint32_t Sector		 = pEraseInit->Sector   ;
		int result =0;
	
    if (TypeErase == FLASH_TYPEERASE_SECTORS){
        uint32_t i;
        for( i=0; i< NbSectors; i++){
            result=FlashErase(Sector);
				}
    }else if(TypeErase == FLASH_TYPEERASE_MASSERASE){
        uint32_t i;
        for( i=0; i< FLASH_NUMBER_OF_BLOCKS; i++)
			{
            result=FlashErase(i);
    }
	}
		if(result==0){
			*SectorError =0xFFFFFFFFU ; 
		}else {
			*SectorError = 3;//random number it just means not successful
		}
		
		return HAL_OK ;
}






HAL_StatusTypeDef HAL_FLASH_Program(uint32_t Length, uint32_t Address, uint8_t * Data)
{

	HAL_StatusTypeDef Status ;
	int result = FlashProgram((uint32_t *)Data, Address, Length);
	if (result==0){
		Status =0x01 ;
	}else{
		Status= 0x00;
	}
	return Status;
}
