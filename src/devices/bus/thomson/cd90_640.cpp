// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CD 90-640 - Floppy drive controller built from a wd1770
//
// Handles up to two 5.25 dual-sided drives (DD 90-320)

#include "emu.h"
#include "cd90_640.h"
#include "formats/sap_dsk.h"
#include "formats/thom_dsk.h"

DEFINE_DEVICE_TYPE(CD90_640, cd90_640_device, "cd90_640", "Thomson CD 90-640 floppy drive controller")

cd90_640_device::cd90_640_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CD90_640, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "%u", 0U),
	m_rom(*this, "rom")
{
}

ROM_START(cd90_640)
	ROM_REGION( 0x7c0, "rom", 0 )
	ROM_LOAD ( "cd90-640.rom", 0x000, 0x7c0, CRC(5114c0a5) SHA1(5c72566c22d8160ef0c75959e1863a1309bbbe49) )
ROM_END

void cd90_640_device::rom_map(address_map &map)
{
	map(0x000, 0x7bf).rom().region(m_rom, 0);
}

void cd90_640_device::io_map(address_map &map)
{
	map(0x10, 0x13).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x18, 0x18).rw(FUNC(cd90_640_device::control_r), FUNC(cd90_640_device::control_w));
}

const tiny_rom_entry *cd90_640_device::device_rom_region() const
{
	return ROM_NAME(cd90_640);
}

void cd90_640_device::floppy_drives(device_slot_interface &device)
{
	device.option_add("dd90_320", FLOPPY_525_DD);
}

void cd90_640_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_THOMSON_525_FORMAT);
	fr.add(FLOPPY_SAP_FORMAT);
}

void cd90_640_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, m_floppy[0], floppy_drives, "dd90_320", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
}

void cd90_640_device::device_start()
{
	save_item(NAME(m_control));
}

void cd90_640_device::device_reset()
{
	m_control = 0;
	m_fdc->set_floppy(nullptr);
	m_fdc->dden_w(0);
}

void cd90_640_device::control_w(u8 data)
{
	m_control = data;
	floppy_image_device *floppy = nullptr;
	if(m_control & 2)
		floppy = m_floppy[0]->get_device();
	else if(m_control & 4)
		floppy = m_floppy[1]->get_device();
	if(floppy)
		floppy->ss_w(m_control & 1);
	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(m_control & 0x80 ? 1 : 0);
	logerror("control %02x\n", m_control);
}

u8 cd90_640_device::control_r()
{
	return m_control;
}
