// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C2400

*******************************************************************************/

#ifndef MAME_MACHINE_S3C2400_H
#define MAME_MACHINE_S3C2400_H

#pragma once

#include "s3c24xx.h"
#include "emupal.h"


enum
{
	S3C2400_GPIO_PORT_A = 0,
	S3C2400_GPIO_PORT_B,
	S3C2400_GPIO_PORT_C,
	S3C2400_GPIO_PORT_D,
	S3C2400_GPIO_PORT_E,
	S3C2400_GPIO_PORT_F,
	S3C2400_GPIO_PORT_G
};


/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

/* Interface */

#define S3C24XX_INTERFACE_LCD_REVERSE 1

/* Memory Controller */

#define S3C24XX_BASE_MEMCON 0x14000000

/* USB Host Controller */

#define S3C24XX_BASE_USBHOST 0x14200000

/* Interrupt Controller */

#define S3C24XX_BASE_INT 0x14400000

/* DMA */

#define S3C24XX_BASE_DMA_0 0x14600000
#define S3C24XX_BASE_DMA_1 0x14600020
#define S3C24XX_BASE_DMA_2 0x14600040
#define S3C24XX_BASE_DMA_3 0x14600060

/* Clock & Power Management */

#define S3C24XX_BASE_CLKPOW 0x14800000

/* LCD Controller */

#define S3C24XX_BASE_LCD    0x14a00000
#define S3C24XX_BASE_LCDPAL 0x14a00400

/* UART */

#define S3C24XX_BASE_UART_0 0x15000000
#define S3C24XX_BASE_UART_1 0x15004000

/* PWM Timer */

#define S3C24XX_BASE_PWM 0x15100000

/* USB Device */

#define S3C24XX_BASE_USBDEV 0x15200140

/* Watchdog Timer */

#define S3C24XX_BASE_WDT 0x15300000

/* IIC */

#define S3C24XX_BASE_IIC 0x15400000

/* IIS */

#define S3C24XX_BASE_IIS 0x15508000

/* I/O Port */

#define S3C24XX_BASE_GPIO 0x15600000

/* RTC */

#define S3C24XX_BASE_RTC 0x15700040

/* A/D Converter */

#define S3C24XX_BASE_ADC 0x15800000

#define S3C24XX_ADCCON  (0x00 / 4) // ADC Control
#define S3C24XX_ADCDAT  (0x04 / 4) // ADC Data

/* SPI */

#define S3C24XX_BASE_SPI_0 0x15900000

/* MMC Interface */

#define S3C24XX_BASE_MMC 0x15a00000

/* ... */

class s3c2400_device : public device_t, protected s3c24xx_peripheral_types
{
public:
	s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~s3c2400_device();

	// configuration
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	auto core_pin_r_callback() { return m_pin_r_cb.bind(); }
	auto core_pin_w_callback() { return m_pin_w_cb.bind(); }
	auto gpio_port_r_callback() { return m_port_r_cb.bind(); }
	auto gpio_port_w_callback() { return m_port_w_cb.bind(); }
	auto i2c_scl_w_callback() { return m_scl_w_cb.bind(); }
	auto i2c_sda_r_callback() { return m_sda_r_cb.bind(); }
	auto i2c_sda_w_callback() { return m_sda_w_cb.bind(); }
	auto adc_data_r_callback() { return m_data_r_cb.bind(); }
	auto i2s_data_w_callback() { return m_data_w_cb.bind(); }
	void set_lcd_flags(int flags) { m_flags = flags; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// FIXME: a bunch of this stuff needs to be public, or used from an internal memory map, but since nothing uses it yet, it's impossible to know what
	// However, the previous situation where all internal state was public is clearly undesirable
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void s3c24xx_reset();
	inline int iface_core_pin_r(int pin);
	void s3c24xx_lcd_reset();
	rgb_t s3c24xx_get_color_tft_16(uint16_t data);
	rgb_t s3c24xx_get_color_stn_12(uint16_t data);
	rgb_t s3c24xx_get_color_stn_08( uint8_t data);
	rgb_t s3c24xx_get_color_stn_01(uint8_t data);
	rgb_t s3c24xx_get_color_stn_02(uint8_t data);
	rgb_t s3c24xx_get_color_stn_04(uint8_t data);
	rgb_t s3c24xx_get_color_tpal();
	void s3c24xx_lcd_dma_reload();
	void s3c24xx_lcd_dma_init();
	uint32_t s3c24xx_lcd_dma_read();
	uint32_t s3c24xx_lcd_dma_read_bits(int count);
	void s3c24xx_lcd_render_tpal();
	void s3c24xx_lcd_render_stn_01();
	void s3c24xx_lcd_render_stn_02();
	void s3c24xx_lcd_render_stn_04();
	void s3c24xx_lcd_render_stn_08();
	void s3c24xx_lcd_render_stn_12_p();
	void s3c24xx_lcd_render_stn_12_u(); // not tested
	void s3c24xx_lcd_render_tft_01();
	void s3c24xx_lcd_render_tft_02();
	void s3c24xx_lcd_render_tft_04();
	void s3c24xx_lcd_render_tft_08();
	void s3c24xx_lcd_render_tft_16();
	TIMER_CALLBACK_MEMBER( s3c24xx_lcd_timer_exp );
	void s3c24xx_video_start();
	void bitmap_blend( bitmap_rgb32 &bitmap_dst, bitmap_rgb32 &bitmap_src_1, bitmap_rgb32 &bitmap_src_2);
	uint32_t s3c24xx_video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t s3c24xx_lcd_r(offs_t offset, uint32_t mem_mask = ~0);
	int s3c24xx_lcd_configure_tft();
	int s3c24xx_lcd_configure_stn();
	int s3c24xx_lcd_configure();
	void s3c24xx_lcd_start();
	void s3c24xx_lcd_stop();
	void s3c24xx_lcd_recalc();
	void s3c24xx_lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_lcd_palette_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_lcd_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_clkpow_reset();
	uint32_t s3c24xx_get_fclk();
	uint32_t s3c24xx_get_hclk();
	uint32_t s3c24xx_get_pclk();
	uint32_t s3c24xx_clkpow_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_clkpow_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_irq_reset();
	void s3c24xx_check_pending_irq();
	void s3c24xx_request_irq(uint32_t int_type);
	uint32_t s3c24xx_irq_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_irq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_pwm_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_pwm_start(int timer);
	void s3c24xx_pwm_stop(int timer);
	void s3c24xx_pwm_recalc(int timer);
	void s3c24xx_pwm_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( s3c24xx_pwm_timer_exp );
	void s3c24xx_dma_reset();
	void s3c24xx_dma_reload(int ch);
	void s3c24xx_dma_trigger(int ch);
	void s3c24xx_dma_request_iis();
	void s3c24xx_dma_request_pwm();
	void s3c24xx_dma_start(int ch);
	void s3c24xx_dma_stop(int ch);
	void s3c24xx_dma_recalc(int ch);
	uint32_t s3c24xx_dma_r(uint32_t ch, uint32_t offset);
	void s3c24xx_dma_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t s3c24xx_dma_0_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_dma_1_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_dma_2_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_dma_3_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_dma_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_dma_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_dma_2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_dma_3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( s3c24xx_dma_timer_exp );
	void s3c24xx_gpio_reset();
	inline uint32_t iface_gpio_port_r(int port, uint32_t mask);
	inline void iface_gpio_port_w(int port, uint32_t mask, uint32_t data);
	uint16_t s3c24xx_gpio_get_mask( uint32_t con, int val);
	uint32_t s3c24xx_gpio_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_memcon_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_memcon_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_usb_host_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_usb_host_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_uart_r(uint32_t ch, uint32_t offset);
	void s3c24xx_uart_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t s3c24xx_uart_0_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_uart_1_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_uart_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_uart_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_uart_fifo_w(int uart, uint8_t data);
	void s3c24xx_usb_device_reset();
	uint32_t s3c24xx_usb_device_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_usb_device_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_wdt_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_wdt_start();
	void s3c24xx_wdt_stop();
	void s3c24xx_wdt_recalc();
	void s3c24xx_wdt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( s3c24xx_wdt_timer_exp );
	void s3c24xx_iic_reset();
	inline void iface_i2c_scl_w( int state);
	inline void iface_i2c_sda_w(int state);
	inline int iface_i2c_sda_r();
	void i2c_send_start();
	void i2c_send_stop();
	uint8_t i2c_receive_byte(int ack);
	int i2c_send_byte(uint8_t data);
	void iic_start();
	void iic_stop();
	void iic_resume();
	uint32_t s3c24xx_iic_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_iic_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( s3c24xx_iic_timer_exp );
	inline void iface_i2s_data_w(int ch, uint16_t data);
	void s3c24xx_iis_start();
	void s3c24xx_iis_stop();
	void s3c24xx_iis_recalc();
	uint32_t s3c24xx_iis_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_iis_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( s3c24xx_iis_timer_exp );
	uint32_t s3c24xx_rtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( s3c24xx_rtc_timer_tick_count_exp );
	void s3c24xx_rtc_update();
	void s3c24xx_rtc_check_alarm();
	TIMER_CALLBACK_MEMBER( s3c24xx_rtc_timer_update_exp );
	void s3c24xx_adc_reset();
	uint32_t iface_adc_data_r(int ch);
	uint32_t s3c24xx_adc_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_adc_start();
	void s3c24xx_adc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_spi_reset();
	uint32_t s3c24xx_spi_r(uint32_t ch, uint32_t offset);
	void s3c24xx_spi_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t s3c24xx_spi_0_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_spi_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t s3c24xx_mmc_r(offs_t offset, uint32_t mem_mask = ~0);
	void s3c24xx_mmc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void s3c24xx_device_reset();
	void s3c24xx_device_start();

	void s3c2400_uart_fifo_w(int uart, uint8_t data);

private:
	static constexpr unsigned UART_COUNT  = 2;
	static constexpr unsigned DMA_COUNT   = 4;
	static constexpr unsigned SPI_COUNT   = 1;

	/*******************************************************************************
	    TYPE DEFINITIONS
	*******************************************************************************/

	struct irq_regs_t
	{
		uint32_t srcpnd;
		uint32_t intmod;
		uint32_t intmsk;
		uint32_t priority;
		uint32_t intpnd;
		uint32_t intoffset;
	};

	struct dma_regs_t
	{
		uint32_t disrc;
		uint32_t didst;
		uint32_t dcon;
		uint32_t dstat;
		uint32_t dcsrc;
		uint32_t dcdst;
		uint32_t dmasktrig;
	};

	struct clkpow_regs_t
	{
		uint32_t locktime;
		uint32_t mpllcon;
		uint32_t upllcon;
		uint32_t clkcon;
		uint32_t clkslow;
		uint32_t clkdivn;
	};

	struct lcd_regs_t
	{
		uint32_t lcdcon1;
		uint32_t lcdcon2;
		uint32_t lcdcon3;
		uint32_t lcdcon4;
		uint32_t lcdcon5;
		uint32_t lcdsaddr1;
		uint32_t lcdsaddr2;
		uint32_t lcdsaddr3;
		uint32_t redlut;
		uint32_t greenlut;
		uint32_t bluelut;
		uint32_t reserved[8];
		uint32_t dithmode;
		uint32_t tpal;
	};

	struct usbdev_regs_t
	{
		uint32_t data[0xBC/4];
	};

	struct iic_regs_t
	{
		uint32_t iiccon;
		uint32_t iicstat;
		uint32_t iicadd;
		uint32_t iicds;
	};

	struct gpio_regs_t
	{
		uint32_t gpacon;
		uint32_t gpadat;
		uint32_t gpbcon;
		uint32_t gpbdat;
		uint32_t gpbup;
		uint32_t gpccon;
		uint32_t gpcdat;
		uint32_t gpcup;
		uint32_t gpdcon;
		uint32_t gpddat;
		uint32_t gpdup;
		uint32_t gpecon;
		uint32_t gpedat;
		uint32_t gpeup;
		uint32_t gpfcon;
		uint32_t gpfdat;
		uint32_t gpfup;
		uint32_t gpgcon;
		uint32_t gpgdat;
		uint32_t gpgup;
		uint32_t opencr;
		uint32_t misccr;
		uint32_t extint;
	};

	struct adc_regs_t
	{
		uint32_t adccon;
		uint32_t adcdat;
	};

	struct irq_t
	{
		irq_regs_t regs;
		int line_irq, line_fiq;
	};

	struct dma_t
	{
		dma_regs_t regs;
		emu_timer *timer;
	};

	struct clkpow_t
	{
		clkpow_regs_t regs;
	};

	struct lcd_t
	{
		lcd_regs_t regs;
		emu_timer *timer;
		std::unique_ptr<bitmap_rgb32> bitmap[2];
		uint32_t vramaddr_cur;
		uint32_t vramaddr_max;
		uint32_t offsize;
		uint32_t pagewidth_cur;
		uint32_t pagewidth_max;
		uint32_t bppmode;
		uint32_t bswp, hwswp;
		int vpos, hpos;
		double framerate;
		uint32_t tpal;
		uint32_t hpos_min, hpos_max, vpos_min, vpos_max;
		uint32_t dma_data, dma_bits;
	};

	struct usbdev_t
	{
		usbdev_regs_t regs;
	};

	struct iic_t
	{
		iic_regs_t regs;
		emu_timer *timer;
		int count;
	};

	struct gpio_t
	{
		gpio_regs_t regs;
	};

	struct adc_t
	{
		adc_regs_t regs;
	};

	// internal state
	required_device<arm7_cpu_device> m_cpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	memory_access<24, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;

	memcon_t m_memcon;
	usbhost_t m_usbhost;
	irq_t m_irq;
	dma_t m_dma[DMA_COUNT];
	clkpow_t m_clkpow;
	lcd_t m_lcd;
	lcdpal_t m_lcdpal;
	uart_t m_uart[UART_COUNT];
	pwm_t m_pwm;
	usbdev_t m_usbdev;
	wdt_t m_wdt;
	iic_t m_iic;
	iis_t m_iis;
	gpio_t m_gpio;
	rtc_t m_rtc;
	adc_t m_adc;
	spi_t m_spi[SPI_COUNT];
	devcb_read32 m_pin_r_cb;
	devcb_write32 m_pin_w_cb;
	devcb_read32 m_port_r_cb;
	devcb_write32 m_port_w_cb;
	devcb_write_line m_scl_w_cb;
	devcb_read_line m_sda_r_cb;
	devcb_write_line m_sda_w_cb;
	devcb_read32 m_data_r_cb;
	devcb_write16 m_data_w_cb;
	int m_flags;

	mmc_t m_mmc;
};

DECLARE_DEVICE_TYPE(S3C2400, s3c2400_device)

#endif // MAME_MACHINE_S3C2400_H
