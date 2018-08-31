// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98543 medium-resolution color board

***************************************************************************/

#include "emu.h"
#include "hp98543.h"
#include "screen.h"
#include "video/topcat.h"
#include "video/nereid.h"
#include "machine/ram.h"

#define HP98543_SCREEN_NAME   "98543_screen"
#define HP98543_ROM_REGION    "98543_rom"

ROM_START(hp98543)
	ROM_REGION(0x2000, HP98543_ROM_REGION, 0)
	ROM_LOAD("1818-3907.bin", 0x000000, 0x002000, CRC(5e2bf02a) SHA1(9ba9391cf39624ef8027ce42c84e100344b2a2b8))
ROM_END

DEFINE_DEVICE_TYPE(HPDIO_98543, dio16_98543_device, "dio98543", "HP98543 medium-res color DIO video card")

MACHINE_CONFIG_START(dio16_98543_device::device_add_mconfig)
	MCFG_SCREEN_ADD(HP98543_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, dio16_98543_device, screen_update)
	MCFG_SCREEN_SIZE(1024,400)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 400-1)
	MCFG_SCREEN_REFRESH_RATE(70)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, dio16_98543_device, vblank_w));

	MCFG_DEVICE_ADD("topcat0", TOPCAT, XTAL(35904000))
	MCFG_TOPCAT_FB_WIDTH(1024)
	MCFG_TOPCAT_FB_HEIGHT(400)
	MCFG_TOPCAT_PLANEMASK(1)

	MCFG_DEVICE_ADD("topcat1", TOPCAT, XTAL(35904000))
	MCFG_TOPCAT_FB_WIDTH(1024)
	MCFG_TOPCAT_FB_HEIGHT(400)
	MCFG_TOPCAT_PLANEMASK(2)

	MCFG_DEVICE_ADD("topcat2", TOPCAT, XTAL(35904000))
	MCFG_TOPCAT_FB_WIDTH(1024)
	MCFG_TOPCAT_FB_HEIGHT(400)
	MCFG_TOPCAT_PLANEMASK(4)

	MCFG_DEVICE_ADD("topcat3", TOPCAT, XTAL(35904000))
	MCFG_TOPCAT_FB_WIDTH(1024)
	MCFG_TOPCAT_FB_HEIGHT(400)
	MCFG_TOPCAT_PLANEMASK(8)

	MCFG_DEVICE_ADD("nereid", NEREID, 0)
MACHINE_CONFIG_END

const tiny_rom_entry *dio16_98543_device::device_rom_region() const
{
	return ROM_NAME(hp98543);
}

void dio16_98543_device::map(address_map& map)
{
	map(0, 0x7ffff).ram().share("vram");
}

device_memory_interface::space_config_vector dio16_98543_device::memory_space_config() const
{
		return space_config_vector {
				std::make_pair(0, &m_space_config)
		};
}

dio16_98543_device::dio16_98543_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98543_device(mconfig, HPDIO_98543, tag, owner, clock)
{
}

dio16_98543_device::dio16_98543_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_topcat(*this, "topcat%u", 0),
	m_nereid(*this, "nereid"),
	m_space_config("vram", ENDIANNESS_BIG, 8, 19, 0, address_map_constructor(FUNC(dio16_98543_device::map), this)),
	m_rom(*this, HP98543_ROM_REGION),
	m_vram(*this, "vram")

{
}

void dio16_98543_device::device_start()
{
	dio().install_memory(
			0x200000, 0x27ffff,
			read16_delegate(FUNC(dio16_98543_device::vram_r), this),
			write16_delegate(FUNC(dio16_98543_device::vram_w), this));
	dio().install_memory(
			0x560000, 0x563fff,
			read16_delegate(FUNC(dio16_98543_device::rom_r), this),
			write16_delegate(FUNC(dio16_98543_device::rom_w), this));
	dio().install_memory(
			0x564000, 0x565fff,
			read16_delegate(FUNC(dio16_98543_device::ctrl_r), this),
			write16_delegate(FUNC(dio16_98543_device::ctrl_w), this));

	dio().install_memory(
			0x566000, 0x567fff,
			read16_delegate(FUNC(nereid_device::ctrl_r), static_cast<nereid_device*>(m_nereid)),
			write16_delegate(FUNC(nereid_device::ctrl_w), static_cast<nereid_device*>(m_nereid)));
}

void dio16_98543_device::device_reset()
{
}

READ16_MEMBER(dio16_98543_device::rom_r)
{
	return 0xff00 | m_rom[offset];
}

WRITE16_MEMBER(dio16_98543_device::rom_w)
{
}

READ16_MEMBER(dio16_98543_device::ctrl_r)
{
	uint16_t ret = 0;

	for (auto &tc: m_topcat)
		ret |= tc->ctrl_r(space, offset, mem_mask);

	return ret;
}

WRITE16_MEMBER(dio16_98543_device::ctrl_w)
{
	for (auto &tc: m_topcat)
		tc->ctrl_w(space, offset, data, mem_mask);
}

READ16_MEMBER(dio16_98543_device::vram_r)
{
	uint16_t ret = 0;
	for (auto &tc: m_topcat)
		ret |= tc->vram_r(space, offset, mem_mask);
	return ret;
}

WRITE16_MEMBER(dio16_98543_device::vram_w)
{
	for (auto &tc: m_topcat)
		tc->vram_w(space, offset, data, mem_mask);
}

WRITE_LINE_MEMBER(dio16_98543_device::vblank_w)
{
	for (auto &tc: m_topcat)
		tc->vblank_w(state);
}

uint32_t dio16_98543_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int startx[TOPCAT_COUNT], starty[TOPCAT_COUNT];
	int endx[TOPCAT_COUNT], endy[TOPCAT_COUNT];

	bool changed = false;

	for (auto& tc: m_topcat)
		changed |= tc->has_changed();

	if (!changed)
		return UPDATE_HAS_NOT_CHANGED;

	for (int i = 0; i < TOPCAT_COUNT; i++)
		m_topcat[i]->get_cursor_pos(startx[i], starty[i], endx[i], endy[i]);

	for (int y = 0; y < m_v_pix; y++) {
		uint32_t *scanline = &bitmap.pix32(y);

		for (int x = 0; x < m_h_pix; x++) {
			uint8_t tmp = m_vram[y * m_h_pix + x];
			for (int i = 0; i < TOPCAT_COUNT; i++) {
				if (y >= starty[i] && y <= endy[i] && x >= startx[i] && x <= endx[i]) {
					tmp |= 1 << i;
				}
			}
			*scanline++ = m_nereid->map_color(tmp);
		}
	}
	return 0;
}
