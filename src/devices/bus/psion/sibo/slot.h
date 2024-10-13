// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Reduced External Expansion slot emulation

**********************************************************************

    Pin  Name     Comments

    1    MSD      Master SIBO serial protocol data line - pulled low
    2    MCLK     Master SIBO serial protocol clock line - low in standby
    3    Vcc      +5 volt supply, switched off in standby
    4    GND      Signal ground, this signal should mate first when connector inserted
    5    SSD/INT  Slave SIBO serial data line or active high interrupt input
    6    SCK/EXON Slave SIBO serial clock line or active high switch on input

**********************************************************************/

#ifndef MAME_BUS_PSION_SIBO_SLOT_H
#define MAME_BUS_PSION_SIBO_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_sibo_slot_device

class device_psion_sibo_interface;

class psion_sibo_slot_device : public device_t, public device_single_card_slot_interface<device_psion_sibo_interface>
{
public:
	// construction/destruction
	template <typename T>
	psion_sibo_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: psion_sibo_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	psion_sibo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_cb() { return m_int_cb.bind(); }

	uint8_t data_r();
	void data_w(uint16_t data);

	void int_w(int state) { m_int_cb(state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_int_cb;

	device_psion_sibo_interface *m_card;
};


// ======================> device_psion_sibo_interface

class device_psion_sibo_interface : public device_interface
{
public:
	virtual uint8_t data_r() { return 0x00; }
	virtual void data_w(uint16_t data) { }

protected:
	device_psion_sibo_interface(const machine_config &mconfig, device_t &device);

	psion_sibo_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_SIBO_SLOT, psion_sibo_slot_device)

void psion_sibo_devices(device_slot_interface &device);


#endif // MAME_BUS_PSION_SIBO_SLOT_H
