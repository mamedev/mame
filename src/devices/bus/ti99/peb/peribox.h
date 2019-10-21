// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Peripheral expansion box
    See peribox.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_PERIBOX_H
#define MAME_BUS_TI99_PEB_PERIBOX_H

#pragma once

#include "bus/ti99/ti99defs.h"
#include "bus/ti99/internal/ioport.h"

namespace bus { namespace ti99 { namespace peb {

class peribox_slot_device;
class device_ti99_peribox_card_interface;

/*****************************************************************************
    The overall Peripheral Expansion Box.
******************************************************************************/

class peribox_device : public bus::ti99::internal::ioport_attached_device
{
	friend class peribox_slot_device;
public:
	peribox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Next eight methods are called from the console
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	DECLARE_WRITE_LINE_MEMBER(senila);
	DECLARE_WRITE_LINE_MEMBER(senilb);

	DECLARE_WRITE_LINE_MEMBER( memen_in ) override;
	DECLARE_WRITE_LINE_MEMBER( msast_in ) override;

	DECLARE_WRITE_LINE_MEMBER( clock_in ) override;

	// Part of configuration
	void set_prefix(int prefix) { m_address_prefix = prefix; }

	// Callbacks
	auto inta_cb() { return m_slot1_inta.bind(); }
	auto intb_cb() { return m_slot1_intb.bind(); }
	auto ready_cb() { return m_slot1_ready.bind(); }
	auto lcp_cb() { return m_slot1_lcp.bind(); }

protected:
	peribox_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_config_complete() override;

	virtual void device_add_mconfig(machine_config &config) override;

	// Next three methods call back the console via slot 1
	devcb_write_line m_slot1_inta;   // INTA line (Box to console)
	devcb_write_line m_slot1_intb;   // INTB line
	devcb_write_line m_slot1_lcp;       // For EVPC with SGCPU only
	devcb_write_line m_slot1_ready;  // READY line (to the datamux)

	void set_slot_loaded(int slot, peribox_slot_device* slotdev);
	peribox_slot_device *m_slot[9];     // for the sake of simplicity we donate the first two positions (0,1)

	required_device<peribox_slot_device> m_slot2;
	required_device<peribox_slot_device> m_slot3;
	required_device<peribox_slot_device> m_slot4;
	required_device<peribox_slot_device> m_slot5;
	required_device<peribox_slot_device> m_slot6;
	required_device<peribox_slot_device> m_slot7;
	required_device<peribox_slot_device> m_slot8;

	// Propagators for the slot signals. All signals are active low, and
	// if any one slot asserts the line, the joint line is asserted.
	void inta_join(int slot, int state);
	void intb_join(int slot, int state);
	void lcp_join(int slot, int state);
	void ready_join(int slot, int state);

	int m_inta_flag;
	int m_intb_flag;
	int m_lcp_flag;
	int m_ready_flag;

	// The TI-99/4(A) Flex Cable Interface (slot 1) pulls up the AMA/AMB/AMC lines to 1/1/1.
	int m_address_prefix;

	// Most significant address byte strobe. Defined by TI-99/8.
	bool    m_msast;

	// Memory enable.
	bool    m_memen;

	// Configured as a slot device (of the ioport)
	bool    m_ioport_connected;

	// Used for Genmod
	bool    m_genmod;
};

/************************************************************************
    Specific Box compositions
************************************************************************/

/*
    Variation for SGCPU (TI-99/4P). We put the EVPC and the HSGPL in slots 2 and 3.
*/
class peribox_sg_device : public peribox_device
{
public:
	peribox_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void device_add_mconfig(machine_config &config) override;
};

/*
    Variation for ti99_4ev. We put the EVPC in slot 2.
*/
class peribox_ev_device : public peribox_device
{
public:
	peribox_ev_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void device_add_mconfig(machine_config &config) override;
};


/*
    Variation for Geneve.
*/
class peribox_gen_device : public peribox_device
{
public:
	peribox_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	peribox_gen_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;
};

/*
    Variation for Geneve with Genmod
*/
class peribox_genmod_device : public peribox_gen_device
{
public:
	peribox_genmod_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void device_add_mconfig(machine_config &config) override;
};

/*****************************************************************************
    The parent class for all expansion cards.
******************************************************************************/

class device_ti99_peribox_card_interface : public device_slot_card_interface
{
	friend class peribox_slot_device;

public:
	virtual DECLARE_READ8Z_MEMBER(readz) = 0;
	virtual void write(offs_t offset, uint8_t data) = 0;
	virtual DECLARE_READ8Z_MEMBER(crureadz) = 0;
	virtual void cruwrite(offs_t offset, uint8_t data) = 0;
	virtual DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) { };

	virtual DECLARE_WRITE_LINE_MEMBER(clock_in) { }
	void    set_senila(int state) { m_senila = state; }
	void    set_senilb(int state) { m_senilb = state; }

protected:
	using device_slot_card_interface::device_slot_card_interface;
	device_ti99_peribox_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_config_complete() override;

	peribox_slot_device *m_slot;        // using a link to the slot for callbacks
	int m_senila;
	int m_senilb;

	// When true, card is accessible. Indicated by a LED.
	bool    m_selected;

	// When true, GenMod is selected. Modified by peribox_slot_device.
	bool    m_genmod;

	// CRU base. Used to configure the address by which a card is selected.
	int     m_cru_base;

	// Used to decide whether this card has been selected.
	int     m_select_mask;
	int     m_select_value;
};

/*****************************************************************************
    A single slot in the box.
******************************************************************************/

class peribox_slot_device : public device_t, public device_slot_interface
{
	friend class peribox_device;
public:
	template <typename U>
	peribox_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, int slot, U &&opts, const char *dflt)
		: peribox_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_slotnumber = slot;
	}

	peribox_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Called from the box (direction to card)
	DECLARE_READ8Z_MEMBER(readz);
	void write(offs_t offset, uint8_t data);
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin);

	DECLARE_WRITE_LINE_MEMBER(senila);
	DECLARE_WRITE_LINE_MEMBER(senilb);
	DECLARE_WRITE_LINE_MEMBER(clock_in);

	// Called from the card (direction to box)
	DECLARE_WRITE_LINE_MEMBER( set_inta );
	DECLARE_WRITE_LINE_MEMBER( set_intb );
	DECLARE_WRITE_LINE_MEMBER( lcp_line );
	DECLARE_WRITE_LINE_MEMBER( set_ready );

	DECLARE_READ8Z_MEMBER(crureadz);
	void cruwrite(offs_t offset, uint8_t data);

	// called from the box itself
	void set_genmod(bool set);
	void set_number(int number) { m_slotnumber = number; }

protected:
	void device_start() override;
	void device_config_complete() override;

private:
	int get_index_from_tagname();
	device_ti99_peribox_card_interface *m_card;
	int m_slotnumber;
	const char* card_name() { return m_card->device().tag(); }
};

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_PERIBOX,      bus::ti99::peb, peribox_device)
DECLARE_DEVICE_TYPE_NS(TI99_PERIBOX_EV,   bus::ti99::peb, peribox_ev_device)
DECLARE_DEVICE_TYPE_NS(TI99_PERIBOX_SLOT, bus::ti99::peb, peribox_slot_device)
DECLARE_DEVICE_TYPE_NS(TI99_PERIBOX_SG,   bus::ti99::peb, peribox_sg_device)
DECLARE_DEVICE_TYPE_NS(TI99_PERIBOX_GEN,  bus::ti99::peb, peribox_gen_device)
DECLARE_DEVICE_TYPE_NS(TI99_PERIBOX_GENMOD,  bus::ti99::peb, peribox_genmod_device)

void ti99_peribox_slot_standard(device_slot_interface &device);
void ti99_peribox_slot_evpc(device_slot_interface &device);
void ti99_peribox_slot_geneve(device_slot_interface &device);
void ti99_peribox_slot_sgcpu(device_slot_interface &device);

#endif // MAME_BUS_TI99_PEB_PERIBOX_H
