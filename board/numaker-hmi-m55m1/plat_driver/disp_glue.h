/**************************************************************************//**
 * @file     lv_glue.c
 * @brief    lvgl glue header
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __LV_GLUE_H__
#define __LV_GLUE_H__

#include <stdio.h>
#include <string.h>
#include "NuMicro.h"
#include "nu_misc.h"

#if !defined(PORT_OFFSET)
    #define PORT_OFFSET              0x40
#endif

#if !defined(GPIO_PIN_DATA)
    #define GPIO_PIN_DATA          GPIO_PIN_DATA_S
#endif

#if defined(USE_NUMAKER_TFT_LCD5_V_1_2)

    //For NUMAKER_TFT_LCD5_V1.2 board
    #define CONFIG_DISP_LT7381          1
    #define DISP_HOR_RES_MAX          800
    #define DISP_VER_RES_MAX          480
    #define DISP_COLOR_DEPTH           16

#elif defined(USE_NUMAKER_TFT_LCD43_V_1_1)

    //For NUMAKER_TFT_LCD43_V1.1 board
    #define CONFIG_DISP_NV3041A         1
    #define DISP_HOR_RES_MAX          480
    #define DISP_VER_RES_MAX          272
    #define DISP_COLOR_DEPTH           16

#elif defined(USE_NUMAKER_TFT_LCD43_V_1_0)

    //For NUMAKER_TFT_LCD43_V1.0 board
    #define CONFIG_DISP_FSA506          1
    #define DISP_HOR_RES_MAX          480
    #define DISP_VER_RES_MAX          272
    #define DISP_COLOR_DEPTH           16

#else

    #error "Select the LCD panel board"

#endif

/* ===== EBI/8080 timing (unit: ns) ===== */
#if defined(CONFIG_DISP_LT7381)

    // For LT7381 EBI/8080 cycle timing
		#define CONFIG_DISPLAY_BOARD_NAME  "LCD5_V1.2 , LT7381"
    #define EBI_8080_ACCESS_NS   35
    #define EBI_8080_WR_IDLE_NS  0
    #define EBI_8080_WR_AHD_NS   0
    #define EBI_8080_RD_AHD_NS   0
    #define EBI_8080_RD_IDLE_NS  EBI_8080_WR_IDLE_NS

#elif defined(CONFIG_DISP_FSA506)

    // For FSA506 EBI/8080 cycle timing
		#define CONFIG_DISPLAY_BOARD_NAME  "LCD43_V1.0, FSA506"
    #define EBI_8080_ACCESS_NS   45
    #define EBI_8080_WR_IDLE_NS  40
    #define EBI_8080_WR_AHD_NS   0
    #define EBI_8080_RD_AHD_NS   0
    #define EBI_8080_RD_IDLE_NS  EBI_8080_WR_IDLE_NS

#elif defined(CONFIG_DISP_NV3041A)

    // For NV3041A EBI/8080 cycle timing
		#define CONFIG_DISPLAY_BOARD_NAME  "LCD43_V1.1, NV3041A"
    #define EBI_8080_ACCESS_NS   55
    #define EBI_8080_WR_IDLE_NS  35
    #define EBI_8080_WR_AHD_NS   0
    #define EBI_8080_RD_AHD_NS   0
    #define EBI_8080_RD_IDLE_NS  EBI_8080_WR_IDLE_NS

#endif


/* FSA506/LT7381/NV3041A EBI */
#define CONFIG_DISP_EBI            EBI_BANK0
#define CONFIG_DISP_USE_PDMA
#define CONFIG_DISP_EBI_ADDR       (EBI_BANK0_BASE_ADDR+(CONFIG_DISP_EBI*EBI_MAX_SIZE))
#define CONFIG_DISP_CMD_ADDR       (CONFIG_DISP_EBI_ADDR+0x0)
#define CONFIG_DISP_DAT_ADDR       (CONFIG_DISP_EBI_ADDR+0x2)
#define CONFIG_DISP_PIN_BACKLIGHT  NU_GET_PININDEX(evGG, 5)
#define CONFIG_DISP_PIN_RESET      NU_GET_PININDEX(evGH, 6)
#define DISP_SET_RST               GPIO_PIN_DATA(NU_GET_PORT(CONFIG_DISP_PIN_RESET),     NU_GET_PIN(CONFIG_DISP_PIN_RESET)) = 1
#define DISP_CLR_RST               GPIO_PIN_DATA(NU_GET_PORT(CONFIG_DISP_PIN_RESET),     NU_GET_PIN(CONFIG_DISP_PIN_RESET)) = 0
#define DISP_SET_BACKLIGHT         GPIO_PIN_DATA(NU_GET_PORT(CONFIG_DISP_PIN_BACKLIGHT), NU_GET_PIN(CONFIG_DISP_PIN_BACKLIGHT)) = 1
#define DISP_CLR_BACKLIGHT         GPIO_PIN_DATA(NU_GET_PORT(CONFIG_DISP_PIN_BACKLIGHT), NU_GET_PIN(CONFIG_DISP_PIN_BACKLIGHT)) = 0

#define CONFIG_VRAM_BUFFER_NUM              1
#define CONFIG_VRAM_TOTAL_ALLOCATED_SIZE    NVT_ALIGN((CONFIG_VRAM_BUFFER_NUM * DISP_HOR_RES_MAX * DISP_VER_RES_MAX * (DISP_COLOR_DEPTH/8)), DCACHE_LINE_SIZE)

int lcd_device_initialize(void);
int lcd_device_finalize(void);
int lcd_device_open(void);
void lcd_device_close(void);
int lcd_device_control(int cmd, void *argv);

void sysDelay(uint32_t ms);

#endif /* __LV_GLUE_H__ */
