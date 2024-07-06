// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Tube emulation

**********************************************************************

  Pinout:

             0V   1   2  R/NW (read/not-write)
             0V   3   4  2MHzE
             0V   5   6  NIRQ (not-interrupt request)
             0V   7   8  NTUBE
             0V   9  10  NRST (not-reset)
             0V  11  12  D0
             0V  13  14  D1
             0V  15  16  D2
             0V  17  18  D3
             0V  19  20  D4
             0V  21  22  D5
             0V  23  24  D6
             0V  25  26  D7
             0V  27  28  A0
             0V  29  30  A1
            +5V  31  32  A2
            +5V  33  34  A3
            +5V  35  36  A4
            +5V  37  38  A5
            +5V  39  40  A6

**********************************************************************/

#ifndef MAME_BUS_BBC_TUBE_TUBE_H
#define MAME_BUS_BBC_TUBE_TUBE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_slot_device

class device_bbc_tube_interface;

class bbc_tube_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_tube_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_tube_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, char const *default_option)
		: bbc_tube_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
		set_insert_rom(true);
	}

	bbc_tube_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	void set_insert_rom(bool insert_rom) { m_insert_rom = insert_rom; }
	bool insert_rom() { return m_insert_rom; }

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }

	uint8_t host_r(offs_t offset);
	void host_w(offs_t offset, uint8_t data);

	// JIM is not accessible on the Tube, but the PMS-B2P device connects to the Tube and also the NPGFD line of the 1MHz bus
	uint8_t jim_r(offs_t offset);
	void jim_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_irq_handler(state); }

protected:
	// device_t overrides
	virtual void device_start() override;

	device_bbc_tube_interface *m_card;

private:
	devcb_write_line m_irq_handler;

	bool m_insert_rom;
};


// ======================> device_bbc_tube_interface

class device_bbc_tube_interface : public device_interface
{
public:
	// reading and writing
	virtual uint8_t host_r(offs_t offset) { return 0xfe; }
	virtual void host_w(offs_t offset, uint8_t data) { }
	virtual uint8_t jim_r(offs_t offset) { return 0xff; }
	virtual void jim_w(offs_t offset, uint8_t data) { }

protected:
	device_bbc_tube_interface(const machine_config &mconfig, device_t &device);

	bbc_tube_slot_device *const m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_SLOT, bbc_tube_slot_device)

void bbc_tube_devices(device_slot_interface &device);
void bbc_extube_devices(device_slot_interface &device);
void bbc_intube_devices(device_slot_interface &device);
void electron_tube_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_TUBE_TUBE_H
