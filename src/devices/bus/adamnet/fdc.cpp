// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam floppy disk controller emulation

**********************************************************************/

/*

    TODO:

    - 320KB DSDD 5.25"
    - 720KB DSDD 3.5"
    - 1.44MB DSHD 3.5"

*/

#include "fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "u6"
#define WD2793_TAG      "u11"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_FDC = &device_creator<adam_fdc_device>;


//-------------------------------------------------
//  ROM( adam_fdc )
//-------------------------------------------------

ROM_START( adam_fdc )
	ROM_REGION( 0x1000, M6801_TAG, 0 )
	ROM_DEFAULT_BIOS("ssdd")
	ROM_SYSTEM_BIOS( 0, "ssdd", "Coleco 160KB SSDD" )
	ROMX_LOAD( "adam disk u10 ad 31 rev a 09-27-84.u10", 0x0000, 0x1000, CRC(4b0b7143) SHA1(1cb68891c3af80e99efad7e309136ca37244f060), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "320ta", "320KB DSDD" )
	ROMX_LOAD( "320ta.u10", 0x0000, 0x1000, CRC(dcd865b3) SHA1(dde583e0d18ce4406e9ea44ab34d083e73ee30e2), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "dbl24", "320KB DSDD" )
	ROMX_LOAD( "dbl2-4.u10", 0x0000, 0x1000, CRC(5df49f15) SHA1(43d5710e4fb05f520e813869a049585b41ada86b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "dsdd", "320KB DSDD" )
	ROMX_LOAD( "unknown.u10", 0x0000, 0x1000, CRC(2b2a9c6d) SHA1(e40304cbb6b9f174d9f5762d920983c79c899b3e), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "a720dipi", "720KB 3.5\" A720DIPI 7607 MMSG" )
	ROMX_LOAD( "a720dipi 7607 mmsg (c) 1988.u10", 0x0000, 0x1000, CRC(5f248557) SHA1(15b3aaebba38af84f6a1a6ccdf840ca3d58635da), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "fp720at", "720KB 3.5\" FastPack 720A(T)" )
	ROMX_LOAD( "fastpack 720a(t).u10", 0x0000, 0x1000, CRC(8f952c88) SHA1(e593a89d7c6e7ea99e7ce376ffa2732d7b646d49), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 6, "mihddd", "1.44MB 3.5\" Micro Innovations HD-DD" )
	ROMX_LOAD( "1440k micro innovations hd-dd.u10", 0x0000, 0x1000, CRC(2efec8c0) SHA1(f6df22339c93dca938b65d0cbe23abcad89ec230), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 7, "pmhd", "1.44MB 3.5\" Powermate High Density" )
	ROMX_LOAD( "pmhdfdc.u10", 0x0000, 0x1000, CRC(fed4006c) SHA1(bc8dd00dd5cde9500a4cd7dc1e4d74330184472a), ROM_BIOS(8) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *adam_fdc_device::device_rom_region() const
{
	return ROM_NAME( adam_fdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( fdc6801_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_fdc_mem, AS_PROGRAM, 8, adam_fdc_device )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE(M6801_TAG, m6801_cpu_device, m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_WRITEONLY AM_SHARE("ram")
	AM_RANGE(0x0800, 0x0800) AM_MIRROR(0x3ff) AM_DEVREAD(WD2793_TAG, wd2793_t, status_r)
	AM_RANGE(0x1400, 0x17ff) AM_RAM AM_READONLY AM_SHARE("ram")
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x3ff) AM_DEVWRITE(WD2793_TAG, wd2793_t, cmd_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x3ff) AM_DEVREAD(WD2793_TAG, wd2793_t, track_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x3ff) AM_DEVWRITE(WD2793_TAG, wd2793_t, track_w)
	AM_RANGE(0x4800, 0x4800) AM_MIRROR(0x3ff) AM_DEVREAD(WD2793_TAG, wd2793_t, sector_r)
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x3ff) AM_DEVWRITE(WD2793_TAG, wd2793_t, sector_w)
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x3ff) AM_DEVREAD(WD2793_TAG, wd2793_t, data_r)
	AM_RANGE(0x6c00, 0x6fff) AM_READ(data_r)
	AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x3ff) AM_DEVWRITE(WD2793_TAG, wd2793_t, data_w)
	AM_RANGE(0x8000, 0x8fff) AM_MIRROR(0x7000) AM_ROM AM_REGION(M6801_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fdc6801_io )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_fdc_io, AS_IO, 8, adam_fdc_device )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(p1_r, p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(p2_r, p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3)
	AM_RANGE(M6801_PORT4, M6801_PORT4)
ADDRESS_MAP_END


//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( adam_fdc_device::floppy_formats )
	FLOPPY_ADAM_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( adam_fdc_floppies )
	SLOT_INTERFACE( "525ssdd", FLOPPY_525_SSDD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_DRIVER( adam_fdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( adam_fdc )
	MCFG_CPU_ADD(M6801_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(adam_fdc_mem)
	MCFG_CPU_IO_MAP(adam_fdc_io)

	MCFG_WD2793_ADD(WD2793_TAG, XTAL_4MHz/4)
	MCFG_WD_FDC_INTRQ_CALLBACK(INPUTLINE(M6801_TAG, INPUT_LINE_NMI))

	MCFG_FLOPPY_DRIVE_ADD(WD2793_TAG":0", adam_fdc_floppies, "525ssdd", adam_fdc_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor adam_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( adam_fdc );
}


//-------------------------------------------------
//  INPUT_PORTS( adam_fdc )
//-------------------------------------------------

static INPUT_PORTS_START( adam_fdc )
	PORT_START("SW3")
	PORT_DIPNAME( 0x01, 0x00, "Drive Select" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, "DS1" )
	PORT_DIPSETTING(    0x01, "DS2" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor adam_fdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( adam_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_fdc_device - constructor
//-------------------------------------------------

adam_fdc_device::adam_fdc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADAM_FDC, "Adam FDC", tag, owner, clock, "adam_fdc", __FILE__),
		device_adamnet_card_interface(mconfig, *this),
		m_maincpu(*this, M6801_TAG),
		m_fdc(*this, WD2793_TAG),
		m_floppy0(*this, WD2793_TAG":0:525ssdd"),
		m_floppy(nullptr),
		m_ram(*this, "ram"),
		m_sw3(*this, "SW3")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_fdc_device::device_start()
{
}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_fdc_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);

	if (state == ASSERT_LINE) m_fdc->reset();
}


//-------------------------------------------------
//  data_r -
//-------------------------------------------------

READ8_MEMBER( adam_fdc_device::data_r )
{
	UINT8 data = m_fdc->data_r();

	m_ram[offset & 0x3ff] = data;

	return data;
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( adam_fdc_device::p1_r )
{
	/*

	    bit     description

	    0       disk in place
	    1
	    2       FDC DRQ
	    3
	    4
	    5
	    6
	    7       SW3 (0=DS1, 1=DS2)

	*/

	UINT8 data = 0x00;

	// disk in place
	data |= m_floppy0->exists() ? 0x00 : 0x01;

	// floppy data request
	data |= m_fdc->drq_r() ? 0x04 : 0x00;

	// drive select
	data |= m_sw3->read() << 7;

	return data;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_fdc_device::p1_w )
{
	/*

	    bit     description

	    0
	    1       FDC ENP
	    2
	    3       FDC _DDEN
	    4
	    5       DRIVE SELECT
	    6       MOTOR ON
	    7

	*/

	// write precompensation
	//m_fdc->enp_w(BIT(data, 1));

	// density select
	m_fdc->dden_w(BIT(data, 3));

	// drive select
	m_floppy = nullptr;

	if (BIT(data, 5))
	{
		m_floppy = m_floppy0;
	}

	m_fdc->set_floppy(m_floppy);

	// motor enable
	if (m_floppy) m_floppy->mon_w(!BIT(data, 6));
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_fdc_device::p2_r )
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4

	*/

	UINT8 data = M6801_MODE_2;

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_fdc_device::p2_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       NET TXD

	*/

	m_bus->txd_w(this, BIT(data, 4));
}
