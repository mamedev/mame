// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C2400

    (c) 2010 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/s3c2400.h"
#include "sound/dac.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

#define DEVICE_S3C2400
#define S3C24_CLASS_NAME s3c2400_device
#include "machine/s3c24xx.inc"
#undef DEVICE_S3C2400

UINT32 s3c2400_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return s3c24xx_video_update(screen, bitmap, cliprect);
}

const device_type S3C2400 = &device_creator<s3c2400_device>;

s3c2400_device::s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, S3C2400, "Samsung S3C2400", tag, owner, clock, "s3c2400", __FILE__),
		m_palette(*this),
		m_cpu(*this, ":maincpu"),
		m_pin_r_cb(*this),
		m_pin_w_cb(*this),
		m_port_r_cb(*this),
		m_port_w_cb(*this),
		m_scl_w_cb(*this),
		m_sda_r_cb(*this),
		m_sda_w_cb(*this),
		m_data_r_cb(*this),
		m_data_w_cb(*this),
		m_flags(0)
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
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void s3c2400_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<s3c2400_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c2400_device::device_start()
{
	s3c24xx_device_start();

	address_space &space = m_cpu->memory().space( AS_PROGRAM);
	space.install_readwrite_handler(0x14000000, 0x1400003b, read32_delegate(FUNC(s3c2400_device::s3c24xx_memcon_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_memcon_w), this));
	space.install_readwrite_handler(0x14200000, 0x1420005b, read32_delegate(FUNC(s3c2400_device::s3c24xx_usb_host_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_usb_host_w), this));
	space.install_readwrite_handler(0x14400000, 0x14400017, read32_delegate(FUNC(s3c2400_device::s3c24xx_irq_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_irq_w), this));
	space.install_readwrite_handler(0x14600000, 0x1460001b, read32_delegate(FUNC(s3c2400_device::s3c24xx_dma_0_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_dma_0_w), this));
	space.install_readwrite_handler(0x14600020, 0x1460003b, read32_delegate(FUNC(s3c2400_device::s3c24xx_dma_1_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_dma_1_w), this));
	space.install_readwrite_handler(0x14600040, 0x1460005b, read32_delegate(FUNC(s3c2400_device::s3c24xx_dma_2_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_dma_2_w), this));
	space.install_readwrite_handler(0x14600060, 0x1460007b, read32_delegate(FUNC(s3c2400_device::s3c24xx_dma_3_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_dma_3_w), this));
	space.install_readwrite_handler(0x14800000, 0x14800017, read32_delegate(FUNC(s3c2400_device::s3c24xx_clkpow_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_clkpow_w), this));
	space.install_readwrite_handler(0x14a00000, 0x14a003ff, read32_delegate(FUNC(s3c2400_device::s3c24xx_lcd_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_lcd_w), this));
	space.install_readwrite_handler(0x14a00400, 0x14a007ff, read32_delegate(FUNC(s3c2400_device::s3c24xx_lcd_palette_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_lcd_palette_w), this));
	space.install_readwrite_handler(0x15000000, 0x1500002b, read32_delegate(FUNC(s3c2400_device::s3c24xx_uart_0_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_uart_0_w), this));
	space.install_readwrite_handler(0x15004000, 0x1500402b, read32_delegate(FUNC(s3c2400_device::s3c24xx_uart_1_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_uart_1_w), this));
	space.install_readwrite_handler(0x15100000, 0x15100043, read32_delegate(FUNC(s3c2400_device::s3c24xx_pwm_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_pwm_w), this));
	space.install_readwrite_handler(0x15200140, 0x152001fb, read32_delegate(FUNC(s3c2400_device::s3c24xx_usb_device_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_usb_device_w), this));
	space.install_readwrite_handler(0x15300000, 0x1530000b, read32_delegate(FUNC(s3c2400_device::s3c24xx_wdt_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_wdt_w), this));
	space.install_readwrite_handler(0x15400000, 0x1540000f, read32_delegate(FUNC(s3c2400_device::s3c24xx_iic_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_iic_w), this));
	space.install_readwrite_handler(0x15508000, 0x15508013, read32_delegate(FUNC(s3c2400_device::s3c24xx_iis_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_iis_w), this));
	space.install_readwrite_handler(0x15600000, 0x1560005b, read32_delegate(FUNC(s3c2400_device::s3c24xx_gpio_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_gpio_w), this));
	space.install_readwrite_handler(0x15700040, 0x1570008b, read32_delegate(FUNC(s3c2400_device::s3c24xx_rtc_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_rtc_w), this));
	space.install_readwrite_handler(0x15800000, 0x15800007, read32_delegate(FUNC(s3c2400_device::s3c24xx_adc_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_adc_w), this));
	space.install_readwrite_handler(0x15900000, 0x15900017, read32_delegate(FUNC(s3c2400_device::s3c24xx_spi_0_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_spi_0_w), this));
	space.install_readwrite_handler(0x15a00000, 0x15a0003f, read32_delegate(FUNC(s3c2400_device::s3c24xx_mmc_r), this), write32_delegate(FUNC(s3c2400_device::s3c24xx_mmc_w), this));

	s3c24xx_video_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c2400_device::device_reset()
{
	s3c24xx_device_reset();
}

void s3c2400_device::s3c2400_uart_fifo_w(int uart, UINT8 data)
{
	s3c24xx_uart_fifo_w(uart, data);
}
