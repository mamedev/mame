// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_ELAN_EU3A05COMMONSYS_H
#define MAME_MACHINE_ELAN_EU3A05COMMONSYS_H

#include "cpu/m6502/m6502.h"

class elan_eu3a05commonsys_device : public device_t
{
public:
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_pal(void) { m_is_pal = true; }
	void disable_timer_irq(void) { m_allow_timer_irq = false; }

	auto bank_change_callback() { return m_bankchange_cb.bind(); }

	void generate_custom_interrupt(int level);

	virtual void map(address_map &map) ATTR_COLD;

	uint8_t nmi_vector_r(offs_t offset);
	uint8_t irq_vector_r(offs_t offset);

	void set_alt_timer() { m_whichtimer = 1; }

protected:
	elan_eu3a05commonsys_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(unknown_timer_tick);

	required_device<m6502_device> m_cpu;

	uint8_t m_intmask[2];

	int m_custom_irq;
	int m_custom_nmi;
	uint16_t m_custom_irq_vector;
	uint16_t m_custom_nmi_vector;

	uint8_t m_rombank_hi;
	uint8_t m_rombank_lo;

	bool m_is_pal; // this is usually a jumper connected to the chip that the software can read (clocks also differ on PAL units)
	bool m_allow_timer_irq;
	bool m_bank_on_low_bank_writes;

protected:
	uint8_t intmask_r(offs_t offset);
	void intmask_w(offs_t offset, uint8_t data);

	void elan_eu3a05_rombank_w(offs_t offset, uint8_t data);
	uint8_t elan_eu3a05_rombank_r(offs_t offset);

	uint8_t elan_eu3a05_5003_r();
	uint8_t elan_eu3a05_pal_ntsc_r();
	void elan_eu3a05_500b_unk_w(uint8_t data);

	uint8_t radica_5009_unk_r() { return machine().rand(); }

	emu_timer *m_unk_timer;
	int m_whichtimer;
	devcb_write16 m_bankchange_cb;

};

#endif // MAME_MACHINE_ELAN_EU3A05COMMONSYS_H
