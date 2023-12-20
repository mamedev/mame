// license:BSD-3-Clause
// copyright-holders:Kelvin Sherlock
/*********************************************************************

    uthernet2.cpp

    Apple II Uthernet II Card

    WIZNet 5100 with indirect bus interface mode.


*********************************************************************/

#include "emu.h"
#include "uthernet2.h"
#include "machine/w5100.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_uthernet2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	a2bus_uthernet2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_uthernet2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:

	required_device<w5100_device> m_netinf;

	void netinf_irq_w(int state);
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_uthernet2_device::a2bus_uthernet2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_uthernet2_device(mconfig, A2BUS_UTHERNET2, tag, owner, clock)
{
}

a2bus_uthernet2_device::a2bus_uthernet2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_netinf(*this, "w5100")
{
}

void a2bus_uthernet2_device::device_add_mconfig(machine_config &config)
{
	W5100(config, m_netinf, 25_MHz_XTAL);

	m_netinf->irq_handler().set(FUNC(a2bus_uthernet2_device::netinf_irq_w));
}

void a2bus_uthernet2_device::device_start()
{
}

void a2bus_uthernet2_device::device_reset()
{
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_uthernet2_device::read_c0nx(uint8_t offset)
{
	/* only 0x04-0x07 are documented but all addresses are wired up. */
	switch(offset & 0x03)
	{
		case 0x00: return m_netinf->read(0x00); /* mode register */
		case 0x01: return m_netinf->read(0x01); /* IDM high */
		case 0x02: return m_netinf->read(0x02); /* IDM low */
		case 0x03: return m_netinf->read(0x03); /* data/register */
	}
	return 0x00;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_uthernet2_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch(offset & 0x03)
	{
		case 0x00: m_netinf->write(0x00, data); break; /* mode register */
		case 0x01: m_netinf->write(0x01, data); break; /* IDM high */
		case 0x02: m_netinf->write(0x02, data); break; /* IDM low */
		case 0x03: m_netinf->write(0x03, data); break; /* data/register */
	}

}

void a2bus_uthernet2_device::netinf_irq_w(int state)
{
	if (state) {
		raise_slot_irq();
	} else {
		lower_slot_irq();
	}
}

} // anonymous namespace

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_UTHERNET2, device_a2bus_card_interface, a2bus_uthernet2_device, "a2uthernet2", "a2RetroSystems Uthernet II")
