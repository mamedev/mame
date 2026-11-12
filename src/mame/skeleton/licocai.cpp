// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
Cai System LICO 立可遊戲教學系統
(Lìkě Yóuxì Jiàoxué Xìtǒng - "LICO" is an alternate transliteration of "立可")

TODO: Is "Cai System" part of the system name or a brand name?

Educational system, TV Paint style with touchpad and tool/palette selection area
also has a controller featuring
circular D-Pad
3 regular buttons (A-Red, B-Blue, C-Green)
3 buttons above those with function labels
1 Start button

SOCRATES is printed on the cart ROM chips and system customs, but this
doesn't seem to be related to the VTech Socrates system

CPU: MC68000P10
custom chip "SOCRATES A.F-810620-001 9422 Z13 JAPAN"

current system ROM is half size, the system attempts to fetch graphical data from outside of it
*/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/ymopl.h"

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
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vdp_data_upload(uint16_t data, uint16_t mem_mask);
	void update_pen(u16 pen);

	void vdp_dest_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vdp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	u16 vdp_status_r(offs_t offset, uint16_t mem_mask = ~0);
	u16 vdp_data_r(offs_t offset, uint16_t mem_mask = ~0);
	u16 lico_2a0000_r(offs_t offset, uint16_t mem_mask = ~0);
	u16 lico_2a000a_r(offs_t offset, uint16_t mem_mask = ~0);
	void pal_addr_low_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pal_addr_high_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pal_low_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pal_high_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void licocai_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	u16 m_vdp_dest;
	u16 m_vdp_enable_flags;
	u32 m_vdp_write_addr;
	u32 m_vdp_read_addr;
	u32 m_vdp_spritebase_addr;

	u16 m_paladdr;


	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_palhigh;
	std::unique_ptr<u8[]> m_pallow;
};

void licocai_state::vdp_dest_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_vdp_dest = data & mem_mask;
	logerror("%s: set m_vdp_dest to %04x\n", machine().describe_context(), m_vdp_dest);
}

void licocai_state::vdp_data_upload(uint16_t data, uint16_t mem_mask)
{
	logerror("%s: write to vdp_data_w with m_vdp_dest %02x addr %04x: %04x %04x (data_upload?)\n", machine().describe_context(), m_vdp_dest, m_vdp_write_addr, data, mem_mask);

	m_vram[(m_vdp_write_addr + 0) & 0xffff] = (data >> 8) & 0x00ff;
	m_vram[(m_vdp_write_addr + 1) & 0xffff] = (data >> 0) & 0x00ff;

	m_gfxdecode->gfx(1)->mark_dirty(m_vdp_write_addr / 0x80);
	m_gfxdecode->gfx(2)->mark_dirty(m_vdp_write_addr / 0x20);
	m_gfxdecode->gfx(3)->mark_dirty(m_vdp_write_addr / 0x20);
	m_vdp_write_addr += 2;
}

u16 licocai_state::vdp_data_r(offs_t offset, uint16_t mem_mask)
{
	switch (m_vdp_dest)
	{
	case 0x0002:
	{
		u16 dat = (uint16_t(m_vram[(m_vdp_read_addr + 0) & 0xffff]) << 8) | m_vram[(m_vdp_read_addr + 1) & 0xffff];
		if (!machine().side_effects_disabled())
		{
			logerror("%s: vdp_data_r with m_vdp_dest %02x addr %04x (data read)\n", machine().describe_context(), m_vdp_dest, m_vdp_read_addr);
			m_vdp_read_addr += 2;
		}
		return dat;
	}

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: vdp_data_r with m_vdp_dest %02x addr %04x (register read)\n", machine().describe_context(), m_vdp_dest, m_vdp_read_addr);
		return 0x00;
	}
}


void licocai_state::vdp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (m_vdp_dest)
	{
	case 0x0000:
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (vram word write position)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		m_vdp_write_addr = data << 1;
		break;

	case 0x0001:
		// after a while
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (vram word read position)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		m_vdp_read_addr = data << 1;
		break;

	case 0x0002:
		vdp_data_upload(data, mem_mask);
		break;

	case 0x0005: // init to 0000
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (enable flags?)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		m_vdp_enable_flags = data;
		break;

	case 0x0006: // inited to 0000
	case 0x0007: // inited to 0000
	case 0x0008: // inited to 0000
	case 0x0009: // inited to 0000, again after 0x13, also as 0000
	case 0x000a: // inited to 0202
	case 0x000b: // inited to 031f
	case 0x000c: // inited to 0f02
	case 0x000d: // inited to 00ef
	case 0x000e: // inited to 0003
	case 0x000f: // inited to 0010
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (unknown)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		break;

	case 0x0013: // more than once, always 7800 (which is where it uploads the sprite list)
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (sprite base?)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		m_vdp_spritebase_addr = data << 1;
		break;


	default:
		fatalerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (unknown)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		break;
	}
}

u16 licocai_state::vdp_status_r(offs_t offset, uint16_t mem_mask)
{
	// read in irq
	logerror("%s: vdp_status_r\n", machine().describe_context());
	return machine().rand();
}

u16 licocai_state::lico_2a0000_r(offs_t offset, uint16_t mem_mask)
{
	// read at end of IRQs, ack?
	logerror("%s: lico_2a0000_r\n", machine().describe_context());
	return machine().rand();
}

u16 licocai_state::lico_2a000a_r(offs_t offset, uint16_t mem_mask)
{
	// read at end of IRQs, ack?
	logerror("%s: lico_2a000a_r\n", machine().describe_context());
	return machine().rand();
}

void licocai_state::pal_addr_low_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0xff00)
	{
		fatalerror("write to upper byte pal_addr_low_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		logerror("%s: pal_addr_low_w %02x\n", machine().describe_context(), data & 0xff);
		m_paladdr = (m_paladdr & 0x0300) | (data & 0xff);
	}
}

void licocai_state::pal_addr_high_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0xff00)
	{
		fatalerror("write to upper byte pal_addr_high_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		logerror("%s: pal_addr_high_w (pal mode/mask?) %02x\n", machine().describe_context(), data & 0xff);
		m_paladdr = (m_paladdr & 0x00ff) | ((data & 0x03) << 8);
	}
}

void licocai_state::update_pen(u16 pen)
{
#if 0 // disabled for now as it wipes palette
	u16 pal = m_pallow[pen] | (m_palhigh[pen] << 8);

	const u8 r = (pal >> 6) & 0x07;
	const u8 g = (pal >> 3) & 0x07;
	const u8 b = (pal >> 0) & 0x07;

	m_palette->set_pen_color(pen, rgb_t(r << 5, g << 5, b << 5));
#endif
}

void licocai_state::pal_low_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0xff00)
	{
		fatalerror("write to upper byte pal_low_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		logerror("%s: pal_low_w addr:%04x %02x\n", machine().describe_context(), m_paladdr, data & 0xff);
		m_pallow[m_paladdr & 0x3ff] = data & 0xff;
	}
}

void licocai_state::pal_high_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0xff00)
	{
		fatalerror("write to upper byte pal_high_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		logerror("%s: pal_high_w addr:%04x %02x\n", machine().describe_context(), m_paladdr, data & 0xff);
		m_palhigh[m_paladdr & 0x3ff] = data & 0xff;
		update_pen(m_paladdr & 0x3ff);

		m_paladdr++;
	}
}

// there seem to be other formats uploaded too?
static const gfx_layout tile16_ram_4bpp_layout =
{
	16,16,
	0x200,
	4,
	{ 0*256,1*256,2*256,3*256 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
	{ 0*16,1*16,2*16,3*16, 4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*16*4
};

static const gfx_layout tile16_ram_1bpp_layout =
{
	16,16,
	0x800,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
	{ 0*16,1*16,2*16,3*16, 4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*16
};

static const gfx_layout tile8_ram_4bpp_layout =
{
	8,8,
	0x800,
	4,
	{ 0,8, 128, 136 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*16,1*16,2*16,3*16, 4*16,5*16,6*16,7*16 },
	16*16
};


static const gfx_layout tile16_1bpp_layout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
	{ 0*16,1*16,2*16,3*16, 4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*16
};


static GFXDECODE_START( gfx_licocai )
	GFXDECODE_ENTRY( "maincpu", 0, tile16_1bpp_layout,   0, 1  ) // entry 0 - debug (has a 16x16x1bpp font from tiles 5580+)
	// entry 1 created in init
	// entry 2 created in init
	// entry 3 created in init
GFXDECODE_END

void licocai_state::machine_start()
{
	save_item(NAME(m_vdp_dest));
	save_item(NAME(m_vdp_write_addr));
	save_item(NAME(m_vdp_enable_flags));
	save_item(NAME(m_vdp_read_addr));
	save_item(NAME(m_vdp_spritebase_addr));
	save_item(NAME(m_paladdr));

	// clears 0x10000 bytes on startup, so assume main VRAM is that size
	m_vram = make_unique_clear<u8[]>(0x10000);
	save_pointer(NAME(m_vram), 0x10000);

	m_palhigh = make_unique_clear<u8[]>(0x400);
	save_pointer(NAME(m_palhigh), 0x400);

	m_pallow = make_unique_clear<u8[]>(0x400);
	save_pointer(NAME(m_pallow), 0x400);

	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, tile16_ram_4bpp_layout, &m_vram[0x0], 0, m_palette->entries() / 16, 0));
	m_gfxdecode->set_gfx(2, std::make_unique<gfx_element>(m_palette, tile16_ram_1bpp_layout, &m_vram[0x0], 0, m_palette->entries() / 2, 0));
	m_gfxdecode->set_gfx(3, std::make_unique<gfx_element>(m_palette, tile8_ram_4bpp_layout, &m_vram[0x0], 0, m_palette->entries() / 16, 0));
}

void licocai_state::machine_reset()
{
	m_vdp_dest = 0;
	m_vdp_write_addr = 0;
	m_vdp_read_addr = 0;
	m_vdp_enable_flags = 0;
	m_paladdr = 0;
	m_vdp_spritebase_addr = 0;
}

void licocai_state::video_start()
{
}

uint32_t licocai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// bits 0x40, 0x8 and 0x4 are also used at least
	if (!(m_vdp_enable_flags & 0x80))
		return 0;

	// there's a tilemap (or large sprite) at the start of RAM (maybe it can be relocated)
	gfx_element *gfx = m_gfxdecode->gfx(3);
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			u16 dat = m_vram[count + 1] | (m_vram[count + 0] << 8);

			u16 tile = (dat & 0x7ff);

			gfx->transpen(bitmap, cliprect, tile, 0x10, 0, 0, x * 8, y * 8, 0);

			count += 2;
		}
	}

	gfx_element *spgfx = m_gfxdecode->gfx(1);
	for (int i = m_vdp_spritebase_addr; i < m_vdp_spritebase_addr + 0x100; i += 8)
	{
		u16 spritex = (m_vram[i + 2] << 8) | m_vram[i + 3];
		u16 spritey = (m_vram[i + 0] << 8) | m_vram[i + 1];
		u16 tile = ((m_vram[i + 4] << 8) | m_vram[i + 5]) >> 1;

		spritex -= 32;
		spritey -= 64;

		u16 ysize = (m_vram[i + 6] & 0x70) >> 4;
		u16 xsize = (m_vram[i + 6] & 0x07);

		u8 xflip = (m_vram[i + 6] & 0x08) >> 3;

		for (int yc = 0; yc <= ysize; yc++)
		{
			for (int xc = 0; xc <= xsize; xc++)
			{
				if (xflip)
					spgfx->transpen(bitmap, cliprect, tile++, 0x10, 1, 0, spritex + (xsize * 16) - (xc * 16), spritey + (yc * 16), 0);
				else
					spgfx->transpen(bitmap, cliprect, tile++, 0x10, 0, 0, spritex + (xc * 16), spritey + (yc * 16), 0);
			}
		}
	}

	return 0;
}

void licocai_state::licocai_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	// some kind of RAM DAC?
	map(0x200004, 0x200005).w(FUNC(licocai_state::pal_addr_low_w));
	map(0x200006, 0x200007).w(FUNC(licocai_state::pal_addr_high_w));
	map(0x200008, 0x200009).w(FUNC(licocai_state::pal_low_w));
	map(0x20000a, 0x20000b).w(FUNC(licocai_state::pal_high_w));

	map(0x210000, 0x210001).rw(FUNC(licocai_state::vdp_status_r), FUNC(licocai_state::vdp_dest_select_w));
	map(0x210002, 0x210003).rw(FUNC(licocai_state::vdp_data_r), FUNC(licocai_state::vdp_data_w));

	map(0x220000, 0x220001).umask16(0xff00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x220002, 0x220003).umask16(0xff00).w("ymsnd", FUNC(ym3812_device::data_w));

	map(0x2a0000, 0x2a0001).r(FUNC(licocai_state::lico_2a0000_r));
	map(0x2a000a, 0x2a000b).r(FUNC(licocai_state::lico_2a000a_r));

	// tests from 0x7e0000 if not mapped it writes 'system ram error' to the start of vram (but hasn't uploaded a font?)
	map(0x7e0000, 0x7fffff).ram();

	// cart ROMs map here if present
	// [:maincpu] ':maincpu' (010D48): unmapped program memory read from 800100 & FF00
	map(0x800000, 0x8fffff).r(m_cart, FUNC(generic_slot_device::read16_rom));
}

static INPUT_PORTS_START( licocai )
INPUT_PORTS_END



void licocai_state::licocai(machine_config &config)
{
	M68000(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &licocai_state::licocai_map);
	// wrong, just to keep things moving
	m_maincpu->set_periodic_int(FUNC(licocai_state::irq3_line_hold), attotime::from_hz(30));
	m_maincpu->set_periodic_int(FUNC(licocai_state::irq5_line_hold), attotime::from_hz(60)); // music tempo driven by this in kshs

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(licocai_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette).set_entries(0x400);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_licocai);

	YM3812(config, "ymsnd", XTAL(16'000'000)/4).add_route(ALL_OUTPUTS, "speaker", 0.80);

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "licocai_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	SOFTWARE_LIST(config, "cart_list").set_original("licocai_cart");
}

ROM_START( licocai )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "systemrom.bin", 0x000000, 0x100000, BAD_DUMP CRC(29b5942f) SHA1(3a035f64848b4da6c0cc7e7667418360e0527fc4) ) // half size
ROM_END

} // anonymous namespace

CONS( 1992?, licocai, 0, 0, licocai, licocai, licocai_state, empty_init, "Socrates Co. / C&E Inc.", "LICO", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
