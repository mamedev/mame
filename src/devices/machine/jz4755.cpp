// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Ingenic JZ4755 SoC on-chip peripherals
	Emulation by R. Belmont

    References: JZ4755 Mobile Application Processor Programming Manual
    (Ingenic, Jan. 2011) and the JZ4755 data sheet.

    Physical register map (the CPU reaches it through kseg1 at 0xb0000000):
      0x10000000 CPM    clock, reset and power controller
      0x10001000 INTC   interrupt controller
      0x10002000 TCU    timers, OS timer and watchdog
      0x10003000 RTC    real time clock and hibernate controller
      0x10010000 GPIO   six 32-bit ports, 0x100 apart
      0x10020000 AIC    audio interface and internal codec
      0x10021000 MSC0   SD/MMC
      0x10022000 MSC1
      0x10030000 UART0  16550 style, 32-bit register stride
      0x10031000 UART1
      0x10032000 UART2
      0x10042000 I2C
      0x10043000 SSI0
      0x10045000 SSI1
      0x10070000 SADC   touch screen and battery ADC
      0x10072000 OWI    one-wire
      0x10073000 TSSI   transport stream interface
      0x13000000 HARB   AHB arbiter
      0x13010000 EMC    external memory controller; 0x13016000 ICSSR
      0x13020000 DMAC
      0x13040000 UDC    USB 2.0 device
      0x13050000 LCDC   LCD controller, OSD, SLCD and TV encoder
      0x13060000 CIM    camera interface
      0x13080000 IPU, MC, ME, DEBLK, IDCT   video accelerators
      0x130d0000 BCH    error correction engine

    The NAND flash hangs off static memory bank 1 (CS1#, 0x18000000),
    with A15 driving CLE and A16 driving ALE; its ready/busy line is a GPIO.

    Not emulated: audio recording, SD/MMC, USB, camera and the video
    accelerators other than the IPU scaler.

***************************************************************************/

#include "emu.h"
#include "jz4755.h"

#include "cpu/mips/mips3.h"

#include "multibyte.h"

#define LOG_CPM     (1U << 1)
#define LOG_INTC    (1U << 2)
#define LOG_TCU     (1U << 3)
#define LOG_RTC     (1U << 4)
#define LOG_GPIO    (1U << 5)
#define LOG_UART    (1U << 6)
#define LOG_EMC     (1U << 7)
#define LOG_NAND    (1U << 8)
#define LOG_BCH     (1U << 9)
#define LOG_LCD     (1U << 10)
#define LOG_STUB    (1U << 11)
#define LOG_SERIAL  (1U << 12)
#define LOG_DMA     (1U << 13)
#define LOG_IPU     (1U << 14)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(JZ4755, jz4755_device, "jz4755", "Ingenic JZ4755 peripherals")

namespace {

// CPM
enum : unsigned
{
	CPM_CPCCR = 0x00 / 4,
	CPM_LCR   = 0x04 / 4,
	CPM_RSR   = 0x08 / 4,
	CPM_CPPCR = 0x10 / 4,
	CPM_CPPSR = 0x14 / 4,
	CPM_CLKGR = 0x20 / 4,
	CPM_OPCR  = 0x24 / 4
};

constexpr uint32_t CPPCR_PLLEN = 1 << 8;
constexpr uint32_t CPPCR_PLLBP = 1 << 9;
constexpr uint32_t CPPCR_PLLS  = 1 << 10;

// the CDIV/HDIV/PDIV/MDIV encodings
const uint8_t CLOCK_DIVIDERS[16] = { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 32, 32, 32, 32, 32, 32 };

// TCU
enum : unsigned
{
	TCU_WDT_TDR  = 0x00 / 4,
	TCU_WDT_TCER = 0x04 / 4,
	TCU_WDT_TCNT = 0x08 / 4,
	TCU_WDT_TCSR = 0x0c / 4,
	TCU_TER      = 0x10 / 4,
	TCU_TESR     = 0x14 / 4,
	TCU_TECR     = 0x18 / 4,
	TCU_TSR      = 0x1c / 4,
	TCU_TFR      = 0x20 / 4,
	TCU_TFSR     = 0x24 / 4,
	TCU_TFCR     = 0x28 / 4,
	TCU_TSSR     = 0x2c / 4,
	TCU_TMR      = 0x30 / 4,
	TCU_TMSR     = 0x34 / 4,
	TCU_TMCR     = 0x38 / 4,
	TCU_TSCR     = 0x3c / 4,
	TCU_CHANNELS_BASE = 0x40 / 4,
	TCU_OSTDR    = 0xe0 / 4,
	TCU_OSTCNT   = 0xe8 / 4,
	TCU_OSTCSR   = 0xec / 4,
	TCU_TSTR     = 0xf0 / 4,
	TCU_TSTSR    = 0xf4 / 4,
	TCU_TSTCR    = 0xf8 / 4
};

constexpr uint32_t TCU_OST_BIT   = 1 << 15;
constexpr uint32_t TCU_WDT_BIT   = 1 << 16;    // TSR only
constexpr uint32_t TCSR_PCK_EN   = 1 << 0;
constexpr uint32_t TCSR_RTC_EN   = 1 << 1;
constexpr uint32_t TCSR_EXT_EN   = 1 << 2;
constexpr uint32_t OSTCSR_CNT_MD = 1 << 15;

// RTC
enum : unsigned
{
	RTC_RTCCR  = 0x00 / 4,
	RTC_RTCSR  = 0x04 / 4,
	RTC_RTCSAR = 0x08 / 4,
	RTC_RTCGR  = 0x0c / 4,
	RTC_HCR    = 0x20 / 4,
	RTC_HWFCR  = 0x24 / 4,
	RTC_HRCR   = 0x28 / 4,
	RTC_HWCR   = 0x2c / 4,
	RTC_HWRSR  = 0x30 / 4,
	RTC_HSPR   = 0x34 / 4
};

constexpr uint32_t RTCCR_WRDY = 1 << 7;

// UART
enum : unsigned
{
	UART_RBR_THR_DLL = 0,
	UART_IER_DLH,
	UART_IIR_FCR,
	UART_LCR,
	UART_MCR,
	UART_LSR,
	UART_MSR,
	UART_SPR,
	UART_ISR,
	UART_UMR,
	UART_UACR
};

constexpr uint8_t LCR_DLAB = 0x80;
constexpr uint8_t IER_RDRIE = 0x01;
constexpr uint8_t IER_TDRIE = 0x02;

// EMC
enum : unsigned
{
	EMC_BCR   = 0x00 / 4,
	EMC_NFCSR = 0x50 / 4,
	EMC_DMCR  = 0x80 / 4
};

constexpr uint32_t BCR_BSR = 1 << 2;

// BCH
enum : unsigned
{
	BCH_BHCR = 0,
	BCH_BHCSR,
	BCH_BHCCR,
	BCH_BHCNT,
	BCH_BHDR,
	BCH_BHPAR0,
	BCH_BHPAR1,
	BCH_BHPAR2,
	BCH_BHPAR3,
	BCH_BHINT,
	BCH_BHERR0,
	BCH_BHERR1,
	BCH_BHERR2,
	BCH_BHERR3,
	BCH_BHINTE,
	BCH_BHINTES,
	BCH_BHINTEC
};

constexpr uint32_t BHCR_BCHE = 1 << 0;
constexpr uint32_t BHCR_BRST = 1 << 1;
constexpr uint32_t BHCR_BSEL = 1 << 2;
constexpr uint32_t BHCR_ENCE = 1 << 3;
constexpr uint32_t BHINT_ERR   = 1 << 0;
constexpr uint32_t BHINT_UNCOR = 1 << 1;
constexpr uint32_t BHINT_ENCF  = 1 << 2;
constexpr uint32_t BHINT_DECF  = 1 << 3;
constexpr uint32_t BHINT_ALL_F = 1 << 4;
constexpr uint32_t BHINT_ALL_0 = 1 << 5;

// LCDC
enum : unsigned
{
	LCD_CFG     = 0x000 / 4,
	LCD_VSYNC   = 0x004 / 4,
	LCD_HSYNC   = 0x008 / 4,
	LCD_VAT     = 0x00c / 4,
	LCD_DAH     = 0x010 / 4,
	LCD_DAV     = 0x014 / 4,
	LCD_CTRL    = 0x030 / 4,
	LCD_STATE   = 0x034 / 4,
	LCD_IID     = 0x038 / 4,
	LCD_DA0     = 0x040 / 4,
	LCD_SA0     = 0x044 / 4,
	LCD_FID0    = 0x048 / 4,
	LCD_CMD0    = 0x04c / 4,
	LCD_DA1     = 0x050 / 4,
	LCD_OFFS0   = 0x060 / 4,
	LCD_PW0     = 0x064 / 4,
	LCD_CNUM0   = 0x068 / 4,
	LCD_DESSIZE0 = 0x06c / 4,
	LCD_OFFS1   = 0x070 / 4,
	LCD_RGBC    = 0x090 / 4,
	LCD_OSDC    = 0x100 / 4,
	LCD_OSDCTRL = 0x104 / 4,
	LCD_OSDS    = 0x108 / 4,
	LCD_BGC     = 0x10c / 4,
	LCD_KEY0    = 0x110 / 4,
	LCD_KEY1    = 0x114 / 4,
	LCD_ALPHA   = 0x118 / 4,
	LCD_IPUR    = 0x11c / 4,
	LCD_XYP0    = 0x120 / 4,
	LCD_XYP1    = 0x124 / 4,
	LCD_SIZE0   = 0x128 / 4,
	LCD_SIZE1   = 0x12c / 4,
	LCD_TVECR   = 0x140 / 4
};

constexpr uint32_t LCDCFG_NEWDES = 1 << 28;
constexpr uint32_t LCDCTRL_EOFM  = 1 << 13;
constexpr uint32_t LCDCTRL_SOFM  = 1 << 12;
constexpr uint32_t LCDCTRL_LDDM  = 1 << 8;
constexpr uint32_t LCDCTRL_QDM   = 1 << 7;
constexpr uint32_t LCDCTRL_DIS   = 1 << 4;
constexpr uint32_t LCDCTRL_ENA   = 1 << 3;
constexpr uint32_t LCDCTRL_RGB555 = 1 << 27;
constexpr uint32_t LCDSTATE_QD   = 1 << 7;
constexpr uint32_t LCDSTATE_EOF  = 1 << 5;
constexpr uint32_t LCDSTATE_SOF  = 1 << 4;
constexpr uint32_t LCDSTATE_LDD  = 1 << 0;
constexpr uint32_t LCDCMD_SOFINT = 1 << 31;
constexpr uint32_t LCDCMD_EOFINT = 1 << 30;
constexpr uint32_t LCDCMD_PAL    = 1 << 28;
constexpr uint32_t LCDOSDC_OSDEN = 1 << 0;
constexpr uint32_t LCDOSDC_F0EN  = 1 << 3;
constexpr uint32_t LCDOSDC_F1EN  = 1 << 4;

// I2C
constexpr uint32_t I2CCR_STA = 1 << 3;
constexpr uint32_t I2CCR_STO = 1 << 2;
constexpr uint32_t I2CSR_TEND = 1 << 2;
constexpr uint32_t I2CSR_DRF  = 1 << 1;

// DMA request types
constexpr unsigned DRT_NAND    = 0x01;
constexpr unsigned DRT_BCH_ENC = 0x02;
constexpr unsigned DRT_BCH_DEC = 0x03;
constexpr unsigned DRT_AUTO    = 0x08;
constexpr unsigned DRT_AIC_TX  = 0x18;

} // anonymous namespace


jz4755_device::jz4755_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JZ4755, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_nand(*this, finder_base::DUMMY_TAG),
	m_uart_tx(*this),
	m_gpio_out(*this),
	m_ost_timer(nullptr),
	m_wdt_timer(nullptr),
	m_nand_rb_port(GPIO_PORTS),
	m_nand_rb_bit(0),
	m_nand_timer(nullptr),
	m_stream(nullptr),
	m_aic_txrd(0),
	m_aic_txlevel(0),
	m_aic_timer(nullptr),
	m_ipu_frame_w(0),
	m_ipu_frame_h(0),
	m_ipu_timer(nullptr),
	m_ipu_frame_pending(false)
{
}

void jz4755_device::device_start()
{
	bch_init_tables();

	for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
	{
		m_tcu[ch].timer = timer_alloc(FUNC(jz4755_device::tcu_tick), this);
		m_tcu[ch].timer->set_param(ch);
	}
	m_ost_timer = timer_alloc(FUNC(jz4755_device::ost_tick), this);
	m_wdt_timer = timer_alloc(FUNC(jz4755_device::wdt_tick), this);
	m_nand_timer = timer_alloc(FUNC(jz4755_device::nand_ready), this);
	m_ipu_timer = timer_alloc(FUNC(jz4755_device::ipu_done), this);
	m_aic_timer = timer_alloc(FUNC(jz4755_device::aic_tick), this);
	m_stream = stream_alloc(0, 2, 48000);

	save_item(NAME(m_cpm));
	save_item(NAME(m_icsr));
	save_item(NAME(m_icmr));
	save_item(STRUCT_MEMBER(m_tcu, tdfr));
	save_item(STRUCT_MEMBER(m_tcu, tdhr));
	save_item(STRUCT_MEMBER(m_tcu, tcsr));
	save_item(STRUCT_MEMBER(m_tcu, tcnt));
	for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
		save_item(NAME(m_tcu[ch].base_time), ch);
	save_item(NAME(m_ter));
	save_item(NAME(m_tfr));
	save_item(NAME(m_tmr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_ostdr));
	save_item(NAME(m_ostcnt));
	save_item(NAME(m_ostcsr));
	save_item(NAME(m_ost_base));
	save_item(NAME(m_wdt_tdr));
	save_item(NAME(m_wdt_tcer));
	save_item(NAME(m_wdt_tcnt));
	save_item(NAME(m_wdt_tcsr));
	save_item(NAME(m_wdt_base));
	save_item(NAME(m_rtc));
	save_item(NAME(m_rtc_base));
	save_item(STRUCT_MEMBER(m_gpio, dat));
	save_item(STRUCT_MEMBER(m_gpio, im));
	save_item(STRUCT_MEMBER(m_gpio, pe));
	save_item(STRUCT_MEMBER(m_gpio, fun));
	save_item(STRUCT_MEMBER(m_gpio, sel));
	save_item(STRUCT_MEMBER(m_gpio, dir));
	save_item(STRUCT_MEMBER(m_gpio, trg));
	save_item(STRUCT_MEMBER(m_gpio, flg));
	save_item(STRUCT_MEMBER(m_gpio, ext));
	save_item(STRUCT_MEMBER(m_gpio, prev));
	save_item(STRUCT_MEMBER(m_gpio, out));
	save_item(STRUCT_MEMBER(m_uart, dll));
	save_item(STRUCT_MEMBER(m_uart, dlh));
	save_item(STRUCT_MEMBER(m_uart, ier));
	save_item(STRUCT_MEMBER(m_uart, fcr));
	save_item(STRUCT_MEMBER(m_uart, lcr));
	save_item(STRUCT_MEMBER(m_uart, mcr));
	save_item(STRUCT_MEMBER(m_uart, spr));
	save_item(STRUCT_MEMBER(m_uart, isr));
	save_item(STRUCT_MEMBER(m_uart, umr));
	save_item(STRUCT_MEMBER(m_uart, uacr));
	save_item(STRUCT_MEMBER(m_uart, thre_pending));
	save_item(NAME(m_emc));
	save_item(NAME(m_sdmr));
	save_item(NAME(m_bhcr));
	save_item(NAME(m_bhcnt));
	save_item(NAME(m_bhint));
	save_item(NAME(m_bhinte));
	save_item(NAME(m_bhpar));
	save_item(NAME(m_bherr));
	save_item(NAME(m_bch_count));
	save_item(NAME(m_bch_all_ff));
	save_item(NAME(m_bch_all_00));
	save_item(NAME(m_bch_buf));
	save_item(NAME(m_lcd));
	save_item(NAME(m_lcd_desc));
	save_item(NAME(m_aic));
	save_item(NAME(m_msc));
	save_item(NAME(m_i2c));
	save_item(NAME(m_i2c_reading));
	save_item(NAME(m_i2c_address_phase));
	save_item(NAME(m_sadc));
	save_item(NAME(m_ipu));
	save_item(NAME(m_ipu_frame));
	save_item(NAME(m_ipu_frame_w));
	save_item(NAME(m_ipu_frame_h));
	save_item(NAME(m_ipu_frame_pending));
	save_item(NAME(m_codec));
	save_item(NAME(m_aic_txfifo));
	save_item(NAME(m_aic_txrd));
	save_item(NAME(m_aic_txlevel));
	save_item(NAME(m_aic_out));
	save_item(NAME(m_stub));
	save_item(STRUCT_MEMBER(m_dma, dsa));
	save_item(STRUCT_MEMBER(m_dma, dta));
	save_item(STRUCT_MEMBER(m_dma, dtc));
	save_item(STRUCT_MEMBER(m_dma, drt));
	save_item(STRUCT_MEMBER(m_dma, dcs));
	save_item(STRUCT_MEMBER(m_dma, dcm));
	save_item(STRUCT_MEMBER(m_dma, dda));
	save_item(STRUCT_MEMBER(m_dma, dsd));
	save_item(STRUCT_MEMBER(m_dma, active));
	save_item(NAME(m_dmac_ctrl));
	save_item(NAME(m_dirqp));
	save_item(NAME(m_ddr));
	save_item(NAME(m_dcke));
}

void jz4755_device::device_reset()
{
	std::fill(std::begin(m_cpm), std::end(m_cpm), 0);
	m_cpm[CPM_CPCCR] = 0x42040000;
	m_cpm[CPM_LCR]   = 0x000000f8;
	m_cpm[CPM_RSR]   = 0x00000001;    // power-on reset
	m_cpm[CPM_CPPCR] = 0x28080011;
	m_cpm[CPM_CPPSR] = 0x80000000;
	m_cpm[CPM_OPCR]  = 0x00001500;
	m_cpm[0x60 / 4]  = 4;
	m_cpm[0x64 / 4]  = 4;
	m_cpm[0x7c / 4]  = 4;

	m_icsr = 0;
	m_icmr = 0xffffffff;

	for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
	{
		m_tcu[ch].tdfr = 0xffff;
		m_tcu[ch].tdhr = 0xffff;
		m_tcu[ch].tcsr = 0;
		m_tcu[ch].tcnt = 0;
		m_tcu[ch].base_time = machine().time();
		m_tcu[ch].timer->adjust(attotime::never);
	}
	m_ter = 0;
	m_tfr = 0;
	m_tmr = 0x003f003f | TCU_OST_BIT;
	m_tsr = 0;
	m_ostdr = 0xffffffff;
	m_ostcnt = 0;
	m_ostcsr = 0;
	m_ost_base = machine().time();
	m_ost_timer->adjust(attotime::never);
	m_wdt_tdr = 0;
	m_wdt_tcer = 0;
	m_wdt_tcnt = 0;
	m_wdt_tcsr = 0;
	m_wdt_base = machine().time();
	m_wdt_timer->adjust(attotime::never);

	std::fill(std::begin(m_rtc), std::end(m_rtc), 0);
	m_rtc[RTC_RTCCR] = 0x00000081;
	m_rtc[RTC_HWRSR] = 0x00000020;    // PPR: power-on
	m_rtc_base = machine().time();

	for (auto &port : m_gpio)
	{
		port.dat = 0;
		port.im = 0xffffffff;
		port.pe = 0;
		port.fun = 0;
		port.sel = 0;
		port.dir = 0;
		port.trg = 0;
		port.flg = 0;
		port.ext = 0xffffffff;
		port.prev = 0xffffffff;
		port.out = 0xffffffff;
	}
	m_nand_timer->adjust(attotime::never);

	for (auto &uart : m_uart)
	{
		uart.rx.clear();
		uart.line.clear();
		uart.dll = uart.dlh = uart.ier = uart.fcr = uart.lcr = uart.mcr = uart.spr = uart.isr = uart.umr = uart.uacr = 0;
		uart.thre_pending = false;
	}

	std::fill(std::begin(m_emc), std::end(m_emc), 0);
	m_emc[EMC_BCR] = 0x00000001;
	for (unsigned n = 0; n < 4; n++)
		m_emc[0x14 / 4 + n] = 0x0fff7700;
	m_emc[0x34 / 4] = 0x000018fc;
	m_emc[0x38 / 4] = 0x000016fe;
	m_emc[0x3c / 4] = 0x000014fe;
	m_emc[0x40 / 4] = 0x00000cfc;
	m_emc[EMC_NFCSR] = 0x00000001;    // NFE1: booted from NAND on CS1
	m_emc[0x90 / 4] = 0x000020f8;
	m_emc[0x94 / 4] = 0x000028f8;
	m_sdmr = 0;

	m_bhcr = 0;
	m_bhcnt = 0;
	m_bhinte = 0;
	bch_reset();

	std::fill(std::begin(m_lcd), std::end(m_lcd), 0);
	m_lcd[LCD_TVECR]       = 0x01040301;
	m_lcd[0x144 / 4]       = 0x00170271;
	m_lcd[0x150 / 4]       = 0x0320011a;
	m_lcd[0x154 / 4]       = 0x012800f0;
	m_lcd[0x158 / 4]       = 0x00000048;
	m_lcd[0x160 / 4]       = 0x00143f4e;
	m_lcd[0x164 / 4]       = 0x05a0103d;
	m_lcd[0x170 / 4]       = 0x2a098acb;
	m_lcd[0x174 / 4]       = 0x00000001;
	m_lcd[0x178 / 4]       = 0x3b3b8989;
	m_lcd[0x180 / 4]       = 0x00000070;
	std::fill(&m_lcd_desc[0][0], &m_lcd_desc[0][0] + 16, 0);

	std::fill(std::begin(m_aic), std::end(m_aic), 0);
	m_aic[0x00 / 4] = 0x00007800;
	m_aic[0x30 / 4] = 0x00000003;
	aic_reset();
	static const uint8_t codec_defaults[0x1c] =
	{
		0x0c, 0xaa, 0x78, 0x00, 0x00, 0xff, 0x03, 0x51, 0x3f, 0x00, 0x00, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0xc0, 0x34, 0x07, 0x44, 0x1f, 0x00
	};
	std::fill(std::begin(m_codec), std::end(m_codec), 0);
	std::copy(std::begin(codec_defaults), std::end(codec_defaults), m_codec);
	aic_set_rate();
	std::fill(std::begin(m_aic_out), std::end(m_aic_out), 0);
	for (auto &msc : m_msc)
	{
		std::fill(std::begin(msc), std::end(msc), 0);
		msc[0x04 / 4] = 0x00000040;
		msc[0x10 / 4] = 0x40;
		msc[0x14 / 4] = 0xffff;
		msc[0x24 / 4] = 0xff;
	}
	std::fill(std::begin(m_i2c), std::end(m_i2c), 0);
	m_i2c[2] = I2CSR_TEND;
	m_i2c_reading = false;
	m_i2c_address_phase = false;
	std::fill(std::begin(m_sadc), std::end(m_sadc), 0);
	m_sadc[0x04 / 4] = 0x0002000c;
	m_sadc[0x08 / 4] = 0x3f;
	std::fill(std::begin(m_ipu), std::end(m_ipu), 0);
	m_ipu[1] = 1;    // OUT_END: idle
	m_ipu_frame.clear();
	m_ipu_frame_w = m_ipu_frame_h = 0;
	m_ipu_frame_pending = false;
	m_ipu_timer->adjust(attotime::never);
	for (auto &ch : m_dma)
	{
		ch.dsa = ch.dta = ch.dtc = ch.drt = ch.dcs = ch.dcm = ch.dda = ch.dsd = 0;
		ch.active = false;
	}
	std::fill(std::begin(m_dmac_ctrl), std::end(m_dmac_ctrl), 0);
	std::fill(std::begin(m_dirqp), std::end(m_dirqp), 0);
	std::fill(std::begin(m_ddr), std::end(m_ddr), 0);
	std::fill(std::begin(m_dcke), std::end(m_dcke), 0);
	for (auto &stub : m_stub)
		std::fill(std::begin(stub), std::end(stub), 0);

	update_irq();
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void jz4755_device::map(address_map &map)
{
	map(0x0000000, 0x00000ff).rw(FUNC(jz4755_device::cpm_r), FUNC(jz4755_device::cpm_w));
	map(0x0001000, 0x000101f).rw(FUNC(jz4755_device::intc_r), FUNC(jz4755_device::intc_w));
	map(0x0002000, 0x00020ff).rw(FUNC(jz4755_device::tcu_r), FUNC(jz4755_device::tcu_w));
	map(0x0003000, 0x000303f).rw(FUNC(jz4755_device::rtc_r), FUNC(jz4755_device::rtc_w));
	map(0x0010000, 0x00105ff).rw(FUNC(jz4755_device::gpio_r), FUNC(jz4755_device::gpio_w));
	map(0x0020000, 0x00200ff).rw(FUNC(jz4755_device::aic_r), FUNC(jz4755_device::aic_w));
	map(0x0021000, 0x002107f).rw(FUNC(jz4755_device::msc_r<0>), FUNC(jz4755_device::msc_w<0>));
	map(0x0022000, 0x002207f).rw(FUNC(jz4755_device::msc_r<1>), FUNC(jz4755_device::msc_w<1>));
	map(0x0030000, 0x003002f).rw(FUNC(jz4755_device::uart_r<0>), FUNC(jz4755_device::uart_w<0>));
	map(0x0031000, 0x003102f).rw(FUNC(jz4755_device::uart_r<1>), FUNC(jz4755_device::uart_w<1>));
	map(0x0032000, 0x003202f).rw(FUNC(jz4755_device::uart_r<2>), FUNC(jz4755_device::uart_w<2>));
	map(0x0042000, 0x004200f).rw(FUNC(jz4755_device::i2c_r), FUNC(jz4755_device::i2c_w));
	map(0x0043000, 0x00430ff).rw(FUNC(jz4755_device::stub_r<1>), FUNC(jz4755_device::stub_w<1>));
	map(0x0045000, 0x00450ff).rw(FUNC(jz4755_device::stub_r<2>), FUNC(jz4755_device::stub_w<2>));
	map(0x0070000, 0x007003f).rw(FUNC(jz4755_device::sadc_r), FUNC(jz4755_device::sadc_w));
	map(0x0072000, 0x00720ff).rw(FUNC(jz4755_device::stub_r<3>), FUNC(jz4755_device::stub_w<3>));
	map(0x0073000, 0x00730ff).rw(FUNC(jz4755_device::stub_r<4>), FUNC(jz4755_device::stub_w<4>));
	map(0x3000000, 0x30000ff).rw(FUNC(jz4755_device::stub_r<0>), FUNC(jz4755_device::stub_w<0>));
	map(0x3010000, 0x301ffff).rw(FUNC(jz4755_device::emc_r), FUNC(jz4755_device::emc_w));
	map(0x3020000, 0x3020fff).rw(FUNC(jz4755_device::dmac_r), FUNC(jz4755_device::dmac_w));
	map(0x3040000, 0x3040fff).rw(FUNC(jz4755_device::stub_r<5>), FUNC(jz4755_device::stub_w<5>));
	map(0x3050000, 0x30501ff).rw(FUNC(jz4755_device::lcd_r), FUNC(jz4755_device::lcd_w));
	map(0x3060000, 0x30600ff).rw(FUNC(jz4755_device::stub_r<6>), FUNC(jz4755_device::stub_w<6>));
	map(0x3080000, 0x30800ff).rw(FUNC(jz4755_device::ipu_r), FUNC(jz4755_device::ipu_w));
	map(0x3090000, 0x30cffff).rw(FUNC(jz4755_device::stub_r<7>), FUNC(jz4755_device::stub_w<7>));
	map(0x30d0000, 0x30d00ff).rw(FUNC(jz4755_device::bch_r), FUNC(jz4755_device::bch_w));
	// VPU auxiliary processor program memory; the firmware loads aux_task.bin here
	map(0x3800000, 0x380ffff).ram();
}

void jz4755_device::nand_map(address_map &map)
{
	map(0x0000000, 0x3ffffff).rw(FUNC(jz4755_device::nand_r), FUNC(jz4755_device::nand_w));
}

uint32_t jz4755_device::read_phys(uint32_t address)
{
	return m_cpu->space(AS_PROGRAM).read_dword(address & 0x1fffffff);
}


/***************************************************************************
    CLOCK, RESET AND POWER CONTROLLER
***************************************************************************/

uint32_t jz4755_device::pll_clock() const
{
	const uint32_t cppcr = m_cpm[CPM_CPPCR];
	if (!(cppcr & CPPCR_PLLEN) || (cppcr & CPPCR_PLLBP))
		return clock();

	const uint32_t m = (cppcr >> 23) + 2;
	const uint32_t n = ((cppcr >> 18) & 0x1f) + 2;
	static const uint8_t od[4] = { 1, 2, 2, 4 };
	return uint32_t((uint64_t(clock()) * m) / (n * od[(cppcr >> 16) & 3]));
}

uint32_t jz4755_device::cpu_clock() const
{
	return pll_clock() / CLOCK_DIVIDERS[m_cpm[CPM_CPCCR] & 0xf];
}

uint32_t jz4755_device::pclk() const
{
	return pll_clock() / CLOCK_DIVIDERS[(m_cpm[CPM_CPCCR] >> 8) & 0xf];
}

uint32_t jz4755_device::cpm_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = m_cpm[offset & 0x3f];
	switch (offset)
	{
		case CPM_CPPCR:
			// the PLL locks instantly
			if (data & CPPCR_PLLEN)
				data |= CPPCR_PLLS;
			break;

		case CPM_CPPSR:
			// PLL state plus "change finished" flags for PS, FS and CS
			data = ((m_cpm[CPM_CPPCR] & CPPCR_PLLEN) ? (1 << 29) : (1 << 31)) | 0x1c000000 | (data & 7);
			break;
	}
	return data;
}

void jz4755_device::cpm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CPM, "%s: CPM reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
	COMBINE_DATA(&m_cpm[offset & 0x3f]);

	if (offset == CPM_CPCCR || offset == CPM_CPPCR)
	{
		// follow the firmware's clock setup; keep the CPU inside sane bounds
		const uint32_t cclk = std::clamp<uint32_t>(cpu_clock(), 12'000'000, 500'000'000);
		if (cclk != m_cpu->unscaled_clock())
		{
			LOGMASKED(LOG_CPM, "CPM: PLL %u Hz, CPU %u Hz, PCLK %u Hz\n", pll_clock(), cclk, pclk());
			m_cpu->set_unscaled_clock(cclk);
		}
		for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
			tcu_restart(ch);
		ost_restart();
	}
}


/***************************************************************************
    INTERRUPT CONTROLLER
***************************************************************************/

uint32_t jz4755_device::intc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0: return m_icsr;              // ICSR
		case 1: return m_icmr;              // ICMR
		case 4: return m_icsr & ~m_icmr;    // ICPR
	}
	return 0;
}

void jz4755_device::intc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 1: m_icmr = data; break;       // ICMR
		case 2: m_icmr |= data; break;      // ICMSR
		case 3: m_icmr &= ~data; break;     // ICMCR
		default:
			LOGMASKED(LOG_INTC, "%s: INTC reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
			return;
	}
	LOGMASKED(LOG_INTC, "%s: INTC mask now %08x\n", machine().describe_context(), m_icmr);
	update_irq();
}

void jz4755_device::set_irq(unsigned source, bool state)
{
	const uint32_t bit = 1U << source;
	const uint32_t icsr = state ? (m_icsr | bit) : (m_icsr & ~bit);
	if (icsr != m_icsr)
	{
		m_icsr = icsr;
		update_irq();
	}
}

void jz4755_device::update_irq()
{
	m_cpu->set_input_line(MIPS3_IRQ0, (m_icsr & ~m_icmr) ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************
    TIMER/COUNTER UNIT, OS TIMER AND WATCHDOG
***************************************************************************/

uint32_t jz4755_device::timer_clock(uint16_t tcsr) const
{
	uint32_t source = 0;
	if (tcsr & TCSR_PCK_EN)
		source = pclk();
	else if (tcsr & TCSR_EXT_EN)
		source = clock();
	else if (tcsr & TCSR_RTC_EN)
		source = 32768;
	const unsigned prescale = (tcsr >> 3) & 7;
	return (prescale > 5) ? 0 : (source >> (2 * prescale));
}

// a 16-bit up-counter that clears when it reaches TDFR
uint32_t jz4755_device::tcu_count(unsigned ch) const
{
	const tcu_channel_t &t = m_tcu[ch];
	const uint32_t freq = timer_clock(t.tcsr);
	const uint32_t period = uint32_t(t.tdfr) + 1;
	if (!BIT(m_ter, ch) || BIT(m_tsr, ch) || freq == 0)
		return t.tcnt % period;
	const uint64_t ticks = (machine().time() - t.base_time).as_ticks(freq);
	return uint32_t((t.tcnt + ticks) % period);
}

// latch the current count and rebase time, e.g. around a clock change
void jz4755_device::tcu_restart(unsigned ch)
{
	m_tcu[ch].tcnt = tcu_count(ch);
	m_tcu[ch].base_time = machine().time();
	tcu_reschedule(ch);
}

void jz4755_device::tcu_reschedule(unsigned ch)
{
	tcu_channel_t &t = m_tcu[ch];
	const uint32_t freq = timer_clock(t.tcsr);
	if (!BIT(m_ter, ch) || BIT(m_tsr, ch) || freq == 0)
	{
		t.timer->adjust(attotime::never);
		return;
	}

	// next event: the half compare if still ahead of us, otherwise the full compare (wrap to 0)
	const uint32_t count = tcu_count(ch);
	uint32_t target = uint32_t(t.tdfr) + 1;
	if (t.tdhr < t.tdfr && count < t.tdhr)
		target = t.tdhr;
	const uint32_t ticks = target - count;
	attotime delay = attotime::from_ticks(ticks ? ticks : 1, freq);
	// guard against an interrupt storm from a tiny period at PCLK rate
	if (delay < attotime::from_usec(1))
		delay = attotime::from_usec(1);
	t.timer->adjust(delay, ch);
}

TIMER_CALLBACK_MEMBER(jz4755_device::tcu_tick)
{
	const unsigned ch = param;
	tcu_channel_t &t = m_tcu[ch];
	const uint32_t count = tcu_count(ch);

	// at the half compare the counter continues; at the full compare it has wrapped to 0
	if (t.tdhr < t.tdfr && count >= t.tdhr)
		m_tfr |= 1 << (16 + ch);
	if (count < t.tdhr || t.tdhr >= t.tdfr)
	{
		m_tfr |= 1 << ch;
		if (t.tdhr <= t.tdfr)
			m_tfr |= 1 << (16 + ch);
	}
	tcu_update_irqs();
	tcu_reschedule(ch);
}

void jz4755_device::tcu_update_irqs()
{
	const uint32_t pending = m_tfr & ~m_tmr;
	set_irq(IRQ_TCU0, (pending & TCU_OST_BIT) != 0);
	set_irq(IRQ_TCU1, (pending & 0x00200020) != 0);
	set_irq(IRQ_TCU2, (pending & 0x001f001f) != 0);
}

uint32_t jz4755_device::ost_count() const
{
	const uint32_t freq = timer_clock(m_ostcsr);
	if (!(m_ter & TCU_OST_BIT) || (m_tsr & TCU_OST_BIT) || freq == 0)
		return m_ostcnt;
	const uint64_t ticks = (machine().time() - m_ost_base).as_ticks(freq);
	if (m_ostcsr & OSTCSR_CNT_MD)
		return uint32_t(m_ostcnt + ticks);
	const uint64_t period = uint64_t(m_ostdr) + 1;
	return uint32_t((m_ostcnt + ticks) % period);
}

void jz4755_device::ost_restart()
{
	m_ostcnt = ost_count();
	m_ost_base = machine().time();
	ost_reschedule();
}

void jz4755_device::ost_reschedule()
{
	const uint32_t freq = timer_clock(m_ostcsr);
	if (!(m_ter & TCU_OST_BIT) || (m_tsr & TCU_OST_BIT) || freq == 0)
	{
		m_ost_timer->adjust(attotime::never);
		return;
	}
	const uint32_t count = ost_count();
	uint64_t ticks;
	if (m_ostcsr & OSTCSR_CNT_MD)
		ticks = (count <= m_ostdr) ? (uint64_t(m_ostdr) - count) : (0x1'0000'0000ULL - count + m_ostdr);
	else
		ticks = uint64_t(m_ostdr) + 1 - count;
	attotime delay = attotime::from_ticks(ticks ? ticks : 1, freq);
	if (delay < attotime::from_usec(1))
		delay = attotime::from_usec(1);
	m_ost_timer->adjust(delay);
}

TIMER_CALLBACK_MEMBER(jz4755_device::ost_tick)
{
	m_tfr |= TCU_OST_BIT;
	tcu_update_irqs();
	if (!(m_ostcsr & OSTCSR_CNT_MD))
	{
		// the counter restarts from zero
		m_ostcnt = 0;
		m_ost_base = machine().time();
	}
	ost_reschedule();
}

void jz4755_device::wdt_reschedule()
{
	const uint32_t freq = timer_clock(m_wdt_tcsr);
	if (!(m_wdt_tcer & 1) || (m_tsr & TCU_WDT_BIT) || freq == 0)
	{
		m_wdt_timer->adjust(attotime::never);
		return;
	}
	const uint32_t ticks = ((m_wdt_tdr - m_wdt_tcnt) & 0xffff);
	m_wdt_timer->adjust(attotime::from_ticks(ticks ? ticks : 0x10000, freq));
}

TIMER_CALLBACK_MEMBER(jz4755_device::wdt_tick)
{
	logerror("watchdog timeout, resetting\n");
	m_cpm[CPM_RSR] = 0x00000002;    // WR
	machine().schedule_soft_reset();
}

uint32_t jz4755_device::tcu_r(offs_t offset, uint32_t mem_mask)
{
	if (offset >= TCU_CHANNELS_BASE && offset < TCU_CHANNELS_BASE + 4 * TCU_CHANNELS)
	{
		const unsigned ch = (offset - TCU_CHANNELS_BASE) / 4;
		switch ((offset - TCU_CHANNELS_BASE) & 3)
		{
			case 0: return m_tcu[ch].tdfr;
			case 1: return m_tcu[ch].tdhr;
			case 2: return tcu_count(ch);
			case 3: return m_tcu[ch].tcsr;
		}
	}

	switch (offset)
	{
		case TCU_WDT_TDR:  return m_wdt_tdr;
		case TCU_WDT_TCER: return m_wdt_tcer;
		case TCU_WDT_TCNT:
		{
			const uint32_t freq = timer_clock(m_wdt_tcsr);
			if (!(m_wdt_tcer & 1) || freq == 0)
				return m_wdt_tcnt;
			return (m_wdt_tcnt + (machine().time() - m_wdt_base).as_ticks(freq)) & 0xffff;
		}
		case TCU_WDT_TCSR: return m_wdt_tcsr;
		case TCU_TER:      return m_ter;
		case TCU_TSR:      return m_tsr;
		case TCU_TFR:      return m_tfr;
		case TCU_TMR:      return m_tmr;
		case TCU_OSTDR:    return m_ostdr;
		case TCU_OSTCNT:   return ost_count();
		case TCU_OSTCSR:   return m_ostcsr;
		case TCU_TSTR:     return 0x00030000;   // REAL1/REAL2: TCU2 counters readable
	}
	LOGMASKED(LOG_TCU, "%s: TCU read of unimplemented reg %02x\n", machine().describe_context(), offset * 4);
	return 0;
}

void jz4755_device::tcu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= TCU_CHANNELS_BASE && offset < TCU_CHANNELS_BASE + 4 * TCU_CHANNELS)
	{
		const unsigned ch = (offset - TCU_CHANNELS_BASE) / 4;
		tcu_channel_t &t = m_tcu[ch];
		LOGMASKED(LOG_TCU, "%s: TCU%u reg %u = %04x\n", machine().describe_context(), ch, (offset - TCU_CHANNELS_BASE) & 3, data & 0xffff);
		switch ((offset - TCU_CHANNELS_BASE) & 3)
		{
			case 0: t.tcnt = tcu_count(ch); t.base_time = machine().time(); t.tdfr = data & 0xffff; break;
			case 1: t.tcnt = tcu_count(ch); t.base_time = machine().time(); t.tdhr = data & 0xffff; break;
			case 2: t.tcnt = data & 0xffff; t.base_time = machine().time(); break;
			case 3: t.tcnt = tcu_count(ch); t.base_time = machine().time(); t.tcsr = data & 0xffff; break;
		}
		tcu_reschedule(ch);
		return;
	}

	switch (offset)
	{
		case TCU_WDT_TDR:  m_wdt_tdr = data & 0xffff; wdt_reschedule(); break;
		case TCU_WDT_TCER:
			m_wdt_tcnt = tcu_r(TCU_WDT_TCNT, 0xffffffff);
			m_wdt_base = machine().time();
			m_wdt_tcer = data & 1;
			LOGMASKED(LOG_TCU, "%s: watchdog %s\n", machine().describe_context(), (data & 1) ? "enabled" : "disabled");
			wdt_reschedule();
			break;
		case TCU_WDT_TCNT: m_wdt_tcnt = data & 0xffff; m_wdt_base = machine().time(); wdt_reschedule(); break;
		case TCU_WDT_TCSR: m_wdt_tcsr = data & 0xffff; wdt_reschedule(); break;

		case TCU_TESR:
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				if (BIT(data, ch) && !BIT(m_ter, ch))
				{
					m_tcu[ch].base_time = machine().time();
				}
			if ((data & TCU_OST_BIT) && !(m_ter & TCU_OST_BIT))
				m_ost_base = machine().time();
			m_ter |= data & (0x3f | TCU_OST_BIT);
			LOGMASKED(LOG_TCU, "%s: TCU enable now %04x\n", machine().describe_context(), m_ter);
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				tcu_reschedule(ch);
			ost_reschedule();
			break;
		case TCU_TECR:
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				if (BIT(data, ch) && BIT(m_ter, ch))
				{
					m_tcu[ch].tcnt = tcu_count(ch);
					m_tcu[ch].base_time = machine().time();
				}
			if ((data & TCU_OST_BIT) && (m_ter & TCU_OST_BIT))
			{
				m_ostcnt = ost_count();
				m_ost_base = machine().time();
			}
			m_ter &= ~data;
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				tcu_reschedule(ch);
			ost_reschedule();
			break;

		case TCU_TSSR:
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				if (BIT(data, ch) && !BIT(m_tsr, ch))
					tcu_restart(ch);
			if ((data & TCU_OST_BIT) && !(m_tsr & TCU_OST_BIT))
				ost_restart();
			m_tsr |= data & (0x3f | TCU_OST_BIT | TCU_WDT_BIT);
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				tcu_reschedule(ch);
			ost_reschedule();
			wdt_reschedule();
			break;
		case TCU_TSCR:
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				if (BIT(data, ch) && BIT(m_tsr, ch))
					m_tcu[ch].base_time = machine().time();
			if ((data & TCU_OST_BIT) && (m_tsr & TCU_OST_BIT))
				m_ost_base = machine().time();
			m_tsr &= ~data;
			for (unsigned ch = 0; ch < TCU_CHANNELS; ch++)
				tcu_reschedule(ch);
			ost_reschedule();
			wdt_reschedule();
			break;

		case TCU_TFSR: m_tfr |= data; tcu_update_irqs(); break;
		case TCU_TFCR:
			m_tfr &= ~data;
			tcu_update_irqs();
			break;
		case TCU_TMSR: m_tmr |= data; tcu_update_irqs(); break;
		case TCU_TMCR:
			m_tmr &= ~data;
			LOGMASKED(LOG_TCU, "%s: TCU mask now %08x\n", machine().describe_context(), m_tmr);
			tcu_update_irqs();
			break;

		case TCU_OSTDR:  m_ostcnt = ost_count(); m_ost_base = machine().time(); m_ostdr = data; ost_reschedule(); break;
		case TCU_OSTCNT: m_ostcnt = data; m_ost_base = machine().time(); ost_reschedule(); break;
		case TCU_OSTCSR:
			m_ostcnt = ost_count();
			m_ost_base = machine().time();
			m_ostcsr = data & 0xffff;
			LOGMASKED(LOG_TCU, "%s: OSTCSR = %04x (clock %u Hz)\n", machine().describe_context(), m_ostcsr, timer_clock(m_ostcsr));
			ost_reschedule();
			break;

		case TCU_TSTSR:
		case TCU_TSTCR:
			break;

		default:
			LOGMASKED(LOG_TCU, "%s: TCU write to unimplemented reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
			break;
	}
}


/***************************************************************************
    REAL TIME CLOCK
***************************************************************************/

uint32_t jz4755_device::rtc_seconds() const
{
	return m_rtc[RTC_RTCSR] + uint32_t((machine().time() - m_rtc_base).as_double());
}

uint32_t jz4755_device::rtc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case RTC_RTCCR: return m_rtc[RTC_RTCCR] | RTCCR_WRDY;    // always ready for the next write
		case RTC_RTCSR: return rtc_seconds();
	}
	return m_rtc[offset & 0xf];
}

void jz4755_device::rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: RTC reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
	switch (offset)
	{
		case RTC_RTCSR:
			m_rtc[RTC_RTCSR] = data;
			m_rtc_base = machine().time();
			return;

		case RTC_HCR:
			if (data & 1)
			{
				// hibernate: the box powers itself down
				logerror("%s: RTC hibernate requested\n", machine().describe_context());
				m_cpu->suspend(SUSPEND_REASON_HALT, true);
			}
			return;
	}
	m_rtc[offset & 0xf] = data;
}


/***************************************************************************
    GENERAL PURPOSE I/O
***************************************************************************/

uint32_t jz4755_device::gpio_pins(unsigned port)
{
	const gpio_port_t &p = m_gpio[port];
	return (p.dat & p.dir & ~p.sel) | (p.ext & ~(p.dir & ~p.sel));
}

void jz4755_device::gpio_input(unsigned port, unsigned bit, int state)
{
	gpio_port_t &p = m_gpio[port];
	const uint32_t mask = 1U << bit;
	const uint32_t ext = state ? (p.ext | mask) : (p.ext & ~mask);
	if (ext == p.ext)
		return;
	p.ext = ext;
	gpio_update(port);
}

// several input levels of one port at once
void jz4755_device::gpio_input_mask(unsigned port, uint32_t mask, uint32_t state)
{
	gpio_port_t &p = m_gpio[port];
	const uint32_t ext = (p.ext & ~mask) | (state & mask);
	if (ext == p.ext)
		return;
	p.ext = ext;
	gpio_update(port);
}

// interrupt pins (FUN = 0, SEL = 1): TRG selects level or edge, DIR the polarity
void jz4755_device::gpio_update(unsigned port)
{
	gpio_port_t &p = m_gpio[port];
	const uint32_t pins = gpio_pins(port);
	const uint32_t irqpins = ~p.fun & p.sel;
	const uint32_t level = irqpins & ~p.trg & ~(pins ^ p.dir);
	const uint32_t rising = irqpins & p.trg & p.dir & pins & ~p.prev;
	const uint32_t falling = irqpins & p.trg & ~p.dir & ~pins & p.prev;
	p.flg |= level | rising | falling;
	p.prev = pins;

	static const unsigned sources[GPIO_PORTS] = { IRQ_GPIO0, IRQ_GPIO1, IRQ_GPIO2, IRQ_GPIO3, IRQ_GPIO4, IRQ_GPIO5 };
	set_irq(sources[port], (p.flg & ~p.im) != 0);
}

// pins configured as outputs drive their data bit; everything else floats high
void jz4755_device::gpio_output_changed(unsigned port)
{
	gpio_port_t &p = m_gpio[port];
	const uint32_t driven = p.dat | ~(p.dir & ~p.sel & ~p.fun);
	if (driven != p.out)
	{
		p.out = driven;
		m_gpio_out[port](driven);
	}
}

TIMER_CALLBACK_MEMBER(jz4755_device::nand_ready)
{
	if (m_nand_rb_port < GPIO_PORTS)
		gpio_input(m_nand_rb_port, m_nand_rb_bit, 1);
}

uint32_t jz4755_device::gpio_r(offs_t offset, uint32_t mem_mask)
{
	const unsigned port = (offset >> 6) % GPIO_PORTS;
	gpio_port_t &p = m_gpio[port];
	switch (offset & 0x3f)
	{
		case 0x00 / 4: return gpio_pins(port);
		case 0x10 / 4: return p.dat;
		case 0x20 / 4: return p.im;
		case 0x30 / 4: return p.pe;
		case 0x40 / 4: return p.fun;
		case 0x50 / 4: return p.sel;
		case 0x60 / 4: return p.dir;
		case 0x70 / 4: return p.trg;
		case 0x80 / 4: return p.flg;
	}
	LOGMASKED(LOG_GPIO, "%s: GPIO%c read of reg %02x\n", machine().describe_context(), 'A' + port, (offset & 0x3f) * 4);
	return 0;
}

void jz4755_device::gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const unsigned port = (offset >> 6) % GPIO_PORTS;
	gpio_port_t &p = m_gpio[port];
	if ((offset & 0x3f) >= 0x30 / 4 && (offset & 0x3f) < 0x80 / 4)
		LOGMASKED(LOG_GPIO, "%s: GPIO%c reg %02x = %08x\n", machine().describe_context(), 'A' + port, (offset & 0x3f) * 4, data);
	switch (offset & 0x3f)
	{
		case 0x10 / 4: p.dat = data; break;
		case 0x14 / 4:
			// PxDATS doubles as PxFLGC: pins in interrupt mode have their flag cleared instead
			p.dat |= data & ~(~p.fun & p.sel);
			p.flg &= ~(data & ~p.fun & p.sel);
			break;
		case 0x18 / 4: p.dat &= ~data; break;
		case 0x20 / 4: p.im = data; break;
		case 0x24 / 4: p.im |= data; break;
		case 0x28 / 4: p.im &= ~data; break;
		case 0x30 / 4: p.pe = data; break;
		case 0x34 / 4: p.pe |= data; break;
		case 0x38 / 4: p.pe &= ~data; break;
		case 0x40 / 4: p.fun = data; break;
		case 0x44 / 4: p.fun |= data; break;
		case 0x48 / 4: p.fun &= ~data; break;
		case 0x50 / 4: p.sel = data; break;
		case 0x54 / 4: p.sel |= data; break;
		case 0x58 / 4: p.sel &= ~data; break;
		case 0x60 / 4: p.dir = data; break;
		case 0x64 / 4: p.dir |= data; break;
		case 0x68 / 4: p.dir &= ~data; break;
		case 0x70 / 4: p.trg = data; break;
		case 0x74 / 4: p.trg |= data; break;
		case 0x78 / 4: p.trg &= ~data; break;
		case 0x80 / 4: p.flg = data; break;
		case 0x84 / 4: p.flg &= ~data; break;
		case 0x00 / 4: return;    // PxPIN is read-only; the firmware writes it anyway
		default:
			LOGMASKED(LOG_GPIO, "%s: GPIO%c write to unknown reg %02x = %08x\n", machine().describe_context(), 'A' + port, (offset & 0x3f) * 4, data);
			return;
	}
	gpio_output_changed(port);
	gpio_update(port);
}


/***************************************************************************
    UARTS
***************************************************************************/

template <unsigned N> uint32_t jz4755_device::uart_r(offs_t offset, uint32_t mem_mask) { return uart_read(N, offset); }
template <unsigned N> void jz4755_device::uart_w(offs_t offset, uint32_t data, uint32_t mem_mask) { uart_write(N, offset, data & 0xff); }

template uint32_t jz4755_device::uart_r<0>(offs_t, uint32_t);
template uint32_t jz4755_device::uart_r<1>(offs_t, uint32_t);
template uint32_t jz4755_device::uart_r<2>(offs_t, uint32_t);
template void jz4755_device::uart_w<0>(offs_t, uint32_t, uint32_t);
template void jz4755_device::uart_w<1>(offs_t, uint32_t, uint32_t);
template void jz4755_device::uart_w<2>(offs_t, uint32_t, uint32_t);

void jz4755_device::uart_rx(unsigned n, uint8_t data)
{
	m_uart[n].rx.push_back(data);
	uart_update_irq(n);
}

void jz4755_device::uart_update_irq(unsigned n)
{
	const uart_t &u = m_uart[n];
	const bool rx = (u.ier & IER_RDRIE) && !u.rx.empty();
	const bool tx = (u.ier & IER_TDRIE) && u.thre_pending;
	static const unsigned sources[UARTS] = { IRQ_UART0, IRQ_UART1, IRQ_UART2 };
	set_irq(sources[n], rx || tx);
}

uint32_t jz4755_device::uart_read(unsigned n, offs_t offset)
{
	uart_t &u = m_uart[n];
	switch (offset)
	{
		case UART_RBR_THR_DLL:
			if (u.lcr & LCR_DLAB)
				return u.dll;
			if (!u.rx.empty())
			{
				const uint8_t data = u.rx.front();
				if (!machine().side_effects_disabled())
				{
					u.rx.pop_front();
					uart_update_irq(n);
				}
				return data;
			}
			return 0;

		case UART_IER_DLH:
			return (u.lcr & LCR_DLAB) ? u.dlh : u.ier;

		case UART_IIR_FCR:
		{
			uint8_t iir = (u.fcr & 1) ? 0xc0 : 0x00;
			if ((u.ier & IER_RDRIE) && !u.rx.empty())
				iir |= 0x04;
			else if ((u.ier & IER_TDRIE) && u.thre_pending)
			{
				// reading the identification clears the transmitter-empty request
				iir |= 0x02;
				if (!machine().side_effects_disabled())
				{
					u.thre_pending = false;
					uart_update_irq(n);
				}
			}
			else
				iir |= 0x01;
			return iir;
		}

		case UART_LCR: return u.lcr;
		case UART_MCR: return u.mcr;
		case UART_LSR: return 0x60 | (u.rx.empty() ? 0x00 : 0x01);    // TEMT | TDRQ, DR
		case UART_MSR: return 0xb0;                                    // CTS, DSR and DCD asserted
		case UART_SPR: return u.spr;
		case UART_ISR: return u.isr;
		case UART_UMR: return u.umr;
		case UART_UACR: return u.uacr;
	}
	return 0;
}

void jz4755_device::uart_write(unsigned n, offs_t offset, uint8_t data)
{
	uart_t &u = m_uart[n];
	switch (offset)
	{
		case UART_RBR_THR_DLL:
			if (u.lcr & LCR_DLAB)
			{
				u.dll = data;
				break;
			}
			m_uart_tx[n](data);
			if (data == '\n' || u.line.length() >= 200)
			{
				LOGMASKED(LOG_SERIAL, "UART%u: %s\n", n, u.line);
				u.line.clear();
			}
			else if (data >= 0x20 && data < 0x7f)
				u.line.push_back(char(data));
			u.thre_pending = true;
			uart_update_irq(n);
			break;

		case UART_IER_DLH:
			if (u.lcr & LCR_DLAB)
				u.dlh = data;
			else
			{
				if ((data & IER_TDRIE) && !(u.ier & IER_TDRIE))
					u.thre_pending = true;
				u.ier = data;
				uart_update_irq(n);
			}
			break;

		case UART_IIR_FCR:
			u.fcr = data;
			if (data & 0x02)
			{
				u.rx.clear();
				uart_update_irq(n);
			}
			break;

		case UART_LCR: u.lcr = data; break;
		case UART_MCR: u.mcr = data; break;
		case UART_SPR: u.spr = data; break;
		case UART_ISR: u.isr = data; break;
		case UART_UMR: u.umr = data; break;
		case UART_UACR: u.uacr = data; break;
		default:
			LOGMASKED(LOG_UART, "%s: UART%u write to unknown reg %02x = %02x\n", machine().describe_context(), n, offset * 4, data);
			break;
	}
}


/***************************************************************************
    EXTERNAL MEMORY CONTROLLER AND NAND INTERFACE
***************************************************************************/

uint32_t jz4755_device::emc_r(offs_t offset, uint32_t mem_mask)
{
	if (offset < 0x40)
		return m_emc[offset];
	if (offset == 0x6000 / 4)
		return BIT(m_icsr, IRQ_SFT) ? 0 : 1;    // ICSSR reads back as "cleared" when no software interrupt is pending
	LOGMASKED(LOG_EMC, "%s: EMC read of reg %04x\n", machine().describe_context(), offset * 4);
	return 0;
}

void jz4755_device::emc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset < 0x40)
	{
		LOGMASKED(LOG_EMC, "%s: EMC reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
		// BCR is read-only apart from BSR
		if (offset == EMC_BCR)
			data = (m_emc[EMC_BCR] & ~BCR_BSR) | (data & BCR_BSR);
		COMBINE_DATA(&m_emc[offset]);
		return;
	}
	if (offset == 0x6000 / 4)
	{
		// ICSSR: 0 raises the software interrupt, 1 clears it
		set_irq(IRQ_SFT, !(data & 1));
		return;
	}
	if ((offset & ~0x3ff) == 0x8000 / 4)
	{
		// SDMR: the SDRAM mode register is programmed through this address window
		m_sdmr = offset & 0x3ff;
		LOGMASKED(LOG_EMC, "%s: SDRAM mode register %03x\n", machine().describe_context(), m_sdmr);
		return;
	}
	LOGMASKED(LOG_EMC, "%s: EMC write to unknown reg %04x = %08x\n", machine().describe_context(), offset * 4, data);
}

// static bank 1: A15 selects the command latch, A16 the address latch (A3/A4 when BCR.BSR is set)
uint32_t jz4755_device::nand_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t address = offset << 2;
	const uint32_t cle = (m_emc[EMC_BCR] & BCR_BSR) ? 0x08 : 0x8000;
	const uint32_t ale = (m_emc[EMC_BCR] & BCR_BSR) ? 0x10 : 0x10000;
	if (address & (cle | ale))
	{
		LOGMASKED(LOG_NAND, "%s: NAND read from latch space %07x\n", machine().describe_context(), address);
		return 0;
	}

	uint32_t data = 0;
	for (unsigned lane = 0; lane < 4; lane++)
	{
		if (BIT(mem_mask, lane * 8, 8))
			data |= uint32_t(m_nand->data_r()) << (lane * 8);
	}
	return data;
}

void jz4755_device::nand_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t address = offset << 2;
	const uint32_t cle = (m_emc[EMC_BCR] & BCR_BSR) ? 0x08 : 0x8000;
	const uint32_t ale = (m_emc[EMC_BCR] & BCR_BSR) ? 0x10 : 0x10000;
	for (unsigned lane = 0; lane < 4; lane++)
	{
		if (!BIT(mem_mask, lane * 8, 8))
			continue;
		const uint8_t byte = data >> (lane * 8);
		if (address & cle)
		{
			LOGMASKED(LOG_NAND, "%s: NAND command %02x\n", machine().describe_context(), byte);
			m_nand->command_w(byte);

			// the ready/busy line drops for the array access; it is a GPIO to the firmware
			attotime busy = attotime::zero;
			switch (byte)
			{
				case 0x30: busy = attotime::from_usec(25); break;      // read start (tR)
				case 0x10: busy = attotime::from_usec(200); break;     // program (tPROG)
				case 0xd0: busy = attotime::from_usec(1500); break;    // block erase (tBERS)
				case 0xff: busy = attotime::from_usec(5); break;       // reset
			}
			if (busy != attotime::zero && m_nand_rb_port < GPIO_PORTS)
			{
				gpio_input(m_nand_rb_port, m_nand_rb_bit, 0);
				m_nand_timer->adjust(busy);
			}
		}
		else if (address & ale)
		{
			LOGMASKED(LOG_NAND, "%s: NAND address %02x\n", machine().describe_context(), byte);
			m_nand->address_w(byte);
		}
		else
			m_nand->data_w(byte);
	}
}


/***************************************************************************
    BCH ERROR CORRECTION ENGINE

    Binary BCH over GF(2^13) built on the primitive polynomial
    x^13 + x^10 + x^8 + x^7 + x^5 + x^3 + x^2 + x + 1 (0x25af), correcting
    4 (52 parity bits) or 8 (104 parity bits) errors per block.  Both data
    and parity are fed least significant bit first; the parity stream
    starts with the highest degree coefficient, and the error index the
    hardware reports is the 1-based bit position in that stream.  The
    polynomial and bit ordering were recovered from the parity stored in
    the Gamebox NAND dump.
***************************************************************************/

uint16_t jz4755_device::s_gf_exp[2 * GF_ORDER];
uint16_t jz4755_device::s_gf_log[GF_ORDER + 1];
uint64_t jz4755_device::s_bch_gen[2][2];
bool jz4755_device::s_bch_tables_ready = false;

void jz4755_device::bch_init_tables()
{
	if (s_bch_tables_ready)
		return;

	unsigned x = 1;
	for (unsigned i = 0; i < GF_ORDER; i++)
	{
		s_gf_exp[i] = x;
		s_gf_log[x] = i;
		x <<= 1;
		if (x & 0x2000)
			x ^= 0x25af;
	}
	for (unsigned i = GF_ORDER; i < 2 * GF_ORDER; i++)
		s_gf_exp[i] = s_gf_exp[i - GF_ORDER];
	s_gf_log[0] = 0;

	// generator polynomials: the product of (x + alpha^r) over the conjugacy classes of alpha^1..alpha^2t
	for (unsigned mode = 0; mode < 2; mode++)
	{
		const unsigned t = mode ? 8 : 4;
		bool is_root[GF_ORDER] = { false };
		for (unsigned i = 1; i <= 2 * t; i++)
			for (unsigned r = i; !is_root[r]; r = (r * 2) % GF_ORDER)
				is_root[r] = true;

		std::vector<uint16_t> g(1, 1);
		for (unsigned r = 0; r < GF_ORDER; r++)
		{
			if (!is_root[r])
				continue;
			const uint16_t a = s_gf_exp[r];
			std::vector<uint16_t> ng(g.size() + 1, 0);
			for (unsigned i = 0; i < g.size(); i++)
			{
				ng[i] ^= gf_mul(g[i], a);
				ng[i + 1] ^= g[i];
			}
			g = std::move(ng);
		}
		assert(g.size() == 13 * t + 1);

		s_bch_gen[mode][0] = s_bch_gen[mode][1] = 0;
		for (unsigned i = 0; i < 13 * t; i++)
		{
			assert(g[i] <= 1);
			if (g[i])
				s_bch_gen[mode][i / 64] |= uint64_t(1) << (i % 64);
		}
	}
	s_bch_tables_ready = true;
}

uint16_t jz4755_device::gf_mul(uint16_t a, uint16_t b)
{
	if (a == 0 || b == 0)
		return 0;
	return s_gf_exp[s_gf_log[a] + s_gf_log[b]];
}

void jz4755_device::bch_reset()
{
	m_bhint = 0;
	m_bch_count = 0;
	m_bch_all_ff = true;
	m_bch_all_00 = true;
	m_bch_buf.clear();
	std::fill(std::begin(m_bhpar), std::end(m_bhpar), 0);
	std::fill(std::begin(m_bherr), std::end(m_bherr), 0);
	set_irq(IRQ_BCH, false);
}

void jz4755_device::bch_data_w(uint8_t data)
{
	if (!(m_bhcr & BHCR_BCHE))
		return;
	if (data != 0xff)
		m_bch_all_ff = false;
	if (data != 0x00)
		m_bch_all_00 = false;
	m_bch_count++;
	m_bch_buf.push_back(data);

	const uint32_t total = (m_bhcr & BHCR_ENCE) ? (m_bhcnt & 0x3ff) : ((m_bhcnt >> 16) & 0x3ff);
	if (m_bch_count == total)
	{
		if (m_bhcr & BHCR_ENCE)
			bch_encode();
		else
			bch_decode();
		set_irq(IRQ_BCH, (m_bhint & m_bhinte) != 0);
	}
}

// systematic encoding: the parity is (data(x) * x^(13t)) mod g(x), kept in a 13t-bit shift register
void jz4755_device::bch_encode()
{
	const unsigned mode = (m_bhcr & BHCR_BSEL) ? 1 : 0;
	const unsigned plen = mode ? 104 : 52;
	uint64_t lo = 0, hi = 0;
	for (const uint8_t byte : m_bch_buf)
	{
		for (unsigned b = 0; b < 8; b++)
		{
			const unsigned top = (plen > 64) ? ((hi >> (plen - 65)) & 1) : ((lo >> (plen - 1)) & 1);
			const unsigned feedback = top ^ ((byte >> b) & 1);
			hi = (hi << 1) | (lo >> 63);
			lo <<= 1;
			if (feedback)
			{
				lo ^= s_bch_gen[mode][0];
				hi ^= s_bch_gen[mode][1];
			}
		}
	}
	if (plen > 64)
		hi &= (uint64_t(1) << (plen - 64)) - 1;
	else
	{
		lo &= (uint64_t(1) << plen) - 1;
		hi = 0;
	}

	// stream bit s is coefficient plen-1-s; the stream packs least significant bit first into BHPAR0..3
	std::fill(std::begin(m_bhpar), std::end(m_bhpar), 0);
	for (unsigned s = 0; s < plen; s++)
	{
		const unsigned coeff = plen - 1 - s;
		const unsigned bit = (coeff >= 64) ? ((hi >> (coeff - 64)) & 1) : ((lo >> coeff) & 1);
		if (bit)
			m_bhpar[s / 32] |= 1U << (s % 32);
	}
	m_bhint |= BHINT_ENCF;
	LOGMASKED(LOG_BCH, "%s: BCH encode of %u bytes: %08x %08x %08x %08x\n", machine().describe_context(), m_bch_count, m_bhpar[0], m_bhpar[1], m_bhpar[2], m_bhpar[3]);
}

void jz4755_device::bch_decode()
{
	const unsigned t = (m_bhcr & BHCR_BSEL) ? 8 : 4;
	const unsigned nbits = m_bch_buf.size() * 8;

	m_bhint |= BHINT_DECF;
	if (m_bch_all_ff)
	{
		// erased blocks are passed through without correction
		m_bhint |= BHINT_ALL_F;
		return;
	}
	if (m_bch_all_00)
		m_bhint |= BHINT_ALL_0;

	// syndromes S_j = r(alpha^j), accumulated Horner style over the stream
	uint16_t synd[17] = { 0 };
	bool clean = true;
	for (unsigned j = 1; j <= 2 * t; j++)
	{
		uint16_t s = 0;
		for (const uint8_t byte : m_bch_buf)
		{
			for (unsigned b = 0; b < 8; b++)
			{
				if (s)
					s = s_gf_exp[s_gf_log[s] + j];
				s ^= (byte >> b) & 1;
			}
		}
		synd[j] = s;
		if (s)
			clean = false;
	}
	if (clean)
		return;

	// Berlekamp-Massey for the error locator polynomial
	uint16_t lambda[17] = { 1 }, prev[17] = { 1 };
	unsigned len = 0, shift = 1;
	uint16_t prev_disc = 1;
	for (unsigned n = 0; n < 2 * t; n++)
	{
		uint16_t disc = synd[n + 1];
		for (unsigned i = 1; i <= len && i <= n; i++)
			disc ^= gf_mul(lambda[i], synd[n + 1 - i]);
		if (disc == 0)
		{
			shift++;
			continue;
		}
		const uint16_t ratio = gf_mul(disc, s_gf_exp[GF_ORDER - s_gf_log[prev_disc]]);
		uint16_t next[17];
		std::copy(std::begin(lambda), std::end(lambda), std::begin(next));
		for (unsigned i = 0; i + shift <= 16; i++)
			next[i + shift] ^= gf_mul(ratio, prev[i]);
		if (2 * len <= n)
		{
			std::copy(std::begin(lambda), std::end(lambda), std::begin(prev));
			len = n + 1 - len;
			prev_disc = disc;
			shift = 1;
		}
		else
			shift++;
		std::copy(std::begin(next), std::end(next), std::begin(lambda));
	}

	// Chien search: a root at alpha^-k puts the error at stream position nbits-1-k
	unsigned found = 0;
	uint32_t index[8];
	if (len <= t)
	{
		for (unsigned k = 0; k < nbits && found <= len; k++)
		{
			uint16_t v = 0;
			for (unsigned i = 0; i <= len; i++)
			{
				if (lambda[i])
					v ^= s_gf_exp[(s_gf_log[lambda[i]] + ((GF_ORDER - k) % GF_ORDER) * i) % GF_ORDER];
			}
			if (v == 0 && found < 8)
				index[found++] = nbits - 1 - k + 1;
		}
	}

	m_bhint |= BHINT_ERR;
	if (len > t || found != len)
	{
		m_bhint |= BHINT_UNCOR;
		LOG("%s: BCH decode of %u bytes: uncorrectable (%u syndrome-derived errors, %u roots)\n", machine().describe_context(), m_bch_count, len, found);
		return;
	}
	m_bhint |= found << 28;
	std::fill(std::begin(m_bherr), std::end(m_bherr), 0);
	for (unsigned i = 0; i < found; i++)
		m_bherr[i / 2] |= (index[i] & 0x1fff) << ((i & 1) ? 16 : 0);
	LOGMASKED(LOG_BCH, "%s: BCH decode of %u bytes: %u error(s), first at bit %u\n", machine().describe_context(), m_bch_count, found, index[0]);
}

uint32_t jz4755_device::bch_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case BCH_BHCR:  return m_bhcr;
		case BCH_BHCNT: return m_bhcnt;
		case BCH_BHPAR0: case BCH_BHPAR1: case BCH_BHPAR2: case BCH_BHPAR3:
			return m_bhpar[offset - BCH_BHPAR0];
		case BCH_BHINT: return m_bhint;
		case BCH_BHERR0: case BCH_BHERR1: case BCH_BHERR2: case BCH_BHERR3:
			return m_bherr[offset - BCH_BHERR0];
		case BCH_BHINTE: return m_bhinte;
	}
	LOGMASKED(LOG_BCH, "%s: BCH read of reg %02x\n", machine().describe_context(), offset * 4);
	return 0;
}

void jz4755_device::bch_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case BCH_BHCSR:
			m_bhcr |= data & (BHCR_BCHE | BHCR_BSEL | BHCR_ENCE | (1 << 4));
			if (data & BHCR_BRST)
				bch_reset();
			break;
		case BCH_BHCCR:
			m_bhcr &= ~(data & (BHCR_BCHE | BHCR_BSEL | BHCR_ENCE | (1 << 4)));
			break;
		case BCH_BHCNT:
			COMBINE_DATA(&m_bhcnt);
			break;
		case BCH_BHDR:
			for (unsigned lane = 0; lane < 4; lane++)
				if (BIT(mem_mask, lane * 8, 8))
					bch_data_w(data >> (lane * 8));
			break;
		case BCH_BHPAR0: case BCH_BHPAR1: case BCH_BHPAR2: case BCH_BHPAR3:
			COMBINE_DATA(&m_bhpar[offset - BCH_BHPAR0]);
			break;
		case BCH_BHINT:
			// documented read-only, but the SPL writes all ones here before a decode; treat it as write-1-to-clear
			m_bhint &= ~data;
			set_irq(IRQ_BCH, (m_bhint & m_bhinte) != 0);
			break;
		case BCH_BHINTE:  m_bhinte = data; set_irq(IRQ_BCH, (m_bhint & m_bhinte) != 0); break;
		case BCH_BHINTES: m_bhinte |= data; set_irq(IRQ_BCH, (m_bhint & m_bhinte) != 0); break;
		case BCH_BHINTEC: m_bhinte &= ~data; set_irq(IRQ_BCH, (m_bhint & m_bhinte) != 0); break;
		default:
			LOGMASKED(LOG_BCH, "%s: BCH write to unknown reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
			break;
	}
}


/***************************************************************************
    LCD CONTROLLER AND TV ENCODER

    Frames come from DMA descriptor chains: channel 0 carries foreground 0
    (or the whole picture outside OSD mode), channel 1 foreground 1.  The
    chain is walked once per frame from the vblank callback, which also
    raises the start/end of frame flags the firmware waits on.
***************************************************************************/

uint32_t jz4755_device::lcd_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == LCD_OSDS)
		return m_lcd[LCD_OSDS] | 1;    // READY
	return m_lcd[offset & 0x7f];
}

void jz4755_device::lcd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: LCD reg %03x = %08x\n", machine().describe_context(), offset * 4, data);
	switch (offset)
	{
		case LCD_CTRL:
		{
			// Either way of disabling the controller is acknowledged at once.  The manual pairs
			// ENA with LDD and DIS with QD, but the firmware waits for QD after clearing ENA and
			// for LDD after setting DIS, so the hardware evidently raises both flags for both.
			const uint32_t prev = m_lcd[LCD_CTRL];
			m_lcd[LCD_CTRL] = data;
			if ((data & LCDCTRL_DIS) || ((prev & LCDCTRL_ENA) && !(data & LCDCTRL_ENA)))
				m_lcd[LCD_STATE] |= LCDSTATE_LDD | LCDSTATE_QD;
			lcd_update_irq();
			return;
		}

		case LCD_STATE:
			m_lcd[LCD_STATE] = data;
			lcd_update_irq();
			return;

		case LCD_OSDS:
			m_lcd[LCD_OSDS] = data;
			lcd_update_irq();
			return;

		case LCD_DA0:
		case LCD_DA1:
			m_lcd[offset] = data;
			return;
	}
	COMBINE_DATA(&m_lcd[offset & 0x7f]);
}

void jz4755_device::lcd_update_irq()
{
	const uint32_t ctrl = m_lcd[LCD_CTRL];
	const uint32_t state = m_lcd[LCD_STATE];
	bool irq = ((ctrl & LCDCTRL_EOFM) && (state & LCDSTATE_EOF))
			|| ((ctrl & LCDCTRL_SOFM) && (state & LCDSTATE_SOF))
			|| ((ctrl & LCDCTRL_LDDM) && (state & LCDSTATE_LDD))
			|| ((ctrl & LCDCTRL_QDM) && (state & LCDSTATE_QD));
	// per-foreground masks in OSD mode share the bit positions between LCDOSDC and LCDOSDS
	irq = irq || ((m_lcd[LCD_OSDC] & m_lcd[LCD_OSDS] & 0xcc00) != 0);
	set_irq(IRQ_LCD, irq);
}

void jz4755_device::vblank_w(int state)
{
	if (!state)
		return;
	if (m_ipu_frame_pending)
	{
		m_ipu_frame_pending = false;
		ipu_done(0);
	}
	const uint32_t ctrl = m_lcd[LCD_CTRL];
	if (!(ctrl & LCDCTRL_ENA) || (ctrl & LCDCTRL_DIS))
		return;

	const bool osd = (m_lcd[LCD_OSDC] & LCDOSDC_OSDEN) != 0;
	const bool newdes = (m_lcd[LCD_CFG] & LCDCFG_NEWDES) != 0;
	for (unsigned ch = 0; ch < 2; ch++)
	{
		if (ch == 1 && !(osd && (m_lcd[LCD_OSDC] & LCDOSDC_F1EN)))
			continue;
		const uint32_t da = m_lcd[LCD_DA0 + ch * 4];
		if (da == 0)
			continue;

		// fetch the descriptor the channel points at, then advance to the next one
		for (unsigned word = 0; word < (newdes ? 8 : 4); word++)
			m_lcd_desc[ch][word] = read_phys(da + word * 4);
		m_lcd[LCD_SA0 + ch * 4]   = m_lcd_desc[ch][1];
		m_lcd[LCD_FID0 + ch * 4]  = m_lcd_desc[ch][2];
		m_lcd[LCD_CMD0 + ch * 4]  = m_lcd_desc[ch][3];
		m_lcd[LCD_OFFS0 + ch * 4] = m_lcd_desc[ch][4];
		m_lcd[LCD_PW0 + ch * 4]   = m_lcd_desc[ch][5];
		m_lcd[LCD_CNUM0 + ch * 4] = m_lcd_desc[ch][6];
		m_lcd[LCD_DESSIZE0 + ch * 4] = m_lcd_desc[ch][7];
		m_lcd[LCD_DA0 + ch * 4] = m_lcd_desc[ch][0];

		const uint32_t cmd = m_lcd_desc[ch][3];
		if (cmd & LCDCMD_SOFINT)
		{
			m_lcd[LCD_STATE] |= LCDSTATE_SOF;
			m_lcd[LCD_OSDS] |= (ch == 0) ? (1 << 11) : (1 << 15);
		}
		if (cmd & LCDCMD_EOFINT)
		{
			m_lcd[LCD_STATE] |= LCDSTATE_EOF;
			m_lcd[LCD_OSDS] |= (ch == 0) ? (1 << 10) : (1 << 14);
			m_lcd[LCD_IID] = m_lcd_desc[ch][2];
		}
	}
	lcd_update_irq();
}

void jz4755_device::draw_foreground(bitmap_rgb32 &bitmap, const rectangle &cliprect, unsigned fg)
{
	const uint32_t *const desc = m_lcd_desc[fg];
	const uint32_t cmd = desc[3];

	// foreground 1 can be fed by the IPU instead of a DMA channel
	if (fg == 1 && (m_lcd[LCD_OSDCTRL] & 0x8000))
	{
		if (m_ipu_frame_w == 0)
			return;
		const uint32_t xyp = m_lcd[LCD_XYP1];
		const int xpos = xyp & 0xfff, ypos = (xyp >> 16) & 0xfff;
		const int width = std::min<int>(m_lcd[LCD_SIZE1] & 0xfff, m_ipu_frame_w);
		const int height = std::min<int>((m_lcd[LCD_SIZE1] >> 16) & 0xfff, m_ipu_frame_h);
		for (int y = 0; y < height; y++)
		{
			const int sy = ypos + y;
			if (sy < cliprect.min_y || sy > cliprect.max_y)
				continue;
			uint32_t *const dst = &bitmap.pix(sy);
			const uint32_t *const src = &m_ipu_frame[y * m_ipu_frame_w];
			for (int x = 0; x < width; x++)
			{
				const int sx = xpos + x;
				if (sx >= cliprect.min_x && sx <= cliprect.max_x)
					dst[sx] = src[x];
			}
		}
		return;
	}

	if (desc[1] == 0 || (cmd & 0xffffff) == 0 || (cmd & LCDCMD_PAL))
		return;

	const bool osd = (m_lcd[LCD_OSDC] & LCDOSDC_OSDEN) != 0;
	const bool newdes = (m_lcd[LCD_CFG] & LCDCFG_NEWDES) != 0;

	// geometry: the OSD foreground size and position, else the display area
	int width, height, xpos = 0, ypos = 0;
	if (osd)
	{
		const uint32_t size = m_lcd[LCD_SIZE0 + fg];
		const uint32_t xyp = m_lcd[LCD_XYP0 + fg];
		width = size & 0xfff;
		height = (size >> 16) & 0xfff;
		xpos = xyp & 0xfff;
		ypos = (xyp >> 16) & 0xfff;
	}
	else
	{
		width = (m_lcd[LCD_DAH] & 0xfff) - ((m_lcd[LCD_DAH] >> 16) & 0xfff);
		height = (m_lcd[LCD_DAV] & 0xfff) - ((m_lcd[LCD_DAV] >> 16) & 0xfff);
	}
	if (newdes && (desc[7] & 0xfff) != 0)
	{
		width = desc[7] & 0xfff;
		height = (desc[7] >> 16) & 0xfff;
	}
	if (width <= 0 || height <= 0)
		return;

	// pixel format: LCDCTRL.BPP for foreground 0, LCDOSDCTRL.OSDBPP for foreground 1
	const uint32_t bpp_code = (fg == 0) ? (m_lcd[LCD_CTRL] & 7) : (m_lcd[LCD_OSDCTRL] & 7);
	const bool rgb555 = (fg == 0) ? ((m_lcd[LCD_CTRL] & LCDCTRL_RGB555) != 0) : ((m_lcd[LCD_OSDCTRL] & 0x10) != 0);
	unsigned bits_per_pixel;
	switch (bpp_code)
	{
		case 3:  bits_per_pixel = 8; break;
		case 4:  bits_per_pixel = 16; break;
		case 5:  bits_per_pixel = 32; break;
		case 6:  bits_per_pixel = 24; break;
		default:
			return;
	}

	// line stride in bytes: PAGE_WIDTH words plus OFFSIZE words of padding when given
	uint32_t stride = (width * bits_per_pixel + 7) / 8;
	if (newdes)
	{
		if (desc[5] != 0)
			stride = desc[5] * 4;
		stride += desc[4] * 4;
	}

	address_space &space = m_cpu->space(AS_PROGRAM);
	const uint32_t base = desc[1] & 0x1fffffff;
	for (int y = 0; y < height; y++)
	{
		const int sy = ypos + y;
		if (sy < cliprect.min_y || sy > cliprect.max_y)
			continue;
		uint32_t *const dst = &bitmap.pix(sy);
		const uint32_t line = base + y * stride;
		for (int x = 0; x < width; x++)
		{
			const int sx = xpos + x;
			if (sx < cliprect.min_x || sx > cliprect.max_x)
				continue;
			uint32_t pixel;
			switch (bits_per_pixel)
			{
				case 8:
					// palette loading is not supported yet; show the index as grey
					pixel = space.read_byte(line + x) * 0x010101;
					break;
				case 16:
				{
					const uint16_t p = space.read_word(line + x * 2);
					if (rgb555)
						pixel = (pal5bit(p >> 10) << 16) | (pal5bit(p >> 5) << 8) | pal5bit(p);
					else
						pixel = (pal5bit(p >> 11) << 16) | (pal6bit(p >> 5) << 8) | pal5bit(p);
					break;
				}
				case 24:
					pixel = space.read_byte(line + x * 3) | (space.read_byte(line + x * 3 + 1) << 8) | (space.read_byte(line + x * 3 + 2) << 16);
					break;
				default:
					pixel = space.read_dword(line + x * 4) & 0xffffff;
					break;
			}
			dst[sx] = pixel;
		}
	}
}

uint32_t jz4755_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint32_t ctrl = m_lcd[LCD_CTRL];
	const bool osd = (m_lcd[LCD_OSDC] & LCDOSDC_OSDEN) != 0;
	bitmap.fill(osd ? (m_lcd[LCD_BGC] & 0xffffff) : 0, cliprect);
	if (!(ctrl & LCDCTRL_ENA) || (ctrl & LCDCTRL_DIS))
		return 0;

	if (!osd || (m_lcd[LCD_OSDC] & LCDOSDC_F0EN))
		draw_foreground(bitmap, cliprect, 0);
	if (osd && (m_lcd[LCD_OSDC] & LCDOSDC_F1EN))
		draw_foreground(bitmap, cliprect, 1);
	return 0;
}


/***************************************************************************
    AUDIO INTERFACE AND INTERNAL CODEC

    Playback through the internal codec is modelled; recording and the
    AC-link side are not.  The sound stream drains the 32-entry transmit
    FIFO at the codec's DAC rate, and whenever the level falls to the
    AICFR threshold the DMA channel with the AIC transmit request type is
    run for one burst.  A DMA buffer therefore takes as long to send as
    it does to play, and its completion interrupt arrives when the last
    burst has left memory, which is what paces the software.
***************************************************************************/

namespace {

constexpr uint32_t AICFR_LSMP = 1 << 6;
constexpr uint32_t AICFR_ICDC = 1 << 5;
constexpr uint32_t AICFR_RST  = 1 << 3;
constexpr uint32_t AICFR_ENB  = 1 << 0;

constexpr uint32_t AICCR_TDMS   = 1 << 14;
constexpr uint32_t AICCR_M2S    = 1 << 11;
constexpr uint32_t AICCR_ENDSW  = 1 << 10;
constexpr uint32_t AICCR_ASVTSU = 1 << 9;
constexpr uint32_t AICCR_TFLUSH = 1 << 8;
constexpr uint32_t AICCR_EROR   = 1 << 6;
constexpr uint32_t AICCR_ETUR   = 1 << 5;
constexpr uint32_t AICCR_ERFS   = 1 << 4;
constexpr uint32_t AICCR_ETFS   = 1 << 3;
constexpr uint32_t AICCR_ERPL   = 1 << 1;

constexpr uint32_t AICSR_ROR = 1 << 6;
constexpr uint32_t AICSR_TUR = 1 << 5;
constexpr uint32_t AICSR_TFS = 1 << 3;

constexpr uint32_t RGADW_RGWR = 1 << 16;

constexpr unsigned CODEC_CCR2 = 0x04;

// codec DAC sample rates selected by CCR2.DFREQ
constexpr uint32_t CODEC_RATES[16] =
{
	96000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 9600, 8000, 8000, 8000, 8000, 8000, 8000
};

} // anonymous namespace

uint32_t jz4755_device::aic_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x14 / 4:    // AICSR
		{
			m_stream->update();
			uint32_t status = (m_aic[offset] & (AICSR_ROR | AICSR_TUR)) | (uint32_t(m_aic_txlevel) << 8);
			if (aic_tx_service())
				status |= AICSR_TFS;
			return status;
		}
		case 0x1c / 4:    // I2SSR: BSY while a frame is going out
			m_stream->update();
			return ((m_aic[0x04 / 4] & AICCR_ERPL) && m_aic_txlevel != 0) ? 0x04 : 0x00;
		case 0x34 / 4:    // AICDR: nothing is ever recorded
			return 0;
		case 0xa4 / 4:    // RGADW: the codec write has completed
			return m_aic[offset] & 0x7fff;
		case 0xa8 / 4:    // RGDATA: codec register readback
			return m_codec[(m_aic[0xa4 / 4] >> 8) & 0x7f];
	}
	return m_aic[offset & 0x3f];
}

void jz4755_device::aic_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset != 0x34 / 4)    // not the FIFO
		LOGMASKED(LOG_STUB, "%s: AIC reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
	switch (offset)
	{
		case 0x00 / 4:    // AICFR
			m_stream->update();
			if (data & AICFR_RST)
				aic_reset();
			m_aic[offset] = data & 0xff77;
			aic_set_rate();
			aic_update();
			return;
		case 0x04 / 4:    // AICCR
			m_stream->update();
			if (data & AICCR_TFLUSH)
				m_aic_txlevel = 0;
			m_aic[offset] = data & 0x003fce7f;
			aic_update();
			return;
		case 0x14 / 4:    // AICSR: writing clears the under-run and over-run flags
			m_aic[offset] = 0;
			aic_update();
			return;
		case 0x34 / 4:    // AICDR
			m_stream->update();
			aic_tx_push(data);
			aic_update();
			return;
		case 0xa4 / 4:    // RGADW: internal codec register access
			if (data & RGADW_RGWR)
			{
				const unsigned reg = (data >> 8) & 0x7f;
				m_codec[reg] = data & 0xff;
				if (reg == CODEC_CCR2)
					aic_set_rate();
			}
			m_aic[offset] = data & 0x7fff;
			return;
	}
	COMBINE_DATA(&m_aic[offset & 0x3f]);
}

// AICFR.RST: everything but AICFR and I2SDIV goes back to its reset state
void jz4755_device::aic_reset()
{
	for (unsigned i = 1; i < 0x40; i++)
		if (i != 0x30 / 4)
			m_aic[i] = 0;
	m_aic[0x04 / 4] = 0x00240000;    // 24-bit samples
	m_aic_txrd = 0;
	m_aic_txlevel = 0;
	m_aic_timer->adjust(attotime::never);
}

void jz4755_device::aic_set_rate()
{
	// only the internal codec's clocking is modelled
	const uint32_t rate = (m_aic[0x00 / 4] & AICFR_ICDC) ? CODEC_RATES[m_codec[CODEC_CCR2] >> 4] : 48000;
	if (rate != m_stream->sample_rate())
		m_stream->set_sample_rate(rate);
}

void jz4755_device::aic_tx_push(uint32_t data)
{
	if (m_aic_txlevel == AIC_FIFO_DEPTH)
	{
		LOGMASKED(LOG_STUB, "AIC: transmit FIFO overflow\n");
		return;
	}
	m_aic_txfifo[(m_aic_txrd + m_aic_txlevel) % AIC_FIFO_DEPTH] = data;
	m_aic_txlevel++;
}

// TFS: the transmit FIFO level is at or below the AICFR.TFTH threshold
bool jz4755_device::aic_tx_service() const
{
	return m_aic_txlevel <= ((m_aic[0x00 / 4] >> 8) & 0xf) * 2;
}

// a FIFO entry as a signed 16-bit sample according to AICCR's output format
int16_t jz4755_device::aic_tx_sample(uint32_t raw) const
{
	static const uint8_t sizes[8] = { 8, 16, 18, 20, 24, 24, 24, 24 };
	const uint32_t cr = m_aic[0x04 / 4];
	const unsigned bits = sizes[(cr >> 19) & 7];
	if (bits == 16 && (cr & AICCR_ENDSW))
		raw = swapendian_int16(uint16_t(raw));
	if (cr & AICCR_ASVTSU)
		raw ^= 1 << (bits - 1);
	return int16_t(int32_t(raw << (32 - bits)) >> 16);
}

// recompute the interrupt and the wake-up for the DMA burst in flight
void jz4755_device::aic_update()
{
	const uint32_t fr = m_aic[0x00 / 4];
	const uint32_t cr = m_aic[0x04 / 4];
	const uint32_t sr = m_aic[0x14 / 4];
	set_irq(IRQ_AIC, ((cr & AICCR_ETFS) && aic_tx_service()) || ((cr & AICCR_ETUR) && (sr & AICSR_TUR)));

	attotime wake = attotime::never;
	if ((fr & AICFR_ENB) && (cr & AICCR_ERPL) && (cr & AICCR_TDMS))
	{
		for (const dma_channel_t &ch : m_dma)
		{
			if (ch.active && ch.drt == DRT_AIC_TX)
			{
				// entries the DAC must consume before the last burst is pulled, and a little margin
				static const uint8_t unit_sizes[8] = { 4, 1, 2, 16, 32, 4, 4, 4 };
				static const uint8_t port_widths[4] = { 4, 1, 2, 4 };
				const unsigned per_unit = unit_sizes[(ch.dcm >> 8) & 7] / port_widths[(ch.dcm >> 12) & 3];
				const int64_t pending = int64_t(m_aic_txlevel) + int64_t(ch.dtc - 1) * per_unit - int64_t((fr >> 8) & 0xf) * 2;
				const uint32_t per_second = m_stream->sample_rate() * ((cr & AICCR_M2S) ? 1 : 2);
				wake = attotime::from_ticks(std::max<int64_t>(pending, 0) + 1, per_second);
				break;
			}
		}
	}
	m_aic_timer->adjust(wake);
}

TIMER_CALLBACK_MEMBER(jz4755_device::aic_tick)
{
	// the stream update pulls the next burst and completes the DMA, then re-arms this timer
	m_stream->update();
}

void jz4755_device::sound_stream_update(sound_stream &stream)
{
	const uint32_t fr = m_aic[0x00 / 4];
	const uint32_t cr = m_aic[0x04 / 4];
	const bool playing = (fr & AICFR_ENB) && (cr & AICCR_ERPL);
	const unsigned channels = (cr & AICCR_M2S) ? 1 : 2;

	for (int i = 0; i < stream.samples(); i++)
	{
		if (playing)
		{
			for (unsigned ch = 0; ch < channels; ch++)
			{
				if (m_aic_txlevel != 0)
				{
					m_aic_out[ch] = aic_tx_sample(m_aic_txfifo[m_aic_txrd]);
					m_aic_txrd = (m_aic_txrd + 1) % AIC_FIFO_DEPTH;
					m_aic_txlevel--;
				}
				else
				{
					// under-run: the last sample or silence goes out again
					m_aic[0x14 / 4] |= AICSR_TUR;
					if (!(fr & AICFR_LSMP))
						m_aic_out[ch] = 0;
				}
			}
			if (channels == 1)
				m_aic_out[1] = m_aic_out[0];
			if ((cr & AICCR_TDMS) && aic_tx_service())
				dma_service(DRT_AIC_TX);
		}
		stream.put_int(0, i, m_aic_out[0], 32768);
		stream.put_int(1, i, m_aic_out[1], 32768);
	}
	aic_update();
}


/***************************************************************************
    REGISTER-STORAGE STUBS
***************************************************************************/

template <unsigned N> uint32_t jz4755_device::msc_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == 0x04 / 4)
		return m_msc[N][offset] | 0x00000040;    // DATA_FIFO_EMPTY
	return m_msc[N][offset & 0x1f];
}

template <unsigned N> void jz4755_device::msc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_STUB, "%s: MSC%u reg %02x = %08x\n", machine().describe_context(), N, offset * 4, data);
	if (offset == 0)
	{
		// STRPCL: a started command finds no card and times out
		if (data & 0x04)
		{
			m_msc[N][0x04 / 4] |= 0x00000802;    // END_CMD_RES | TIME_OUT_RES
			m_msc[N][0x28 / 4] |= 0x00000004;    // IREG END_CMD_RES
		}
		if (data & 0x08)
			m_msc[N][0x04 / 4] = 0x00000040;
		return;
	}
	if (offset == 0x28 / 4)
	{
		// IREG: write 1 to clear
		m_msc[N][offset] &= ~data;
		return;
	}
	COMBINE_DATA(&m_msc[N][offset & 0x1f]);
}

template uint32_t jz4755_device::msc_r<0>(offs_t, uint32_t);
template uint32_t jz4755_device::msc_r<1>(offs_t, uint32_t);
template void jz4755_device::msc_w<0>(offs_t, uint32_t, uint32_t);
template void jz4755_device::msc_w<1>(offs_t, uint32_t, uint32_t);

// I2C: nothing answers on the bus, so a master read returns 0xff and every transfer completes at once
uint32_t jz4755_device::i2c_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0:    // I2CDR
			return m_i2c_reading ? 0xff : m_i2c[0];
		case 2:    // I2CSR
		{
			uint32_t sr = m_i2c[2] | I2CSR_TEND;
			if (!machine().side_effects_disabled())
			{
				// a transmitted byte has gone; a receive buffer fills up
				if (m_i2c_reading && !m_i2c_address_phase)
					m_i2c[2] |= I2CSR_DRF;
				else
					m_i2c[2] &= ~I2CSR_DRF;
			}
			return sr;
		}
	}
	return m_i2c[offset & 3];
}

void jz4755_device::i2c_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_STUB, "%s: I2C reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
	switch (offset)
	{
		case 0:    // I2CDR
			if (m_i2c_address_phase)
			{
				m_i2c_reading = BIT(data, 0);
				m_i2c_address_phase = false;
			}
			m_i2c[0] = data & 0xff;
			break;
		case 1:    // I2CCR
			if (data & I2CCR_STA)
			{
				m_i2c_address_phase = true;
				m_i2c_reading = false;
			}
			if (data & I2CCR_STO)
			{
				m_i2c_address_phase = false;
				m_i2c_reading = false;
			}
			m_i2c[1] = data & 0x1f;
			break;
		case 2:    // I2CSR: DRF is the only writable bit
			m_i2c[2] = (m_i2c[2] & ~I2CSR_DRF) | (data & I2CSR_DRF);
			break;
		default:
			m_i2c[offset & 3] = data;
			break;
	}
}

uint32_t jz4755_device::sadc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x0c / 4: return m_sadc[offset] | 0x02;    // ADSTATE: battery data ready
		case 0x1c / 4: return 0x0800;                   // ADBDAT: a healthy battery level
	}
	return m_sadc[offset & 0xf];
}

void jz4755_device::sadc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_STUB, "%s: SADC reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
	if (offset == 0x0c / 4)
	{
		// flags clear on write of 1
		m_sadc[offset] &= ~data;
		return;
	}
	COMBINE_DATA(&m_sadc[offset & 0xf]);
}

/***************************************************************************
    DMA CONTROLLER

    Twelve channels in two groups of six.  Auto, NAND and BCH requests
    run to completion as soon as the channel is enabled, since nothing
    they touch has any timing of its own.  A channel with the AIC
    transmit request type moves one burst each time the transmit FIFO
    drops to its threshold and so completes in real time.  Descriptor
    chains are followed; other peripheral request types are not used by
    the known software and are treated like auto requests.
***************************************************************************/

namespace {

constexpr uint32_t DCS_NDES = 1 << 31;
constexpr uint32_t DCS_DES8 = 1 << 30;
constexpr uint32_t DCS_INV  = 1 << 6;
constexpr uint32_t DCS_AR   = 1 << 4;
constexpr uint32_t DCS_TT   = 1 << 3;
constexpr uint32_t DCS_CT   = 1 << 1;
constexpr uint32_t DCS_CTE  = 1 << 0;
constexpr uint32_t DCM_BLAST = 1 << 25;
constexpr uint32_t DCM_SAI  = 1 << 23;
constexpr uint32_t DCM_DAI  = 1 << 22;
constexpr uint32_t DCM_V    = 1 << 4;
constexpr uint32_t DCM_VM   = 1 << 3;
constexpr uint32_t DCM_TIE  = 1 << 1;
constexpr uint32_t DCM_LINK = 1 << 0;
constexpr uint32_t DMAC_DMAE = 1 << 0;

constexpr uint8_t DMA_UNIT_SIZES[8] = { 4, 1, 2, 16, 32, 4, 4, 4 };
constexpr uint8_t DMA_PORT_WIDTHS[4] = { 4, 1, 2, 4 };

} // anonymous namespace

uint32_t jz4755_device::dmac_r(offs_t offset, uint32_t mem_mask)
{
	const unsigned group = (offset >> 6) & 1;    // 0x100 apart
	if ((offset & ~0x7f) == 0x000 / 4 || (offset & ~0x7f) == 0x100 / 4)
	{
		const unsigned idx = offset & 0x3f;
		if (idx < 0x30)
		{
			dma_sync_aic();
			const dma_channel_t &ch = m_dma[group * 6 + idx / 8];
			switch (idx & 7)
			{
				case 0: return ch.dsa;
				case 1: return ch.dta;
				case 2: return ch.dtc;
				case 3: return ch.drt;
				case 4: return ch.dcs;
				case 5: return ch.dcm;
				case 6: return ch.dda;
			}
			return 0;
		}
		if (idx >= 0x30 && idx < 0x36)
			return m_dma[group * 6 + idx - 0x30].dsd;
	}
	switch (offset)
	{
		case 0x300 / 4: return m_dmac_ctrl[0];
		case 0x304 / 4: dma_sync_aic(); return m_dirqp[0];
		case 0x308 / 4: return m_ddr[0];
		case 0x310 / 4: return m_dcke[0];
		case 0x400 / 4: return m_dmac_ctrl[1];
		case 0x404 / 4: dma_sync_aic(); return m_dirqp[1];
		case 0x408 / 4: return m_ddr[1];
		case 0x410 / 4: return m_dcke[1];
	}
	LOGMASKED(LOG_DMA, "%s: DMAC read of unknown reg %03x\n", machine().describe_context(), offset * 4);
	return 0;
}

void jz4755_device::dmac_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: DMAC reg %03x = %08x\n", machine().describe_context(), offset * 4, data);
	const unsigned group = (offset >> 6) & 1;
	if ((offset & ~0x7f) == 0x000 / 4 || (offset & ~0x7f) == 0x100 / 4)
	{
		const unsigned idx = offset & 0x3f;
		if (idx < 0x30)
		{
			const unsigned chnum = group * 6 + idx / 8;
			dma_channel_t &ch = m_dma[chnum];
			switch (idx & 7)
			{
				case 0: ch.dsa = data; break;
				case 1: ch.dta = data; break;
				case 2: ch.dtc = data & 0xffffff; break;
				case 3: ch.drt = data & 0x3f; break;
				case 4:
					ch.dcs = data;
					if (!(data & DCS_CTE))
						ch.active = false;
					dma_update_irq();
					if (data & DCS_CTE)
						dma_start(chnum);
					break;
				case 5: ch.dcm = data; dma_update_irq(); break;
				case 6: ch.dda = data; break;
			}
			return;
		}
		if (idx >= 0x30 && idx < 0x36)
		{
			m_dma[group * 6 + idx - 0x30].dsd = data;
			return;
		}
	}
	switch (offset)
	{
		case 0x300 / 4:
		case 0x400 / 4:
		{
			const unsigned g = (offset == 0x400 / 4) ? 1 : 0;
			m_dmac_ctrl[g] = data;
			if (data & DMAC_DMAE)
			{
				// BCH channels consume buffers the other channels fill, so they go last
				for (unsigned pass = 0; pass < 2; pass++)
					for (unsigned n = 0; n < 6; n++)
					{
						const unsigned drt = m_dma[g * 6 + n].drt;
						if ((drt == DRT_BCH_ENC || drt == DRT_BCH_DEC) == (pass == 1))
							dma_start(g * 6 + n);
					}
			}
			return;
		}
		case 0x304 / 4:
		case 0x404 / 4:
			// pending status is read-only here; it clears with the channel's TT
			return;
		case 0x308 / 4: m_ddr[0] = data & 0x3f; return;
		case 0x408 / 4: m_ddr[1] = data & 0x3f; return;
		case 0x30c / 4:
		case 0x40c / 4:
		{
			const unsigned g = (offset == 0x40c / 4) ? 1 : 0;
			m_ddr[g] |= data & 0x3f;
			for (unsigned n = 0; n < 6; n++)
				if (BIT(data, n))
					dma_start(g * 6 + n);
			return;
		}
		case 0x310 / 4: m_dcke[0] = data & 0x3f; return;
		case 0x410 / 4: m_dcke[1] = data & 0x3f; return;
	}
	LOGMASKED(LOG_DMA, "%s: DMAC write to unknown reg %03x = %08x\n", machine().describe_context(), offset * 4, data);
}

// the pending bits follow each channel's TT (with TIE) rather than latching separately
void jz4755_device::dma_update_irq()
{
	for (unsigned group = 0; group < 2; group++)
	{
		m_dirqp[group] = 0;
		for (unsigned n = 0; n < 6; n++)
		{
			const dma_channel_t &ch = m_dma[group * 6 + n];
			if ((ch.dcs & DCS_TT) && (ch.dcm & DCM_TIE))
				m_dirqp[group] |= 1U << n;
		}
	}
	set_irq(IRQ_DMA0, m_dirqp[0] != 0);
	set_irq(IRQ_DMA1, m_dirqp[1] != 0);
}

// bring an audio transfer up to date before its registers are read
void jz4755_device::dma_sync_aic()
{
	for (const dma_channel_t &ch : m_dma)
	{
		if (ch.active && ch.drt == DRT_AIC_TX)
		{
			m_stream->update();
			return;
		}
	}
}

// read the descriptor DDA points at into the channel registers; false if it is marked invalid
bool jz4755_device::dma_load_descriptor(unsigned chnum)
{
	dma_channel_t &ch = m_dma[chnum];
	const uint32_t addr = ch.dda & ~0xf;
	uint32_t desc[8];
	for (unsigned i = 0; i < ((ch.dcs & DCS_DES8) ? 8 : 4); i++)
		desc[i] = read_phys(addr + i * 4);
	ch.dcm = desc[0];
	ch.dsa = desc[1];
	ch.dta = desc[2];
	ch.dtc = desc[3] & 0xffffff;
	ch.dcs = (ch.dcs & ~0x00ff0000) | ((desc[3] >> 24) << 16);    // CDOA
	if (ch.dcs & DCS_DES8)
	{
		ch.dsd = desc[4];
		ch.drt = desc[5] & 0x3f;
	}
	if ((ch.dcm & DCM_VM) && !(ch.dcm & DCM_V))
	{
		ch.dcs |= DCS_INV;
		return false;
	}
	LOGMASKED(LOG_DMA, "DMA%u: descriptor %08x: %08x -> %08x, %u x %u bytes (cmd %08x, request %u)\n", chnum, addr, ch.dsa, ch.dta,
			ch.dtc, DMA_UNIT_SIZES[(ch.dcm >> 8) & 7], ch.dcm, ch.drt);
	return true;
}

// move one DTC unit; a port with a fixed address sees every item at that address
void jz4755_device::dma_unit(unsigned chnum)
{
	dma_channel_t &ch = m_dma[chnum];
	const unsigned unit = DMA_UNIT_SIZES[(ch.dcm >> 8) & 7];
	const unsigned swidth = DMA_PORT_WIDTHS[(ch.dcm >> 14) & 3];
	const unsigned dwidth = DMA_PORT_WIDTHS[(ch.dcm >> 12) & 3];
	const bool bch = (ch.drt == DRT_BCH_ENC || ch.drt == DRT_BCH_DEC);
	address_space &space = m_cpu->space(AS_PROGRAM);

	uint8_t buf[32];
	for (unsigned i = 0; i < unit; i += swidth)
	{
		const uint32_t addr = (ch.dcm & DCM_SAI) ? ch.dsa + i : ch.dsa;
		switch (swidth)
		{
			case 1: buf[i] = space.read_byte(addr); break;
			case 2: put_u16le(&buf[i], space.read_word(addr)); break;
			default: put_u32le(&buf[i], space.read_dword(addr)); break;
		}
	}

	if (bch)
	{
		// BCH requests feed the engine's data register; the target only receives the result
		for (unsigned i = 0; i < unit; i++)
			bch_data_w(buf[i]);
	}
	else if (ch.drt == DRT_AIC_TX)
	{
		// one FIFO entry per item at the port width
		for (unsigned i = 0; i < unit; i += dwidth)
		{
			switch (dwidth)
			{
				case 1: aic_tx_push(buf[i]); break;
				case 2: aic_tx_push(get_u16le(&buf[i])); break;
				default: aic_tx_push(get_u32le(&buf[i])); break;
			}
		}
	}
	else
	{
		for (unsigned i = 0; i < unit; i += dwidth)
		{
			const uint32_t addr = (ch.dcm & DCM_DAI) ? ch.dta + i : ch.dta;
			switch (dwidth)
			{
				case 1: space.write_byte(addr, buf[i]); break;
				case 2: space.write_word(addr, get_u16le(&buf[i])); break;
				default: space.write_dword(addr, get_u32le(&buf[i])); break;
			}
		}
	}

	if (ch.dcm & DCM_SAI)
		ch.dsa += unit;
	if ((ch.dcm & DCM_DAI) && !bch)
		ch.dta += unit;
	ch.dtc--;
}

// run the current block as far as the request type allows; false if it is waiting on its peripheral
bool jz4755_device::dma_transfer(unsigned chnum)
{
	dma_channel_t &ch = m_dma[chnum];
	const bool bch = (ch.drt == DRT_BCH_ENC || ch.drt == DRT_BCH_DEC);

	while (ch.dtc != 0)
	{
		if (ch.drt == DRT_AIC_TX && !aic_tx_service())
			return false;
		dma_unit(chnum);
	}

	if (bch && ((ch.dcm & DCM_BLAST) || (ch.dcs & DCS_NDES)))
	{
		// end of a block; afterwards the engine resets
		address_space &space = m_cpu->space(AS_PROGRAM);
		if (ch.drt == DRT_BCH_ENC)
		{
			// Encoding leaves just the parity bytes at the target, in the order they are stored in
			// the spare area.  The manual says BHINT comes first, but the firmware lays its targets
			// 13 bytes apart and copies the result straight into the page, and the parity in the
			// NAND dump sits exactly there.
			const unsigned bytes = (m_bhcr & BHCR_BSEL) ? 13 : 7;
			for (unsigned n = 0; n < bytes; n++)
				space.write_byte(ch.dta + n, m_bhpar[n / 4] >> ((n % 4) * 8));
		}
		else
		{
			// decoding reports BHINT, followed by the error indices when there were any
			space.write_dword(ch.dta, m_bhint);
			if (m_bhint & BHINT_ERR)
			{
				for (unsigned n = 0; n < 4; n++)
					space.write_dword(ch.dta + 4 + n * 4, m_bherr[n]);
			}
		}
		LOGMASKED(LOG_DMA, "DMA%u: BCH block done, status %08x, result written to %08x\n", chnum, m_bhint, ch.dta);
		bch_reset();
	}
	return true;
}

// begin a transfer on a newly enabled channel
void jz4755_device::dma_start(unsigned chnum)
{
	const unsigned group = chnum / 6;
	dma_channel_t &ch = m_dma[chnum];
	if (!(m_dmac_ctrl[group] & DMAC_DMAE) || !(ch.dcs & DCS_CTE) || (ch.dcs & (DCS_TT | DCS_AR)) || ch.active)
		return;

	if (ch.dcs & DCS_NDES)
	{
		// no-descriptor transfer: the registers describe the whole block
		if (ch.dtc == 0)
			return;
		LOGMASKED(LOG_DMA, "DMA%u: %08x -> %08x, %u x %u bytes (cmd %08x, request %u)\n", chnum, ch.dsa, ch.dta, ch.dtc,
				DMA_UNIT_SIZES[(ch.dcm >> 8) & 7], ch.dcm, ch.drt);
	}
	else
	{
		// descriptor chain, started by the doorbell
		if (!BIT(m_ddr[group], chnum % 6))
			return;
		m_ddr[group] &= ~(1U << (chnum % 6));
		if (!dma_load_descriptor(chnum))
		{
			ch.dcs |= DCS_TT;
			dma_update_irq();
			return;
		}
	}

	ch.active = true;
	if (ch.drt == DRT_AIC_TX)
		m_stream->update();    // the FIFO level must be current before the first burst
	dma_run(chnum);
}

// continue an active transfer until it completes or has to wait for its peripheral
void jz4755_device::dma_run(unsigned chnum)
{
	dma_channel_t &ch = m_dma[chnum];
	address_space &space = m_cpu->space(AS_PROGRAM);
	for (unsigned link = 0; ch.active; link++)
	{
		if (!dma_transfer(chnum))
			return;
		if (ch.dcs & DCS_NDES)
			break;

		// a finished descriptor is marked consumed, and the chain may go on
		const uint32_t addr = ch.dda & ~0xf;
		if (ch.dcm & DCM_VM)
			space.write_dword(addr, ch.dcm & ~DCM_V);
		if (!(ch.dcm & DCM_LINK) || link >= 1024)
			break;
		ch.dcs |= DCS_CT;
		ch.dda = (ch.dda & 0xfffff000) | (((ch.dcs >> 16) & 0xff) << 4);
		if (!dma_load_descriptor(chnum))
			break;
	}

	ch.active = false;
	ch.dcs |= DCS_TT;
	dma_update_irq();
}

// a peripheral has room (or data): give every waiting channel of its request type another go
void jz4755_device::dma_service(unsigned drt)
{
	for (unsigned chnum = 0; chnum < DMA_CHANNELS; chnum++)
	{
		if (m_dma[chnum].active && m_dma[chnum].drt == drt)
			dma_run(chnum);
	}
}

/***************************************************************************
    IMAGE PROCESSING UNIT

    A scaler and colour space converter that writes to memory or streams
    straight into LCD foreground 1.  Resizing uses a plain ratio rather
    than the coefficient tables, and a frame takes a few microseconds per
    thousand output pixels so the firmware's frame loop is paced.
***************************************************************************/

namespace {

enum : unsigned
{
	IPU_CONTROL  = 0x00 / 4,
	IPU_STATUS   = 0x04 / 4,
	IPU_D_FMT    = 0x08 / 4,
	IPU_Y_ADDR   = 0x0c / 4,
	IPU_U_ADDR   = 0x10 / 4,
	IPU_V_ADDR   = 0x14 / 4,
	IPU_IN_FM_GS = 0x18 / 4,
	IPU_Y_STRIDE = 0x1c / 4,
	IPU_UV_STRIDE = 0x20 / 4,
	IPU_OUT_ADDR = 0x24 / 4,
	IPU_OUT_GS   = 0x28 / 4,
	IPU_OUT_STRIDE = 0x2c / 4
};

constexpr uint32_t IPU_CTRL_CHIP_EN   = 1 << 0;
constexpr uint32_t IPU_CTRL_RUN       = 1 << 1;
constexpr uint32_t IPU_CTRL_CSC_EN    = 1 << 4;
constexpr uint32_t IPU_CTRL_FM_IRQ_EN = 1 << 5;
constexpr uint32_t IPU_CTRL_RST       = 1 << 6;
constexpr uint32_t IPU_CTRL_STOP      = 1 << 7;
constexpr uint32_t IPU_CTRL_SPKG_SEL  = 1 << 10;
constexpr uint32_t IPU_CTRL_LCDC_SEL  = 1 << 11;
constexpr uint32_t IPU_STATUS_OUT_END = 1 << 0;

} // anonymous namespace

uint32_t jz4755_device::ipu_r(offs_t offset, uint32_t mem_mask)
{
	return m_ipu[offset & 0x3f];
}

void jz4755_device::ipu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case IPU_CONTROL:
			m_ipu[IPU_CONTROL] = data & ~(IPU_CTRL_RST | IPU_CTRL_STOP);
			if (data & IPU_CTRL_RST)
			{
				m_ipu_timer->adjust(attotime::never);
				m_ipu_frame_pending = false;
				m_ipu[IPU_CONTROL] &= ~IPU_CTRL_RUN;
				m_ipu[IPU_STATUS] = IPU_STATUS_OUT_END;
			}
			if (data & IPU_CTRL_STOP)
			{
				m_ipu_timer->adjust(attotime::never);
				m_ipu_frame_pending = false;
				m_ipu[IPU_CONTROL] &= ~IPU_CTRL_RUN;
			}
			if ((data & IPU_CTRL_RUN) && (data & IPU_CTRL_CHIP_EN) && !m_ipu_frame_pending && m_ipu_timer->remaining() == attotime::never)
				ipu_run();
			return;

		case IPU_STATUS:
			m_ipu[IPU_STATUS] = data & IPU_STATUS_OUT_END;
			if (!(data & IPU_STATUS_OUT_END))
				set_irq(IRQ_IPU, false);
			return;
	}
	LOGMASKED(LOG_IPU, "%s: IPU reg %02x = %08x\n", machine().describe_context(), offset * 4, data);
	COMBINE_DATA(&m_ipu[offset & 0x3f]);
}

void jz4755_device::ipu_run()
{
	const uint32_t ctrl = m_ipu[IPU_CONTROL];
	const uint32_t fmt = m_ipu[IPU_D_FMT];
	const unsigned in_w = (m_ipu[IPU_IN_FM_GS] >> 16) & 0xfff;
	const unsigned in_h = m_ipu[IPU_IN_FM_GS] & 0xfff;
	const unsigned out_fmt = (fmt >> 19) & 3;         // 0 RGB555, 1 RGB565, 2 RGB888, 3 YUV422
	const unsigned out_bpp = (out_fmt == 2) ? (BIT(fmt, 24) ? 3 : 4) : 2;
	const unsigned out_w = ((m_ipu[IPU_OUT_GS] >> 16) & 0xfff) / out_bpp;
	const unsigned out_h = m_ipu[IPU_OUT_GS] & 0xfff;
	const unsigned in_fmt = fmt & 3;                   // 0 YUV420, 1 YUV422, 2 YUV444, 3 YUV411
	const bool packed = (ctrl & IPU_CTRL_SPKG_SEL) != 0;
	const bool to_lcd = (ctrl & IPU_CTRL_LCDC_SEL) != 0;
	const unsigned order = (fmt >> 21) & 7;

	if (in_w == 0 || in_h == 0 || out_w == 0 || out_h == 0 || out_w > 2048 || out_h > 2048)
	{
		m_ipu[IPU_STATUS] |= 1 << 2;    // SIZE_ERR
		m_ipu[IPU_CONTROL] &= ~IPU_CTRL_RUN;
		return;
	}

	address_space &space = m_cpu->space(AS_PROGRAM);
	const uint32_t y_addr = m_ipu[IPU_Y_ADDR] & 0x1fffffff;
	const uint32_t u_addr = m_ipu[IPU_U_ADDR] & 0x1fffffff;
	const uint32_t v_addr = m_ipu[IPU_V_ADDR] & 0x1fffffff;
	const uint32_t y_stride = m_ipu[IPU_Y_STRIDE] & 0xffff;
	const uint32_t u_stride = m_ipu[IPU_UV_STRIDE] & 0xffff;
	const uint32_t v_stride = (m_ipu[IPU_UV_STRIDE] >> 16) & 0xffff;
	const uint32_t out_addr = m_ipu[IPU_OUT_ADDR] & 0x1fffffff;
	const uint32_t out_stride = m_ipu[IPU_OUT_STRIDE] & 0xffff;

	if (to_lcd)
	{
		m_ipu_frame_w = out_w;
		m_ipu_frame_h = out_h;
		m_ipu_frame.resize(out_w * out_h);
	}

	LOGMASKED(LOG_IPU, "IPU: %ux%u format %u at %08x/%08x/%08x -> %ux%u format %u %s, control %08x\n",
		in_w, in_h, in_fmt, y_addr, u_addr, v_addr, out_w, out_h, out_fmt, to_lcd ? "to LCD" : "to memory", ctrl);

	for (unsigned y = 0; y < out_h; y++)
	{
		const unsigned sy = (uint64_t(y) * in_h) / out_h;
		for (unsigned x = 0; x < out_w; x++)
		{
			const unsigned sx = (uint64_t(x) * in_w) / out_w;
			unsigned yy, uu, vv;
			if (packed || in_fmt == 1)
			{
				// Y1 U Y0 V style pairs, ordering from IN_OFT
				const uint32_t word = space.read_dword(y_addr + sy * y_stride + (sx & ~1) * 2);
				static const uint8_t y0_pos[4] = { 2, 2, 3, 3 }, y1_pos[4] = { 0, 0, 1, 1 }, u_pos[4] = { 1, 3, 0, 2 }, v_pos[4] = { 3, 1, 2, 0 };
				const unsigned oft = (fmt >> 2) & 3;
				yy = (word >> (8 * ((sx & 1) ? y1_pos[oft] : y0_pos[oft]))) & 0xff;
				uu = (word >> (8 * u_pos[oft])) & 0xff;
				vv = (word >> (8 * v_pos[oft])) & 0xff;
			}
			else
			{
				const unsigned cx = (in_fmt == 2) ? sx : (in_fmt == 3) ? (sx / 4) : (sx / 2);
				const unsigned cy = (in_fmt == 0) ? (sy / 2) : sy;
				yy = space.read_byte(y_addr + sy * y_stride + sx);
				uu = space.read_byte(u_addr + cy * u_stride + cx);
				vv = space.read_byte(v_addr + cy * v_stride + cx);
			}

			unsigned r, g, b;
			if (ctrl & IPU_CTRL_CSC_EN)
			{
				const int c = int(yy) - 16, d = int(uu) - 128, e = int(vv) - 128;
				r = std::clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
				g = std::clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
				b = std::clamp((298 * c + 516 * d + 128) >> 8, 0, 255);
			}
			else
			{
				r = yy;
				g = uu;
				b = vv;
			}
			// output component order
			unsigned c0 = r, c1 = g, c2 = b;
			switch (order)
			{
				case 1: c1 = b; c2 = g; break;
				case 2: c0 = g; c1 = b; c2 = r; break;
				case 3: c0 = g; c1 = r; break;
				case 4: c0 = b; c1 = r; c2 = g; break;
				case 5: c0 = b; c2 = r; break;
			}

			if (to_lcd)
				m_ipu_frame[y * out_w + x] = (c0 << 16) | (c1 << 8) | c2;
			else
			{
				const uint32_t dst = out_addr + y * out_stride + x * out_bpp;
				switch (out_fmt)
				{
					case 0: space.write_word(dst, ((c0 >> 3) << 10) | ((c1 >> 3) << 5) | (c2 >> 3)); break;
					case 1: space.write_word(dst, ((c0 >> 3) << 11) | ((c1 >> 2) << 5) | (c2 >> 3)); break;
					case 2:
						if (out_bpp == 3)
						{
							space.write_byte(dst, c2);
							space.write_byte(dst + 1, c1);
							space.write_byte(dst + 2, c0);
						}
						else
							space.write_dword(dst, (c0 << 16) | (c1 << 8) | c2);
						break;
					default:
						space.write_word(dst, (yy << 8) | ((x & 1) ? vv : uu));
						break;
				}
			}
		}
	}

	// completion: frames for the LCD controller are taken at its next frame, others take a few hundred megapixels per second
	if (to_lcd)
		m_ipu_frame_pending = true;
	else
		m_ipu_timer->adjust(attotime::from_nsec(20000 + uint64_t(out_w) * out_h * 4));
}

TIMER_CALLBACK_MEMBER(jz4755_device::ipu_done)
{
	m_ipu[IPU_CONTROL] &= ~IPU_CTRL_RUN;
	m_ipu[IPU_STATUS] |= IPU_STATUS_OUT_END;
	if (m_ipu[IPU_CONTROL] & IPU_CTRL_FM_IRQ_EN)
		set_irq(IRQ_IPU, true);
}


template <unsigned N> uint32_t jz4755_device::stub_r(offs_t offset, uint32_t mem_mask)
{
	static const char *const names[8] = { "HARB", "SSI0", "SSI1", "OWI", "TSSI", "UDC", "CIM", "VPU" };
	LOGMASKED(LOG_STUB, "%s: %s read of reg %05x\n", machine().describe_context(), names[N], offset * 4);
	return m_stub[N][offset & 0xff];
}

template <unsigned N> void jz4755_device::stub_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static const char *const names[8] = { "HARB", "SSI0", "SSI1", "OWI", "TSSI", "UDC", "CIM", "VPU" };
	LOGMASKED(LOG_STUB, "%s: %s reg %05x = %08x\n", machine().describe_context(), names[N], offset * 4, data);
	COMBINE_DATA(&m_stub[N][offset & 0xff]);
}

template uint32_t jz4755_device::stub_r<0>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<1>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<2>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<3>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<4>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<5>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<6>(offs_t, uint32_t);
template uint32_t jz4755_device::stub_r<7>(offs_t, uint32_t);
template void jz4755_device::stub_w<0>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<1>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<2>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<3>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<4>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<5>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<6>(offs_t, uint32_t, uint32_t);
template void jz4755_device::stub_w<7>(offs_t, uint32_t, uint32_t);
