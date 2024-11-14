// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C2400

    (c) 2010 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "s3c2400.h"

#include "screen.h"


#define S3C24XX_SRCPND    (0x00 / 4) // Interrupt Request Status
#define S3C24XX_INTMOD    (0x04 / 4) // Interrupt Mode Control
#define S3C24XX_INTMSK    (0x08 / 4) // Interrupt Mask Control
#define S3C24XX_PRIORITY  (0x0C / 4) // IRQ Priority Control
#define S3C24XX_INTPND    (0x10 / 4) // Interrupt Request Status
#define S3C24XX_INTOFFSET (0x14 / 4) // Interrupt Request Source Offset

#define S3C24XX_DISRC     (0x00 / 4) // DMA Initial Source
#define S3C24XX_DIDST     (0x04 / 4) // DMA Initial Destination
#define S3C24XX_DCON      (0x08 / 4) // DMA Control
#define S3C24XX_DSTAT     (0x0C / 4) // DMA Count
#define S3C24XX_DCSRC     (0x10 / 4) // DMA Current Source Address
#define S3C24XX_DCDST     (0x14 / 4) // DMA Current Destination Address
#define S3C24XX_DMASKTRIG (0x18 / 4) // DMA Mask Trigger

#define S3C24XX_LOCKTIME (0x00 / 4) // PLL Lock Time Counter
#define S3C24XX_MPLLCON  (0x04 / 4) // MPLL Control
#define S3C24XX_UPLLCON  (0x08 / 4) // UPLL Control
#define S3C24XX_CLKCON   (0x0C / 4) // Clock Generator Control
#define S3C24XX_CLKSLOW  (0x10 / 4) // Slow Clock Control
#define S3C24XX_CLKDIVN  (0x14 / 4) // Clock Divider Control

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

#define S3C24XX_IICCON  (0x00 / 4) // IIC Control
#define S3C24XX_IICSTAT (0x04 / 4) // IIC Status
#define S3C24XX_IICADD  (0x08 / 4) // IIC Address
#define S3C24XX_IICDS   (0x0C / 4) // IIC Data Shift

#define S3C24XX_GPACON (0x00 / 4) // Port A Control
#define S3C24XX_GPADAT (0x04 / 4) // Port A Data
#define S3C24XX_GPBCON (0x08 / 4) // Port B Control
#define S3C24XX_GPBDAT (0x0C / 4) // Port B Data
#define S3C24XX_GPBUP  (0x10 / 4) // Pull-up Control B
#define S3C24XX_GPCCON (0x14 / 4) // Port C Control
#define S3C24XX_GPCDAT (0x18 / 4) // Port C Data
#define S3C24XX_GPCUP  (0x1C / 4) // Pull-up Control C
#define S3C24XX_GPDCON (0x20 / 4) // Port D Control
#define S3C24XX_GPDDAT (0x24 / 4) // Port D Data
#define S3C24XX_GPDUP  (0x28 / 4) // Pull-up Control D
#define S3C24XX_GPECON (0x2C / 4) // Port E Control
#define S3C24XX_GPEDAT (0x30 / 4) // Port E Data
#define S3C24XX_GPEUP  (0x34 / 4) // Pull-up Control E
#define S3C24XX_GPFCON (0x38 / 4) // Port F Control
#define S3C24XX_GPFDAT (0x3C / 4) // Port F Data
#define S3C24XX_GPFUP  (0x40 / 4) // Pull-up Control F
#define S3C24XX_GPGCON (0x44 / 4) // Port G Control
#define S3C24XX_GPGDAT (0x48 / 4) // Port G Data
#define S3C24XX_GPGUP  (0x4C / 4) // Pull-up Control G
#define S3C24XX_OPENCR (0x50 / 4) // Open Drain Enable
#define S3C24XX_MISCCR (0x54 / 4) // Miscellaneous Control
#define S3C24XX_EXTINT (0x58 / 4) // External Interrupt Control

#define S3C24XX_GPADAT_MASK 0x0003FFFF
#define S3C24XX_GPBDAT_MASK 0x0000FFFF
#define S3C24XX_GPCDAT_MASK 0x0000FFFF
#define S3C24XX_GPDDAT_MASK 0x000007FF
#define S3C24XX_GPEDAT_MASK 0x00000FFF
#define S3C24XX_GPFDAT_MASK 0x0000007F
#define S3C24XX_GPGDAT_MASK 0x000003FF

#define S3C24XX_INT_ADC       31
#define S3C24XX_INT_RTC       30
#define S3C24XX_INT_UTXD1     29
#define S3C24XX_INT_UTXD0     28
#define S3C24XX_INT_IIC       27
#define S3C24XX_INT_USBH      26
#define S3C24XX_INT_USBD      25
#define S3C24XX_INT_URXD1     24
#define S3C24XX_INT_URXD0     23
#define S3C24XX_INT_SPI       22
#define S3C24XX_INT_MMC       21
#define S3C24XX_INT_DMA3      20
#define S3C24XX_INT_DMA2      19
#define S3C24XX_INT_DMA1      18
#define S3C24XX_INT_DMA0      17
#define S3C24XX_INT_RESERVED  16
#define S3C24XX_INT_UERR      15
#define S3C24XX_INT_TIMER4    14
#define S3C24XX_INT_TIMER3    13
#define S3C24XX_INT_TIMER2    12
#define S3C24XX_INT_TIMER1    11
#define S3C24XX_INT_TIMER0    10
#define S3C24XX_INT_WDT        9
#define S3C24XX_INT_TICK       8
#define S3C24XX_INT_EINT7      7
#define S3C24XX_INT_EINT6      6
#define S3C24XX_INT_EINT5      5
#define S3C24XX_INT_EINT4      4
#define S3C24XX_INT_EINT3      3
#define S3C24XX_INT_EINT2      2
#define S3C24XX_INT_EINT1      1
#define S3C24XX_INT_EINT0      0

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

#define S3C24XX_GPIO_PORT_A S3C2400_GPIO_PORT_A
#define S3C24XX_GPIO_PORT_B S3C2400_GPIO_PORT_B
#define S3C24XX_GPIO_PORT_C S3C2400_GPIO_PORT_C
#define S3C24XX_GPIO_PORT_D S3C2400_GPIO_PORT_D
#define S3C24XX_GPIO_PORT_E S3C2400_GPIO_PORT_E
#define S3C24XX_GPIO_PORT_F S3C2400_GPIO_PORT_F
#define S3C24XX_GPIO_PORT_G S3C2400_GPIO_PORT_G

#define DEVICE_S3C2400
#define S3C24_CLASS_NAME s3c2400_device
#include "machine/s3c24xx.hxx"
#undef DEVICE_S3C2400

uint32_t s3c2400_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return s3c24xx_video_update(screen, bitmap, cliprect);
}

DEFINE_DEVICE_TYPE(S3C2400, s3c2400_device, "s3c2400", "Samsung S3C2400 SoC")

s3c2400_device::s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S3C2400, tag, owner, clock)
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
	, m_flags(0)
{
	memset(&m_memcon, 0, sizeof(m_memcon));
	memset(&m_usbhost, 0, sizeof(m_usbhost));
	memset(&m_irq, 0, sizeof(m_irq));
	memset(m_dma, 0, sizeof(m_dma));
	memset(&m_clkpow, 0, sizeof(m_clkpow));
	memset(&m_lcd, 0, sizeof(m_lcd));
	memset(&m_lcdpal, 0, sizeof(m_lcdpal));
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
	memset(&m_mmc, 0, sizeof(m_mmc));
}

s3c2400_device::~s3c2400_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c2400_device::device_start()
{
	s3c24xx_device_start();

	address_space &space = m_cpu->space(AS_PROGRAM);
	space.install_readwrite_handler(0x14000000, 0x1400003b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_memcon_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_memcon_w)));
	space.install_readwrite_handler(0x14200000, 0x1420005b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_usb_host_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_usb_host_w)));
	space.install_readwrite_handler(0x14400000, 0x14400017, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_irq_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_irq_w)));
	space.install_readwrite_handler(0x14600000, 0x1460001b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_0_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_0_w)));
	space.install_readwrite_handler(0x14600020, 0x1460003b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_1_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_1_w)));
	space.install_readwrite_handler(0x14600040, 0x1460005b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_2_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_2_w)));
	space.install_readwrite_handler(0x14600060, 0x1460007b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_3_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_dma_3_w)));
	space.install_readwrite_handler(0x14800000, 0x14800017, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_clkpow_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_clkpow_w)));
	space.install_readwrite_handler(0x14a00000, 0x14a003ff, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_lcd_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_lcd_w)));
	space.install_readwrite_handler(0x14a00400, 0x14a007ff, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_lcd_palette_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_lcd_palette_w)));
	space.install_readwrite_handler(0x15000000, 0x1500002b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_uart_0_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_uart_0_w)));
	space.install_readwrite_handler(0x15004000, 0x1500402b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_uart_1_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_uart_1_w)));
	space.install_readwrite_handler(0x15100000, 0x15100043, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_pwm_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_pwm_w)));
	space.install_readwrite_handler(0x15200140, 0x152001fb, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_usb_device_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_usb_device_w)));
	space.install_readwrite_handler(0x15300000, 0x1530000b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_wdt_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_wdt_w)));
	space.install_readwrite_handler(0x15400000, 0x1540000f, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_iic_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_iic_w)));
	space.install_readwrite_handler(0x15508000, 0x15508013, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_iis_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_iis_w)));
	space.install_readwrite_handler(0x15600000, 0x1560005b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_gpio_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_gpio_w)));
	space.install_readwrite_handler(0x15700040, 0x1570008b, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_rtc_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_rtc_w)));
	space.install_readwrite_handler(0x15800000, 0x15800007, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_adc_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_adc_w)));
	space.install_readwrite_handler(0x15900000, 0x15900017, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_spi_0_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_spi_0_w)));
	space.install_readwrite_handler(0x15a00000, 0x15a0003f, read32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_mmc_r)), write32s_delegate(*this, FUNC(s3c2400_device::s3c24xx_mmc_w)));

	s3c24xx_video_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c2400_device::device_reset()
{
	s3c24xx_device_reset();
}

void s3c2400_device::s3c2400_uart_fifo_w(int uart, uint8_t data)
{
	s3c24xx_uart_fifo_w(uart, data);
}
