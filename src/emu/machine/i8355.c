/**********************************************************************

    Intel 8355 - 16,384-Bit ROM with I/O emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "i8355.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

enum
{
	REGISTER_PORT_A = 0,
	REGISTER_PORT_B,
	REGISTER_PORT_A_DDR,
	REGISTER_PORT_B_DDR
};

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_COUNT
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type I8355 = i8355_device_config::static_alloc_device_config;


// default address map
static ADDRESS_MAP_START( i8355, AS_0, 8 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  i8355_device_config - constructor
//-------------------------------------------------

i8355_device_config::i8355_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Intel 8355", tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("ram", ENDIANNESS_LITTLE, 8, 11, 0, NULL, *ADDRESS_MAP_NAME(i8355))
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *i8355_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(i8355_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *i8355_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(machine, i8355_device(machine, *this));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *i8355_device_config::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8355_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const i8355_interface *intf = reinterpret_cast<const i8355_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8355_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&in_pa_func, 0, sizeof(in_pa_func));
		memset(&out_pa_func, 0, sizeof(out_pa_func));
		memset(&in_pb_func, 0, sizeof(in_pb_func));
		memset(&out_pb_func, 0, sizeof(out_pb_func));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_port - read from input port
//-------------------------------------------------

inline UINT8 i8355_device::read_port(int port)
{
	UINT8 data = m_output[port] & m_ddr[port];

	if (m_ddr[port] != 0xff)
	{
		data |= devcb_call_read8(&m_in_port_func[port], 0) & ~m_ddr[port];
	}

	return data;
}


//-------------------------------------------------
//  write_port - write to output port
//-------------------------------------------------

inline void i8355_device::write_port(int port, UINT8 data)
{
	m_output[port] = data;

	devcb_call_write8(&m_out_port_func[port], 0, m_output[port] & m_ddr[port]);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8355_device - constructor
//-------------------------------------------------

i8355_device::i8355_device(running_machine &_machine, const i8355_device_config &config)
    : device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
      m_config(config)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8355_device::device_start()
{
	// resolve callbacks
	devcb_resolve_read8(&m_in_port_func[0], &m_config.in_pa_func, this);
	devcb_resolve_read8(&m_in_port_func[1], &m_config.in_pb_func, this);
	devcb_resolve_write8(&m_out_port_func[0], &m_config.out_pa_func, this);
	devcb_resolve_write8(&m_out_port_func[1], &m_config.out_pb_func, this);

	// register for state saving
	save_item(NAME(m_output));
	save_item(NAME(m_ddr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8355_device::device_reset()
{
	// set ports to input mode
	m_ddr[PORT_A] = 0;
	m_ddr[PORT_B] = 0;
}


//-------------------------------------------------
//  io_r - register read
//-------------------------------------------------

READ8_MEMBER( i8355_device::io_r )
{
	int port = offset & 0x01;

	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_PORT_A:
	case REGISTER_PORT_B:
		data = read_port(port);
		break;

	case REGISTER_PORT_A_DDR:
	case REGISTER_PORT_B_DDR:
		// write only
		break;
	}

	return data;
}


//-------------------------------------------------
//  io_w - register write
//-------------------------------------------------

WRITE8_MEMBER( i8355_device::io_w )
{
	int port = offset & 0x01;

	switch (offset & 0x03)
	{
	case REGISTER_PORT_A:
	case REGISTER_PORT_B:
		if (LOG) logerror("I8355 '%s' Port %c Write %02x\n", tag(), 'A' + port, data);

		write_port(port, data);
		break;

	case REGISTER_PORT_A_DDR:
	case REGISTER_PORT_B_DDR:
		if (LOG) logerror("I8355 '%s' Port %c DDR: %02x\n", tag(), 'A' + port, data);

		m_ddr[port] = data;
		write_port(port, data);
		break;
	}
}


//-------------------------------------------------
//  memory_r - internal ROM read
//-------------------------------------------------

READ8_MEMBER( i8355_device::memory_r )
{
	return this->space()->read_byte(offset);
}
