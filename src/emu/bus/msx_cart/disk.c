/*****************************************************************************
 *
 * MSX Floopy drive interface add-on cartridges
 *
 * Currently supported:
 * - Philips VY-0010 (Interface cartridge + 1 3.5" SS floppy drive)
 *
 ****************************************************************************/

#include "emu.h"
#include "disk.h"
#include "formats/msx_dsk.h"


const device_type MSX_CART_VY0010 = &device_creator<msx_cart_vy0010>;


FLOPPY_FORMATS_MEMBER( msx_cart_vy0010::floppy_formats )
	FLOPPY_MSX_FORMAT
FLOPPY_FORMATS_END


static SLOT_INTERFACE_START( msx_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
	SLOT_INTERFACE( "35ssdd", FLOPPY_35_SSDD )
SLOT_INTERFACE_END


msx_cart_vy0010::msx_cart_vy0010(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_VY0010, "MSX Cartridge - VY0010", tag, owner, clock, "msx_cart_vy0010", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_floppy0(*this, "fdc:0")
	, m_floppy1(*this, "fdc:1")
	, m_floppy(NULL)
	, m_control(0)
{
}


static MACHINE_CONFIG_FRAGMENT( vy0010 )
	// From VY-0010 schematic:
	// HLT pulled high
	// SSO/-ENMF + -DDEN + ENP + -5/8 - pulled low
	// READY inverted in VY-0010 cartridge and pulled low on VY-0010/VY-0011 floppy drive
	MCFG_WD2793x_ADD("fdc", XTAL_4MHz / 4)
	MCFG_WD_FDC_FORCE_READY

	// Single sided 3.5" floppy drive
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", msx_floppies, "35ssdd", msx_cart_vy0010::floppy_formats)

	// Attach software lists
	// We do not know in what kind of machine the user has inserted the floppy interface
	// so we list all msx floppy software lists.
	//
	MCFG_SOFTWARE_LIST_ADD("flop_list","msx2_flop")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("msx1_flop_list","msx1_flop")
MACHINE_CONFIG_END


machine_config_constructor msx_cart_vy0010::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vy0010 );
}


void msx_cart_vy0010::device_start()
{
	save_item(NAME(m_side_control));
	save_item(NAME(m_control));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_vy0010::post_load), this));
}


void msx_cart_vy0010::post_load()
{
	UINT8 data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_cart_vy0010::set_control(UINT8 data)
{
	UINT8 old_m_control = m_control;

	m_control = data;

	switch (m_control & 0x03)
	{
		case 0:
		case 2:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : NULL;
			break;

		case 1:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : NULL;
			break;

		default:
			m_floppy = NULL;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w((m_control & 0x80) ? 0 : 1);
		m_floppy->ss_w(m_side_control & 0x01);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		set_led_status(machine(), 0, !(m_control & 0x40));
	}
}


void msx_cart_vy0010::set_side_control(UINT8 data)
{
	m_side_control = data;

	if (m_floppy)
	{
		m_floppy->ss_w(m_side_control & 0x01);
	}
}


void msx_cart_vy0010::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_cart_vy0010::initialize_cartridge()
{
	if ( get_rom_size() != 0x4000 )
	{
		fatalerror("vy0010: Invalid ROM size\n");
	}
}


READ8_MEMBER(msx_cart_vy0010::read_cart)
{
	switch (offset)
	{
		case 0x7ff8:
		case 0xbff8:
			return m_fdc->status_r();

		case 0x7ff9:
		case 0xbff9:
			return m_fdc->track_r();

		case 0x7ffa:
		case 0xbffa:
			return m_fdc->sector_r();

		case 0x7ffb:
		case 0xbffb:
			return m_fdc->data_r();

		case 0x7ffc:
		case 0xbffc:
			return 0xfe | (m_side_control & 0x01);

		case 0x7ffd:
		case 0xbffd:
			return ( m_control & 0x83 ) | 0x78;

		case 0x7fff:
		case 0xbfff:
			return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
	}

	if (offset >= 0x4000 && offset < 0x8000)
	{
		return get_rom_base()[offset & 0x3fff];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_vy0010::write_cart)
{
	switch (offset)
	{
		case 0x7ff8:
		case 0xbff8:
			m_fdc->cmd_w(data);
			break;

		case 0x7ff9:
		case 0xbff9:
			m_fdc->track_w(data);
			break;

		case 0x7ffa:
		case 0xbffa:
			m_fdc->sector_w(data);
			break;

		case 0x7ffb:
		case 0xbffb:
			m_fdc->data_w(data);
			break;

		case 0x7ffc:
		case 0xbffc:
			set_side_control(data);
			break;

		case 0x7ffd:
		case 0xbffd:
			set_control(data);
			break;

		default:
			logerror("msx_cart_vy0010::write_cart: Unmapped write writing %02x to %04x\n", data, offset);
			break;
	}
}


