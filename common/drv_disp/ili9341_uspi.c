/**************************************************************************//**
 * @file     ili9341_uspi.c
 * @brief    ili9431 uspi interface glue
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "numaker_disp.h"
#include "drv_uspi.h"

static struct nu_uspi s_NuUSPI =
{
    .base           = CONFIG_DISP_SPI,
#if defined(CONFIG_DISP_SPI_SS_PIN)
    .ss_pin         = CONFIG_DISP_SPI_SS_PIN,
#else
    .ss_pin         = -1,
#endif
#if defined(CONFIG_DISP_USE_PDMA)
    .pdma_perp_tx   = CONFIG_PDMA_SPI_TX,
    .pdma_chanid_tx = -1,
    .pdma_perp_rx   = CONFIG_PDMA_SPI_RX,
    .pdma_chanid_rx = -1,
    .m_psSemBus     = 0,
#endif
};

/**
 * @brief Write command byte to ILI9341 via USPI (Universal SPI) interface.
 *
 * Sets data/command line low, transfers 8-bit command via USPI, then sets
 * data/command line high for data phase.
 *
 * @param u8Cmd[in]  Command byte to send
 */
void DISP_WRITE_REG(uint8_t u8Cmd)
{
    USPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 8);

    /* Pull RS line low to indicate command transfer */
    DISP_CLR_RS;
    nu_uspi_transfer(&s_NuUSPI, (const void *)&u8Cmd, NULL, 1);
    /* Pull RS line high for subsequent data transfers */
    DISP_SET_RS;
}

/**
 * @brief Write data byte to ILI9341 via USPI interface.
 *
 * Transfers 8-bit data to the display controller via USPI.
 *
 * @param u8Dat[in]  Data byte to send
 */
void DISP_WRITE_DATA(uint8_t u8Dat)
{
    nu_uspi_transfer(&s_NuUSPI, (const void *)&u8Dat, NULL, 1);
}

/**
 * @brief Write 16-bit data to ILI9341 via USPI interface.
 *
 * Transfers 16-bit data to the display controller using 16-bit USPI mode.
 *
 * @param u16Dat[in]  16-bit data word to send
 */
static void DISP_WRITE_DATA_2B(uint16_t u16Dat)
{
    USPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 16);

    nu_uspi_transfer(&s_NuUSPI, (const void *)&u16Dat, NULL, 2);
}

/**
 * @brief Send pixel data to ILI9341 via USPI interface.
 *
 * Transfers 16-bit pixel color data using USPI transfers.
 *
 * @param pixels[in]    Pointer to pixel data array (16-bit RGB565)
 * @param byte_len[in]  Total length of data in bytes
 */
void disp_send_pixels(uint16_t *pixels, int byte_len)
{
    USPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 16);
    nu_uspi_transfer(&s_NuUSPI, (const void *)pixels, NULL, byte_len);
}

/**
 * @brief Set column address window for ILI9341 via USPI interface.
 *
 * Configures the horizontal address window using 16-bit data writes.
 *
 * @param StartCol[in]  Starting column address
 * @param EndCol[in]    Ending column address
 */
void disp_set_column(uint16_t StartCol, uint16_t EndCol)
{
    DISP_WRITE_REG(0x2A);
    DISP_WRITE_DATA_2B(StartCol);
    DISP_WRITE_DATA_2B(EndCol);
}

/**
 * @brief Set row/page address window for ILI9341 via USPI interface.
 *
 * Configures the vertical address window using 16-bit data writes.
 *
 * @param StartPage[in]  Starting row address
 * @param EndPage[in]    Ending row address
 */
void disp_set_page(uint16_t StartPage, uint16_t EndPage)
{
    DISP_WRITE_REG(0x2B);
    DISP_WRITE_DATA_2B(StartPage);
    DISP_WRITE_DATA_2B(EndPage);
}
