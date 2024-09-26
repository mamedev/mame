// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************************************************************
 *
 * Intel XScale PXA255 peripheral emulation
 *
 * TODO:
 *   Most things
 *
 **************************************************************************/

#ifndef MAME_MACHINE_PXA255
#define MAME_MACHINE_PXA255

#pragma once

#include "cpu/arm7/arm7.h"
#include "sound/dmadac.h"
#include "emupal.h"


class pxa255_periphs_device : public device_t
{
public:
	template <typename T>
	pxa255_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: pxa255_periphs_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	pxa255_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <int Bit> auto gpio_out() { return m_gpio_w[Bit].bind(); }
	template <int Bit> void gpio_in(int state);

	void map(address_map &map) ATTR_COLD;

	// gpio_bit_w

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	static constexpr u32 INTERNAL_OSC = 3686400;

	// DMA Hardware
	void dma_irq_check();
	void dma_load_descriptor_and_start(int channel);
	TIMER_CALLBACK_MEMBER(audio_dma_end_tick);
	TIMER_CALLBACK_MEMBER(dma_end_tick);
	void dma_finish(int channel);

	u32 dma_dcsr_r(offs_t offset, u32 mem_mask);
	void dma_dcsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 dma_dint_r(offs_t offset, u32 mem_mask);
	void dma_dint_w(offs_t offset, u32 data, u32 mem_mask);
	u32 dma_drcmr_r(offs_t offset, u32 mem_mask);
	void dma_drcmr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 dma_ddadr_r(offs_t offset, u32 mem_mask);
	template <int Which> void dma_ddadr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 dma_dsadr_r(offs_t offset, u32 mem_mask);
	template <int Which> void dma_dsadr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 dma_dtadr_r(offs_t offset, u32 mem_mask);
	template <int Which> void dma_dtadr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 dma_dcmd_r(offs_t offset, u32 mem_mask);
	template <int Which> void dma_dcmd_w(offs_t offset, u32 data, u32 mem_mask);

	enum dma_bits_t : u32
	{
		DCSR_BUSERRINTR  = 1u <<  0,
		DCSR_STARTINTR   = 1u <<  1,
		DCSR_ENDINTR     = 1u <<  2,
		DCSR_STOPSTATE   = 1u <<  3,
		DCSR_REQPEND     = 1u <<  8,
		DCSR_STOPIRQ     = 1u << 29,
		DCSR_NODESCFETCH = 1u << 30,
		DCSR_RUN         = 1u << 31,

		DDADR_STOP       = 1u <<  0,

		DCMD_INCSRCADDR  = 1u << 31,
		DCMD_INCTRGADDR  = 1u << 30,
		DCMD_STARTIRQEN  = 1u << 22,
		DCMD_ENDIRQEN    = 1u << 21,
		DCMD_SIZE_SHIFT  = 16,
		DCMD_SIZE_MASK   = 3,
		DCMD_SIZE_0      = 0,
		DCMD_SIZE_8      = 1,
		DCMD_SIZE_16     = 2,
		DCMD_SIZE_32     = 3
	};

	struct dma_regs
	{
		u32 dcsr[16];
		u32 dint;
		u32 drcmr[40];

		u32 ddadr[16];
		u32 dsadr[16];
		u32 dtadr[16];
		u32 dcmd[16];

		emu_timer* timer[16];
	};

	dma_regs m_dma_regs;

	// RTC Hardware
	TIMER_CALLBACK_MEMBER(rtc_tick);
	u32 rtc_rcnr_r(offs_t offset, u32 mem_mask);
	void rtc_rcnr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rtc_rtar_r(offs_t offset, u32 mem_mask);
	void rtc_rtar_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rtc_rtsr_r(offs_t offset, u32 mem_mask);
	void rtc_rtsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rtc_rttr_r(offs_t offset, u32 mem_mask);
	void rtc_rttr_w(offs_t offset, u32 data, u32 mem_mask);

	struct rtc_regs
	{
		u32 rcnr;
		u32 rtar;
		u32 rtsr;
		u32 rttr;
		emu_timer *timer;
	};

	rtc_regs m_rtc_regs;

	// I2S (Audio) Hardware
	u32 i2s_sacr0_r(offs_t offset, u32 mem_mask);
	void i2s_sacr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 i2s_sacr1_r(offs_t offset, u32 mem_mask);
	void i2s_sacr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 i2s_sasr0_r(offs_t offset, u32 mem_mask);
	void i2s_sasr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 i2s_saimr_r(offs_t offset, u32 mem_mask);
	void i2s_saimr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 i2s_saicr_r(offs_t offset, u32 mem_mask);
	void i2s_saicr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 i2s_sadiv_r(offs_t offset, u32 mem_mask);
	void i2s_sadiv_w(offs_t offset, u32 data, u32 mem_mask);
	u32 i2s_sadr_r(offs_t offset, u32 mem_mask);
	void i2s_sadr_w(offs_t offset, u32 data, u32 mem_mask);

	enum i2s_bits_t : u32
	{
		SASR0_TNF =  1u <<  0,
		SASR0_RNE =  1u <<  1,
		SASR0_BSY =  1u <<  2,
		SASR0_TFS =  1u <<  3,
		SASR0_RFS =  1u <<  4,
		SASR0_TUR =  1u <<  5,
		SASR0_ROR =  1u <<  6,
		SASR0_TFL = 15u <<  8,
		SASR0_RFL = 15u << 12,

		SAICR_TUR =  1u << 5,
		SAICR_ROR =  1u << 6,
	};

	struct i2s_regs
	{
		u32 sacr0;
		u32 sacr1;
		u32 sasr0;
		u32 saimr;
		u32 saicr;
		u32 sadiv;
		u32 sadr;
	};

	i2s_regs m_i2s_regs;

	// Timer Hardware
	void ostimer_irq_check();
	TIMER_CALLBACK_MEMBER(ostimer_match_tick);
	template <int Which> void ostimer_update_interrupts();
	void ostimer_update_count();

	template <int Which> u32 tmr_osmr_r(offs_t offset, u32 mem_mask);
	template <int Which> void tmr_osmr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_oscr_r(offs_t offset, u32 mem_mask);
	void tmr_oscr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_ossr_r(offs_t offset, u32 mem_mask);
	void tmr_ossr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_ower_r(offs_t offset, u32 mem_mask);
	void tmr_ower_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_oier_r(offs_t offset, u32 mem_mask);
	void tmr_oier_w(offs_t offset, u32 data, u32 mem_mask);

	enum tmr_bits_t : u32
	{
		OSSR_M0 = 1u << 0,
		OSSR_M1 = 1u << 1,
		OSSR_M2 = 1u << 2,
		OSSR_M3 = 1u << 3,

		OIER_E0 = 1u << 0,
		OIER_E1 = 1u << 1,
		OIER_E2 = 1u << 2,
		OIER_E3 = 1u << 3
	};

	struct ostmr_regs
	{
		u32 osmr[4];
		u32 oscr;
		u32 ossr;
		u32 ower;
		u32 oier;

		emu_timer* timer[4];
		attotime last_count_sync;
	};

	ostmr_regs m_ostimer_regs;

	// Interrupt Hardware
	enum intc_bits_t : u32
	{
		INT_HUART     = 1u <<  7,
		INT_GPIO0     = 1u <<  8,
		INT_GPIO1     = 1u <<  9,
		INT_GPIO84_2  = 1u << 10,
		INT_USB       = 1u << 11,
		INT_PMU       = 1u << 12,
		INT_I2S       = 1u << 13,
		INT_AC97      = 1u << 14,
		INT_NETWORK   = 1u << 16,
		INT_LCD       = 1u << 17,
		INT_I2C       = 1u << 18,
		INT_ICP       = 1u << 19,
		INT_STUART    = 1u << 20,
		INT_BTUART    = 1u << 21,
		INT_FFUART    = 1u << 22,
		INT_MMC       = 1u << 23,
		INT_SSP       = 1u << 24,
		INT_DMA       = 1u << 25,
		INT_OSTIMER0  = 1u << 26,
		INT_OSTIMER1  = 1u << 27,
		INT_OSTIMER2  = 1u << 28,
		INT_OSTIMER3  = 1u << 29,
		INT_RTC_HZ    = 1u << 30,
		INT_RTC_ALARM = 1u << 31
	};

	void update_interrupts();
	u32 intc_icip_r(offs_t offset, u32 mem_mask);
	void intc_icip_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_icmr_r(offs_t offset, u32 mem_mask);
	void intc_icmr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_iclr_r(offs_t offset, u32 mem_mask);
	void intc_iclr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_icfp_r(offs_t offset, u32 mem_mask);
	void intc_icfp_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_icpr_r(offs_t offset, u32 mem_mask);
	void intc_icpr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_iccr_r(offs_t offset, u32 mem_mask);
	void intc_iccr_w(offs_t offset, u32 data, u32 mem_mask);

	struct intc_regs
	{
		u32 icip;
		u32 icmr;
		u32 iclr;
		u32 icfp;
		u32 icpr;
		u32 iccr;
	};

	intc_regs m_intc_regs;

	// GPIO Hardware
	template <int Which> void update_gpio_outputs(const u32 old);
	template <int Which> void check_gpio_irqs(const u32 old);
	template <int Which> u32 gpio_gplr_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gplr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gpdr_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gpdr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gpsr_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gpsr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gpcr_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gpcr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_grer_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_grer_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gfer_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gfer_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gedr_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gedr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gafrl_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gafrl_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 gpio_gafru_r(offs_t offset, u32 mem_mask);
	template <int Which> void gpio_gafru_w(offs_t offset, u32 data, u32 mem_mask);

	struct gpio_regs
	{
		u32 gpdr[3];
		u32 gpsr[3];
		u32 gpcr[3];
		u32 grer[3];
		u32 gfer[3];
		u32 gedr[3];
		u32 gafrl[3];
		u32 gafru[3];
		u32 out_data[3]; // Output data
		u32 in_data[3]; // Input data
	};

	gpio_regs m_gpio_regs;

	// LCD Hardware
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(lcd_dma_eof_tick);
	void lcd_load_dma_descriptor(u32 address, int channel);
	void lcd_irq_check();
	void lcd_dma_kickoff(int channel);
	void lcd_check_load_next_branch(int channel);

	template <int Which> u32 lcd_lccr_r(offs_t offset, u32 mem_mask);
	template <int Which> void lcd_lccr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 lcd_fbr_r(offs_t offset, u32 mem_mask);
	template <int Which> void lcd_fbr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lcd_lcsr_r(offs_t offset, u32 mem_mask);
	void lcd_lcsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lcd_liidr_r(offs_t offset, u32 mem_mask);
	void lcd_liidr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lcd_trgbr_r(offs_t offset, u32 mem_mask);
	void lcd_trgbr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 lcd_tcr_r(offs_t offset, u32 mem_mask);
	void lcd_tcr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 lcd_fdadr_r(offs_t offset, u32 mem_mask);
	template <int Which> void lcd_fdadr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 lcd_fsadr_r(offs_t offset, u32 mem_mask);
	template <int Which> void lcd_fsadr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 lcd_fidr_r(offs_t offset, u32 mem_mask);
	template <int Which> void lcd_fidr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 lcd_ldcmd_r(offs_t offset, u32 mem_mask);
	template <int Which> void lcd_ldcmd_w(offs_t offset, u32 data, u32 mem_mask);

	enum lcd_bits_t : u32
	{
		LCCR0_ENB    =   1u <<  0,
		LCCR0_CMS    =   1u <<  1,
		LCCR0_SDS    =   1u <<  2,
		LCCR0_LDM    =   1u <<  3,
		LCCR0_SFM    =   1u <<  4,
		LCCR0_IUM    =   1u <<  5,
		LCCR0_EFM    =   1u <<  6,
		LCCR0_PAS    =   1u <<  7,
		LCCR0_DPD    =   1u <<  9,
		LCCR0_DIS    =   1u << 10,
		LCCR0_QDM    =   1u << 11,
		LCCR0_PDD    = 0xff << 12,
		LCCR0_BM     =   1u << 20,
		LCCR0_OUM    =   1u << 21,

		LCCR1_PPL    = 0x000003ff,

		LCCR2_LPP    = 0x000003ff,

		LCSR_LDD     = 1u <<  0,
		LCSR_SOF     = 1u <<  1,
		LCSR_BER     = 1u <<  2,
		LCSR_ABC     = 1u <<  3,
		LCSR_IUL     = 1u <<  4,
		LCSR_IUU     = 1u <<  5,
		LCSR_OU      = 1u <<  6,
		LCSR_QD      = 1u <<  7,
		LCSR_EOF     = 1u <<  8,
		LCSR_BS      = 1u <<  9,
		LCSR_SINT    = 1u << 10,

		LDCMD_EOFINT = 1u << 21,
		LDCMD_SOFINT = 1u << 22,
		LDCMD_PAL    = 1u << 26
	};

	struct lcd_dma_regs
	{
		u32 fdadr;
		u32 fsadr;
		u32 fidr;
		u32 ldcmd;
		emu_timer *eof;
	};

	struct lcd_regs
	{
		u32 lccr[4];

		u32 fbr[2];

		u32 lcsr;
		u32 liidr;
		u32 trgbr;
		u32 tcr;

		lcd_dma_regs dma[2];
	};

	lcd_regs m_lcd_regs;

	// Power Management Hardware
	u32 pwr_pmcr_r(offs_t offset, u32 mem_mask);
	void pwr_pmcr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pssr_r(offs_t offset, u32 mem_mask);
	void pwr_pssr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pspr_r(offs_t offset, u32 mem_mask);
	void pwr_pspr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pwer_r(offs_t offset, u32 mem_mask);
	void pwr_pwer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_prer_r(offs_t offset, u32 mem_mask);
	void pwr_prer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pfer_r(offs_t offset, u32 mem_mask);
	void pwr_pfer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pedr_r(offs_t offset, u32 mem_mask);
	void pwr_pedr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pcfr_r(offs_t offset, u32 mem_mask);
	void pwr_pcfr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Which> u32 pwr_pgsr_r(offs_t offset, u32 mem_mask);
	template <int Which> void pwr_pgsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_rcsr_r(offs_t offset, u32 mem_mask);
	u32 pwr_pmfw_r(offs_t offset, u32 mem_mask);
	void pwr_pmfw_w(offs_t offset, u32 data, u32 mem_mask);

	struct power_regs
	{
		u32 pmcr;
		u32 pssr;
		u32 pspr;
		u32 pwer;
		u32 prer;
		u32 pfer;
		u32 pedr;
		u32 pcfr;
		u32 pgsr[3];
		u32 rcsr;
		u32 pmfw;
	};

	power_regs m_power_regs;

	// System Clock Hardware
	u32 clk_cccr_r(offs_t offset, u32 mem_mask);
	void clk_cccr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 clk_cken_r(offs_t offset, u32 mem_mask);
	void clk_cken_w(offs_t offset, u32 data, u32 mem_mask);
	u32 clk_oscc_r(offs_t offset, u32 mem_mask);
	void clk_oscc_w(offs_t offset, u32 data, u32 mem_mask);

	struct clk_regs
	{
		u32 cccr;
		u32 cken;
		u32 oscc;
	};

	clk_regs m_clk_regs;

	void set_irq_line(u32 line, int state);

	devcb_write_line::array<96> m_gpio_w;

	required_device<cpu_device> m_maincpu;
	required_device_array<dmadac_sound_device, 2> m_dmadac;
	required_device<palette_device> m_palette;

	std::unique_ptr<u32[]> m_lcd_palette; // 0x100
	std::unique_ptr<u8[]> m_lcd_framebuffer; // 0x100000
	std::unique_ptr<s16[]> m_samples; // 0x1000
};

DECLARE_DEVICE_TYPE(PXA255_PERIPHERALS, pxa255_periphs_device)

#endif // MAME_MACHINE_PXA255
