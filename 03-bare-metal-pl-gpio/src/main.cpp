#include <stdio.h>

extern "C" {
    #include "xil_printf.h"
    #include "xgpio.h"
    #include "xparameters.h"
    #include "sleep.h"
}

XGpio Gpio;

int main() {

    xil_printf("\r\n==========================================\r\n");
    xil_printf("  ZUBoard 1CG - FPGA PL Logic Gate Test   \r\n");
    xil_printf("==========================================\r\n");

    // 1. Initialise AXI GPIO
    int status = XGpio_Initialize(&Gpio, XPAR_AXI_GPIO_0_BASEADDR);
    if (status != XST_SUCCESS) {
        xil_printf("AXI GPIO Initialisation Failed!\r\n");
        return -1;
    }

    // 2. Set Channel 1 as input
    XGpio_SetDataDirection(&Gpio, 1, 0xFFFFFFFF);

    xil_printf("Monitoring Push-Button SW3...\r\n");

    // 3. Read FPGA Logic Gate result
    while (1) {
        u32 logic_output = XGpio_DiscreteRead(&Gpio, 1);

        if (logic_output & 0x01) {
            xil_printf("Button SW3 Pressed! Logic Output = 1 (LED ON)\r\n");
        } else {
            xil_printf("Button SW3 Released. Logic Output = 0 (LED OFF)\r\n");
        }

        usleep(500000);
    }

    return 0;
}
