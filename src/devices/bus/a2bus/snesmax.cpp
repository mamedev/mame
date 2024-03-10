// license:BSD-3-Clause
// copyright-holders:Kelvin Sherlock
/*********************************************************************

    snesmax.cpp

    SNES controller serial interface card for the Apple II by Lukazi
    https://lukazi.blogspot.com/2021/06/game-controller-snes-max-snes.html

*********************************************************************/

#include "emu.h"
#include "snesmax.h"
#include "bus/snes_ctrl/ctrl.h"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_snes_max_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_snes_max_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_snes_max_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<snes_control_port_device> m_ctrl1;
	required_device<snes_control_port_device> m_ctrl2;

	uint32_t m_latch1;
	uint32_t m_latch2;
};

a2bus_snes_max_device::a2bus_snes_max_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_snes_max_device(mconfig, A2BUS_SNES_MAX, tag, owner, clock)
{
}

a2bus_snes_max_device::a2bus_snes_max_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_ctrl1(*this, "ctrl1"),
		m_ctrl2(*this, "ctrl2"),
		m_latch1(0),
		m_latch2(0)
{
}

void a2bus_snes_max_device::device_add_mconfig(machine_config &config)
{
	SNES_CONTROL_PORT(config, m_ctrl1, snes_control_port_devices, "joypad");
	SNES_CONTROL_PORT(config, m_ctrl2, snes_control_port_devices, "joypad");
}

void a2bus_snes_max_device::device_start()
{
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}


uint8_t a2bus_snes_max_device::read_c0nx(uint8_t offset)
{
	/* only data lines 6/7 are present so bits 0-5 should be floating bus. */
	return ((m_latch1 & 0x01) << 7) | ((m_latch2 & 0x01) << 6);
}

void a2bus_snes_max_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch(offset & 0x01)
	{
	case 0:
		/* latch */
		m_ctrl1->port_poll();
		m_ctrl2->port_poll();

		/* bit 16 indicates if the controller is plugged in */
		m_latch1 = m_ctrl1->get_card_device() ?  0x10000 : 0;
		m_latch2 = m_ctrl2->get_card_device() ?  0x10000 : 0;

		for (int i = 0; i < 16; ++i)
		{
			m_latch1 |= (m_ctrl1->read_pin4() & 1) << i;
			m_latch2 |= (m_ctrl2->read_pin4() & 1) << i;
		}
		m_latch1 = ~m_latch1;
		m_latch2 = ~m_latch2;
		break;
	case 1:
		/* clock */
		m_latch1 >>= 1;
		m_latch2 >>= 1;
		break;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SNES_MAX, device_a2bus_card_interface, a2bus_snes_max_device, "a2snesmax", "SNES MAX Joystick Card")
