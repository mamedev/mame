// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    DEC Q-Bus emulation (skeleton)

**********************************************************************/

#ifndef MAME_BUS_QBUS_QBUS_H
#define MAME_BUS_QBUS_QBUS_H

#pragma once

#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class qbus_device;

// ======================> device_qbus_card_interface

class device_qbus_card_interface : public device_interface
{
	friend class qbus_device;
	template <class ElementType> friend class simple_list;

public:
	device_qbus_card_interface *next() const { return m_next; }

	// device_qbus_card_interface overrides
	virtual void biaki_w(int state) { }
	virtual void bdmgi_w(int state) { }

protected:
	// construction/destruction
	device_qbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual int z80daisy_irq_state() { return 0; }
	virtual int z80daisy_irq_ack() { return -1; }
	virtual void z80daisy_irq_reti() { }

	qbus_device  *m_bus;

private:
	device_qbus_card_interface *m_next;
};



// ======================> qbus_device

class qbus_device : public device_t,
	public device_memory_interface,
    public device_z80daisy_interface
{
public:
	// construction/destruction
	qbus_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *cputag)
		: qbus_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_cputag(cputag);
	}

	qbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	~qbus_device() { m_device_list.detach_all(); }

	// inline configuration
	void set_cputag(const char *tag) { m_cputag = tag; }

	virtual space_config_vector memory_space_config() const override
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
	}

	auto birq4() { return m_out_birq4_cb.bind(); }
	auto birq5() { return m_out_birq6_cb.bind(); }
	auto birq6() { return m_out_birq6_cb.bind(); }
	auto birq7() { return m_out_birq7_cb.bind(); }

	void add_card(device_qbus_card_interface *card);
	void install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, uint32_t mask=0xffffffff);

	DECLARE_WRITE_LINE_MEMBER(birq4_w) { m_out_birq4_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(birq5_w) { m_out_birq5_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(birq6_w) { m_out_birq6_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(birq7_w) { m_out_birq7_cb(state); }

	DECLARE_WRITE_LINE_MEMBER(bdmr_w) { m_out_bdmr_cb(state); }

	const address_space_config m_program_config;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal state
	cpu_device *m_maincpu;
	const char *m_cputag;

private:
	devcb_write_line m_out_birq4_cb;
	devcb_write_line m_out_birq5_cb;
	devcb_write_line m_out_birq6_cb;
	devcb_write_line m_out_birq7_cb;
	devcb_write_line m_out_bdmr_cb;

	simple_list<device_qbus_card_interface> m_device_list;
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
	DECLARE_WRITE_LINE_MEMBER( biaki_w ) { if (m_card) m_card->biaki_w(state); }
	DECLARE_WRITE_LINE_MEMBER( bdmgi_w ) { if (m_card) m_card->bdmgi_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override { if (m_card) get_card_device()->reset(); }

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
