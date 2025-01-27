#include "Mocks.h" // Include the header file for the mock

extern uint8_t testData[BL_HOST_BUFFER_RX_LENGTH];

  Mock_CRC_Engine mock_crc_engine = {0, false};
  Mock_Adress_Verification mock_adress_verification = {false};
  bool simulateFlashERROR = false;
// Mock implementation of HAL_CRC_Accumulate
uint32_t mock_HAL_CRC_Accumulate(CRC_HandleTypeDef *hcrc, uint8_t *pData, uint32_t Data_Len) {
    // Check if an error scenario is forced
    if (mock_crc_engine.Force_Unmatch) {
        return 0x00000000;  // Return an incorrect CRC to simulate failure (Very difficult to be optained normally)
    }

    // Otherwise, return the mocked CRC result
    return mock_crc_engine.Mocked_CRC_Result;
}

// Mock for HAL_UART_Transmit
HAL_StatusTypeDef mock_HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout) {

    if(simulate_uart_transmit_error)
    {
        return HAL_ERROR; // Simulate unsuccessful reception
    }
    else
    {
        return HAL_OK;  // Simulate successful reception
    }
}




HAL_StatusTypeDef mock_HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    // Mock UART data reception
    //we assume the data received by UART is already stored in testData Buffer
    for(int i = 0 ; i < Size ; i++)
    {
        pData[i] = testData[testDataPtr++];
    }
    //What if UART failed ?!
    if(simulate_uart_receive_error)
    {
        return HAL_ERROR; // Simulate unsuccessful reception
    }
    else
    {
        return HAL_OK;  // Simulate successful reception
    }
}

void simulate_uart_receive_error_condition(void) {
    simulate_uart_receive_error = true;
}

void clear_uart_receive_error_condition(void) {
    simulate_uart_receive_error = false;
}

// Mocked Bootloader functions
void mock_Bootloader_Get_Version() {
    // Simulate Bootloader_Get_Version behavior
    // For example, set the version info in the buffer
    // Just as an example of a successful command
}

void mock_Bootloader_Get_Help() {
    // Simulate Bootloader_Get_Help behavior
    // For example, set help message or mock result
    // Example of setting a mock result
}

void mock_Bootloader_Get_Chip_Identification_Number() {
    // Simulate Bootloader_Get_Chip_Identification_Number behavior
    // Example mock value for CID
}

void mock_Bootloader_Jump_To_Address() {
    // Simulate Bootloader_Jump_To_Address behavior
    // Mock the jump to address command
}

void mock_Bootloader_Erase_Flash() {
    // Simulate Bootloader_Erase_Flash behavior
    // Example of mock flash erase operation
}

void mock_Bootloader_Memory_Write() {
    // Simulate Bootloader_Memory_Write behavior
    // Example of mock memory write command
}
uint8_t mock_Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC){
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	if(mock_crc_engine.Force_Unmatch){ //decide which senerio you want to go through
		CRC_Status = CRC_VERIFICATION_FAILED;
	}
	else{
		CRC_Status = CRC_VERIFICATION_PASSED;
	}

	return CRC_Status;
}
uint8_t mock_Host_Address_Verification (uint32_t Jump_Address){
    if(mock_adress_verification.Force_inValid) // Determine which senerio path
    return ADDRESS_IS_INVALID;
    else
	return ADDRESS_IS_VALID;
}
// Mock of FlashErase function for testing
int mock_FlashErase(uint32_t address) {
    // This mock simulates a flash erase operation.
    if (simulateFlashERROR) {
        return -1; // Simulate error if the address is invalid
    }
    return 0; // Simulate successful erase
}
