// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 SID socket emulation

**********************************************************************/

#ifndef MAME_BUS_C64_SID_H
#define MAME_BUS_C64_SID_H

#pragma once

#include "sound/mos6581.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class c64_sid_slot_device;


// ======================> device_c64_sid_interface

class device_c64_sid_interface : public device_interface
{
public:
	// the socket carries A0-A4 only, the full address is for boards with flying leads to A5-A15
	virtual uint8_t read(offs_t offset) = 0;
	virtual void write(offs_t offset, uint8_t data) = 0;

protected:
	device_c64_sid_interface(const machine_config &mconfig, device_t &device);

	c64_sid_slot_device *const m_slot;
};


// ======================> c64_sid_slot_device

class c64_sid_slot_device : public device_t, public device_single_card_slot_interface<device_c64_sid_interface>, public device_mixer_interface
{
public:
	template <typename T>
	c64_sid_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: c64_sid_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	c64_sid_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto potx() { return m_read_potx.bind(); }
	auto poty() { return m_read_poty.bind(); }

	uint8_t read(offs_t offset) { return m_card->read(offset); }
	void write(offs_t offset, uint8_t data) { m_card->write(offset, data); }

	// card interface
	uint8_t potx_r() { return m_read_potx(); }
	uint8_t poty_r() { return m_read_poty(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	devcb_read8 m_read_potx;
	devcb_read8 m_read_poty;

	device_c64_sid_interface *m_card;
};


// ======================> c64_sid_6581_device

class c64_sid_6581_device : public device_t, public device_c64_sid_interface
{
public:
	c64_sid_6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_c64_sid_interface implementation
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	c64_sid_6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void sid_config() ATTR_COLD;

	required_device<mos6581_device> m_sid;
};


// ======================> c64_sid_8580_device

class c64_sid_8580_device : public c64_sid_6581_device
{
public:
	c64_sid_8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(C64_SID_SLOT, c64_sid_slot_device)
DECLARE_DEVICE_TYPE(C64_SID_6581, c64_sid_6581_device)
DECLARE_DEVICE_TYPE(C64_SID_8580, c64_sid_8580_device)

void c64_sid_cards(device_slot_interface &device) ATTR_COLD;

#endif // MAME_BUS_C64_SID_H
