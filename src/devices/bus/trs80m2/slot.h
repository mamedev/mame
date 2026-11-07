// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 card cage slot emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_SLOT_H
#define MAME_BUS_TRS80M2_SLOT_H

#pragma once

#include "machine/z80daisy.h"


class trs80m2_slot_device;

class device_trs80m2_card_interface : public device_interface
{
public:
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

protected:
	device_trs80m2_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	trs80m2_slot_device *m_slot;
};


class trs80m2_slot_device : public device_t,
		public device_single_card_slot_interface<device_trs80m2_card_interface>,
		public device_z80daisy_interface
{
public:
	template <typename T>
	trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt) :
		trs80m2_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	trs80m2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	auto int_wr_callback() { return m_write_int.bind(); }
	auto drq_wr_callback() { return m_write_drq.bind(); }

	address_space &io() { return *m_io; }

	void int_w(int state) { m_write_int(state); }
	void drq_w(int state) { m_write_drq(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	required_address_space m_io;

	devcb_write_line m_write_int;
	devcb_write_line m_write_drq;

	device_trs80m2_card_interface *m_card;
};


DECLARE_DEVICE_TYPE(TRS80M2_SLOT, trs80m2_slot_device)

void trs80m2_cards(device_slot_interface &device);

#endif // MAME_BUS_TRS80M2_SLOT_H
