/**************************************************************************//**
 * @file     main.c
 * @brief    NuMaker Display Driver Sample Code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "NuMicro.h"
#include "disp.h"

extern uint64_t GetSysTickCycleCount(void);
extern uint64_t InitSysTick(void);

static void sys_init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable PLL0 clock from HXT and switch SCLK clock source to PLL0 */
    CLK_SetBusClock(CLK_SCLKSEL_SCLKSEL_APLL0, CLK_APLLCTL_APLLSRC_HXT, FREQ_220MHZ);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Enable UART0 module clock */
    SetDebugUartCLK();

    CLK_EnableModuleClock(GPIOA_MODULE);
    CLK_EnableModuleClock(GPIOB_MODULE);
    CLK_EnableModuleClock(GPIOC_MODULE);
    CLK_EnableModuleClock(GPIOD_MODULE);
    CLK_EnableModuleClock(GPIOE_MODULE);
    CLK_EnableModuleClock(GPIOF_MODULE);
    CLK_EnableModuleClock(GPIOG_MODULE);
    CLK_EnableModuleClock(GPIOH_MODULE);
    CLK_EnableModuleClock(GPIOI_MODULE);
    CLK_EnableModuleClock(GPIOJ_MODULE);

    /* Enable EBI clock */
    CLK_EnableModuleClock(EBI0_MODULE);

    /* Enable I2C1 clock */
    CLK_EnableModuleClock(I2C1_MODULE);

    /* Enable SPI2 clock */
    CLK_EnableModuleClock(SPI2_MODULE);

    /* Enable PDMA clock */
    CLK_EnableModuleClock(PDMA0_MODULE);
    CLK_EnableModuleClock(PDMA1_MODULE);

    /* Select TIMER clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_TMRSEL_TMR0SEL_HIRC, 0);

    /* Enable TIMER module clock */
    CLK_EnableModuleClock(TMR0_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    SET_SPI2_SS_PA11();
    SET_SPI2_CLK_PA10();
    SET_SPI2_MISO_PA9();
    SET_SPI2_MOSI_PA8();

    SET_I2C1_SDA_PB10();
    SET_I2C1_SCL_PB11();

    SET_EBI_AD0_PA5();
    SET_EBI_AD1_PA4();
    SET_EBI_AD2_PC2();
    SET_EBI_AD3_PC3();
    SET_EBI_AD4_PC4();
    SET_EBI_AD5_PC5();
    SET_EBI_AD6_PD8();
    SET_EBI_AD7_PD9();
    SET_EBI_AD8_PE14();
    SET_EBI_AD9_PE15();
    SET_EBI_AD10_PE1();
    SET_EBI_AD11_PE0();
    SET_EBI_AD12_PH8();
    SET_EBI_AD13_PH9();
    SET_EBI_AD14_PH10();
    SET_EBI_AD15_PH11();
    SET_EBI_nWR_PJ9();
    SET_EBI_nRD_PJ8();
    SET_EBI_nCS0_PD14();
    SET_EBI_ADR0_PH7();
    GPIO_SetSlewCtl(PH, (BIT7 | BIT8 | BIT9 | BIT10 | BIT11), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PA, (BIT4 | BIT5), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PC, (BIT2 | BIT3 | BIT4 | BIT5), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PD, (BIT8 | BIT9), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PE, (BIT14 | BIT15), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PE, (BIT0 | BIT1), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PJ, (BIT8 | BIT9), GPIO_SLEWCTL_HIGH);
    GPIO_SetSlewCtl(PD, BIT14, GPIO_SLEWCTL_HIGH);

    SetDebugUartMFP();

    InitDebugUart();

    /* Initial systick for performence evaluation. */
    InitSysTick();
}

static uint32_t disp_area_pixel_count(const disp_area_t *area)
{
    if (!area)
        return 0;

    /* Invalid area */
    if (area->x2 < area->x1 || area->y2 < area->y1)
        return 0;

    uint32_t width  = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t height = (uint32_t)(area->y2 - area->y1 + 1);

    return width * height;
}

static void demo_lcd_flush(const S_LCD_INFO *psLcdInfo)
{
    uint16_t color[3] = {0xF800, 0x07E0, 0x001F};
    uint64_t start, elapsed;

    disp_area_t sFullRefresh = {0, 0,
                                DISP_HOR_RES_MAX - 1,
                                DISP_VER_RES_MAX - 1
                               };

    disp_area_t sPartialUpdate = { DISP_HOR_RES_MAX / 4,
                                   DISP_VER_RES_MAX / 4,
                                   (DISP_HOR_RES_MAX / 2 + DISP_HOR_RES_MAX / 4) - 1,
                                   (DISP_VER_RES_MAX / 2 + DISP_VER_RES_MAX / 4) - 1
                                 };

    uint16_t *pu16Color = (uint16_t *)psLcdInfo->pvVramStartAddr;
    for (int c = 0; c < sizeof(color) / sizeof(uint16_t); c++)
    {
        uint32_t u32AreaPixelCount;

        /* Render to shadow buffer */
        u32AreaPixelCount = disp_area_pixel_count(&sFullRefresh);
        for (int i = 0; i < u32AreaPixelCount; i++)
        {
            pu16Color[i] = color[c];
        }

        /* Flush to VRAM on LCD panel */
        start = GetSysTickCycleCount();
        lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sFullRefresh);
        elapsed = GetSysTickCycleCount() - start;
        printf("[%s] FullRefresh flush %d pixels, elpased %.2fms.\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);

        /* Render to shadow buffer */
        u32AreaPixelCount = disp_area_pixel_count(&sPartialUpdate);
        for (int i = 0; i < u32AreaPixelCount; i++)
        {
            pu16Color[i] = color[(c + 1) % 3];
        }

        /* Flush to VRAM on LCD panel */
        start = GetSysTickCycleCount();
        lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sPartialUpdate);
        elapsed = GetSysTickCycleCount() - start;
        printf("[%s] PartialUpdate flush %d pixels, elpased %.2fms.\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);

        /* Optional: delay to see color */
        TIMER_Delay(TIMER0, 1000000);
    }
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

    while (1)
    {
        demo_lcd_flush(&sLcdInfo);
    }

    lcd_device_close();

    return 0;
}
