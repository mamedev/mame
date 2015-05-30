// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C2400

*******************************************************************************/

#ifndef __S3C2400_H__
#define __S3C2400_H__


/*******************************************************************************
    MACROS / CONSTANTS
*******************************************************************************/

#define S3C2400_TAG "s3c2400"

#define MCFG_S3C2400_PALETTE(_palette_tag) \
	s3c2400_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_S3C2400_CORE_PIN_R_CB(_devcb) \
	devcb = &s3c2400_device::set_core_pin_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_CORE_PIN_W_CB(_devcb) \
	devcb = &s3c2400_device::set_core_pin_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_GPIO_PORT_R_CB(_devcb) \
	devcb = &s3c2400_device::set_gpio_port_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_GPIO_PORT_W_CB(_devcb) \
	devcb = &s3c2400_device::set_gpio_port_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_I2C_SCL_W_CB(_devcb) \
	devcb = &s3c2400_device::set_i2c_scl_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_I2C_SDA_R_CB(_devcb) \
	devcb = &s3c2400_device::set_i2c_sda_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_I2C_SDA_W_CB(_devcb) \
	devcb = &s3c2400_device::set_i2c_sda_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_ADC_DATA_R_CB(_devcb) \
	devcb = &s3c2400_device::set_adc_data_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_I2S_DATA_W_CB(_devcb) \
	devcb = &s3c2400_device::set_i2s_data_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C2400_LCD_FLAGS(_flags) \
	s3c2400_device::set_lcd_flags(*device, _flags);

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

#define S3C24XX_SRCPND    (0x00 / 4) // Interrupt Request Status
#define S3C24XX_INTMOD    (0x04 / 4) // Interrupt Mode Control
#define S3C24XX_INTMSK    (0x08 / 4) // Interrupt Mask Control
#define S3C24XX_PRIORITY  (0x0C / 4) // IRQ Priority Control
#define S3C24XX_INTPND    (0x10 / 4) // Interrupt Request Status
#define S3C24XX_INTOFFSET (0x14 / 4) // Interrupt Request Source Offset

/* DMA */

#define S3C24XX_BASE_DMA_0 0x14600000
#define S3C24XX_BASE_DMA_1 0x14600020
#define S3C24XX_BASE_DMA_2 0x14600040
#define S3C24XX_BASE_DMA_3 0x14600060

#define S3C24XX_DISRC     (0x00 / 4) // DMA Initial Source
#define S3C24XX_DIDST     (0x04 / 4) // DMA Initial Destination
#define S3C24XX_DCON      (0x08 / 4) // DMA Control
#define S3C24XX_DSTAT     (0x0C / 4) // DMA Count
#define S3C24XX_DCSRC     (0x10 / 4) // DMA Current Source Address
#define S3C24XX_DCDST     (0x14 / 4) // DMA Current Destination Address
#define S3C24XX_DMASKTRIG (0x18 / 4) // DMA Mask Trigger

/* Clock & Power Management */

#define S3C24XX_BASE_CLKPOW 0x14800000

#define S3C24XX_LOCKTIME (0x00 / 4) // PLL Lock Time Counter
#define S3C24XX_MPLLCON  (0x04 / 4) // MPLL Control
#define S3C24XX_UPLLCON  (0x08 / 4) // UPLL Control
#define S3C24XX_CLKCON   (0x0C / 4) // Clock Generator Control
#define S3C24XX_CLKSLOW  (0x10 / 4) // Slow Clock Control
#define S3C24XX_CLKDIVN  (0x14 / 4) // Clock Divider Control

/* LCD Controller */

#define S3C24XX_BASE_LCD    0x14a00000
#define S3C24XX_BASE_LCDPAL 0x14a00400

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

/* UART */

#define S3C24XX_BASE_UART_0 0x15000000
#define S3C24XX_BASE_UART_1 0x15004000

#define S3C24XX_ULCON   (0x00 / 4) // UART Line Control
#define S3C24XX_UCON    (0x04 / 4) // UART Control
#define S3C24XX_UFCON   (0x08 / 4) // UART FIFO Control
#define S3C24XX_UMCON   (0x0C / 4) // UART Modem Control
#define S3C24XX_UTRSTAT (0x10 / 4) // UART Tx/Rx Status
#define S3C24XX_UERSTAT (0x14 / 4) // UART Rx Error Status
#define S3C24XX_UFSTAT  (0x18 / 4) // UART FIFO Status
#define S3C24XX_UMSTAT  (0x1C / 4) // UART Modem Status
#define S3C24XX_UTXH    (0x20 / 4) // UART Transmission Hold
#define S3C24XX_URXH    (0x24 / 4) // UART Receive Buffer
#define S3C24XX_UBRDIV  (0x28 / 4) // UART Baud Rate Divisor

/* PWM Timer */

#define S3C24XX_BASE_PWM 0x15100000

#define S3C24XX_TCFG0  (0x00 / 4) // Timer Configuration
#define S3C24XX_TCFG1  (0x04 / 4) // Timer Configuration
#define S3C24XX_TCON   (0x08 / 4) // Timer Control
#define S3C24XX_TCNTB0 (0x0C / 4) // Timer Count Buffer 0
#define S3C24XX_TCMPB0 (0x10 / 4) // Timer Compare Buffer 0
#define S3C24XX_TCNTO0 (0x14 / 4) // Timer Count Observation 0
#define S3C24XX_TCNTB1 (0x18 / 4) // Timer Count Buffer 1
#define S3C24XX_TCMPB1 (0x1C / 4) // Timer Compare Buffer 1
#define S3C24XX_TCNTO1 (0x20 / 4) // Timer Count Observation 1
#define S3C24XX_TCNTB2 (0x24 / 4) // Timer Count Buffer 2
#define S3C24XX_TCMPB2 (0x28 / 4) // Timer Compare Buffer 2
#define S3C24XX_TCNTO2 (0x2C / 4) // Timer Count Observation 2
#define S3C24XX_TCNTB3 (0x30 / 4) // Timer Count Buffer 3
#define S3C24XX_TCMPB3 (0x34 / 4) // Timer Compare Buffer 3
#define S3C24XX_TCNTO3 (0x38 / 4) // Timer Count Observation 3
#define S3C24XX_TCNTB4 (0x3C / 4) // Timer Count Buffer 4
#define S3C24XX_TCNTO4 (0x40 / 4) // Timer Count Observation 4

/* USB Device */

#define S3C24XX_BASE_USBDEV 0x15200140

/* Watchdog Timer */

#define S3C24XX_BASE_WDT 0x15300000

#define S3C24XX_WTCON (0x00 / 4) // Watchdog Timer Mode
#define S3C24XX_WTDAT (0x04 / 4) // Watchdog Timer Data
#define S3C24XX_WTCNT (0x08 / 4) // Watchdog Timer Count

/* IIC */

#define S3C24XX_BASE_IIC 0x15400000

#define S3C24XX_IICCON  (0x00 / 4) // IIC Control
#define S3C24XX_IICSTAT (0x04 / 4) // IIC Status
#define S3C24XX_IICADD  (0x08 / 4) // IIC Address
#define S3C24XX_IICDS   (0x0C / 4) // IIC Data Shift

/* IIS */

#define S3C24XX_BASE_IIS 0x15508000

#define S3C24XX_IISCON  (0x00 / 4) // IIS Control
#define S3C24XX_IISMOD  (0x04 / 4) // IIS Mode
#define S3C24XX_IISPSR  (0x08 / 4) // IIS Prescaler
#define S3C24XX_IISFCON (0x0C / 4) // IIS FIFO Control
#define S3C24XX_IISFIFO (0x10 / 4) // IIS FIFO Entry

/* I/O Port */

#define S3C24XX_BASE_GPIO 0x15600000

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

/* RTC */

#define S3C24XX_BASE_RTC 0x15700040

#define S3C24XX_RTCCON  (0x00 / 4) // RTC Control
#define S3C24XX_TICNT   (0x04 / 4) // Tick Time count
#define S3C24XX_RTCALM  (0x10 / 4) // RTC Alarm Control
#define S3C24XX_ALMSEC  (0x14 / 4) // Alarm Second
#define S3C24XX_ALMMIN  (0x18 / 4) // Alarm Minute
#define S3C24XX_ALMHOUR (0x1C / 4) // Alarm Hour
#define S3C24XX_ALMDAY  (0x20 / 4) // Alarm Day
#define S3C24XX_ALMMON  (0x24 / 4) // Alarm Month
#define S3C24XX_ALMYEAR (0x28 / 4) // Alarm Year
#define S3C24XX_RTCRST  (0x2C / 4) // RTC Round Reset
#define S3C24XX_BCDSEC  (0x30 / 4) // BCD Second
#define S3C24XX_BCDMIN  (0x34 / 4) // BCD Minute
#define S3C24XX_BCDHOUR (0x38 / 4) // BCD Hour
#define S3C24XX_BCDDAY  (0x3C / 4) // BCD Day
#define S3C24XX_BCDDOW  (0x40 / 4) // BCD Day of Week
#define S3C24XX_BCDMON  (0x44 / 4) // BCD Month
#define S3C24XX_BCDYEAR (0x48 / 4) // BCD Year

/* A/D Converter */

#define S3C24XX_BASE_ADC 0x15800000

#define S3C24XX_ADCCON  (0x00 / 4) // ADC Control
#define S3C24XX_ADCDAT  (0x04 / 4) // ADC Data

/* SPI */

#define S3C24XX_BASE_SPI_0 0x15900000

#define S3C24XX_SPCON  (0x00 / 4) // SPI Control
#define S3C24XX_SPSTA  (0x04 / 4) // SPI Status
#define S3C24XX_SPPIN  (0x08 / 4) // SPI Pin Control
#define S3C24XX_SPPRE  (0x0C / 4) // SPI Baud Rate Prescaler
#define S3C24XX_SPTDAT (0x10 / 4) // SPI Tx Data
#define S3C24XX_SPRDAT (0x14 / 4) // SPI Rx Data

/* MMC Interface */

#define S3C24XX_BASE_MMC 0x15a00000

/* ... */

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

#define S3C24XX_UART_COUNT  2
#define S3C24XX_DMA_COUNT   4
#define S3C24XX_SPI_COUNT   1

class s3c2400_device : public device_t
{
public:
	s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s3c2400_device();

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);
	template<class _Object> static devcb_base &set_core_pin_r_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_pin_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_core_pin_w_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_pin_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_gpio_port_r_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_port_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_gpio_port_w_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_port_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2c_scl_w_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_scl_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2c_sda_r_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_sda_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2c_sda_w_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_sda_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_adc_data_r_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_data_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2s_data_w_callback(device_t &device, _Object object) { return downcast<s3c2400_device &>(device).m_data_w_cb.set_callback(object); }
	static void set_lcd_flags(device_t &device, int flags) { downcast<s3c2400_device &>(device).m_flags = flags; }

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	required_device<palette_device> m_palette;
public:
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void s3c24xx_reset();
	inline int iface_core_pin_r(int pin);
	void s3c24xx_lcd_reset();
	rgb_t s3c24xx_get_color_tft_16(UINT16 data);
	rgb_t s3c24xx_get_color_stn_12(UINT16 data);
	rgb_t s3c24xx_get_color_stn_08( UINT8 data);
	rgb_t s3c24xx_get_color_stn_01(UINT8 data);
	rgb_t s3c24xx_get_color_stn_02(UINT8 data);
	rgb_t s3c24xx_get_color_stn_04(UINT8 data);
	rgb_t s3c24xx_get_color_tpal();
	void s3c24xx_lcd_dma_reload();
	void s3c24xx_lcd_dma_init();
	UINT32 s3c24xx_lcd_dma_read();
	UINT32 s3c24xx_lcd_dma_read_bits(int count);
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
	UINT32 s3c24xx_video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	READ32_MEMBER( s3c24xx_lcd_r );
	int s3c24xx_lcd_configure_tft();
	int s3c24xx_lcd_configure_stn();
	int s3c24xx_lcd_configure();
	void s3c24xx_lcd_start();
	void s3c24xx_lcd_stop();
	void s3c24xx_lcd_recalc();
	WRITE32_MEMBER( s3c24xx_lcd_w );
	READ32_MEMBER( s3c24xx_lcd_palette_r );
	WRITE32_MEMBER( s3c24xx_lcd_palette_w );
	void s3c24xx_clkpow_reset();
	UINT32 s3c24xx_get_fclk();
	UINT32 s3c24xx_get_hclk();
	UINT32 s3c24xx_get_pclk();
	READ32_MEMBER( s3c24xx_clkpow_r );
	WRITE32_MEMBER( s3c24xx_clkpow_w );
	void s3c24xx_irq_reset();
	void s3c24xx_check_pending_irq();
	void s3c24xx_request_irq(UINT32 int_type);
	READ32_MEMBER( s3c24xx_irq_r );
	WRITE32_MEMBER( s3c24xx_irq_w );
	void s3c24xx_pwm_reset();
	UINT16 s3c24xx_pwm_calc_observation(int ch);
	READ32_MEMBER( s3c24xx_pwm_r );
	void s3c24xx_pwm_start(int timer);
	void s3c24xx_pwm_stop(int timer);
	void s3c24xx_pwm_recalc(int timer);
	WRITE32_MEMBER( s3c24xx_pwm_w );
	TIMER_CALLBACK_MEMBER( s3c24xx_pwm_timer_exp );
	void s3c24xx_dma_reset();
	void s3c24xx_dma_reload(int ch);
	void s3c24xx_dma_trigger(int ch);
	void s3c24xx_dma_request_iis();
	void s3c24xx_dma_request_pwm();
	void s3c24xx_dma_start(int ch);
	void s3c24xx_dma_stop(int ch);
	void s3c24xx_dma_recalc(int ch);
	UINT32 s3c24xx_dma_r(UINT32 ch, UINT32 offset);
	void s3c24xx_dma_w(UINT32 ch, UINT32 offset, UINT32 data, UINT32 mem_mask);
	READ32_MEMBER( s3c24xx_dma_0_r );
	READ32_MEMBER( s3c24xx_dma_1_r );
	READ32_MEMBER( s3c24xx_dma_2_r );
	READ32_MEMBER( s3c24xx_dma_3_r );
	WRITE32_MEMBER( s3c24xx_dma_0_w );
	WRITE32_MEMBER( s3c24xx_dma_1_w );
	WRITE32_MEMBER( s3c24xx_dma_2_w );
	WRITE32_MEMBER( s3c24xx_dma_3_w );
	TIMER_CALLBACK_MEMBER( s3c24xx_dma_timer_exp );
	void s3c24xx_gpio_reset();
	inline UINT32 iface_gpio_port_r(int port, UINT32 mask);
	inline void iface_gpio_port_w(int port, UINT32 mask, UINT32 data);
	UINT16 s3c24xx_gpio_get_mask( UINT32 con, int val);
	READ32_MEMBER( s3c24xx_gpio_r );
	WRITE32_MEMBER( s3c24xx_gpio_w );
	void s3c24xx_memcon_reset();
	READ32_MEMBER( s3c24xx_memcon_r );
	WRITE32_MEMBER( s3c24xx_memcon_w );
	void s3c24xx_usb_host_reset();
	READ32_MEMBER( s3c24xx_usb_host_r );
	WRITE32_MEMBER( s3c24xx_usb_host_w );
	void s3c24xx_uart_reset();
	UINT32 s3c24xx_uart_r(UINT32 ch, UINT32 offset);
	void s3c24xx_uart_w(UINT32 ch, UINT32 offset, UINT32 data, UINT32 mem_mask);
	READ32_MEMBER( s3c24xx_uart_0_r );
	READ32_MEMBER( s3c24xx_uart_1_r );
	WRITE32_MEMBER( s3c24xx_uart_0_w );
	WRITE32_MEMBER( s3c24xx_uart_1_w );
	void s3c24xx_uart_fifo_w(int uart, UINT8 data);
	void s3c24xx_usb_device_reset();
	READ32_MEMBER( s3c24xx_usb_device_r );
	WRITE32_MEMBER( s3c24xx_usb_device_w );
	void s3c24xx_wdt_reset();
	UINT16 s3c24xx_wdt_calc_current_count();
	READ32_MEMBER( s3c24xx_wdt_r );
	void s3c24xx_wdt_start();
	void s3c24xx_wdt_stop();
	void s3c24xx_wdt_recalc();
	WRITE32_MEMBER( s3c24xx_wdt_w );
	TIMER_CALLBACK_MEMBER( s3c24xx_wdt_timer_exp );
	void s3c24xx_iic_reset();
	inline void iface_i2c_scl_w( int state);
	inline void iface_i2c_sda_w(int state);
	inline int iface_i2c_sda_r();
	void i2c_send_start();
	void i2c_send_stop();
	UINT8 i2c_receive_byte(int ack);
	int i2c_send_byte(UINT8 data);
	void iic_start();
	void iic_stop();
	void iic_resume();
	READ32_MEMBER( s3c24xx_iic_r );
	WRITE32_MEMBER( s3c24xx_iic_w );
	TIMER_CALLBACK_MEMBER( s3c24xx_iic_timer_exp );
	void s3c24xx_iis_reset();
	inline void iface_i2s_data_w(int ch, UINT16 data);
	void s3c24xx_iis_start();
	void s3c24xx_iis_stop();
	void s3c24xx_iis_recalc();
	READ32_MEMBER( s3c24xx_iis_r );
	WRITE32_MEMBER( s3c24xx_iis_w );
	TIMER_CALLBACK_MEMBER( s3c24xx_iis_timer_exp );
	void s3c24xx_rtc_reset();
	READ32_MEMBER( s3c24xx_rtc_r );
	void s3c24xx_rtc_recalc();
	WRITE32_MEMBER( s3c24xx_rtc_w );
	TIMER_CALLBACK_MEMBER( s3c24xx_rtc_timer_tick_count_exp );
	void s3c24xx_rtc_update();
	void s3c24xx_rtc_check_alarm();
	TIMER_CALLBACK_MEMBER( s3c24xx_rtc_timer_update_exp );
	void s3c24xx_adc_reset();
	UINT32 iface_adc_data_r(int ch);
	READ32_MEMBER( s3c24xx_adc_r );
	void s3c24xx_adc_start();
	WRITE32_MEMBER( s3c24xx_adc_w );
	void s3c24xx_spi_reset();
	UINT32 s3c24xx_spi_r(UINT32 ch, UINT32 offset);
	void s3c24xx_spi_w(UINT32 ch, UINT32 offset, UINT32 data, UINT32 mem_mask);
	READ32_MEMBER( s3c24xx_spi_0_r );
	WRITE32_MEMBER( s3c24xx_spi_0_w );
	void s3c24xx_mmc_reset();
	READ32_MEMBER( s3c24xx_mmc_r );
	WRITE32_MEMBER( s3c24xx_mmc_w );
	void s3c24xx_device_reset();
	void s3c24xx_device_start();


	void s3c2400_uart_fifo_w(int uart, UINT8 data);

	/*******************************************************************************
	    TYPE DEFINITIONS
	*******************************************************************************/

	struct s3c24xx_memcon_regs_t
	{
		UINT32 data[0x34/4];
	};

	struct s3c24xx_usbhost_regs_t
	{
		UINT32 data[0x5C/4];
	};

	struct s3c24xx_irq_regs_t
	{
		UINT32 srcpnd;
		UINT32 intmod;
		UINT32 intmsk;
		UINT32 priority;
		UINT32 intpnd;
		UINT32 intoffset;
	};

	struct s3c24xx_dma_regs_t
	{
		UINT32 disrc;
		UINT32 didst;
		UINT32 dcon;
		UINT32 dstat;
		UINT32 dcsrc;
		UINT32 dcdst;
		UINT32 dmasktrig;
	};

	struct s3c24xx_clkpow_regs_t
	{
		UINT32 locktime;
		UINT32 mpllcon;
		UINT32 upllcon;
		UINT32 clkcon;
		UINT32 clkslow;
		UINT32 clkdivn;
	};

	struct s3c24xx_lcd_regs_t
	{
		UINT32 lcdcon1;
		UINT32 lcdcon2;
		UINT32 lcdcon3;
		UINT32 lcdcon4;
		UINT32 lcdcon5;
		UINT32 lcdsaddr1;
		UINT32 lcdsaddr2;
		UINT32 lcdsaddr3;
		UINT32 redlut;
		UINT32 greenlut;
		UINT32 bluelut;
		UINT32 reserved[8];
		UINT32 dithmode;
		UINT32 tpal;
	};

	struct s3c24xx_lcdpal_regs_t
	{
		UINT32 data[0x400/4];
	};

	struct s3c24xx_uart_regs_t
	{
		UINT32 ulcon;
		UINT32 ucon;
		UINT32 ufcon;
		UINT32 umcon;
		UINT32 utrstat;
		UINT32 uerstat;
		UINT32 ufstat;
		UINT32 umstat;
		UINT32 utxh;
		UINT32 urxh;
		UINT32 ubrdiv;
	};

	struct s3c24xx_pwm_regs_t
	{
		UINT32 tcfg0;
		UINT32 tcfg1;
		UINT32 tcon;
		UINT32 tcntb0;
		UINT32 tcmpb0;
		UINT32 tcnto0;
		UINT32 tcntb1;
		UINT32 tcmpb1;
		UINT32 tcnto1;
		UINT32 tcntb2;
		UINT32 tcmpb2;
		UINT32 tcnto2;
		UINT32 tcntb3;
		UINT32 tcmpb3;
		UINT32 tcnto3;
		UINT32 tcntb4;
		UINT32 tcnto4;
	};

	struct s3c24xx_usbdev_regs_t
	{
		UINT32 data[0xBC/4];
	};

	struct s3c24xx_wdt_regs_t
	{
		UINT32 wtcon;
		UINT32 wtdat;
		UINT32 wtcnt;
	};

	struct s3c24xx_iic_regs_t
	{
		UINT32 iiccon;
		UINT32 iicstat;
		UINT32 iicadd;
		UINT32 iicds;
	};

	struct s3c24xx_iis_regs_t
	{
		UINT32 iiscon;
		UINT32 iismod;
		UINT32 iispsr;
		UINT32 iisfcon;
		UINT32 iisfifo;
	};

	struct s3c24xx_gpio_regs_t
	{
		UINT32 gpacon;
		UINT32 gpadat;
		UINT32 gpbcon;
		UINT32 gpbdat;
		UINT32 gpbup;
		UINT32 gpccon;
		UINT32 gpcdat;
		UINT32 gpcup;
		UINT32 gpdcon;
		UINT32 gpddat;
		UINT32 gpdup;
		UINT32 gpecon;
		UINT32 gpedat;
		UINT32 gpeup;
		UINT32 gpfcon;
		UINT32 gpfdat;
		UINT32 gpfup;
		UINT32 gpgcon;
		UINT32 gpgdat;
		UINT32 gpgup;
		UINT32 opencr;
		UINT32 misccr;
		UINT32 extint;
	};

	struct s3c24xx_rtc_regs_t
	{
		UINT32 rtccon;
		UINT32 ticnt;
		UINT32 reserved[2];
		UINT32 rtcalm;
		UINT32 almsec;
		UINT32 almmin;
		UINT32 almhour;
		UINT32 almday;
		UINT32 almmon;
		UINT32 almyear;
		UINT32 rtcrst;
		UINT32 bcdsec;
		UINT32 bcdmin;
		UINT32 bcdhour;
		UINT32 bcdday;
		UINT32 bcddow;
		UINT32 bcdmon;
		UINT32 bcdyear;
	};

	struct s3c24xx_adc_regs_t
	{
		UINT32 adccon;
		UINT32 adcdat;
	};

	struct s3c24xx_spi_regs_t
	{
		UINT32 spcon;
		UINT32 spsta;
		UINT32 sppin;
		UINT32 sppre;
		UINT32 sptdat;
		UINT32 sprdat;
	};

	struct s3c24xx_mmc_regs_t
	{
		UINT32 data[0x40/4];
	};

	struct s3c24xx_memcon_t
	{
		s3c24xx_memcon_regs_t regs;
	};

	struct s3c24xx_usbhost_t
	{
		s3c24xx_usbhost_regs_t regs;
	};

	struct s3c24xx_irq_t
	{
		s3c24xx_irq_regs_t regs;
		int line_irq, line_fiq;
	};

	struct s3c24xx_dma_t
	{
		s3c24xx_dma_regs_t regs;
		emu_timer *timer;
	};

	struct s3c24xx_clkpow_t
	{
		s3c24xx_clkpow_regs_t regs;
	};

	struct s3c24xx_lcd_t
	{
		s3c24xx_lcd_regs_t regs;
		emu_timer *timer;
		bitmap_rgb32 *bitmap[2];
		UINT32 vramaddr_cur;
		UINT32 vramaddr_max;
		UINT32 offsize;
		UINT32 pagewidth_cur;
		UINT32 pagewidth_max;
		UINT32 bppmode;
		UINT32 bswp, hwswp;
		int vpos, hpos;
		double framerate;
		UINT32 tpal;
		UINT32 hpos_min, hpos_max, vpos_min, vpos_max;
		UINT32 dma_data, dma_bits;
	};

	struct s3c24xx_lcdpal_t
	{
		s3c24xx_lcdpal_regs_t regs;
	};

	struct s3c24xx_uart_t
	{
		s3c24xx_uart_regs_t regs;
	};

	struct s3c24xx_pwm_t
	{
		s3c24xx_pwm_regs_t regs;
		emu_timer *timer[5];
		UINT32 cnt[5];
		UINT32 cmp[5];
		UINT32 freq[5];
	};

	struct s3c24xx_usbdev_t
	{
		s3c24xx_usbdev_regs_t regs;
	};

	struct s3c24xx_wdt_t
	{
		s3c24xx_wdt_regs_t regs;
		emu_timer *timer;
	};

	struct s3c24xx_iic_t
	{
		s3c24xx_iic_regs_t regs;
		emu_timer *timer;
		int count;
	};

	struct s3c24xx_iis_t
	{
		s3c24xx_iis_regs_t regs;
		emu_timer *timer;
		UINT16 fifo[16/2];
		int fifo_index;
	};

	struct s3c24xx_gpio_t
	{
		s3c24xx_gpio_regs_t regs;
	};

	struct s3c24xx_rtc_t
	{
		s3c24xx_rtc_regs_t regs;
		emu_timer *timer_tick_count;
		emu_timer *timer_update;
	};

	struct s3c24xx_adc_t
	{
		s3c24xx_adc_regs_t regs;
	};

	struct s3c24xx_spi_t
	{
		s3c24xx_spi_regs_t regs;
	};

	struct s3c24xx_mmc_t
	{
		s3c24xx_mmc_regs_t regs;
	};


	s3c24xx_memcon_t m_memcon;
	s3c24xx_usbhost_t m_usbhost;
	s3c24xx_irq_t m_irq;
	s3c24xx_dma_t m_dma[S3C24XX_DMA_COUNT];
	s3c24xx_clkpow_t m_clkpow;
	s3c24xx_lcd_t m_lcd;
	s3c24xx_lcdpal_t m_lcdpal;
	s3c24xx_uart_t m_uart[S3C24XX_UART_COUNT];
	s3c24xx_pwm_t m_pwm;
	s3c24xx_usbdev_t m_usbdev;
	s3c24xx_wdt_t m_wdt;
	s3c24xx_iic_t m_iic;
	s3c24xx_iis_t m_iis;
	s3c24xx_gpio_t m_gpio;
	s3c24xx_rtc_t m_rtc;
	s3c24xx_adc_t m_adc;
	s3c24xx_spi_t m_spi[S3C24XX_SPI_COUNT];
	s3c24xx_mmc_t m_mmc;
	required_device<device_t> m_cpu;
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
};

extern const device_type S3C2400;


#endif
