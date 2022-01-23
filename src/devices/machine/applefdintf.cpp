// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont

#include "emu.h"
#include "applefdintf.h"
#include "formats/ap2_dsk.h"
#include "formats/ap_dsk35.h"
#include "formats/pc_dsk.h"
#include "formats/fs_prodos.h"

void applefdintf_device::formats_525_13(format_registration &fr)
{
	fr.add(FLOPPY_EDD_FORMAT);
	fr.add(FLOPPY_WOZ_FORMAT);
	fr.add(FLOPPY_NIB_FORMAT);
}

void applefdintf_device::formats_525(format_registration &fr)
{
	fr.add(FLOPPY_A216S_FORMAT);
	fr.add(FLOPPY_RWTS18_FORMAT);
	fr.add(FLOPPY_EDD_FORMAT);
	fr.add(FLOPPY_WOZ_FORMAT);
	fr.add(FLOPPY_NIB_FORMAT);
}

void applefdintf_device::formats_35(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_DC42_FORMAT);
	fr.add(FLOPPY_WOZ_FORMAT);
	fr.add(FLOPPY_APPLE_GCR_FORMAT);
	fr.add(FLOPPY_APPLE_2MG_FORMAT);

	fr.add(fs::PRODOS);
}

void applefdintf_device::floppies_525(device_slot_interface &device)
{
	device.option_add("525", FLOPPY_525_SD);
}

void applefdintf_device::floppies_35(device_slot_interface &device)
{
	device.option_add("35sd", OAD34V);
	device.option_add("35dd", MFD51W);
	device.option_add("35hd", MFD75W);
}

applefdintf_device::applefdintf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_phases_cb(*this),
	m_devsel_cb(*this),
	m_sel35_cb(*this),
	m_hdsel_cb(*this),
	m_dat1byte_cb(*this)
{
}

void applefdintf_device::device_start()
{
	m_phases_cb.resolve_safe();
	m_devsel_cb.resolve_safe();
	m_sel35_cb.resolve_safe();
	m_hdsel_cb.resolve_safe();
	m_dat1byte_cb.resolve_safe();
	save_item(NAME(m_phases));
	save_item(NAME(m_phases_input));
}

void applefdintf_device::device_reset()
{
	m_phases = 0xf0;
	m_phases_input = 0x00;
	update_phases();
}

void applefdintf_device::phases_w(u8 phases)
{
	m_phases_input = phases;
	update_phases();
}

void applefdintf_device::update_phases()
{
	u8 mask = m_phases >> 4;
	m_phases_cb((m_phases & mask) | (m_phases_input & (mask ^ 0xf)));
}
