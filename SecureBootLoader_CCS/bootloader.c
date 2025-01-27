#include "bootloader.h"

/*******Static Function Declarations**********/
static uint32_t programSize=0; 
static uint32_t programAddress=0 ;
static uint32_t CipherAddress =0;
static uint32_t HashAddress=0;\
static uint32_t size ;
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
//___________________________________________________________________________________________________________________
//___________________________________________CAUTION : e7tares 7etet 2l flag NOT TESTED _____________________________
//___________________________________________________________________________________________________________________
/*            ____                    _______           ____     ____      ___       ______
 * |   |     |       |       |       |       |         |    |   |         |   |     |      |  QUESTION :2l ba7r byd7ak leeeeeeeeeeh wana nazla atdalla3 amla 2l 2olal?
 * |___|     |____   |       |       |       |         |    |   |____     |___|     |______|  ANSWER   :2l ba7r 8adban mabyda77aksh asl 2l 7ekaya matda77aksh
 * |   |     |       |       |       |       |         |    |   |         |   |     |    \    QUESTION :Howwa 2l 7ob le3ba ?
 * |   |     |____   |____   |____   |_______|         |____|   |____     |   |     |     \   ANSWER   :AH
 *                                                                                            QUESTION :yaa fo2aaaaaady la tasal ayn 2l hawa ,Leh ?
 *                                                                                            ANSWER   :Kan Sar7an mn 5yaaal fa hawa
                                                                                              FULLMARK (3/3)
 */
//int FLAG=1;
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
static uint8_t Bootloader_Supported_CMDs[7] = {
    CBL_GET_VER_CMD,
    CBL_GET_HELP_CMD,
    CBL_GET_CID_CMD,
    CBL_AES_ENCRYPT_DECRYPT_CMD,
    CBL_GO_TO_ADDR_CMD,
    CBL_FLASH_ERASE_CMD,
    CBL_MEM_WRITE_CMD,

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
				case CBL_AES_ENCRYPT_DECRYPT_CMD:
					Bootloader_AES_ENCRYPT_DECRYPT(BL_Host_Buffer);
					Status = BL_OK;
					break;
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
        __asm(" CPSID I ");
        __asm (" MOVW r0 , #1  ");
        __asm(" LSL  r0 , r0 ,#16 ");
        __asm(" LDR  r1 , [r0, #0] ");
        __asm(" MSR MSP, r1 " );
        __asm(" ADD r0 ,r0 , #4 ");
        __asm(" LDR r1 ,[r0,#0]");
        __asm(" MOV PC , r1") ;


}


static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC){
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	uint32_t MCU_CRC_Calculated = 0;
	uint8_t Data_Counter = 0;

	MCU_CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE_OBJ, pData, Data_Len);

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
			Bootloader_Send_ACK(7) ;
			Bootloader_Send_Data_To_Host(Bootloader_Supported_CMDs,7);
			
		}else{
			Bootloader_Send_NACK();
			#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("CRC Verification Failed \r\n");
			#endif
		}
}

static Bootloader_AES_ENCRYPT_DECRYPT (uint8_t *Host_Buffer){
    struct tc_aes_key_sched_struct s ;
    const uint8_t nist_key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    const uint8_t iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    uint8_t ctr[16] = {
            0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
            0xfc, 0xfd, 0xfe, 0xff
            };

    tc_aes128_set_encrypt_key(&s, nist_key);
    uint16_t Host_CMD_Packet_Len =0;
    uint32_t Host_CRC32 =0;
    Host_CMD_Packet_Len = Host_Buffer[0]+1;
    Host_CRC32 = *((uint32_t *)((Host_Buffer+Host_CMD_Packet_Len)-CRC_TYPE_SIZE_BYTE)) ;
    if(CRC_VERIFICATION_PASSED==Bootloader_CRC_Verify( Host_Buffer,Host_CMD_Packet_Len-CRC_TYPE_SIZE_BYTE,Host_CRC32)){
        Bootloader_Send_ACK(16) ;

        int number_of_blocks = Host_Buffer[4] ;
        uint8_t out[32];
        uint8_t in[16] ;
        for (int i =0 ; i< 16 ; i++){
            in[i]=Host_Buffer[i+5] ;
        }


        if(AES_ENCRYPT==Host_Buffer[2]){
            switch (Host_Buffer[3]){
            case AES_CBC :
                tc_cbc_mode_encrypt(out, 16 + TC_AES_BLOCK_SIZE,
                                in, 16, iv, &s);
                break ;
            case AES_ECB :
                tc_aes_encrypt(out, in, &s) ;
                break;
            case AES_CTR :
                tc_ctr_mode(out, 16 , in,
                            16, ctr, &s) ;
                break;
            }
        }else if (AES_DECRYPT==Host_Buffer[2]){
            switch (Host_Buffer[3]){
            case AES_CBC :
                tc_cbc_mode_decrypt(out, 16, in, 16, iv, &s);
                break ;
            case AES_ECB :
                tc_aes_decrypt(out, in, &s) ;
                break;
            case AES_CTR :
                tc_ctr_mode(out,16, in,
                            16, ctr, &s);
                break;
            }

        }

      Bootloader_Send_Data_To_Host(out, 16);


    }else{
        Bootloader_Send_NACK();
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
	
	uint8_t* plainText ;
	
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
			  //  bootloader_jump_to_user_app();
			    Bootloader_Send_Data_To_Host(&Address_Verification, 1);



				size =  HashAddress-16-FLASH_SECTOR2_BASE_ADDRESS ;


				plainText= (uint8_t*) calloc(size,sizeof(uint8_t)) ;
				//uint8_t TESTPLAINTEXT[932] ;
				memset(plainText,0,size);
				uint8_t StoredHashValue [TC_SHA256_DIGEST_SIZE] ;

				for (uint32_t i =0 ; i< size ; i++){

				    uint8_t temp =   *( uint8_t*)((( uint8_t*)(FLASH_SECTOR2_BASE_ADDRESS))+i) ;

                    temp =   *( uint8_t*)((( uint8_t*)(FLASH_SECTOR2_BASE_ADDRESS))+i) ;


				    plainText[i] =   temp ;
				   // TESTPLAINTEXT[i] =  *( uint8_t*)((( uint8_t*)(FLASH_SECTOR2_BASE_ADDRESS))+i) ;
				}
                struct tc_sha256_state_struct s;
                uint8_t hash[TC_SHA256_DIGEST_SIZE];
                tc_sha256_init(&s);
                tc_sha256_update(&s, plainText, size );
                tc_sha256_final(hash, &s);
                /////////////////////////////////////////////////
//                const uint8_t key[16] = {
//                        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
//                        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
//                    };
//                struct tc_cmac_struct state;
//                struct tc_aes_key_sched_struct sched;
//                tc_cmac_setup(&state, key, &sched);
//                uint8_t Tag[16] ;
//                const uint8_t msg[16] = {
//                        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
//                        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
//                    };
//
//                tc_cmac_init(&state);
//                tc_cmac_update(&state, msg, sizeof(msg));
//                tc_cmac_final(Tag, &state);
                /////////////////////////////////////////////////
//                uint8_t hash2[TC_SHA256_DIGEST_SIZE];
//                tc_sha256_init(&s);
//                tc_sha256_update(&s, TESTPLAINTEXT, size );
//                tc_sha256_final(hash2, &s);

                int Secured =1;
                for (uint32_t i =0 ; i<TC_SHA256_DIGEST_SIZE ;i++ ){
                    StoredHashValue[i]= *( uint8_t*)(( (( uint8_t*)(HashAddress))+i ));
                    if(StoredHashValue[i]!=hash[i]){
                        Secured=0;
                        break ;

                    }
                }

                if(1==Secured){

				bootloader_jump_to_user_app();
                }else {
                    BL_Print_Message("Code has been illegaly manipulated  \r\n");//TRY TO CALCULATE THE DIGEST ON THE OTHER APPLICATIO  ---> FLAG+=(1%2)
                }
                free (plainText) ;




				
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
        int i =Sector_Number ;
        int Erase_Validity_Status=0;

        if (Number_Of_Sectors!=0){
            for (; i<(Number_Of_Sectors+Sector_Number) ;i++){
                Erase_Validity_Status= FlashErase(i*SECTOR_SIZE) ;
                if(-1==Erase_Validity_Status){
                    Erase_Validity_Status= 2 ;
                    break ;
                }else{
                    Erase_Validity_Status= 3 ;
                }
            }

        }else {
            int i =0 ;
            int numberOfAppSectors = (size/SECTOR_SIZE)+1 ;
            for (; i<(numberOfAppSectors) ;i++){
                int num= ((0x10000)/SECTOR_SIZE)*SECTOR_SIZE ;
                Erase_Validity_Status=FlashErase((num)+i*SECTOR_SIZE) ;
                if(-1==Erase_Validity_Status){
                    Erase_Validity_Status= 2 ;
                    break ;
                }else{
                    Erase_Validity_Status= 3 ;
                }
            }

        }


		return Erase_Validity_Status;
}



static void Bootloader_Erase_Flash(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
  uint32_t Host_CRC32 = 0;
	uint8_t Erase_Status = 0;

	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));		
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){


		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
		/* Perform Mass erase or sector erase of the user flash */
//		FlashErase(0x10240) ;
//		uint32 * ptr =(uint32 * )(0x10240) ;
//
		Erase_Status = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
		if(SUCCESSFUL_ERASE == Erase_Status){
			/* Report erase Passed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
		}
		else{
			/* Report erase failed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
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

	    if(Payload_Len % 4 == 0){

	        Flash_Payload_Write_Status = HAL_FLASH_Program(Payload_Len, Payload_Start_Address, Host_Payload);
	    }else if (201 == Payload_Len) {
	        Flash_Payload_Write_Status = HAL_FLASH_Program(32, Payload_Start_Address, Host_Payload);
	        HashAddress= Payload_Start_Address;
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
		//Perform_Flash_Erase(0, 0) ;
		/* Extract the start address from the Host packet */
		HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
//		if (HOST_Address== FLASH_SECTOR2_BASE_ADDRESS){
//		    FLAG=1 ;
//		}else if (FLASH_SECTOR2_BASE_ADDRESS2 == HOST_Address){
//		    FLAG=2 ;
//		}

		
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("HOST_Address = 0x%X \r\n", HOST_Address);
#endif
		/* Extract the payload length from the Host packet */
		Payload_Len = Host_Buffer[6];
		
		
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Address_Verification(HOST_Address);
		if(ADDRESS_IS_VALID == Address_Verification){
		    Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], HOST_Address, Payload_Len);

			if(FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status){


				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);

			}
			else{
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





	
