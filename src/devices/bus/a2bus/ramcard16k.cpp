// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ramcard16k.c

    Implemention of the Apple II 16K RAM card (aka "language card")

*********************************************************************/

#include "emu.h"
#include "ramcard16k.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_RAMCARD16K, a2bus_ramcard_device, "a2ram16k", "Apple II 16K Language Card")

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ramcard_device::a2bus_ramcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this), m_inh_state(0), m_prewrite(false), m_dxxx_bank(0)
{
}

a2bus_ramcard_device::a2bus_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ramcard_device(mconfig, A2BUS_RAMCARD16K, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ramcard_device::device_start()
{
	memset(m_ram, 0, 16*1024);

	save_item(NAME(m_inh_state));
	save_item(NAME(m_ram));
	save_item(NAME(m_dxxx_bank));
	save_item(NAME(m_prewrite));
}

void a2bus_ramcard_device::device_reset()
{
	m_inh_state = INH_WRITE;
	m_dxxx_bank = 0;
	m_prewrite = false;
}

void a2bus_ramcard_device::do_io(int offset, bool writing)
{
	int old_inh_state = m_inh_state;

	//any even access disables pre-write and writing
	if ((offset & 1) == 0)
	{
		m_prewrite = false;
		m_inh_state &= ~INH_WRITE;
	}

	//any write disables pre-write
	//has no effect on write-enable if writing was enabled already
	if (writing == true)
	{
		m_prewrite = false;
	}
	//first odd read enables pre-write, second one enables writing
	else if ((offset & 1) == 1)
	{
		if (m_prewrite == false)
		{
			m_prewrite = true;
		}
		else
		{
			m_inh_state |= INH_WRITE;
		}
	}

	switch (offset & 3)
	{
		case 0:
		case 3:
		{
			m_inh_state |= INH_READ;
			break;
		}

		case 1:
		case 2:
		{
			m_inh_state &= ~INH_READ;
			break;
		}
	}

	m_dxxx_bank = 0;

	if (!(offset & 8))
	{
		m_dxxx_bank = 0x1000;
	}

	if (m_inh_state != old_inh_state)
	{
		recalc_slot_inh();
	}

	#if 0
	printf("LC: new state %c%c dxxx=%04x\n",
			(m_inh_state & INH_READ) ? 'R' : 'x',
			(m_inh_state & INH_WRITE) ? 'W' : 'x',
			m_dxxx_bank);
	#endif
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_ramcard_device::read_c0nx(uint8_t offset)
{
	do_io(offset & 0xf, false);
	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ramcard_device::write_c0nx(uint8_t offset, uint8_t data)
{
	do_io(offset & 0xf, true);
}

uint8_t a2bus_ramcard_device::read_inh_rom(uint16_t offset)
{
	assert(m_inh_state & INH_READ); // this should never happen

	if (offset < 0xe000)
	{
		return m_ram[(offset & 0xfff) + m_dxxx_bank];
	}

	return m_ram[(offset & 0x1fff) + 0x2000];
}

void a2bus_ramcard_device::write_inh_rom(uint16_t offset, uint8_t data)
{
	// are writes enabled?
	if (!(m_inh_state & INH_WRITE))
	{
		return;
	}

	if (offset < 0xe000)
	{
		m_ram[(offset & 0xfff) + m_dxxx_bank] = data;
		return;
	}

	m_ram[(offset & 0x1fff) + 0x2000] = data;
}

int a2bus_ramcard_device::inh_type()
{
	return m_inh_state;
}
