// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor NMC9306 256-Bit Serial EEPROM emulation

**********************************************************************/

#include "emu.h"
#include "nmc9306.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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
	STATE_START,
	STATE_COMMAND,
	STATE_ADDRESS,
	STATE_DATA_IN,
	STATE_DUMMY_OUT,
	STATE_DATA_OUT,
	STATE_ERASE,
	STATE_WRITE
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NMC9306, nmc9306_device, "nmc9306", "NMC9306 EEPROM")

//-------------------------------------------------
//  nmc9306_device - constructor
//-------------------------------------------------

inline u16 nmc9306_device::read(offs_t offset)
{
	return m_register[offset];
}


//-------------------------------------------------
//  nmc9306_device - constructor
//-------------------------------------------------

inline void nmc9306_device::write(offs_t offset, u16 data)
{
	if (m_ewen)
	{
		m_register[offset] = data;
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

nmc9306_device::nmc9306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NMC9306, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_region(*this, DEVICE_SELF),
	m_bits(0),
	m_state(STATE_IDLE),
	m_command(0),
	m_address(0),
	m_data(0),
	m_ewen(false),
	m_cs(0),
	m_sk(0),
	m_do(0),
	m_di(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmc9306_device::device_start()
{
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
	if (m_region.found())
	{
		if (m_region->bytes() != sizeof(m_register))
			fatalerror("%s incorrect region size", tag());

		std::memcpy(m_register, m_region->base(), sizeof(m_register));
	}
	else
	{
		for (auto & elem : m_register)
			elem = 0xffff;
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool nmc9306_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_register, RAM_SIZE);
	return !err && (actual == RAM_SIZE);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool nmc9306_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_register, RAM_SIZE);
	return !err;
}


//-------------------------------------------------
//  cs_w - chip select input
//-------------------------------------------------

void nmc9306_device::cs_w(int state)
{
	if (m_cs != state)
	{
		LOG("NMC9306 CS %u\n", state);

		if (m_cs && !state)
		{
			switch ((m_command >> 2) & 0x03)
			{
			case OTHER:
				switch (m_command & 0x03)
				{
				case WRAL:
					if (m_state == STATE_WRITE)
					{
						LOG("NMC9306 WRAL\n");
						for (int address = 0; address < 16; address++)
						{
							write(address, m_data);
						}
					}
					break;

				case ERAL:
					if (m_state == STATE_ERASE)
					{
						LOG("NMC9306 ERAL\n");
						for (int address = 0; address < 16; address++)
						{
							erase(address);
						}
					}
					break;
				}
				break;

			case WRITE:
				if (m_state == STATE_WRITE)
				{
					LOG("NMC9306 WRITE %u:%04x\n", m_address, m_data);
					write(m_address, m_data);
				}
				break;

			case ERASE:
				if (m_state == STATE_ERASE)
				{
					LOG("NMC9306 ERASE %u\n", m_address);
					erase(m_address);
				}
				break;
			}

			m_state = STATE_IDLE;
			m_bits = 0;
			m_do = 0;
		}
	}

	m_cs = state;
}


//-------------------------------------------------
//  ck_w - serial clock input
//-------------------------------------------------

void nmc9306_device::sk_w(int state)
{
	if (!m_cs) return;
	if (m_sk == state) return;

	LOG("NMC9306 SK %u\n", state);

	m_sk = state;

	if (m_sk && ((m_state == STATE_DUMMY_OUT) || (m_state == STATE_DATA_OUT))) return;
	if (!m_sk && (m_state != STATE_DUMMY_OUT) && (m_state != STATE_DATA_OUT)) return;

	switch (m_state)
	{
	case STATE_IDLE:
		LOG("NMC9306 Idle %u\n", m_di);

		if (!m_di)
		{
			// start bit received
			m_state = STATE_START;
		}
		break;

	case STATE_START:
		LOG("NMC9306 Start %u\n", m_di);

		if (m_di)
		{
			m_state = STATE_COMMAND;
			m_bits = 0;
		}
		break;

	case STATE_COMMAND:
		LOG("NMC9306 Command Bit %u\n", m_di);

		m_command <<= 1;
		m_command |= m_di;
		m_command &= 0xf;

		m_bits++;

		if (m_bits == 4)
		{
			m_state = STATE_ADDRESS;
			m_bits = 0;
		}
		break;

	case STATE_ADDRESS:
		LOG("NMC9306 Address Bit %u\n", m_di);

		m_address <<= 1;
		m_address |= m_di;
		m_address &= 0xf;

		m_bits++;

		if (m_bits == 4)
		{
			switch ((m_command >> 2) & 0x03)
			{
			case OTHER:
				switch (m_command & 0x03)
				{
				case EWDS:
					LOG("NMC9306 EWDS\n");
					m_ewen = false;
					m_state = STATE_IDLE;
					break;

				case WRAL:
					LOG("NMC9306 Command WRAL\n");
					m_state = STATE_DATA_IN;
					break;

				case ERAL:
					LOG("NMC9306 Command ERAL\n");
					m_state = ERASE;
					break;

				case EWEN:
					LOG("NMC9306 EWEN\n");
					m_ewen = true;
					m_state = STATE_IDLE;
					break;
				}
				break;

			case WRITE:
				LOG("NMC9306 Command WRITE %u\n", m_address);
				m_state = STATE_DATA_IN;
				break;

			case READ:
				m_data = read(m_address);
				LOG("NMC9306 READ %u:%04x\n", m_address, m_data);
				m_state = STATE_DUMMY_OUT;
				break;

			case ERASE:
				LOG("NMC9306 Command ERASE %u\n", m_address);
				m_state = ERASE;
				break;
			}

			m_bits = 0;
		}
		break;

	case STATE_DATA_IN:
		LOG("NMC9306 Data Bit IN %u\n", m_di);

		m_data <<= 1;
		m_data |= m_di;

		m_bits++;

		if (m_bits == 16)
		{
			m_state = STATE_WRITE;
		}
		break;

	case STATE_DUMMY_OUT:
		m_do = 0;

		LOG("NMC9306 Dummy Bit OUT %u\n", m_do);

		m_state = STATE_DATA_OUT;
		break;

	case STATE_DATA_OUT:
		m_do = BIT(m_data, 15);
		m_data <<= 1;

		LOG("NMC9306 Data Bit OUT %u\n", m_do);

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

void nmc9306_device::di_w(int state)
{
	m_di = state;
}


//-------------------------------------------------
//  do_r - serial data output
//-------------------------------------------------

int nmc9306_device::do_r()
{
	return m_do;
}
