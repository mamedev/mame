// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun bwtwo monochrome video controller

***************************************************************************/

#include "emu.h"
#include "bwtwo.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(SBUS_BWTWO, sbus_bwtwo_device, "bwtwo", "Sun bwtwo SBus Video")

void sbus_bwtwo_device::mem_map(address_map &map)
{
	map(0x00000000, 0x00007fff).r(FUNC(sbus_bwtwo_device::rom_r));
	map(0x00400000, 0x0040001f).rw(FUNC(sbus_bwtwo_device::regs_r), FUNC(sbus_bwtwo_device::regs_w));
	map(0x00800000, 0x008fffff).rw(FUNC(sbus_bwtwo_device::vram_r), FUNC(sbus_bwtwo_device::vram_w));
}

ROM_START( sbus_bwtwo )
	ROM_REGION32_BE(0x8000, "prom", ROMREGION_ERASEFF)

	ROM_SYSTEM_BIOS(0, "1081", "P/N 525-1081-01")
	ROMX_LOAD( "bw2_525-1081-01.bin", 0x0000, 0x8000, CRC(8b70c8c7) SHA1(fd750ad2fd6efdde957f8b0f9abf962e14fe221a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "1124", "P/N 525-1124-01")
	ROMX_LOAD( "bw2_525-1124-01.bin", 0x0000, 0x0800, CRC(e37a3314) SHA1(78761bd2369cb0c58ef1344c697a47d3a659d4bc), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *sbus_bwtwo_device::device_rom_region() const
{
	return ROM_NAME( sbus_bwtwo );
}

void sbus_bwtwo_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(sbus_bwtwo_device::screen_update));
	screen.set_size(1152, 900);
	screen.set_visarea(0, 1152-1, 0, 900-1);
	screen.set_refresh_hz(72);
}


sbus_bwtwo_device::sbus_bwtwo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_BWTWO, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
	, m_screen(*this, "screen")
{
}

void sbus_bwtwo_device::device_start()
{
	m_vram = std::make_unique<uint8_t[]>(0x100000);
	save_pointer(NAME(m_vram), 0x100000);

	for (int i = 0; i < 0x100; i++)
	{
		for (int bit = 7; bit >= 0; bit--)
		{
			m_mono_lut[i][7 - bit] = BIT(i, bit) ? 0 : ~0;
		}
	}
}

void sbus_bwtwo_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_bwtwo_device::mem_map);
}

uint32_t sbus_bwtwo_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *line = &m_vram[0];

	for (int y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix32(y);
		for (int x = 0; x < 1152/8; x++)
		{
			memcpy(scanline, m_mono_lut[*line], sizeof(uint32_t) * 8);
			line++;
			scanline += 8;
		}
	}

	return 0;
}

READ8_MEMBER(sbus_bwtwo_device::regs_r)
{
	logerror("%s: regs_r (unimplemented): %08x\n", machine().describe_context(), 0x400000 + offset);
	return 0;
}

WRITE8_MEMBER(sbus_bwtwo_device::regs_w)
{
	logerror("%s: regs_w (unimplemented): %08x = %02x\n", machine().describe_context(), 0x400000 + offset, data);
}

READ32_MEMBER(sbus_bwtwo_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}

READ8_MEMBER(sbus_bwtwo_device::vram_r)
{
	return m_vram[offset];
}

WRITE8_MEMBER(sbus_bwtwo_device::vram_w)
{
	m_vram[offset] = data;
}
