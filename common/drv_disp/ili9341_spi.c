/**************************************************************************//**
 * @file     ili9341_spi.c
 * @brief    ili9431 spi interface glue
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "disp.h"
#include "drv_spi.h"

static struct nu_spi s_NuSPI =
{
    .base           = CONFIG_DISP_SPI,
    .ss_pin         = -1,
#if defined(CONFIG_DISP_USE_PDMA)
    .pdma_perp_tx   = CONFIG_PDMA_SPI_TX,
    .pdma_chanid_tx = -1,
    .pdma_perp_rx   = CONFIG_PDMA_SPI_RX,
    .pdma_chanid_rx = -1,
    .m_psSemBus     = 0,
#endif
};

void DISP_WRITE_REG(uint8_t u8Cmd)
{
    SPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 8);

    DISP_CLR_RS;
    nu_spi_transfer(&s_NuSPI, (const void *)&u8Cmd, NULL, 1);
    DISP_SET_RS;
}

void DISP_WRITE_DATA(uint8_t u8Dat)
{
    nu_spi_transfer(&s_NuSPI, (const void *)&u8Dat, NULL, 1);
}

static void DISP_WRITE_DATA_2B(uint16_t u16Dat)
{
    SPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 16);
    nu_spi_transfer(&s_NuSPI, (const void *)&u16Dat, NULL, 2);
}

void disp_set_column(uint16_t StartCol, uint16_t EndCol)
{
    DISP_WRITE_REG(0x2A);
    DISP_WRITE_DATA_2B(StartCol);
    DISP_WRITE_DATA_2B(EndCol);
}

void disp_set_page(uint16_t StartPage, uint16_t EndPage)
{
    DISP_WRITE_REG(0x2B);
    DISP_WRITE_DATA_2B(StartPage);
    DISP_WRITE_DATA_2B(EndPage);
}

void disp_send_pixels(uint16_t *pixels, int byte_len)
{
    SPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 16);
    nu_spi_transfer(&s_NuSPI, (const void *)pixels, NULL, byte_len);
}

static void debug_read_id(void)
{
    uint8_t cmd = 0x04;
    uint8_t id_data[4];

    nu_spi_send_then_recv(&s_NuSPI, (const uint8_t *)&cmd, 1, id_data, 4, 1);
    printf("LCD ID: %02X %02X %02X %02X\n", id_data[0], id_data[1], id_data[2], id_data[3]);
}

static void disp_receive_pixel(uint16_t *pixel)
{
    typedef union
    {
        uint32_t rgbx;
        struct
        {
            uint8_t x;
            uint8_t r;
            uint8_t g;
            uint8_t b;
        } S;
    } ili9341_pixel;
    ili9341_pixel bgrx;

    uint8_t cmd = 0x2E;
    nu_spi_send_then_recv(&s_NuSPI, (const uint8_t *)&cmd, 1, (uint8_t *)&bgrx, sizeof(bgrx), 1);
    printf("%08x\n", bgrx.rgbx);
}

void disp_receive_pixels(uint16_t *pixels, int byte_len)
{
    //debug_read_id();
    //disp_receive_pixel(pixels);
}
