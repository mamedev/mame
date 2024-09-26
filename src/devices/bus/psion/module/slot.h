// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Extended Internal Expansion slot emulation

**********************************************************************

    Pin  Name    Comments

    1    GND     Should mate first when device inserted
    2    GND     Should mate first when device inserted
    3    AD0     8 bit multiplexed address and data bus pulled low
    4    AD1
    5    AD2
    6    AD3
    7    AD4
    8    AD5
    9    AD6
    10   AD7
    11   ALE     Address latch enable - high when valid address on AD0-AD7
    12   IOWR    I/O write strobe - active high, data valid on falling edge
    13   IORD    I/O read strobe - high when device can place valid data on AD0-AD7
    14   EES     External Expansion Select - high during I/O cycles to expansion device
    15   SCLX    512 KHz SCL signal for SLD bus - usually Hi-Z and pulled low
    16   DNC     For future expansion - do not use
    17   THERM   Connected to thermistor
    18   VB1     Connected to internal NiCd battery
    19   Vsup    Unregulated battery voltage - present all the time
    20   INTR    Active high interrupt input
    21   _EXON   Active low input pulled up to Vcc1 - pull low to switch machine on
    22   SD      SIBO serial protocol data line - pulled low
    23   SCLK    SIBO serial protocol clock line - Hi-Z in standby needs a pull down
    24   GND                     0v
    25   Vcc2    +5 volt supply, switched off in standby

**********************************************************************/

#ifndef MAME_BUS_PSION_MODULE_SLOT_H
#define MAME_BUS_PSION_MODULE_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_module_slot_device

class device_psion_module_interface;

class psion_module_slot_device : public device_t, public device_single_card_slot_interface<device_psion_module_interface>
{
public:
	// construction/destruction
	template <typename T>
	psion_module_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: psion_module_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	psion_module_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto intr_cb() { return m_intr_cb.bind(); }

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t data_r();
	void data_w(uint16_t data);

	void intr_w(int state) { m_intr_cb(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_psion_module_interface *m_card;

private:
	devcb_write_line m_intr_cb;
};


// ======================> device_psion_module_interface

class device_psion_module_interface : public device_interface
{
public:
	virtual uint8_t io_r(offs_t offset) { return 0x00; }
	virtual void io_w(offs_t offset, uint8_t data) { }

	virtual uint8_t data_r() { return 0x00; }
	virtual void data_w(uint16_t data) { }

protected:
	device_psion_module_interface(const machine_config &mconfig, device_t &device);

	psion_module_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_MODULE_SLOT, psion_module_slot_device)

void psion_hcmodule_devices(device_slot_interface &device);
void psion_mcmodule_devices(device_slot_interface &device);


#endif // MAME_BUS_PSION_MODULE_SLOT_H
