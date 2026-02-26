/**************************************************************************//**
 * @file     disp.h
 * @brief    for display driver
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2024 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __DISP_H__
#define __DISP_H__

#include <stdint.h>
#include "disp_glue.h"

#if defined(CONFIG_DISP_USE_PDMA)
    #include "drv_pdma.h"
#endif

/** Represents an area of the screen.*/
typedef struct
{
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
} disp_area_t;

#if defined(CONFIG_DISP_EBI)

    /* ===== EBI/8080 timing (unit: ns) ===== */
    #if defined(CONFIG_DISP_LT7381)
        #define DISP_HOR_RES_MAX          800
        #define DISP_VER_RES_MAX          480
        #define DISP_COLOR_DEPTH           16

        // For LT7381 EBI/8080 cycle timing
        #define CONFIG_DISPLAY_BOARD_NAME  "LCD5_V1.2 , LT7381"
        #define EBI_8080_ACCESS_NS   40
        #define EBI_8080_WR_IDLE_NS  20
        #define EBI_8080_WR_AHD_NS   5
        #define EBI_8080_RD_AHD_NS   5
        #define EBI_8080_RD_IDLE_NS  EBI_8080_WR_IDLE_NS

    #elif defined(CONFIG_DISP_FSA506)
        #define DISP_HOR_RES_MAX          480
        #define DISP_VER_RES_MAX          272
        #define DISP_COLOR_DEPTH           16

        // For FSA506 EBI/8080 cycle timing
        #define CONFIG_DISPLAY_BOARD_NAME  "LCD43_V1.0, FSA506"
        #define EBI_8080_ACCESS_NS   45
        #define EBI_8080_WR_IDLE_NS  40
        #define EBI_8080_WR_AHD_NS   0
        #define EBI_8080_RD_AHD_NS   0
        #define EBI_8080_RD_IDLE_NS  EBI_8080_WR_IDLE_NS

    #elif defined(CONFIG_DISP_NV3041A)
        #define DISP_HOR_RES_MAX          480
        #define DISP_VER_RES_MAX          272
        #define DISP_COLOR_DEPTH           16

        // For NV3041A EBI/8080 cycle timing
        #define CONFIG_DISPLAY_BOARD_NAME  "LCD43_V1.1, NV3041A"
        #define EBI_8080_ACCESS_NS   55
        #define EBI_8080_WR_IDLE_NS  20
        #define EBI_8080_WR_AHD_NS   20
        #define EBI_8080_RD_AHD_NS   20
        #define EBI_8080_RD_IDLE_NS  EBI_8080_WR_IDLE_NS

    #else

        #error "Select the EBI LCD panel board"

    #endif

    #define DISP_WRITE_REG(u16RegAddr)  (*((volatile uint16_t *)CONFIG_DISP_CMD_ADDR) = u16RegAddr)
    #define DISP_WRITE_DATA(u16Data)    (*((volatile uint16_t *)CONFIG_DISP_DAT_ADDR) = u16Data)
    #define DISP_READ_STATUS()          (*((volatile uint16_t *)CONFIG_DISP_CMD_ADDR))
    #define DISP_READ_DATA()            (*((volatile uint16_t *)CONFIG_DISP_DAT_ADDR))

#elif defined(CONFIG_DISP_SPI) //SPI/USPI

    #if defined(CONFIG_DISP_ILI9341)
        #define DISP_HOR_RES_MAX          320
        #define DISP_VER_RES_MAX          240
        #define DISP_COLOR_DEPTH           16

        #define CONFIG_DISPLAY_BOARD_NAME  "NUTFT_V1.3, ILI9341-SPI"
    #endif

    void DISP_WRITE_REG(uint8_t u8Cmd);
    void DISP_WRITE_DATA(uint8_t u8Dat);
#endif

#define CONFIG_VRAM_BUFFER_NUM              1
#define CONFIG_VRAM_TOTAL_ALLOCATED_SIZE    (CONFIG_VRAM_BUFFER_NUM * DISP_HOR_RES_MAX * DISP_VER_RES_MAX * (DISP_COLOR_DEPTH/8))

#define disp_delay_ms(ms)            sysDelay(ms)

void disp_write_reg(uint16_t reg, uint16_t data);
void disp_set_column(uint16_t StartCol, uint16_t EndCol);
void disp_set_page(uint16_t StartPage, uint16_t EndPage);
void disp_send_pixels(uint16_t *pixels, int byte_len);
void disp_fillrect(uint16_t *pixels, const disp_area_t *area);
int  disp_init(void);

#endif /* __DISP_H__ */
