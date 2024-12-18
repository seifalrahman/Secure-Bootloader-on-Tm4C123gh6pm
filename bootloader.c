#include "bootloader.h"

/*******Static Function Declarations**********/
static uint32_t programSize=0; 
static uint32_t programAddress=0 ;
static uint32_t CipherAddress =0;
UART_HandleTypeDef huart2;
CRC_HandleTypeDef  hcrc  ;
static uint8_t Bootloader_CRC_Verify(uint8_t *pData , uint32_t Data_Len , uint32_t Host_CRC);
static void Bootloader_Send_ACK(uint16_t Replay_Len);
static void Bootloader_Send_NACK();

static void Bootloader_Get_Version(uint8_t *Host_Buffer);
static void Bootloader_Get_Help(uint8_t *Host_Buffer);
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
//static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
/*static void Bootloader_Enable_RW_Protection(uint8_t *Host_Buffer);

static void Bootloader_Memory_Read(uint8_t *Host_Buffer);
static void Bootloader_Get_Sector_Protection_Status(uint8_t *Host_Buffer);
static void Bootloader_Read_OTP(uint8_t *Host_Buffer);
static void Bootloader_Disable_RW_Protection(uint8_t *Host_Buffer);

*/
uint32_t AES_CTR_128_KEY []={
	0x302B493D,
	0x656E3646,
	0x57337643,
	0x71465138,
	0x4E396D63,
	0x73423834,
	0x4B717763,
	0x41625166,
	0x7250506D,
	0x34654C69,
	0x32646947,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};

uint32_t AES_CBC_IV []={
	0x01988700,
	0x18fe1dc8,
	0xb7957618,
	0x517be8d1

};




uint32_t AES_CTR_IV []={
	0x746d0225,
	0xa60a2296,
	0xbea794f7,
	0xa0e630aa
	
};

uint32_t AES_CBC_128_KEY []={
	0x4D757A4F,
	0x6C63694D,
	0x62542B73,
	0x4D464730,
	0x6B6F4748,
	0x65325476,
	0x41652B55,
	0x414D5455,
	0x3141526A,
	0x33766A38,
	0x366F3149,
	0x34763442,
	0x6E4D376F,
	0x36517854,
	0x5A534A4F,
	0x58794642
};

uint32_t AES_ECB_128_KEY []={
	0xf34132e3,
	0xb9935321,
	0x63ba9073,
	0xfa3f3062,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};




/*****************Static Variables******************/
static uint8_t BL_Host_Buffer[BL_HOST_BUFFER_RX_LENGTH] ;
static uint8_t Bootloader_Supported_CMDs[12] = {
    CBL_GET_VER_CMD,
    CBL_GET_HELP_CMD,
    CBL_GET_CID_CMD,
    CBL_GET_RDP_STATUS_CMD,
    CBL_GO_TO_ADDR_CMD,
    CBL_FLASH_ERASE_CMD,
    CBL_MEM_WRITE_CMD,
    CBL_EN_R_W_PROTECT_CMD,
    CBL_MEM_READ_CMD,
    CBL_READ_SECTOR_STATUS_CMD,
    CBL_OTP_READ_CMD,
    CBL_DIS_R_W_PROTECT_CMD
};
void BL_Print_Message(char *format ,...){
//	HAL_UART_Transmit(&huart2,Message_1 , sizeof(Message_1), HAL_MAX_DELAY);
		char Message [100] ={0} ;
		va_list args ;
		va_start(args,format);
		vsprintf(Message,format,args);
		
		#if ( BL_DEBUG_METHOD == BL_ENABLE_UART_DEBUG_MESSAGE)
		HAL_UART_Transmit(BL_DEBUG_UART,(uint8_t*)Message , sizeof(Message), HAL_MAX_DELAY);
		#elif ( BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE)
		
		#elif ( BL_DEBUG_METHOD == BL_ENABLE_SPI_DEBUG_MESSAGE)
		
		#endif
		va_end(args);
}




BL_Status BL_UART_Fetch_Host_Command (){
	BL_Status Status = BL_NACK ;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;
	uint8_t Data_Length=0;
	
	memset(BL_Host_Buffer,0,BL_HOST_BUFFER_RX_LENGTH);
	HAL_Status= HAL_UART_Receive(BL_HOST_COMMUNICATION_UART,BL_Host_Buffer,1,HAL_MAX_DELAY);
	if(HAL_Status!= HAL_OK ){
		Status = BL_NACK ;
	}else{
		Data_Length = BL_Host_Buffer[0];
		HAL_Status= HAL_UART_Receive(BL_HOST_COMMUNICATION_UART,&BL_Host_Buffer[1],Data_Length,HAL_MAX_DELAY);
		
		if(HAL_Status!= HAL_OK ){
			Status = BL_NACK ;
			}else{
					switch(BL_Host_Buffer[1]){
				case CBL_GET_VER_CMD:
					Bootloader_Get_Version(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_GET_HELP_CMD:
					Bootloader_Get_Help(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_GET_CID_CMD:
					Bootloader_Get_Chip_Identification_Number(BL_Host_Buffer);
					Status = BL_OK;
					break;
			/*	case CBL_GET_RDP_STATUS_CMD:
					Bootloader_Read_Protection_Level(BL_Host_Buffer);
					Status = BL_OK;
					break;*/
				case CBL_GO_TO_ADDR_CMD:
					Bootloader_Jump_To_Address(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_FLASH_ERASE_CMD:
					Bootloader_Erase_Flash(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_MEM_WRITE_CMD:
					Bootloader_Memory_Write(BL_Host_Buffer);
					Status = BL_OK;
					break;
					/*case CBL_EN_R_W_PROTECT_CMD:
					BL_Print_Message("Enable or Disable write protect on different sectors of the user flash \r\n");
					Bootloader_Enable_RW_Protection(BL_Host_Buffer);
					Status = BL_OK;
					break;
			case CBL_MEM_READ_CMD:
					BL_Print_Message("Read data from different memories of the microcontroller \r\n");
					Bootloader_Memory_Read(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_READ_SECTOR_STATUS_CMD:
					BL_Print_Message("Read all the sector protection status \r\n");
					Bootloader_Get_Sector_Protection_Status(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_OTP_READ_CMD:
					BL_Print_Message("Read the OTP contents \r\n");
					Bootloader_Read_OTP(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_DIS_R_W_PROTECT_CMD:
					Bootloader_Disable_RW_Protection(BL_Host_Buffer);
					Status = BL_OK;
					break;
				*/
				default:
					BL_Print_Message("Invalid command code received from host !! \r\n");
					break;
			}
		}
	}
	
	return Status ;
}


static void bootloader_jump_to_user_app(void){
		uint32_t MSP_Value = *((volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS) ;
		uint32_t MainAppAddr = *((volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS+4));
		pMainApp ResetHandler  = (pMainApp)MainAppAddr;
		
	
		__asm(" CPSID I ");
		__asm("MOV r0, %[input]\n"
      "MSR MSP, r0"
      :
      : [input] "r" (MSP_Value)
      : "r0");
		/*DEINIT MODULES*/
		//Se it later***********************************
    //HAL_RCC_DeInit() ;
		ResetHandler();
}


static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC){
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	uint32_t MCU_CRC_Calculated = 0;
	uint8_t Data_Counter = 0;
	uint32_t Data_Buffer = 0;
	/* Calculate CRC32 */
	for(Data_Counter = 0; Data_Counter < Data_Len; Data_Counter++){
		Data_Buffer = (uint32_t)pData[Data_Counter];
		MCU_CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE_OBJ, &Data_Buffer, 1);
	}
	/* Reset the CRC Calculation Unit */
  __HAL_CRC_DR_RESET(CRC_ENGINE_OBJ);
	/* Compare the Host CRC and Calculated CRC */
	if(MCU_CRC_Calculated == Host_CRC){
		CRC_Status = CRC_VERIFICATION_PASSED;
	}
	else{
		CRC_Status = CRC_VERIFICATION_FAILED;
	}
	
	return CRC_Status;
}


static void Bootloader_Send_ACK(uint16_t Replay_Len){
	uint8_t Ack_Value[2]={0};
	Ack_Value[0]= CBL_SEND_ACK;
	Ack_Value[1]=Replay_Len;
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,Ack_Value,2,HAL_MAX_DELAY);
	
}
static void Bootloader_Send_NACK(){
	uint8_t Ack_Value=CBL_SEND_NACK;

	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,&Ack_Value,1,HAL_MAX_DELAY);
}


static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len){
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}


static void Bootloader_Get_Version(uint8_t *Host_Buffer){
		uint8_t BL_Version[4] ={CBL_VENDOR_ID,CBL_SW_MAJOR_VERSION,CBL_SW_MINOR_VERSION,CBL_SW_PATCH_VERSION};
		uint16_t Host_CMD_Packet_Len =0;
		uint32_t Host_CRC32 =0;
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Read the bootloader version from the MCU \r\n");
		#endif
		
		Host_CMD_Packet_Len = Host_Buffer[0]+1;
		Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE)) ;
		if(CRC_VERIFICATION_PASSED==Bootloader_CRC_Verify( Host_Buffer,Host_CMD_Packet_Len-CRC_TYPE_SIZE_BYTE,Host_CRC32)){
				//Send ACK
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Passed \r\n");
			#endif
			Bootloader_Send_ACK(4) ;
			Bootloader_Send_Data_To_Host((uint8_t *)(&BL_Version[0]), 4);
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Bootloader Ver. %d.%d.%d \r\n",BL_Version[1],BL_Version[2],BL_Version[3]);
			#endif
		}else{
			Bootloader_Send_NACK();
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Failed \r\n");
			#endif
		}
}


static void Bootloader_Get_Help(uint8_t *Host_Buffer){
		uint16_t Host_CMD_Packet_Len =0;
		uint32_t Host_CRC32 =0;
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Read the command supported by the bootloader \r\n");
		#endif
		Host_CMD_Packet_Len = Host_Buffer[0]+1;
		Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE)) ;
		if(CRC_VERIFICATION_PASSED==Bootloader_CRC_Verify( Host_Buffer,Host_CMD_Packet_Len-CRC_TYPE_SIZE_BYTE,Host_CRC32)){
				//Send ACK
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Passed \r\n");
			#endif
			Bootloader_Send_ACK(12) ;
			Bootloader_Send_Data_To_Host(Bootloader_Supported_CMDs,12);
			
		}else{
			Bootloader_Send_NACK();
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Failed \r\n");
			#endif
		}
}





static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer){
		uint16_t Host_CMD_Packet_Len =0;
		uint32_t Host_CRC32 =0;
		uint16_t MCU_Identification_Number=0;
		#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Read the MCU chip identification number \r\n");
		#endif
		Host_CMD_Packet_Len = Host_Buffer[0]+1;
		Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE)) ;
		if(CRC_VERIFICATION_PASSED==Bootloader_CRC_Verify( Host_Buffer,Host_CMD_Packet_Len-CRC_TYPE_SIZE_BYTE,Host_CRC32)){
				//Send ACK
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Passed \r\n");
			#endif
			/*Get chip identification number*/
			MCU_Identification_Number=((uint16_t)(((NVIC_CPUID_R)&NVIC_CPUID_PARTNO_M))&0x00000FFF) ;
			/*Report chip identification number */
			Bootloader_Send_ACK(2) ;
			Bootloader_Send_Data_To_Host((uint8_t *)(&MCU_Identification_Number),2);
			
		}else{
			Bootloader_Send_NACK();
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Failed \r\n");
			#endif
		}
		 

}






	static uint8_t Host_Address_Verification (uint32_t Jump_Address){                                                                                                                                    		
		uint8_t Address_Verification = ADDRESS_IS_INVALID;
		 if ((Jump_Address >= 0) && (Jump_Address <= TM4C123GH6PM_FLASH_END) ){
			Address_Verification= ADDRESS_IS_VALID;
		}else {
			Address_Verification = ADDRESS_IS_INVALID;
		} 
		
		return Address_Verification;
		
	}
	

	
	static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 =0 ;
	uint32_t Host_Jump_Address ;
	int i =0 ;
	unsigned char Secured =1 ;
	uint8_t Address_Verification ;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Read the MCU chip identification number \r\n");
#endif
		Host_CMD_Packet_Len = Host_Buffer[0]+1;
		Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE)) ;
		if(CRC_VERIFICATION_PASSED==Bootloader_CRC_Verify( Host_Buffer,Host_CMD_Packet_Len-CRC_TYPE_SIZE_BYTE,Host_CRC32)){
				//Send ACK
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Passed \r\n");
			#endif
			Bootloader_Send_ACK(1) ;
			Host_Jump_Address= *((uint32_t*)&Host_Buffer[2]);
			/*Verify the Extracted address to be valid address*/
			Address_Verification = Host_Address_Verification(Host_Jump_Address);
			if(ADDRESS_IS_VALID==Address_Verification){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Address Verification Succeeded \r\n");
#endif
				Bootloader_Send_Data_To_Host(&Address_Verification, 1);
				Jump_Ptr Jump_Address =(Jump_Ptr)(Host_Jump_Address+1);
			/*	
			uint32_t * CipherText = (uint32_t *) malloc( ((Payload_Len/4)+1)*sizeof(uint32_t) ) ;
			
			AESDataProcess(AES_BASE,(uint32_t *) Host_Buffer,
										CipherText , ((Payload_Len/4)+1) ) ;
			*/
				
				uint32_t* plainText = (uint32_t *) malloc( ((programSize/4)+1)*sizeof(uint32_t) ) ;
				for (i=0 ; i< programSize ; i++){
						*( ((uint8_t *)plainText) +i ) =  *(((uint8_t *)programAddress)+i) ;
				}
				uint32_t * CipherText = (uint32_t *) malloc( ((programSize/4)+1)*sizeof(uint32_t) ) ;

				AESDataProcess(AES_BASE,(uint32_t *) plainText,
									  CipherText , ((programSize/4)+1) ) ;
				
				for (i=0 ; i< ((programSize/4)+1) ; i++){
						if(CipherText[i]!=*(( (uint32_t*) CipherAddress )+ i)    ){
								Secured=0;
								break;
						}
				}
				
				
				
				
				if(Secured==1)
					Jump_Address ();
				else
					BL_Print_Message("ERROR IN DATA INTEGRITY. \r\n");
				
			}else{
				Bootloader_Send_Data_To_Host(&Address_Verification, 1);
			}
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Bootloader Ver. \r\n");
			#endif
		}else{
			Bootloader_Send_NACK();
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Failed \r\n");
			#endif
		}
	
}
	



static uint8_t Perform_Flash_Erase (uint8_t Sector_Number , uint8_t Number_Of_Sectors){
	uint8_t Sector_Validity_Status = INVALID_SECTOR_NUMBER ;
	FLASH_EraseInitTypeDef pEraseInit ;
	uint8_t Remaining_Sectors =0 ;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;
	uint32_t SectorError=0;
	if(Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER){
			Sector_Validity_Status = INVALID_SECTOR_NUMBER ;
	}else{
			if((Sector_Number <= (CBL_FLASH_MAX_SECTOR_NUMBER-1))||(CBL_FLASH_MASS_ERASE==Sector_Number) ){
				
				if(CBL_FLASH_MASS_ERASE == Sector_Number){
						pEraseInit.TypeErase= FLASH_TYPEERASE_MASSERASE;
				}else{
					
						Remaining_Sectors =CBL_FLASH_MAX_SECTOR_NUMBER - Sector_Number ;
						if(Number_Of_Sectors > Remaining_Sectors){
							Number_Of_Sectors=Remaining_Sectors ;
						}else{/*Nothing*/}
					
						pEraseInit.TypeErase= FLASH_TYPEERASE_SECTORS;
						pEraseInit.Sector=Sector_Number;
						pEraseInit.NbSectors=Number_Of_Sectors;
						
						
	//					HAL_Status = HAL_FLASH_Unlock();
						
						HAL_Status=HAL_FLASHEx_Erase(&pEraseInit,&SectorError );
						if(HAL_SUCCESSFUL_ERASE==SectorError){
								Sector_Validity_Status= SUCCESSFUL_ERASE;
						}else{
								Sector_Validity_Status= UNSUCCESSFUL_ERASE;
						}
//						HAL_Status = HAL_FLASH_Lock();

				
				}
				pEraseInit.Banks=FLASH_BANK_1 ;//only one bank 
				pEraseInit.VoltageRange= FLASH_VOLTAGE_RANGE_3;/*Device operating range*/
				
				
			}else{}
		
				
	}
		
		
		return Sector_Validity_Status;
}



static void Bootloader_Erase_Flash(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
  uint32_t Host_CRC32 = 0;
	uint8_t Erase_Status = 0;
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Mass erase or sector erase of the user flash \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));		
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
		/* Perform Mass erase or sector erase of the user flash */
		Erase_Status = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
		if(SUCCESSFUL_ERASE == Erase_Status){
			/* Report erase Passed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Successful Erase \r\n");
#endif
		}
		else{
			/* Report erase failed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Erase request failed !!\r\n");
#endif
		}
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_Send_NACK();
	}
}



static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len){
	
	
	
	
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Payload_Counter = 0;
	
	/* Unlock the FLASH control register access */
  HAL_Status = HAL_OK;
	
	if(HAL_Status != HAL_OK){
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	else{
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter++){
			/* Program a byte at a specified address */
			HAL_Status = HAL_FLASH_Program(1, Payload_Start_Address + Payload_Counter, Host_Payload[Payload_Counter]);
			
			if(HAL_Status != HAL_OK){
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
				break;
			}
			else{
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
			}
		}
	}
	
	if((FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) && (HAL_OK == HAL_Status)){
		/* Locks the FLASH control register access */
		HAL_Status = HAL_OK;
		if(HAL_Status != HAL_OK){
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
		}
		else{
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
		}
	}
	else{
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	
	return Flash_Payload_Write_Status;
}





uint32_t CodeAddress [200] ;





static void Bootloader_Memory_Write(uint8_t *Host_Buffer){
	
	uint16_t Host_CMD_Packet_Len = 0;
  uint32_t Host_CRC32 = 0;
	uint32_t HOST_Address = 0;
	uint8_t Payload_Len = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	uint8_t Flash_Payload_Write_Status =   FLASH_PAYLOAD_WRITE_FAILED;
	uint8_t Flash_Payload_Write_Status2 =  FLASH_PAYLOAD_WRITE_FAILED;
	
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Write data into different memories of the MCU \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));	
/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
		/* Extract the start address from the Host packet */
		HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////
		uint32_t AES_ui32Config=0;
		AES_ui32Config|=(AES_CFG_DIR_ENCRYPT|AES_CFG_KEY_SIZE_128BIT
										|AES_ENCRYPTION_MODE);// E7tares mo4 3aref dh configs kfaya walla la2
		AESConfigSet(AES_BASE,AES_ui32Config);
		
		#if (AES_ENCRYPTION_MODE == AES_CBC_MODE)
		AESKey1Set(AES_BASE,AES_CBC_128_KEY,AES_CFG_KEY_SIZE_128BIT);
		AESIVSet(AES_BASE, AES_CBC_IV) ;
		#elif (AES_ENCRYPTION_MODE == AES_ECB_MODE)
		AESKey1Set(AES_BASE,AES_ECB_128_KEY,AES_CFG_KEY_SIZE_128BIT);
		
		#elif (AES_ENCRYPTION_MODE == AES_CTR_MODE)
		AESKey1Set(AES_BASE,AES_CTR_128_KEY,AES_CFG_KEY_SIZE_128BIT);
		AESIVSet(AES_BASE, AES_CTR_IV) ;
		#endif 
		
		
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////
		
		
		
		
		
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("HOST_Address = 0x%X \r\n", HOST_Address);
#endif
		/* Extract the payload length from the Host packet */
		Payload_Len = Host_Buffer[6];
		
		
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Address_Verification(HOST_Address);
		if(ADDRESS_IS_VALID == Address_Verification){
			/* Write the payload to the Flash memory */
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], HOST_Address, Payload_Len);
			
			uint32_t * CipherText = (uint32_t *) malloc( ((Payload_Len/4)+1)*sizeof(uint32_t) ) ;
			
			AESDataProcess(AES_BASE,(uint32_t *) Host_Buffer,
										CipherText , ((Payload_Len/4)+1) ) ;
			programSize   =Payload_Len;
			programAddress=HOST_Address;
			CipherAddress =HOST_Address +Payload_Len +50 ;
			Flash_Payload_Write_Status2=Flash_Memory_Write_Payload( (uint8_t*)CipherText , CipherAddress , Payload_Len );
			
			free(CipherText);
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////			
			
			if(FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status){
				/* Report payload write passed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Payload Valid \r\n");
#endif
			}
			else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Payload InValid \r\n");
#endif
				/* Report payload write failed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
			}
		}
		else{
			/* Report address verification failed */
			Address_Verification = ADDRESS_IS_INVALID;
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_Send_NACK();
	}	
	
}





	