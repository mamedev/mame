// license:BSD-3-Clause
// copyright-holders:Thorbjørn Ravn Andersen
/***************************************************************************

    Regnecentralen RC702 / RC703 -- Z80 PIO port slot

    Generic 8-bit + STB/RDY peripheral slot for the RC702's Z80-PIO ports
    (J3 and J4 on the back of the machine).  Modelled on
    bus/einstein/userport -- `read()` / `write(uint8_t)` / `rdy_w(int)`
    interface; cards assert STB into the chip via the slot's
    out_strobe_handler.

    Card options: keyboard (default on PIO-A).  PIO-B defaults to empty.

***************************************************************************/

#ifndef MAME_REGNECENTRALEN_PIO_PORT_PIO_PORT_H
#define MAME_REGNECENTRALEN_PIO_PORT_PIO_PORT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_rc702_pio_port_interface;

// supported devices
void rc702_pio_port_cards(device_slot_interface &device);

class rc702_pio_port_device :
	public device_t,
	public device_single_card_slot_interface<device_rc702_pio_port_interface>
{
public:
	// construction/destruction
	rc702_pio_port_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: rc702_pio_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		rc702_pio_port_cards(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	rc702_pio_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~rc702_pio_port_device();

	// driver-side: bind handlers wiring the slot back into the Z80 PIO chip
	auto out_strobe_handler() { return m_strobe_handler.bind(); }

	// called from card device -- assert STB into the chip
	void strobe_w(int state) { m_strobe_handler(state); }

	// chip-side: PIO uses these to talk to the card
	uint8_t read();
	void write(uint8_t data);
	void rdy_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

	device_rc702_pio_port_interface *m_card;

private:
	devcb_write_line m_strobe_handler;
};

// abstract interface -- every card implements these
class device_rc702_pio_port_interface : public device_interface
{
public:
	virtual ~device_rc702_pio_port_interface();

	// chip is reading from the port: card produces a byte
	virtual uint8_t read() { return 0xff; }

	// chip wrote to the port: card consumes a byte
	virtual void write(uint8_t data) { }

	// RDY edge from chip (BRDY on Port B, ARDY on Port A)
	virtual void rdy_w(int state) { }

protected:
	device_rc702_pio_port_interface(const machine_config &mconfig, device_t &device);

	rc702_pio_port_device *m_slot;
};

DECLARE_DEVICE_TYPE(RC702_PIO_PORT, rc702_pio_port_device)

#endif // MAME_REGNECENTRALEN_PIO_PORT_PIO_PORT_H
