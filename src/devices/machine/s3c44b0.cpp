// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Samsung S3C44B0

    (c) 2011 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "s3c44b0.h"

#include "cpu/arm7/arm7.h"
#include "screen.h"

#include <cstdarg>


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

#define S3C44B0_DCON      (0x00 / 4) // DMA Control
#define S3C44B0_DISRC     (0x04 / 4) // DMA Initial Source
#define S3C44B0_DIDST     (0x08 / 4) // DMA Initial Destination
#define S3C44B0_DICNT     (0x0C / 4) // DMA Initial Transfer Count
#define S3C44B0_DCSRC     (0x10 / 4) // DMA Current Source Address
#define S3C44B0_DCDST     (0x14 / 4) // DMA Current Destination Address
#define S3C44B0_DCCNT     (0x18 / 4) // DMA Current Transfer Count

#define S3C44B0_PLLCON   (0x00 / 4) // PLL Control
#define S3C44B0_CLKCON   (0x04 / 4) // Clock Generator Control
#define S3C44B0_CLKSLOW  (0x08 / 4) // Slow Clock Control
#define S3C44B0_LOCKTIME (0x0C / 4) // PLL lock time Counter

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

#define S3C44B0_WTCON (0x00 / 4) // Watchdog Timer Mode
#define S3C44B0_WTDAT (0x04 / 4) // Watchdog Timer Data
#define S3C44B0_WTCNT (0x08 / 4) // Watchdog Timer Count

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

#define S3C44B0_IICCON  (0x00 / 4) // IIC Control
#define S3C44B0_IICSTAT (0x04 / 4) // IIC Status
#define S3C44B0_IICADD  (0x08 / 4) // IIC Address
#define S3C44B0_IICDS   (0x0C / 4) // IIC Data Shift

#define S3C44B0_IISCON  (0x00 / 4) // IIS Control
#define S3C44B0_IISMOD  (0x04 / 4) // IIS Mode
#define S3C44B0_IISPSR  (0x08 / 4) // IIS Prescaler
#define S3C44B0_IISFCON (0x0C / 4) // IIS FIFO Control
#define S3C44B0_IISFIFO (0x10 / 4) // IIS FIFO Entry

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

#define S3C44B0_ADCCON  (0x00 / 4) // ADC Control
#define S3C44B0_ADCPSR  (0x04 / 4) // ADC Prescaler
#define S3C44B0_ADCDAT  (0x08 / 4) // ADC Data

#define S3C44B0_SYSCFG    (0x00 / 4) // System Configuration
#define S3C44B0_NCACHBE0  (0x04 / 4) // Non Cacheable Area 0
#define S3C44B0_NCACHBE1  (0x08 / 4) // Non Cacheable Area 1

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


#define VERBOSE_LEVEL ( 0 )

static inline void ATTR_PRINTF(3,4) verboselog( device_t &device, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		device.logerror( "%s: %s", device.machine().describe_context( ), buf);
	}
}

DEFINE_DEVICE_TYPE(S3C44B0, s3c44b0_device, "s3c44b0", "Samsung S3C44B0 SoC")

s3c44b0_device::s3c44b0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S3C44B0, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_port_r_cb(*this, 0)
	, m_port_w_cb(*this)
	, m_scl_w_cb(*this)
	, m_sda_r_cb(*this, 0)
	, m_sda_w_cb(*this)
	, m_data_r_cb(*this, 0)
	, m_data_w_cb(*this)
{
	memset(&m_irq, 0, sizeof(s3c44b0_irq_t));
	memset(m_zdma, 0, sizeof(s3c44b0_dma_t)*2);
	memset(m_bdma, 0, sizeof(s3c44b0_dma_t)*2);
	memset(&m_clkpow, 0, sizeof(s3c44b0_clkpow_t));
	m_lcd.clear();
	memset(m_uart, 0, sizeof(s3c44b0_uart_t)*2);
	memset(&m_sio, 0, sizeof(s3c44b0_sio_t));
	memset(&m_pwm, 0, sizeof(s3c44b0_pwm_t));
	memset(&m_wdt, 0, sizeof(s3c44b0_wdt_t));
	memset(&m_iic, 0, sizeof(s3c44b0_iic_t));
	memset(&m_iis, 0, sizeof(s3c44b0_iis_t));
	memset(&m_gpio, 0, sizeof(s3c44b0_gpio_t));
	memset(&m_adc, 0, sizeof(s3c44b0_adc_t));
	memset(&m_cpuwrap, 0, sizeof(s3c44b0_cpuwrap_t));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3c44b0_device::device_start()
{
	m_cpu->space(AS_PROGRAM).cache(m_cache);

	for (int i = 0; i < 6; i++) m_pwm.timer[i] = timer_alloc(FUNC(s3c44b0_device::pwm_timer_exp), this);
	for (auto & elem : m_uart) elem.timer = timer_alloc(FUNC(s3c44b0_device::uart_timer_exp), this);
	for (auto & elem : m_zdma) elem.timer = timer_alloc(FUNC(s3c44b0_device::zdma_timer_exp), this);
	for (auto & elem : m_bdma) elem.timer = timer_alloc(FUNC(s3c44b0_device::bdma_timer_exp), this);

	m_lcd.timer = timer_alloc(FUNC(s3c44b0_device::lcd_timer_exp), this);
	m_wdt.timer = timer_alloc(FUNC(s3c44b0_device::wdt_timer_exp), this);
	m_sio.timer = timer_alloc(FUNC(s3c44b0_device::sio_timer_exp), this);
	m_adc.timer = timer_alloc(FUNC(s3c44b0_device::adc_timer_exp), this);
	m_iic.timer = timer_alloc(FUNC(s3c44b0_device::iic_timer_exp), this);
	m_iis.timer = timer_alloc(FUNC(s3c44b0_device::iis_timer_exp), this);

	video_start();

	save_item(NAME(m_irq.regs.intcon));
	save_item(NAME(m_irq.regs.intpnd));
	save_item(NAME(m_irq.regs.intmod));
	save_item(NAME(m_irq.regs.intmsk));
	save_item(NAME(m_irq.regs.i_pslv));
	save_item(NAME(m_irq.regs.i_pmst));
	save_item(NAME(m_irq.regs.i_cslv));
	save_item(NAME(m_irq.regs.i_cmst));
	save_item(NAME(m_irq.regs.i_ispr));
	save_item(NAME(m_irq.regs.i_ispc));
	save_item(NAME(m_irq.regs.reserved));
	save_item(NAME(m_irq.regs.f_ispr));
	save_item(NAME(m_irq.regs.f_ispc));
	save_item(NAME(m_irq.line_irq));
	save_item(NAME(m_irq.line_fiq));

	save_item(NAME(m_clkpow.regs.pllcon));
	save_item(NAME(m_clkpow.regs.clkcon));
	save_item(NAME(m_clkpow.regs.clkslow));
	save_item(NAME(m_clkpow.regs.locktime));

	// FIXME: how to save m_lcd.bitmap which gets allocated/freed during emulation?
	save_item(NAME(m_lcd.regs.lcdcon1));
	save_item(NAME(m_lcd.regs.lcdcon2));
	save_item(NAME(m_lcd.regs.lcdsaddr1));
	save_item(NAME(m_lcd.regs.lcdsaddr2));
	save_item(NAME(m_lcd.regs.lcdsaddr3));
	save_item(NAME(m_lcd.regs.redlut));
	save_item(NAME(m_lcd.regs.greenlut));
	save_item(NAME(m_lcd.regs.bluelut));
	save_item(NAME(m_lcd.regs.reserved));
	save_item(NAME(m_lcd.regs.lcdcon3));
	save_item(NAME(m_lcd.regs.dithmode));
	save_item(NAME(m_lcd.vramaddr_cur));
	save_item(NAME(m_lcd.vramaddr_max));
	save_item(NAME(m_lcd.offsize));
	save_item(NAME(m_lcd.pagewidth_cur));
	save_item(NAME(m_lcd.pagewidth_max));
	save_item(NAME(m_lcd.modesel));
	save_item(NAME(m_lcd.bswp));
	save_item(NAME(m_lcd.vpos));
	save_item(NAME(m_lcd.hpos));
	save_item(NAME(m_lcd.framerate));
	save_item(NAME(m_lcd.hpos_min));
	save_item(NAME(m_lcd.hpos_max));
	save_item(NAME(m_lcd.hpos_end));
	save_item(NAME(m_lcd.vpos_min));
	save_item(NAME(m_lcd.vpos_max));
	save_item(NAME(m_lcd.vpos_end));
	save_item(NAME(m_lcd.frame_time));

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_zdma[i].regs.dcon), i);
		save_item(NAME(m_zdma[i].regs.disrc), i);
		save_item(NAME(m_zdma[i].regs.didst), i);
		save_item(NAME(m_zdma[i].regs.dicnt), i);
		save_item(NAME(m_zdma[i].regs.dcsrc), i);
		save_item(NAME(m_zdma[i].regs.dcdst), i);
		save_item(NAME(m_zdma[i].regs.dccnt), i);

		save_item(NAME(m_bdma[i].regs.dcon), i);
		save_item(NAME(m_bdma[i].regs.disrc), i);
		save_item(NAME(m_bdma[i].regs.didst), i);
		save_item(NAME(m_bdma[i].regs.dicnt), i);
		save_item(NAME(m_bdma[i].regs.dcsrc), i);
		save_item(NAME(m_bdma[i].regs.dcdst), i);
		save_item(NAME(m_bdma[i].regs.dccnt), i);

		save_item(NAME(m_uart[i].regs.ulcon), i);
		save_item(NAME(m_uart[i].regs.ucon), i);
		save_item(NAME(m_uart[i].regs.ufcon), i);
		save_item(NAME(m_uart[i].regs.umcon), i);
		save_item(NAME(m_uart[i].regs.utrstat), i);
		save_item(NAME(m_uart[i].regs.uerstat), i);
		save_item(NAME(m_uart[i].regs.ufstat), i);
		save_item(NAME(m_uart[i].regs.umstat), i);
		save_item(NAME(m_uart[i].regs.utxh), i);
		save_item(NAME(m_uart[i].regs.urxh), i);
		save_item(NAME(m_uart[i].regs.ubrdiv), i);
	}

	save_item(NAME(m_sio.regs.siocon));
	save_item(NAME(m_sio.regs.siodat));
	save_item(NAME(m_sio.regs.sbrdr));
	save_item(NAME(m_sio.regs.itvcnt));
	save_item(NAME(m_sio.regs.dcntz));

	save_item(NAME(m_pwm.regs.tcfg0));
	save_item(NAME(m_pwm.regs.tcfg1));
	save_item(NAME(m_pwm.regs.tcon));
	save_item(NAME(m_pwm.regs.tcntb0));
	save_item(NAME(m_pwm.regs.tcmpb0));
	save_item(NAME(m_pwm.regs.tcnto0));
	save_item(NAME(m_pwm.regs.tcntb1));
	save_item(NAME(m_pwm.regs.tcmpb1));
	save_item(NAME(m_pwm.regs.tcnto1));
	save_item(NAME(m_pwm.regs.tcntb2));
	save_item(NAME(m_pwm.regs.tcmpb2));
	save_item(NAME(m_pwm.regs.tcnto2));
	save_item(NAME(m_pwm.regs.tcntb3));
	save_item(NAME(m_pwm.regs.tcmpb3));
	save_item(NAME(m_pwm.regs.tcnto3));
	save_item(NAME(m_pwm.regs.tcntb4));
	save_item(NAME(m_pwm.regs.tcmpb4));
	save_item(NAME(m_pwm.regs.tcnto4));
	save_item(NAME(m_pwm.regs.tcntb5));
	save_item(NAME(m_pwm.regs.tcnto5));
	save_item(NAME(m_pwm.cnt));
	save_item(NAME(m_pwm.cmp));
	save_item(NAME(m_pwm.freq));

	save_item(NAME(m_wdt.regs.wtcon));
	save_item(NAME(m_wdt.regs.wtdat));
	save_item(NAME(m_wdt.regs.wtcnt));

	save_item(NAME(m_iic.regs.iiccon));
	save_item(NAME(m_iic.regs.iicstat));
	save_item(NAME(m_iic.regs.iicadd));
	save_item(NAME(m_iic.regs.iicds));
	save_item(NAME(m_iic.count));

	save_item(NAME(m_iis.regs.iiscon));
	save_item(NAME(m_iis.regs.iismod));
	save_item(NAME(m_iis.regs.iispsr));
	save_item(NAME(m_iis.regs.iisfcon));
	save_item(NAME(m_iis.regs.iisfifo));
	save_item(NAME(m_iis.fifo));
	save_item(NAME(m_iis.fifo_index));

	save_item(NAME(m_gpio.regs.gpacon));
	save_item(NAME(m_gpio.regs.gpadat));
	save_item(NAME(m_gpio.regs.gpbcon));
	save_item(NAME(m_gpio.regs.gpbdat));
	save_item(NAME(m_gpio.regs.gpccon));
	save_item(NAME(m_gpio.regs.gpcdat));
	save_item(NAME(m_gpio.regs.gpcup));
	save_item(NAME(m_gpio.regs.gpdcon));
	save_item(NAME(m_gpio.regs.gpddat));
	save_item(NAME(m_gpio.regs.gpdup));
	save_item(NAME(m_gpio.regs.gpecon));
	save_item(NAME(m_gpio.regs.gpedat));
	save_item(NAME(m_gpio.regs.gpeup));
	save_item(NAME(m_gpio.regs.gpfcon));
	save_item(NAME(m_gpio.regs.gpfdat));
	save_item(NAME(m_gpio.regs.gpfup));
	save_item(NAME(m_gpio.regs.gpgcon));
	save_item(NAME(m_gpio.regs.gpgdat));
	save_item(NAME(m_gpio.regs.gpgup));
	save_item(NAME(m_gpio.regs.spucr));
	save_item(NAME(m_gpio.regs.extint));
	save_item(NAME(m_gpio.regs.extintpnd));

	save_item(NAME(m_adc.regs.adccon));
	save_item(NAME(m_adc.regs.adcpsr));
	save_item(NAME(m_adc.regs.adcdat));

	save_item(NAME(m_cpuwrap.regs.syscfg));
	save_item(NAME(m_cpuwrap.regs.ncachbe0));
	save_item(NAME(m_cpuwrap.regs.ncachbe1));
}


//-------------------------------------------------
//  device_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expaneded as necessary
//-------------------------------------------------

void s3c44b0_device::device_post_load()
{
	m_lcd.frame_period = HZ_TO_ATTOSECONDS(m_lcd.framerate);
	m_lcd.scantime = m_lcd.frame_period / m_lcd.vpos_end;
	m_lcd.pixeltime = m_lcd.frame_period / (m_lcd.vpos_end * m_lcd.hpos_end);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3c44b0_device::device_reset()
{
	m_iis.fifo_index = 0;
	//  m_iic.data_index = 0;
#if defined(DEVICE_S3C2410) || defined(DEVICE_S3C2440)
	m_gpio.regs.gstatus2 = 0x00000001; // Boot is caused by power on reset
#endif
	m_irq.line_irq = m_irq.line_fiq = CLEAR_LINE;
}


/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

#define UART_PRINTF

#define CLOCK_MULTIPLIER 1

#define BITS(x,m,n) (((x)>>(n))&(((uint32_t)1<<((m)-(n)+1))-1))
#define CLR_BITS(x,m,n) ((x) & ~((((uint32_t)1 << ((m) - (n) + 1)) - 1) << n))


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* LCD Controller */

rgb_t s3c44b0_device::lcd_get_color_stn_04(uint8_t data)
{
	uint8_t r, g, b;
	r = g = b = BITS(data, 3, 0) << 4;
	return rgb_t(r, g, b);
}

uint8_t s3c44b0_device::lcd_get_color_stn_08_r(uint8_t data)
{
	return ((m_lcd.regs.redlut >> (BITS(data, 7, 5) << 2)) & 0xf) << 4;
}

uint8_t s3c44b0_device::lcd_get_color_stn_08_g(uint8_t data)
{
	return ((m_lcd.regs.greenlut >> (BITS(data, 4, 2) << 2)) & 0xf) << 4;
}

uint8_t s3c44b0_device::lcd_get_color_stn_08_b(uint8_t data)
{
	return ((m_lcd.regs.bluelut >> (BITS(data, 1, 0) << 2)) & 0xf) << 4;
}

void s3c44b0_device::lcd_dma_reload()
{
	int lcdbank, lcdbaseu, lcdbasel;
	lcdbank  = BITS(m_lcd.regs.lcdsaddr1, 26, 21);
	lcdbaseu = BITS(m_lcd.regs.lcdsaddr1, 20, 0);
	lcdbasel = BITS(m_lcd.regs.lcdsaddr2, 20, 0);

	m_lcd.vramaddr_cur = (lcdbank << 22) | (lcdbaseu << 1);
	m_lcd.vramaddr_max = (lcdbank << 22) | (lcdbasel << 1);
	if (lcdbasel == 0) m_lcd.vramaddr_max += 1 << 22;
	m_lcd.offsize = BITS(m_lcd.regs.lcdsaddr3, 19, 9);
	m_lcd.pagewidth_cur = 0;
	m_lcd.pagewidth_max = BITS(m_lcd.regs.lcdsaddr3, 8, 0);
	m_lcd.bswp = BIT(m_lcd.regs.lcdsaddr2, 29); // note: juicebox changes bswp when video playback starts
//  verboselog( *this, 3, "LCD - vramaddr %08X %08X offsize %08X pagewidth %08X\n", m_lcd.vramaddr_cur, m_lcd.vramaddr_max, m_lcd.offsize, m_lcd.pagewidth_max);
}

void s3c44b0_device::lcd_dma_init()
{
	m_lcd.modesel = BITS(m_lcd.regs.lcdsaddr1, 28, 27);
//  verboselog( *this, 3, "LCD - modesel %d bswp %d\n", m_lcd.modesel, m_lcd.bswp);
	lcd_dma_reload();
}

void s3c44b0_device::lcd_dma_read(int count, uint8_t *data)
{
	for (int i = 0; i < count / 2; i++)
	{
		if (m_lcd.bswp == 0)
		{
			if ((m_lcd.vramaddr_cur & 2) == 0)
			{
				data[0] = m_cache.read_byte(m_lcd.vramaddr_cur + 3);
				data[1] = m_cache.read_byte(m_lcd.vramaddr_cur + 2);
			}
			else
			{
				data[0] = m_cache.read_byte(m_lcd.vramaddr_cur - 1);
				data[1] = m_cache.read_byte(m_lcd.vramaddr_cur - 2);
			}
		}
		else
		{
			data[0] = m_cache.read_byte(m_lcd.vramaddr_cur + 0);
			data[1] = m_cache.read_byte(m_lcd.vramaddr_cur + 1);
		}
		m_lcd.vramaddr_cur += 2;
		m_lcd.pagewidth_cur++;
		if (m_lcd.pagewidth_cur >= m_lcd.pagewidth_max)
		{
			m_lcd.vramaddr_cur += m_lcd.offsize << 1;
			if (m_lcd.vramaddr_cur >= m_lcd.vramaddr_max)
			{
				lcd_dma_reload();
			}
			m_lcd.pagewidth_cur = 0;
		}
		data += 2;
	}
}

void s3c44b0_device::lcd_render_stn_04()
{
	uint8_t *bitmap = m_lcd.bitmap.get() + ((m_lcd.vpos - m_lcd.vpos_min) * (m_lcd.hpos_max - m_lcd.hpos_min + 1)) + (m_lcd.hpos - m_lcd.hpos_min);
	uint8_t data[16];
	lcd_dma_read(16, data);
	for (auto & elem : data)
	{
		for (int j = 0; j < 2; j++)
		{
			*bitmap++ = lcd_get_color_stn_04((elem >> 4) & 0x0F);
			elem = elem << 4;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max << 2))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max)
				{
					m_lcd.vpos = m_lcd.vpos_min;
					bitmap = m_lcd.bitmap.get();
				}
				m_lcd.hpos = m_lcd.hpos_min;
			}
		}
	}
}

void s3c44b0_device::lcd_render_stn_08()
{
	uint8_t *bitmap = m_lcd.bitmap.get() + ((m_lcd.vpos - m_lcd.vpos_min) * (m_lcd.hpos_max - m_lcd.hpos_min + 1)) + (m_lcd.hpos - m_lcd.hpos_min);
	uint8_t data[16];
	lcd_dma_read(16, data);
	for (auto & elem : data)
	{
		uint8_t xxx[3];
		xxx[0] = lcd_get_color_stn_08_r(elem);
		xxx[1] = lcd_get_color_stn_08_g(elem);
		xxx[2] = lcd_get_color_stn_08_b(elem);
		for (auto & xxx_j : xxx)
		{
			*bitmap++ = xxx_j;
			m_lcd.hpos++;
			if (m_lcd.hpos >= m_lcd.hpos_min + (m_lcd.pagewidth_max * 6))
			{
				m_lcd.vpos++;
				if (m_lcd.vpos > m_lcd.vpos_max)
				{
					m_lcd.vpos = m_lcd.vpos_min;
					bitmap = m_lcd.bitmap.get();
				}
				m_lcd.hpos = m_lcd.hpos_min;
			}
		}
	}
}

attotime s3c44b0_device::time_until_pos(int vpos, int hpos)
{
	attoseconds_t time1, time2;
	attotime retval;
	verboselog( *this, 3, "s3c44b0_time_until_pos - vpos %d hpos %d\n", vpos, hpos);
	time1 = (attoseconds_t)vpos * m_lcd.scantime + (attoseconds_t)hpos * m_lcd.pixeltime;
	time2 = (machine().time() - m_lcd.frame_time).as_attoseconds();
	verboselog( *this, 3, "machine %f frametime %f time1 %f time2 %f\n", machine().time().as_double(), m_lcd.frame_time.as_double(), attotime(0, time1).as_double(), attotime(0, time2).as_double());
	while (time1 <= time2) time1 += m_lcd.frame_period;
	retval = attotime( 0, time1 - time2);
	verboselog( *this, 3, "result %f\n", retval.as_double());
	return retval;
}

int s3c44b0_device::lcd_get_vpos()
{
	attoseconds_t delta;
	int vpos;
	delta = (machine().time() - m_lcd.frame_time).as_attoseconds();
	delta = delta + (m_lcd.pixeltime / 2);
	vpos = delta / m_lcd.scantime;
	return (m_lcd.vpos_min + vpos) % m_lcd.vpos_end;
}

int s3c44b0_device::lcd_get_hpos()
{
	attoseconds_t delta;
	int vpos;
	delta = (machine().time() - m_lcd.frame_time).as_attoseconds();
	delta = delta + (m_lcd.pixeltime / 2);
	vpos = delta / m_lcd.scantime;
	delta = delta - (vpos * m_lcd.scantime);
	return delta / m_lcd.pixeltime;
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::lcd_timer_exp )
{
	int vpos = m_lcd.vpos;
	verboselog( *this, 2, "LCD timer callback (%f)\n", machine().time().as_double());
	verboselog( *this, 3, "LCD - (1) vramaddr %08X vpos %d hpos %d\n", m_lcd.vramaddr_cur, m_lcd.vpos, m_lcd.hpos);
	switch (m_lcd.modesel)
	{
		case S3C44B0_MODESEL_04 : lcd_render_stn_04(); break;
		case S3C44B0_MODESEL_08 : lcd_render_stn_08(); break;
		default : verboselog( *this, 0, "s3c44b0_lcd_timer_exp: modesel %d not supported\n", m_lcd.modesel); break;
	}
	verboselog( *this, 3, "LCD - (2) vramaddr %08X vpos %d hpos %d\n", m_lcd.vramaddr_cur, m_lcd.vpos, m_lcd.hpos);
	if (m_lcd.vpos < vpos)
	{
//      verboselog( *this, 3, "LCD - (1) frame_time %f\n", attotime_to_double(m_lcd.frame_time));
		m_lcd.frame_time = machine().time() + time_until_pos(m_lcd.vpos_min, m_lcd.hpos_min);
//      verboselog( *this, 3, "LCD - (2) frame_time %f\n", attotime_to_double(m_lcd.frame_time));
	}
	m_lcd.timer->adjust(time_until_pos(m_lcd.vpos, m_lcd.hpos), 0);
}

void s3c44b0_device::video_start()
{
	// do nothing
}

uint32_t s3c44b0_device::video_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_lcd.regs.lcdcon1 & (1 << 0))
	{
		if (m_lcd.bitmap)
		{
			for (int y = 0; y < screen.height(); y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				uint8_t const *vram = m_lcd.bitmap.get() + y * (m_lcd.hpos_max - m_lcd.hpos_min + 1);
				for (int x = 0; x < screen.width(); x++)
				{
					*scanline++ = rgb_t(vram[0], vram[1], vram[2]);
					vram += 3;
				}
			}
		}
	}
	else
	{
		for (int y = 0; y < screen.height(); y++)
		{
			std::fill_n(&bitmap.pix(y), screen.width(), 0);
		}
	}
	return 0;
}

uint32_t s3c44b0_device::lcd_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_lcd.regs)[offset];
	switch (offset)
	{
		case S3C44B0_LCDCON1 :
		{
			int vpos = 0;
			// make sure line counter is going
			if (m_lcd.regs.lcdcon1 & (1 << 0))
			{
				vpos = lcd_get_vpos();
				int hpos = lcd_get_hpos();
				if (hpos < m_lcd.hpos_min) vpos = vpos - 1;
				if ((vpos < m_lcd.vpos_min) || (vpos > m_lcd.vpos_max)) vpos = m_lcd.vpos_max;
				vpos = m_lcd.vpos_max - vpos;
			}
			data = (data & ~0xFFC00000) | (vpos << 22);
		}
		break;
	}
//  verboselog( *this, 9, "(LCD) %08X -> %08X\n", S3C44B0_BASE_LCD + (offset << 2), data);
	return data;
}

void s3c44b0_device::lcd_configure()
{
	int dismode, clkval, lineval, wdly, hozval, lineblank, wlh, mclk;
	double vclk, framerate;
	int width, height;
	verboselog( *this, 5, "s3c44b0_lcd_configure\n");
	dismode = BITS(m_lcd.regs.lcdcon1, 6, 5);
	clkval = BITS(m_lcd.regs.lcdcon1, 21, 12);
	lineval = BITS(m_lcd.regs.lcdcon2, 9, 0);
	wdly = BITS(m_lcd.regs.lcdcon1, 9, 8);
	hozval = BITS(m_lcd.regs.lcdcon2, 20, 10);
	lineblank = BITS(m_lcd.regs.lcdcon2, 31, 21);
	wlh = BITS(m_lcd.regs.lcdcon1, 11, 10);
	mclk = get_mclk();
	verboselog( *this, 3, "LCD - dismode %d clkval %d lineval %d wdly %d hozval %d lineblank %d wlh %d mclk %d\n", dismode, clkval, lineval, wdly, hozval, lineblank, wlh, mclk);
	vclk = (double)(mclk / (clkval * 2));
	verboselog( *this, 3, "LCD - vclk %f\n", vclk);
	framerate = 1 / (((1 / vclk) * (hozval + 1) + (1 / mclk) * (wlh + wdly + lineblank)) * (lineval + 1));
	framerate = framerate / 3; // ???
	verboselog( *this, 3, "LCD - framerate %f\n", framerate);
	switch (dismode)
	{
		case S3C44B0_PNRMODE_STN_04_SS : width = ((hozval + 1) * 4); break;
		case S3C44B0_PNRMODE_STN_04_DS : width = ((hozval + 1) * 4); break;
		case S3C44B0_PNRMODE_STN_08_SS : width = ((hozval + 1) * 8); break;
		default : fatalerror("invalid display mode (%d)\n", dismode);
	}
	height = lineval + 1;
	m_lcd.framerate = framerate;
	verboselog( *this, 3, "video_screen_configure %d %d %f\n", width, height, m_lcd.framerate);
	screen().configure(screen().width(), screen().height(), screen().visible_area(), HZ_TO_ATTOSECONDS(m_lcd.framerate));
	m_lcd.hpos_min = 25;
	m_lcd.hpos_max = 25 + width - 1;
	m_lcd.hpos_end = 25 + width - 1 + 25;
	m_lcd.vpos_min = 25;
	m_lcd.vpos_max = 25 + height - 1;
	m_lcd.vpos_end = 25 + height - 1 + 25;
	verboselog( *this, 3, "LCD - min_x %d min_y %d max_x %d max_y %d\n", m_lcd.hpos_min, m_lcd.vpos_min, m_lcd.hpos_max, m_lcd.vpos_max);
	if (m_lcd.bitmap)
	{
		m_lcd.bitmap = nullptr;
	}
	m_lcd.bitmap = std::make_unique<uint8_t[]>((m_lcd.hpos_max - m_lcd.hpos_min + 1) * (m_lcd.vpos_max - m_lcd.vpos_min + 1) * 3);
	m_lcd.frame_period = HZ_TO_ATTOSECONDS(m_lcd.framerate);
	m_lcd.scantime = m_lcd.frame_period / m_lcd.vpos_end;
	m_lcd.pixeltime = m_lcd.frame_period / (m_lcd.vpos_end * m_lcd.hpos_end);
//  printf("frame_period %f\n", attotime( 0, m_lcd.frame_period).as_double());
//  printf("scantime %f\n", attotime( 0, m_lcd.scantime).as_double());
//  printf("pixeltime %f\n", attotime( 0, m_lcd.pixeltime).as_double());
}


void s3c44b0_device::lcd_start()
{
	verboselog( *this, 1, "LCD start\n");
	lcd_configure();
	lcd_dma_init();
	m_lcd.vpos = m_lcd.vpos_min;
	m_lcd.hpos = m_lcd.hpos_min;
	m_lcd.frame_time = screen().time_until_pos( 0, 0);
	m_lcd.timer->adjust(m_lcd.frame_time, 0);
	m_lcd.frame_time = machine().time() + m_lcd.frame_time;
}

void s3c44b0_device::lcd_stop()
{
	verboselog( *this, 1, "LCD stop\n");
	m_lcd.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::lcd_recalc()
{
	if (m_lcd.regs.lcdcon1 & (1 << 0))
		lcd_start();
	else
		lcd_stop();
}

void s3c44b0_device::lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_lcd.regs)[offset];
//  verboselog( *this, 9, "(LCD) %08X <- %08X\n", S3C44B0_BASE_LCD + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_lcd.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_LCDCON1 :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				lcd_recalc();
			}
		}
		break;
	}
}


/* Clock & Power Management */

uint32_t s3c44b0_device::get_mclk()
{
	uint32_t data, mdiv, pdiv, sdiv;
	data = m_clkpow.regs.pllcon;
	mdiv = BITS(data, 19, 12);
	pdiv = BITS(data, 9, 4);
	sdiv = BITS(data, 1, 0);
	return (uint32_t)((double)((mdiv + 8) * clock()) / (double)((pdiv + 2) * (1 << sdiv)));
}

uint32_t s3c44b0_device::clkpow_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_clkpow.regs)[offset];
	verboselog( *this, 9, "(CLKPOW) %08X -> %08X\n", S3C44B0_BASE_CLKPOW + (offset << 2), data);
	return data;
}

void s3c44b0_device::clkpow_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(CLKPOW) %08X <- %08X\n", S3C44B0_BASE_CLKPOW + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_clkpow.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_PLLCON :
		{
			verboselog( *this, 5, "CLKPOW - mclk %d\n", get_mclk());
			m_cpu->set_unscaled_clock(get_mclk() * CLOCK_MULTIPLIER);
		}
		break;
		case S3C44B0_CLKCON :
		{
			if (data & (1 << 2))
			{
				m_cpu->spin_until_interrupt();
			}
		}
		break;
	}
}

/* Interrupt Controller */

void s3c44b0_device::check_pending_irq()
{
	// normal irq
	uint32_t temp = (m_irq.regs.intpnd & ~m_irq.regs.intmsk) & ~m_irq.regs.intmod;

	if (temp != 0)
	{
		uint32_t int_type = 0;
		while ((temp & 1) == 0)
		{
			int_type++;
			temp = temp >> 1;
		}
		m_irq.regs.i_ispr |= (1 << int_type);
		if (m_irq.line_irq != ASSERT_LINE)
		{
			m_cpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
			m_irq.line_irq = ASSERT_LINE;
		}
	}
	else
	{
		if (m_irq.line_irq != CLEAR_LINE)
		{
			m_cpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
			m_irq.line_irq = CLEAR_LINE;
		}
	}
	// fast irq
	temp = (m_irq.regs.intpnd & ~m_irq.regs.intmsk) & m_irq.regs.intmod;
	if (temp != 0)
	{
		if (m_irq.line_fiq != ASSERT_LINE)
		{
			m_cpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, ASSERT_LINE);
			m_irq.line_fiq = ASSERT_LINE;
		}
	}
	else
	{
		if (m_irq.line_fiq != CLEAR_LINE)
		{
			m_cpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, CLEAR_LINE);
			m_irq.line_fiq = CLEAR_LINE;
		}
	}
}

void s3c44b0_device::request_irq(uint32_t int_type)
{
	verboselog( *this, 5, "request irq %d\n", int_type);
	m_irq.regs.intpnd |= (1 << int_type);
	check_pending_irq();
}

void s3c44b0_device::check_pending_eint()
{
	if (m_gpio.regs.extintpnd != 0)
		request_irq(S3C44B0_INT_EINT4_7);
}

void s3c44b0_device::request_eint(uint32_t number)
{
	verboselog( *this, 5, "request external interrupt %d\n", number);
	if (number < 4)
	{
		request_irq(S3C44B0_INT_EINT0 + number);
	}
	else
	{
		m_gpio.regs.extintpnd |= (1 << (number - 4));
		check_pending_eint();
	}
}

uint32_t s3c44b0_device::irq_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_irq.regs)[offset];
	verboselog( *this, 9, "(IRQ) %08X -> %08X\n", S3C44B0_BASE_INT + (offset << 2), data);
	return data;
}

void s3c44b0_device::irq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(IRQ) %08X <- %08X\n", S3C44B0_BASE_INT + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_irq.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_INTMSK :
		{
			check_pending_irq();
		}
		break;
		case S3C44B0_I_ISPC :
		{
			m_irq.regs.intpnd = (m_irq.regs.intpnd & ~data); // The bit of INTPND bit is cleared to zero by writing '1' on I_ISPC/F_ISPC
			m_irq.regs.i_ispr = (m_irq.regs.i_ispr & ~data); // The pending bit in I_ISPR register should be cleared by writing I_ISPC register.
			check_pending_irq();
		}
		break;
		case S3C44B0_F_ISPC :
		{
			m_irq.regs.intpnd = (m_irq.regs.intpnd & ~data); // The bit of INTPND bit is cleared to zero by writing '1' on I_ISPC/F_ISPC
			check_pending_irq();
		}
		break;
	}
}

/* PWM Timer */

uint16_t s3c44b0_device::pwm_calc_observation(int ch)
{
	double timeleft, x1, x2;
	uint32_t cnto;
	timeleft = (m_pwm.timer[ch]->remaining()).as_double();
//  printf( "timeleft %f freq %d cntb %d cmpb %d\n", timeleft, m_pwm.freq[ch], m_pwm.cnt[ch], m_pwm.cmp[ch]);
	x1 = 1 / ((double)m_pwm.freq[ch] / (m_pwm.cnt[ch]- m_pwm.cmp[ch] + 1));
	x2 = x1 / timeleft;
//  printf( "x1 %f\n", x1);
	cnto = m_pwm.cmp[ch] + ((m_pwm.cnt[ch]- m_pwm.cmp[ch]) / x2);
//  printf( "cnto %d\n", cnto);
	return cnto;
}

uint32_t s3c44b0_device::pwm_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_pwm.regs)[offset];
	switch (offset)
	{
		case S3C44B0_TCNTO0 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(0);
		}
		break;
		case S3C44B0_TCNTO1 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(1);
		}
		break;
		case S3C44B0_TCNTO2 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(2);
		}
		break;
		case S3C44B0_TCNTO3 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(3);
		}
		break;
		case S3C44B0_TCNTO4 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(4);
		}
		break;
		case S3C44B0_TCNTO5 :
		{
			data = (data & ~0x0000FFFF) | pwm_calc_observation(5);
		}
		break;
	}
	verboselog( *this, 9, "(PWM) %08X -> %08X\n", S3C44B0_BASE_PWM + (offset << 2), data);
	return data;
}

void s3c44b0_device::pwm_start(int timer)
{
	const int mux_table[] = { 2, 4, 8, 16};
	const int prescaler_shift[] = { 0, 0, 8, 8, 16, 16};
	const int mux_shift[] = { 0, 4, 8, 12, 16, 20};
	uint32_t mclk, prescaler, mux, cnt, cmp, auto_reload;
	double freq, hz;
	verboselog( *this, 1, "PWM %d start\n", timer);
	mclk = get_mclk();
	prescaler = (m_pwm.regs.tcfg0 >> prescaler_shift[timer]) & 0xFF;
	mux = (m_pwm.regs.tcfg1 >> mux_shift[timer]) & 0x0F;
	if (mux < 4)
	{
		freq = (double)mclk / (prescaler + 1) / mux_table[mux];
	}
	else
	{
		// todo
		freq = (double)mclk / (prescaler + 1) / 1;
	}
	switch (timer)
	{
		case 0 :
		{
			cnt = BITS(m_pwm.regs.tcntb0, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb0, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 3);
		}
		break;
		case 1 :
		{
			cnt = BITS(m_pwm.regs.tcntb1, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb1, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 11);
		}
		break;
		case 2 :
		{
			cnt = BITS(m_pwm.regs.tcntb2, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb2, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 15);
		}
		break;
		case 3 :
		{
			cnt = BITS(m_pwm.regs.tcntb3, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb3, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 19);
		}
		break;
		case 4 :
		{
			cnt = BITS(m_pwm.regs.tcntb4, 15, 0);
			cmp = BITS(m_pwm.regs.tcmpb4, 15, 0);
			auto_reload = BIT(m_pwm.regs.tcon, 23);
		}
		break;
		case 5 :
		{
			cnt = BITS(m_pwm.regs.tcntb5, 15, 0);
			cmp = 0;
			auto_reload = BIT(m_pwm.regs.tcon, 26);
		}
		break;
		default :
		{
			fatalerror("Invalid timer index %d!", timer);
		}
	}
//  hz = freq / (cnt - cmp + 1);
	if (cnt < 2)
	{
		hz = freq;
	}
	else
	{
		hz = freq / cnt;
	}
	verboselog( *this, 5, "PWM %d - mclk=%d prescaler=%d div=%d freq=%f cnt=%d cmp=%d auto_reload=%d hz=%f\n", timer, mclk, prescaler, mux_table[mux], freq, cnt, cmp, auto_reload, hz);
	m_pwm.cnt[timer] = cnt;
	m_pwm.cmp[timer] = cmp;
	m_pwm.freq[timer] = freq;
	if (cnt == 0)
	{
		m_pwm.timer[timer]->adjust(attotime::never, 0);
	}
	else
	{
		if (auto_reload)
		{
			m_pwm.timer[timer]->adjust(attotime::from_hz(hz), timer, attotime::from_hz(hz));
		}
		else
		{
			m_pwm.timer[timer]->adjust(attotime::from_hz(hz), timer);
		}
	}
}

void s3c44b0_device::pwm_stop(int timer)
{
	verboselog( *this, 1, "PWM %d stop\n", timer);
	m_pwm.timer[timer]->adjust(attotime::never, 0);
}

void s3c44b0_device::pwm_recalc(int timer)
{
	const int tcon_shift[] = { 0, 8, 12, 16, 20, 24};
	if (m_pwm.regs.tcon & (1 << tcon_shift[timer]))
		pwm_start(timer);
	else
		pwm_stop(timer);
}

void s3c44b0_device::pwm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_pwm.regs)[offset];
	verboselog( *this, 9, "(PWM) %08X <- %08X\n", S3C44B0_BASE_PWM + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_pwm.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_TCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				pwm_recalc(0);
			}
			if ((data & (1 << 8)) != (old_value & (1 << 8)))
			{
				pwm_recalc(1);
			}
			if ((data & (1 << 12)) != (old_value & (1 << 12)))
			{
				pwm_recalc(2);
			}
			if ((data & (1 << 16)) != (old_value & (1 << 16)))
			{
				pwm_recalc(3);
			}
			if ((data & (1 << 20)) != (old_value & (1 << 20)))
			{
				pwm_recalc(4);
			}
			if ((data & (1 << 24)) != (old_value & (1 << 24)))
			{
				pwm_recalc(5);
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::pwm_timer_exp )
{
	int ch = param;
	const int ch_int[] = { S3C44B0_INT_TIMER0, S3C44B0_INT_TIMER1, S3C44B0_INT_TIMER2, S3C44B0_INT_TIMER3, S3C44B0_INT_TIMER4, S3C44B0_INT_TIMER5 };
	verboselog( *this, 2, "PWM %d timer callback\n", ch);
	if (BITS(m_pwm.regs.tcfg1, 27, 24) == (ch + 1))
	{
		fatalerror("s3c44b0_dma_request_pwm( device)\n");
	}
	else
	{
		request_irq(ch_int[ch]);
	}
}

/* IIC */

inline void s3c44b0_device::iface_i2c_scl_w(int state)
{
	m_scl_w_cb(state);
}

inline void s3c44b0_device::iface_i2c_sda_w(int state)
{
	m_sda_w_cb(state);
}

inline int s3c44b0_device::iface_i2c_sda_r()
{
	return m_sda_r_cb();
}

void s3c44b0_device::i2c_send_start()
{
	verboselog( *this, 5, "i2c_send_start\n");
	iface_i2c_sda_w(1);
	iface_i2c_scl_w(1);
	iface_i2c_sda_w(0);
	iface_i2c_scl_w(0);
}

void s3c44b0_device::i2c_send_stop()
{
	verboselog( *this, 5, "i2c_send_stop\n");
	iface_i2c_sda_w(0);
	iface_i2c_scl_w(1);
	iface_i2c_sda_w(1);
	iface_i2c_scl_w(0);
}

uint8_t s3c44b0_device::i2c_receive_byte(int ack)
{
	uint8_t data = 0;
	verboselog( *this, 5, "i2c_receive_byte ...\n");
	iface_i2c_sda_w(1);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_scl_w(1);
		data = (data << 1) + (iface_i2c_sda_r() ? 1 : 0);
		iface_i2c_scl_w(0);
	}
	verboselog( *this, 5, "recv data %02X\n", data);
	verboselog( *this, 5, "send ack %d\n", ack);
	iface_i2c_sda_w(ack ? 0 : 1);
	iface_i2c_scl_w(1);
	iface_i2c_scl_w(0);
	return data;
}

int s3c44b0_device::i2c_send_byte(uint8_t data)
{
	int ack;
	verboselog( *this, 5, "i2c_send_byte ...\n");
	verboselog( *this, 5, "send data %02X\n", data);
	for (int i = 0; i < 8; i++)
	{
		iface_i2c_sda_w((data & 0x80) ? 1 : 0);
		data = data << 1;
		iface_i2c_scl_w(1);
		iface_i2c_scl_w(0);
	}
	iface_i2c_sda_w(1); // ack bit
	iface_i2c_scl_w(1);
	ack = iface_i2c_sda_r();
	verboselog( *this, 5, "recv ack %d\n", ack);
	iface_i2c_scl_w(0);
	return ack;
}

void s3c44b0_device::iic_start()
{
	int mode_selection;
	verboselog( *this, 1, "IIC start\n");
	i2c_send_start();
	mode_selection = BITS(m_iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : i2c_send_byte(m_iic.regs.iicds | 0x01); break;
		case 3 : i2c_send_byte(m_iic.regs.iicds & 0xFE); break;
	}
	m_iic.timer->adjust(attotime::from_usec( 1), 0);
}

void s3c44b0_device::iic_stop()
{
	verboselog( *this, 1, "IIC stop\n");
	i2c_send_stop();
	m_iic.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::iic_resume()
{
	int mode_selection;
	verboselog( *this, 1, "IIC resume\n");
	mode_selection = BITS(m_iic.regs.iicstat, 7, 6);
	switch (mode_selection)
	{
		case 2 : m_iic.regs.iicds = i2c_receive_byte(BIT(m_iic.regs.iiccon, 7)); break;
		case 3 : i2c_send_byte(m_iic.regs.iicds & 0xFF); break;
	}
	m_iic.timer->adjust(attotime::from_usec( 1), 0);
}

uint32_t s3c44b0_device::iic_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_iic.regs)[offset];
	switch (offset)
	{
		case S3C44B0_IICSTAT :
		{
			data = data & ~0x0000000F;
		}
		break;
	}
	verboselog( *this, 9, "(IIC) %08X -> %08X\n", S3C44B0_BASE_IIC + (offset << 2), data);
	return data;
}

void s3c44b0_device::iic_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_iic.regs)[offset];
	verboselog( *this, 9, "(IIC) %08X <- %08X\n", S3C44B0_BASE_IIC + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_iic.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_IICCON :
		{
			int interrupt_pending_flag;
#if 0
			const int div_table[] = { 16, 512};
			int enable_interrupt, transmit_clock_value, tx_clock_source_selection
			double clock;
			transmit_clock_value = (data >> 0) & 0xF;
			tx_clock_source_selection = (data >> 6) & 1;
			enable_interrupt = (data >> 5) & 1;
			clock = (double)get_pclk() / div_table[tx_clock_source_selection] / (transmit_clock_value + 1);
#endif
			interrupt_pending_flag = BIT(old_value, 4);
			if (interrupt_pending_flag != 0)
			{
				interrupt_pending_flag = BIT(data, 4);
				if (interrupt_pending_flag == 0)
				{
					int start_stop_condition;
					start_stop_condition = BIT(m_iic.regs.iicstat, 5);
					if (start_stop_condition != 0)
					{
						if (m_iic.count == 0)
						{
							iic_start();

						}
						else
						{
							iic_resume();
						}
					}
					else
					{
						iic_stop();
					}
				}
			}
		}
		break;
		case  S3C44B0_IICSTAT :
		{
			int interrupt_pending_flag;
			m_iic.count = 0;
			interrupt_pending_flag = BIT(m_iic.regs.iiccon, 4);
			if (interrupt_pending_flag == 0)
			{
				int start_stop_condition;
				start_stop_condition = BIT(data, 5);
				if (start_stop_condition != 0)
				{
					if (m_iic.count == 0)
					{
						iic_start();

					}
					else
					{
						iic_resume();
					}
				}
				else
				{
					iic_stop();
				}
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::iic_timer_exp )
{
	int enable_interrupt;
	verboselog( *this, 2, "IIC timer callback\n");
	m_iic.count++;
	enable_interrupt = BIT(m_iic.regs.iiccon, 5);

	m_iic.regs.iicds = 0xFF; // TEST

	if (enable_interrupt)
	{
		m_iic.regs.iiccon |= (1 << 4); // [bit 4] interrupt is pending
		request_irq(S3C44B0_INT_IIC);
	}
}

/* I/O Port */

inline uint32_t s3c44b0_device::iface_gpio_port_r(int port)
{
	return m_port_r_cb(port);
}

inline void s3c44b0_device::iface_gpio_port_w(int port, uint32_t data)
{
	m_port_w_cb(port, data, 0xffff);
}

uint32_t s3c44b0_device::gpio_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_gpio.regs)[offset];
	switch (offset)
	{
		case S3C44B0_GPADAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_A) & S3C44B0_GPADAT_MASK;
		}
		break;
		case S3C44B0_GPBDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_B) & S3C44B0_GPBDAT_MASK;
		}
		break;
		case S3C44B0_GPCDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_C) & S3C44B0_GPCDAT_MASK;
		}
		break;
		case S3C44B0_GPDDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_D) & S3C44B0_GPDDAT_MASK;
		}
		break;
		case S3C44B0_GPEDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_E) & S3C44B0_GPEDAT_MASK;
		}
		break;
		case S3C44B0_GPFDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_F) & S3C44B0_GPFDAT_MASK;
		}
		break;
		case S3C44B0_GPGDAT :
		{
			data = iface_gpio_port_r(S3C44B0_GPIO_PORT_G) & S3C44B0_GPGDAT_MASK;
		}
		break;
	}
	verboselog( *this, 9, "(GPIO) %08X -> %08X\n", S3C44B0_BASE_GPIO + (offset << 2), data);
	return data;
}

void s3c44b0_device::gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_gpio.regs)[offset];
	verboselog( *this, 9, "(GPIO) %08X <- %08X\n", S3C44B0_BASE_GPIO + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_gpio.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_GPADAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_A, data & S3C44B0_GPADAT_MASK);
		}
		break;
		case S3C44B0_GPBDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_B, data & S3C44B0_GPBDAT_MASK);
		}
		break;
		case S3C44B0_GPCDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_C, data & S3C44B0_GPCDAT_MASK);
		}
		break;
		case S3C44B0_GPDDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_D, data & S3C44B0_GPDDAT_MASK);
		}
		break;
		case S3C44B0_GPEDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_E, data & S3C44B0_GPEDAT_MASK);
		}
		break;
		case S3C44B0_GPFDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_F, data & S3C44B0_GPFDAT_MASK);
		}
		break;
		case S3C44B0_GPGDAT :
		{
				iface_gpio_port_w(S3C44B0_GPIO_PORT_G, data & S3C44B0_GPGDAT_MASK);
		}
		break;
		case S3C44B0_EXTINTPND :
		{
			m_gpio.regs.extintpnd = (old_value & ~data);
			check_pending_eint();
		}
		break;
	}
}

/* UART */

uint32_t s3c44b0_device::uart_r(int ch, uint32_t offset)
{
	uint32_t data = ((uint32_t*)&m_uart[ch].regs)[offset];
	switch (offset)
	{
		case S3C44B0_UTRSTAT :
		{
			data = (data & ~0x00000006) | 0x00000004 | 0x00000002; // [bit 2] Transmitter empty / [bit 1] Transmit buffer empty
		}
		break;
		case S3C44B0_URXH :
		{
			uint8_t rxdata = data & 0xFF;
			verboselog( *this, 5, "UART %d read %02X (%c)\n", ch, rxdata, ((rxdata >= 32) && (rxdata < 128)) ? (char)rxdata : '?');
			m_uart[ch].regs.utrstat &= ~1; // [bit 0] Receive buffer data ready
		}
		break;
	}
	return data;
}

void s3c44b0_device::uart_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&((uint32_t*)&m_uart[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_UTXH :
		{
			uint8_t txdata = data & 0xFF;
			verboselog( *this, 5, "UART %d write %02X (%c)\n", ch, txdata, ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#ifdef UART_PRINTF
			printf( "%c", ((txdata >= 32) && (txdata < 128)) ? (char)txdata : '?');
#endif
		}
		break;
		case S3C44B0_UBRDIV :
		{
			uint32_t mclk, hz;
			mclk = get_mclk();
			hz = (mclk / (m_uart->regs.ubrdiv + 1)) / 16;
			verboselog( *this, 5, "UART %d - mclk %08X hz %08X\n", ch, mclk, hz);
			m_uart->timer->adjust(attotime::from_hz(hz), ch, attotime::from_hz(hz));
		}
		break;
	}
}

uint32_t s3c44b0_device::uart_0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = uart_r(0, offset);
//  verboselog( *this, 9, "(UART 0) %08X -> %08X\n", S3C44B0_BASE_UART_0 + (offset << 2), data);
	return data;
}

uint32_t s3c44b0_device::uart_1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = uart_r(1, offset);
//  verboselog( *this, 9, "(UART 1) %08X -> %08X\n", S3C44B0_BASE_UART_1 + (offset << 2), data);
	return data;
}

void s3c44b0_device::uart_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(UART 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_UART_0 + (offset << 2), data, mem_mask);
	uart_w(0, offset, data, mem_mask);
}

void s3c44b0_device::uart_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(UART 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_UART_1 + (offset << 2), data, mem_mask);
	uart_w(1, offset, data, mem_mask);
}

void s3c44b0_device::uart_fifo_w(int uart, uint8_t data)
{
//  printf("s3c44b0_uart_fifo_w (%c)\n", data);
	m_uart[uart].regs.urxh = data;
	m_uart[uart].regs.utrstat |= 1; // [bit 0] Receive buffer data ready
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::uart_timer_exp )
{
	int ch = param;
	verboselog( *this, 2, "UART %d timer callback\n", ch);
	if ((m_uart->regs.ucon & (1 << 9)) != 0)
	{
		const int ch_int[] = { S3C44B0_INT_UTXD0, S3C44B0_INT_UTXD1 };
		request_irq(ch_int[ch]);
	}
}

/* Watchdog Timer */

uint16_t s3c44b0_device::wdt_calc_current_count()
{
	return 0;
}

uint32_t s3c44b0_device::wdt_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_wdt.regs)[offset];
	switch (offset)
	{
		case S3C44B0_WTCNT :
		{
			// is wdt active?
			if ((m_wdt.regs.wtcon & (1 << 5)) != 0)
			{
				data = wdt_calc_current_count();
			}
		}
		break;
	}
	verboselog( *this, 9, "(WDT) %08X -> %08X\n", S3C44B0_BASE_WDT + (offset << 2), data);
	return data;
}

void s3c44b0_device::wdt_start()
{
	uint32_t mclk, prescaler, clock;
	double freq, hz;
	verboselog( *this, 1, "WDT start\n");
	mclk = get_mclk();
	prescaler = BITS(m_wdt.regs.wtcon, 15, 8);
	clock = 16 << BITS(m_wdt.regs.wtcon, 4, 3);
	freq = (double)mclk / (prescaler + 1) / clock;
	hz = freq / m_wdt.regs.wtcnt;
	verboselog( *this, 5, "WDT mclk %d prescaler %d clock %d freq %f hz %f\n", mclk, prescaler, clock, freq, hz);
	m_wdt.timer->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
}

void s3c44b0_device::wdt_stop()
{
	verboselog( *this, 1, "WDT stop\n");
	m_wdt.regs.wtcnt = wdt_calc_current_count();
	m_wdt.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::wdt_recalc()
{
	if ((m_wdt.regs.wtcon & (1 << 5)) != 0)
		wdt_start();
	else
		wdt_stop();
}

void s3c44b0_device::wdt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_wdt.regs)[offset];
	verboselog( *this, 9, "(WDT) %08X <- %08X\n", S3C44B0_BASE_WDT + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_wdt.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_WTCON :
		{
			if ((data & (1 << 5)) != (old_value & (1 << 5)))
			{
				wdt_recalc();
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::wdt_timer_exp )
{
	verboselog( *this, 2, "WDT timer callback\n");
	if ((m_wdt.regs.wtcon & (1 << 2)) != 0)
	{
		request_irq(S3C44B0_INT_WDT);
	}
	if ((m_wdt.regs.wtcon & (1 << 0)) != 0)
	{
		//s3c44b0_reset();
		fatalerror("s3c44b0_reset\n");
	}
}

/* CPU Wrapper */

uint32_t s3c44b0_device::cpuwrap_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_cpuwrap.regs)[offset];
	verboselog( *this, 9, "(CPUWRAP) %08X -> %08X\n", S3C44B0_BASE_CPU_WRAPPER + (offset << 2), data);
	return data;
}

void s3c44b0_device::cpuwrap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(CPUWRAP) %08X <- %08X\n", S3C44B0_BASE_CPU_WRAPPER + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_cpuwrap.regs)[offset]);
}

/* A/D Converter */

uint32_t s3c44b0_device::adc_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_adc.regs)[offset];
	verboselog( *this, 9, "(ADC) %08X -> %08X\n", S3C44B0_BASE_ADC + (offset << 2), data);
	return data;
}

void s3c44b0_device::adc_start()
{
	uint32_t mclk, prescaler;
	double freq, hz;
	verboselog( *this, 1, "ADC start\n");
	mclk = get_mclk();
	prescaler = BITS(m_adc.regs.adcpsr, 7, 0);
	freq = (double)mclk / (2 * (prescaler + 1)) / 16;
	hz = freq / 1; //m_wdt.regs.wtcnt;
	verboselog( *this, 5, "ADC mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	m_adc.timer->adjust(attotime::from_hz(hz), 0);
}

void s3c44b0_device::adc_stop()
{
	verboselog( *this, 1, "ADC stop\n");
	m_adc.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::adc_recalc()
{
	if ((m_adc.regs.adccon & (1 << 0)) != 0)
		adc_start();
	else
		adc_stop();
}

void s3c44b0_device::adc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_wdt.regs)[offset];
	verboselog( *this, 9, "(ADC) %08X <- %08X\n", S3C44B0_BASE_ADC + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_adc.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_ADCCON :
		{
			if ((data & (1 << 0)) != (old_value & (1 << 0)))
			{
				adc_recalc();
			}
			m_adc.regs.adccon &= ~(1 << 0); // "this bit is cleared after the start-up"
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::adc_timer_exp )
{
	verboselog( *this, 2, "ADC timer callback\n");
	m_adc.regs.adccon |= (1 << 6);
	request_irq(S3C44B0_INT_ADC);
}

/* SIO */

uint32_t s3c44b0_device::sio_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_sio.regs)[offset];
	verboselog( *this, 9, "(SIO) %08X -> %08X\n", S3C44B0_BASE_SIO + (offset << 2), data);
	return data;
}

void s3c44b0_device::sio_start()
{
	uint32_t mclk, prescaler;
	double freq, hz;
	verboselog( *this, 1, "SIO start\n");
	mclk = get_mclk();
	prescaler = BITS(m_sio.regs.sbrdr, 11, 0);
	freq = (double)mclk / 2 / (prescaler + 1);
	hz = freq / 1; //m_wdt.regs.wtcnt;
	verboselog( *this, 5, "SIO mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	m_sio.timer->adjust(attotime::from_hz(hz), 0);
//  printf("SIO transmit %02X (%c)\n", m_sio.regs.siodat, ((m_sio.regs.siodat >= 32) && (m_sio.regs.siodat < 128)) ? (char)m_sio.regs.siodat : '?');
}

void s3c44b0_device::sio_stop()
{
	verboselog( *this, 1, "SIO stop\n");
//  m_wdt.regs.wtcnt = s3c44b0_wdt_calc_current_count( device);
	m_sio.timer->adjust(attotime::never, 0);
}

void s3c44b0_device::sio_recalc()
{
	if ((m_sio.regs.siocon & (1 << 3)) != 0)
		sio_start();
	else
		sio_stop();
}

void s3c44b0_device::sio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_sio.regs)[offset];
	verboselog( *this, 9, "(SIO) %08X <- %08X\n", S3C44B0_BASE_SIO + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_sio.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_SIOCON :
		{
			if ((old_value & (1 << 3)) != (data & (1 << 3)))
			{
				sio_recalc();
			}
			m_sio.regs.siocon &= ~(1 << 3); // "This bit is cleared just after writing this bit as 1."
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::sio_timer_exp )
{
	verboselog( *this, 2, "SIO timer callback\n");

	m_sio.regs.siodat = 0x00; // TEST

	if ((m_sio.regs.siocon & (1 << 0)) != 0)
	{
		request_irq(S3C44B0_INT_SIO);
	}
}

/* IIS */

inline void s3c44b0_device::iface_i2s_data_w(int ch, uint16_t data)
{
	m_data_w_cb(ch, data, 0);
}

void s3c44b0_device::iis_start()
{
	uint32_t mclk;
	int prescaler;
	double freq, hz;
	const int div[] = { 2, 4, 6, 8, 10, 12, 14, 16, 1, 0, 3, 0, 5, 0, 7, 0 };
	verboselog( *this, 1, "IIS start\n");
	mclk = get_mclk();
	prescaler = BITS(m_iis.regs.iispsr, 3, 0);
	freq = (double)mclk / div[prescaler];
	hz = freq / 256 * 2;
	verboselog( *this, 5, "IIS mclk %d prescaler %d freq %f hz %f\n", mclk, prescaler, freq, hz);
	m_iis.timer->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
}

void s3c44b0_device::iis_stop()
{
	verboselog( *this, 1, "IIS stop\n");
	m_iis.timer->adjust(attotime::never, 0);
}

uint32_t s3c44b0_device::iis_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = ((uint32_t*)&m_iis.regs)[offset];
	verboselog( *this, 9, "(IIS) %08X -> %08X\n", S3C44B0_BASE_IIS + (offset << 2), data);
	return data;
}

void s3c44b0_device::iis_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_iis.regs)[offset];
	verboselog( *this, 9, "(IIS) %08X <- %08X\n", S3C44B0_BASE_IIS + (offset << 2), data);
	COMBINE_DATA(&((uint32_t*)&m_iis.regs)[offset]);
	switch (offset)
	{
		case S3C44B0_IISCON :
		{
			if ((old_value & (1 << 0)) != (data & (1 << 0)))
			{
				if ((data & (1 << 0)) != 0)
				{
					iis_start();
				}
				else
				{
					iis_stop();
				}
			}
		}
		break;
		case S3C44B0_IISFIFO :
		{
			if (ACCESSING_BITS_16_31)
			{
				m_iis.fifo[m_iis.fifo_index++] = BITS(data, 31, 16);
			}
			if (ACCESSING_BITS_0_15)
			{
				m_iis.fifo[m_iis.fifo_index++] = BITS(data, 15, 0);
			}
			if (m_iis.fifo_index == 2)
			{
				m_iis.fifo_index = 0;
				iface_i2s_data_w(0, m_iis.fifo[0]);
				iface_i2s_data_w(1, m_iis.fifo[1]);
			}
		}
		break;
	}
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::iis_timer_exp )
{
	verboselog( *this, 2, "IIS timer callback\n");
	if ((m_iis.regs.iiscon & (1 << 5)) != 0)
	{
		bdma_request_iis();
	}
}

/* ZDMA */

void s3c44b0_device::zdma_trigger(int ch)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	uint32_t saddr, daddr;
	int dal, dst, opt, das, cnt;
	verboselog( *this, 5, "s3c44b0_zdma_trigger %d\n", ch);
	dst = BITS(m_zdma->regs.dcsrc, 31, 30);
	dal = BITS(m_zdma->regs.dcsrc, 29, 28);
	saddr = BITS(m_zdma->regs.dcsrc, 27, 0);
	verboselog( *this, 5, "dst %d dal %d saddr %08X\n", dst, dal, saddr);
	opt = BITS(m_zdma->regs.dcdst, 31, 30);
	das = BITS(m_zdma->regs.dcdst, 29, 28);
	daddr = BITS(m_zdma->regs.dcdst, 27, 0);
	verboselog( *this, 5, "opt %d das %d daddr %08X\n", opt, das, daddr);
	cnt = BITS(m_zdma->regs.dccnt, 19, 0);
	verboselog( *this, 5, "icnt %08X\n", cnt);
	while (cnt > 0)
	{
		verboselog( *this, 9, "[%08X] -> [%08X]\n", saddr, daddr);
		switch (dst)
		{
			case 0 : space.write_byte(daddr, space.read_byte(saddr)); break;
			case 1 : space.write_word(daddr, space.read_word(saddr)); break;
			case 2 : space.write_dword(daddr, space.read_dword(saddr)); break;
		}
		switch (dal)
		{
			case 1 : saddr += (1 << dst); break;
			case 2 : saddr -= (1 << dst); break;
		}
		switch (das)
		{
			case 1 : daddr += (1 << dst); break;
			case 2 : daddr -= (1 << dst); break;
		}
		cnt -= (1 << dst);
	}
	m_zdma->regs.dcsrc = CLR_BITS(m_zdma->regs.dcsrc, 27, 0) | saddr;
	m_zdma->regs.dcdst = CLR_BITS(m_zdma->regs.dcdst, 27, 0) | daddr;
	m_zdma->regs.dccnt = CLR_BITS(m_zdma->regs.dcdst, 19, 0) | cnt;
	if (cnt == 0)
	{
		if ((m_zdma->regs.dccnt & (1 << 23)) != 0)
		{
			const int ch_int[] = { S3C44B0_INT_ZDMA0, S3C44B0_INT_ZDMA1 };
			request_irq(ch_int[ch]);
		}
	}
}

void s3c44b0_device::zdma_start(int ch)
{
	verboselog( *this, 5, "ZDMA %d start\n", ch);
	m_zdma->regs.dcsrc = m_zdma->regs.disrc;
	m_zdma->regs.dcdst = m_zdma->regs.didst;
	m_zdma->regs.dccnt = m_zdma->regs.dicnt;
	zdma_trigger(ch);
}

uint32_t s3c44b0_device::zdma_r(int ch, uint32_t offset)
{
	uint32_t data = ((uint32_t*)&m_zdma[ch].regs)[offset];
	return data;
}

void s3c44b0_device::zdma_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_zdma[ch].regs)[offset];
	COMBINE_DATA(&((uint32_t*)&m_zdma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_DCON :
		{
			if ((old_value & 3) != (data & 3))
			{
				switch (data & 3)
				{
					case 1 : zdma_start(ch); break;
				}
			}
			m_zdma[ch].regs.dcon &= ~3; // "After writing 01,10,11, CMD bit is cleared automatically"
		}
		break;
	}
}

uint32_t s3c44b0_device::zdma_0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = zdma_r(0, offset);
	verboselog( *this, 9, "(ZDMA 0) %08X -> %08X\n", S3C44B0_BASE_ZDMA_0 + (offset << 2), data);
	return data;
}

uint32_t s3c44b0_device::zdma_1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = zdma_r(1, offset);
	verboselog( *this, 9, "(ZDMA 1) %08X -> %08X\n", S3C44B0_BASE_ZDMA_1 + (offset << 2), data);
	return data;
}

void s3c44b0_device::zdma_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(ZDMA 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_ZDMA_0 + (offset << 2), data, mem_mask);
	zdma_w(0, offset, data, mem_mask);
}

void s3c44b0_device::zdma_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(ZDMA 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_ZDMA_1 + (offset << 2), data, mem_mask);
	zdma_w(1, offset, data, mem_mask);
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::zdma_timer_exp )
{
	int ch = param;
	verboselog( *this, 2, "ZDMA %d timer callback\n", ch);
}

/* BDMA */

void s3c44b0_device::bdma_trigger(int ch)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	uint32_t saddr, daddr;
	int dal, dst, tdm, das, cnt;
	verboselog( *this, 5, "s3c44b0_bdma_trigger %d\n", ch);
	dst = BITS(m_bdma->regs.dcsrc, 31, 30);
	dal = BITS(m_bdma->regs.dcsrc, 29, 28);
	saddr = BITS(m_bdma->regs.dcsrc, 27, 0);
	verboselog( *this, 5, "dst %d dal %d saddr %08X\n", dst, dal, saddr);
	tdm = BITS(m_bdma->regs.dcdst, 31, 30);
	das = BITS(m_bdma->regs.dcdst, 29, 28);
	daddr = BITS(m_bdma->regs.dcdst, 27, 0);
	verboselog( *this, 5, "tdm %d das %d daddr %08X\n", tdm, das, daddr);
	cnt = BITS(m_bdma->regs.dccnt, 19, 0);
	verboselog( *this, 5, "icnt %08X\n", cnt);
	verboselog( *this, 9, "[%08X] -> [%08X]\n", saddr, daddr);
	switch (dst)
	{
		case 0 : space.write_byte(daddr, space.read_byte(saddr)); break;
		case 1 : space.write_word(daddr, space.read_word(saddr)); break;
		case 2 : space.write_dword(daddr, space.read_dword(saddr)); break;
	}
	switch (dal)
	{
		case 1 : saddr += (1 << dst); break;
		case 2 : saddr -= (1 << dst); break;
	}
	switch (das)
	{
		case 1 : daddr += (1 << dst); break;
		case 2 : daddr -= (1 << dst); break;
	}
	cnt -= (1 << dst);
	m_bdma->regs.dcsrc = CLR_BITS(m_bdma->regs.dcsrc, 27, 0) | saddr;
	m_bdma->regs.dcdst = CLR_BITS(m_bdma->regs.dcdst, 27, 0) | daddr;
	m_bdma->regs.dccnt = CLR_BITS(m_bdma->regs.dcdst, 19, 0) | cnt;
	if (cnt == 0)
	{
		if ((m_bdma->regs.dccnt & (1 << 23)) != 0)
		{
			const int ch_int[] = { S3C44B0_INT_BDMA0, S3C44B0_INT_BDMA1 };
			request_irq(ch_int[ch]);
		}
	}
}

void s3c44b0_device::bdma_request_iis()
{
	verboselog( *this, 5, "s3c44b0_bdma_request_iis\n");
	bdma_trigger(0);
}

uint32_t s3c44b0_device::bdma_r(int ch, uint32_t offset)
{
	uint32_t data = ((uint32_t*)&m_bdma[ch].regs)[offset];
	return data;
}

void s3c44b0_device::bdma_start(int ch)
{
	verboselog( *this, 5, "BDMA %d start\n", ch);
	int qsc = BITS(m_bdma->regs.dicnt, 31, 30);
	if ((ch == 0) && (qsc == 1))
	{
		// IIS
	}
	else
	{
		printf( "s3c44b0_bdma_start - todo\n");
	}
	m_bdma->regs.dcsrc = m_bdma->regs.disrc;
	m_bdma->regs.dcdst = m_bdma->regs.didst;
	m_bdma->regs.dccnt = m_bdma->regs.dicnt;
}

void s3c44b0_device::bdma_stop(int ch)
{
	verboselog( *this, 5, "BDMA %d stop\n", ch);
	m_bdma[ch].timer->adjust(attotime::never, ch);
}

void s3c44b0_device::bdma_w(int ch, uint32_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old_value = ((uint32_t*)&m_bdma[ch].regs)[offset];
	COMBINE_DATA(&((uint32_t*)&m_bdma[ch].regs)[offset]);
	switch (offset)
	{
		case S3C44B0_DICNT :
		{
			if ((old_value & (1 << 20)) != (data & (1 << 20)))
			{
				if ((data & (1 << 20)) != 0)
				{
					bdma_start(ch);
				}
				else
				{
					bdma_stop(ch);
				}
			}
		}
		break;
	}
}

uint32_t s3c44b0_device::bdma_0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = bdma_r(0, offset);
	verboselog( *this, 9, "(BDMA 0) %08X -> %08X\n", S3C44B0_BASE_BDMA_0 + (offset << 2), data);
	return data;
}

uint32_t s3c44b0_device::bdma_1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = bdma_r(1, offset);
	verboselog( *this, 9, "(BDMA 1) %08X -> %08X\n", S3C44B0_BASE_BDMA_1 + (offset << 2), data);
	return data;
}

void s3c44b0_device::bdma_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(BDMA 0) %08X <- %08X (%08X)\n", S3C44B0_BASE_BDMA_0 + (offset << 2), data, mem_mask);
	bdma_w(0, offset, data, mem_mask);
}

void s3c44b0_device::bdma_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	verboselog( *this, 9, "(BDMA 1) %08X <- %08X (%08X)\n", S3C44B0_BASE_BDMA_1 + (offset << 2), data, mem_mask);
	bdma_w(1, offset, data, mem_mask);
}

TIMER_CALLBACK_MEMBER( s3c44b0_device::bdma_timer_exp )
{
	int ch = param;
	verboselog( *this, 2, "BDMA %d timer callback\n", ch);
}
