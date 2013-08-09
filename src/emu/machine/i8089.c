/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#include "i8089.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define VERBOSE 1


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
	m_write_sintr1(*this),
	m_write_sintr2(*this)
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
	// resolve callbacks
	m_write_sintr1.resolve_safe();
	m_write_sintr2.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8089_device::device_reset()
{
}


//**************************************************************************
//  EXTERNAL INPUTS
//**************************************************************************

WRITE_LINE_MEMBER( i8089_device::ca_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ca_w: %u\n", shortname(), basetag(), state);
}

WRITE_LINE_MEMBER( i8089_device::sel_w )
{
	if (VERBOSE)
		logerror("%s('%s'): sel_w: %u\n", shortname(), basetag(), state);
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
