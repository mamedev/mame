// License:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

HP98550 high-resolution color board

1280x1024 @ 60Hz, 8 bpp

***************************************************************************/

#include "emu.h"
#include "hp98550.h"
#include "screen.h"
#include "video/nereid.h"
#include "video/catseye.h"
#include "machine/ram.h"

//#define VERBOSE 1
#include "logmacro.h"

namespace {

class dio32_98550_device :
	public device_t,
	public bus::hp_dio::device_dio32_card_interface,
	public device_memory_interface
{
public:
	dio32_98550_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t rom_r(offs_t offset, uint16_t mem_mask = ~0);
	void rom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t catseye_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void catseye_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t vram_r(offs_t offset, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	static constexpr int CATSEYE_COUNT = 8;


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
	required_device<nereid_device> m_nereid;
	required_device_array<catseye_device, CATSEYE_COUNT> m_catseye;

	dio32_98550_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual space_config_vector memory_space_config() const override;

	void vblank_w(int state);
	void int_w(offs_t offset, uint8_t data);

	const address_space_config m_space_config;
	void map(address_map &map);
	void update_int();

	static constexpr int m_fb_width = 2048;
	static constexpr int m_h_pix = 1280;
	static constexpr int m_v_pix = 1024;

	required_region_ptr<uint8_t> m_rom;
	required_shared_ptr_array<uint8_t, 2> m_vram;

	uint16_t m_plane_mask;
	uint8_t m_intreg;
	uint8_t m_ints;
};

ROM_START(hp98550)
	ROM_REGION(0x8000, "hp98550a_rom", 0)
	ROM_LOAD("98550a.bin", 0x000000, 0x008000, CRC(9d639233) SHA1(d6b23a34850f24525ca5fb36de3deb91196d2dc5))
ROM_END

void dio32_98550_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "hp98550_screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(dio32_98550_device::screen_update));
	screen.screen_vblank().set(FUNC(dio32_98550_device::vblank_w));
	screen.set_raw(XTAL(108'108'000), 1689, 0, m_h_pix, 1066, 0, m_v_pix);

	for (int i = 0; i < CATSEYE_COUNT; i++) {
		CATSEYE(config, m_catseye[i], XTAL(108'108'000));
		m_catseye[i]->set_fb_width(m_fb_width);
		m_catseye[i]->set_fb_height(m_v_pix);
		m_catseye[i]->set_plane(i);
		m_catseye[i]->irq_out_cb().set(FUNC(dio32_98550_device::int_w));
	}

	NEREID(config, m_nereid, 0);
}

const tiny_rom_entry *dio32_98550_device::device_rom_region() const
{
	return ROM_NAME(hp98550);
}

void dio32_98550_device::map(address_map& map)
{
	map(0, 0x3fffff).ram().share("vram_video");
	map(0x400000, 0x7fffff).ram().share("vram_overlay");
}

device_memory_interface::space_config_vector dio32_98550_device::memory_space_config() const
{
		return space_config_vector {
				std::make_pair(0, &m_space_config)
		};
}

dio32_98550_device::dio32_98550_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio32_98550_device(mconfig, HPDIO_98550, tag, owner, clock)
{
}

dio32_98550_device::dio32_98550_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio32_card_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_nereid(*this, "nereid"),
	m_catseye(*this, "catseye%d", 0),
	m_space_config("vram", ENDIANNESS_BIG, 8, 23, 0, address_map_constructor(FUNC(dio32_98550_device::map), this)),
	m_rom(*this, "hp98550a_rom"),
	m_vram(*this, { "vram_video", "vram_overlay"}),
	m_intreg(0),
	m_ints(0)
{
}

void dio32_98550_device::device_start()
{
	save_item(NAME(m_intreg));
	save_item(NAME(m_ints));

	dio().install_memory(
			0x200000, 0x3fffff,
			read16s_delegate(*this, FUNC(dio32_98550_device::vram_r)),
			write16s_delegate(*this, FUNC(dio32_98550_device::vram_w)));

	dio().install_memory(
			0x560000, 0x56ffff,
			read16s_delegate(*this, FUNC(dio32_98550_device::rom_r)),
			write16s_delegate(*this, FUNC(dio32_98550_device::rom_w)));

	dio().install_memory(
			0x564000, 0x5648ff,
			read16_delegate(*this, FUNC(dio32_98550_device::catseye_r)),
			write16_delegate(*this, FUNC(dio32_98550_device::catseye_w)));

	dio().install_memory(
			0x566000, 0x5660ff,
			read16s_delegate(*m_nereid, FUNC(nereid_device::ctrl_r)),
			write16s_delegate(*m_nereid, FUNC(nereid_device::ctrl_w)));
}

void dio32_98550_device::device_reset()
{
	m_intreg = 0;
	m_ints = 0;
}

uint16_t dio32_98550_device::rom_r(offs_t offset, uint16_t mem_mask)
{
	LOG("%s: %04x\n", __func__, offset);

	if (offset == 1)
		return m_intreg;

	return 0xff00 | m_rom[offset];
}

void dio32_98550_device::rom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset) {
	case 0:
		reset();
		break;

	case 1:
		m_intreg = data;
		break;

	default:
		logerror("%s: %04x = %04x (mask %04x)\n", __func__, offset << 1, data, mem_mask);
		break;
	}
}

uint16_t dio32_98550_device::catseye_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0;

	for (auto &ce: m_catseye)
		ret |= ce->ctrl_r(space, offset, mem_mask);
	LOG("%s: %04X = %04X\n", __func__, offset << 1, ret);
	return ret;
}

void dio32_98550_device::catseye_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s: %04X = %04X\n", __func__, offset << 1, data);
	for (auto &ce: m_catseye)
		ce->ctrl_w(offset, data, mem_mask);
}

uint16_t dio32_98550_device::vram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0;

	for (auto &ce: m_catseye)
		ret |= ce->vram_r(offset, mem_mask);

	return ret;
}

void dio32_98550_device::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (auto &ce: m_catseye)
		ce->vram_w(offset, data, mem_mask);
}

void dio32_98550_device::vblank_w(int state)
{
	for (auto &ce: m_catseye)
		ce->vblank_w(state);
}

void dio32_98550_device::int_w(offs_t offset, uint8_t data)
{
	LOG("%s: plane%d = %s\n", __func__, offset, data ? "assert" : "deassert");
	m_ints &= ~(1 << offset);
	m_ints |= data;
	update_int();
}

void dio32_98550_device::update_int()
{
	bool state = m_ints;
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

uint32_t dio32_98550_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool changed = false;
	uint8_t mask = 0;

	for (auto &ce: m_catseye)
		changed |= ce->has_changed();

	if (!changed)
		return UPDATE_HAS_NOT_CHANGED;

	for (auto &ce: m_catseye)
		mask |= ce->plane_enabled();

	for (int y = 0; y < m_v_pix; y++) {
		uint32_t *scanline = &bitmap.pix(y);

		for (int x = 0; x < m_h_pix; x++) {
			const int offset = y * m_fb_width +x;
			uint8_t tmp = m_vram[0][offset] & mask;
			uint8_t ovl = m_vram[1][offset] & mask;
			*scanline++ = m_nereid->map_color(tmp, ovl);
		}
	}
	return 0;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98550, bus::hp_dio::device_dio32_card_interface, dio32_98550_device, "dio98550", "HP98550A high-res color DIO video card")
