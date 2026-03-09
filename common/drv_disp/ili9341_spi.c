/**************************************************************************//**
 * @file     ili9341_spi.c
 * @brief    ili9431 spi interface glue
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "numaker_disp.h"
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
    SPI_SET_DATA_WIDTH(CONFIG_DISP_SPI, 8);

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

int ili9341_spi_send_then_recv(struct nu_spi *psNuSPI, const uint8_t *tx, int tx_len, uint8_t *rx, int rx_len, int dw)
{
    SPI_SET_DATA_WIDTH(psNuSPI->base, dw * 8);

    if (psNuSPI->ss_pin > 0)
    {
        GPIO_PIN_DATA(NU_GET_PORT(psNuSPI->ss_pin), NU_GET_PIN(psNuSPI->ss_pin)) = 0;
    }
    else
    {
        SPI_SET_SS_LOW(psNuSPI->base);
    }

    if (tx)
    {
        DISP_CLR_RS;
        int sent = 0;
        while (sent < tx_len)
        {
            sent += nu_spi_write(psNuSPI->base, tx + sent, dw);
        }
        while (SPI_IS_BUSY(psNuSPI->base));
    }

    DISP_SET_RS;

    /* Clear SPI RX FIFO */
    nu_spi_drain_rxfifo(psNuSPI->base);

    if (rx)
    {
        uint32_t dummy_tx = 0xFFFFFFFF;
        uint8_t *curr_rx = rx;
        int remaining = rx_len;
        int received = 0;

        while (received < rx_len)
        {
            if (remaining > 0)
            {
                int w = nu_spi_write(psNuSPI->base, (uint8_t *)&dummy_tx, dw);
                remaining -= w;
            }

            received += nu_spi_read(psNuSPI->base, curr_rx + received, dw);
        }
    }

    if (psNuSPI->ss_pin > 0)
    {
        GPIO_PIN_DATA(NU_GET_PORT(psNuSPI->ss_pin), NU_GET_PIN(psNuSPI->ss_pin)) = 1;
    }
    else
    {
        SPI_SET_SS_HIGH(psNuSPI->base);
    }

    return 0;
}

void disp_receive_pixels(uint16_t *pixels, int byte_len)
{
//    uint8_t cmd = 0x2E;
//    ili9341_spi_send_then_recv(&s_NuSPI, (const uint8_t*)&cmd, 1, (uint8_t *)pixels, 4, 1);
}
