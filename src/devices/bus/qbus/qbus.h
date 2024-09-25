// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    DEC Q-Bus emulation (skeleton)

**********************************************************************/

#ifndef MAME_BUS_QBUS_QBUS_H
#define MAME_BUS_QBUS_QBUS_H

#pragma once

#include "machine/z80daisy.h"

#include <functional>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class qbus_device;

// ======================> device_qbus_card_interface

class device_qbus_card_interface : public device_interface
{
	friend class qbus_device;

public:
	// Q-Bus interface
	virtual void biaki_w(int state) { }
	virtual void bdmgi_w(int state) { }
	virtual void init_w() { device_reset(); }

protected:
	// construction/destruction
	device_qbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void device_reset() { }

	virtual int z80daisy_irq_state() { return 0; }
	virtual int z80daisy_irq_ack() { return -1; }
	virtual void z80daisy_irq_reti() { }

	qbus_device *m_bus;
};



// ======================> qbus_device

class qbus_device : public device_t,
	public device_memory_interface,
	public device_z80daisy_interface
{
public:
	// construction/destruction
	template <typename T>
	qbus_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cputag, int spacenum)
		: qbus_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_cputag(std::forward<T>(cputag), spacenum);
	}

	qbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	~qbus_device();

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	virtual space_config_vector memory_space_config() const override;

	auto birq4() { return m_out_birq4_cb.bind(); }
	auto birq5() { return m_out_birq6_cb.bind(); }
	auto birq6() { return m_out_birq6_cb.bind(); }
	auto birq7() { return m_out_birq7_cb.bind(); }

	void add_card(device_qbus_card_interface &card);
	void install_device(offs_t start, offs_t end, read16sm_delegate rhandler, write16sm_delegate whandler, uint32_t mask=0xffffffff);

	void init_w();

	void birq4_w(int state) { m_out_birq4_cb(state); }
	void birq5_w(int state) { m_out_birq5_cb(state); }
	void birq6_w(int state) { m_out_birq6_cb(state); }
	void birq7_w(int state) { m_out_birq7_cb(state); }

	void bdmr_w(int state) { m_out_bdmr_cb(state); }

	const address_space_config m_program_config;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal state
	required_address_space m_space;

private:
	using card_vector = std::vector<std::reference_wrapper<device_qbus_card_interface> >;

	devcb_write_line m_out_birq4_cb;
	devcb_write_line m_out_birq5_cb;
	devcb_write_line m_out_birq6_cb;
	devcb_write_line m_out_birq7_cb;
	devcb_write_line m_out_bdmr_cb;

	card_vector m_device_list;
};


// ======================> qbus_slot_device

class qbus_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	qbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: qbus_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1, 1))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	qbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// computer interface
	void biaki_w(int state) { if (m_card) m_card->biaki_w(state); }
	void bdmgi_w(int state) { if (m_card) m_card->bdmgi_w(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_write_birq4;
	devcb_write_line m_write_birq5;
	devcb_write_line m_write_birq6;
	devcb_write_line m_write_birq7;
	devcb_write_line m_write_bdmr;

	device_qbus_card_interface *m_card;

private:
	required_device<qbus_device> m_bus;
};


// device type definition
DECLARE_DEVICE_TYPE(QBUS, qbus_device)
DECLARE_DEVICE_TYPE(QBUS_SLOT, qbus_slot_device)

void qbus_cards(device_slot_interface &device);

#endif // MAME_BUS_QBUS_QBUS_H
