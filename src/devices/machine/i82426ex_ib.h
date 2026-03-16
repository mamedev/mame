// license:BSD-3-Clause
// copyright-holders: Angelo Salese

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
	template <typename T, typename U>
	i82426ex_ib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cputag, U &&keybctag)
		: i82426ex_ib_device(mconfig, tag, owner, clock)
	{
		set_hostcputag(std::forward<T>(cputag));
		set_keybctag(std::forward<U>(keybctag));
	}

	i82426ex_ib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = XTAL(14'318'181).value());

	void io_map(address_map &map);
	void remap_bridge();

	auto intr() { return m_write_intr.bind(); }
	auto spkr() { return m_write_spkr.bind(); }

	auto rtcale() { return m_rtcale.bind(); }
	auto rtccs_read() { return m_rtccs_read.bind(); }
	auto rtccs_write() { return m_rtccs_write.bind(); }

	template <typename T> void set_hostcputag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_keybctag(T &&tag) { m_keybc.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	required_device<cpu_device> m_host_cpu;
	required_device<at_kbc_device_base> m_keybc;

	required_device_array<am9517a_device, 2> m_dma;
	required_device_array<pic8259_device, 2> m_intc;
	required_device<pit8254_device> m_pit;
	required_device<isa16_device> m_isabus;

	devcb_write_line m_write_intr;
	devcb_write_line m_write_spkr;

	devcb_write8 m_rtcale;
	devcb_read8 m_rtccs_read;
	devcb_write8 m_rtccs_write;

	u8 portb_r();
	void portb_w(u8 data);
	void iochck_w(int state);

	int m_dma_eop;
	u8 m_dma_page[0x10];
	u8 m_dma_high_byte;
	int m_dma_channel;

	u8 m_portb;
	int m_refresh_toggle;
	int m_iochck;
	u8 m_nmi_mask;

	offs_t page_offset();
	void set_dma_channel(int channel, bool state);

	u8 dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, u8 data);
	u8 dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, u8 data);
	void dma1_eop_w(int state);
	void dma2_hreq_w(int state);
};

DECLARE_DEVICE_TYPE(I82426EX_IB, i82426ex_ib_device)


#endif // MAME_MACHINE_I82426EX_IB_H
