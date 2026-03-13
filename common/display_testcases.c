/**************************************************************************//**
 * @file     display_testcases.c
 * @brief    NuMaker Display Driver testcases routines
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdint.h>
#include "NuMicro.h"
#include "numaker_disp.h"
#include "numaker_touch.h"
#include "sw_crc.h"
#include "perf_ev.h"

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

void demo_lcd_flush(const S_LCD_INFO *psLcdInfo)
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
        if (CONFIG_VRAM_TOTAL_ALLOCATED_SIZE >= (u32AreaPixelCount * psLcdInfo->u32BytePerPixel))
        {
            for (int i = 0; i < u32AreaPixelCount; i++)
            {
                pu16Color[i] = color[c];
            }

            /* Flush to VRAM on LCD panel */
            start = GetSysTickCycleCount();
            lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sFullRefresh);
            elapsed = GetSysTickCycleCount() - start;
            printf("\033[32m[%s] FullRefresh flush %d pixels, elpased %.2fms.\033[0m\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);
        }

        /* Render to shadow buffer */
        u32AreaPixelCount = disp_area_pixel_count(&sPartialUpdate);
        if (CONFIG_VRAM_TOTAL_ALLOCATED_SIZE >= (u32AreaPixelCount * psLcdInfo->u32BytePerPixel))
        {
            for (int i = 0; i < u32AreaPixelCount; i++)
            {
                pu16Color[i] = color[(c + 1) % 3];
            }

            /* Flush to VRAM on LCD panel */
            start = GetSysTickCycleCount();
            lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sPartialUpdate);
            elapsed = GetSysTickCycleCount() - start;
            printf("\033[32m[%s] PartialUpdate flush %d pixels, elpased %.2fms.\033[0m\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);

            /* Optional: delay to see color */
            TIMER_Delay(TIMER0, 500000);
        }
    }
}

void demo_lcd_readback(const S_LCD_INFO *psLcdInfo)
{
    int i;
    uint64_t start, elapsed;

    disp_area_t *area;

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
    uint32_t u32AreaPixelCount;
    uint32_t u32CheckSumW, u32CheckSumR;

    /* Render to shadow buffer */
    u32AreaPixelCount = disp_area_pixel_count(&sFullRefresh);
    if (CONFIG_VRAM_TOTAL_ALLOCATED_SIZE >= (u32AreaPixelCount * psLcdInfo->u32BytePerPixel))
    {
        area = &sFullRefresh;
    }
    else
    {
        u32AreaPixelCount = disp_area_pixel_count(&sPartialUpdate);
        area = &sPartialUpdate;
    }

    for (i = 0; i < u32AreaPixelCount; i++)
    {
        pu16Color[i] = (i & 0xFFFF);
        //pu16Color[i] = (0xF800 & 0xFFFF);
        //pu16Color[i] = (0x07E0 & 0xFFFF);
        //pu16Color[i] = (0x001F & 0xFFFF);
    }
    u32CheckSumW = crc32((uint8_t *)pu16Color, u32AreaPixelCount * sizeof(uint16_t));
    lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)area);

    for (i = 0; i < u32AreaPixelCount; i++)
    {
        pu16Color[i] = 0xFFFF;
    }

#if (NVT_DCACHE_ON == 1)
    SCB_CleanDCache_by_Addr((volatile void *)pu16Color, (int32_t)u32AreaPixelCount);
    SCB_InvalidateDCache_by_Addr((volatile void *)pu16Color, (int32_t)u32AreaPixelCount);
#endif

    start = GetSysTickCycleCount();
    lcd_device_control(evLCD_CTRL_RECT_READ, (void *)area);
    elapsed = GetSysTickCycleCount() - start;
    printf("\033[33m[%s] Read %d pixels, elpased %.2fms.\033[0m\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);

    u32CheckSumR = crc32((uint8_t *)pu16Color, u32AreaPixelCount * sizeof(uint16_t));
    if (u32CheckSumW != u32CheckSumR)
    {
        printf("\033[31mW != R (%08X != %08X)\033[0m\n", u32CheckSumW, u32CheckSumR);
        for (int i = 0; i < 16 ; i++)
        {
            if (pu16Color[i] != (i & 0xFFFF))
            {
                printf("[%d] %04X != %04X\n", i, (i & 0xFFFF), pu16Color[i]);
            }
        }
    }
    else
    {
        printf("\033[34mW == R (%08X = %08X)\033[0m\n", u32CheckSumW, u32CheckSumR);
    }
}

void demo_touchpad_getpoint(void)
{
    static numaker_indev_data_t s_data = {0};
    int count = 0;

    while (count++ < 3000)
    {
        touchpad_device_read(&s_data);
        printf("X(%d) Y(%d), State(%d)\n", s_data.point.x, s_data.point.y, s_data.state);
    }
}
