// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor NMC9306 256-Bit Serial EEPROM emulation

**********************************************************************/

#include "emu.h"
#include "nmc9306.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1

#define RAM_SIZE 32


// instructions
enum
{
	OTHER = 0,
	WRITE,          // write register A3A2A1A0
	READ,           // read register  A3A2A1A0
	ERASE           // erase register A3A2A1A0
};

// other instructions
enum
{
	EWDS = 0,       // erase/write disable
	WRAL,           // write all registers
	ERAL,           // erase all registers
	EWEN            // erase/write enable
};

// states
enum
{
	STATE_IDLE = 0,
	STATE_COMMAND,
	STATE_ADDRESS,
	STATE_DATA_IN,
	STATE_DATA_OUT,
	STATE_ERASE
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

// device type definition
const device_type NMC9306 = &device_creator<nmc9306_device>;

//-------------------------------------------------
//  nmc9306_device - constructor
//-------------------------------------------------

inline UINT16 nmc9306_device::read(offs_t offset)
{
	return m_register[offset];
}


//-------------------------------------------------
//  nmc9306_device - constructor
//-------------------------------------------------

inline void nmc9306_device::write(offs_t offset, UINT16 data)
{
	if (m_ewen)
	{
		m_register[offset] &= data;
	}
}


//-------------------------------------------------
//  nmc9306_device - constructor
//-------------------------------------------------

inline void nmc9306_device::erase(offs_t offset)
{
	if (m_ewen)
	{
		m_register[offset] = 0xffff;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nmc9306_device - constructor
//-------------------------------------------------

nmc9306_device::nmc9306_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NMC9306, "NMC9306", tag, owner, clock, "nmc9306", __FILE__),
		device_nvram_interface(mconfig, *this),
		m_state(STATE_IDLE),
		m_ewen(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmc9306_device::device_start()
{
	memset(m_register, 0, sizeof(m_register));

	// state saving
	save_item(NAME(m_bits));
	save_item(NAME(m_state));
	save_item(NAME(m_command));
	save_item(NAME(m_address));
	save_item(NAME(m_data));
	save_item(NAME(m_ewen));
	save_item(NAME(m_cs));
	save_item(NAME(m_sk));
	save_item(NAME(m_do));
	save_item(NAME(m_di));
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void nmc9306_device::nvram_default()
{
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void nmc9306_device::nvram_read(emu_file &file)
{
	file.read(m_register, RAM_SIZE);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void nmc9306_device::nvram_write(emu_file &file)
{
	file.write(m_register, RAM_SIZE);
}


//-------------------------------------------------
//  cs_w - chip select input
//-------------------------------------------------

WRITE_LINE_MEMBER( nmc9306_device::cs_w )
{
	m_cs = state;
}


//-------------------------------------------------
//  ck_w - serial clock input
//-------------------------------------------------

WRITE_LINE_MEMBER( nmc9306_device::sk_w )
{
	m_sk = state;

	if (!m_cs || !m_sk) return;

	switch (m_state)
	{
	case STATE_IDLE:
		if (LOG) logerror("NMC9306 '%s' Idle %u\n", tag().c_str(), m_di);

		if (m_di)
		{
			// start bit received
			m_state = STATE_COMMAND;
			m_bits = 0;
		}
		break;

	case STATE_COMMAND:
		if (LOG) logerror("NMC9306 '%s' Command Bit %u\n", tag().c_str(), m_di);

		m_command <<= 1;
		m_command |= m_di;
		m_bits++;

		if (m_bits == 4)
		{
			m_state = STATE_ADDRESS;
			m_bits = 0;
		}
		break;

	case STATE_ADDRESS:
		if (LOG) logerror("NMC9306 '%s' Address Bit %u\n", tag().c_str(), m_di);

		m_address <<= 1;
		m_address |= m_di;
		m_bits++;

		if (m_bits == 4)
		{
			switch ((m_command >> 2) & 0x03)
			{
			case OTHER:
				switch (m_command & 0x03)
				{
				case EWDS:
					if (LOG) logerror("NMC9306 '%s' EWDS\n", tag().c_str());
					m_ewen = false;
					m_state = STATE_IDLE;
					break;

				case WRAL:
					if (LOG) logerror("NMC9306 '%s' WRAL\n", tag().c_str());
					break;

				case ERAL:
					if (LOG) logerror("NMC9306 '%s' ERAL\n", tag().c_str());
					break;

				case EWEN:
					if (LOG) logerror("NMC9306 '%s' EWEN\n", tag().c_str());
					m_ewen = true;
					m_state = STATE_IDLE;
					break;
				}
				break;

			case WRITE:
				if (LOG) logerror("NMC9306 '%s' WRITE %u\n", tag().c_str(), m_address & 0x0f);
				m_state = STATE_DATA_IN;
				break;

			case READ:
				if (LOG) logerror("NMC9306 '%s' READ %u\n", tag().c_str(), m_address & 0x0f);
				m_data = read(m_address & 0x0f);
				m_state = STATE_DATA_OUT;
				break;

			case ERASE:
				if (LOG) logerror("NMC9306 '%s' ERASE %u\n", tag().c_str(), m_address & 0x0f);
				erase(m_address & 0x0f);
				m_state = STATE_ERASE;
				break;
			}

			m_bits = 0;
		}
		break;

	case STATE_DATA_IN:
		if (LOG) logerror("NMC9306 '%s' Data Bit IN %u\n", tag().c_str(), m_di);

		m_data <<= 1;
		m_data |= m_di;
		m_bits++;

		if (m_bits == 16)
		{
			write(m_address & 0x0f, m_data);

			m_state = STATE_IDLE;
		}
		break;

	case STATE_DATA_OUT:
		if (LOG) logerror("NMC9306 '%s' Data Bit OUT %u\n", tag().c_str(), m_di);

		m_do = BIT(m_data, 15);
		m_data <<= 1;
		m_bits++;

		if (m_bits == 16)
		{
			m_state = STATE_IDLE;
		}
		break;
	}
}


//-------------------------------------------------
//  di_w - serial data input
//-------------------------------------------------

WRITE_LINE_MEMBER( nmc9306_device::di_w )
{
	m_di = state;
}


//-------------------------------------------------
//  do_r - serial data output
//-------------------------------------------------

READ_LINE_MEMBER( nmc9306_device::do_r )
{
	return m_do;
}
