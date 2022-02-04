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
#include "cpu/arm7/arm7core.h"
#include "sound/dmadac.h"
#include "emupal.h"

#include "pxa255defs.h"


class pxa255_periphs_device : public device_t
{
public:
	template <typename T>
	pxa255_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: pxa255_periphs_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	pxa255_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto gpio0_write() { return m_gpio0_w.bind(); }
	auto gpio0_read() { return m_gpio0_r.bind(); }
	auto gpio1_write() { return m_gpio1_w.bind(); }
	auto gpio1_read() { return m_gpio1_r.bind(); }
	auto gpio2_write() { return m_gpio2_w.bind(); }
	auto gpio2_read() { return m_gpio2_r.bind(); }

	uint32_t dma_r(offs_t offset, uint32_t mem_mask = ~0);
	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t i2s_r(offs_t offset, uint32_t mem_mask = ~0);
	void i2s_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ostimer_r(offs_t offset, uint32_t mem_mask = ~0);
	void ostimer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t intc_r(offs_t offset, uint32_t mem_mask = ~0);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void gpio_bit_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint32_t gpio_r(offs_t offset, uint32_t mem_mask = ~0);
	void gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t lcd_r(offs_t offset, uint32_t mem_mask = ~0);
	void lcd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t power_r(offs_t offset, uint32_t mem_mask = ~0);
	void power_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t clocks_r(offs_t offset, uint32_t mem_mask = ~0);
	void clocks_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	static const device_timer_id TIMER_DMA0 = 0;
	static const device_timer_id TIMER_OSTIMER0 = 16;
	static const device_timer_id TIMER_LCD_EOF0 = 20;
	static const device_timer_id TIMER_RTC = 22;

	void dma_irq_check();
	void dma_load_descriptor_and_start(int channel);
	void ostimer_irq_check();
	void update_interrupts();
	void lcd_load_dma_descriptor(address_space & space, uint32_t address, int channel);
	void lcd_irq_check();
	void lcd_dma_kickoff(int channel);
	void lcd_check_load_next_branch(int channel);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void dma_end_tick(int channel);
	void ostimer_match_tick(int channel);
	void lcd_dma_eof_tick(int channel);
	void rtc_tick();

	void set_irq_line(uint32_t line, int state);

	struct dma_regs
	{
		uint32_t dcsr[16];
		uint32_t pad0[44];

		uint32_t dint;
		uint32_t pad1[3];

		uint32_t drcmr[40];
		uint32_t pad2[24];

		uint32_t ddadr[16];
		uint32_t dsadr[16];
		uint32_t dtadr[16];
		uint32_t dcmd[16];

		emu_timer* timer[16];
	};

	struct i2s_regs
	{
		uint32_t sacr0;
		uint32_t sacr1;
		uint32_t pad0;

		uint32_t sasr0;
		uint32_t pad1;

		uint32_t saimr;
		uint32_t saicr;
		uint32_t pad2[17];

		uint32_t sadiv;
		uint32_t pad3[6];

		uint32_t sadr;
	};

	struct rtc_regs
	{
		uint32_t rcnr;
		uint32_t rtar;
		uint32_t rtsr;
		uint32_t rttr;
		emu_timer *timer;
	};

	struct ostmr_regs
	{
		uint32_t osmr[4];
		uint32_t oscr;
		uint32_t ossr;
		uint32_t ower;
		uint32_t oier;

		emu_timer* timer[4];
	};

	struct intc_regs
	{
		uint32_t icip;
		uint32_t icmr;
		uint32_t iclr;
		uint32_t icfp;
		uint32_t icpr;
		uint32_t iccr;
	};

	struct gpio_regs
	{
		uint32_t gplr0; // GPIO Pin-Level
		uint32_t gplr1;
		uint32_t gplr2;

		uint32_t gpdr0;
		uint32_t gpdr1;
		uint32_t gpdr2;

		uint32_t gpsr0;
		uint32_t gpsr1;
		uint32_t gpsr2;

		uint32_t gpcr0;
		uint32_t gpcr1;
		uint32_t gpcr2;

		uint32_t grer0;
		uint32_t grer1;
		uint32_t grer2;

		uint32_t gfer0;
		uint32_t gfer1;
		uint32_t gfer2;

		uint32_t gedr0;
		uint32_t gedr1;
		uint32_t gedr2;

		uint32_t gafr0l;
		uint32_t gafr0u;
		uint32_t gafr1l;
		uint32_t gafr1u;
		uint32_t gafr2l;
		uint32_t gafr2u;
	};

	struct lcd_dma_regs
	{
		uint32_t fdadr;
		uint32_t fsadr;
		uint32_t fidr;
		uint32_t ldcmd;
		emu_timer *eof;
	};

	struct lcd_regs
	{
		uint32_t lccr0;
		uint32_t lccr1;
		uint32_t lccr2;
		uint32_t lccr3;

		uint32_t fbr[2];

		uint32_t lcsr;
		uint32_t liidr;
		uint32_t trgbr;
		uint32_t tcr;

		lcd_dma_regs dma[2];
	};

	struct power_regs
	{
		uint32_t pmcr;
		uint32_t pssr;
		uint32_t pspr;
		uint32_t pwer;
		uint32_t prer;
		uint32_t pfer;
		uint32_t pedr;
		uint32_t pcfr;
		uint32_t pgsr0;
		uint32_t pgsr1;
		uint32_t pgsr2;
		uint32_t rcsr;
		uint32_t pmfw;
	};

	struct clocks_regs
	{
		uint32_t cccr;
		uint32_t cken;
		uint32_t oscc;
	};

	dma_regs m_dma_regs;
	i2s_regs m_i2s_regs;
	rtc_regs m_rtc_regs;
	ostmr_regs m_ostimer_regs;
	intc_regs m_intc_regs;
	gpio_regs m_gpio_regs;
	lcd_regs m_lcd_regs;
	power_regs m_power_regs;
	clocks_regs m_clocks_regs;

	devcb_write32 m_gpio0_w;
	devcb_write32 m_gpio1_w;
	devcb_write32 m_gpio2_w;
	devcb_read32 m_gpio0_r;
	devcb_read32 m_gpio1_r;
	devcb_read32 m_gpio2_r;

	required_device<cpu_device> m_maincpu;
	required_device_array<dmadac_sound_device, 2> m_dmadac;
	required_device<palette_device> m_palette;

	std::unique_ptr<uint32_t[]> m_lcd_palette; // 0x100
	std::unique_ptr<uint8_t[]> m_lcd_framebuffer; // 0x100000
	std::unique_ptr<uint32_t[]> m_words; // 0x800
	std::unique_ptr<int16_t[]> m_samples; // 0x1000
};

DECLARE_DEVICE_TYPE(PXA255_PERIPHERALS, pxa255_periphs_device)

#endif // MAME_MACHINE_PXA255
