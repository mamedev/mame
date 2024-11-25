// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Matrox QG-640 640×480×8 graphics card (skeleton)

**********************************************************************/

#include "emu.h"
#include "qg640.h"

#include "cpu/ns32000/ns32000.h"
#include "screen.h"

// device type definition
DEFINE_DEVICE_TYPE(MATROX_QG640, qg640_device, "qg640", "Matrox QG-640 Color Display Processor Card")

qg640_device::qg640_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MATROX_QG640, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_qgcpu(*this, "qgcpu")
	, m_fifo(*this, "fifo")
	, m_acrtc(*this, "acrtc")
	, m_clut(*this, "clut%u", 1U)
{
}

void qg640_device::device_start()
{
}

u32 qg640_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 qg640_device::status_r()
{
	return 0;
}

void qg640_device::unknown_w(u8 data)
{
	logerror("%s: unknown_w 0x%02X\n", machine().describe_context(), data);
}

void qg640_device::clut_w(offs_t offset, u8 data)
{
	m_clut[BIT(offset, 18)]->write(BIT(offset, 16, 2), data);
}

u16 qg640_device::acrtc_r(offs_t offset)
{
	return m_acrtc->read16(!BIT(offset, 17));
}

void qg640_device::acrtc_w(offs_t offset, u16 data)
{
	m_acrtc->write16(!BIT(offset, 17), data);
}

void qg640_device::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("firmware", 0);
	map(0x400000, 0x400000).r(FUNC(qg640_device::status_r));
	map(0x500000, 0x500000).w(FUNC(qg640_device::unknown_w));
	map(0x600000, 0x600000).select(0x70000).w(FUNC(qg640_device::clut_w));
	map(0x780000, 0x780001).select(0x40000).rw(FUNC(qg640_device::acrtc_r), FUNC(qg640_device::acrtc_w));
	map(0xc00000, 0xc1ffff).ram(); // 4x OKI M41464-12
}

void qg640_device::videoram_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram(); // 10x HM53461P-12
}

void qg640_device::device_add_mconfig(machine_config &config)
{
	NS32016(config, m_qgcpu, 18.432_MHz_XTAL / 2); // NS32016N-10 + NS32C201D-10C
	m_qgcpu->set_addrmap(AS_PROGRAM, &qg640_device::mem_map);

	IDT7201(config, m_fifo);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(50_MHz_XTAL / 2, 816, 0, 640, 510, 0, 480); // 25 MHz pixel clock, 30.63 kHz horizontal, 60.07 Hz vertical
	screen.set_screen_update(FUNC(qg640_device::screen_update));

	HD63484(config, m_acrtc, 50_MHz_XTAL / 8); // SCC63484C8N64; divider unknown
	m_acrtc->set_screen("screen");
	m_acrtc->set_addrmap(0, &qg640_device::videoram_map);

	BT471(config, m_clut[0], 0); // IMSG170P-35
	BT471(config, m_clut[1], 0); // IMSG170P-35
}

ROM_START(qg640)
	ROM_REGION16_LE(0x20000, "firmware", 0)
	// "COPYRIGHT (C) 1985 Matrox Electronics, Ltd." (agrees with board copyright date; EPROMs and most other ICs have 1989 date codes, however)
	ROM_LOAD16_BYTE("504-8.bin", 0x00000, 0x10000, CRC(213c5804) SHA1(6ab86aeda47d148dd85a548ef716c98b61c53a1a)) // TMS27C512JL
	ROM_LOAD16_BYTE("505-8.bin", 0x00001, 0x10000, CRC(73610833) SHA1(815b195c37ca1053e5cddfa1efd4764c4550f4b5)) // TMS27C512JL
ROM_END

const tiny_rom_entry *qg640_device::device_rom_region() const
{
	return ROM_NAME(qg640);
}
