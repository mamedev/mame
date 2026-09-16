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
#include "machine/timer.h"

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

	void vdp_data_upload(uint16_t data, uint16_t mem_mask);

	void vdp_dest_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vdp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	u16 vdp_status_r(offs_t offset, uint16_t mem_mask = ~0);
	u16 lico_2a0000_r(offs_t offset, uint16_t mem_mask = ~0);
	u16 lico_2a000a_r(offs_t offset, uint16_t mem_mask = ~0);
	void lico_200008_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lico_20000a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void licocai_map(address_map &map) ATTR_COLD;

	u16 m_vdp_dest;
	u16 m_vdp_write_type;
	u32 m_vdp_addr;
	std::unique_ptr<u8[]> m_vram;
	//std::unique_ptr<u8[]> m_spram;
};

void licocai_state::vdp_dest_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_vdp_dest = data & mem_mask;
	logerror("%s: set m_vdp_dest to %04x\n", machine().describe_context(), m_vdp_dest);
}

void licocai_state::vdp_data_upload(uint16_t data, uint16_t mem_mask)
{
	//if (m_vdp_write_type == 0x0000)
	{
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x addr %04x: %04x %04x (data_upload?)\n", machine().describe_context(), m_vdp_dest, m_vdp_addr, data, mem_mask);

		m_vram[(m_vdp_addr + 0) & 0xffff] = (data >> 8) & 0x00ff;
		m_vram[(m_vdp_addr + 1) & 0xffff] = (data >> 0) & 0x00ff;

		m_gfxdecode->gfx(1)->mark_dirty(m_vdp_addr / 0x80);
		m_gfxdecode->gfx(2)->mark_dirty(m_vdp_addr / 0x20);
		m_gfxdecode->gfx(3)->mark_dirty(m_vdp_addr / 0x20);
		m_vdp_addr += 2;
	}
	/*
	else if (m_vdp_write_type == 0x00c8)
	{
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x addr %04x: %04x %04x (spritelist upload?)\n", machine().describe_context(), m_vdp_dest, m_vdp_addr, data, mem_mask);
		// write type c8 and address 7800 might be a spritelist?
		m_spram[(m_vdp_addr + 0) & 0x7ff] = (data >> 8) & 0x00ff;
		m_spram[(m_vdp_addr + 1) & 0x7ff] = (data >> 0) & 0x00ff;
		m_vdp_addr += 2;
	}
	else
	{
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x addr %04x: %04x %04x (write type %04x)\n", machine().describe_context(), m_vdp_dest, m_vdp_addr, data, mem_mask, m_vdp_write_type);
	}
	*/
}

void licocai_state::vdp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (m_vdp_dest)
	{
	case 0x0000:
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (vram word position?)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		m_vdp_addr = data << 1;
		break;

	case 0x0001:
		// after a while
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (unknown)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		break;

	case 0x0002:
		vdp_data_upload(data, mem_mask);
		break;

	case 0x0005: // init to 0000
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (write type?)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
		m_vdp_write_type = data;
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

	case 0x0013: // more than once, always 7800
		logerror("%s: write to vdp_data_w with m_vdp_dest %02x: %04x %04x (unknown - writes 7800)\n", machine().describe_context(), m_vdp_dest, data, mem_mask);
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


void licocai_state::lico_200008_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0xff00)
	{
		fatalerror("write to upper byte lico_200008_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		logerror("%s: lico_200008_w %02x\n", machine().describe_context(), data & 0xff);
	}
}

void licocai_state::lico_20000a_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask & 0xff00)
	{
		fatalerror("write to upper byte lico_20000a_w %04x %04x\n", data, mem_mask);
	}
	else
	{
		logerror("%s: lico_20000a_w %02x\n", machine().describe_context(), data & 0xff);
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
	save_item(NAME(m_vdp_addr));
	save_item(NAME(m_vdp_write_type));

	// clears 0x10000 bytes on startup, so assume main VRAM is that size
	m_vram = make_unique_clear<u8[]>(0x10000);
	save_pointer(NAME(m_vram), 0x10000);

	/*
	// uploads ~0x800 bytes of data in what might be this mode
	m_spram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_spram), 0x800);
	*/

	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, tile16_ram_4bpp_layout, &m_vram[0x0], 0, m_palette->entries() / 16, 0));
	m_gfxdecode->set_gfx(2, std::make_unique<gfx_element>(m_palette, tile16_ram_1bpp_layout, &m_vram[0x0], 0, m_palette->entries() / 2, 0));
	m_gfxdecode->set_gfx(3, std::make_unique<gfx_element>(m_palette, tile8_ram_4bpp_layout, &m_vram[0x0], 0, m_palette->entries() / 16, 0));
}

void licocai_state::machine_reset()
{
	m_vdp_dest = 0;
	m_vdp_addr = 0;
	m_vdp_write_type = 0;
}

void licocai_state::video_start()
{
}

uint32_t licocai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	// there's a tilemap (or large sprite) at the start of RAM (maybe it can be relocated)
	gfx_element *gfx = m_gfxdecode->gfx(3);
	int count = 0;
	// left side of screen
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			u16 dat = m_vram[count + 1] | (m_vram[count + 0] << 8);

			u16 tile = (dat & 0x7ff);

			gfx->transpen(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 8, 0);

			count += 2;
		}
	}

	return 0;
}

void licocai_state::licocai_map(address_map &map)
{
	map(0x000000, 0x0fffff).mirror(0x100000).rom();
	// there are data reads from 19E564 etc. why? (is the ROM the proper size?) - handled with mirror for now
	// could be there's a gap in how the ROM maps?

	// 0x200004, 0x200005 // similar to lico_200008_w  reg num?  (could be sound?)
	// 0x200006, 0x200007 // similar to lico_20000a_w  value?

	map(0x200008, 0x200009).w(FUNC(licocai_state::lico_200008_w)); // used as a pair
	map(0x20000a, 0x20000b).w(FUNC(licocai_state::lico_20000a_w));

	map(0x210000, 0x210001).rw(FUNC(licocai_state::vdp_status_r), FUNC(licocai_state::vdp_dest_select_w));
	map(0x210002, 0x210003).w(FUNC(licocai_state::vdp_data_w));

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
	m_maincpu->set_periodic_int(FUNC(licocai_state::irq3_line_hold), attotime::from_hz(400));
	m_maincpu->set_periodic_int(FUNC(licocai_state::irq5_line_hold), attotime::from_hz(200));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(licocai_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x100); // wrong

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_licocai);

	SPEAKER(config, "speaker", 2).front();

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "licocai_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	SOFTWARE_LIST(config, "cart_list").set_original("licocai_cart");
}

ROM_START( licocai )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "systemrom.bin", 0x000000, 0x100000, CRC(29b5942f) SHA1(3a035f64848b4da6c0cc7e7667418360e0527fc4) )
ROM_END

} // anonymous namespace

// or is Cai System the publisher?
GAME( 1992?, licocai,     0,        licocai,    licocai,    licocai_state, empty_init, ROT0,  "Socrates / C&E", "LICO Cai System", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
