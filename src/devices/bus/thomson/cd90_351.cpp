// license:BSD-3-Clause
// copyright-holders:Olivier Galibert


// CD 90-351 - Custom floppy drive controller (THMFC1)
//
// Handles up to two 3.5 dual-sided drives (DD 90-352)
// or up to two 2.8 dual-sided QDD drivers (QD 90-280)


#include "emu.h"
#include "cd90_351.h"
#include "formats/thom_dsk.h"
#include "machine/thmfc1.h"

DEFINE_DEVICE_TYPE(CD90_351, cd90_351_device, "cd90_351", "Thomson CD 90-351 Diskette Controller")

cd90_351_device::cd90_351_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CD90_351, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_rom(*this, "rom"),
	m_rom_bank(*this, "rom_bank")
{
}

ROM_START(cd90_351)
	// Rom has been dumped from the system, so the unaccessible ranges are
	// missing (and probably totally unimportant)

	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD ( "cd-351-0.rom", 0x0000, 0x7c0, CRC(2c0159fd) SHA1(bab5395ed8bc7c06f9897897f836054e6546e8e8) )
	ROM_LOAD ( "cd-351-1.rom", 0x0800, 0x7c0, CRC(8e58d159) SHA1(dcf992c96e7556b2faee6bacd3f744e56998e6ea) )
	ROM_LOAD ( "cd-351-2.rom", 0x1000, 0x7c0, CRC(c9228b60) SHA1(179e10107d5be91e684069dee80f94847b83201f) )
	ROM_LOAD ( "cd-351-3.rom", 0x1800, 0x7c0, CRC(3ca8e5dc) SHA1(7118636fb5c597c78c2fce17b02aed5e4ba38635) )
ROM_END

void cd90_351_device::rom_map(address_map &map)
{
	map(0x000, 0x7bf).bankr(m_rom_bank);
}

void cd90_351_device::io_map(address_map &map)
{
	map(0x10, 0x17).m("thmfc1", FUNC(thmfc1_device::map));
	map(0x18, 0x18).w(FUNC(cd90_351_device::bank_w));
}

const tiny_rom_entry *cd90_351_device::device_rom_region() const
{
	return ROM_NAME(cd90_351);
}

void cd90_351_device::floppy_drives(device_slot_interface &device)
{
	device.option_add("dd90_352", FLOPPY_35_DD);
	//  device.option_add("qd90_280", FLOPPY_28_QDD);
}

void cd90_351_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_THOMSON_35_FORMAT);
}

void cd90_351_device::device_add_mconfig(machine_config &config)
{
	THMFC1(config, "thmfc1", 16000000);
	FLOPPY_CONNECTOR(config, "thmfc1:0", floppy_drives, "dd90_352", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "thmfc1:1", floppy_drives, nullptr,    floppy_formats).enable_sound(true);
}

void cd90_351_device::device_start()
{
	m_rom_bank->configure_entries(0, 4, m_rom->base(), 0x800);
}

void cd90_351_device::device_reset()
{
	m_rom_bank->set_entry(0);
}

void cd90_351_device::bank_w(u8 data)
{
	logerror("bank_w %d\n", data & 3);
	m_rom_bank->set_entry(data & 3);
}
