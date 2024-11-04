// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System expansion slot emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_SMS_EXP_SMSEXP_H
#define MAME_BUS_SMS_EXP_SMSEXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_sms_expansion_slot_interface

// class representing interface-specific live sms_expansion card
class device_sms_expansion_slot_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_sms_expansion_slot_interface();

	// reading and writing
	virtual uint8_t read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual void write_mapper(offs_t offset, uint8_t data) { }
	virtual uint8_t read_ram(offs_t offset) { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) { }

	virtual int get_lphaser_xoffs() { return 0; }

protected:
	device_sms_expansion_slot_interface(const machine_config &mconfig, device_t &device);
};


// ======================> sms_expansion_slot_device

class sms_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_sms_expansion_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	sms_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sms_expansion_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sms_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~sms_expansion_slot_device();

	// reading and writing
	uint8_t read(offs_t offset) { return m_device ? m_device->read(offset) : 0xff; }
	void write(offs_t offset, uint8_t data) { if (m_device) m_device->write(offset, data); }
	void write_mapper(offs_t offset, uint8_t data) { if (m_device) m_device->write_mapper(offset, data); }
	uint8_t read_ram(offs_t offset) { return m_device ? m_device->read_ram(offset) : 0xff; }
	void write_ram(offs_t offset, uint8_t data) { if (m_device) m_device->write_ram(offset, data); }

	int get_lphaser_xoffs() { return m_device ? m_device->get_lphaser_xoffs() : 0; }

	bool device_present() const { return bool(m_device); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_sms_expansion_slot_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(SMS_EXPANSION_SLOT, sms_expansion_slot_device)


void sms_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_SMS_EXP_SMSEXP_H
