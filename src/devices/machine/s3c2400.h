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
	s3c2400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	required_device<palette_device> m_palette;
public:
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
	void s3c24xx_lcd_timer_exp(void *ptr, int32_t param);
	void s3c24xx_video_start();
	void bitmap_blend( bitmap_rgb32 &bitmap_dst, bitmap_rgb32 &bitmap_src_1, bitmap_rgb32 &bitmap_src_2);
	uint32_t s3c24xx_video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t s3c24xx_lcd_r(address_space &space, offs_t offset, uint32_t mem_mask);
	int s3c24xx_lcd_configure_tft();
	int s3c24xx_lcd_configure_stn();
	int s3c24xx_lcd_configure();
	void s3c24xx_lcd_start();
	void s3c24xx_lcd_stop();
	void s3c24xx_lcd_recalc();
	void s3c24xx_lcd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t s3c24xx_lcd_palette_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_lcd_palette_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_clkpow_reset();
	uint32_t s3c24xx_get_fclk();
	uint32_t s3c24xx_get_hclk();
	uint32_t s3c24xx_get_pclk();
	uint32_t s3c24xx_clkpow_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_clkpow_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_irq_reset();
	void s3c24xx_check_pending_irq();
	void s3c24xx_request_irq(uint32_t int_type);
	uint32_t s3c24xx_irq_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_irq_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_pwm_reset();
	uint16_t s3c24xx_pwm_calc_observation(int ch);
	uint32_t s3c24xx_pwm_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_pwm_start(int timer);
	void s3c24xx_pwm_stop(int timer);
	void s3c24xx_pwm_recalc(int timer);
	void s3c24xx_pwm_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_pwm_timer_exp(void *ptr, int32_t param);
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
	uint32_t s3c24xx_dma_0_r(address_space &space, offs_t offset, uint32_t mem_mask);
	uint32_t s3c24xx_dma_1_r(address_space &space, offs_t offset, uint32_t mem_mask);
	uint32_t s3c24xx_dma_2_r(address_space &space, offs_t offset, uint32_t mem_mask);
	uint32_t s3c24xx_dma_3_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_dma_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_dma_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_dma_2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_dma_3_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_dma_timer_exp(void *ptr, int32_t param);
	void s3c24xx_gpio_reset();
	inline uint32_t iface_gpio_port_r(int port, uint32_t mask);
	inline void iface_gpio_port_w(int port, uint32_t mask, uint32_t data);
	uint16_t s3c24xx_gpio_get_mask( uint32_t con, int val);
	uint32_t s3c24xx_gpio_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_gpio_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_memcon_reset();
	uint32_t s3c24xx_memcon_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_memcon_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_usb_host_reset();
	uint32_t s3c24xx_usb_host_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_usb_host_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_uart_reset();
	uint32_t s3c24xx_uart_r(uint32_t ch, uint32_t offset);
	void s3c24xx_uart_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t s3c24xx_uart_0_r(address_space &space, offs_t offset, uint32_t mem_mask);
	uint32_t s3c24xx_uart_1_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_uart_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_uart_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_uart_fifo_w(int uart, uint8_t data);
	void s3c24xx_usb_device_reset();
	uint32_t s3c24xx_usb_device_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_usb_device_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_wdt_reset();
	uint16_t s3c24xx_wdt_calc_current_count();
	uint32_t s3c24xx_wdt_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_wdt_start();
	void s3c24xx_wdt_stop();
	void s3c24xx_wdt_recalc();
	void s3c24xx_wdt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_wdt_timer_exp(void *ptr, int32_t param);
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
	uint32_t s3c24xx_iic_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_iic_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_iic_timer_exp(void *ptr, int32_t param);
	void s3c24xx_iis_reset();
	inline void iface_i2s_data_w(int ch, uint16_t data);
	void s3c24xx_iis_start();
	void s3c24xx_iis_stop();
	void s3c24xx_iis_recalc();
	uint32_t s3c24xx_iis_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_iis_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_iis_timer_exp(void *ptr, int32_t param);
	void s3c24xx_rtc_reset();
	uint32_t s3c24xx_rtc_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_rtc_recalc();
	void s3c24xx_rtc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_rtc_timer_tick_count_exp(void *ptr, int32_t param);
	void s3c24xx_rtc_update();
	void s3c24xx_rtc_check_alarm();
	void s3c24xx_rtc_timer_update_exp(void *ptr, int32_t param);
	void s3c24xx_adc_reset();
	uint32_t iface_adc_data_r(int ch);
	uint32_t s3c24xx_adc_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_adc_start();
	void s3c24xx_adc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_spi_reset();
	uint32_t s3c24xx_spi_r(uint32_t ch, uint32_t offset);
	void s3c24xx_spi_w(uint32_t ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t s3c24xx_spi_0_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_spi_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_mmc_reset();
	uint32_t s3c24xx_mmc_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void s3c24xx_mmc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void s3c24xx_device_reset();
	void s3c24xx_device_start();


	void s3c2400_uart_fifo_w(int uart, uint8_t data);

	/*******************************************************************************
	    TYPE DEFINITIONS
	*******************************************************************************/

	struct s3c24xx_memcon_regs_t
	{
		uint32_t data[0x34/4];
	};

	struct s3c24xx_usbhost_regs_t
	{
		uint32_t data[0x5C/4];
	};

	struct s3c24xx_irq_regs_t
	{
		uint32_t srcpnd;
		uint32_t intmod;
		uint32_t intmsk;
		uint32_t priority;
		uint32_t intpnd;
		uint32_t intoffset;
	};

	struct s3c24xx_dma_regs_t
	{
		uint32_t disrc;
		uint32_t didst;
		uint32_t dcon;
		uint32_t dstat;
		uint32_t dcsrc;
		uint32_t dcdst;
		uint32_t dmasktrig;
	};

	struct s3c24xx_clkpow_regs_t
	{
		uint32_t locktime;
		uint32_t mpllcon;
		uint32_t upllcon;
		uint32_t clkcon;
		uint32_t clkslow;
		uint32_t clkdivn;
	};

	struct s3c24xx_lcd_regs_t
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

	struct s3c24xx_lcdpal_regs_t
	{
		uint32_t data[0x400/4];
	};

	struct s3c24xx_uart_regs_t
	{
		uint32_t ulcon;
		uint32_t ucon;
		uint32_t ufcon;
		uint32_t umcon;
		uint32_t utrstat;
		uint32_t uerstat;
		uint32_t ufstat;
		uint32_t umstat;
		uint32_t utxh;
		uint32_t urxh;
		uint32_t ubrdiv;
	};

	struct s3c24xx_pwm_regs_t
	{
		uint32_t tcfg0;
		uint32_t tcfg1;
		uint32_t tcon;
		uint32_t tcntb0;
		uint32_t tcmpb0;
		uint32_t tcnto0;
		uint32_t tcntb1;
		uint32_t tcmpb1;
		uint32_t tcnto1;
		uint32_t tcntb2;
		uint32_t tcmpb2;
		uint32_t tcnto2;
		uint32_t tcntb3;
		uint32_t tcmpb3;
		uint32_t tcnto3;
		uint32_t tcntb4;
		uint32_t tcnto4;
	};

	struct s3c24xx_usbdev_regs_t
	{
		uint32_t data[0xBC/4];
	};

	struct s3c24xx_wdt_regs_t
	{
		uint32_t wtcon;
		uint32_t wtdat;
		uint32_t wtcnt;
	};

	struct s3c24xx_iic_regs_t
	{
		uint32_t iiccon;
		uint32_t iicstat;
		uint32_t iicadd;
		uint32_t iicds;
	};

	struct s3c24xx_iis_regs_t
	{
		uint32_t iiscon;
		uint32_t iismod;
		uint32_t iispsr;
		uint32_t iisfcon;
		uint32_t iisfifo;
	};

	struct s3c24xx_gpio_regs_t
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

	struct s3c24xx_rtc_regs_t
	{
		uint32_t rtccon;
		uint32_t ticnt;
		uint32_t reserved[2];
		uint32_t rtcalm;
		uint32_t almsec;
		uint32_t almmin;
		uint32_t almhour;
		uint32_t almday;
		uint32_t almmon;
		uint32_t almyear;
		uint32_t rtcrst;
		uint32_t bcdsec;
		uint32_t bcdmin;
		uint32_t bcdhour;
		uint32_t bcdday;
		uint32_t bcddow;
		uint32_t bcdmon;
		uint32_t bcdyear;
	};

	struct s3c24xx_adc_regs_t
	{
		uint32_t adccon;
		uint32_t adcdat;
	};

	struct s3c24xx_spi_regs_t
	{
		uint32_t spcon;
		uint32_t spsta;
		uint32_t sppin;
		uint32_t sppre;
		uint32_t sptdat;
		uint32_t sprdat;
	};

	struct s3c24xx_mmc_regs_t
	{
		uint32_t data[0x40/4];
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
		uint32_t cnt[5];
		uint32_t cmp[5];
		uint32_t freq[5];
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
		uint16_t fifo[16/2];
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
