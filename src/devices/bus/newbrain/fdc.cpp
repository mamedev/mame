// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Interface Module emulation

**********************************************************************/

/*

    TODO:

    - map d413 ROM to computer space
    - paging

*/

#include "fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define Z80_TAG     "416"
#define UPD765_TAG  "418"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NEWBRAIN_FDC = &device_creator<newbrain_fdc_t>;


//-------------------------------------------------
//  ROM( newbrain_fdc )
//-------------------------------------------------

ROM_START( newbrain_fdc )
	ROM_REGION( 0x2000, "d413", 0 )
	ROM_LOAD( "d413-2.rom", 0x0000, 0x2000, CRC(097591f1) SHA1(c2aa1d27d4f3a24ab0c8135df746a4a44201a7f4) )

	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("issue2")
	ROM_SYSTEM_BIOS( 0, "issue1", "Issue 1" )
	ROMX_LOAD( "d417-1.rom", 0x0000, 0x2000, CRC(40fad31c) SHA1(5137be4cc026972c0ffd4fa6990e8583bdfce163), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "issue2", "Issue 2" )
	ROMX_LOAD( "d417-2.rom", 0x0000, 0x2000, CRC(e8bda8b9) SHA1(c85a76a5ff7054f4ef4a472ce99ebaed1abd269c), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *newbrain_fdc_t::device_rom_region() const
{
	return ROM_NAME( newbrain_fdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( newbrain_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( newbrain_fdc_mem, AS_PROGRAM, 8, newbrain_fdc_t )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( newbrain_fdc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( newbrain_fdc_io, AS_IO, 8, newbrain_fdc_t )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xd1)
	AM_RANGE(0x00, 0x01) AM_MIRROR(0x1e) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x1f) AM_WRITE(fdc_auxiliary_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x1f) AM_READ(fdc_control_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  newbrain_floppies
//-------------------------------------------------

static SLOT_INTERFACE_START( newbrain_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( newbrain_fdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( newbrain_fdc )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(newbrain_fdc_mem)
	MCFG_CPU_IO_MAP(newbrain_fdc_io)

	MCFG_UPD765A_ADD(UPD765_TAG, false, true)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(newbrain_fdc_t, fdc_int_w))

	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", newbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", newbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":2", newbrain_floppies, nullptr, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":3", newbrain_floppies, nullptr, floppy_image_device::default_floppy_formats)

	MCFG_NEWBRAIN_EXPANSION_SLOT_ADD(NEWBRAIN_EXPANSION_SLOT_TAG, XTAL_16MHz/8, newbrain_expansion_cards, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor newbrain_fdc_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( newbrain_fdc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  newbrain_fdc_t - constructor
//-------------------------------------------------

newbrain_fdc_t::newbrain_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NEWBRAIN_FDC, "NewBrain FDC", tag, owner, clock, "newbrain_fdc", __FILE__),
	device_newbrain_expansion_slot_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_fdc(*this, UPD765_TAG),
	m_floppy0(*this, UPD765_TAG ":0"),
	m_floppy1(*this, UPD765_TAG ":1"),
	m_floppy2(*this, UPD765_TAG ":2"),
	m_floppy3(*this, UPD765_TAG ":3"),
	m_exp(*this, NEWBRAIN_EXPANSION_SLOT_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newbrain_fdc_t::device_start()
{
	save_item(NAME(m_paging));
	save_item(NAME(m_ma16));
	save_item(NAME(m_mpm));
	save_item(NAME(m_fdc_att));
	save_item(NAME(m_fdc_int));
	save_item(NAME(m_pa15));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newbrain_fdc_t::device_reset()
{
	m_maincpu->reset();

	moton(0);
	m_fdc->tc_w(0);
	m_pa15 = 0;
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

UINT8 newbrain_fdc_t::mreq_r(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh)
{
	return m_exp->mreq_r(space, offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

void newbrain_fdc_t::mreq_w(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh)
{
	m_exp->mreq_w(space, offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

UINT8 newbrain_fdc_t::iorq_r(address_space &space, offs_t offset, UINT8 data, bool &prtov)
{
	return m_exp->iorq_r(space, offset, data, prtov);
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

void newbrain_fdc_t::iorq_w(address_space &space, offs_t offset, UINT8 data, bool &prtov)
{
	m_exp->iorq_w(space, offset, data, prtov);

	if ((offset & 0x20f) == 0x20f)
	{
		io_dec_w(space, 0, data);
	}
}


//-------------------------------------------------
//  moton - floppy motor on
//-------------------------------------------------

void newbrain_fdc_t::moton(int state)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
	if (m_floppy2->get_device()) m_floppy2->get_device()->mon_w(!state);
	if (m_floppy3->get_device()) m_floppy3->get_device()->mon_w(!state);
}


//-------------------------------------------------
//  fdc_int_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_fdc_t::fdc_int_w )
{
	m_fdc_int = state;
}


//-------------------------------------------------
//  fdc_auxiliary_w -
//-------------------------------------------------

WRITE8_MEMBER( newbrain_fdc_t::fdc_auxiliary_w )
{
	/*

	    bit     description

	    0       MOTON
	    1       765 RESET
	    2       TC
	    3
	    4
	    5       PA15
	    6
	    7

	*/

	moton(BIT(data, 0));

	if (BIT(data, 1))
	{
		m_fdc->reset();
	}

	m_fdc->tc_w(BIT(data, 2));

	m_pa15 = BIT(data, 5);
}


//-------------------------------------------------
//  fdc_control_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_fdc_t::fdc_control_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       FDC INT
	    6       PAGING
	    7       FDC ATT

	*/

	return (m_fdc_att << 7) | (m_paging << 6) | (m_fdc_int << 5);
}


//-------------------------------------------------
//  io_dec_w - 0x20f
//-------------------------------------------------

WRITE8_MEMBER( newbrain_fdc_t::io_dec_w )
{
	/*

	    bit     description

	    0       PAGING
	    1
	    2       MA16
	    3       MPM
	    4
	    5       _FDC RESET
	    6
	    7       FDC ATT

	*/

	m_paging = BIT(data, 0);
	m_ma16 = BIT(data, 2);
	m_mpm = BIT(data, 3);

	if (!BIT(data, 5))
	{
		device_reset();
	}

	m_fdc_att = BIT(data, 7);
}
