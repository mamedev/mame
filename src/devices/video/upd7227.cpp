// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    uPD7227 Intelligent Dot-Matrix LCD Controller/Driver emulation

**********************************************************************/

#include "emu.h"
#include "upd7227.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UPD7227, upd7227_device, "upd7227", "NEC uPD7227")


void upd7227_device::upd7227_map(address_map &map)
{
	map(0x00, 0x27).ram();
	map(0x40, 0x67).ram();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7227_device - constructor
//-------------------------------------------------

upd7227_device::upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD7227, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("videoram", ENDIANNESS_BIG, 8, 7, 0, address_map_constructor(FUNC(upd7227_device::upd7227_map), this))
	, m_cs(1)
	, m_cd(1)
	, m_sck(1)
	, m_si(1)
	, m_so(1)
	, m_data(0)
	, m_bits(0)
	, m_pa(0)
	, m_mode(CMD_SWM)
	, m_disp(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7227_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_cd));
	save_item(NAME(m_sck));
	save_item(NAME(m_si));
	save_item(NAME(m_so));
	save_item(NAME(m_data));
	save_item(NAME(m_bits));
	save_item(NAME(m_pa));
	save_item(NAME(m_mode));
	save_item(NAME(m_disp));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7227_device::device_reset()
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd7227_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

uint32_t upd7227_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int bank = 0; 2 > bank; ++bank)
	{
		for (int column = 0; 40 > column; ++column)
		{
			uint8_t const data = m_disp ? space().read_byte((bank << 6) | column) : 0;

			for (int y = 0; 8 > y; ++y)
			{
				int const sx = m_sx + column;
				int const sy = m_sy + (bank << 3) + y;

				if (cliprect.contains(sx, sy))
					bitmap.pix(sy, sx) = BIT(data, y);
			}
		}
	}

	return 0;
}


//-------------------------------------------------
//  write_byte - process a received byte
//-------------------------------------------------

void upd7227_device::write_byte(uint8_t data)
{
	if (m_cd)
	{
		if (data & CMD_LDPI)
		{
			m_pa = data & 0x7f;
		}
		else if ((data & 0xf8) == CMD_BSET)
		{
			space().write_byte(m_pa, space().read_byte(m_pa) | (1 << (data & 0x07)));
		}
		else if ((data & 0xf8) == CMD_BRESET)
		{
			space().write_byte(m_pa, space().read_byte(m_pa) & ~(1 << (data & 0x07)));
		}
		else
		{
			switch (data)
			{
			case CMD_DISP_ON:   m_disp = 1; break;
			case CMD_DISP_OFF:  m_disp = 0; break;

			case CMD_SRM:
			case CMD_SWM:
			case CMD_SORM:
			case CMD_SANDM:
			case CMD_SCM:
				m_mode = data;
				break;

			default:
				LOG("%s: unhandled command %02x\n", machine().describe_context(), data);
				break;
			}
		}
	}
	else
	{
		uint8_t byte = space().read_byte(m_pa);

		switch (m_mode)
		{
		case CMD_SWM:   byte = data; break;
		case CMD_SORM:  byte |= data; break;
		case CMD_SANDM: byte &= data; break;
		case CMD_SCM:   byte ^= data; break;
		default: break;
		}

		space().write_byte(m_pa, byte);

		m_pa = (m_pa + 1) & 0x7f;
	}
}


//-------------------------------------------------
//  cs_w - chip select
//-------------------------------------------------

void upd7227_device::cs_w(int state)
{
	if (m_cs != state)
		m_bits = 0;

	m_cs = state;
}


//-------------------------------------------------
//  cd_w - command/data select
//-------------------------------------------------

void upd7227_device::cd_w(int state)
{
	m_cd = state;
}


//-------------------------------------------------
//  sck_w - serial clock
//-------------------------------------------------

void upd7227_device::sck_w(int state)
{
	if (!m_cs && !m_sck && state)
	{
		m_data = (m_data >> 1) | (m_si << 7);

		if (++m_bits == 8)
		{
			m_bits = 0;
			write_byte(m_data);
		}
	}

	m_sck = state;
}


//-------------------------------------------------
//  si_w - serial input
//-------------------------------------------------

void upd7227_device::si_w(int state)
{
	m_si = state;
}


//-------------------------------------------------
//  so_r - serial output/busy
//-------------------------------------------------

int upd7227_device::so_r()
{
	return m_so;
}
