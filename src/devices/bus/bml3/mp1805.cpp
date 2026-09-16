// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1805.c

    Hitachi MP-1805 floppy disk controller card for the MB-6890
    Floppy drive is attached (single-sided, single density)

*********************************************************************/

#include "emu.h"
#include "mp1805.h"

#include "softlist_dev.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BML3BUS_MP1805, bml3bus_mp1805_device, "bml3mp1805", "Hitachi MP-1805 3\" Floppy Controller Card")

ROM_START( mp1805 )
	ROM_REGION(0x800, "mp1805_rom", 0)
	// MP-1805 disk controller ROM, which replaces part of the system ROM
	ROM_LOAD( "mp1805.rom", 0x000, 0x800, BAD_DUMP CRC(b532d8d9) SHA1(6f1160356d5bf64b5926b1fdb60db414edf65f22))
ROM_END

void bml3bus_mp1805_device::floppy_drives(device_slot_interface &device)
{
	device.option_add("mb_6890", FLOPPY_3_SSSD);
}


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bml3bus_mp1805_device::device_add_mconfig(machine_config &config)
{
	MC6843(config, m_mc6843, 500000);
	m_mc6843->irq().set(FUNC(bml3bus_mp1805_device::nmi_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], floppy_drives, "mb_6890", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], floppy_drives, nullptr,   floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], floppy_drives, nullptr,   floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], floppy_drives, nullptr,   floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	SOFTWARE_LIST(config, "flop_list").set_original("bml3_flop").set_filter("3");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bml3bus_mp1805_device::device_rom_region() const
{
	return ROM_NAME( mp1805 );
}

uint8_t bml3bus_mp1805_device::read()
{
	// TODO: read supported or not?
	//  return mc6843_drq_r(m_mc6843) ? 0x00 : 0x80;
	return -1;
}

void bml3bus_mp1805_device::write(uint8_t data)
{
	// b7 b6 b5 b4 b3 b2 b1 b0
	// MT ?  ?  ?  D3 D2 D1 D0
	// MT: 0=motor off, 1=motor on
	// Dn: 1=select drive <n>

	logerror("control_w %02x\n", data);
	u8 prev, next;
	bool mon = BIT(data, 7);
	floppy_image_device *fprev = nullptr, *fnext = nullptr;

	if (!mon)
	{
		for(prev = 0; prev < 4; prev++)
			if(BIT(m_control, prev))
				break;

		if (prev < 4)
		{
			fprev = m_floppy[prev]->get_device();
			if (fprev)
			{
				logerror("Drive %d motor off\n",prev);
				fprev->mon_w(1);
			}
		}
	}
	else
	if (data & 15)
	{
		for(next = 0; next < 4; next++)
			if(BIT(data, next))
				break;

		fnext = m_floppy[next]->get_device();
		if (fnext)
		{
			logerror("Drive %d motor on\n",next);
			fnext->mon_w(0);
		}
	}

	m_control = data;
	m_mc6843->set_floppy(fnext);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_mp1805_device::bml3bus_mp1805_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BML3BUS_MP1805, tag, owner, clock),
	device_bml3bus_card_interface(mconfig, *this),
	m_floppy(*this, "%u", 0U),
	m_mc6843(*this, "mc6843"),
	m_rom(*this, "mp1805_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_mp1805_device::device_start()
{
	save_item(NAME(m_control));
}

void bml3bus_mp1805_device::device_reset()
{
	m_control = 0;
}

void bml3bus_mp1805_device::map_exrom(address_space_installer &space)
{
	space.install_rom(0xf800, 0xfeff, &m_rom[0]);
	space.install_rom(0xfff0, 0xffff, &m_rom[0x7f0]);
}

void bml3bus_mp1805_device::map_io(address_space_installer &space)
{
	// install into memory
	space.install_device(0xff18, 0xff1f, *m_mc6843, &mc6843_device::map);
	space.install_readwrite_handler(0xff20, 0xff20, read8smo_delegate(*this, FUNC(bml3bus_mp1805_device::read)), write8smo_delegate(*this, FUNC(bml3bus_mp1805_device::write)));
}
