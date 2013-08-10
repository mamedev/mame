/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#include "i8089.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  I/O CHANNEL
//**************************************************************************

const device_type I8089_CHANNEL = &device_creator<i8089_channel>;

class i8089_channel : public device_t,
                      public device_execute_interface
{
public:
	// construction/destruction
	i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void execute_run();

	int m_icount;
};

i8089_channel::i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8089_CHANNEL, "Intel 8089 I/O Channel", tag, owner, clock, "i8089_channel", __FILE__),
	device_execute_interface(mconfig, *this),
	m_icount(0)
{
}

void i8089_channel::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;
}

void i8089_channel::execute_run()
{
	do
	{
		m_icount--;
	}
	while (m_icount > 0);
}


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I8089 = &device_creator<i8089_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8089_device - constructor
//-------------------------------------------------

i8089_device::i8089_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8089, "Intel 8089", tag, owner, clock, "i8089", __FILE__),
	m_ch1(*this, "1"),
	m_ch2(*this, "2"),
	m_write_sintr1(*this),
	m_write_sintr2(*this),
	m_16bit_system(false),
	m_16bit_remote(false),
	m_master(false),
	m_request_grant(false),
	m_control_block(0),
	m_ca(0),
	m_sel(0)
{
}

void i8089_device::static_set_cputag(device_t &device, const char *tag)
{
	i8089_device &i8089 = downcast<i8089_device &>(device);
	i8089.m_cputag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8089_device::device_start()
{
	// make sure our channels have been setup first
	if (!m_ch1->started() || !m_ch2->started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_write_sintr1.resolve_safe();
	m_write_sintr2.resolve_safe();

	// get pointers to memory
	device_t *cpu = machine().device(m_cputag);
	m_mem = &cpu->memory().space(AS_PROGRAM);
	m_io = &cpu->memory().space(AS_IO);

	// initialize channel clock
	m_ch1->set_unscaled_clock(clock());
	m_ch2->set_unscaled_clock(clock());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8089_device::device_reset()
{
	m_initialized = false;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( i8089 )
	MCFG_DEVICE_ADD("1", I8089_CHANNEL, 0)
	MCFG_DEVICE_ADD("2", I8089_CHANNEL, 0)
MACHINE_CONFIG_END

machine_config_constructor i8089_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( i8089 );
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void i8089_device::initialize()
{
	assert(!m_initialized);

	// get system bus width
	UINT8 sys_bus = m_mem->read_byte(0xffff6);
	m_16bit_system = BIT(sys_bus, 0);

	// get system configuration block address
	UINT16 scb_offset = read_word(0xffff8);
	UINT16 scb_segment = read_word(0xffffa);
	offs_t scb_address = (scb_segment << 4) + scb_offset;

	// get system operation command
	UINT16 soc = read_word(scb_address);
	m_16bit_remote = BIT(soc, 0);
	m_request_grant = BIT(soc, 1);
	m_master = !m_sel;

	UINT16 cb_offset = read_word(scb_address + 2);
	UINT16 cb_segment = read_word(scb_address + 4);
	m_control_block = (cb_segment << 4) + cb_offset;

	// todo: set/clear busy
	m_initialized = true;

	// output some debug info
	if (VERBOSE)
	{
		logerror("%s('%s'): initializing\n", shortname(), basetag());
		logerror("%s('%s'): %s system bus\n", shortname(), basetag(), m_16bit_system ? "16-bit" : "8-bit");
		logerror("%s('%s'): system configuration block location: %06x\n", shortname(), basetag(), scb_address);
		logerror("%s('%s'): %s remote bus\n", shortname(), basetag(), m_16bit_remote ? "16-bit" : "8-bit");
		logerror("%s('%s'): request/grant: %d\n", shortname(), basetag(), m_request_grant);
		logerror("%s('%s'): is %s\n", shortname(), basetag(), m_master ? "master" : "slave");
		logerror("%s('%s'): channel control block location: %06x\n", shortname(), basetag(), m_control_block);
	}
}

UINT16 i8089_device::read_word(offs_t address)
{
	assert(m_initialized);

	UINT16 data = 0xffff;

	if (m_16bit_system)
	{
		data = m_mem->read_word(address);
	}
	else
	{
		data  = m_mem->read_byte(address);
		data |= m_mem->read_byte(address + 1) << 8;
	}

	return data;
}


//**************************************************************************
//  EXTERNAL INPUTS
//**************************************************************************

WRITE_LINE_MEMBER( i8089_device::ca_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ca_w: %u\n", shortname(), basetag(), state);

	if (m_ca == 1 && state == 0)
	{
		if (!m_initialized)
			initialize();
	}

	m_ca = state;
}

WRITE_LINE_MEMBER( i8089_device::sel_w )
{
	if (VERBOSE)
		logerror("%s('%s'): sel_w: %u\n", shortname(), basetag(), state);

	m_sel = state;
}

WRITE_LINE_MEMBER( i8089_device::drq1_w )
{
	if (VERBOSE)
		logerror("%s('%s'): drq1_w: %u\n", shortname(), basetag(), state);
}

WRITE_LINE_MEMBER( i8089_device::drq2_w )
{
	if (VERBOSE)
		logerror("%s('%s'): drq2_w: %u\n", shortname(), basetag(), state);
}

WRITE_LINE_MEMBER( i8089_device::ext1_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ext1_w: %u\n", shortname(), basetag(), state);
}

WRITE_LINE_MEMBER( i8089_device::ext2_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ext2_w: %u\n", shortname(), basetag(), state);
}
