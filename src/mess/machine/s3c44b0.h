/*******************************************************************************

    Samsung S3C44B0

    (c) 2011 Tim Schuerewegen

*******************************************************************************/

#ifndef __S3C44B0_H__
#define __S3C44B0_H__

#include "devlegcy.h"

/*******************************************************************************
    MACROS / CONSTANTS
*******************************************************************************/

#define S3C44B0_TAG "s3c44b0"

#define MCFG_S3C44B0_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, S3C44B0, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define S3C44B0_INTERFACE(name) \
	const s3c44b0_interface(name) =

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
	s3c44b0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s3c44b0_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type S3C44B0;


/*******************************************************************************
    TYPE DEFINITIONS
*******************************************************************************/

typedef UINT32 (*s3c44b0_gpio_port_r_func)( device_t *device, int port);
typedef void (*s3c44b0_gpio_port_w_func)( device_t *device, int port, UINT32 data);

struct s3c44b0_interface_gpio
{
	s3c44b0_gpio_port_r_func port_r;
	s3c44b0_gpio_port_w_func port_w;
};

struct s3c44b0_interface_i2c
{
	write_line_device_func scl_w;
	read_line_device_func sda_r;
	write_line_device_func sda_w;
};

struct s3c44b0_interface_adc
{
	read32_device_func data_r;
};

struct s3c44b0_interface_i2s
{
	write16_device_func data_w;
};

struct s3c44b0_interface
{
	s3c44b0_interface_gpio gpio;
	s3c44b0_interface_i2c i2c;
	s3c44b0_interface_adc adc;
	s3c44b0_interface_i2s i2s;
};

/*******************************************************************************
    PROTOTYPES
*******************************************************************************/

VIDEO_START( s3c44b0 );
SCREEN_UPDATE_RGB32( s3c44b0 );

void s3c44b0_request_eint( device_t *device, UINT32 number);

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
    TYPE DEFINITIONS
*******************************************************************************/

struct s3c44b0_memcon_regs_t
{
	UINT32 data[0x34/4];
};

struct s3c44b0_irq_regs_t
{
	UINT32 intcon;
	UINT32 intpnd;
	UINT32 intmod;
	UINT32 intmsk;
	UINT32 i_pslv;
	UINT32 i_pmst;
	UINT32 i_cslv;
	UINT32 i_cmst;
	UINT32 i_ispr;
	UINT32 i_ispc;
	UINT32 reserved[4];
	UINT32 f_ispr;
	UINT32 f_ispc;
};

struct s3c44b0_dma_regs_t
{
	UINT32 dcon;
	UINT32 disrc;
	UINT32 didst;
	UINT32 dicnt;
	UINT32 dcsrc;
	UINT32 dcdst;
	UINT32 dccnt;
};

struct s3c44b0_clkpow_regs_t
{
	UINT32 pllcon;
	UINT32 clkcon;
	UINT32 clkslow;
	UINT32 locktime;
};

struct s3c44b0_lcd_regs_t
{
	UINT32 lcdcon1;
	UINT32 lcdcon2;
	UINT32 lcdsaddr1;
	UINT32 lcdsaddr2;
	UINT32 lcdsaddr3;
	UINT32 redlut;
	UINT32 greenlut;
	UINT32 bluelut;
	UINT32 reserved[8];
	UINT32 lcdcon3;
	UINT32 dithmode;
};

struct s3c44b0_uart_regs_t
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

struct s3c44b0_sio_regs_t
{
	UINT32 siocon;
	UINT32 siodat;
	UINT32 sbrdr;
	UINT32 itvcnt;
	UINT32 dcntz;
};

struct s3c44b0_pwm_regs_t
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
	UINT32 tcmpb4;
	UINT32 tcnto4;
	UINT32 tcntb5;
	UINT32 tcnto5;
};

struct s3c44b0_wdt_regs_t
{
	UINT32 wtcon;
	UINT32 wtdat;
	UINT32 wtcnt;
};

struct s3c44b0_iic_regs_t
{
	UINT32 iiccon;
	UINT32 iicstat;
	UINT32 iicadd;
	UINT32 iicds;
};

struct s3c44b0_iis_regs_t
{
	UINT32 iiscon;
	UINT32 iismod;
	UINT32 iispsr;
	UINT32 iisfcon;
	UINT32 iisfifo;
};

struct s3c44b0_gpio_regs_t
{
	UINT32 gpacon;
	UINT32 gpadat;
	UINT32 gpbcon;
	UINT32 gpbdat;
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
	UINT32 spucr;
	UINT32 extint;
	UINT32 extintpnd;
};

struct s3c44b0_rtc_regs_t
{
	UINT32 rtccon;
	UINT32 reserved[3];
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
	UINT32 ticnt;
};

struct s3c44b0_adc_regs_t
{
	UINT32 adccon;
	UINT32 adcpsr;
	UINT32 adcdat;
};

struct s3c44b0_cpuwrap_regs_t
{
	UINT32 syscfg;
	UINT32 ncachbe0;
	UINT32 ncachbe1;
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
	UINT8 *bitmap;
	UINT32 vramaddr_cur;
	UINT32 vramaddr_max;
	UINT32 offsize;
	UINT32 pagewidth_cur;
	UINT32 pagewidth_max;
	UINT32 modesel;
	UINT32 bswp;
	int vpos, hpos;
	double framerate;
	UINT32 hpos_min, hpos_max, hpos_end, vpos_min, vpos_max, vpos_end;
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
	UINT32 cnt[6];
	UINT32 cmp[6];
	UINT32 freq[6];
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
	UINT16 fifo[16/2];
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

struct s3c44b0_t
{
	const s3c44b0_interface *iface;
	address_space* space;
	cpu_device *cpu;
	s3c44b0_memcon_t memcon;
	s3c44b0_irq_t irq;
	s3c44b0_dma_t zdma[2];
	s3c44b0_dma_t bdma[2];
	s3c44b0_clkpow_t clkpow;
	s3c44b0_lcd_t lcd;
	s3c44b0_uart_t uart[2];
	s3c44b0_sio_t sio;
	s3c44b0_pwm_t pwm;
	s3c44b0_wdt_t wdt;
	s3c44b0_iic_t iic;
	s3c44b0_iis_t iis;
	s3c44b0_gpio_t gpio;
	s3c44b0_rtc_t rtc;
	s3c44b0_adc_t adc;
	s3c44b0_cpuwrap_t cpuwrap;
};

#endif
