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

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", cpuexec_describe_context( machine), buf);
	}
}

#define DEVICE_S3C2400
#include "machine/s3c24xx.c"
#undef DEVICE_S3C2400

VIDEO_START( s3c2400 )
{
	device_t *device = machine->device( S3C2400_TAG);
	s3c24xx_video_start( device, machine);
}

VIDEO_UPDATE( s3c2400 )
{
	device_t *device = screen->machine->device( S3C2400_TAG);
	return s3c24xx_video_update( device, screen, bitmap, cliprect);
}

DEVICE_START( s3c2400 )
{
	const address_space *space = cputag_get_address_space( device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	DEVICE_START_CALL(s3c24xx);
	memory_install_readwrite32_device_handler( space, device, 0x14000000, 0x1400003b, 0, 0, s3c24xx_memcon_r, s3c24xx_memcon_w);
	memory_install_readwrite32_device_handler( space, device, 0x14200000, 0x1420005b, 0, 0, s3c24xx_usb_host_r, s3c24xx_usb_host_w);
	memory_install_readwrite32_device_handler( space, device, 0x14400000, 0x14400017, 0, 0, s3c24xx_irq_r, s3c24xx_irq_w);
	memory_install_readwrite32_device_handler( space, device, 0x14600000, 0x1460001b, 0, 0, s3c24xx_dma_0_r, s3c24xx_dma_0_w);
	memory_install_readwrite32_device_handler( space, device, 0x14600020, 0x1460003b, 0, 0, s3c24xx_dma_1_r, s3c24xx_dma_1_w);
	memory_install_readwrite32_device_handler( space, device, 0x14600040, 0x1460005b, 0, 0, s3c24xx_dma_2_r, s3c24xx_dma_2_w);
	memory_install_readwrite32_device_handler( space, device, 0x14600060, 0x1460007b, 0, 0, s3c24xx_dma_3_r, s3c24xx_dma_3_w);
	memory_install_readwrite32_device_handler( space, device, 0x14800000, 0x14800017, 0, 0, s3c24xx_clkpow_r, s3c24xx_clkpow_w);
	memory_install_readwrite32_device_handler( space, device, 0x14a00000, 0x14a003ff, 0, 0, s3c2400_lcd_r, s3c24xx_lcd_w);
	memory_install_readwrite32_device_handler( space, device, 0x14a00400, 0x14a007ff, 0, 0, s3c24xx_lcd_palette_r, s3c24xx_lcd_palette_w);
	memory_install_readwrite32_device_handler( space, device, 0x15000000, 0x1500002b, 0, 0, s3c24xx_uart_0_r, s3c24xx_uart_0_w);
	memory_install_readwrite32_device_handler( space, device, 0x15004000, 0x1500402b, 0, 0, s3c24xx_uart_1_r, s3c24xx_uart_1_w);
	memory_install_readwrite32_device_handler( space, device, 0x15100000, 0x15100043, 0, 0, s3c24xx_pwm_r, s3c24xx_pwm_w);
	memory_install_readwrite32_device_handler( space, device, 0x15200140, 0x152001fb, 0, 0, s3c24xx_usb_device_r, s3c24xx_usb_device_w);
	memory_install_readwrite32_device_handler( space, device, 0x15300000, 0x1530000b, 0, 0, s3c24xx_wdt_r, s3c24xx_wdt_w);
	memory_install_readwrite32_device_handler( space, device, 0x15400000, 0x1540000f, 0, 0, s3c24xx_iic_r, s3c24xx_iic_w);
	memory_install_readwrite32_device_handler( space, device, 0x15508000, 0x15508013, 0, 0, s3c24xx_iis_r, s3c24xx_iis_w);
	memory_install_readwrite32_device_handler( space, device, 0x15600000, 0x1560005b, 0, 0, s3c24xx_gpio_r, s3c24xx_gpio_w);
	memory_install_readwrite32_device_handler( space, device, 0x15700040, 0x1570008b, 0, 0, s3c24xx_rtc_r, s3c24xx_rtc_w);
	memory_install_readwrite32_device_handler( space, device, 0x15800000, 0x15800007, 0, 0, s3c24xx_adc_r, s3c24xx_adc_w);
	memory_install_readwrite32_device_handler( space, device, 0x15900000, 0x15900017, 0, 0, s3c24xx_spi_0_r, s3c24xx_spi_0_w);
	memory_install_readwrite32_device_handler( space, device, 0x15a00000, 0x15a0003f, 0, 0, s3c24xx_mmc_r, s3c24xx_mmc_w);
}

DEVICE_GET_INFO( s3c2400 )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(s3c2400); break;
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME: strcpy(info->s, "Samsung S3C2400"); break;
		/* --- default --- */
		default: DEVICE_GET_INFO_CALL(s3c24xx); break;
    }
}

void s3c2400_uart_fifo_w( device_t *device, int uart, UINT8 data)
{
	s3c24xx_uart_fifo_w( device, uart, data);
}

DEFINE_LEGACY_DEVICE(S3C2400, s3c2400);
