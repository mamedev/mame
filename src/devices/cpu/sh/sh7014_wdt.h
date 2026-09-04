// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

  SH7014 watchdog timer

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_WDT_H
#define MAME_CPU_SH_SH7014_WDT_H

#pragma once

#include "sh7014_intc.h"


class sh7014_wdt_device : public device_t
{
public:
	sh7014_wdt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> sh7014_wdt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc)
		: sh7014_wdt_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc));
	}

	void map(address_map &map) ATTR_COLD;

	uint8_t tcsr_r();
	uint8_t tcnt_r();
	uint8_t rstcsr_r();
	void timer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void reset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : uint8_t
	{
		TCSR_OVF   = 0x80,
		TCSR_WT_IT = 0x40,
		TCSR_TME   = 0x20,
		TCSR_CKS   = 0x07,

		RSTCSR_WOVF = 0x80,
		RSTCSR_RSTE = 0x40
	};

	TIMER_CALLBACK_MEMBER(overflow);

	uint8_t counter() const;
	int prescaler() const;
	void update_timer();
	void update_interrupt();

	required_device<sh7014_intc_device> m_intc;

	emu_timer *m_timer;
	attotime m_count_time;

	uint8_t m_tcsr;
	uint8_t m_tcnt;
	uint8_t m_rstcsr;
	bool m_int_state;
};

DECLARE_DEVICE_TYPE(SH7014_WDT, sh7014_wdt_device)

#endif // MAME_CPU_SH_SH7014_WDT_H
