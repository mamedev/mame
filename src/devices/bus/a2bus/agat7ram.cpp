// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat7ram.c

    Implemention of the Agat-7 RAM card

*********************************************************************/

#include "agat7ram.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_AGAT7RAM = device_creator<a2bus_agat7ram_device>;

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_agat7ram_device::a2bus_agat7ram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this), m_inh_state(0), m_last_offset(0), m_main_bank(0)
{
}

a2bus_agat7ram_device::a2bus_agat7ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2BUS_AGAT7RAM, "Agat-7 32K RAM Card", tag, owner, clock, "a7ram", __FILE__),
	device_a2bus_card_interface(mconfig, *this), m_inh_state(0), m_last_offset(0), m_main_bank(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_agat7ram_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	memset(m_ram, 0, 32*1024);

	save_item(NAME(m_inh_state));
	save_item(NAME(m_ram));
	save_item(NAME(m_main_bank));
	save_item(NAME(m_last_offset));
}

void a2bus_agat7ram_device::device_reset()
{
	m_inh_state = INH_NONE;
	m_main_bank = 0;
	m_last_offset = -1;
}

void a2bus_agat7ram_device::do_io(int offset)
{
	int old_inh_state = m_inh_state;

	m_last_offset = offset;
	m_inh_state = INH_NONE;
	m_main_bank = 0;

	if (offset & 0x8)
	{
		m_inh_state = INH_READ|INH_WRITE;
	}

	if (offset & 0x1)
	{
		m_main_bank = 0x4000;
	}

	if (m_inh_state != old_inh_state)
	{
		recalc_slot_inh();
	}

#if 1
	logerror("RAM: (ofs %02x) new state %c%c main=%05x\n",
			offset,
			(m_inh_state & INH_READ) ? 'R' : 'x',
			(m_inh_state & INH_WRITE) ? 'W' : 'x',
			m_main_bank);
#endif
}


/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_agat7ram_device::read_cnxx(address_space &space, uint8_t offset)
{
	return m_last_offset < 0 ? 0 : (m_last_offset & 0x7f);
}


/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
-------------------------------------------------*/

void a2bus_agat7ram_device::write_cnxx(address_space &space, uint8_t offset, uint8_t data)
{
	do_io(offset);
}

uint8_t a2bus_agat7ram_device::read_inh_rom(address_space &space, uint16_t offset)
{
	assert(m_inh_state & INH_READ); // this should never happen

	return m_ram[(offset & 0x3fff) + m_main_bank];
}

void a2bus_agat7ram_device::write_inh_rom(address_space &space, uint16_t offset, uint8_t data)
{
	// are writes enabled?
	if (!(m_inh_state & INH_WRITE))
	{
		return;
	}

	m_ram[(offset & 0x3fff) + m_main_bank] = data;
}

int a2bus_agat7ram_device::inh_type()
{
	return m_inh_state;
}
