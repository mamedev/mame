// license: BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Mike Stedman
/**************************************************************************************************

Hitachi B(asic Master?) 16000 series?

TODO:
- Barely anything is known about the HW;
- Requires a system disk to make it go further;
- b16ex2: "error message 1101", bypassed between ports $80 and $48
- b16ex2: prints gibberish before the "insert disk" screen, uses $b4000 for the upper tile bank
  in an unidentified kanji format (so bit 7 high for lr, then low)
  \- 0x2517 - 0x2519 - 0x2526 - 0x2d00 - 0x254f - 0x26b0 - 0x23e6

===================================================================================================

B16 EX-II PCB contents:

TIM uPD8253C-5 @ 16B
DMAC uPD8257C-5 @ 18B
INTM / INTS 2x uPD8259AC @ 13D / 15D
Labelless CRTC, Fujitsu 6845 package
FDC is unpopulated on board, should be a 765 coupled with a SED9420C/AC
BDC HN65013G025 @ 2A
CAL HN6223S @ 12K  - Near bus slots
CA HN6022B @ 12L   /
MPX HG61H06R29F @ 9J
RAC HN60236 - 81005V @ 9H
A HN60230 - 81007V @ 9F
PAC HG61H20B12F @ 9D
DECO HG61H15B19F @ 11D
KAM HN671105AU @ 16A
DECI HG61H15B19F @ 2F
VAP NEC uPD65030G035 @ 4K
GN NEC uPD65021G030 @ 2K
4x 32x3 slots, labeled SLOT0 to 3
11 Jumpers "SH1"
Several connectors scattered across the board
Centronics port CN9, Serial port CN8
2 empty sockets for CN10 / CN11 (DE-9 options?)

===================================================================================================

Error codes (TODO: RE them all)
0301 PIT failure
1101 b16ex2 bus config error?

**************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class b16_state : public driver_device
{
public:
	b16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_intm(*this, "intm")
		, m_ints(*this, "ints")
		, m_vram(*this, "vram")
		, m_crtc(*this, "crtc")
		, m_dma(*this, "8237dma")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_char_rom(*this, "pcg")
	{ }

	void b16(machine_config &config);
	void b16ex2(machine_config &config);

protected:
	virtual void video_start() override;

	void b16_map(address_map &map);
	void b16_io(address_map &map);
	void b16ex2_map(address_map &map);
private:
	uint8_t m_crtc_vreg[0x100]{}, m_crtc_index = 0;

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_intm;
	required_device<pic8259_device> m_ints;
	required_shared_ptr<uint16_t> m_vram;
	required_device<mc6845_device> m_crtc;
	required_device<am9517a_device> m_dma;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_char_rom;

	void pcg_w(offs_t offset, uint8_t data);
	void crtc_address_w(uint8_t data);
	void crtc_data_w(uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


void b16_state::video_start()
{
	save_item(NAME(m_crtc_vreg));
	save_item(NAME(m_crtc_index));
}


uint32_t b16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int y=0;y<mc6845_v_display;y++)
	{
		for(int x=0;x<mc6845_h_display;x++)
		{
			int const tile = m_vram[x+y*mc6845_h_display] & 0xff;
			int const color = (m_vram[x+y*mc6845_h_display] & 0x700) >> 8;

			for(int yi=0;yi<mc6845_tile_height;yi++)
			{
				for(int xi=0;xi<8;xi++)
				{
					int const pen = (m_char_rom[tile*16+yi] >> (7-xi) & 1) ? color : 0;

					if(y*mc6845_tile_height < 400 && x*8+xi < 640) /* TODO: safety check */
						bitmap.pix(y*mc6845_tile_height+yi, x*8+xi) = m_palette->pen(pen);
				}
			}
		}
	}

	return 0;
}

void b16_state::pcg_w(offs_t offset, uint8_t data)
{
	m_char_rom[offset] = data;

	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
}

void b16_state::b16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x9ffff).ram(); // probably not all of it.
	map(0xa0000, 0xaffff).ram(); // bitmap?
	map(0xb0000, 0xb7fff).ram().share("vram");
	map(0xb8000, 0xbbfff).w(FUNC(b16_state::pcg_w)).umask16(0x00ff); // pcg
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

void b16_state::b16ex2_map(address_map &map)
{
	b16_state::b16_map(map);
//  map(0x0e****) bus slot ROM?
	map(0x0f8000, 0x0fffff).rom().region("ipl", 0);
	map(0xff8000, 0xffffff).rom().region("ipl", 0);
}

void b16_state::crtc_address_w(uint8_t data)
{
	m_crtc_index = data;
	m_crtc->address_w(data);
}

void b16_state::crtc_data_w(uint8_t data)
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

void b16_state::b16_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x00, 0x07).umask16(0x00ff); // DMA device? PIT mirror?
	map(0x08, 0x0f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x10, 0x13).rw(m_intm, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x18, 0x1b).rw(m_ints, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x20, 0x20).w(FUNC(b16_state::crtc_address_w));
	map(0x22, 0x22).w(FUNC(b16_state::crtc_data_w));
	// b16ex2: jumps to $e0000 if bit 7 high
	map(0x48, 0x48).lr8(NAME([] () { return 0; }));
	//0x79 bit 0 DSW?
	map(0x80, 0x81).portr("SYSTEM");
}


static INPUT_PORTS_START( b16 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END




static const gfx_layout b16_charlayout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( gfx_b16 )
	GFXDECODE_ENTRY( "pcg", 0x0000, b16_charlayout, 0, 1 )
GFXDECODE_END

uint8_t b16_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void b16_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}


void b16_state::b16(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(14'318'181)/2); // unknown xtal, should be 8088 for base machine?
	m_maincpu->set_addrmap(AS_PROGRAM, &b16_state::b16_map);
	m_maincpu->set_addrmap(AS_IO, &b16_state::b16_io);
	m_maincpu->set_irq_acknowledge_callback(m_intm, FUNC(pic8259_device::inta_cb));

	PIT8253(config, m_pit);
	// TODO: unconfirmed, just enough to make it surpass POST checks
	m_pit->set_clk<0>(XTAL(14'318'181) / 8);
//  m_pit->out_handler<0>()
	m_pit->set_clk<1>(XTAL(14'318'181) / 4);
//  m_pit->out_handler<1>()
	m_pit->set_clk<2>(XTAL(14'318'181) / 4);
//  m_pit->out_handler<2>()

    // TODO: wrong type
	AM9517A(config, m_dma, XTAL(14'318'181)/2);
	m_dma->in_memr_callback().set(FUNC(b16_state::memory_read_byte));
	m_dma->out_memw_callback().set(FUNC(b16_state::memory_write_byte));

	MC6845(config, m_crtc, XTAL(14'318'181)/5);    /* unknown variant, unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(b16_state::screen_update));
	screen.set_size(640, 400);
	screen.set_visarea_full();
	screen.set_palette(m_palette);

	PIC8259(config, m_intm);
	m_intm->out_int_callback().set_inputline(m_maincpu, 0);
	m_intm->in_sp_callback().set_constant(1);
//  m_intm->read_slave_ack_callback()

	PIC8259(config, m_ints, 0);
//  m_ints->out_int_callback().set(m_intm, FUNC(pic8259_device::ir?_w));
	m_ints->in_sp_callback().set_constant(0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_b16);
	// TODO: palette format is a guess
	PALETTE(config, m_palette, palette_device::BRG_3BIT).set_entries(8);
}

void b16_state::b16ex2(machine_config &config)
{
	b16_state::b16(config);
	I80286(config.replace(), m_maincpu, XTAL(16'000'000) / 2); // A80286-8 / S
	m_maincpu->set_addrmap(AS_PROGRAM, &b16_state::b16ex2_map);
	m_maincpu->set_addrmap(AS_IO, &b16_state::b16_io);

	// TODO: as above
	m_pit->set_clk<0>(XTAL(16'000'000) / 4);
//  m_pit->out_handler<0>()
	m_pit->set_clk<1>(XTAL(16'000'000) / 4);
//  m_pit->out_handler<1>()
	m_pit->set_clk<2>(XTAL(16'000'000) / 2);
//  m_pit->out_handler<2>()
}


ROM_START( b16 )
	ROM_REGION16_LE( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(7c1c93d5) SHA1(2a1e63a689c316ff836f21646166b38714a18e03) )

	ROM_REGION( 0x4000/2, "pcg", ROMREGION_ERASE00 )
ROM_END

ROM_START( b16ex2 )
	ROM_REGION16_LE( 0x8000, "ipl", ROMREGION_ERASEFF )
	// M27128-L
	ROM_LOAD16_BYTE( "j4c-0072.4b", 0x0001, 0x4000, CRC(7f6e4143) SHA1(afe126639b7161562d93e955c1fc720a93e1596b))
	// M27128-H
	ROM_LOAD16_BYTE( "j5c-0072.6b", 0x0000, 0x4000, CRC(5f3c85ca) SHA1(4988d8e1e763268a62f2a86104db4d106babd242))

	ROM_REGION( 0x4000/2, "pcg", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASE00 )
	// Both HN62301AP, sockets labeled "KANJI1" and "KANJI2"
	ROM_LOAD( "7l1.11f", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD( "7m1.12f", 0x20000, 0x20000, NO_DUMP )
ROM_END

} // anonymous namespace


// TODO: pinpoint MB SKU for both
// Original would be MB-16001, flyer shows a "Basic Master" subtitle
COMP( 1982?, b16,     0,      0,      b16,     b16,   b16_state, empty_init, "Hitachi", "B16",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1983?, b16ex2,  0,      0,      b16ex2,  b16,   b16_state, empty_init, "Hitachi", "B16 EX-II",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// B16 EX-III known to exist
