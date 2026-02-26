/**************************************************************************//**
 * @file     disp_glue.c
 * @brief    NuMaker Display Driver glue code for EBI 8080 interface LCD panel
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdint.h>
#include "disp.h"

static uint8_t s_au8FrameBuf[CONFIG_VRAM_TOTAL_ALLOCATED_SIZE] __attribute__((aligned(4)));

void sysDelay(uint32_t ms)
{
#if defined(LV_USE_OS) && (LV_USE_OS==LV_OS_FREERTOS)
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    TIMER_Delay(TIMER0, ms * 1000);
#endif
}

static uint32_t ns_to_cycles(uint32_t ns, uint32_t clk_hz)
{
    if (ns == 0 || clk_hz == 0)
        return 0;

    uint64_t numerator = (uint64_t)ns * clk_hz;

    return (uint32_t)((numerator + 1000000000ULL - 1) / 1000000000ULL);
}

static uint32_t hz_to_ns(uint32_t hz)
{
    if (hz == 0)
        return 0;

    /* ceil(1e9 / hz) */
    return (uint32_t)((1000000000ULL + hz - 1) / hz);
}

static void EBI_OptimizeTiming(void)
{
#define TAHD_MAX(a, b)   ((a) > (b) ? (a) : (b))
    for (int i32MCLKDiv = EBI_MCLKDIV_1; i32MCLKDiv <= EBI_MCLKDIV_128; i32MCLKDiv++)
    {
        uint32_t u32EBI_MCLK_hz = CLK_GetHCLKFreq() / (i32MCLKDiv + 1);

        /* Convert to cycles */
        uint32_t TACC = ns_to_cycles(EBI_8080_ACCESS_NS,  u32EBI_MCLK_hz) - 1;
        uint32_t W2X  = ns_to_cycles(EBI_8080_WR_IDLE_NS, u32EBI_MCLK_hz);
        uint32_t TAHD = ns_to_cycles(TAHD_MAX(EBI_8080_WR_AHD_NS, EBI_8080_RD_AHD_NS),  u32EBI_MCLK_hz);
        uint32_t R2R  = ns_to_cycles(EBI_8080_RD_IDLE_NS, u32EBI_MCLK_hz);

        printf("EBI_MCLK_hz: %d\n", u32EBI_MCLK_hz);
        printf("EBI_MCLK_ns: %d\n", hz_to_ns(u32EBI_MCLK_hz));
        printf("EBI_8080_ACCESS_NS: %d ns\n", EBI_8080_ACCESS_NS);
        printf("EBI_8080_WR_IDLE_NS: %d ns\n", EBI_8080_WR_IDLE_NS);
        printf("EBI_8080_AHD_NS: %d ns\n", EBI_8080_WR_AHD_NS);
        printf("EBI_8080_AHD_NS: %d ns\n", EBI_8080_RD_AHD_NS);
        printf("EBI_8080_RD_IDLE_NS: %d ns\n", EBI_8080_RD_IDLE_NS);
        printf("Calculated: TACC:%d, W2X:%d, TAHD:%d, R2R:%d\n", TACC, W2X, TAHD, R2R);

        /* Hardware register limitation */
        if ((TACC > 31) || (W2X  > 15) || (TAHD > 7) || (R2R  > 15))
        {
            continue;
        }

        uint32_t WAHDOFF = (EBI_8080_WR_AHD_NS == 0) ? 1 : 0;
        uint32_t RAHDOFF = (EBI_8080_RD_AHD_NS == 0) ? 1 : 0;

        EBI_SetBusTiming(CONFIG_DISP_EBI,
                         (RAHDOFF << EBI_TCTL_RAHDOFF_Pos) |
                         (WAHDOFF << EBI_TCTL_WAHDOFF_Pos) |
                         (W2X      << EBI_TCTL_W2X_Pos)    |
                         (R2R      << EBI_TCTL_R2R_Pos)    |
                         (TAHD     << EBI_TCTL_TAHD_Pos)   |
                         (TACC     << EBI_TCTL_TACC_Pos),
                         i32MCLKDiv);

        printf("Applied: TACC:%d, W2X:%d, TAHD:%d, R2R:%d\n", TACC, W2X, TAHD, R2R);
        printf("Verify EBI->CTL0: 0x%08X\n", EBI->CTL0);
        printf("Verify EBI->TCTL0: 0x%08X\n", EBI->TCTL0);

        return;
    }

    printf("Failed to found avaialbe 8080 timing parameter for this panel.\n");
}

int lcd_device_initialize(void)
{
    GPIO_T *PORT;

    /* Set GPIO Output mode for display pins. */
    PORT    = (GPIO_T *)(GPIOA_BASE + (NU_GET_PORT(CONFIG_DISP_PIN_RESET) * PORT_OFFSET));
    GPIO_SetMode(PORT, NU_GET_PIN_MASK(NU_GET_PIN(CONFIG_DISP_PIN_RESET)), GPIO_MODE_OUTPUT);

    PORT    = (GPIO_T *)(GPIOA_BASE + (NU_GET_PORT(CONFIG_DISP_PIN_BACKLIGHT) * PORT_OFFSET));
    GPIO_SetMode(PORT, NU_GET_PIN_MASK(NU_GET_PIN(CONFIG_DISP_PIN_BACKLIGHT)), GPIO_MODE_OUTPUT);


    /* Open EBI  */
    EBI_Open(CONFIG_DISP_EBI, EBI_BUSWIDTH_16BIT, EBI_TIMING_FAST, EBI_OPMODE_ADSEPARATE, EBI_CS_ACTIVE_LOW);
    EBI_ENABLE_WRITE_BUFFER();
    EBI_OptimizeTiming();

    return disp_init();
}

int lcd_device_open(void)
{
    return 0;
}

int lcd_device_control(int cmd, void *argv)
{
    switch (cmd)
    {
    case evLCD_CTRL_GET_INFO:
    {
        S_LCD_INFO *psLCDInfo = (S_LCD_INFO *)argv;

        psLCDInfo->pvVramStartAddr = (void *)s_au8FrameBuf;
        psLCDInfo->u32VramSize = CONFIG_VRAM_TOTAL_ALLOCATED_SIZE;
        psLCDInfo->u32ResWidth = DISP_HOR_RES_MAX;
        psLCDInfo->u32ResHeight = DISP_VER_RES_MAX;
        psLCDInfo->u32BytePerPixel = (DISP_COLOR_DEPTH / 8);
        psLCDInfo->evLCDType = evLCD_TYPE_MPU;
    }
    break;

    case evLCD_CTRL_RECT_UPDATE:
    {
        disp_fillrect((uint16_t *)s_au8FrameBuf, (const disp_area_t *)argv);
    }
    break;

    default:
        while (1);
    }

    return 0;
}

void lcd_device_close(void)
{
}

int lcd_device_finalize(void)
{
    return 0;
}
