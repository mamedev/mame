// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C44B0

    (c) 2011 Tim Schuerewegen

*******************************************************************************/

#ifndef __S3C44B0_H__
#define __S3C44B0_H__


/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

/* Memory Controller */

#define S3C44B0_BASE_MEMCON 0x01C80000

/* Interrupt Controller */

#define S3C44B0_BASE_INT 0x01E00000

#define S3C44B0_INTCON    (0x00 / 4) // Interrupt Control
#define S3C44B0_INTPND    (0x04 / 4) // Interrupt Request Status
#define S3C44B0_INTMOD    (0x08 / 4) // Interrupt Mode Control
#define S3C44B0_INTMSK    (0x0C / 4) // Interrupt Mask Control
#define S3C44B0_I_PSLV    (0x10 / 4)
#define S3C44B0_I_PMST    (0x14 / 4)
#define S3C44B0_I_CSLV    (0x18 / 4)
#define S3C44B0_I_CMST    (0x1C / 4)
#define S3C44B0_I_ISPR    (0x20 / 4)
#define S3C44B0_I_ISPC    (0x24 / 4)
#define S3C44B0_F_ISPR    (0x38 / 4)
#define S3C44B0_F_ISPC    (0x3C / 4)

/* ZDMA & BDMA */

#define S3C44B0_BASE_ZDMA_0 0x01E80000
#define S3C44B0_BASE_ZDMA_1 0x01E80020
#define S3C44B0_BASE_BDMA_0 0x01F80000
#define S3C44B0_BASE_BDMA_1 0x01F80020

#define S3C44B0_DCON      (0x00 / 4) // DMA Control
#define S3C44B0_DISRC     (0x04 / 4) // DMA Initial Source
#define S3C44B0_DIDST     (0x08 / 4) // DMA Initial Destination
#define S3C44B0_DICNT     (0x0C / 4) // DMA Initial Transfer Count
#define S3C44B0_DCSRC     (0x10 / 4) // DMA Current Source Address
#define S3C44B0_DCDST     (0x14 / 4) // DMA Current Destination Address
#define S3C44B0_DCCNT     (0x18 / 4) // DMA Current Transfer Count

/* Clock & Power Management */

#define S3C44B0_BASE_CLKPOW 0x01D80000

#define S3C44B0_PLLCON   (0x00 / 4) // PLL Control
#define S3C44B0_CLKCON   (0x04 / 4) // Clock Generator Control
#define S3C44B0_CLKSLOW  (0x08 / 4) // Slow Clock Control
#define S3C44B0_LOCKTIME (0x0C / 4) // PLL lock time Counter

/* LCD Controller */

#define S3C44B0_BASE_LCD    0x01F00000

#define S3C44B0_LCDCON1   (0x00 / 4) // LCD Control 1
#define S3C44B0_LCDCON2   (0x04 / 4) // LCD Control 2
#define S3C44B0_LCDSADDR1 (0x08 / 4) // Frame Buffer Start Address 1
#define S3C44B0_LCDSADDR2 (0x0C / 4) // Frame Buffer Start Address 2
#define S3C44B0_LCDSADDR3 (0x10 / 4) // Virtual Screen Address Set
#define S3C44B0_REDLUT    (0x14 / 4) // STN: Red Lookup Table
#define S3C44B0_GREENLUT  (0x18 / 4) // STN: Green Lookup Table
#define S3C44B0_BLUELUT   (0x1C / 4) // STN: Blue Lookup Table
#define S3C44B0_LCDCON3   (0x40 / 4) // LCD Control 3
#define S3C44B0_DITHMODE  (0x44 / 4) // STN: Dithering Mode

/* UART */

#define S3C44B0_BASE_UART_0 0x01D00000
#define S3C44B0_BASE_UART_1 0x01D04000

#define S3C44B0_ULCON   (0x00 / 4) // UART Line Control
#define S3C44B0_UCON    (0x04 / 4) // UART Control
#define S3C44B0_UFCON   (0x08 / 4) // UART FIFO Control
#define S3C44B0_UMCON   (0x0C / 4) // UART Modem Control
#define S3C44B0_UTRSTAT (0x10 / 4) // UART Tx/Rx Status
#define S3C44B0_UERSTAT (0x14 / 4) // UART Rx Error Status
#define S3C44B0_UFSTAT  (0x18 / 4) // UART FIFO Status
#define S3C44B0_UMSTAT  (0x1C / 4) // UART Modem Status
#define S3C44B0_UTXH    (0x20 / 4) // UART Transmission Hold
#define S3C44B0_URXH    (0x24 / 4) // UART Receive Buffer
#define S3C44B0_UBRDIV  (0x28 / 4) // UART Baud Rate Divisor

/* SIO */

#define S3C44B0_BASE_SIO 0x01D14000

#define S3C44B0_SIOCON  (0x00 / 4) // SIO Control
#define S3C44B0_SIODAT  (0x04 / 4) // SIO Data
#define S3C44B0_SBRDR   (0x08 / 4) // SIO Baud Rate Prescaler
#define S3C44B0_ITVCNT  (0x0C / 4) // SIO Interval Counter
#define S3C44B0_DCNTZ   (0x10 / 4) // SIO DMA Count Zero

/* PWM Timer */

#define S3C44B0_BASE_PWM 0x01D50000

#define S3C44B0_TCFG0  (0x00 / 4) // Timer Configuration
#define S3C44B0_TCFG1  (0x04 / 4) // Timer Configuration
#define S3C44B0_TCON   (0x08 / 4) // Timer Control
#define S3C44B0_TCNTB0 (0x0C / 4) // Timer Count Buffer 0
#define S3C44B0_TCMPB0 (0x10 / 4) // Timer Compare Buffer 0
#define S3C44B0_TCNTO0 (0x14 / 4) // Timer Count Observation 0
#define S3C44B0_TCNTB1 (0x18 / 4) // Timer Count Buffer 1
#define S3C44B0_TCMPB1 (0x1C / 4) // Timer Compare Buffer 1
#define S3C44B0_TCNTO1 (0x20 / 4) // Timer Count Observation 1
#define S3C44B0_TCNTB2 (0x24 / 4) // Timer Count Buffer 2
#define S3C44B0_TCMPB2 (0x28 / 4) // Timer Compare Buffer 2
#define S3C44B0_TCNTO2 (0x2C / 4) // Timer Count Observation 2
#define S3C44B0_TCNTB3 (0x30 / 4) // Timer Count Buffer 3
#define S3C44B0_TCMPB3 (0x34 / 4) // Timer Compare Buffer 3
#define S3C44B0_TCNTO3 (0x38 / 4) // Timer Count Observation 3
#define S3C44B0_TCNTB4 (0x3C / 4) // Timer Count Buffer 4
#define S3C44B0_TCMPB4 (0x40 / 4) // Timer Compare Buffer 4
#define S3C44B0_TCNTO4 (0x44 / 4) // Timer Count Observation 4
#define S3C44B0_TCNTB5 (0x48 / 4) // Timer Count Buffer 5
#define S3C44B0_TCNTO5 (0x4C / 4) // Timer Count Observation 5

/* USB Device */

#define S3C44B0_BASE_USBDEV 0x15200140

/* Watchdog Timer */

#define S3C44B0_BASE_WDT 0x01D30000

#define S3C44B0_WTCON (0x00 / 4) // Watchdog Timer Mode
#define S3C44B0_WTDAT (0x04 / 4) // Watchdog Timer Data
#define S3C44B0_WTCNT (0x08 / 4) // Watchdog Timer Count

/* IIC */

#define S3C44B0_BASE_IIC 0x01D60000

#define S3C44B0_IICCON  (0x00 / 4) // IIC Control
#define S3C44B0_IICSTAT (0x04 / 4) // IIC Status
#define S3C44B0_IICADD  (0x08 / 4) // IIC Address
#define S3C44B0_IICDS   (0x0C / 4) // IIC Data Shift

/* IIS */

#define S3C44B0_BASE_IIS 0x01D18000

#define S3C44B0_IISCON  (0x00 / 4) // IIS Control
#define S3C44B0_IISMOD  (0x04 / 4) // IIS Mode
#define S3C44B0_IISPSR  (0x08 / 4) // IIS Prescaler
#define S3C44B0_IISFCON (0x0C / 4) // IIS FIFO Control
#define S3C44B0_IISFIFO (0x10 / 4) // IIS FIFO Entry

/* I/O Port */

#define S3C44B0_BASE_GPIO 0x01D20000

#define S3C44B0_GPACON    (0x00 / 4) // Port A Control
#define S3C44B0_GPADAT    (0x04 / 4) // Port A Data
#define S3C44B0_GPBCON    (0x08 / 4) // Port B Control
#define S3C44B0_GPBDAT    (0x0C / 4) // Port B Data
#define S3C44B0_GPCCON    (0x10 / 4) // Port C Control
#define S3C44B0_GPCDAT    (0x14 / 4) // Port C Data
#define S3C44B0_GPCUP     (0x18 / 4) // Pull-up Control C
#define S3C44B0_GPDCON    (0x1C / 4) // Port D Control
#define S3C44B0_GPDDAT    (0x20 / 4) // Port D Data
#define S3C44B0_GPDUP     (0x24 / 4) // Pull-up Control D
#define S3C44B0_GPECON    (0x28 / 4) // Port E Control
#define S3C44B0_GPEDAT    (0x2C / 4) // Port E Data
#define S3C44B0_GPEUP     (0x30 / 4) // Pull-up Control E
#define S3C44B0_GPFCON    (0x34 / 4) // Port F Control
#define S3C44B0_GPFDAT    (0x38 / 4) // Port F Data
#define S3C44B0_GPFUP     (0x3C / 4) // Pull-up Control F
#define S3C44B0_GPGCON    (0x40 / 4) // Port G Control
#define S3C44B0_GPGDAT    (0x44 / 4) // Port G Data
#define S3C44B0_GPGUP     (0x48 / 4) // Pull-up Control G
#define S3C44B0_SPUCR     (0x4C / 4) // Special Pull-up
#define S3C44B0_EXTINT    (0x50 / 4) // External Interrupt Control
#define S3C44B0_EXTINTPND (0x54 / 4) // External Interrupt Pending

#define S3C44B0_GPADAT_MASK 0x000003FF
#define S3C44B0_GPBDAT_MASK 0x000007FF
#define S3C44B0_GPCDAT_MASK 0x0000FFFF
#define S3C44B0_GPDDAT_MASK 0x000000FF
#define S3C44B0_GPEDAT_MASK 0x000001FF
#define S3C44B0_GPFDAT_MASK 0x000001FF
#define S3C44B0_GPGDAT_MASK 0x000000FF

/* RTC */

#define S3C44B0_BASE_RTC 0x01D70040

#define S3C44B0_RTCCON  (0x00 / 4) // RTC Control
#define S3C44B0_RTCALM  (0x10 / 4) // RTC Alarm Control
#define S3C44B0_ALMSEC  (0x14 / 4) // Alarm Second
#define S3C44B0_ALMMIN  (0x18 / 4) // Alarm Minute
#define S3C44B0_ALMHOUR (0x1C / 4) // Alarm Hour
#define S3C44B0_ALMDAY  (0x20 / 4) // Alarm Day
#define S3C44B0_ALMMON  (0x24 / 4) // Alarm Month
#define S3C44B0_ALMYEAR (0x28 / 4) // Alarm Year
#define S3C44B0_RTCRST  (0x2C / 4) // RTC Round Reset
#define S3C44B0_BCDSEC  (0x30 / 4) // BCD Second
#define S3C44B0_BCDMIN  (0x34 / 4) // BCD Minute
#define S3C44B0_BCDHOUR (0x38 / 4) // BCD Hour
#define S3C44B0_BCDDAY  (0x3C / 4) // BCD Day
#define S3C44B0_BCDDOW  (0x40 / 4) // BCD Day of Week
#define S3C44B0_BCDMON  (0x44 / 4) // BCD Month
#define S3C44B0_BCDYEAR (0x48 / 4) // BCD Year
#define S3C44B0_TICNT   (0x4C / 4) // Tick Time count

/* A/D Converter */

#define S3C44B0_BASE_ADC 0x01D40000

#define S3C44B0_ADCCON  (0x00 / 4) // ADC Control
#define S3C44B0_ADCPSR  (0x04 / 4) // ADC Prescaler
#define S3C44B0_ADCDAT  (0x08 / 4) // ADC Data

/* CPU Wrapper */

#define S3C44B0_BASE_CPU_WRAPPER 0x01C00000

#define S3C44B0_SYSCFG    (0x00 / 4) // System Configuration
#define S3C44B0_NCACHBE0  (0x04 / 4) // Non Cacheable Area 0
#define S3C44B0_NCACHBE1  (0x08 / 4) // Non Cacheable Area 1

/* ... */

#define S3C44B0_INT_ADC        0
#define S3C44B0_INT_RTC        1
#define S3C44B0_INT_UTXD1      2
#define S3C44B0_INT_UTXD0      3
#define S3C44B0_INT_SIO        4
#define S3C44B0_INT_IIC        5
#define S3C44B0_INT_URXD1      6
#define S3C44B0_INT_URXD0      7
#define S3C44B0_INT_TIMER5     8
#define S3C44B0_INT_TIMER4     9
#define S3C44B0_INT_TIMER3    10
#define S3C44B0_INT_TIMER2    11
#define S3C44B0_INT_TIMER1    12
#define S3C44B0_INT_TIMER0    13
#define S3C44B0_INT_UERR      14
#define S3C44B0_INT_WDT       15
#define S3C44B0_INT_BDMA1     16
#define S3C44B0_INT_BDMA0     17
#define S3C44B0_INT_ZDMA1     18
#define S3C44B0_INT_ZDMA0     19
#define S3C44B0_INT_TICK      20
#define S3C44B0_INT_EINT4_7   21
#define S3C44B0_INT_EINT3     22
#define S3C44B0_INT_EINT2     23
#define S3C44B0_INT_EINT1     24
#define S3C44B0_INT_EINT0     25

#define S3C44B0_MODESEL_01      0
#define S3C44B0_MODESEL_02      1
#define S3C44B0_MODESEL_04      2
#define S3C44B0_MODESEL_08      3

#define S3C44B0_PNRMODE_STN_04_DS  0
#define S3C44B0_PNRMODE_STN_04_SS  1
#define S3C44B0_PNRMODE_STN_08_SS  2

#define S3C44B0_GPIO_PORT_A S3C44B0_GPIO_PORT_A
#define S3C44B0_GPIO_PORT_B S3C44B0_GPIO_PORT_B
#define S3C44B0_GPIO_PORT_C S3C44B0_GPIO_PORT_C
#define S3C44B0_GPIO_PORT_D S3C44B0_GPIO_PORT_D
#define S3C44B0_GPIO_PORT_E S3C44B0_GPIO_PORT_E
#define S3C44B0_GPIO_PORT_F S3C44B0_GPIO_PORT_F
#define S3C44B0_GPIO_PORT_G S3C44B0_GPIO_PORT_G


/*******************************************************************************
 MACROS / CONSTANTS
 *******************************************************************************/


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

class s3c44b0_device : public device_t
{
public:
	s3c44b0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~s3c44b0_device() {}

	template<class _Object> static devcb_base &set_gpio_port_r_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_port_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_gpio_port_w_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_port_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2c_scl_w_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_scl_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2c_sda_r_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_sda_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2c_sda_w_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_sda_w_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_adc_data_r_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_data_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_i2s_data_w_callback(device_t &device, _Object object) { return downcast<s3c44b0_device &>(device).m_data_w_cb.set_callback(object); }

	uint32_t lcd_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t clkpow_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t irq_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t pwm_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t iic_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t gpio_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t uart_0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t uart_1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t wdt_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t cpuwrap_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t adc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t sio_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t iis_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t zdma_0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t zdma_1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t bdma_0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t bdma_1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	void lcd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void clkpow_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void irq_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void pwm_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void iic_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void gpio_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void uart_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void uart_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void wdt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void cpuwrap_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void adc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sio_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void iis_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void zdma_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void zdma_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void bdma_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void bdma_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void request_eint(uint32_t number);
	uint32_t video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
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
	void lcd_timer_exp(void *ptr, int32_t param);

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
	void pwm_timer_exp(void *ptr, int32_t param);
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
	void iic_timer_exp(void *ptr, int32_t param);

	// I/O Port
	inline uint32_t iface_gpio_port_r(int port);
	inline void iface_gpio_port_w(int port, uint32_t data);

	// UART
	uint32_t uart_r(int ch, uint32_t offset);
	void uart_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	void uart_fifo_w(int uart, uint8_t data);
	void uart_timer_exp(void *ptr, int32_t param);

	// Watchdog Timer
	uint16_t wdt_calc_current_count();
	void wdt_start();
	void wdt_stop();
	void wdt_recalc();
	void wdt_timer_exp(void *ptr, int32_t param);

	// A/D Converter
	void adc_start();
	void adc_stop();
	void adc_recalc();
	void adc_timer_exp(void *ptr, int32_t param);

	// SIO
	void sio_start();
	void sio_stop();
	void sio_recalc();
	void sio_timer_exp(void *ptr, int32_t param);

	// IIS
	inline void iface_i2s_data_w(address_space &space, int ch, uint16_t data);
	void iis_start();
	void iis_stop();
	void iis_timer_exp(void *ptr, int32_t param);

	// ZDMA
	void zdma_trigger(int ch);
	void zdma_start(int ch);
	uint32_t zdma_r(int ch, uint32_t offset);
	void zdma_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	void zdma_timer_exp(void *ptr, int32_t param);

	// BDMA
	void bdma_trigger(int ch);
	void bdma_request_iis();
	uint32_t bdma_r(int ch, uint32_t offset);
	void bdma_start(int ch);
	void bdma_stop(int ch);
	void bdma_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask);
	void bdma_timer_exp(void *ptr, int32_t param);

	cpu_device *m_cpu;
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

	void s3c44b0_postload();
};

extern const device_type S3C44B0;


#define MCFG_S3C44B0_GPIO_PORT_R_CB(_devcb) \
	devcb = &s3c44b0_device::set_gpio_port_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C44B0_GPIO_PORT_W_CB(_devcb) \
	devcb = &s3c44b0_device::set_gpio_port_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C44B0_I2C_SCL_W_CB(_devcb) \
	devcb = &s3c44b0_device::set_i2c_scl_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C44B0_I2C_SDA_R_CB(_devcb) \
	devcb = &s3c44b0_device::set_i2c_sda_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C44B0_I2C_SDA_W_CB(_devcb) \
	devcb = &s3c44b0_device::set_i2c_sda_w_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C44B0_ADC_DATA_R_CB(_devcb) \
	devcb = &s3c44b0_device::set_adc_data_r_callback(*device, DEVCB_##_devcb);

#define MCFG_S3C44B0_I2S_DATA_W_CB(_devcb) \
	devcb = &s3c44b0_device::set_i2s_data_w_callback(*device, DEVCB_##_devcb);


#endif
