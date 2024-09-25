// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C2410

    (c) 2010 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "s3c2410.h"

#include "screen.h"


#define S3C24XX_SRCPND    (0x00 / 4) // Interrupt Request Status
#define S3C24XX_INTMOD    (0x04 / 4) // Interrupt Mode Control
#define S3C24XX_INTMSK    (0x08 / 4) // Interrupt Mask Control
#define S3C24XX_PRIORITY  (0x0C / 4) // IRQ Priority Control
#define S3C24XX_INTPND    (0x10 / 4) // Interrupt Request Status
#define S3C24XX_INTOFFSET (0x14 / 4) // Interrupt Request Source Offset
#define S3C24XX_SUBSRCPND (0x18 / 4) // Sub Source Pending
#define S3C24XX_INTSUBMSK (0x1C / 4) // Interrupt Sub Mask

#define S3C24XX_DISRC     (0x00 / 4) // DMA Initial Source
#define S3C24XX_DISRCC    (0x04 / 4) // DMA Initial Source Control
#define S3C24XX_DIDST     (0x08 / 4) // DMA Initial Destination
#define S3C24XX_DIDSTC    (0x0C / 4) // DMA Initial Destination Control
#define S3C24XX_DCON      (0x10 / 4) // DMA Control
#define S3C24XX_DSTAT     (0x14 / 4) // DMA Count
#define S3C24XX_DCSRC     (0x18 / 4) // DMA Current Source
#define S3C24XX_DCDST     (0x1C / 4) // DMA Current Destination
#define S3C24XX_DMASKTRIG (0x20 / 4) // DMA Mask Trigger

#define S3C24XX_LCDCON1   (0x00 / 4) // LCD Control 1
#define S3C24XX_LCDCON2   (0x04 / 4) // LCD Control 2
#define S3C24XX_LCDCON3   (0x08 / 4) // LCD Control 3
#define S3C24XX_LCDCON4   (0x0C / 4) // LCD Control 4
#define S3C24XX_LCDCON5   (0x10 / 4) // LCD Control 5
#define S3C24XX_LCDSADDR1 (0x14 / 4) // STN/TFT: Frame Buffer Start Address 1
#define S3C24XX_LCDSADDR2 (0x18 / 4) // STN/TFT: Frame Buffer Start Address 2
#define S3C24XX_LCDSADDR3 (0x1C / 4) // STN/TFT: Virtual Screen Address Set
#define S3C24XX_REDLUT    (0x20 / 4) // STN: Red Lookup Table
#define S3C24XX_GREENLUT  (0x24 / 4) // STN: Green Lookup Table
#define S3C24XX_BLUELUT   (0x28 / 4) // STN: Blue Lookup Table
#define S3C24XX_DITHMODE  (0x4C / 4) // STN: Dithering Mode
#define S3C24XX_TPAL      (0x50 / 4) // TFT: Temporary Palette
#define S3C24XX_LCDINTPND (0x54 / 4) // LCD Interrupt Pending
#define S3C24XX_LCDSRCPND (0x58 / 4) // LCD Interrupt Source
#define S3C24XX_LCDINTMSK (0x5C / 4) // LCD Interrupt Mask
#define S3C24XX_LPCSEL    (0x60 / 4) // LPC3600 Control

#define S3C24XX_NFCONF (0x00 / 4) // NAND Flash Configuration
#define S3C24XX_NFCMD  (0x04 / 4) // NAND Flash Command
#define S3C24XX_NFADDR (0x08 / 4) // NAND Flash Address
#define S3C24XX_NFDATA (0x0C / 4) // NAND Flash Data
#define S3C24XX_NFSTAT (0x10 / 4) // NAND Flash Operation Status
#define S3C24XX_NFECC  (0x14 / 4) // NAND Flash ECC

#define S3C24XX_IICCON  (0x00 / 4) // IIC Control
#define S3C24XX_IICSTAT (0x04 / 4) // IIC Status
#define S3C24XX_IICADD  (0x08 / 4) // IIC Address
#define S3C24XX_IICDS   (0x0C / 4) // IIC Data Shift

#define S3C24XX_GPACON   (0x00 / 4) // Port A Control
#define S3C24XX_GPADAT   (0x04 / 4) // Port A Data
#define S3C24XX_GPBCON   (0x10 / 4) // Port B Control
#define S3C24XX_GPBDAT   (0x14 / 4) // Port B Data
#define S3C24XX_GPBUP    (0x18 / 4) // Pull-up Control B
#define S3C24XX_GPCCON   (0x20 / 4) // Port C Control
#define S3C24XX_GPCDAT   (0x24 / 4) // Port C Data
#define S3C24XX_GPCUP    (0x28 / 4) // Pull-up Control C
#define S3C24XX_GPDCON   (0x30 / 4) // Port D Control
#define S3C24XX_GPDDAT   (0x34 / 4) // Port D Data
#define S3C24XX_GPDUP    (0x38 / 4) // Pull-up Control D
#define S3C24XX_GPECON   (0x40 / 4) // Port E Control
#define S3C24XX_GPEDAT   (0x44 / 4) // Port E Data
#define S3C24XX_GPEUP    (0x48 / 4) // Pull-up Control E
#define S3C24XX_GPFCON   (0x50 / 4) // Port F Control
#define S3C24XX_GPFDAT   (0x54 / 4) // Port F Data
#define S3C24XX_GPFUP    (0x58 / 4) // Pull-up Control F
#define S3C24XX_GPGCON   (0x60 / 4) // Port G Control
#define S3C24XX_GPGDAT   (0x64 / 4) // Port G Data
#define S3C24XX_GPGUP    (0x68 / 4) // Pull-up Control G
#define S3C24XX_GPHCON   (0x70 / 4) // Port H Control
#define S3C24XX_GPHDAT   (0x74 / 4) // Port H Data
#define S3C24XX_GPHUP    (0x78 / 4) // Pull-up Control H
#define S3C24XX_MISCCR   (0x80 / 4) // Miscellaneous Control
#define S3C24XX_DCLKCON  (0x84 / 4) // DCLK0/1 Control
#define S3C24XX_EXTINT0  (0x88 / 4) // External Interrupt Control Register 0
#define S3C24XX_EXTINT1  (0x8C / 4) // External Interrupt Control Register 1
#define S3C24XX_EXTINT2  (0x90 / 4) // External Interrupt Control Register 2
#define S3C24XX_EINTFLT0 (0x94 / 4) // Reserved
#define S3C24XX_EINTFLT1 (0x98 / 4) // Reserved
#define S3C24XX_EINTFLT2 (0x9C / 4) // External Interrupt Filter Control Register 2
#define S3C24XX_EINTFLT3 (0xA0 / 4) // External Interrupt Filter Control Register 3
#define S3C24XX_EINTMASK (0xA4 / 4) // External Interrupt Mask
#define S3C24XX_EINTPEND (0xA8 / 4) // External Interrupt Pending
#define S3C24XX_GSTATUS0 (0xAC / 4) // External Pin Status
#define S3C24XX_GSTATUS1 (0xB0 / 4) // Chip ID
#define S3C24XX_GSTATUS2 (0xB4 / 4) // Reset Status
#define S3C24XX_GSTATUS3 (0xB8 / 4) // Inform Register
#define S3C24XX_GSTATUS4 (0xBC / 4) // Inform Register

#define S3C24XX_GPADAT_MASK 0x007FFFFF
#define S3C24XX_GPBDAT_MASK 0x000007FF
#define S3C24XX_GPCDAT_MASK 0x0000FFFF
#define S3C24XX_GPDDAT_MASK 0x0000FFFF
#define S3C24XX_GPEDAT_MASK 0x0000FFFF
#define S3C24XX_GPFDAT_MASK 0x000000FF
#define S3C24XX_GPGDAT_MASK 0x0000FFFF
#define S3C24XX_GPHDAT_MASK 0x000007FF

#define S3C24XX_ADCCON  (0x00 / 4) // ADC Control
#define S3C24XX_ADCTSC  (0x04 / 4) // ADC Touch Screen Control
#define S3C24XX_ADCDLY  (0x08 / 4) // ADC Start or Interval Delay
#define S3C24XX_ADCDAT0 (0x0C / 4) // ADC Conversion Data
#define S3C24XX_ADCDAT1 (0x10 / 4) // ADC Conversion Data

#define S3C24XX_INT_ADC       31
#define S3C24XX_INT_RTC       30
#define S3C24XX_INT_SPI1      29
#define S3C24XX_INT_UART0     28
#define S3C24XX_INT_IIC       27
#define S3C24XX_INT_USBH      26
#define S3C24XX_INT_USBD      25
#define S3C24XX_INT_24        24
#define S3C24XX_INT_UART1     23
#define S3C24XX_INT_SPI0      22
#define S3C24XX_INT_SDI       21
#define S3C24XX_INT_DMA3      20
#define S3C24XX_INT_DMA2      19
#define S3C24XX_INT_DMA1      18
#define S3C24XX_INT_DMA0      17
#define S3C24XX_INT_LCD       16
#define S3C24XX_INT_UART2     15
#define S3C24XX_INT_TIMER4    14
#define S3C24XX_INT_TIMER3    13
#define S3C24XX_INT_TIMER2    12
#define S3C24XX_INT_TIMER1    11
#define S3C24XX_INT_TIMER0    10
#define S3C24XX_INT_WDT        9
#define S3C24XX_INT_TICK       8
#define S3C24XX_INT_BATT_FLT   7
#define S3C24XX_INT_6          6
#define S3C24XX_INT_EINT8_23   5
#define S3C24XX_INT_EINT4_7    4
#define S3C24XX_INT_EINT3      3
#define S3C24XX_INT_EINT2      2
#define S3C24XX_INT_EINT1      1
#define S3C24XX_INT_EINT0      0

#define S3C24XX_SUBINT_ADC  10
#define S3C24XX_SUBINT_TC    9
#define S3C24XX_SUBINT_ERR2  8
#define S3C24XX_SUBINT_TXD2  7
#define S3C24XX_SUBINT_RXD2  6
#define S3C24XX_SUBINT_ERR1  5
#define S3C24XX_SUBINT_TXD1  4
#define S3C24XX_SUBINT_RXD1  3
#define S3C24XX_SUBINT_ERR0  2
#define S3C24XX_SUBINT_TXD0  1
#define S3C24XX_SUBINT_RXD0  0

static const uint32_t MAP_SUBINT_TO_INT[11] =
{
	S3C24XX_INT_UART0, S3C24XX_INT_UART0, S3C24XX_INT_UART0,
	S3C24XX_INT_UART1, S3C24XX_INT_UART1, S3C24XX_INT_UART1,
	S3C24XX_INT_UART2, S3C24XX_INT_UART2, S3C24XX_INT_UART2,
	S3C24XX_INT_ADC, S3C24XX_INT_ADC
};

#define S3C24XX_BPPMODE_STN_01      0x00
#define S3C24XX_BPPMODE_STN_02      0x01
#define S3C24XX_BPPMODE_STN_04      0x02
#define S3C24XX_BPPMODE_STN_08      0x03
#define S3C24XX_BPPMODE_STN_12_P    0x04
#define S3C24XX_BPPMODE_STN_12_U    0x05
#define S3C24XX_BPPMODE_STN_16      0x06
#define S3C24XX_BPPMODE_TFT_01      0x08
#define S3C24XX_BPPMODE_TFT_02      0x09
#define S3C24XX_BPPMODE_TFT_04      0x0A
#define S3C24XX_BPPMODE_TFT_08      0x0B
#define S3C24XX_BPPMODE_TFT_16      0x0C
#define S3C24XX_BPPMODE_TFT_24      0x0D

#define S3C24XX_PNRMODE_STN_04_DS  0
#define S3C24XX_PNRMODE_STN_04_SS  1
#define S3C24XX_PNRMODE_STN_08_SS  2
#define S3C24XX_PNRMODE_TFT        3

#define S3C24XX_GPIO_PORT_A S3C2410_GPIO_PORT_A
#define S3C24XX_GPIO_PORT_B S3C2410_GPIO_PORT_B
#define S3C24XX_GPIO_PORT_C S3C2410_GPIO_PORT_C
#define S3C24XX_GPIO_PORT_D S3C2410_GPIO_PORT_D
#define S3C24XX_GPIO_PORT_E S3C2410_GPIO_PORT_E
#define S3C24XX_GPIO_PORT_F S3C2410_GPIO_PORT_F
#define S3C24XX_GPIO_PORT_G S3C2410_GPIO_PORT_G
#define S3C24XX_GPIO_PORT_H S3C2410_GPIO_PORT_H

#define S3C24XX_CORE_PIN_NCON S3C2410_CORE_PIN_NCON
#define S3C24XX_CORE_PIN_OM0  S3C2410_CORE_PIN_OM0
#define S3C24XX_CORE_PIN_OM1  S3C2410_CORE_PIN_OM1

#define DEVICE_S3C2410
#define S3C24_CLASS_NAME s3c2410_device
#include "machine/s3c24xx.hxx"
#undef DEVICE_S3C2410

uint32_t s3c2410_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return s3c24xx_video_update(screen, bitmap, cliprect);
}

DEFINE_DEVICE_TYPE(S3C2410, s3c2410_device, "s3c2410", "Samsung S3C2410 SoC")

s3c2410_device::s3c2410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S3C2410, tag, owner, clock)
	, m_cpu(*this, ":maincpu")
	, m_palette(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_pin_r_cb(*this, 0)
	, m_pin_w_cb(*this)
	, m_port_r_cb(*this, 0)
	, m_port_w_cb(*this)
	, m_scl_w_cb(*this)
	, m_sda_r_cb(*this, 1)
	, m_sda_w_cb(*this)
	, m_data_r_cb(*this, 0)
	, m_data_w_cb(*this)
	, m_command_w_cb(*this)
	, m_address_w_cb(*this)
	, m_nand_data_r_cb(*this, 0)
	, m_nand_data_w_cb(*this)
	, m_flags(0)
{
	memset(m_steppingstone, 0, sizeof(m_steppingstone));
	memset(&m_memcon, 0, sizeof(m_memcon));
	memset(&m_usbhost, 0, sizeof(m_usbhost));
	memset(&m_irq, 0, sizeof(m_irq));
	memset(m_dma, 0, sizeof(m_dma));
	memset(&m_clkpow, 0, sizeof(m_clkpow));
	memset(&m_lcd, 0, sizeof(m_lcd));
	memset(&m_lcdpal, 0, sizeof(m_lcdpal));
	memset(&m_nand, 0, sizeof(m_nand));
	memset(m_uart, 0, sizeof(m_uart));
	memset(&m_pwm, 0, sizeof(m_pwm));
	memset(&m_usbdev, 0, sizeof(m_usbdev));
	memset(&m_wdt, 0, sizeof(m_wdt));
	memset(&m_iic, 0, sizeof(m_iic));
	memset(&m_iis, 0, sizeof(m_iis));
	memset(&m_gpio, 0, sizeof(m_gpio));
	memset(&m_rtc, 0, sizeof(m_rtc));
	memset(&m_adc, 0, sizeof(m_adc));
	memset(m_spi, 0, sizeof(m_spi));
	memset(&m_sdi, 0, sizeof(m_sdi));
}

s3c2410_device::~s3c2410_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c2410_device::device_start()
{
	s3c24xx_device_start();

	address_space &space = m_cpu->space(AS_PROGRAM);
	space.install_readwrite_handler( 0x48000000, 0x4800003b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_memcon_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_memcon_w)));
	space.install_readwrite_handler( 0x49000000, 0x4900005b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_usb_host_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_usb_host_w)));
	space.install_readwrite_handler( 0x4a000000, 0x4a00001f, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_irq_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_irq_w)));
	space.install_readwrite_handler( 0x4b000000, 0x4b000023, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_0_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_0_w)));
	space.install_readwrite_handler( 0x4b000040, 0x4b000063, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_1_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_1_w)));
	space.install_readwrite_handler( 0x4b000080, 0x4b0000a3, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_2_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_2_w)));
	space.install_readwrite_handler( 0x4b0000c0, 0x4b0000e3, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_3_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_dma_3_w)));
	space.install_readwrite_handler( 0x4c000000, 0x4c000017, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_clkpow_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_clkpow_w)));
	space.install_readwrite_handler( 0x4d000000, 0x4d000063, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_lcd_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_lcd_w)));
	space.install_readwrite_handler( 0x4d000400, 0x4d0007ff, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_lcd_palette_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_lcd_palette_w)));
	space.install_readwrite_handler( 0x4e000000, 0x4e000017, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_nand_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_nand_w)));
	space.install_readwrite_handler( 0x50000000, 0x5000002b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_uart_0_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_uart_0_w)));
	space.install_readwrite_handler( 0x50004000, 0x5000402b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_uart_1_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_uart_1_w)));
	space.install_readwrite_handler( 0x50008000, 0x5000802b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_uart_2_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_uart_2_w)));
	space.install_readwrite_handler( 0x51000000, 0x51000043, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_pwm_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_pwm_w)));
	space.install_readwrite_handler( 0x52000140, 0x5200026f, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_usb_device_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_usb_device_w)));
	space.install_readwrite_handler( 0x53000000, 0x5300000b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_wdt_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_wdt_w)));
	space.install_readwrite_handler( 0x54000000, 0x5400000f, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_iic_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_iic_w)));
	space.install_readwrite_handler( 0x55000000, 0x55000013, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_iis_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_iis_w)));
	space.install_readwrite_handler( 0x56000000, 0x560000bf, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_gpio_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_gpio_w)));
	space.install_readwrite_handler( 0x57000040, 0x5700008b, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_rtc_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_rtc_w)));
	space.install_readwrite_handler( 0x58000000, 0x58000013, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_adc_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_adc_w)));
	space.install_readwrite_handler( 0x59000000, 0x59000017, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_spi_0_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_spi_0_w)));
	space.install_readwrite_handler( 0x59000020, 0x59000037, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_spi_1_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_spi_1_w)));
	space.install_readwrite_handler( 0x5a000000, 0x5a000043, read32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_sdi_r)), write32s_delegate(*this, FUNC(s3c2410_device::s3c24xx_sdi_w)));

	s3c24xx_video_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c2410_device::device_reset()
{
	s3c24xx_device_reset();
}

void s3c2410_device::s3c2410_uart_fifo_w( int uart, uint8_t data)
{
	s3c24xx_uart_fifo_w( uart, data);
}

void s3c2410_device::s3c2410_touch_screen( int state)
{
	s3c24xx_touch_screen(state);
}

void s3c2410_device::frnb_w(int state)
{
	s3c24xx_pin_frnb_w(state);
}

void s3c2410_device::s3c2410_nand_calculate_mecc( uint8_t *data, uint32_t size, uint8_t *mecc)
{
	mecc[0] = mecc[1] = mecc[2] = mecc[3] = 0xFF;
	for (int i = 0; i < size; i++) nand_update_mecc( mecc, i, data[i]);
}

void s3c2410_device::s3c2410_request_eint(uint32_t number)
{
	s3c24xx_request_eint(number);
}
