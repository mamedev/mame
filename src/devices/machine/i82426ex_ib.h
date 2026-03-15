// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_MACHINE_I82426EX_IB_H
#define MAME_MACHINE_I82426EX_IB_H

#pragma once

#include "pci.h"

#include "bus/isa/isa.h"
#include "cpu/i386/i386.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/at_keybc.h"
#include "sound/spkrdev.h"

class i82426ex_ib_device : public device_t
{
public:
	template <typename T>
	i82426ex_ib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&keybctag)
		: i82426ex_ib_device(mconfig, tag, owner, clock)
	{
		set_keybctag(std::forward<T>(keybctag));
	}

	i82426ex_ib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = XTAL(14'318'181).value());

	void io_map(address_map &map);

	auto intr() { return m_write_intr.bind(); }
	auto spkr() { return m_write_spkr.bind(); }

	auto rtcale() { return m_rtcale.bind(); }
	auto rtccs_read() { return m_rtccs_read.bind(); }
	auto rtccs_write() { return m_rtccs_write.bind(); }

	template <typename T> void set_keybctag(T &&tag) { m_keybc.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<at_kbc_device_base> m_keybc;

	required_device_array<am9517a_device, 2> m_dma;
	required_device_array<pic8259_device, 2> m_intc;
	required_device<pit8254_device> m_pit;

	devcb_write_line m_write_intr;
	devcb_write_line m_write_spkr;

	devcb_write8 m_rtcale;
	devcb_read8 m_rtccs_read;
	devcb_write8 m_rtccs_write;

	u8 portb_r();
	void portb_w(u8 data);

	u8 m_portb;
	int m_refresh_toggle;
	u8 m_nmi_mask;
};

DECLARE_DEVICE_TYPE(I82426EX_IB, i82426ex_ib_device)


#endif // MAME_MACHINE_I82426EX_IB_H
