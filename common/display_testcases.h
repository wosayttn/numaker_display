/**************************************************************************//**
 * @file     display_testcases.h
 * @brief    Display test cases
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2026 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __DISPLAY_TESTCASES_H__
#define __DISPLAY_TESTCASES_H__

#include "numaker_disp.h"
#include "numaker_touch.h"

uint32_t crc32(uint8_t *ptr, uint32_t len);

void demo_lcd_flush(const S_LCD_INFO *psLcdInfo);
void demo_lcd_readback(const S_LCD_INFO *psLcdInfo);
void demo_lcd_readback_random(const S_LCD_INFO *psLcdInfo);
void demo_touchpad_getpoint(void);
void EBI_AutomatedSearch(const S_LCD_INFO *psLcdInfo);

#endif //__DISPLAY_TESTCASES_H__
