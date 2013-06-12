/**********************************************************************

    IMI 5000H 5.25" Winchester Hard Disk Controller emulation

    Used in Corvus Systems H-Series drives (Model 6/11/20)

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "imi5000h.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG			"u70"
#define Z80CTC_TAG		"u45"
#define Z80PIO_0_TAG	"u25"
#define Z80PIO_2_TAG	"u64"
#define Z80PIO_3_TAG	"u73"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IMI5000H = &device_creator<imi5000h_device>;


//-------------------------------------------------
//  ROM( imi5000h )
//-------------------------------------------------

ROM_START( imi5000h )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "c 7.63.u62", 0x0000, 0x1000, CRC(822aac68) SHA1(ab3ad7726ab20dd1041cb754d266e2f191fa3ec3) )

	ROM_REGION( 0x320, "proms", 0 )
	ROM_LOAD( "8152323-2a.u52", 0x000, 0x100, CRC(b36bc7e1) SHA1(de00b5bc17ff86b66af3e974dfd9b53245de12bd) )
	ROM_LOAD( "8152323-4a.u53", 0x100, 0x100, CRC(016fe2f7) SHA1(909f815a61e759fdf998674ee383512ecd8fee65) )
	ROM_LOAD( "8152323-1a.u54", 0x200, 0x100, CRC(512f1f39) SHA1(50c68289a19fdfca3665dbb0e98373608458c5d8) )
	ROM_LOAD( "8152323-3a.u71", 0x300, 0x020, CRC(b1092f02) SHA1(646c5a3e951535a80d24d9ce8764a3f373c508db) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *imi5000h_device::device_rom_region() const
{
	return ROM_NAME( imi5000h );
}


//-------------------------------------------------
//  ADDRESS_MAP( imi5000h_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( imi5000h_mem, AS_PROGRAM, 8, imi5000h_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x1000) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x4000, 0x47ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x6000, 0x63ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_MIRROR(0x1c00) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( imi5000h_io )
//-------------------------------------------------

static ADDRESS_MAP_START( imi5000h_io, AS_IO, 8, imi5000h_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x9f)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(Z80PIO_0_TAG, z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE(Z80PIO_2_TAG, z80pio_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE(Z80PIO_3_TAG, z80pio_device, read, write)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0x03) // BEGRDY
	AM_RANGE(0x14, 0x14) AM_MIRROR(0x03) // HSXCLR
	AM_RANGE(0x18, 0x18) AM_MIRROR(0x03) // XFERSTB
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  z80_daisy_config z80_daisy_chain
//-------------------------------------------------

static const z80_daisy_config z80_daisy_chain[] =
{
	{ Z80PIO_0_TAG },
	{ Z80CTC_TAG },
	{ Z80PIO_2_TAG },
	{ NULL }
};


//-------------------------------------------------
//  Z80CTC_INTERFACE( ctc_intf )
//-------------------------------------------------

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio0_intf )
//-------------------------------------------------

static Z80PIO_INTERFACE( pio0_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio2_intf )
//-------------------------------------------------

static Z80PIO_INTERFACE( pio2_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio3_intf )
//-------------------------------------------------

static Z80PIO_INTERFACE( pio3_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_DRIVER( imi5000h )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( imi5000h )
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)
	MCFG_CPU_CONFIG(z80_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(imi5000h_mem)
	MCFG_CPU_IO_MAP(imi5000h_io)

	MCFG_Z80CTC_ADD(Z80CTC_TAG, 4000000, ctc_intf)
	MCFG_Z80PIO_ADD(Z80PIO_0_TAG, 4000000, pio0_intf)
	MCFG_Z80PIO_ADD(Z80PIO_2_TAG, 4000000, pio2_intf)
	MCFG_Z80PIO_ADD(Z80PIO_3_TAG, 4000000, pio3_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor imi5000h_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( imi5000h );
}


//-------------------------------------------------
//  INPUT_PORTS( imi5000h )
//-------------------------------------------------

static INPUT_PORTS_START( imi5000h )
	PORT_START("UB4")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "UB4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "UB4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "UB4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "UB4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "UB4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "UB4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "UB4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "UB4:8" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor imi5000h_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( imi5000h );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  imi5000h_device - constructor
//-------------------------------------------------

imi5000h_device::imi5000h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IMI5000H, "IMI 5000H", tag, owner, clock, "imi5000h", __FILE__),
		m_maincpu(*this, Z80_TAG),
		m_ub4(*this, "UB4")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void imi5000h_device::device_start()
{
}
