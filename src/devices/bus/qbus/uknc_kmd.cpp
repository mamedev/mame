// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    UKNC floppy controller (device driver MZ.SYS)

***************************************************************************/

#include "emu.h"
#include "uknc_kmd.h"

#include "machine/pdp11.h"

#include "formats/bk0010_dsk.h"


namespace {

void uknc_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void uknc_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BK0010_FORMAT);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UKNC_KMD, uknc_kmd_device, "uknc_kmd", "UKNC floppy")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  uknc_kmd_device - constructor
//-------------------------------------------------

uknc_kmd_device::uknc_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UKNC_KMD, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
{
}

void uknc_kmd_device::device_add_mconfig(machine_config &config)
{
	K1801VP128(config, m_fdc, XTAL(4'000'000));
	m_fdc->ds_in_callback().set(
			[] (uint16_t data) -> uint16_t
			{
				switch (data & 02003)
				{
					case 02000: return 3;
					case 02001: return 2;
					case 02002: return 1;
					case 02003: return 0;
					default: return -1;
				}
			});
	FLOPPY_CONNECTOR(config, "fdc:0", uknc_floppies, "525qd", uknc_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", uknc_floppies, "525qd", uknc_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", uknc_floppies, "525qd", uknc_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", uknc_floppies, "525qd", uknc_floppy_formats);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void uknc_kmd_device::device_start()
{
	m_bus->install_device(0177130, 0177133,
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::read)),
			emu::rw_delegate(m_fdc, FUNC(k1801vp128_device::write)));
}

