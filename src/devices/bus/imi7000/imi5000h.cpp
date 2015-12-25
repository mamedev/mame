// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IMI 5000H 5.25" Winchester Hard Disk Controller emulation

    Used in Corvus Systems H-Series drives (Model 6/11/20)

**********************************************************************/

#include "imi5000h.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "u70"
#define Z80CTC_TAG      "u45"
#define Z80PIO_0_TAG    "u25"
#define Z80PIO_2_TAG    "u64"
#define Z80PIO_3_TAG    "u73"



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
	{ nullptr }
};


//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

WRITE_LINE_MEMBER( imi5000h_device::ctc_z0_w )
{
	m_ctc->trg1(state);
}

WRITE_LINE_MEMBER( imi5000h_device::ctc_z1_w )
{
	m_ctc->trg2(state);
	m_ctc->trg3(state);
}

WRITE_LINE_MEMBER( imi5000h_device::ctc_z2_w )
{
	//m_memory_enable = state;
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

//-------------------------------------------------
//  Z80PIO 1
//-------------------------------------------------

READ8_MEMBER( imi5000h_device::pio0_pa_r )
{
	/*

	    bit     description

	    0       -SEEK COMPLETE
	    1       -SECTOR SIZE 2 (UB4:4)
	    2       -SECTOR SIZE 1 (UB4:1)
	    3       -SECTOR SEL
	    4       CRC ERROR
	    5       WRITE FAULT
	    6       -INDEX SEL
	    7

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio0_pa_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       ACTIVITY LED

	*/
}

READ8_MEMBER( imi5000h_device::pio0_pb_r )
{
	/*

	    bit     description

	    0       -READY
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio0_pb_w )
{
	/*

	    bit     description

	    0
	    1       DIRECTION IN
	    2       -HSXSTB
	    3       STEP
	    4       HEAD SEL 2^0
	    5       HEAD SEL 2^1
	    6       HEAD SEL 2^2
	    7       REDUCE WR CURRENT

	*/
}

//-------------------------------------------------
//  Z80PIO 2
//-------------------------------------------------

READ8_MEMBER( imi5000h_device::pio2_pa_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       -SYNC
	    7       -DRV.ACK

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio2_pa_w )
{
	/*

	    bit     description

	    0       BUS DIR
	    1       -DRV.ACK
	    2       -ALT SEL
	    3       -HSXFER
	    4       PIO RDY
	    5       -COMPL
	    6
	    7

	*/
}

READ8_MEMBER( imi5000h_device::pio2_pb_r )
{
	// command bus
	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio2_pb_w )
{
	// command bus
}

//-------------------------------------------------
//  Z80PIO 3
//-------------------------------------------------

READ8_MEMBER( imi5000h_device::pio3_pa_r )
{
	/*

	    bit     description

	    0       -TIMEOUT DISABLE (UB4:8)
	    1       -UNIT SELECT 1 (UB4:7)
	    2       -UNIT SELECT 2 (UB4:6)
	    3       SYSTEM/-DIAG (UB4:5)
	    4       -RXD
	    5
	    6       -TRACK 00
	    7

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio3_pa_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       TXD
	    6
	    7       -WRITE DISABLE

	*/
}

READ8_MEMBER( imi5000h_device::pio3_pb_r )
{
	/*

	    bit     description

	    0
	    1
	    2       6MB1
	    3       -WRITE PROTECT (W2)
	    4       -FORMAT ENABLE
	    5       6MB2
	    6       12MB1
	    7       12MB2

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio3_pb_w )
{
	/*

	    bit     description

	    0       -DRV 1 SEL
	    1       -DRV 2 SEL
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}

//-------------------------------------------------
//  MACHINE_DRIVER( imi5000h )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( imi5000h )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_CONFIG(z80_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(imi5000h_mem)
	MCFG_CPU_IO_MAP(imi5000h_io)

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_8MHz / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(imi5000h_device, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(imi5000h_device, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(imi5000h_device, ctc_z2_w))

	MCFG_DEVICE_ADD(Z80PIO_0_TAG, Z80PIO, XTAL_8MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(imi5000h_device, pio0_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(imi5000h_device, pio0_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(DEVWRITELINE(Z80PIO_0_TAG, z80pio_device, strobe_a))
	MCFG_Z80PIO_IN_PB_CB(READ8(imi5000h_device, pio0_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(imi5000h_device, pio0_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(DEVWRITELINE(Z80PIO_0_TAG, z80pio_device, strobe_b))

	MCFG_DEVICE_ADD(Z80PIO_2_TAG, Z80PIO, XTAL_8MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(imi5000h_device, pio2_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(imi5000h_device, pio2_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(DEVWRITELINE(Z80PIO_2_TAG, z80pio_device, strobe_a))
	MCFG_Z80PIO_IN_PB_CB(READ8(imi5000h_device, pio2_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(imi5000h_device, pio2_pb_w))

	MCFG_DEVICE_ADD(Z80PIO_3_TAG, Z80PIO, XTAL_8MHz/2)
	MCFG_Z80PIO_IN_PA_CB(READ8(imi5000h_device, pio3_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(imi5000h_device, pio3_pa_w))
	MCFG_Z80PIO_OUT_ARDY_CB(DEVWRITELINE(Z80PIO_3_TAG, z80pio_device, strobe_a))
	MCFG_Z80PIO_IN_PB_CB(READ8(imi5000h_device, pio3_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(imi5000h_device, pio3_pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(DEVWRITELINE(Z80PIO_3_TAG, z80pio_device, strobe_b))

	//MCFG_HARDDISK_ADD("harddisk1")
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
	PORT_START("LSI-11")
	PORT_DIPNAME( 0x01, 0x00, "LSI-11" )
	PORT_DIPSETTING(    0x01, "Normal" )
	PORT_DIPSETTING(    0x00, "LSI-11" ) // emulate DEC RL01 and RL02

	PORT_START("MUX")
	PORT_DIPNAME( 0x01, 0x00, "MUX" )
	PORT_DIPSETTING(    0x01, "Single" )
	PORT_DIPSETTING(    0x00, "Multiplexer" ) // Corvus Multiplexer Network

	PORT_START("FORMAT")
	PORT_DIPNAME( 0x01, 0x00, "FORMAT" )
	PORT_DIPSETTING(    0x01, "Normal" ) // read controller firmware from cylinders 0 and 1
	PORT_DIPSETTING(    0x00, "Format" ) // drive ready after self-test, allow format

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("RESET")

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

imi5000h_device::imi5000h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, IMI5000H, "IMI 5000H", tag, owner, clock, "imi5000h", __FILE__),
	device_imi7000_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_ctc(*this, Z80CTC_TAG),
	m_lsi11(*this, "LSI-11"),
	m_mux(*this, "MUX"),
	m_format(*this, "FORMAT"),
	m_ub4(*this, "UB4")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void imi5000h_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void imi5000h_device::device_reset()
{
	m_maincpu->reset();
	m_ctc->reset();
}
