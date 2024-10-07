// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C44B0

    (c) 2011 Tim Schuerewegen

*******************************************************************************/

#ifndef MAME_MACHINE_S3C44B0_H
#define MAME_MACHINE_S3C44B0_H

#pragma once


/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

/* Memory Controller */

#define S3C44B0_BASE_MEMCON 0x01C80000

/* Interrupt Controller */

#define S3C44B0_BASE_INT 0x01E00000

/* ZDMA & BDMA */

#define S3C44B0_BASE_ZDMA_0 0x01E80000
#define S3C44B0_BASE_ZDMA_1 0x01E80020
#define S3C44B0_BASE_BDMA_0 0x01F80000
#define S3C44B0_BASE_BDMA_1 0x01F80020

/* Clock & Power Management */

#define S3C44B0_BASE_CLKPOW 0x01D80000

/* LCD Controller */

#define S3C44B0_BASE_LCD    0x01F00000

/* UART */

#define S3C44B0_BASE_UART_0 0x01D00000
#define S3C44B0_BASE_UART_1 0x01D04000

/* SIO */

#define S3C44B0_BASE_SIO 0x01D14000

#define S3C44B0_SIOCON  (0x00 / 4) // SIO Control
#define S3C44B0_SIODAT  (0x04 / 4) // SIO Data
#define S3C44B0_SBRDR   (0x08 / 4) // SIO Baud Rate Prescaler
#define S3C44B0_ITVCNT  (0x0C / 4) // SIO Interval Counter
#define S3C44B0_DCNTZ   (0x10 / 4) // SIO DMA Count Zero

/* PWM Timer */

#define S3C44B0_BASE_PWM 0x01D50000

/* USB Device */

#define S3C44B0_BASE_USBDEV 0x15200140

/* Watchdog Timer */

#define S3C44B0_BASE_WDT 0x01D30000

/* IIC */

#define S3C44B0_BASE_IIC 0x01D60000

/* IIS */

#define S3C44B0_BASE_IIS 0x01D18000

/* I/O Port */

#define S3C44B0_BASE_GPIO 0x01D20000

/* RTC */

#define S3C44B0_BASE_RTC 0x01D70040

/* A/D Converter */

#define S3C44B0_BASE_ADC 0x01D40000

/* CPU Wrapper */

#define S3C44B0_BASE_CPU_WRAPPER 0x01C00000

/* ... */


/*******************************************************************************
 MACROS / CONSTANTS
 *******************************************************************************/

enum
{
	S3C44B0_GPIO_PORT_A = 0,
	S3C44B0_GPIO_PORT_B,
	S3C44B0_GPIO_PORT_C,
	S3C44B0_GPIO_PORT_D,
	S3C44B0_GPIO_PORT_E,
	S3C44B0_GPIO_PORT_F,
	S3C44B0_GPIO_PORT_G
};


class s3c44b0_device : public device_t, public device_video_interface
{
public:
	s3c44b0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto gpio_port_r_cb() { return m_port_r_cb.bind(); }
	auto gpio_port_w_cb() { return m_port_w_cb.bind(); }
	auto i2c_scl_w_cb() { return m_scl_w_cb.bind(); }
	auto i2c_sda_r_cb() { return m_sda_r_cb.bind(); }
	auto i2c_sda_w_cb() { return m_sda_w_cb.bind(); }
	auto adc_data_r_cb() { return m_data_r_cb.bind(); }
	auto i2s_data_w_cb() { return m_data_w_cb.bind(); }

	uint32_t lcd_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t clkpow_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t irq_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t pwm_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t iic_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t gpio_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t uart_0_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t uart_1_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t wdt_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t cpuwrap_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t adc_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t sio_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t iis_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t zdma_0_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t zdma_1_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t bdma_0_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t bdma_1_r(offs_t offset, uint32_t mem_mask = ~0);

	void lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void clkpow_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void irq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void pwm_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void iic_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void uart_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void uart_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void wdt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void cpuwrap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void adc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void sio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void iis_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void zdma_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void zdma_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bdma_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bdma_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void request_eint(uint32_t number);
	uint32_t video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_reset() override ATTR_COLD;

private:
	struct s3c44b0_memcon_regs_t
	{
		uint32_t data[0x34/4];
	};

	struct s3c44b0_irq_regs_t
	{
		uint32_t intcon;
		uint32_t intpnd;
		uint32_t intmod;
		uint32_t intmsk;
		uint32_t i_pslv;
		uint32_t i_pmst;
		uint32_t i_cslv;
		uint32_t i_cmst;
		uint32_t i_ispr;
		uint32_t i_ispc;
		uint32_t reserved[4];
		uint32_t f_ispr;
		uint32_t f_ispc;
	};

	struct s3c44b0_dma_regs_t
	{
		uint32_t dcon;
		uint32_t disrc;
		uint32_t didst;
		uint32_t dicnt;
		uint32_t dcsrc;
		uint32_t dcdst;
		uint32_t dccnt;
	};

	struct s3c44b0_clkpow_regs_t
	{
		uint32_t pllcon;
		uint32_t clkcon;
		uint32_t clkslow;
		uint32_t locktime;
	};

	struct s3c44b0_lcd_regs_t
	{
		uint32_t lcdcon1;
		uint32_t lcdcon2;
		uint32_t lcdsaddr1;
		uint32_t lcdsaddr2;
		uint32_t lcdsaddr3;
		uint32_t redlut;
		uint32_t greenlut;
		uint32_t bluelut;
		uint32_t reserved[8];
		uint32_t lcdcon3;
		uint32_t dithmode;
	};

	struct s3c44b0_uart_regs_t
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

	struct s3c44b0_sio_regs_t
	{
		uint32_t siocon;
		uint32_t siodat;
		uint32_t sbrdr;
		uint32_t itvcnt;
		uint32_t dcntz;
	};

	struct s3c44b0_pwm_regs_t
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
		uint32_t tcmpb4;
		uint32_t tcnto4;
		uint32_t tcntb5;
		uint32_t tcnto5;
	};

	struct s3c44b0_wdt_regs_t
	{
		uint32_t wtcon;
		uint32_t wtdat;
		uint32_t wtcnt;
	};

	struct s3c44b0_iic_regs_t
	{
		uint32_t iiccon;
		uint32_t iicstat;
		uint32_t iicadd;
		uint32_t iicds;
	};

	struct s3c44b0_iis_regs_t
	{
		uint32_t iiscon;
		uint32_t iismod;
		uint32_t iispsr;
		uint32_t iisfcon;
		uint32_t iisfifo;
	};

	struct s3c44b0_gpio_regs_t
	{
		uint32_t gpacon;
		uint32_t gpadat;
		uint32_t gpbcon;
		uint32_t gpbdat;
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
		uint32_t spucr;
		uint32_t extint;
		uint32_t extintpnd;
	};

	struct s3c44b0_rtc_regs_t
	{
		uint32_t rtccon;
		uint32_t reserved[3];
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
		uint32_t ticnt;
	};

	struct s3c44b0_adc_regs_t
	{
		uint32_t adccon;
		uint32_t adcpsr;
		uint32_t adcdat;
	};

	struct s3c44b0_cpuwrap_regs_t
	{
		uint32_t syscfg;
		uint32_t ncachbe0;
		uint32_t ncachbe1;
	};

	struct s3c44b0_memcon_t
	{
		s3c44b0_memcon_regs_t regs;
	};

	struct s3c44b0_irq_t
	{
		s3c44b0_irq_regs_t regs;
		int line_irq, line_fiq;
	};

	struct s3c44b0_dma_t
	{
		s3c44b0_dma_regs_t regs;
		emu_timer *timer;
	};

	struct s3c44b0_clkpow_t
	{
		s3c44b0_clkpow_regs_t regs;
	};

	struct rectangle_t
	{
		int x1, y1, x2, y2;
	};

	struct s3c44b0_lcd_t
	{
		s3c44b0_lcd_regs_t regs;
		emu_timer *timer;
		std::unique_ptr<uint8_t[]> bitmap;
		uint32_t vramaddr_cur;
		uint32_t vramaddr_max;
		uint32_t offsize;
		uint32_t pagewidth_cur;
		uint32_t pagewidth_max;
		uint32_t modesel;
		uint32_t bswp;
		int vpos, hpos;
		double framerate;
		uint32_t hpos_min, hpos_max, hpos_end, vpos_min, vpos_max, vpos_end;
		attotime frame_time;
		attoseconds_t frame_period, pixeltime, scantime;
	};

	struct s3c44b0_uart_t
	{
		s3c44b0_uart_regs_t regs;
		emu_timer *timer;
	};

	struct s3c44b0_sio_t
	{
		s3c44b0_sio_regs_t regs;
		emu_timer *timer;
	};

	struct s3c44b0_pwm_t
	{
		s3c44b0_pwm_regs_t regs;
		emu_timer *timer[6];
		uint32_t cnt[6];
		uint32_t cmp[6];
		uint32_t freq[6];
	};

	struct s3c44b0_wdt_t
	{
		s3c44b0_wdt_regs_t regs;
		emu_timer *timer;
	};

	struct s3c44b0_iic_t
	{
		s3c44b0_iic_regs_t regs;
		emu_timer *timer;
		int count;
	};

	struct s3c44b0_iis_t
	{
		s3c44b0_iis_regs_t regs;
		emu_timer *timer;
		uint16_t fifo[16/2];
		int fifo_index;
	};

	struct s3c44b0_gpio_t
	{
		s3c44b0_gpio_regs_t regs;
	};

	struct s3c44b0_rtc_t
	{
		s3c44b0_rtc_regs_t regs;
		emu_timer *timer_tick_count;
		emu_timer *timer_update;
	};

	struct s3c44b0_adc_t
	{
		s3c44b0_adc_regs_t regs;
		emu_timer *timer;
	};

	struct s3c44b0_cpuwrap_t
	{
		s3c44b0_cpuwrap_regs_t regs;
	};


	// internal state
	// LCD Controller
	rgb_t lcd_get_color_stn_04(uint8_t data);
	uint8_t lcd_get_color_stn_08_r(uint8_t data);
	uint8_t lcd_get_color_stn_08_g(uint8_t data);
	uint8_t lcd_get_color_stn_08_b(uint8_t data);
	void lcd_dma_reload();
	void lcd_dma_init();
	void lcd_dma_read(int count, uint8_t *data);
	void lcd_render_stn_04();
	void lcd_render_stn_08();
	attotime time_until_pos(int vpos, int hpos);
	int lcd_get_vpos();
	int lcd_get_hpos();
	void video_start();
	void lcd_configure();
	void lcd_start();
	void lcd_stop();
	void lcd_recalc();
	TIMER_CALLBACK_MEMBER(lcd_timer_exp);

	// Clock & Power Management
	uint32_t get_mclk();

	// Interrupt Controller
	void check_pending_irq();
	void request_irq(uint32_t int_type);
	void check_pending_eint();

	// PWM Timer
	uint16_t pwm_calc_observation(int ch);
	void pwm_start(int timer);
	void pwm_stop(int timer);
	void pwm_recalc(int timer);
	TIMER_CALLBACK_MEMBER(pwm_timer_exp);
	//void dma_request_pwm();

	// IIC
	inline void iface_i2c_scl_w(int state);
	inline void iface_i2c_sda_w(int state);
	inline int iface_i2c_sda_r();
	void i2c_send_start();
	void i2c_send_stop();
	uint8_t i2c_receive_byte(int ack);
	int i2c_send_byte(uint8_t data);
	void iic_start();
	void iic_stop();
	void iic_resume();
	TIMER_CALLBACK_MEMBER(iic_timer_exp);

	// I/O Port
	inline uint32_t iface_gpio_port_r(int port);
	inline void iface_gpio_port_w(int port, uint32_t data);

	// UART
	uint32_t uart_r(int ch, uint32_t offset);
	void uart_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	void uart_fifo_w(int uart, uint8_t data);
	TIMER_CALLBACK_MEMBER(uart_timer_exp);

	// Watchdog Timer
	uint16_t wdt_calc_current_count();
	void wdt_start();
	void wdt_stop();
	void wdt_recalc();
	TIMER_CALLBACK_MEMBER(wdt_timer_exp);

	// A/D Converter
	void adc_start();
	void adc_stop();
	void adc_recalc();
	TIMER_CALLBACK_MEMBER(adc_timer_exp);

	// SIO
	void sio_start();
	void sio_stop();
	void sio_recalc();
	TIMER_CALLBACK_MEMBER(sio_timer_exp);

	// IIS
	inline void iface_i2s_data_w(int ch, uint16_t data);
	void iis_start();
	void iis_stop();
	TIMER_CALLBACK_MEMBER(iis_timer_exp);

	// ZDMA
	void zdma_trigger(int ch);
	void zdma_start(int ch);
	uint32_t zdma_r(int ch, uint32_t offset);
	void zdma_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	TIMER_CALLBACK_MEMBER(zdma_timer_exp);

	// BDMA
	void bdma_trigger(int ch);
	void bdma_request_iis();
	uint32_t bdma_r(int ch, uint32_t offset);
	void bdma_start(int ch);
	void bdma_stop(int ch);
	void bdma_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	TIMER_CALLBACK_MEMBER(bdma_timer_exp);

	required_device<cpu_device> m_cpu;
	//s3c44b0_memcon_t m_memcon;
	s3c44b0_irq_t m_irq;
	s3c44b0_dma_t m_zdma[2];
	s3c44b0_dma_t m_bdma[2];
	s3c44b0_clkpow_t m_clkpow;
	s3c44b0_lcd_t m_lcd;
	s3c44b0_uart_t m_uart[2];
	s3c44b0_sio_t m_sio;
	s3c44b0_pwm_t m_pwm;
	s3c44b0_wdt_t m_wdt;
	s3c44b0_iic_t m_iic;
	s3c44b0_iis_t m_iis;
	s3c44b0_gpio_t m_gpio;
	//s3c44b0_rtc_t m_rtc;
	s3c44b0_adc_t m_adc;
	s3c44b0_cpuwrap_t m_cpuwrap;

	devcb_read32 m_port_r_cb;
	devcb_write32 m_port_w_cb;
	devcb_write_line m_scl_w_cb;
	devcb_read_line m_sda_r_cb;
	devcb_write_line m_sda_w_cb;
	devcb_read32 m_data_r_cb;
	devcb_write16 m_data_w_cb;

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
};

DECLARE_DEVICE_TYPE(S3C44B0, s3c44b0_device)


#endif // MAME_MACHINE_S3C44B0_H
