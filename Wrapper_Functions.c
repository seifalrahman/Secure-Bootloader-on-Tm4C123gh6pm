#include "Wrapper_Functions.h"




HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *j, uint8_t *pData, uint16_t Size, uint32_t Timeout){
	char temp ;
	for (int i =0 ; i< Size ; i++){
		temp = UARTCharGet(UART0_BASE);
		pData[i]=temp ;
	}
	
	return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart,
																		const uint8_t *pData,
																		uint16_t Size,
																		uint32_t Timeout){
	for (int i=0 ; i< Size ;i++){
			UARTCharPut(UART0_BASE,pData[i]);
	}	

	return HAL_OK;
}
void CRC_init(){
	uint32_t ui32CRCConfig=0 ;
	ui32CRCConfig=(CRC_CFG_TYPE_P4C11DB7 |
								 CRC_CFG_INIT_SEED     |
	               CRC_CFG_SIZE_32BIT    ) ;
	
	uint32_t ui32Seed = 0xFFFFFFFF;
	CRCConfigSet(CCM0_BASE , ui32CRCConfig) ;
	CRCSeedSet  (CCM0_BASE ,ui32Seed )      ;
	
}

													
uint32_t HAL_CRC_Accumulate(CRC_HandleTypeDef *ui32Base, uint32_t pBuffer[], uint32_t BufferLength)
{
    uint32_t crcResult;

    crcResult = CRCDataProcess(CCM0_BASE, pBuffer, BufferLength, true);

    return crcResult;
}

void __HAL_CRC_DR_RESET(CRC_HandleTypeDef * ui32Base){
	
	uint32_t ui32Seed = 0xFFFFFFFF					;
	CRCSeedSet  (CCM0_BASE ,ui32Seed )       ;
	
}


HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef  *pEraseInit , uint32_t *SectorError ) 
{
		uint32_t TypeErase = pEraseInit->TypeErase;
		uint32_t NbSectors = pEraseInit->NbSectors;
		uint32_t Sector		 = pEraseInit->Sector   ;
		int result =0;
	
    if (TypeErase == FLASH_TYPEERASE_SECTORS){
        for(uint32_t i=0; i< NbSectors; i++){
            result=FlashErase(Sector);
				}
    }else if(TypeErase == FLASH_TYPEERASE_MASSERASE){
        for(uint32_t i=0; i< FLASH_NUMBER_OF_BLOCKS; i++)
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






HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
	HAL_StatusTypeDef Status ;
	int result = FlashProgram(&Data, Address, 1);
	if (result==0){
		Status =HAL_OK ;
	}else{
		Status= HAL_ERROR;
	}
	return Status;
}