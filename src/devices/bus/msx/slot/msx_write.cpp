// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper in the Sanyo PHC-77.

*/

#include "emu.h"
#include "msx_write.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_MSX_WRITE, msx_slot_msx_write_device, "msx_slot_msx_write", "MSX Internal MSX-Write")


msx_slot_msx_write_device::msx_slot_msx_write_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_MSX_WRITE, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_switch_port(*this, "SWITCH")
	, m_rombank(*this, "rombank%u", 0U)
	, m_view{ {*this, "view1"}, {*this, "view2"} }
	, m_region_offset(0)
	, m_enabled(true)
{
}

static INPUT_PORTS_START(msx_write)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x01, 0x01, "Firmware is")
	PORT_CONFSETTING(0x00, "disabled")
	PORT_CONFSETTING(0x01, "enabled")
INPUT_PORTS_END

ioport_constructor msx_slot_msx_write_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_write);
}

void msx_slot_msx_write_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() != 0x80000)
	{
		fatalerror("Memory region '%s' is not the correct size for the MSX-Write firmware\n", m_rom_region.finder_tag());
	}

	m_rombank[0]->configure_entries(0, 0x20, m_rom_region->base() + m_region_offset, 0x4000);
	m_rombank[1]->configure_entries(0, 0x20, m_rom_region->base() + m_region_offset, 0x4000);

	page(1)->install_view(0x4000, 0x7fff, m_view[0]);
	m_view[0][0].install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	m_view[0][0].install_write_handler(0x6fff, 0x6fff, emu::rw_delegate(*this, FUNC(msx_slot_msx_write_device::bank_w<0>)));
	m_view[0][0].install_write_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_slot_msx_write_device::bank_w<1>)));
	m_view[0][1];

	page(2)->install_view(0x8000, 0xbfff, m_view[1]);
	m_view[1][0].install_read_bank(0x8000, 0xbfff, m_rombank[1]);
	m_view[1][1];
}

void msx_slot_msx_write_device::device_reset()
{
	m_enabled = BIT(m_switch_port->read(), 0);

	for (int i = 0; i < 2; i++)
	{
		m_rombank[i]->set_entry(i);
		m_view[i].select(m_enabled ? 0 : 1);
	}
}

template <int Bank>
void msx_slot_msx_write_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & 0x1f);
}
