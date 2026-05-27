// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    titan3plus3.cpp

    Implemention of the Titan "/// Plus //"" card.
    By R. Belmont, based on reverse-engineering by Rob Justice.

    This extends the Apple ///'s II emulation to include a 128K
    extended language card and Apple II compatible game I/O.

    The card enables itself if the /// is at 1 MHz and accesses the
    card's $CNxx ROM space.  If the /// goes back to 2 MHz mode at
    any time, the card disables itself.

    Address ranges this messes with:
    $C05X - passed through to ///, but also used for annunciators on A2 gameio
    $C06X - /// doesn't see when card enabled, A2 paddles
    $C07X - passed through to ///, but also used for A2 paddle reset
    $C08X - unused by /// (language card control)
    $D000-$FFFF: usual (for A2) language card space.  Not so usual for ///.

*********************************************************************/

#include "emu.h"
#include "titan3plus2.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_TITAN3PLUS2, a2bus_titan3plus2_device, "titan3p2", "Titan /// Plus //")

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void a2bus_titan3plus2_device::device_add_mconfig(machine_config &config)
{
	APPLE2_GAMEIO(config, m_gameio, apple2_gameio_device::default_options, nullptr);
}

a2bus_titan3plus2_device::a2bus_titan3plus2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_a2bus_card_interface(mconfig, *this),
	m_gameio(*this, "gameio"),
	m_inh_state(0), m_last_offset(0), m_dxxx_bank(0), m_main_bank(0), m_enabled(false)
{
}

a2bus_titan3plus2_device::a2bus_titan3plus2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_titan3plus2_device(mconfig, A2BUS_TITAN3PLUS2, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_titan3plus2_device::device_start()
{
	memset(m_ram, 0, 128*1024);

	save_item(NAME(m_inh_state));
	save_item(NAME(m_ram));
	save_item(NAME(m_dxxx_bank));
	save_item(NAME(m_main_bank));
	save_item(NAME(m_last_offset));
	save_item(NAME(m_enabled));

	m_x_calibration = m_y_calibration = attotime::from_nsec(10800).as_double();
}

void a2bus_titan3plus2_device::device_reset()
{
	reset_from_bus();
}

void a2bus_titan3plus2_device::reset_from_bus()
{
	m_inh_state = INH_NONE;
	m_dxxx_bank = 0;
	m_main_bank = 0;
	m_last_offset = -1;
}

void a2bus_titan3plus2_device::do_io(int offset)
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
    read_c08x - called for reads from the LC switches
-------------------------------------------------*/

uint8_t a2bus_titan3plus2_device::read_c08x(uint8_t offset)
{
	if (!machine().side_effects_disabled())
	{
		do_io(offset & 0xf);
	}
	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to the LC switches
-------------------------------------------------*/

void a2bus_titan3plus2_device::write_c08x(uint8_t offset, uint8_t data)
{
	do_io(offset & 0xf);
}

uint8_t a2bus_titan3plus2_device::read_inh_rom(uint16_t offset)
{
	if (offset < 0xd000)
	{
		if ((offset >= 0xc080) && (offset <= 0xc08f))
		{
			return read_c08x(offset & 0xf);
		}

		if ((offset >= 0xc060) && (offset <= 0xc06f))
		{
			switch (offset & 0x7)
			{
			case 1: // button 0
				return (m_gameio->sw0_r() ? 0x80 : 0);

			case 2: // button 1
				return (m_gameio->sw1_r() ? 0x80 : 0);

			case 3: // button 2
				return (m_gameio->sw2_r() ? 0x80 : 0);

			case 4: // joy 1 X axis
				if (!m_gameio->is_device_connected())
					return 0x80;
				return ((machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0);

			case 5: // joy 1 Y axis
				if (!m_gameio->is_device_connected())
					return 0x80;
				return ((machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0);

			case 6: // joy 2 X axis
				if (!m_gameio->is_device_connected())
					return 0x80;
				return ((machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0);

			case 7: // joy 2 Y axis
				if (!m_gameio->is_device_connected())
					return 0x80;
				return ((machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0);
			}
		}
	}
	else
	{
		if (offset < 0xe000)
		{
			return m_ram[(offset & 0xfff) + m_dxxx_bank + m_main_bank];
		}

		return m_ram[(offset & 0x1fff) + 0x2000 + m_main_bank];
	}

	return 0xff;
}

void a2bus_titan3plus2_device::write_inh_rom(uint16_t offset, uint8_t data)
{
	if (offset < 0xd000)
	{
		if ((offset >= 0xc080) && (offset <= 0xc08f))
		{
			write_c08x(offset & 0xf, data);
			return;
		}
	}
	else
	{
		if (offset < 0xe000)
		{
			m_ram[(offset & 0xfff) + m_dxxx_bank + m_main_bank] = data;
			return;
		}

		m_ram[(offset & 0x1fff) + 0x2000 + m_main_bank] = data;
	}
}

u8 a2bus_titan3plus2_device::read_cnxx(u8 offset)
{
	m_enabled = true;
	return 0xff;
}

// returns if we want to /INH a read or write to a specific address
bool a2bus_titan3plus2_device::inh_check(u16 offset, bool bIsWrite)
{
	if (!m_enabled)
	{
		return false;
	}

	// handle LC
	if (offset >= 0xd000)
	{
		if ((bIsWrite) && (m_inh_state & INH_WRITE))
		{
			return true;
		}

		if (!(bIsWrite) && (m_inh_state & INH_READ))
		{
			return true;
		}
	}

	if ((offset >= 0xc080) && (offset <= 0xc08f))
	{
		return true;
	}

	if ((offset >= 0xc070) && (offset <= 0xc07f))
	{
		m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl0_r();
		m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl1_r();
		m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl2_r();
		m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl3_r();

		return false;   // Apple /// should still handle this
	}

	if ((offset >= 0xc060) && (offset <= 0xc06f))
	{
		return true;
	}

	if ((offset >= 0xc058) && (offset <= 0xc05f))
	{
		switch (offset & 0xf)
		{
			case 8:
			case 9:
				m_gameio->an0_w(offset & 1);
				break;
			case 0xa:
			case 0xb:
				m_gameio->an1_w(offset & 1);
				break;
			case 0xc:
			case 0xd:
				m_gameio->an2_w(offset & 1);
				break;
			case 0xe:
			case 0xf:
				m_gameio->an3_w(offset & 1);
				break;
		}
		return false;   // Apple /// should still see this
	}

	return false;
}
