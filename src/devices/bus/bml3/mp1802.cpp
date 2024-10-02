// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1802.c

    Hitachi MP-1802 floppy disk controller card for the MB-6890
    Hitachi MP-3550 floppy drive is attached

*********************************************************************/

#include "emu.h"
#include "mp1802.h"

#include "softlist_dev.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BML3BUS_MP1802, bml3bus_mp1802_device, "bml3mp1802", "Hitachi MP-1802 5.25\" Floppy Controller Card")

static void mp1802_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_525_DD);
}

void bml3bus_mp1802_device::nmi_w(int state)
{
	if (state)
		raise_slot_nmi();
	else
		lower_slot_nmi();
}

ROM_START( mp1802 )
	ROM_REGION(0x800, "mp1802_rom", 0)
	// MP-1802 disk controller ROM, which replaces part of the system ROM
	ROM_LOAD( "mp1802.rom", 0x000, 0x800, BAD_DUMP CRC(8d0dc101) SHA1(92f7d1cebecafa7472e45c4999520de5c01c6dbc))
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
	m_fdc->intrq_wr_callback().set(m_nmigate, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_nmigate);
	m_nmigate->output_handler().set(FUNC(bml3bus_mp1802_device::nmi_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], mp1802_floppies, "dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], mp1802_floppies, "dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], mp1802_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], mp1802_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("bml3_flop").set_filter("5");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bml3bus_mp1802_device::device_rom_region() const
{
	return ROM_NAME( mp1802 );
}

uint8_t bml3bus_mp1802_device::read()
{
	return (m_fdc->drq_r() ? 0x00 : 0x80) | (m_fdc->intrq_r() ? 0x00 : 0x40);
}

void bml3bus_mp1802_device::write(uint8_t data)
{
	floppy_image_device *floppy = m_floppy[data & 0x03]->get_device();

	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(!BIT(data, 5));

	if (floppy)
	{
		floppy->mon_w(!BIT(data, 3));
		floppy->ss_w(BIT(data, 4));
	}

	m_nmigate->in_w<1>(!BIT(data, 6));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_mp1802_device::bml3bus_mp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BML3BUS_MP1802, tag, owner, clock),
	device_bml3bus_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%u", 0U),
	m_nmigate(*this, "nmigate"),
	m_rom(*this, "mp1802_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_mp1802_device::device_start()
{
}

void bml3bus_mp1802_device::device_reset()
{
	write(0);
}

void bml3bus_mp1802_device::map_exrom(address_space_installer &space)
{
	space.install_rom(0xf800, 0xfeff, &m_rom[0]);
	space.install_rom(0xfff0, 0xffff, &m_rom[0x7f0]);
}

void bml3bus_mp1802_device::map_io(address_space_installer &space)
{
	// install into memory
	space.install_readwrite_handler(0xff00, 0xff03, read8sm_delegate(*m_fdc, FUNC(mb8866_device::read)), write8sm_delegate(*m_fdc, FUNC(mb8866_device::write)));
	space.install_readwrite_handler(0xff04, 0xff04, read8smo_delegate(*this, FUNC(bml3bus_mp1802_device::read)), write8smo_delegate(*this, FUNC(bml3bus_mp1802_device::write)));
}
