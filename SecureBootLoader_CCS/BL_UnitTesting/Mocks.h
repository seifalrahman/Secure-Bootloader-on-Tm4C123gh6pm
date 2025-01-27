#ifndef MOCK_HAL_UART_H
#define MOCK_HAL_UART_H

#include "stdint.h"
#include "stdbool.h"  // For bool type (if your compiler doesn't support it)
#include "bootloader.h"
UART_HandleTypeDef huart2;
CRC_HandleTypeDef  hcrc  ;

 uint8_t testData[BL_HOST_BUFFER_RX_LENGTH];
 uint8_t mock_BL_Buffer[BL_HOST_BUFFER_RX_LENGTH];
 static uint8_t testDataPtr = 0;
// Global variables for mock simulation
 static uint8_t mock_uart_transmit_buffer[BL_HOST_BUFFER_RX_LENGTH]; // Buffer to hold transmitted data
 static bool simulate_uart_receive_error = false; // Flag to simulate errors
 static bool simulate_uart_transmit_error = false; // Flag to simulate errors
// Mock structure to simulate the CRC Engine object
typedef struct {
    uint32_t Mocked_CRC_Result;  // Predefined CRC result to simulate HAL behavior
    bool Force_Unmatch;            // Simulate an error in CRC calculation
} Mock_CRC_Engine;

typedef struct {
    bool Force_inValid;
} Mock_Adress_Verification;
typedef struct{
    bool HashTag_Force_unmatch;
}mockHashVerification_Process;


// Mock function declarations
uint32_t mock_HAL_CRC_Accumulate(CRC_HandleTypeDef *hcrc, uint8_t *pData, uint32_t Data_Len);
HAL_StatusTypeDef mock_HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t* get_mock_uart_transmit_buffer(void);
uint16_t get_mock_uart_transmit_size(void);
void simulate_uart_transmit_error_condition(void);
void clear_uart_transmit_error_condition(void);

// Prototypes for HAL UART Receive Mock
HAL_StatusTypeDef mock_HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
void set_mock_uart_receive_data(uint8_t *data, uint16_t size);
uint8_t* get_mock_uart_receive_buffer(void);
uint16_t get_mock_uart_receive_size(void);
void simulate_uart_receive_error_condition(void);
void clear_uart_receive_error_condition(void);
// Prototypes for Bootloader Mock Functions
void mock_Bootloader_Get_Version();
void mock_Bootloader_Get_Help();
void mock_Bootloader_Get_Chip_Identification_Number();
void mock_Bootloader_Jump_To_Address();
void mock_Bootloader_Erase_Flash();
void mock_Bootloader_Memory_Write();
uint8_t mock_Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
uint8_t mock_Host_Address_Verification (uint32_t Jump_Address);
int mock_FlashErase(uint32_t address);
#endif // MOCK_HAL_UART_H
