#include "bootloader.h"
#include "Mocks.h"
#include "inc/tm4c123gh6pm.h"
#include <stdarg.h>
extern uint8_t test_status;
mockHashVerification_Process HashMock={false};
uint8_t* plainText ;
static uint32_t programSize=0;
static uint32_t programAddress=0 ;
static uint32_t CipherAddress =0;
static uint32_t HashAddress=0;
static uint32_t size ;
#define TC_SHA256_DIGEST_SIZE (32)
#define SECTOR_SIZE            1024
// UnderTest Version of BL_Print_Message
void BL_Print_Message_UnderTest(char *format ,...) {
    // Message buffer to store formatted message
    char Message[100] = {0};

    // Use va_list to handle variable arguments
    va_list args;
    va_start(args, format);
    vsprintf(Message, format, args);

    // Instead of transmitting the message over UART, we store it in a global test buffer for validation
    store_message_for_testing(Message);  // You would write this function to handle storing the message

    va_end(args);
}
// Under-Test Version: This one calls the mocks for testing
BL_Status BL_UART_Fetch_Host_Command_UnderTest() {
    BL_Status Status = BL_NACK;
    HAL_StatusTypeDef HAL_Status = HAL_ERROR;
    uint8_t Data_Length = 0;

    // Clear the buffer
    memset(mock_BL_Buffer, 0, BL_HOST_BUFFER_RX_LENGTH);

    // Use the mocked HAL_UART_Receive instead of the real one
    HAL_Status = mock_HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, mock_BL_Buffer, 1, HAL_MAX_DELAY);

    if (HAL_Status != HAL_OK) {
        Status = BL_NACK;
    } else {
        Data_Length = mock_BL_Buffer[0];

        // Again, use the mocked function to simulate UART receive
        HAL_Status = mock_HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, &mock_BL_Buffer[1], Data_Length, HAL_MAX_DELAY);

        if (HAL_Status != HAL_OK) {
            Status = BL_NACK;
        } else {
            // Process the command from the buffer, assuming that these Bootloader functions are mocked too
            switch (mock_BL_Buffer[1]) {
                case CBL_GET_VER_CMD:
                    mock_Bootloader_Get_Version();  // Mocked Bootloader function
                    Status = BL_OK;
                    break;
                case CBL_GET_HELP_CMD:
                    mock_Bootloader_Get_Help();     // Mocked Bootloader function
                    Status = BL_OK;
                    break;
                case CBL_GET_CID_CMD:
                    mock_Bootloader_Get_Chip_Identification_Number(); // Mocked
                    Status = BL_OK;
                    break;
                case CBL_GO_TO_ADDR_CMD:
                    mock_Bootloader_Jump_To_Address();  // Mocked
                    Status = BL_OK;
                    break;
                case CBL_FLASH_ERASE_CMD:
                    mock_Bootloader_Erase_Flash(); // Mocked
                    Status = BL_OK;
                    break;
                case CBL_MEM_WRITE_CMD:
                    mock_Bootloader_Memory_Write(); // Mocked
                    Status = BL_OK;
                    break;
                default:
                    BL_Print_Message_UnderTest("Invalid command code received from host !! \r\n"); // Mocked Print Message
                    break;
            }
        }
    }

    return Status;
}

// This version is for testing only
uint8_t Bootloader_CRC_Verify_UnderTesting(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC){
    uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
    uint32_t MCU_CRC_Calculated = 0;

    // Use the injected mock function for CRC calculation
    MCU_CRC_Calculated = mock_HAL_CRC_Accumulate(CRC_ENGINE_OBJ, pData, Data_Len);

    // Compare calculated CRC with Host CRC
    if(MCU_CRC_Calculated == Host_CRC){
        CRC_Status = CRC_VERIFICATION_PASSED;
    }

    return CRC_Status;
}
void Bootloader_Send_Data_To_Host_UnderTesting(uint8_t *Host_Buffer, uint32_t Data_Len){
	mock_HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}
void Bootloader_Get_Version_UnderTesting(uint8_t *mock_Host_Buffer){
	uint8_t BL_Version[4] = { CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION, CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION };
	uint16_t Host_CMD_Packet_Len = 0;
    uint32_t Host_CRC32 = 0;

	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = mock_Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((mock_Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == mock_Bootloader_CRC_Verify((uint8_t *)&mock_Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
        //if we are in a scenerio where CRC matches user
        test_status = 1; // To mark successful scenerio
        //it means that crc verified and commands are sent through UART with ACK
	}
	else{
        test_status = 0;
        //it means that crc verification failed and NACK sent
    }
}
void Bootloader_Get_Help_UnderTesting(uint8_t *mock_Host_Buffer)
{
    uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = mock_Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((mock_Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == mock_Bootloader_CRC_Verify((uint8_t *)&mock_Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
        //if we are in a scenerio where CRC matches user
        test_status = 1; // To mark successful scenerio
        //it means that crc verified and commands are sent through UART with ACK
	}
	else{
        test_status = 0;
        //it means that crc verification failed and NACK sent
	}
}
void Bootloader_Get_Chip_Identification_Number_UnderTesting(uint8_t *mock_Host_Buffer)
{
	uint16_t Host_CMD_Packet_Len = 0;
    uint32_t Host_CRC32 = 0;
	uint16_t MCU_Identification_Number = 0;
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = mock_Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((mock_Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == mock_Bootloader_CRC_Verify((uint8_t *)&mock_Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
        //if we are in a scenerio where CRC matches user
        test_status = 1; // To mark successful scenerio
	}
	else{
        test_status = 0;
	}
}
uint8_t Host_Address_Verification_UnderTesting (uint32_t Jump_Address){
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	 if ((Jump_Address >= 0) && (Jump_Address <= TM4C123GH6PM_FLASH_END) ){
		Address_Verification= ADDRESS_IS_VALID;
	}else {
		Address_Verification = ADDRESS_IS_INVALID;
	}
	return Address_Verification;
}

// Under-Test Function
void Bootloader_Jump_To_Address_UnderTesting(uint8_t *Host_Buffer) {
    uint16_t Host_CMD_Packet_Len = 0;
    uint32_t Host_CRC32 = 0;
    uint32_t Host_Jump_Address;
    uint8_t Address_Verification;
    uint32_t size;
    uint8_t *plainText = NULL;

    Host_CMD_Packet_Len = Host_Buffer[0] + 1;
    Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));

    if (CRC_VERIFICATION_PASSED == mock_Bootloader_CRC_Verify(Host_Buffer, Host_CMD_Packet_Len - CRC_TYPE_SIZE_BYTE, Host_CRC32)) {
        //MOCK->Bootloader_Send_ACK(1);
        Host_Jump_Address = *((uint32_t *)&Host_Buffer[2]);
        test_status = 1;
        // Mocked verification of the extracted address
        Address_Verification = mock_Host_Address_Verification(Host_Jump_Address);

        if (ADDRESS_IS_VALID == Address_Verification) {
            //MOCK->Bootloader_Send_Data_To_Host(&Address_Verification, 1);
            //MOCK for remainings we just need to test the point it will reach all others are mocks
            test_status = 2;
            int Secured = 1;

            //mock for hash validation
            if(HashMock.HashTag_Force_unmatch)
            {
                Secured = 0;
            }else
            {
                Secured = 1;
            }
            ///////////////////////////
            if (1 == Secured) {
                test_status = 3;
                //MOCK->bootloader_jump_to_user_app();
            } else {
                //MOCK->BL_Print_Message("Code has been illegally manipulated\r\n");
                test_status = 4;
            }
        } else {
            //MOCK->Bootloader_Send_Data_To_Host(&Address_Verification, 1);
            test_status = 5;
            //ERROR STATE in adress verification
        }
    } else {
            //MOCK->Bootloader_Send_NACK();
            test_status = 6;
            //ERROR STATE in CRC Check
    }
}
// Under-Test Function
 uint8_t Perform_Flash_Erase_UnderTesting(uint8_t Sector_Number, uint8_t Number_Of_Sectors) {
    int i = Sector_Number;
    int Erase_Validity_Status = 0;

    // Test with valid number of sectors
    if (Number_Of_Sectors != 0) {
        for (; i < (Number_Of_Sectors + Sector_Number); i++) {
            Erase_Validity_Status = mock_FlashErase(i * SECTOR_SIZE);

            // Mock validation for FlashErase return value
            if (-1 == Erase_Validity_Status) {
                Erase_Validity_Status = 2;
                test_status = 0;
                break;
            } else {
                Erase_Validity_Status = 3;
                test_status = 1;
            }
        }
    } else {
        // Case where Number_Of_Sectors is 0
        // Delete the entire application
        int i = 0;
        int numberOfAppSectors = (size / SECTOR_SIZE) + 1;

        for (; i < (numberOfAppSectors); i++) {
            int num = ((0x10000) / SECTOR_SIZE) * SECTOR_SIZE;
            Erase_Validity_Status = mock_FlashErase((num) + i * SECTOR_SIZE);

            // Mock validation for FlashErase return value
            if (-1 == Erase_Validity_Status) {
                Erase_Validity_Status = 2;
                test_status = 2;
                break;
            } else {
                Erase_Validity_Status = 3;
                test_status = 3;
            }
        }
    }

    return Erase_Validity_Status;
}

