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

#include "emu.h"
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

DEFINE_DEVICE_TYPE(NEWBRAIN_FDC, newbrain_fdc_device, "newbrain_fdc", "NewBrain FDC")


//-------------------------------------------------
//  ROM( newbrain_fdc )
//-------------------------------------------------

ROM_START( newbrain_fdc )
	ROM_REGION( 0x2000, "d413", 0 )
	ROM_LOAD( "d413-2.rom", 0x0000, 0x2000, CRC(097591f1) SHA1(c2aa1d27d4f3a24ab0c8135df746a4a44201a7f4) )

	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("issue2")
	ROM_SYSTEM_BIOS( 0, "issue1", "Issue 1" )
	ROMX_LOAD( "d417-1.rom", 0x0000, 0x2000, CRC(40fad31c) SHA1(5137be4cc026972c0ffd4fa6990e8583bdfce163), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "issue2", "Issue 2" )
	ROMX_LOAD( "d417-2.rom", 0x0000, 0x2000, CRC(e8bda8b9) SHA1(c85a76a5ff7054f4ef4a472ce99ebaed1abd269c), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *newbrain_fdc_device::device_rom_region() const
{
	return ROM_NAME( newbrain_fdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( newbrain_fdc_mem )
//-------------------------------------------------

void newbrain_fdc_device::newbrain_fdc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}


//-------------------------------------------------
//  ADDRESS_MAP( newbrain_fdc_io )
//-------------------------------------------------

void newbrain_fdc_device::newbrain_fdc_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x71);
	map(0x00, 0x01).mirror(0x10).m(m_fdc, FUNC(upd765a_device::map));
	map(0x20, 0x20).mirror(0x11).w(FUNC(newbrain_fdc_device::fdc_auxiliary_w));
	map(0x40, 0x40).mirror(0x11).r(FUNC(newbrain_fdc_device::fdc_control_r));
}


//-------------------------------------------------
//  newbrain_floppies
//-------------------------------------------------

static void newbrain_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void newbrain_fdc_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &newbrain_fdc_device::newbrain_fdc_mem);
	m_maincpu->set_addrmap(AS_IO, &newbrain_fdc_device::newbrain_fdc_io);

	UPD765A(config, m_fdc, 8'000'000, false, true);
	m_fdc->intrq_wr_callback().set(FUNC(newbrain_fdc_device::fdc_int_w));

	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", newbrain_floppies, "525dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", newbrain_floppies, "525dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":2", newbrain_floppies, nullptr, floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":3", newbrain_floppies, nullptr, floppy_image_device::default_floppy_formats);

	NEWBRAIN_EXPANSION_SLOT(config, m_exp, XTAL(16'000'000)/8, newbrain_expansion_cards, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  newbrain_fdc_device - constructor
//-------------------------------------------------

newbrain_fdc_device::newbrain_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEWBRAIN_FDC, tag, owner, clock),
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

void newbrain_fdc_device::device_start()
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

void newbrain_fdc_device::device_reset()
{
	m_maincpu->reset();

	moton(0);
	m_fdc->tc_w(0);
	m_pa15 = 0;
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

uint8_t newbrain_fdc_device::mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	return m_exp->mreq_r(offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

void newbrain_fdc_device::mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	m_exp->mreq_w(offset, data, romov, exrm, raminh);
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

uint8_t newbrain_fdc_device::iorq_r(offs_t offset, uint8_t data, bool &prtov)
{
	return m_exp->iorq_r(offset, data, prtov);
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

void newbrain_fdc_device::iorq_w(offs_t offset, uint8_t data, bool &prtov)
{
	m_exp->iorq_w(offset, data, prtov);

	if ((offset & 0x20f) == 0x20f)
	{
		io_dec_w(data);
	}
}


//-------------------------------------------------
//  moton - floppy motor on
//-------------------------------------------------

void newbrain_fdc_device::moton(int state)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
	if (m_floppy2->get_device()) m_floppy2->get_device()->mon_w(!state);
	if (m_floppy3->get_device()) m_floppy3->get_device()->mon_w(!state);
}


//-------------------------------------------------
//  fdc_int_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_fdc_device::fdc_int_w )
{
	m_fdc_int = state;
}


//-------------------------------------------------
//  fdc_auxiliary_w -
//-------------------------------------------------

void newbrain_fdc_device::fdc_auxiliary_w(uint8_t data)
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

uint8_t newbrain_fdc_device::fdc_control_r()
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

void newbrain_fdc_device::io_dec_w(uint8_t data)
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
