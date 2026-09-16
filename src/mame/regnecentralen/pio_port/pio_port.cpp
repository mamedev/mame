// license:BSD-3-Clause
// copyright-holders:Thorbjørn Ravn Andersen
/***************************************************************************

    Regnecentralen RC702 / RC703 -- Z80 PIO port slot

***************************************************************************/

#include "emu.h"
#include "pio_port.h"

// supported card devices
#include "keyboard.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RC702_PIO_PORT, rc702_pio_port_device, "rc702_pio_port", "RC702 PIO Port")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

rc702_pio_port_device::rc702_pio_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, RC702_PIO_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_rc702_pio_port_interface>(mconfig, *this),
	m_card(nullptr),
	m_strobe_handler(*this)
{
}

rc702_pio_port_device::~rc702_pio_port_device()
{
}

void rc702_pio_port_device::device_start()
{
	m_card = get_card_device();
}


//**************************************************************************
//  CHIP-SIDE FORWARDING
//**************************************************************************

uint8_t rc702_pio_port_device::read()
{
	return m_card ? m_card->read() : 0xff;
}

void rc702_pio_port_device::write(uint8_t data)
{
	if (m_card)
		m_card->write(data);
}

void rc702_pio_port_device::rdy_w(int state)
{
	if (m_card)
		m_card->rdy_w(state);
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_rc702_pio_port_interface::device_rc702_pio_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "rc702piocard")
{
	m_slot = dynamic_cast<rc702_pio_port_device *>(device.owner());
}

device_rc702_pio_port_interface::~device_rc702_pio_port_interface()
{
}


//**************************************************************************
//  SLOT OPTION LIST
//**************************************************************************

void rc702_pio_port_cards(device_slot_interface &device)
{
	device.option_add("keyboard", RC702_PIO_KEYBOARD);
}
