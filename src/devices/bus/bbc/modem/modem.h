// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Internal Modem port

**********************************************************************

  Pinout:

              1  NIRQ
              2  NRST
              3  BD0
              4  BD1
              5  BD2
              6  BD3
              7  BD4
              8  BD5
              9  BD6
             10  BD7
             11  MODEM
             12  A0
             13  A1
             14  A2
             15  A3
             16  1MHzE
             17  R/W
             18  +5V
             19  -5V
             20  0V

**********************************************************************/

#ifndef MAME_BUS_BBC_MODEM_MODEM_H
#define MAME_BUS_BBC_MODEM_MODEM_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_modem_slot_device

class device_bbc_modem_interface;

class bbc_modem_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_modem_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_modem_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: bbc_modem_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_modem_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	void irq_w(int state) { m_irq_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_bbc_modem_interface *m_card;

private:
	devcb_write_line m_irq_handler;
};


// ======================> device_bbc_modem_interface

class device_bbc_modem_interface : public device_interface
{
public:
	virtual uint8_t read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data) { }

protected:
	device_bbc_modem_interface(const machine_config &mconfig, device_t &device);

	bbc_modem_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_MODEM_SLOT, bbc_modem_slot_device)

void bbcm_modem_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_MODEM_MODEM_H
