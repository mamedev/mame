// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1802.c

    Hitachi MP-1802 floppy disk controller card for the MB-6890
    Hitachi MP-3550 floppy drive is attached

*********************************************************************/

#include "emu.h"
#include "bml3mp1802.h"

#include "softlist_dev.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BML3BUS_MP1802, bml3bus_mp1802_device, "bml3mp1802", "Hitachi MP-1802 Floppy Controller Card")

static void mp1802_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_525_DD);
}

WRITE_LINE_MEMBER( bml3bus_mp1802_device::bml3_wd17xx_intrq_w )
{
	if (state)
	{
		raise_slot_nmi();
		lower_slot_nmi();
	}
}

#define MP1802_ROM_REGION  "mp1802_rom"

ROM_START( mp1802 )
	ROM_REGION(0x10000, MP1802_ROM_REGION, 0)
	// MP-1802 disk controller ROM, which replaces part of the system ROM
	ROM_LOAD( "mp1802.rom", 0xf800, 0x800, BAD_DUMP CRC(8d0dc101) SHA1(92f7d1cebecafa7472e45c4999520de5c01c6dbc))
ROM_END


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bml3bus_mp1802_device::device_add_mconfig(machine_config &config)
{
	constexpr auto CLK16M = 32.256_MHz_XTAL / 2;

	MB8866(config, m_fdc, CLK16M / 16); // 16MCLK divided by IC628 (HD74LS93P)
	m_fdc->intrq_wr_callback().set(FUNC(bml3bus_mp1802_device::bml3_wd17xx_intrq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, mp1802_floppies, "dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, mp1802_floppies, "dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy2, mp1802_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy3, mp1802_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("bml3_flop").set_filter("5");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bml3bus_mp1802_device::device_rom_region() const
{
	return ROM_NAME( mp1802 );
}

uint8_t bml3bus_mp1802_device::bml3_mp1802_r()
{
	return m_fdc->drq_r() ? 0x00 : 0x80;
}

void bml3bus_mp1802_device::bml3_mp1802_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (data & 0x03)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	case 2: floppy = m_floppy2->get_device(); break;
	case 3: floppy = m_floppy3->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(!BIT(data, 3));
		floppy->ss_w(BIT(data, 4));
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_mp1802_device::bml3bus_mp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BML3BUS_MP1802, tag, owner, clock),
	device_bml3bus_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_floppy2(*this, "fdc:2"),
	m_floppy3(*this, "fdc:3"), m_rom(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_mp1802_device::device_start()
{
	m_rom = memregion(MP1802_ROM_REGION)->base();

	// install into memory
	address_space &space_prg = space();
	space_prg.install_readwrite_handler(0xff00, 0xff03, read8sm_delegate(*m_fdc, FUNC(mb8866_device::read)), write8sm_delegate(*m_fdc, FUNC(mb8866_device::write)));
	space_prg.install_readwrite_handler(0xff04, 0xff04, read8smo_delegate(*this, FUNC(bml3bus_mp1802_device::bml3_mp1802_r)), write8smo_delegate(*this, FUNC(bml3bus_mp1802_device::bml3_mp1802_w)));
	// overwriting the main ROM (rather than using e.g. install_rom) should mean that bank switches for RAM expansion still work...
	uint8_t *mainrom = device().machine().root_device().memregion("maincpu")->base();
	memcpy(mainrom + 0xf800, m_rom + 0xf800, 0x800);
}

void bml3bus_mp1802_device::device_reset()
{
}
