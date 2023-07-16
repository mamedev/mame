// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Honda Expansion slot emulation

**********************************************************************

    Pin  Name    Comments

    1    VEXT
    2    RTS
    3    DTR
    4    TXD
    5    DSR
    6    DCD
    7    CTS
    8    RXD
    9    SDOE
    10   XSTAT
    11   EXON
    12   INT
    13   SD
    14   SCK
    15   GND

**********************************************************************/

#ifndef MAME_BUS_PSION_HONDA_SLOT_H
#define MAME_BUS_PSION_HONDA_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_honda_slot_device

class device_psion_honda_interface;

class psion_honda_slot_device : public device_t, public device_single_card_slot_interface<device_psion_honda_interface>
{
public:
	// construction/destruction
	template <typename T>
	psion_honda_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: psion_honda_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	psion_honda_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto int_cb() { return m_int_cb.bind(); }

	uint8_t data_r();
	void data_w(uint16_t data);

	void int_w(int state) { m_int_cb(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_psion_honda_interface *m_card;

private:
	devcb_write_line m_int_cb;
};


// ======================> device_psion_honda_interface

class device_psion_honda_interface : public device_interface
{
public:
	virtual uint8_t data_r() { return 0x00; }
	virtual void data_w(uint16_t data) { }

protected:
	device_psion_honda_interface(const machine_config &mconfig, device_t &device);

	psion_honda_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_HONDA_SLOT, psion_honda_slot_device)

void psion_honda_devices(device_slot_interface &device);


#endif // MAME_BUS_PSION_HONDA_SLOT_H
