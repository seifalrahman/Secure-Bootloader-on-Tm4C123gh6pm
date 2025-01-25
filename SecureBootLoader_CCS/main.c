#include "bootloader.h"
#include "uart0.h"
#include "tinycrypt/sha256.h"
#include "tinycrypt/utils.h"
int main(void)
{
    UART0_Init();

//    uint8_t hash[TC_SHA256_DIGEST_SIZE];
//    const uint8_t data[] = "Hello, TinyCrypt!";
//
//    struct tc_sha256_state_struct s;
//
//    tc_sha256_init(&s);
//    tc_sha256_update(&s, data, sizeof(data) - 1);
//   // tc_sha256_update(&s, data+3,sizeof(data) - 4);
//    tc_sha256_final(hash, &s);
    while(1){

        BL_UART_Fetch_Host_Command();
    }
	return 0;
}
