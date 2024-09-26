// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Econet Module

**********************************************************************

  Pinout:

              1  NEFIQ
              2  NWBE
              3  NS2
              4  CLK2
              5  LA2
              6  LA3
              7  BD0
              8  BD1
              9  BD2
             10  BD3
             11  BD4
             12  BD5
             13  BD6
             14  BD7
             15  NRST
             16  0V
             17  +5V

**********************************************************************/

#ifndef MAME_BUS_ARCHIMEDES_ECONET_SLOT_H
#define MAME_BUS_ARCHIMEDES_ECONET_SLOT_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> archimedes_econet_slot_device

class device_archimedes_econet_interface;

class archimedes_econet_slot_device : public device_t, public device_single_card_slot_interface<device_archimedes_econet_interface>
{
public:
	// construction/destruction
	template <typename T>
	archimedes_econet_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: archimedes_econet_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	archimedes_econet_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// callbacks
	auto efiq_handler() { return m_efiq_handler.bind(); }

	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

	void efiq_w(int state) { m_efiq_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_archimedes_econet_interface *m_device;

private:
	devcb_write_line m_efiq_handler;
};


// ======================> device_archimedes_econet_interface

class device_archimedes_econet_interface : public device_interface
{
public:
	virtual u8 read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, u8 data) { }

protected:
	device_archimedes_econet_interface(const machine_config &mconfig, device_t &device);

	archimedes_econet_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(ARCHIMEDES_ECONET_SLOT, archimedes_econet_slot_device)

void archimedes_econet_devices(device_slot_interface &device);


#endif // MAME_BUS_ARCHIMEDES_ECONET_SLOT_H
