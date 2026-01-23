// license:BSD-3-Clause
// copyright-holders: D. Donohoe

#include "emu.h"
#include "att6300p_fdc.h"

#include "imagedev/floppy.h"
#include "machine/upd765.h"


static void att6300p_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

isa8_fdc_6300p_device::isa8_fdc_6300p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_upd765_fdc_device(mconfig, ISA8_FDC_6300P, tag, owner, clock)
{
}

void isa8_fdc_6300p_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, m_fdc, 8'000'000, false, false);
	m_fdc->intrq_wr_callback().set(FUNC(isa8_fdc_6300p_device::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(isa8_fdc_6300p_device::fdc_drq_w));

	// According to "Getting Started with you AT&T 6300 Plus", when the
	// system came in a two-floppy configuration, drive A (lower drive)
	// was 1.2MB, and drive B was 360K.
	FLOPPY_CONNECTOR(config, "fdc:0", att6300p_floppies, "525hd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", att6300p_floppies, "525dd", isa8_fdc_device::floppy_formats).enable_sound(true);
}

void isa8_fdc_6300p_device::rc_map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(isa8_fdc_6300p_device::rc_r), FUNC(isa8_fdc_6300p_device::rc_w));
}

void isa8_fdc_6300p_device::map(address_map &map)
{
	map(0x2, 0x2).rw(FUNC(isa8_fdc_6300p_device::dor_r), FUNC(isa8_fdc_6300p_device::dor_w));
	map(0x4, 0x5).m(m_fdc, FUNC(upd765a_device::map));
}

void isa8_fdc_6300p_device::device_start()
{
	set_isa_device();

	m_isa->install_device(0x0065, 0x0065, *this, &isa8_fdc_6300p_device::rc_map);
	m_isa->install_device(0x03f0, 0x03f7, *this, &isa8_fdc_6300p_device::map);
	m_isa->set_dma_channel(2, this, true);

	isa8_upd765_fdc_device::device_start();

	m_fdc->set_rate(300000);

	save_item(NAME(m_rate));
}

uint8_t isa8_fdc_6300p_device::rc_r()
{
	return m_rate;
}

void isa8_fdc_6300p_device::rc_w(uint8_t data)
{
	constexpr int32_t rates[4] = { 500'000, 300'000, 250'000, 250'000 };
	m_rate = rates[data & 3];
	m_fdc->set_rate(m_rate);
}

DEFINE_DEVICE_TYPE(ISA8_FDC_6300P,   isa8_fdc_6300p_device,   "isa8_fdc_6300p",   "ISA 8bits 6300 Plus FDC hookup")
