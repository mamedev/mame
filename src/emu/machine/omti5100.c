/***************************************************************************

    SMS OMTI 5100

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    SCSI/SASI Intelligent Data Controller

    Note: - Skeleton device
          - Supports up to two ST-506/412 hard drives
          - Z8681 (Z8)
          - 8 KB RAM
          - 2 KB Buffer RAM

***************************************************************************/

#include "omti5100.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type OMTI5100 = &device_creator<omti5100_device>;

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( omti5100_firmware )
	ROM_REGION(0x2000, "firmware", 0)
	ROM_LOAD("1002401-n.7a", 0x0000, 0x2000, CRC(d531e25c) SHA1(22e4762a70841b80e843a5d76175c1fdb6838e18))
ROM_END

const rom_entry *omti5100_device::device_rom_region() const
{
	return ROM_NAME( omti5100_firmware );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( omti5100_z8 )
//  MCFG_CPU_ADD("z8", Z8681, XTAL_20MHz / 3 /* ??? */)
MACHINE_CONFIG_END

machine_config_constructor omti5100_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( omti5100_z8 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  omti5100_device - constructor
//-------------------------------------------------

omti5100_device::omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, OMTI5100, "OMTI 5100 SCSI/SASI Controller", tag, owner, clock, "omti5100", __FILE__),
//  m_cpu(*this, "z8"),
	m_bsy_w(*this),
	m_cd_w(*this),
	m_io_w(*this),
	m_req_w(*this),
	m_msg_w(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void omti5100_device::device_start()
{
	// resolve callbacks
	m_bsy_w.resolve_safe();
	m_cd_w.resolve_safe();
	m_io_w.resolve_safe();
	m_req_w.resolve_safe();
	m_msg_w.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void omti5100_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( omti5100_device::data_r )
{
	if (VERBOSE)
		logerror("%s: data_r\n", tag());

	return 0xff;
}

WRITE8_MEMBER( omti5100_device::data_w )
{
	if (VERBOSE)
		logerror("%s: rst_w: %02x\n", tag(), data);
}

READ_LINE_MEMBER( omti5100_device::parity_r )
{
	if (VERBOSE)
		logerror("%s: parity_r\n", tag());

	return 1;
}

WRITE_LINE_MEMBER( omti5100_device::parity_w )
{
	if (VERBOSE)
		logerror("%s: parity_w: %d\n", tag(), state);
}

WRITE_LINE_MEMBER( omti5100_device::rst_w )
{
	if (VERBOSE)
		logerror("%s: rst_w: %d\n", tag(), state);
}

WRITE_LINE_MEMBER( omti5100_device::sel_w )
{
	if (VERBOSE)
		logerror("%s: sel_w: %d\n", tag(), state);
}

WRITE_LINE_MEMBER( omti5100_device::ack_w )
{
	if (VERBOSE)
		logerror("%s: ack_w: %d\n", tag(), state);
}
