/**************************************************************************//**
 * @file     display_testcases.c
 * @brief    NuMaker Display Driver testcases routines
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "NuMicro.h"
#include "numaker_disp.h"
#include "numaker_touch.h"
#include "sw_crc.h"
#include "perf_ev.h"

#if defined(GDMA_BASE)
    #include "dma350_lib.h"
    #include "gdma.h"
#endif

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

void demo_lcd_readback_random(const S_LCD_INFO *psLcdInfo)
{
    int i;
    uint64_t start, elapsed;
    disp_area_t sRandArea;
    disp_area_t *area = &sRandArea;

    /* 1. Calculate the maximum number of pixels allowed by the allocated VRAM */
    uint32_t u32MaxPixels = CONFIG_VRAM_TOTAL_ALLOCATED_SIZE / psLcdInfo->u32BytePerPixel;
    uint32_t u32Width, u32Height;
    uint16_t u16Color;

    /* 2. Generate random width and height that fit within VRAM limits */
    do
    {
        // Random width between 1 and MAX_HOR
        u32Width  = (rand() % DISP_HOR_RES_MAX) + 1;
        // Random height between 1 and MAX_VER
        u32Height = (rand() % DISP_VER_RES_MAX) + 1;
    }
    while ((u32Width * u32Height) > u32MaxPixels);

    /* 3. Generate random X, Y start coordinates within display boundaries */
    area->x1 = rand() % (DISP_HOR_RES_MAX - u32Width + 1);
    area->y1 = rand() % (DISP_VER_RES_MAX - u32Height + 1);
    area->x2 = area->x1 + u32Width - 1;
    area->y2 = area->y1 + u32Height - 1;

    uint32_t u32AreaPixelCount = disp_area_pixel_count(area);
    uint16_t *pu16Color = (uint16_t *)psLcdInfo->pvVramStartAddr;
    uint32_t u32CheckSumW, u32CheckSumR;

    printf("\033[32m[Random Test] Area: (%d,%d) to (%d,%d), Pixels: %u\033[0m\n",
           area->x1, area->y1, area->x2, area->y2, u32AreaPixelCount);

    /* 4. Render test pattern to VRAM (Shadow Buffer) */
    u16Color = rand() & 0xFFFF;
    for (i = 0; i < u32AreaPixelCount; i++)
    {
        pu16Color[i] = u16Color;
    }

    /* Calculate Checksum of the written data */
    u32CheckSumW = crc32((uint8_t *)pu16Color, u32AreaPixelCount * sizeof(uint16_t));

    /* Push data to LCD Controller (Write operation) */
    lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)area);

    /* 5. Trap: Overwrite local VRAM with 0xFFFF to ensure read-back is authentic */
    for (i = 0; i < u32AreaPixelCount; i++)
    {
        pu16Color[i] = 0xFFFF;
    }

#if (NVT_DCACHE_ON == 1)
    /* Flush and Invalidate Cache to ensure CPU reads directly from physical memory/bus */
    SCB_CleanDCache_by_Addr((volatile void *)pu16Color, (int32_t)(u32AreaPixelCount * sizeof(uint16_t)));
    SCB_InvalidateDCache_by_Addr((volatile void *)pu16Color, (int32_t)(u32AreaPixelCount * sizeof(uint16_t)));
#endif

    /* 6. Read back data from LCD Controller */
    start = GetSysTickCycleCount();
    lcd_device_control(evLCD_CTRL_RECT_READ, (void *)area);
    elapsed = GetSysTickCycleCount() - start;

    /* 7. Verify Data Integrity */
    u32CheckSumR = crc32((uint8_t *)pu16Color, u32AreaPixelCount * sizeof(uint16_t));

    if (u32CheckSumW != u32CheckSumR)
    {
        printf("\033[31mFAIL: W != R (W:%08X, R:%08X)\033[0m\n", u32CheckSumW, u32CheckSumR);
//        /* Debug: Print first few mismatches */
//        for (int j = 0; j < 16 && j < u32AreaPixelCount; j++)
//        {
//            if (pu16Color[j] != u16Color)
//            {
//                printf("  Mismatch at [%d]: Expected %04X, Got %04X\n", j, u16Color, pu16Color[j]);
//            }
//        }
    }
    else
    {
        printf("\033[34mPASS: W == R (%08X)\033[0m\n", u32CheckSumW);
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

#if defined(CONFIG_DISP_EBI)

/**
 * @brief Performs a pixel readback test on a random area.
 * @return 0 on success (stable), -1 on failure (unstable)
 */
static int verify_ebi_stability(const S_LCD_INFO *psLcdInfo)
{
    disp_area_t sArea;
    uint32_t u32MaxPixels = CONFIG_VRAM_TOTAL_ALLOCATED_SIZE / psLcdInfo->u32BytePerPixel;
    uint16_t *pu16Color = (uint16_t *)psLcdInfo->pvVramStartAddr;

    // Generate random area
    uint32_t w = (rand() % DISP_HOR_RES_MAX) + 1;
    uint32_t h = (rand() % DISP_VER_RES_MAX) + 1;
    if ((w * h) > u32MaxPixels)
    {
        w = u32MaxPixels / h; // Clamp to VRAM size
    }

    sArea.x1 = rand() % (DISP_HOR_RES_MAX - w + 1);
    sArea.y1 = rand() % (DISP_VER_RES_MAX - h + 1);
    sArea.x2 = sArea.x1 + w - 1;
    sArea.y2 = sArea.y1 + h - 1;

    uint32_t pixelCount = disp_area_pixel_count(&sArea);

    // Write pattern
    for (int i = 0; i < pixelCount; i++)
    {
        pu16Color[i] = (uint16_t)(i & 0xFFFF);
    }

    uint32_t crcW = crc32((uint8_t *)pu16Color, pixelCount * sizeof(uint16_t));
    lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sArea);

    // Clear local buffer to trap false positives
    for (int i = 0; i < pixelCount; i++)
    {
        pu16Color[i] = 0x5AA5;
    }

#if (NVT_DCACHE_ON == 1)
    SCB_CleanDCache_by_Addr((volatile void *)pu16Color, (int32_t)(pixelCount * sizeof(uint16_t)));
    SCB_InvalidateDCache_by_Addr((volatile void *)pu16Color, (int32_t)(pixelCount * sizeof(uint16_t)));
#endif

    // Read back
    lcd_device_control(evLCD_CTRL_RECT_READ, (void *)&sArea);

    // Verify
    uint32_t crcR = crc32((uint8_t *)pu16Color, pixelCount * sizeof(uint16_t));
    if (crcW != crcR)
    {
        printf("crcW: %08x, crcR:%08x\n", crcW, crcR);
    }
    return (crcW == crcR) ? 0 : -1;
}

/**
 * @brief Attempts to apply EBI timings and validates them via readback.
 * @return 0 if parameters are stable at any MCLKDiv, -1 otherwise.
 */
static int EBI_ApplyAndVerifyTiming(const S_LCD_INFO *psLcdInfo,
                                    int acc_ns, int wr_idle_ns,
                                    int wr_ahd_ns, int rd_ahd_ns, int rd_idle_ns)
{
    int i32MCLKDiv;

    if (EBI_ApplyTiming(acc_ns,
                        wr_idle_ns,
                        wr_ahd_ns,
                        rd_ahd_ns,
                        rd_idle_ns) < 0)
    {
        printf("EBI_ApplyTiming failre\n");
        return -1;
    }
    else
    {
        // Run readback test multiple times to ensure stability
#define RETRY 64
        for (int trial = 0; trial < RETRY; trial++)
        {
            if (verify_ebi_stability(psLcdInfo) != 0)
            {
                printf("\033[31m[FAILURE] %d/%d\033[0m\n", trial, RETRY);

                return -1;
            }
        }
    }

    return 0;
}

/**
 * @brief Automated search to find the minimum stable nanoseconds for all EBI parameters.
 * It optimizes TACC first, followed by Idle and Address Hold timings.
 */
void EBI_AutomatedSearch(const S_LCD_INFO *psLcdInfo)
{
    return ; //Not ready.

    const int EBI_1MCLK_NS = 5;
    const int MARGIN = EBI_1MCLK_NS;

    int start_acc = EBI_8080_ACCESS_NS;
    int start_ahd = EBI_8080_WR_AHD_NS;
    int start_widle = EBI_8080_WR_IDLE_NS;
    int start_ridle = EBI_8080_RD_IDLE_NS;

    int best_acc = start_acc, best_ahd = start_ahd;
    int best_widle = start_widle, best_ridle = start_ridle;
    int min_total_ns = 9999;

    printf("\n\033[36m--- Starting 2D Matrix EBI Optimization ---\033[0m\n");

    for (int acc = start_acc; acc >= 10; acc -= EBI_1MCLK_NS)
    {
        int acc_ok = 0;
        for (int ahd = start_ahd; ahd >= 5; ahd -= EBI_1MCLK_NS)
        {
            if (EBI_ApplyAndVerifyTiming(psLcdInfo, acc, start_widle, ahd, ahd, start_ridle) == 0)
            {
                acc_ok = 1;
                if ((acc + ahd) < min_total_ns)
                {
                    min_total_ns = acc + ahd;
                    best_acc = acc;
                    best_ahd = ahd;
                }
            }
            else
            {
                break;
            }
        }
        if (!acc_ok && acc < start_acc / 2) break;
    }

    for (int idle = start_widle; idle >= 0; idle -= EBI_1MCLK_NS)
    {
        if (EBI_ApplyAndVerifyTiming(psLcdInfo, best_acc, idle, best_ahd, best_ahd, start_ridle) == 0)
        {
            best_widle = idle;
        }
        else
        {
            break;
        }
    }

    for (int idle = start_ridle; idle >= 0; idle -= EBI_1MCLK_NS)
    {
        if (EBI_ApplyAndVerifyTiming(psLcdInfo, best_acc, best_widle, best_ahd, best_ahd, idle) == 0)
        {
            best_ridle = idle;
        }
        else
        {
            break;
        }
    }

    printf("\n\033[32m--- Optimization Complete ---\033[0m\n");
    disp_init();
    EBI_ApplyTiming(best_acc,
                    best_widle,
                    best_ahd,
                    best_ahd,
                    best_ridle);

    printf("Applied: ACC=%d, WIDLE=%d, AHD=%d, RIDLE=%d,\n",
           best_acc, best_widle, best_ahd, best_ridle);
}
#endif

#if defined(GDMA_BASE)

extern uint8_t acimageSRC_200x200[80016UL + 1] __attribute__((aligned(4)));

void demo_lcd_gdma_2d_copy(const S_LCD_INFO *psLcdInfo)
{
    uint64_t start, elapsed;
    uint32_t u32AreaPixelCount;

    /* Full-screen area. */
    disp_area_t sArea_clean = { 0, 0, psLcdInfo->u32ResWidth - 1, psLcdInfo->u32ResHeight - 1};

    /* Clean shadow buffer to zero. */
    memset(psLcdInfo->pvVramStartAddr, 0, psLcdInfo->u32VramSize);

    /* Flush shadow buffer to specified area on LCD panel. */
    lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sArea_clean);

    {
        /* Top-left */
        disp_area_t sArea_memcpy = { 0, 0, (200 - 1), (200 - 1) };
        u32AreaPixelCount = disp_area_pixel_count(&sArea_memcpy);

        start = GetSysTickCycleCount();

        /* Copy to shadow buffer. */
        memcpy(psLcdInfo->pvVramStartAddr, acimageSRC_200x200 + 16, sizeof(acimageSRC_200x200) - 16);

        /* Flush shadow buffer to specified area on LCD panel. */
        lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sArea_memcpy);

        elapsed = GetSysTickCycleCount() - start;

        printf("\033[32m[%s] CPU-Render+flush %d pixels, elpased %.2fms.\033[0m\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);
    }

    {
        /* Right-bottom */
        disp_area_t sArea_gdma = { DISP_HOR_RES_MAX - 200, DISP_VER_RES_MAX - 200, DISP_HOR_RES_MAX - 1, DISP_VER_RES_MAX - 1 };
        u32AreaPixelCount = disp_area_pixel_count(&sArea_gdma);

        start = GetSysTickCycleCount();
        /* Use GDMA to copy pixel data to shadow buffer. */
        dma350_draw_from_canvas(
            GDMA_CH_DEV_S[0],                        // DMA350 channel device structure
            (const void *)(acimageSRC_200x200 + 16), // Source address, top left corner
            psLcdInfo->pvVramStartAddr,              // Destination address, top left corner,
            200, 200, 200,                           // src_width, src_height, source stride(line width of image)
            200, 200, 200,                           // dest_width, dest_height, dest_stride(line width of buffer)
            DMA350_CH_TRANSIZE_16BITS,               // Size of a pixel, dma350_ch_transize_t
            DMA350_LIB_TRANSFORM_NONE,               // Transform, dma350_lib_transform_t
            DMA350_LIB_EXEC_BLOCKING);               // exec_type, dma350_lib_exec_type_t

        /* Flush shadow buffer to specified area on LCD panel. */
        lcd_device_control(evLCD_CTRL_RECT_UPDATE, (void *)&sArea_gdma);

        elapsed = GetSysTickCycleCount() - start;

        printf("\033[32m[%s] GDMA-Render+flush %d pixels, elpased %.2fms.\033[0m\n", CONFIG_DISPLAY_BOARD_NAME, u32AreaPixelCount, (double)elapsed * 1000.0 / SystemCoreClock);
    }

    /* Optional: delay to see color */
    TIMER_Delay(TIMER0, 500000);
}
#endif