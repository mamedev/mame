// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "ymf740c.h"

#include "speaker.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(YMF740C, ymf740c_device,   "ymf740c",   "Yamaha YMF740C DS-1L")


ymf740c_device::ymf740c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_opl3(*this, "opl3")
{
}

ymf740c_device::ymf740c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ymf740c_device(mconfig, YMF740C, tag, owner, clock)
{
	// Subsystem has write once at $44~$47
	// NOTE: by default it really wants 0x107a000c ("Networth" DS-XG PCI Audio CODEC)
	set_ids(0x1073000c, 0x03, 0x040100, 0x107a000c);
}

void ymf740c_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	// YMF289B compatible with no Power Management capability,
	// which is a 3.3V version of OPL3
	YMF262(config, m_opl3, XTAL(14'318'181));
	m_opl3->add_route(0, "speaker", 1.0, 0);
	m_opl3->add_route(1, "speaker", 1.0, 1);
	m_opl3->add_route(2, "speaker", 1.0, 0);
	m_opl3->add_route(3, "speaker", 1.0, 1);
}

void ymf740c_device::device_start()
{
	pci_card_device::device_start();

	add_map(32*1024, M_MEM, FUNC(ymf740c_device::map));

	// INTA#
	intr_pin = 1;

	minimum_grant = 0x05;
	maximum_latency = 0x19;

	save_item(NAME(m_legacy_audio_control));
}

void ymf740c_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x146;
	status = 0x0210;

	m_legacy_audio_control = 0x0000'907f;

	remap_cb();
}


u8 ymf740c_device::capptr_r()
{
	return 0x50;
}


void ymf740c_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x40, 0x43).lrw32(
		NAME([this] () { return m_legacy_audio_control; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_legacy_audio_control);
			// bit 14 is hardwired to 0, bits
			m_legacy_audio_control &= ~0x4000;
			remap_cb();
		})
	);
//  map(0x44, 0x47) Subsystem Vendor/Device ID write once
//  map(0x48, 0x49) DS-1L Control
//  map(0x4a, 0x4b) DS-1L Power Control
	// Power Management v1.0, D2 support
	map(0x50, 0x53).lr32(NAME([] () { return 0x0401'0001; }));
//  map(0x54, 0x55) Power Management Control / Status
//  map(0x58, 0x59) ACPI mode
}

void ymf740c_device::map(address_map &map)
{
}

void ymf740c_device::fm_map(address_map &map)
{
	map(0x00, 0x03).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
}

void ymf740c_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// assume legacy mapping won't follow PCI bit 0
	if (BIT(m_legacy_audio_control, 1))
	{
		const u16 fm_ranges[4] = { 0x388, 0x398, 0x3a0, 0x3a8 };
		const u16 fm_port = fm_ranges[(m_legacy_audio_control >> 8) & 3];
		io_space->install_device(fm_port, fm_port + 3, *this, &ymf740c_device::fm_map);
	}
}
