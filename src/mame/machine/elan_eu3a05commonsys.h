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

	void generate_custom_interrupt(int level);

	DECLARE_READ8_MEMBER(intmask_r);
	DECLARE_WRITE8_MEMBER(intmask_w);

	DECLARE_READ8_MEMBER(nmi_vector_r);
	DECLARE_READ8_MEMBER(irq_vector_r);

	DECLARE_WRITE8_MEMBER(elan_eu3a05_rombank_w);
	DECLARE_READ8_MEMBER(elan_eu3a05_rombank_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<m6502_device> m_cpu;
	required_device<address_map_bank_device> m_bank;
	uint8_t m_intmask[2];

	int m_custom_irq;
	int m_custom_nmi;
	uint16_t m_custom_irq_vector;
	uint16_t m_custom_nmi_vector;

	uint8_t m_rombank_hi;
	uint8_t m_rombank_lo;
};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_COMMONSYS, elan_eu3a05commonsys_device)

#endif // MAME_MACHINE_ELAN_EU3A05COMMONSYS_H
