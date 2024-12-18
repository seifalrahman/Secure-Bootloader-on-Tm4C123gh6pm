#include "bootloader.h"
#include "C:\MYCOMPUTER\TM4C_SDK\driverlib\sysctl.h"
int main (void){

	 // Enable the CRC module.
	 //
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_CCM0);
	 //
	 // Wait for the CRC module to be ready.
	 //
	 while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CCM0))
	 {
	 }
	 CRC_init();
	 AESReset(AES_BASE);
	 // Enable the UART0 module.
	 //
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	 //
	 // Wait for the UART0 module to be ready.
	 while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
	 {
	 }
	 
	 UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
	 (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
		UART_CONFIG_PAR_NONE));
	 
		while(1){
			
			BL_UART_Fetch_Host_Command();
		
		}

}