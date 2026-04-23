/**************************************************************************//**
 * @file     main.c
 * @brief    NuMaker Display Driver Sample Code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "NuMicro.h"
#include "display_testcases.h"
#include "perf_ev.h"

static void sys_init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*------------------------------------------------------------------------*/
    /* Init System Clock                                                      */
    /*------------------------------------------------------------------------*/
//    /* Enable HXT clock */
//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

//    /* Wait for HXT clock ready */
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

//    /* Set core clock */
//    CLK_SetCoreClock(__HSI);

//    /* Update System Core Clock */
//    SystemCoreClockUpdate();

    /* Set PCLK-related clock */
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);

    /* Enable all GPIO clock */
    CLK->AHBCLK0 |= CLK_AHBCLK0_GPACKEN_Msk | CLK_AHBCLK0_GPBCKEN_Msk | CLK_AHBCLK0_GPCCKEN_Msk | CLK_AHBCLK0_GPDCKEN_Msk |
                    CLK_AHBCLK0_GPECKEN_Msk | CLK_AHBCLK0_GPFCKEN_Msk | CLK_AHBCLK0_GPGCKEN_Msk | CLK_AHBCLK0_GPHCKEN_Msk;

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);
	
    /* EADC Analog Pin */
    CLK_EnableModuleClock(EADC0_MODULE);
    CLK_SetModuleClock(EADC0_MODULE, CLK_CLKSEL0_EADC0SEL_PLL, CLK_CLKDIV0_EADC0(12));

    /* USCI0, NU5 */
    CLK_EnableModuleClock(USCI0_MODULE);
    SET_USCI0_CTL1_PA8();
    SET_USCI0_DAT1_PA9();
    SET_USCI0_DAT0_PA10();
    SET_USCI0_CLK_PA11();

    /* EADC Analog Pin: UNO_A0, UNO_A1, UNO_A2, UNO_A3 */
    SET_EADC0_CH4_PB4();
    SET_EADC0_CH5_PB5();
    SET_EADC0_CH6_PB6();
    SET_EADC0_CH7_PB7();

    /* Disable digital path on these EADC pins */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT4 | BIT5 | BIT6 | BIT7);

    /* Vref connect to internal */
    SYS_SetVRef(SYS_VREFCTL_VREF_PIN);

    /* Enable PDMA0 module clock */
    CLK_EnableModuleClock(PDMA0_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Enable SysTick module clock */
    CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HCLK, 0);
    InitSysTick();

    /* Enable UART module clock */
    SetDebugUartCLK();
    SetDebugUartMFP();
    InitDebugUart();
}

int main(void)
{
    S_LCD_INFO sLcdInfo = {0};

    sys_init();

    /* Initial display device */
    lcd_device_initialize();

    /* Qurey display information. */
    lcd_device_control(evLCD_CTRL_GET_INFO, (void *)&sLcdInfo);

    printf("Display board name: %s, WxHxBPP: %dx%dx%dB\n", CONFIG_DISPLAY_BOARD_NAME, sLcdInfo.u32ResWidth, sLcdInfo.u32ResHeight, sLcdInfo.u32BytePerPixel);
    printf("Screen-size shadow buffer: 0x%08x, size: %d Bytes\n", (uint32_t)sLcdInfo.pvVramStartAddr, sLcdInfo.u32VramSize);

    lcd_device_open();

    touchpad_device_initialize();
    touchpad_device_open();

    while (1)
    {
        demo_lcd_flush(&sLcdInfo);
        //demo_touchpad_getpoint();
        //demo_lcd_readback_random(&sLcdInfo); //FIXME: USPI write/read issue.
    }

    lcd_device_close();

    return 0;
}
