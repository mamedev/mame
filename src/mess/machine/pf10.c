/**********************************************************************

    EPSON PF-10

    Battery operated portable 3.5" floppy drive

    Status: Skeleton driver, not doing much.

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "pf10.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EPSON_PF10 = &device_creator<epson_pf10_device>;


//-------------------------------------------------
//  address maps
//-------------------------------------------------

static ADDRESS_MAP_START( cpu_mem, AS_PROGRAM, 8, epson_pf10_device )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE_LEGACY(m6801_io_r, m6801_io_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM /* 192 bytes internal ram */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* external 2k ram */
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_io, AS_IO, 8, epson_pf10_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( pf10 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("k3pf1.bin", 0x0000, 0x2000, CRC(eef4593a) SHA1(bb176e4baf938fe58c2d32f7c46d7bb7b0627755))
ROM_END

const rom_entry *epson_pf10_device::device_rom_region() const
{
	return ROM_NAME( pf10 );
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static const floppy_format_type pf10_floppy_formats[] =
{
	FLOPPY_D88_FORMAT,
	FLOPPY_MFM_FORMAT,
	FLOPPY_MFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( pf10_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( pf10 )
	MCFG_CPU_ADD("maincpu", HD6303Y, XTAL_2_4576MHz / 4 /* ??? */) // HD63A03XF
	MCFG_CPU_PROGRAM_MAP(cpu_mem)
	MCFG_CPU_IO_MAP(cpu_io)

	MCFG_UPD765A_ADD("upd765a", false, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765a:0", pf10_floppies, "35dd", 0, pf10_floppy_formats) // SMD-165

	MCFG_EPSON_SIO_ADD("sio")
MACHINE_CONFIG_END

machine_config_constructor epson_pf10_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pf10 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_pf10_device - constructor
//-------------------------------------------------

epson_pf10_device::epson_pf10_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EPSON_PF10, "EPSON PF-10 floppy drive", tag, owner, clock),
	device_epson_sio_interface(mconfig, *this),
	m_cpu(*this, "maincpu"),
	m_fdc(*this, "upd765a"),
	m_sio(*this, "sio")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_pf10_device::device_start()
{
	m_floppy = subdevice<floppy_connector>("upd765a:0")->get_device();
}


//-------------------------------------------------
//  tx_w
//-------------------------------------------------

void epson_pf10_device::tx_w(int level)
{
	logerror("%s: tx_w(%d)\n", tag(), level);
}


//-------------------------------------------------
//  pout_w
//-------------------------------------------------

void epson_pf10_device::pout_w(int level)
{
	logerror("%s: pout_w(%d)\n", tag(), level);
}


//-------------------------------------------------
//  rx_r
//-------------------------------------------------

int epson_pf10_device::rx_r()
{
	logerror("%s: rx_r\n", tag());

	return 1;
}


//-------------------------------------------------
//  pin_r
//-------------------------------------------------

int epson_pf10_device::pin_r()
{
	logerror("%s: pin_r\n", tag());

	return 1;
}
