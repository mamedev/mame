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

DEFINE_DEVICE_TYPE(HPDIO_98543, bus::hp_dio::dio16_98543_device, "dio98543", "HP98543 medium-res color DIO video card")

namespace bus::hp_dio {

void dio16_98543_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, HP98543_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(dio16_98543_device::screen_update));
	screen.screen_vblank().set(FUNC(dio16_98543_device::vblank_w));
	screen.set_raw(35.904_MHz_XTAL, 1408, 0, 1024, 425, 0, 400);

	topcat_device &topcat0(TOPCAT(config, "topcat0", 35.904_MHz_XTAL));
	topcat0.set_fb_width(1024);
	topcat0.set_fb_height(400);
	topcat0.set_planemask(1);
	topcat0.irq_out_cb().set(FUNC(dio16_98543_device::int0_w));

	topcat_device &topcat1(TOPCAT(config, "topcat1", 35.904_MHz_XTAL));
	topcat1.set_fb_width(1024);
	topcat1.set_fb_height(400);
	topcat1.set_planemask(2);
	topcat1.irq_out_cb().set(FUNC(dio16_98543_device::int1_w));

	topcat_device &topcat2(TOPCAT(config, "topcat2", 35.904_MHz_XTAL));
	topcat2.set_fb_width(1024);
	topcat2.set_fb_height(400);
	topcat2.set_planemask(4);
	topcat2.irq_out_cb().set(FUNC(dio16_98543_device::int2_w));

	topcat_device &topcat3(TOPCAT(config, "topcat3", 35.904_MHz_XTAL));
	topcat3.set_fb_width(1024);
	topcat3.set_fb_height(400);
	topcat3.set_planemask(8);
	topcat3.irq_out_cb().set(FUNC(dio16_98543_device::int3_w));

	NEREID(config, m_nereid, 0);
}

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
	m_vram(*this, "vram"),
	m_intreg(0)
{
}

void dio16_98543_device::device_start()
{
	save_item(NAME(m_intreg));
	save_item(NAME(m_ints));

	dio().install_memory(
			0x200000, 0x27ffff,
			read16s_delegate(*this, FUNC(dio16_98543_device::vram_r)),
			write16s_delegate(*this, FUNC(dio16_98543_device::vram_w)));
	dio().install_memory(
			0x560000, 0x563fff,
			read16sm_delegate(*this, FUNC(dio16_98543_device::rom_r)),
			write16sm_delegate(*this, FUNC(dio16_98543_device::rom_w)));
	dio().install_memory(
			0x564000, 0x565fff,
			read16_delegate(*this, FUNC(dio16_98543_device::ctrl_r)),
			write16_delegate(*this, FUNC(dio16_98543_device::ctrl_w)));

	dio().install_memory(
			0x566000, 0x567fff,
			read16s_delegate(*m_nereid, FUNC(nereid_device::ctrl_r)),
			write16s_delegate(*m_nereid, FUNC(nereid_device::ctrl_w)));
}

void dio16_98543_device::device_reset()
{
	m_intreg = 0;
}

uint16_t dio16_98543_device::rom_r(offs_t offset)
{
	if (offset == 1)
		return m_intreg;
	return 0xff00 | m_rom[offset];
}

void dio16_98543_device::rom_w(offs_t offset, uint16_t data)
{
	if (offset == 1)
		m_intreg = data;
}

uint16_t dio16_98543_device::ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0;

	for (auto &tc: m_topcat)
		ret |= tc->ctrl_r(space, offset, mem_mask);

	return ret;
}

void dio16_98543_device::ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (auto &tc: m_topcat)
		tc->ctrl_w(space, offset, data, mem_mask);
}

uint16_t dio16_98543_device::vram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0;
	for (auto &tc: m_topcat)
		ret |= tc->vram_r(offset, mem_mask);
	return ret;
}

void dio16_98543_device::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (auto &tc: m_topcat)
		tc->vram_w(offset, data, mem_mask);
}

WRITE_LINE_MEMBER(dio16_98543_device::vblank_w)
{
	for (auto &tc: m_topcat)
		tc->vblank_w(state);
}

WRITE_LINE_MEMBER(dio16_98543_device::int0_w)
{
	m_ints[0] = state;
	update_int();
}

WRITE_LINE_MEMBER(dio16_98543_device::int1_w)
{
	m_ints[1] = state;
	update_int();
}

WRITE_LINE_MEMBER(dio16_98543_device::int2_w)
{
	m_ints[2] = state;
	update_int();
}

WRITE_LINE_MEMBER(dio16_98543_device::int3_w)
{

	m_ints[3] = state;
	update_int();
}


void dio16_98543_device::update_int()
{
	bool state = m_ints[0] || m_ints[1] || m_ints[2] || m_ints[3];
	int line = (m_intreg >> 3) & 7;

	if (!(m_intreg & 0x80))
		state = false;

	irq1_out(line == 1 && state);
	irq2_out(line == 2 && state);
	irq3_out(line == 3 && state);
	irq4_out(line == 4 && state);
	irq5_out(line == 5 && state);
	irq6_out(line == 6 && state);
	irq7_out(line == 7 && state);
}

uint32_t dio16_98543_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int startx[TOPCAT_COUNT], starty[TOPCAT_COUNT];
	int endx[TOPCAT_COUNT], endy[TOPCAT_COUNT];
	bool plane_enabled[TOPCAT_COUNT];
	bool changed = false;

	for (auto& tc: m_topcat)
		changed |= tc->has_changed();

	if (!changed)
		return UPDATE_HAS_NOT_CHANGED;

	for (int i = 0; i < TOPCAT_COUNT; i++) {
		m_topcat[i]->get_cursor_pos(startx[i], starty[i], endx[i], endy[i]);
		plane_enabled[i] = m_topcat[i]->plane_enabled();
	}

	for (int y = 0; y < m_v_pix; y++) {
		uint32_t *scanline = &bitmap.pix(y);

		for (int x = 0; x < m_h_pix; x++) {
			uint8_t tmp = m_vram[y * m_h_pix + x];
			for (int i = 0; i < TOPCAT_COUNT; i++) {
				if (y >= starty[i] && y <= endy[i] && x >= startx[i] && x <= endx[i]) {
					tmp |= 1 << i;
				}
				if (!plane_enabled[i])
					tmp &= ~(1 << i);
			}
			*scanline++ = m_nereid->map_color(tmp);
		}
	}
	return 0;
}

} // namespace bus::hp_dio
