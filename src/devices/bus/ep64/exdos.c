// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Software EXDOS Disk Controller Module emulation

**********************************************************************/

/*

Floppy Drive Controller PCB Layout
----------------------------------

INTELLIGENT SOFTWARE LTD DISK CONTROLLER
ISS1
 |--------------------------------------------|
 |                                            |
 |                                            |
|-|  7438  74LS273             WD1770         |
|I|                                           |
|D|                                           |
|C|                                  EPROM.IC2|
|3|           74LS32  74LS02  74LS266         |
|4|  7438                                     |
|-|      74LS126  74LS10  74LS245  74LS266    |
 |                                            |
 |                                            |
 |----------------------------|||||||||||||||||
                              |---------------|
Notes: (All IC's shown)

This PCB plugs into the external expansion connector on the right side of the mainboard

      EPROM.IC2 - 16k x8-bit EPROM labelled 'EXDOS V1.0 P/N 08-60' (DIP28)
         WD1770 - Western Digital WD1770 Floppy Drive Controller (DIP28)
         74LS02 - Quad 2-Input NOR Gate (DIP14)
         74LS10 - Triple 3-input NAND Gate (DIP14)
         74LS32 - Quad 2-Input Positive OR Gate (DIP14)
           7438 - Quad 2-input NAND Buffer (DIP14)
        74LS126 - Quad Bus Buffer (DIP14)
        74LS245 - Octal Bus Tranceiver with Tri-State Outputs (DIP20)
        74LS266 - Quad EXCLUSIVE-NOR Gate (DIP14)
        74LS273 - Octal D-Type Flip-Flop With Clear (DIP20)
          IDC34 - IDC 34-way flat cable connector for floppy drive data cable

*/

#include "exdos.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1770_TAG  "u1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EP64_EXDOS = &device_creator<ep64_exdos_device>;


//-------------------------------------------------
//  ROM( ep64_exdos )
//-------------------------------------------------

ROM_START( ep64_exdos )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "exdos13.rom", 0x0000, 0x8000, CRC(d1d7e157) SHA1(31c8be089526aa8aa019c380cdf51ddd3ee76454) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ep64_exdos_device::device_rom_region() const
{
	return ROM_NAME( ep64_exdos );
}


//-------------------------------------------------
//  SLOT_INTERFACE( ep64_exdos_floppies )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( ep64_exdos_device::floppy_formats )
	FLOPPY_EP64_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( ep64_exdos_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( ep64_exdos )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ep64_exdos )
	MCFG_WD1770_ADD(WD1770_TAG, XTAL_8MHz)

	MCFG_FLOPPY_DRIVE_ADD(WD1770_TAG":0", ep64_exdos_floppies, "35dd", ep64_exdos_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1770_TAG":1", ep64_exdos_floppies, NULL,  ep64_exdos_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1770_TAG":2", ep64_exdos_floppies, NULL,  ep64_exdos_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1770_TAG":3", ep64_exdos_floppies, NULL,  ep64_exdos_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ep64_exdos_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ep64_exdos );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ep64_exdos_device - constructor
//-------------------------------------------------

ep64_exdos_device::ep64_exdos_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EP64_EXDOS, "EXDOS", tag, owner, clock, "ep64_exdos", __FILE__),
	device_ep64_expansion_bus_card_interface(mconfig, *this),
	m_fdc(*this, WD1770_TAG),
	m_floppy0(*this, WD1770_TAG":0"),
	m_floppy1(*this, WD1770_TAG":1"),
	m_floppy2(*this, WD1770_TAG":2"),
	m_floppy3(*this, WD1770_TAG":3"),
	m_floppy(NULL),
	m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ep64_exdos_device::device_start()
{
	m_slot->program().install_rom(0x080000, 0x087fff, 0, 0, m_rom->base());

	m_slot->io().install_readwrite_handler(0x10, 0x13, 0, 0x04, READ8_DEVICE_DELEGATE(m_fdc, wd_fdc_t, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd_fdc_t, write));
	m_slot->io().install_readwrite_handler(0x18, 0x18, 0, 0x04, READ8_DELEGATE(ep64_exdos_device, read), WRITE8_DELEGATE(ep64_exdos_device, write));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ep64_exdos_device::device_reset()
{
	m_fdc->reset();

	m_floppy = NULL;
	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(0);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( ep64_exdos_device::read )
{
	/*

	    bit     description

	    0
	    1       INTRQ
	    2
	    3
	    4
	    5
	    6       DCHG
	    7       DRQ

	*/

	UINT8 data = 0;

	data |= m_fdc->intrq_r() << 1;
	data |= m_fdc->drq_r() << 7;

	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 6;

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( ep64_exdos_device::write )
{
	/*

	    bit     description

	    0       SELECT 0
	    1       SELECT 1
	    2       SELECT 2
	    3       SELECT 3
	    4       SIDE 1
	    5       _DDEN
	    6       DISK CHANGE RESET
	    7       IN USE

	*/

	m_floppy = NULL;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->ss_w(BIT(data, 4));
	}

	m_fdc->dden_w(BIT(data, 5));
}
