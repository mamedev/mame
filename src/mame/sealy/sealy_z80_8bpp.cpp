// license:BSD-3-Clause
// copyright-holders:

/*
Sealy Z80-based games with 8bpp tiles

Earlier hardware than the games in sealy_z80.cpp: palette-based 8bpp tiles
instead of direct colour, a smaller program ROM and the ESAM on lines of its own.

PCB silkscreened SEALY 2001.

Main components (read from a PCB photo):

Zilog Z84C0008PEC
Altera MAX CPLD (part number not readable)
Second, larger Altera device (covered by a sticker)
HM6264ALP-10 RAMs
XTAL (frequency not readable)
Oki M6295 or clone (part number not readable)
Bwsmart E6205 ESAM chip


TODO:
- the game never lets the player win with an all-zero ESAM file: offset 50-63
  holds the odds table (probability out of 64 of accepting a better hand after
  the draw, per hand rank), with byte 59 selecting the tables built into the
  program instead.  The menu that edits it (state 17, 684d) is unreachable in
  this build, so a real chip must come preprogrammed; its contents are unknown
  (the default image has byte 59 set as a guess)
- there seem to be no outputs (lamps, coin counters, hopper): besides the
  ESAM lines the program only writes ports 10, 70, 80, 90 and b0
- check the layer priority and the use of ports 20 and 80
- verify CPU, ESAM, sound and video clocks against hardware
*/


#include "emu.h"

#include "sealy_z80_esam.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <memory>
#include <vector>


namespace {

class sealy_z80_8bpp_state : public driver_device
{
public:
	sealy_z80_8bpp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_esam(*this, "esam"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_opfixed(*this, "opfixed")
	{ }

	void clonekng(machine_config &config) ATTR_COLD;

	void init_clonekng() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sealy_z80_esam_device> m_esam;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_opfixed;

	std::unique_ptr<uint8_t[]> m_decrypted_opcodes;
	tilemap_t *m_tilemap[2] {};
	uint8_t m_nmi_enable = 0;
	uint8_t m_esam_data = 1;
	uint8_t m_esam_release = 1;

	template <unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void videoram_w(offs_t offset, uint8_t data);
	void nmi_enable_w(uint8_t data) { m_nmi_enable = BIT(data, 0); }
	void crypt_mode_w(uint8_t data);
	void esam_data_w(uint8_t data);
	void esam_release_w(uint8_t data);
	void vblank(int state);

	void program_map(address_map &map) ATTR_COLD;
	void opcodes_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


template <unsigned Layer>
TILE_GET_INFO_MEMBER(sealy_z80_8bpp_state::get_tile_info)
{
	uint8_t const *const ram = &m_videoram[Layer * 0x1000];
	uint16_t const code = ram[tile_index] | (ram[tile_index + 0x800] << 8);

	tileinfo.set(Layer, code & (Layer ? 0x0fff : 0x1fff), code >> 13, 0);
}

void sealy_z80_8bpp_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sealy_z80_8bpp_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sealy_z80_8bpp_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[0]->set_transparent_pen(0);
}

void sealy_z80_8bpp_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap[BIT(offset, 12)]->mark_tile_dirty(offset & 0x7ff);
}

uint32_t sealy_z80_8bpp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void sealy_z80_8bpp_state::machine_start()
{
	m_opfixed->set_base(m_decrypted_opcodes.get());

	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_esam_data));
	save_item(NAME(m_esam_release));
}

void sealy_z80_8bpp_state::machine_reset()
{
	m_nmi_enable = 0;
	m_esam_data = 1;
	m_esam_release = 1;
	m_esam->io_w(1);
}

void sealy_z80_8bpp_state::crypt_mode_w(uint8_t data)
{
	// 0x55 is written before calling the ESAM routines at 0x8000-0x88ff and 0x00
	// after they return; purpose unknown, maybe it enables the transforms used there
	logerror("%s: port 90 = %02x\n", machine().describe_context(), data);
}

// The host drives the ESAM I/O line with the value written to port 30 while
// port 40 is 0, and releases it while port 40 is 1.
void sealy_z80_8bpp_state::esam_data_w(uint8_t data)
{
	m_esam_data = BIT(data, 0);
	m_esam->io_w(m_esam_release | m_esam_data);
}

void sealy_z80_8bpp_state::esam_release_w(uint8_t data)
{
	m_esam_release = BIT(data, 0);
	m_esam->io_w(m_esam_release | m_esam_data);
}

void sealy_z80_8bpp_state::vblank(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void sealy_z80_8bpp_state::program_map(address_map &map)
{
	map(0x0000, 0xafff).rom(); // a820-a83f holds a table read at 05c2 and 5f02
	map(0xafd0, 0xafd0).portr("SYSTEM");
	map(0xafe0, 0xafe0).portr("IN0");
	map(0xafe2, 0xafe2).portr("IN1");
	map(0xb000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram().w(FUNC(sealy_z80_8bpp_state::videoram_w)).share(m_videoram);
	map(0xe000, 0xefff).rom(); // default palette, copied to f000 at boot
	map(0xf000, 0xf7ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xf800, 0xffff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
}

void sealy_z80_8bpp_state::opcodes_map(address_map &map)
{
	map(0x0000, 0x9fff).bankr(m_opfixed);
}

void sealy_z80_8bpp_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w("oki", FUNC(okim6295_device::write));
	map(0x20, 0x20).nopw(); // 1 written at the start of the ESAM reset sequence, nothing else
	map(0x30, 0x30).w(FUNC(sealy_z80_8bpp_state::esam_data_w));
	map(0x40, 0x40).w(FUNC(sealy_z80_8bpp_state::esam_release_w));
	map(0x50, 0x50).portw("ESAMCLK");
	map(0x60, 0x60).portw("ESAMRST");
	map(0x70, 0x70).w(FUNC(sealy_z80_8bpp_state::nmi_enable_w)); // same use as in sealy_z80.cpp
	map(0x80, 0x80).nopw(); // 1 written before showing the RAM error screen; video enable as in sealy_z80.cpp?
	map(0x90, 0x90).w(FUNC(sealy_z80_8bpp_state::crypt_mode_w));
	map(0xb0, 0xb0).nopw(); // periodic watchdog strobe
}


static INPUT_PORTS_START( clonekng )
	PORT_START("ESAMCLK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("esam", FUNC(sealy_z80_esam_device::clk_w))

	PORT_START("ESAMRST")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("esam", FUNC(sealy_z80_esam_device::rst_w))

	PORT_START("SYSTEM")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED ) // never read
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET) // ?? the NMI handler restarts the game when this reads 0 for ten frames in a row
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("esam", FUNC(sealy_z80_esam_device::io_r))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Menu Increment")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) // also required when the credits exceed the limit
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Menu Decrement")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double Up / Menu Select")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // never tested
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
INPUT_PORTS_END


static GFXDECODE_START( gfx_clonekng )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x8_raw, 0, 8 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x8_raw, 0, 8 )
GFXDECODE_END


void sealy_z80_8bpp_state::clonekng(machine_config &config)
{
	Z80(config, m_maincpu, 12'000'000 / 2); // Z84C0008PEC, clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_z80_8bpp_state::program_map);
	m_maincpu->set_addrmap(AS_OPCODES, &sealy_z80_8bpp_state::opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &sealy_z80_8bpp_state::io_map);

	// the host samples a bit every ~374 CPU cycles, as on ddz1
	SEALY_Z80_ESAM(config, m_esam, 12'000'000);
	m_esam->set_file_id(0x004d);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(sealy_z80_8bpp_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(sealy_z80_8bpp_state::vblank));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_clonekng);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x800);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 12'000'000 / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock and pin 7 not verified
}


// 克隆王 (Kèlóng Wáng), Clone King - 克隆王 on the title screen. ROM labels 克隆王 1, 克隆王 2, MADE IN SEALY
ROM_START( clonekng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u1", 0x00000, 0x10000, CRC(67e4aa25) SHA1(d5b83e94c116ad436703a76b7ee4c23aefe858f5) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "1.u18", 0x00000, 0x40000, CRC(a825f926) SHA1(119617b00b81e765cd1372526fdbd5efc64f231c) BAD_DUMP )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "2.u21", 0x00000, 0x80000, CRC(f7e9d1fc) SHA1(dddf6c1fc3173c3afc956621599debee216dd145) )

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x20000, CRC(3e8af403) SHA1(36f497820ed7e7cb761f98fe2d9c4f107258c864) )

	// PIN taken from the program, file contents zeroed (the game accepts that as cleared)
	ROM_REGION( 0x120, "esam", 0 )
	ROM_LOAD( "esam.bin", 0x000, 0x120, BAD_DUMP CRC(5c2781c0) SHA1(5564478de7e6e19970cba96b502f091cdbf537d4) ) // reconstructed, not dumped
ROM_END


/*
Program ROM encryption, done by the CPLD as on the sealy_z80.cpp games.

Opcode fetches (M1) and ordinary reads go through two different transforms, each
an XOR followed by a bit permutation.  One of four transform pairs is selected by
A11-A8, A4 and A1 of the ROM offset; the ESAM routines at 0x8000-0x8fff use
different pairs 0 and 1.  On top of that, 256-byte blocks are swapped around
inside each 4K page.
*/

// opcode fetches
uint8_t clonekng_decrypt_opcode(uint8_t data, uint8_t key)
{
	switch (key)
	{
		default:
		case 0: return bitswap<8>(data ^ 0x22, 0, 2, 1, 3, 5, 4, 6, 7);
		case 1: return bitswap<8>(data ^ 0xc0, 6, 3, 7, 1, 4, 2, 0, 5);
		case 2: return bitswap<8>(data ^ 0x88, 7, 3, 6, 2, 1, 0, 5, 4);
		case 3: return bitswap<8>(data ^ 0x0c, 3, 0, 4, 5, 2, 7, 1, 6);
		case 4: return bitswap<8>(data ^ 0x12, 6, 1, 4, 7, 3, 0, 2, 5);
		case 5: return bitswap<8>(data ^ 0x0d, 5, 2, 1, 4, 7, 3, 6, 0);
	}
}

// ordinary reads
uint8_t clonekng_decrypt_data(uint8_t data, uint8_t key)
{
	switch (key)
	{
		default:
		case 0: return bitswap<8>(data ^ 0x28, 4, 5, 2, 6, 0, 3, 7, 1);
		case 1: return bitswap<8>(data ^ 0x09, 2, 4, 0, 7, 6, 1, 5, 3);
		case 2: return bitswap<8>(data ^ 0x50, 4, 7, 5, 0, 6, 1, 2, 3);
		case 3: return bitswap<8>(data ^ 0x11, 6, 1, 3, 4, 7, 5, 0, 2);
		case 4: return bitswap<8>(data ^ 0x88, 3, 6, 1, 0, 4, 5, 7, 2);
		case 5: return bitswap<8>(data ^ 0x05, 0, 4, 6, 5, 2, 1, 3, 7);
	}
}

// transform pair used for each value of bitswap<6>(ROM offset, 11, 10, 9, 8, 4, 1)
constexpr uint8_t CLONEKNG_SELECTORS[64] =
{
	2, 0, 3, 1, 2, 0, 3, 1,
	3, 2, 1, 3, 3, 1, 3, 2,
	3, 2, 0, 1, 2, 0, 2, 1,
	2, 0, 2, 1, 2, 1, 3, 0,
	3, 0, 3, 1, 2, 1, 3, 0,
	3, 0, 3, 1, 3, 2, 0, 1,
	3, 2, 1, 3, 0, 1, 0, 2,
	3, 1, 3, 2, 0, 1, 0, 2,
};

void sealy_z80_8bpp_state::init_clonekng()
{
	uint8_t *const rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(rom, rom + 0x10000);
	m_decrypted_opcodes = std::make_unique<uint8_t[]>(0x10000);

	// ROM A11-A8 for each value of CPU A11-A8
	static constexpr uint8_t BLOCKS[16] = { 0x4, 0x7, 0xb, 0x9, 0x0, 0x6, 0x1, 0x5, 0xc, 0xf, 0x2, 0xd, 0xe, 0x8, 0x3, 0xa };

	for (uint32_t a = 0; a < 0x10000; a++)
	{
		const uint32_t src = (a & 0xf0ff) | (BLOCKS[(a >> 8) & 0x0f] << 8);

		if (src < 0xe000)
		{
			uint8_t key = CLONEKNG_SELECTORS[bitswap<6>(src, 11, 10, 9, 8, 4, 1)];

			// the ESAM routines at 8000-8fff use different pairs 0 and 1
			if ((src & 0xf000) == 0x8000 && key < 2)
				key += 4;

			m_decrypted_opcodes[a] = clonekng_decrypt_opcode(buffer[src], key);
			rom[a] = clonekng_decrypt_data(buffer[src], key);
		}
		else
		{
			// e000-efff holds plain data
			m_decrypted_opcodes[a] = buffer[src];
			rom[a] = buffer[src];
		}
	}

	// The tile number lines are scrambled on the address bus of both tile ROMs;
	// the bits within a tile are not
	uint8_t *const bg = memregion("bgtiles")->base();
	std::vector<uint8_t> bgbuf(bg, bg + 0x40000);
	for (uint32_t i = 0; i < 0x40000; i++)
		bg[i] = bgbuf[bitswap<18>(i, 12, 7, 9, 13, 8, 6, 15, 16, 11, 17, 10, 14, 5, 4, 3, 2, 1, 0)];

	uint8_t *const fg = memregion("fgtiles")->base();
	std::vector<uint8_t> fgbuf(fg, fg + 0x80000);
	for (uint32_t i = 0; i < 0x80000; i++)
		fg[i] = fgbuf[bitswap<19>(i, 6, 14, 9, 12, 8, 11, 18, 7, 17, 10, 16, 13, 15, 5, 4, 3, 2, 1, 0)];
}

} // anonymous namespace


GAME( 2001, clonekng, 0, clonekng, clonekng, sealy_z80_8bpp_state, init_clonekng, ROT0, "Sealy", "Clone King - Kelong Wang", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
