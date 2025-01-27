#include "unity.h"
#include "unity_internals.h"
#include "Mocks.h"  // Include the mocked UART functions
#include "UnderTestingComponents.h"
// Global test buffer to store the message for validation
char test_message_buffer[100];
extern Mock_CRC_Engine mock_crc_engine;
extern Mock_Adress_Verification mock_adress_verification;
extern mockHashVerification_Process HashMock;
extern bool simulateFlashERROR;
uint8_t test_status = -1;
typedef enum {CRC_AddressRange_HashTag_VALID = 3,CRC_INVALID=6,AdressRange_INVALID = 5,HASHTag_INVALID = 4}JumpToAdressTest_Status;
void setUp(void) {
    memset(test_message_buffer,'\0',100); //Set the buffer with nulls
    testDataPtr = 0;
    test_status = -1;
    mock_adress_verification.Force_inValid = false;
    mock_crc_engine.Force_Unmatch = false;
    HashMock.HashTag_Force_unmatch = false;
}

void tearDown(void) {
    clear_uart_receive_error_condition();
    memset(testData,'\0',100);              //Set the buffer with nulls
}

// Function to store the message for testing
void store_message_for_testing(char *message) {
    // Store the message in the global test buffer
    strncpy(test_message_buffer, message, sizeof(test_message_buffer) - 1);
    test_message_buffer[sizeof(test_message_buffer) - 1] = '\0';  // Ensure null termination
}

// Test: Transmit a formatted message via UART
// We are testing the BL_Print_Message function to ensure that it
// correctly transmits a formatted message to the UART.
void test_BL_Print_Message_TC001(void) {
    char testMessage[] = "Bootloader version: 1.0.0";

    // Simulate UART transmission
    BL_Print_Message_UnderTest(testMessage);

    // Validate the message is transmitted correctly
    TEST_ASSERT_EQUAL_STRING(testMessage, test_message_buffer);
}
// Test: Handle empty message correctly
// We are testing the BL_Print_Message function to verify that it
// correctly handles an empty string and transmits it without errors.
void test_BL_Print_Message_TC002(void) {
    char testMessage[] = "";

    // Simulate UART transmission of an empty string
    BL_Print_Message_UnderTest(testMessage);

    // Validate the empty string is transmitted
    TEST_ASSERT_EQUAL_STRING(testMessage, test_message_buffer);
}

// Test: Handle valid version command (CBL_GET_VER_CMD)
// We are testing the BL_UART_Fetch_Host_Command function with a valid
// version command. The function should process the command, return BL_OK,
// and invoke the correct handler (Bootloader_Get_Version).
void test_BL_UART_Fetch_Host_Command_TC003(void) {
    // Simulate receiving the valid command CBL_GET_VER_CMD
    testData[0] = 5;
    testData[1] = CBL_GET_VER_CMD;
    // Call the function to process the command
    BL_Status status = BL_UART_Fetch_Host_Command_UnderTest();

    // Validate the status and the command processing
    TEST_ASSERT_EQUAL(BL_OK, status);
    TEST_ASSERT_EQUAL(CBL_GET_VER_CMD, mock_BL_Buffer[1]);
}

// Test: Handle invalid command gracefully
// We are testing the BL_UART_Fetch_Host_Command function with an invalid
// command (0xFF). The function should return BL_NACK and should not invoke
// any command handler.
void test_BL_UART_Fetch_Host_Command_TC004(void) {
    // Simulate receiving an invalid command (0xFF)
    testData[0] = 5;
    testData[1] = 0xFF;  // Set an invalid command

    // Call the function to process the command
    BL_Status status = BL_UART_Fetch_Host_Command_UnderTest();

    // Validate the status and ensure no handler is invoked
    TEST_ASSERT_EQUAL(BL_NACK, status);
}

// Test: Handle UART receive error
// We are testing the BL_UART_Fetch_Host_Command function to ensure that
// if a UART receive error occurs, the function returns BL_NACK and
// UART remains in an error-free state for subsequent communication.
void test_BL_UART_Fetch_Host_Command_TC005(void) {

    // Force the error scenario by overriding the behavior of HAL_UART_Receive
    simulate_uart_receive_error_condition();  // which means UART_Receive will return HAL_ERROR

    // Call the function to handle UART communication
    BL_Status status = BL_UART_Fetch_Host_Command_UnderTest();

    // Validate that the status is NACK due to the receive error
    TEST_ASSERT_EQUAL(BL_NACK, status);
}

// Test Case 6: Verify CRC matches when Host and MCU CRC are equal
void test_Bootloader_CRC_Verify_TC006(void) {
    uint32_t result;
    // Set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    // Assume CRC will return the same value of the host
    mock_crc_engine.Force_Unmatch = false;
    mock_crc_engine.Mocked_CRC_Result = 0xB63CFBCD;
    uint32_t mockHostCRC = 0xB63CFBCD;

    // Call function under test
    result = Bootloader_CRC_Verify_UnderTesting(testData, 8, mockHostCRC);

    // Validate result
    TEST_ASSERT_EQUAL(CRC_VERIFICATION_PASSED, result);
}

// Test Case 2: Verify CRC fails when Host and MCU CRC are not equal
void test_Bootloader_CRC_Verify_TC007(void) {
    uint8_t result;

    // Set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    //Force unmatched calculated CRC
    uint32_t mockHostCRC = 0x87654321;              // invalid crc from host
    mock_crc_engine.Mocked_CRC_Result = 0xB63CFBCD; //the right CRC for this data
    mock_crc_engine.Force_Unmatch = false;

    // Call function under test
    result = Bootloader_CRC_Verify_UnderTesting(testData, 8, mockHostCRC);

    // Validate result
    TEST_ASSERT_EQUAL(CRC_VERIFICATION_FAILED, result);
}

// Test Case 3: Verify CRC with zero-length data
void test_Bootloader_CRC_Verify_TC008(void) {
    uint8_t result;

    // Assume CRC will return the same value of the host
    mock_crc_engine.Force_Unmatch = false;
    mock_crc_engine.Mocked_CRC_Result = 0x00000000;
    uint32_t mockHostCRC = 0x00000000;

    // Call function under test
    result = Bootloader_CRC_Verify_UnderTesting(testData, 0, mockHostCRC);


    // Validate result
    TEST_ASSERT_EQUAL(CRC_VERIFICATION_PASSED, result);
}


// Test Case 5: Verify CRC fails when MCU CRC calculation fails
void test_Bootloader_CRC_Verify_TC009(void) {
    uint8_t result;

    // Set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    //Force Failing of CRC HW
    uint32_t mockHostCRC = 0xB63CFBCD;      //true crc from host side
    mock_crc_engine.Force_Unmatch = true;   //CRC fails to calculate it correctly

    // Call function under test
    result = Bootloader_CRC_Verify_UnderTesting(testData, 4, mockHostCRC);

    // Validate result
    TEST_ASSERT_EQUAL(CRC_VERIFICATION_FAILED, result);
}
// Test Case 10: Corrupted CRC in host packet
void test_Bootloader_Get_Version_TC010(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    testData[4]=0x01;
    testData[5]=0x01;
    testData[6]=0x01;
    testData[7]=0x01;
    mock_crc_engine.Force_Unmatch = true;
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(0, test_status);
    mock_crc_engine.Force_Unmatch = false;
}
// Test Case 11: Partial or interrupted host packet
void test_Bootloader_Get_Version_TC011(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    mock_crc_engine.Force_Unmatch = true;
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(0, test_status);
    mock_crc_engine.Force_Unmatch = false;
}
// Test Case 12: Valid CRC and packet length for Bootloader_Get_Help
void test_Bootloader_Get_Version_TC012(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(1, test_status);
}
// Test Case 13: Invalid CRC for Bootloader_Get_Help
void test_Bootloader_Get_Version_TC013(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    mock_crc_engine.Force_Unmatch = true;
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(0, test_status);
    mock_crc_engine.Force_Unmatch = false;
}
// Test Case 14: Valid data transmission for Bootloader_Get_Help
void test_Bootloader_Get_Version_TC014(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    mock_crc_engine.Force_Unmatch = false;
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(1, test_status);
}
// Test Case 15: Valid CRC and correct chip identification for Bootloader_Get_Chip_Identification_Number
void test_Bootloader_Get_Chip_Identification_Number_TC015(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    mock_crc_engine.Force_Unmatch = false;
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(1, test_status);
}
// Test Case 16: Valid CRC and correct chip identification for Bootloader_Get_Chip_Identification_Number
void test_Bootloader_Get_Chip_Identification_Number_TC016(void) {
    //set test data
    testData[0]=0x08;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    mock_crc_engine.Force_Unmatch = true; //INVALID CRC SENT
    Bootloader_Get_Version_UnderTesting(testData);
    // Validate result
    TEST_ASSERT_EQUAL(0, test_status);
    mock_crc_engine.Force_Unmatch = false;
}
//TC017,018,019
void Test_Host_Address_Verification_Valid_Range() {
    uint8_t result;
    int32_t testadress;
    // TC017: Verify valid address within range
    testadress = 0x1000;
    result = Host_Address_Verification_UnderTesting(testadress);
    TEST_ASSERT_EQUAL_UINT8(ADDRESS_IS_VALID, result);

    // TC018: Verify address at the lower boundary of the valid range
    testadress = 0x0000;
    result = Host_Address_Verification_UnderTesting(testadress);
    TEST_ASSERT_EQUAL_UINT8(ADDRESS_IS_VALID, result);

    // TC019: Verify address at the upper boundary of the valid range
    testadress = TM4C123GH6PM_FLASH_END;
    result = Host_Address_Verification_UnderTesting(testadress);
    TEST_ASSERT_EQUAL_UINT8(ADDRESS_IS_VALID, result);
}
//TC020,021,022
void Test_Host_Address_Verification_Invalid_Range() {
    uint8_t result;
    int32_t testadress=0;
    // TC020: Verify address below the valid range
    testadress = -1;  // Using int32_t to simulate signed value
    result = Host_Address_Verification_UnderTesting(testadress);
    TEST_ASSERT_EQUAL_UINT8(ADDRESS_IS_INVALID, result);

    // TC021: Verify address above the valid range
    testadress = TM4C123GH6PM_FLASH_END + 1;
    result = Host_Address_Verification_UnderTesting(testadress);
    TEST_ASSERT_EQUAL_UINT8(ADDRESS_IS_INVALID, result);

    // TC022: Verify function with large value beyond flash size
    testadress = 0xFFFFFFF;
    result = Host_Address_Verification_UnderTesting(testadress);
    TEST_ASSERT_EQUAL_UINT8(ADDRESS_IS_INVALID, result);
}
//TC023 : Valid CRC and valid jump address, hash verified
void Test_Bootloader_Jump_To_Address_TC023() {
    //set test data
    testData[0]=0x09;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    testData[5]=0x05;
    mock_crc_engine.Force_Unmatch=false;
    mock_adress_verification.Force_inValid = false;
    HashMock.HashTag_Force_unmatch = false;
    Bootloader_Jump_To_Address_UnderTesting(testData);
    TEST_ASSERT_EQUAL(CRC_AddressRange_HashTag_VALID, test_status);
}
//TC024 : Valid CRC but invalid jump address
void Test_Bootloader_Jump_To_Address_TC024() {
    //set test data
    testData[0]=0x09;
    testData[1]=0x01;
    testData[2]=0xFF;
    testData[3]=0xFF;
    testData[4]=0xFF;
    testData[5]=0xFF;
    mock_crc_engine.Force_Unmatch=false;
    mock_adress_verification.Force_inValid = true;
    //HashMock.HashTag_Force_unmatch = false;
    Bootloader_Jump_To_Address_UnderTesting(testData);
    TEST_ASSERT_EQUAL(AdressRange_INVALID, test_status);
}
//TC025 : Valid CRC, valid jump address, but hash mismatch
void Test_Bootloader_Jump_To_Address_TC025() {
    //set test data
    testData[0]=0x09;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    testData[5]=0x05;
    mock_crc_engine.Force_Unmatch=false;
    mock_adress_verification.Force_inValid = false;
    HashMock.HashTag_Force_unmatch = true;
    Bootloader_Jump_To_Address_UnderTesting(testData);
    TEST_ASSERT_EQUAL(HASHTag_INVALID, test_status);
}

//TC026 : Invalid CRC
void Test_Bootloader_Jump_To_Address_TC026() {
    //set test data
    testData[0]=0x09;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    testData[5]=0x05;
    mock_crc_engine.Force_Unmatch = true;
    //mock_adress_verification.Force_inValid = false;
    //HashMock.HashTag_Force_unmatch = true;
    Bootloader_Jump_To_Address_UnderTesting(testData);
    TEST_ASSERT_EQUAL(CRC_INVALID, test_status);
}
//TC027 : Valid CRC but address verification mock fails
void Test_Bootloader_Jump_To_Address_TC027() {
    //set test data
    testData[0]=0x09;
    testData[1]=0x01;
    testData[2]=0x02;
    testData[3]=0x03;
    testData[4]=0x04;
    testData[5]=0x05;
    mock_crc_engine.Force_Unmatch=false;
    mock_adress_verification.Force_inValid = false;
    HashMock.HashTag_Force_unmatch = true;
    Bootloader_Jump_To_Address_UnderTesting(testData);
    TEST_ASSERT_EQUAL(HASHTag_INVALID, test_status);
}

//TC028 : Valid flash erase operation
void Test_Perform_Flash_Erase_TC028() {
    uint8_t result;
    //set test data
    uint8_t testSectorNumber = 5;
    uint8_t testNumberOfSectors = 3;
    simulateFlashERROR = false;
    result = Perform_Flash_Erase_UnderTesting(testSectorNumber,testNumberOfSectors);
    TEST_ASSERT_EQUAL(3, result); //3 for flash driver success
    TEST_ASSERT_EQUAL(1,test_status);//To check which path it took.
}
//TC029 : Valid flash erase operation but Flash driver failed
void Test_Perform_Flash_Erase_TC029() {
    uint8_t result;
    //set test data
    uint8_t testSectorNumber = 5;
    uint8_t testNumberOfSectors = 3;
    simulateFlashERROR = true;
    result = Perform_Flash_Erase_UnderTesting(testSectorNumber,testNumberOfSectors);
    TEST_ASSERT_EQUAL(2, result); //0 for flash driver failure
    TEST_ASSERT_EQUAL(0,test_status);//To check which path it took.
}
//TC030 : Erase the entire application sucessful
void Test_Perform_Flash_Erase_TC030() {
    uint8_t result;
    //set test data
    uint8_t testSectorNumber = 254;
    uint8_t testNumberOfSectors = 0;
    simulateFlashERROR = false;
    result = Perform_Flash_Erase_UnderTesting(testSectorNumber,testNumberOfSectors);
    TEST_ASSERT_EQUAL(3, result); //3 for flash driver success
    TEST_ASSERT_EQUAL(3,test_status);//To check which path it took.
}
//TC031 : erasing the entire application was valid but the flash driver failed
void Test_Perform_Flash_Erase_TC031() {
    uint8_t result;
    //set test data
    uint8_t testSectorNumber = 254;
    uint8_t testNumberOfSectors = 0;
    simulateFlashERROR = true;
    result = Perform_Flash_Erase_UnderTesting(testSectorNumber,testNumberOfSectors);
    TEST_ASSERT_EQUAL(2, result); //3 for flash driver failure
    TEST_ASSERT_EQUAL(2,test_status);//To check which path it took.
}
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_BL_Print_Message_TC001);
    RUN_TEST(test_BL_Print_Message_TC002);
    RUN_TEST(test_BL_UART_Fetch_Host_Command_TC003);
    RUN_TEST(test_BL_UART_Fetch_Host_Command_TC004);
    RUN_TEST(test_BL_UART_Fetch_Host_Command_TC005);
    RUN_TEST(test_Bootloader_CRC_Verify_TC006);
    RUN_TEST(test_Bootloader_CRC_Verify_TC007);
    RUN_TEST(test_Bootloader_CRC_Verify_TC008);
    RUN_TEST(test_Bootloader_CRC_Verify_TC009);
    RUN_TEST(test_Bootloader_Get_Version_TC010);
    RUN_TEST(test_Bootloader_Get_Version_TC011);
    RUN_TEST(test_Bootloader_Get_Version_TC012);
    RUN_TEST(test_Bootloader_Get_Version_TC013);
    RUN_TEST(test_Bootloader_Get_Version_TC014);
    RUN_TEST(test_Bootloader_Get_Chip_Identification_Number_TC015);
    RUN_TEST(test_Bootloader_Get_Chip_Identification_Number_TC016);
    RUN_TEST(Test_Host_Address_Verification_Valid_Range);
    RUN_TEST(Test_Host_Address_Verification_Invalid_Range);
    RUN_TEST(Test_Bootloader_Jump_To_Address_TC023);
    RUN_TEST(Test_Bootloader_Jump_To_Address_TC024);
    RUN_TEST(Test_Bootloader_Jump_To_Address_TC025);
    RUN_TEST(Test_Bootloader_Jump_To_Address_TC026);
    RUN_TEST(Test_Bootloader_Jump_To_Address_TC027);
    RUN_TEST(Test_Perform_Flash_Erase_TC028);
    RUN_TEST(Test_Perform_Flash_Erase_TC029);
    RUN_TEST(Test_Perform_Flash_Erase_TC030);
    RUN_TEST(Test_Perform_Flash_Erase_TC031);


    return UNITY_END();
}


