// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************************************************************
 *
 * Intel XScale SA1110 peripheral emulation
 *
 **************************************************************************/

#ifndef MAME_MACHINE_SA1110
#define MAME_MACHINE_SA1110

#pragma once

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/dmadac.h"
#include "emupal.h"

class sa1110_periphs_device : public device_t
{
public:
	template <typename T>
	sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: sa1110_periphs_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t intc_r(offs_t offset, uint32_t mem_mask = ~0);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t power_r(offs_t offset, uint32_t mem_mask = ~0);
	void power_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void update_interrupts();
	void set_irq_line(uint32_t line, int state);

	enum
	{
		INTC_BASE_ADDR  = 0x90050000,
		REG_ICIP        = (0x00000000 >> 2),
		REG_ICMR        = (0x00000004 >> 2),
		REG_ICLR        = (0x00000008 >> 2),
		REG_ICCR        = (0x0000000c >> 2),
		REG_ICFP        = (0x00000010 >> 2),
		REG_ICPR        = (0x00000020 >> 2),

		POWER_BASE_ADDR = 0x90020000,
		REG_PMCR        = (0x00000000 >> 2),
		REG_PSSR        = (0x00000004 >> 2),
		REG_PSPR        = (0x00000008 >> 2),
		REG_PWER        = (0x0000000c >> 2),
		REG_PCFR        = (0x00000010 >> 2),
		REG_PPCR        = (0x00000014 >> 2),
		REG_PGSR        = (0x00000018 >> 2),
		REG_POSR        = (0x0000001c >> 2)
	};

	struct intc_regs
	{
		uint32_t icip;
		uint32_t icmr;
		uint32_t iclr;
		uint32_t iccr;
		uint32_t icfp;
		uint32_t icpr;
	};

	struct power_regs
	{
		uint32_t pmcr;
		uint32_t pssr;
		uint32_t pspr;
		uint32_t pwer;
		uint32_t pcfr;
		uint32_t ppcr;
		uint32_t pgsr;
		uint32_t posr;
	};

	intc_regs m_intc_regs;
	power_regs m_power_regs;

	required_device<cpu_device> m_maincpu;
};

DECLARE_DEVICE_TYPE(SA1110_PERIPHERALS, sa1110_periphs_device)

#endif // MAME_MACHINE_SA1110
