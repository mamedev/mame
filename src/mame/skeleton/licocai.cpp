// license:BSD-3-Clause
// copyright-holders:David Haywood

// Educational system, TV Paint style with touchpad and tool/palette selection area
// also has a controller featuring
// circular D-Pad
// 3 regular buttons (A-Red, B-Blue, C-Green)
// 3 buttons above those (turbo?)
// 1 Start button
// 
// SOCRATES is printed on the cart ROM chips and system customs, but this
// doesn't seem to be related to the VTech Socrates system
//
// CPU: MC68000P10
// custom chip "SOCRATES A.F-810620-001 9422 Z13 JAPAN"

#include "emu.h"

#include "cpu/m68000/m68000.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class licocai_state : public driver_device
{
public:
	licocai_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void licocai(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vdp_dest_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vdp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void licocai_map(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load) ATTR_COLD;

	u16 m_vdp_dest;
	u32 m_vdp_addr;
	std::unique_ptr<u8[]> m_vram;
};

DEVICE_IMAGE_LOAD_MEMBER(licocai_state::cart_load)
{
	// carts seem to be full 68000 programs so probably replace the System ROM when loaded

	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void licocai_state::vdp_dest_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_vdp_dest = data & mem_mask;
}

void licocai_state::vdp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x\n", machine().describe_context(), m_vdp_dest, data, mem_mask);

	if (m_vdp_dest == 0x0002) // uploads some 16x16x4 tiles to RAM
	{
		m_vram[(m_vdp_addr + 0) & 0x3ffff] = (data >> 8) & 0x00ff;
		m_vram[(m_vdp_addr + 1) & 0x3ffff] = (data >> 0) & 0x00ff;

		m_gfxdecode->gfx(0)->mark_dirty(m_vdp_addr / 0x80);

		m_vdp_addr+=2;

	}
	else if (m_vdp_dest == 0x0005)
	{
		m_vdp_addr = data;
	}
}

static const gfx_layout tile_layout =
{
	16,16,
	0x800,
	4,
	{ 0*256,1*256,2*256,3*256 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
	{ 0*16,1*16,2*16,3*16, 4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*16*4
};


void licocai_state::machine_start()
{
	save_item(NAME(m_vdp_dest));
	save_item(NAME(m_vdp_addr));

	m_vram = make_unique_clear<u8[]>(0x40000);
	save_pointer(NAME(m_vram), 0x40000);

	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, tile_layout, &m_vram[0x0], 0, m_palette->entries() / 16, 0));
}

void licocai_state::machine_reset()
{
	m_vdp_dest = 0;
	m_vdp_addr = 0;
}

void licocai_state::video_start()
{
}

uint32_t licocai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void licocai_state::licocai_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x210000, 0x210001).w(FUNC(licocai_state::vdp_dest_select_w));
	map(0x210002, 0x210003).w(FUNC(licocai_state::vdp_data_w));

	map(0x700000, 0x7fffff).ram();
}

static INPUT_PORTS_START( licocai )
INPUT_PORTS_END

void licocai_state::licocai(machine_config &config)
{
	M68000(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &licocai_state::licocai_map);
	m_maincpu->set_vblank_int("screen", FUNC(licocai_state::irq3_line_hold));

	// irqs 3 & 5 (indirectly) point to valid code, others (indirectly) point to rte

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(licocai_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x100); // wrong

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "licocai_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(licocai_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("licocai_cart");
}

ROM_START( licocai )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "systemrom.bin", 0x000000, 0x100000, CRC(29b5942f) SHA1(3a035f64848b4da6c0cc7e7667418360e0527fc4) )
ROM_END

} // anonymous namespace

// or is Cai System the publisher?
GAME( 1992?, licocai,     0,        licocai,    licocai,    licocai_state, empty_init, ROT0,  "Socrates / C&E", "LICO Cai System", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
