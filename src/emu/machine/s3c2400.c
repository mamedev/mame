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
#include "machine/s3c24xx.c"
#undef DEVICE_S3C2400

UINT32 s3c2400_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return s3c24xx_video_update( this, screen, bitmap, cliprect);
}

DEVICE_START( s3c2400 )
{
	address_space *space = device->machine().device( "maincpu")->memory().space( AS_PROGRAM);
	DEVICE_START_CALL(s3c24xx);
	space->install_legacy_readwrite_handler( *device, 0x14000000, 0x1400003b, FUNC(s3c24xx_memcon_r), FUNC(s3c24xx_memcon_w));
	space->install_legacy_readwrite_handler( *device, 0x14200000, 0x1420005b, FUNC(s3c24xx_usb_host_r), FUNC(s3c24xx_usb_host_w));
	space->install_legacy_readwrite_handler( *device, 0x14400000, 0x14400017, FUNC(s3c24xx_irq_r), FUNC(s3c24xx_irq_w));
	space->install_legacy_readwrite_handler( *device, 0x14600000, 0x1460001b, FUNC(s3c24xx_dma_0_r), FUNC(s3c24xx_dma_0_w));
	space->install_legacy_readwrite_handler( *device, 0x14600020, 0x1460003b, FUNC(s3c24xx_dma_1_r), FUNC(s3c24xx_dma_1_w));
	space->install_legacy_readwrite_handler( *device, 0x14600040, 0x1460005b, FUNC(s3c24xx_dma_2_r), FUNC(s3c24xx_dma_2_w));
	space->install_legacy_readwrite_handler( *device, 0x14600060, 0x1460007b, FUNC(s3c24xx_dma_3_r), FUNC(s3c24xx_dma_3_w));
	space->install_legacy_readwrite_handler( *device, 0x14800000, 0x14800017, FUNC(s3c24xx_clkpow_r), FUNC(s3c24xx_clkpow_w));
	space->install_legacy_readwrite_handler( *device, 0x14a00000, 0x14a003ff, FUNC(s3c2400_lcd_r), FUNC(s3c24xx_lcd_w));
	space->install_legacy_readwrite_handler( *device, 0x14a00400, 0x14a007ff, FUNC(s3c24xx_lcd_palette_r), FUNC(s3c24xx_lcd_palette_w));
	space->install_legacy_readwrite_handler( *device, 0x15000000, 0x1500002b, FUNC(s3c24xx_uart_0_r), FUNC(s3c24xx_uart_0_w));
	space->install_legacy_readwrite_handler( *device, 0x15004000, 0x1500402b, FUNC(s3c24xx_uart_1_r), FUNC(s3c24xx_uart_1_w));
	space->install_legacy_readwrite_handler( *device, 0x15100000, 0x15100043, FUNC(s3c24xx_pwm_r), FUNC(s3c24xx_pwm_w));
	space->install_legacy_readwrite_handler( *device, 0x15200140, 0x152001fb, FUNC(s3c24xx_usb_device_r), FUNC(s3c24xx_usb_device_w));
	space->install_legacy_readwrite_handler( *device, 0x15300000, 0x1530000b, FUNC(s3c24xx_wdt_r), FUNC(s3c24xx_wdt_w));
	space->install_legacy_readwrite_handler( *device, 0x15400000, 0x1540000f, FUNC(s3c24xx_iic_r), FUNC(s3c24xx_iic_w));
	space->install_legacy_readwrite_handler( *device, 0x15508000, 0x15508013, FUNC(s3c24xx_iis_r), FUNC(s3c24xx_iis_w));
	space->install_legacy_readwrite_handler( *device, 0x15600000, 0x1560005b, FUNC(s3c24xx_gpio_r), FUNC(s3c24xx_gpio_w));
	space->install_legacy_readwrite_handler( *device, 0x15700040, 0x1570008b, FUNC(s3c24xx_rtc_r), FUNC(s3c24xx_rtc_w));
	space->install_legacy_readwrite_handler( *device, 0x15800000, 0x15800007, FUNC(s3c24xx_adc_r), FUNC(s3c24xx_adc_w));
	space->install_legacy_readwrite_handler( *device, 0x15900000, 0x15900017, FUNC(s3c24xx_spi_0_r), FUNC(s3c24xx_spi_0_w));
	space->install_legacy_readwrite_handler( *device, 0x15a00000, 0x15a0003f, FUNC(s3c24xx_mmc_r), FUNC(s3c24xx_mmc_w));

	s3c24xx_video_start( device, device->machine());
}

const device_type S3C2400 = &device_creator<s3c2400_device>;

s3c2400_device::s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
       : device_t(mconfig, S3C2400, "Samsung S3C2400", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(s3c24xx_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void s3c2400_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c2400_device::device_start()
{
	DEVICE_START_NAME( s3c2400 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c2400_device::device_reset()
{
	DEVICE_RESET_NAME( s3c24xx )(this);
}

void s3c2400_uart_fifo_w( device_t *device, int uart, UINT8 data)
{
	s3c24xx_uart_fifo_w( device, uart, data);
}

