// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_ELAN_EU3A05COMMONSYS_H
#define MAME_MACHINE_ELAN_EU3A05COMMONSYS_H

#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"

class elan_eu3a05commonsys_device : public device_t
{
public:
	elan_eu3a05commonsys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	elan_eu3a05commonsys_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_addrbank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }
	void set_pal(void) { m_is_pal = true; }
	void disable_timer_irq(void) { m_allow_timer_irq = false; }

	void generate_custom_interrupt(int level);

	virtual void map(address_map& map);

	uint8_t nmi_vector_r(offs_t offset);
	uint8_t irq_vector_r(offs_t offset);

	void set_alt_timer() { m_whichtimer = 1; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	required_device<m6502_device> m_cpu;
	required_device<address_map_bank_device> m_bank;

	uint8_t m_intmask[2];

	int m_custom_irq;
	int m_custom_nmi;
	uint16_t m_custom_irq_vector;
	uint16_t m_custom_nmi_vector;

	uint8_t m_rombank_hi;
	uint8_t m_rombank_lo;

	bool m_is_pal; // this is usually a jumper connected to the chip that the software can read (clocks also differ on PAL units)
	bool m_allow_timer_irq;
private:
	uint8_t intmask_r(offs_t offset);
	void intmask_w(offs_t offset, uint8_t data);


	void elan_eu3a05_rombank_w(offs_t offset, uint8_t data);
	uint8_t elan_eu3a05_rombank_r(offs_t offset);

	uint8_t elan_eu3a05_5003_r();
	uint8_t elan_eu3a05_pal_ntsc_r();
	void elan_eu3a05_500b_unk_w(uint8_t data);

	uint8_t radica_5009_unk_r() { return machine().rand(); }

	emu_timer *m_unk_timer;
	static const device_timer_id TIMER_UNK = 0;
	int m_whichtimer;

};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_COMMONSYS, elan_eu3a05commonsys_device)

#endif // MAME_MACHINE_ELAN_EU3A05COMMONSYS_H
