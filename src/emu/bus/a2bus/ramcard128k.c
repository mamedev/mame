// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ramcard128k.c

    Implemention of the Saturn Systems 128K extended language card

*********************************************************************/

#include "ramcard128k.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_RAMCARD128K = &device_creator<a2bus_ssramcard_device>;

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ssramcard_device::a2bus_ssramcard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this)
{
}

a2bus_ssramcard_device::a2bus_ssramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_RAMCARD128K, "Saturn Systems 128K Extended Language Card", tag, owner, clock, "ssram128", __FILE__),
	device_a2bus_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ssramcard_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	memset(m_ram, 0, 128*1024);

	save_item(NAME(m_inh_state));
	save_item(NAME(m_ram));
	save_item(NAME(m_dxxx_bank));
	save_item(NAME(m_main_bank));
	save_item(NAME(m_last_offset));
}

void a2bus_ssramcard_device::device_reset()
{
	m_inh_state = INH_NONE;
	m_dxxx_bank = 0;
	m_main_bank = 0;
	m_last_offset = -1;
}

void a2bus_ssramcard_device::do_io(int offset)
{
	int old_inh_state = m_inh_state;

	switch (offset)
	{
		case 0x1: case 0x3: case 0x9: case 0xb:
			if (offset != m_last_offset)
			{
				m_last_offset = offset;
				return;
			}
			break;
	}
	m_last_offset = offset;

	if (offset & 4)
	{
		switch (offset)
		{
			case 0x4: m_main_bank = 0x00000; break;
			case 0x5: m_main_bank = 0x04000; break;
			case 0x6: m_main_bank = 0x08000; break;
			case 0x7: m_main_bank = 0x0c000; break;
			case 0xc: m_main_bank = 0x10000; break;
			case 0xd: m_main_bank = 0x14000; break;
			case 0xe: m_main_bank = 0x18000; break;
			case 0xf: m_main_bank = 0x1c000; break;
		}
	}
	else
	{
		m_inh_state = INH_NONE;
		m_dxxx_bank = 0;

		if (offset & 0x1)
		{
			m_inh_state |= INH_WRITE;
		}

		switch(offset & 0x03)
		{
			case 0x00:
			case 0x03:
				m_inh_state |= INH_READ;
				break;
		}

		if (!(offset & 8))
		{
			m_dxxx_bank = 0x1000;
		}
	}

	if (m_inh_state != old_inh_state)
	{
		recalc_slot_inh();
	}

	#if 0
	printf("LC: (ofs %x) new state %c%c dxxx=%04x main=%05x\n",
			offset,
			(m_inh_state & INH_READ) ? 'R' : 'x',
			(m_inh_state & INH_WRITE) ? 'W' : 'x',
			m_dxxx_bank, m_main_bank);
	#endif
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_ssramcard_device::read_c0nx(address_space &space, UINT8 offset)
{
	do_io(offset & 0xf);
	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ssramcard_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	do_io(offset & 0xf);
}

UINT8 a2bus_ssramcard_device::read_inh_rom(address_space &space, UINT16 offset)
{
	assert(m_inh_state & INH_READ); // this should never happen

	if (offset < 0xe000)
	{
		return m_ram[(offset & 0xfff) + m_dxxx_bank + m_main_bank];
	}

	return m_ram[(offset & 0x1fff) + 0x2000 + m_main_bank];
}

void a2bus_ssramcard_device::write_inh_rom(address_space &space, UINT16 offset, UINT8 data)
{
	// are writes enabled?
	if (!(m_inh_state & INH_WRITE))
	{
		return;
	}

	if (offset < 0xe000)
	{
		m_ram[(offset & 0xfff) + m_dxxx_bank + m_main_bank] = data;
		return;
	}

	m_ram[(offset & 0x1fff) + 0x2000 + m_main_bank] = data;
}

int a2bus_ssramcard_device::inh_type()
{
	return m_inh_state;
}
