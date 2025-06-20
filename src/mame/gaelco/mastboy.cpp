// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Master Boy - Gaelco (c) 1991 / 1992

    this is the 2nd release of Master Boy, the original 1987 game is on different hardware, see mastboyo.cpp for that one

    MAME Driver by David Haywood

    Special Thanks to Charles MacDonald
                  and ClawGrip

    Notes:

    Why does RAM-M fail on first boot, Charles mentioned it can (randomly?) fail on real HW too, is it buggy code?
    The EAROM (28C16 parallel EEPROM) takes a long time to write out if RAM-M fails internal testing.

    Are the correct/incorrect samples when you answer a question meant to loop as they do?
    Video timing should be hooked up with MAME's new video timing system

    Are the glitches on the startup screen in the Italian version caused by the original
    having a different internal program, or is it just a (bad) hack?

    This Can be converted to tilemaps very easily, but probably not worth it
*/

/*
    -----------------------------
     Hardware Notes (from f205v)
    -----------------------------

    Master Boy

    Produttore  Gaelco
    N.revisione rev A - Italian
    CPU
    1x HD647180X0CP6-1M1R (main)(on a small piggyback)
    1x SAA1099P (sound)
    1x OKI5205 (sound)
    1x crystal resonator POE400B (close to sound)
    1x oscillator 24.000000MHz (close to main)
    1x Exel XLS28C16AP (backup memory)
    ROMs
    1x M27C2001 (1)
    4x AM27C010 (2,5,6,7)
    2x TMS27C512 (3,4)

*/

/*
    ------------------------------------------------
     Technical Information (from Charles MacDonald)
    ------------------------------------------------

     ROM information

     'hd647180.bin' is a dump of the internal 16K ROM from the Master Boy CPU
     module, stamped with:

     "MASTER-BOY"
     "REV- A"

     .. on the back. This is the Spanish version of the game.

     Technical stuff

     The MMU is hardly used, but the game does ensure the MMU registers
     are set up when an NMI occurs, so that needs to be emulated. Otherwise
     it jumps to address zero to force a reset.

     CPU memory map:

     0000-3FFF : HD647180 internal ROM
     4000-7FFF : Last 16K of 'S.MASTER 3*' ROM
     8000-8FFF : Name table RAM (4K)
     9000-9FFF : Work RAM (4K)
     A000-AFFF : Color RAM
     B000-B7FF : EEPROM (28C16, 2Kx8)
     B800-BBFF : I/O area
     BC00-BFFF : Unused; MCU maps it's internal 512-byte RAM here.
     C000-FFFF : Banked memory area

     I/O area

     The 2K area is split into a 64 byte section repeatedly mirrored throughout
     the 2K range, and the 64 byte section is split into 8 byte sections mirrored
     throughout the 64 bytes:

     B800-B807 : Input port #1 (r/o)
     B808-B80F : Input port #2 (r/o)
     B810-B817 : DIP switch #1 (r/o)
     B818-B81F : DIP switch #2 (r/o)
     B820-B827 : Bank control latch: (w/o)
                 D7 : 1= Access ROMs    | 0= Access VRAM/VROM
                 D6 : ROM select bit 2  | ?
                 D5 : ROM select bit 1  | ?
                 D4 : ROM select bit 0  | ?
                 D3 : Common ROM A17    | ?
                 D2 : Common ROM A16    | ?
                 D1 : Common ROM A15    | VRAM/VROM A15
                 D0 : Common ROM A14    | VRAM/VROM A14

                 ROM select bits:

                 0= "S.MASTER 1" (27C020)
                 1= "S.MASTER 2" (27C010)
                 2= "S.MASTER 5" (27C010)
                 3= "S.MASTER 6" (27C010)
                 4= "S.MASTER 7" (27C010)
                 5= "S.MASTER 8" (27C010)
                 6= Empty socket
                 7= Empty socket

                 The common ROM banking bits address a 256K space; if a 128K ROM
                 is selected then it appears to be mirrored twice.

                 The game may scan the empty socket area when it checks
                 the question data in the ROMs during startup. Not sure
                 what value should be returned, most likely the last byte
                 that was left on the data bus is still present.

     B828-B82F : SAA1099 PSG
     B830-B837 : ADPCM data latch (w/o)
     B838-B83F : Output latches (w/o, only bit 0 of each address is latched)
          B838 : IC30 pin 10 (write to this to acknowledge /INT0 interrupt)
          B839 : MSM5205 S1 (sample rate select, bit 0)
          B83A : MSM5205 S2 (sample rate select, bit 1)
          B83B : MSM5205 RESET
          B83C : IC46 p11 (write to this to enable EEPROM)
          B83D : ?
          B83E : ?
          B83F : ?

     EEPROM

     The EEPROM has a really weird write-protect, I think any access to it
     disables it so that you have to continuously write to the output
     port latch listed above for every read/write. The circuit for this
     is a total mess.

     Timing

     - HD647180 runs at 12.000 / 2 MHz (divider is internal)
     - SAA1099 PSG runs at 6.000 MHz
     - Display pixel clock is 6.000 MHz
     - Oki MSM5205 has a 384 KHz oscillator, this is the standard setup
       so S1,S2 select the same frequencies listed in the datasheet.

     Interrupts

     /INT0 from the scanline counter, probably once per frame but it could
     be ever couple of lines. Needs to be acknowledged by writing to the
     output port latch listed above.

     /NMI from the MSM5205 on every other sample, see below:

     Sound hardware

     SAA1099 PSG has two addresses at $B828,$B829.

     Oki MSM5205 that is driven by the CPU:

     Write to $B830 to load 8-bit latch with two ADPCM samples.
     When MSM5205 RESET is inactive, it automatically generates VCKs at the
     rate specified by S1, S2. On the first VCK a nibble of the ADPCM
     latch is output to the MSM5205. On the second VCK the next nibble
     is output and an NMI is triggered. The NMI handler should load the next
     byte of ADPCM data into $B830.

     Video hardware

     No sprites, one tiled 32x32 background. Tiles use packed nibbles to
     provide 4 bits of data, a palette provides the other 4 to index 256
     color RAM entries that are 12-bits of RGB each (more info follows)
     Tiles come from video RAM (64K) or video ROM (64K) mapped to a 128K space:

     00000-07FFF : 32K RAM (IC90)
     08000-0FFFF : 32K RAM (IC89)
     10000-1FFFF : 64K ROM ("S.MASTER 4")

     The CPU can access this region in 16K banks to read the ROM or read/write
     the RAM.

     The name table RAM is a 32x32 matrix of 4-byte entries. I don't have the
     exact layout but there are at least 12 bits of tile number, 4 bits of
     palette select, and possibly 2 bits for tile flipping.

     The name table RAM mapping to the screen is a little odd. Rows are stored
     in reverse order. (31 to 0 rather than 0 to 31). Of each row, it looks
     like the first 64 bytes define the right half of the row, the next 64
     bytes define the left half.

     Video timing

     Horizontal timing

      84 pixels black right border
     256 pixels active display
      12 pixels black left border
      32 pixels horizontal sync pulse width
     384 pixels total

     Vertical timing

     224 scanlines active display
      58 scanlines vblank + vsync (don't have the exact numbers for top/bottom border)
     282 scanlines total

     Pixel clock is 6.000 MHz, so:

     6.000 / 384 pixels per line / 282 lines per frame = 55.408 Hz refresh rate

     Palette

     There is 1K of color RAM, arranged as 256 words of 16-bit data with
     only 12 bits used:

     (MSB)
     lCD7 - Green bit 3
     lCD6 - Green bit 2
     lCD5 - Green bit 1
     lCD4 - Green bit 0
     lCD3 - Red bit 3
     lCD2 - Red bit 2
     lCD1 - Red bit 1
     lCD0 - Red bit 0
     (LSB)
      CD7 - Blue bit 3
      CD6 - Blue bit 2
      CD5 - Blue bit 1
      CD4 - Blue bit 0
      CD3 - Not used
      CD2 - Not used
      CD1 - Not used
      CD0 - Not used

     So white is $FFF0, etc. The lowest nibble is not connected to anything
     and has no purpose.

     Color DAC resistor weighting is:

     Color bit 0 is 2K ohms
     Color bit 1 is 1K ohms
     Color bit 2 is 500 ohms
     Color bit 3 is 250 ohms
*/


#include "emu.h"

#include "cpu/z180/hd647180x.h"

#include "machine/74259.h"
#include "machine/bankdev.h"
#include "machine/eeprompar.h"

#include "sound/saa1099.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>

namespace {

class mastboy_state : public driver_device
{
public:
	mastboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_msm(*this, "msm")
		, m_outlatch(*this, "outlatch")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_earom(*this, "earom")
		, m_bank_c000(*this, "bank_c000")
		, m_workram(*this, "workram")
		, m_tileram(*this, "tileram")
		, m_vram(*this, "vram")
	{ }

	void mastboy(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);
	void bank_w(uint8_t data);
	void msm5205_data_w(uint8_t data);
	void irq0_ack_w(int state);
	uint8_t port_38_read();
	uint8_t nmi_read();
	void adpcm_int(int state);

	TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);
	void bank_c000_map(address_map &map) ATTR_COLD;
	void mastboy_io_map(address_map &map) ATTR_COLD;
	void mastboy_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<eeprom_parallel_28xx_device> m_earom;
	required_device<address_map_bank_device> m_bank_c000;

	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_vram;

	// video related
	tilemap_t *m_tilemap = nullptr;

	//int m_irq0_ack = 0;
	uint8_t m_m5205_next = 0;
	bool m_m5205_part = false;
};


// VIDEO EMULATION
TILE_GET_INFO_MEMBER(mastboy_state::get_tile_info)
{
	// bytes 0 and 3 seem to be unused for rendering , they appear to contain data the game uses internally
	uint32_t const tileno = (m_tileram[(tile_index << 2) | 1] | (m_tileram[(tile_index << 2) | 2] << 8)) & 0xfff;
	uint32_t const attr = (m_tileram[(tile_index << 2) | 2] & 0xf0) >> 4;

	tileinfo.set((tileno & 0x800) ? 1 : 0, tileno & 0x7ff, attr, 0);
}


void mastboy_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mastboy_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t mastboy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// Access to Banked RAM

uint8_t mastboy_state::vram_r(offs_t offset)
{
	// we have to invert the data for the GFX decode
	return m_vram[offset] ^ 0xff;
}

void mastboy_state::vram_w(offs_t offset, uint8_t data)
{
	// we have to invert the data for the GFX decode
	m_vram[offset] = data ^ 0xff;

	// Decode the new tile
	m_gfxdecode->gfx(0)->mark_dirty(offset/32);
}

void mastboy_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	if (((offset & 3) == 1) || ((offset & 3) == 2)) // see above
		m_tilemap->mark_tile_dirty(offset >> 2);
}

void mastboy_state::bank_w(uint8_t data)
{
	// controls access to banked RAM / ROM
	m_bank_c000->set_bank(data);
}

// MSM5205 Related

void mastboy_state::msm5205_data_w(uint8_t data)
{
	m_m5205_part = false;
	m_m5205_next = data;
}

void mastboy_state::adpcm_int(int state)
{
	m_msm->data_w(m_m5205_next);
	m_m5205_next >>= 4;

	m_m5205_part = !m_m5205_part;
	if(!m_m5205_part)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


// Interrupt Handling

void mastboy_state::irq0_ack_w(int state)
{
	if (state)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void mastboy_state::vblank_irq(int state)
{
	if (state && m_outlatch->q0_r() == 1)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

// Memory Maps

void mastboy_state::mastboy_map(address_map &map)
{
	map(0x4000, 0x7fff).rom(); // External ROM

	map(0x8000, 0x8fff).ram().share(m_workram); // Work RAM
	map(0x9000, 0x9fff).ram().w(FUNC(mastboy_state::tileram_w)).share(m_tileram); // Tilemap RAM
	map(0xa000, 0xa1ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette").mirror(0x0e00);  // Colour RAM

	map(0xc000, 0xffff).m(m_bank_c000, FUNC(address_map_bank_device::amap8));

	map(0xff000, 0xff7ff).rw(m_earom, FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));

	map(0xff800, 0xff807).portr("P1");
	map(0xff808, 0xff80f).portr("P2");
	map(0xff810, 0xff817).portr("DSW1");
	map(0xff818, 0xff81f).portr("DSW2");

	map(0xff820, 0xff827).w(FUNC(mastboy_state::bank_w));
	map(0xff828, 0xff829).w("saa", FUNC(saa1099_device::write));
	map(0xff830, 0xff830).w(FUNC(mastboy_state::msm5205_data_w));
	map(0xff838, 0xff83f).w(m_outlatch, FUNC(ls259_device::write_d0));
}

// TODO : banked map is mirrored?
void mastboy_state::bank_c000_map(address_map &map)
{
	map(0x000000, 0x00ffff).mirror(0x1e0000).rw(FUNC(mastboy_state::vram_r), FUNC(mastboy_state::vram_w)).share(m_vram);
	map(0x010000, 0x01ffff).mirror(0x1e0000).rom().region("vrom", 0);
	map(0x200000, 0x3fffff).rom().region("bankedrom", 0);
}

// Ports

uint8_t mastboy_state::port_38_read()
{
	return 0x00;
}

uint8_t mastboy_state::nmi_read()
{
	// this is read in the NMI, it's related to the Z180 MMU I think, must return right value or game jumps to 0000
	return 0x00;
}

void mastboy_state::mastboy_io_map(address_map &map)
{
	map(0x38, 0x38).r(FUNC(mastboy_state::port_38_read));
	map(0x39, 0x39).r(FUNC(mastboy_state::nmi_read));
}

// Input Ports

static INPUT_PORTS_START( mastboy )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Mode" )
	PORT_DIPSETTING(    0x01, "1" ) // 1: Counts only the right or wrong answer from the player who answered first.
	PORT_DIPSETTING(    0x00, "2" ) // 2: Waits until both players have answered and then counts the right or wrong answers.

	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) // Demo Sounds
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0C, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_DIPNAME( 0x10, 0x10, "Erase Records" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )

	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_8C ) )

	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
INPUT_PORTS_END

// GFX Decodes

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 1) },
	{ 24,28,16,20,8,12,0,4 },
	{ STEP8(0, 32) },
	32*8
};


static GFXDECODE_START( gfx_mastboy )
	GFXDECODE_RAM(   "vram", 0, tiles8x8_layout,      0, 16 )
	GFXDECODE_ENTRY( "vrom", 0, gfx_8x8x4_packed_msb, 0, 16 )
GFXDECODE_END

// Machine Functions / Driver

void mastboy_state::machine_start()
{
	//save_item(NAME(m_irq0_ack));
	save_item(NAME(m_m5205_next));
	save_item(NAME(m_m5205_part));
}

void mastboy_state::machine_reset()
{
	// clear some RAM
	std::fill(&m_workram[0], &m_workram[m_workram.bytes()], 0);
	std::fill(&m_tileram[0], &m_tileram[m_tileram.bytes()], 0);
	std::fill(&m_vram[0],    &m_vram[m_vram.bytes()],       0);
}

void mastboy_state::mastboy(machine_config &config)
{
	HD647180X(config, m_maincpu, 24_MHz_XTAL / 2);   // HD647180X0CP6-1M1R
	m_maincpu->set_addrmap(AS_PROGRAM, &mastboy_state::mastboy_map);
	m_maincpu->set_addrmap(AS_IO, &mastboy_state::mastboy_io_map);

	EEPROM_2816(config, m_earom);

	LS259(config, m_outlatch); // IC17
	m_outlatch->q_out_cb<0>().set(FUNC(mastboy_state::irq0_ack_w));
	m_outlatch->q_out_cb<1>().set("msm", FUNC(msm5205_device::s2_w));
	m_outlatch->q_out_cb<2>().set("msm", FUNC(msm5205_device::s1_w));
	m_outlatch->q_out_cb<3>().set("msm", FUNC(msm5205_device::reset_w));
	m_outlatch->q_out_cb<4>().set("earom", FUNC(eeprom_parallel_28xx_device::oe_w));

	ADDRESS_MAP_BANK(config, m_bank_c000).set_map(&mastboy_state::bank_c000_map).set_options(ENDIANNESS_LITTLE, 8, 22, 0x4000);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(6000000.0 / 384.0 / 282.0);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(mastboy_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mastboy_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mastboy);
	PALETTE(config, m_palette).set_format(palette_device::GRBx_444, 0x100);
	m_palette->set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SAA1099(config, "saa", 6000000).add_route(ALL_OUTPUTS, "mono", 0.50);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(mastboy_state::adpcm_int));  // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::SEX_4B);      // 4KHz 4-bit
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

// Romsets

// On the test screen at boot, most games show an error on "3 RAMM" until NVRAM is recorded after a full demo loop.

/* Gaelco had an internal version of this set with no internal ROM for the MCU, having the first 0x4000 bytes of the EPROM with actual code, and this hash:
   CRC(50a8feaf) SHA1(bf912fbc6883f06aaccbe5e5e71553976fd836d3) */
ROM_START( mastboy )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x0000, 0x4000, CRC(75716dd1) SHA1(9b14b9b889b29b6022a3815de95487fb6a720d7a) ) // game code is internal to the CPU!
	ROM_LOAD( "3.ic77",       0x4000, 0x4000, CRC(64a712ba) SHA1(a8318fa6f5b3fe1aaff4cef07aced927e3503542) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(             0x4000, 0x4000 )
	ROM_CONTINUE(             0x4000, 0x4000 )
	ROM_CONTINUE(             0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "04.bin", 0x00000, 0x10000, CRC(565932f4) SHA1(4b184aa445b5671072031ad4a2ccb13868d6d3a4) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "01.bin", 0x00000, 0x40000, CRC(36755831) SHA1(706fba5fc765502774643bfef8a3c9d2c01eb01b) ) // 99% gfx
	ROM_LOAD( "02.bin", 0x40000, 0x20000, CRC(69cf6b7c) SHA1(a7bdc62051d09636dcd54db102706a9b42465e63) ) // data
	ROM_RELOAD(         0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "05.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "06.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "07.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "08.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

// this set has a number of strings used in the boot-up display intentionally terminated at the first character so they don't display
ROM_START( mastboya )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x0000, 0x4000, CRC(75716dd1) SHA1(9b14b9b889b29b6022a3815de95487fb6a720d7a) ) // game code is internal to the CPU!
	ROM_LOAD( "03.bin",       0x4000, 0x4000, CRC(5020a37f) SHA1(8bc75623232f3ab457b47d5af6cd1c3fb24c0d0e) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(             0x4000, 0x4000 )
	ROM_CONTINUE(             0x4000, 0x4000 )
	ROM_CONTINUE(             0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "04.bin", 0x00000, 0x10000, CRC(565932f4) SHA1(4b184aa445b5671072031ad4a2ccb13868d6d3a4) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "01.bin", 0x00000, 0x40000, CRC(36755831) SHA1(706fba5fc765502774643bfef8a3c9d2c01eb01b) ) // 99% gfx
	ROM_LOAD( "02.bin", 0x40000, 0x20000, CRC(69cf6b7c) SHA1(a7bdc62051d09636dcd54db102706a9b42465e63) ) // data
	ROM_RELOAD(         0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "05.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "06.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "07.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "08.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

/* No internal ROM on the MCU, all the code is on the EPROM.
   Romset labeled "MB ESPANYA ORIGINAL - *CUSTOM I PROG. NOV". */
ROM_START( mastboyb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mboy_espana_programa_tablas_27-oct-92.bin", 0x0000, 0x8000, CRC(8280774c) SHA1(0a850647a9ffe728e800efba8bea033e11b4d559) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE( 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "4.bin", 0x000000, 0x10000, CRC(565932f4) SHA1(4b184aa445b5671072031ad4a2ccb13868d6d3a4) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "s.master.bin",                0x00000, 0x40000, CRC(36755831) SHA1(706fba5fc765502774643bfef8a3c9d2c01eb01b) ) // 99% gfx
	ROM_LOAD( "montajes_sonido_colores.bin", 0x40000, 0x20000, CRC(ad54e36e) SHA1(8d29319937c7594ec360c9fa71e1101f2351f444) ) // data
	ROM_RELOAD( 0x060000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "05.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "06.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "07.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "08.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

/* No internal ROM on the MCU, all the code is on the EPROM.
   Romset labeled "MASTER BOY. 2.". */
ROM_START( mastboyc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3_mb_23-10-91_27c512.bin", 0x0000, 0x8000, CRC(c0bbddf6) SHA1(02759bbe2ad2699c5e0f71cdaf3d0e8d877ef601) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE( 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "original_4_27c512.bin", 0x000000, 0x10000, CRC(eb5fa30d) SHA1(e4acedb70691985fe0de1b4429325d18b791fb90) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "original_1_27c020.bin", 0x00000, 0x40000, CRC(36755831) SHA1(706fba5fc765502774643bfef8a3c9d2c01eb01b) ) // 99% gfx
	ROM_LOAD( "original_2_27c010.bin", 0x40000, 0x20000, CRC(ad54e36e) SHA1(8d29319937c7594ec360c9fa71e1101f2351f444) ) // data
	ROM_RELOAD( 0x060000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "mb_0_preg_5_27c010.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(                         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "mb_1_preg_6_27c010.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(                         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "mb_2_preg_7_27c010.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(                         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "mb_3_preg_8_27c010.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(                         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( mastboyd )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "boy_programa_espana_19-2-92_d92627_27c512.bin", 0x0000, 0x4000, CRC(e46deb17) SHA1(7cf317df9f4a0c88865b5d8059d341051d08aad5) )
	ROM_IGNORE(                                                        0xc000 )
	ROM_LOAD( "3.ic77",                                        0x4000, 0x4000, CRC(64a712ba) SHA1(a8318fa6f5b3fe1aaff4cef07aced927e3503542) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(                                              0x4000, 0x4000 )
	ROM_CONTINUE(                                              0x4000, 0x4000 )
	ROM_CONTINUE(                                              0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "04.bin", 0x00000, 0x10000, CRC(565932f4) SHA1(4b184aa445b5671072031ad4a2ccb13868d6d3a4) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "01.bin", 0x00000, 0x40000, CRC(36755831) SHA1(706fba5fc765502774643bfef8a3c9d2c01eb01b) ) // 99% gfx
	ROM_LOAD( "02.bin", 0x40000, 0x20000, CRC(69cf6b7c) SHA1(a7bdc62051d09636dcd54db102706a9b42465e63) ) // data
	ROM_RELOAD(         0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "05.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "06.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "07.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "08.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( mastboyv2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_sp_v2.bin",     0x0000, 0x4000, CRC(f4c4ae83) SHA1(b131dcf1e4f2a274aed515054d43b316628a3c58) ) // game code is internal to the CPU!
	ROM_LOAD( "mboy_espana_tablas.bin", 0x4000, 0x4000, CRC(3dcd71bd) SHA1(67ceee75c8329c51cf83b56ddfd37c5257f8d8dd) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(                       0x4000, 0x4000 )
	ROM_CONTINUE(                       0x4000, 0x4000 )
	ROM_CONTINUE(                       0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "fi6_esp_4.bin", 0x000000, 0x10000, CRC(eb5fa30d) SHA1(e4acedb70691985fe0de1b4429325d18b791fb90) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "fi6_esp_3.bin",            0x00000, 0x40000, CRC(c491998d) SHA1(10cd8855391d24fc09831e7e812dda3de24b789b) ) // 99% gfx
	ROM_LOAD( "mont_sonid_esp_color.bin", 0x40000, 0x20000, CRC(514b1c71) SHA1(cbdd8cef90afe547ead34d6f71c092ed07d7781d) ) // data
	ROM_RELOAD( 0x060000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "05.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "06.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "07.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "08.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( mastboyv2a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_sp_v2.bin",         0x0000, 0x4000, CRC(f4c4ae83) SHA1(b131dcf1e4f2a274aed515054d43b316628a3c58) ) // game code is internal to the CPU!
	ROM_LOAD( "mboy_espana_tablas_alt.bin", 0x4000, 0x4000, CRC(71ac5623) SHA1(53a6ca20745a92a40e6168804164903bec2c2bbd) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(                           0x4000, 0x4000 )
	ROM_CONTINUE(                           0x4000, 0x4000 )
	ROM_CONTINUE(                           0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "fi6_esp_4.bin", 0x000000, 0x10000, CRC(eb5fa30d) SHA1(e4acedb70691985fe0de1b4429325d18b791fb90) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "fi6_esp_3.bin",            0x00000, 0x40000, CRC(c491998d) SHA1(10cd8855391d24fc09831e7e812dda3de24b789b) ) // 99% gfx
	ROM_LOAD( "mont_sonid_esp_color.bin", 0x40000, 0x20000, CRC(514b1c71) SHA1(cbdd8cef90afe547ead34d6f71c092ed07d7781d) ) // data
	ROM_RELOAD( 0x060000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347  */
	ROM_LOAD( "05.bin", 0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(         0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "06.bin", 0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(         0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "07.bin", 0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(         0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "08.bin", 0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(         0x160000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

/* Romset labeled "MB ESPANYA OLIMPIADES".
   Gaelco had an internal version of this set with no internal ROM for the MCU, having the first 0x4000 bytes of the EPROM with actual code, and this hash:
   CRC(ce6f4be5) SHA1(8190f0d7579cedf4f995edbc4632d74a1af218be) */
ROM_START( mastboyol )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_sp.bin",                  0x0000, 0x4000, CRC(826f56fb) SHA1(70ee8ff1d7a87ac6425327a93ac51a9e34d9ebef) ) // game code is internal to the CPU!
	ROM_LOAD( "mboy_espana_tablas_18-oct-92.bin", 0x4000, 0x4000, CRC(a463b5f9) SHA1(87348613d4200c485b66f98184a8c0cc2ce23883) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(                                 0x4000, 0x4000 )
	ROM_CONTINUE(                                 0x4000, 0x4000 )
	ROM_CONTINUE(                                 0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "mb_4a_fig_esp_18-oct-92.bin", 0x000000, 0x10000, CRC(eb5fa30d) SHA1(e4acedb70691985fe0de1b4429325d18b791fb90) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "mb_1a_fig_esp-18-oct-92.bin",                0x00000, 0x40000, CRC(2dde3ae0) SHA1(b7f771340d7ce207e75c6c77f8bc93ffc54f649b) ) // 99% gfx
	ROM_LOAD( "mb_2a_montaje_sonido_colores_18-oct-92.bin", 0x40000, 0x20000, CRC(523789d5) SHA1(db29debda088f9ad35f5cfdb51b21c0eb8642593) ) // data
	ROM_RELOAD( 0x060000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347 */
	ROM_LOAD( "mb_0_preg_5_27c010.bin",           0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(                                   0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "mb_1_preg_6_27c010.bin",           0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(                                   0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "mb_2_preg_7_27c010.bin",           0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(                                   0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "mb_3_preg_8_27c010.bin",           0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(                                   0x160000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - BARCELONA'92   011092 6526 0385 */
	ROM_LOAD( "mb_9b_pregunt_espana_6-11-92.bin", 0x180000, 0x020000, CRC(60b52d6b) SHA1(29eda481bce073ae0555bb75a72c7ec3926d499f) ) // questions
	ROM_RELOAD(                                   0x1A0000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

// Romset labeled "MB ESPANYA OLIMPIADES *".
ROM_START( mastboyola )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_sp.bin",                      0x0000, 0x4000, CRC(826f56fb) SHA1(70ee8ff1d7a87ac6425327a93ac51a9e34d9ebef) ) // game code is internal to the CPU!
	ROM_LOAD( "mboy_espana_tablas_alt_18-oct_92.bin", 0x4000, 0x4000, CRC(e2691a11) SHA1(f624e7f92c3f6b88bb9f008a8f00970815c7910e) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE(                                     0x4000, 0x4000 )
	ROM_CONTINUE(                                     0x4000, 0x4000 )
	ROM_CONTINUE(                                     0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "mb_4a_fig_esp_18-oct-92.bin", 0x000000, 0x10000, CRC(eb5fa30d) SHA1(e4acedb70691985fe0de1b4429325d18b791fb90) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // banked data - 8 banks, 6 'question' slots
	ROM_LOAD( "mb_1a_fig_esp-18-oct-92.bin",                0x00000, 0x40000, CRC(2dde3ae0) SHA1(b7f771340d7ce207e75c6c77f8bc93ffc54f649b) ) // 99% gfx
	ROM_LOAD( "mb_2a_montaje_sonido_colores_18-oct-92.bin", 0x40000, 0x20000, CRC(523789d5) SHA1(db29debda088f9ad35f5cfdb51b21c0eb8642593) ) // data
	ROM_RELOAD( 0x060000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA ESP. 110691 7659 0412
    SOCIALES - HISTORIA       110691 7635 0428
    ESPECTACULOS - CINE       110691 7455 0318
    CIENCIAS - GENERAL        110691 7482 0347 */
	ROM_LOAD( "mb_0_preg_5_27c010.bin",                0x080000, 0x020000, CRC(394cb674) SHA1(1390c666772f1e1e2da8866b960a3d24dc660e68) ) // questions
	ROM_RELOAD(                                        0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SOCIALES - GEOGRAFIA MUN. 110691 7054 0513
    VARIOS - CULTURA GENERAL  110691 7419 0352 */
	ROM_LOAD( "mb_1_preg_6_27c010.bin",                0x0c0000, 0x020000, CRC(aace7120) SHA1(5655b56a7c241bc7908081088042601174c0a0b2) ) // questions
	ROM_RELOAD(                                        0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - GENERAL        011091 9700 1045 */
	ROM_LOAD( "mb_2_preg_7_27c010.bin",                0x100000, 0x020000, CRC(6618b002) SHA1(79942350da335a3362b6fc43527b6568ce134ceb) ) // questions
	ROM_RELOAD(                                        0x120000, 0x020000 ) // 128kb ROMs are mirrored

/*  VARIOS - CULTURA GENERAL  041091 5970 0585
    VARIOS - CULTURA GENERAL  061091 5300 0245
    CIENCIAS - GENERAL        041091 5630 0275 */
	ROM_LOAD( "mb_3_preg_8_27c010.bin",                0x140000, 0x020000, CRC(6a4870dd) SHA1(f8ca94a5bc4ba3f512767901e4ae3579c2c6355a) ) // questions
	ROM_RELOAD(                                        0x160000, 0x020000 ) // 128kb ROMs are mirrored

/*  DEPORTES - BARCELONA'92   011092 6526 0385 */
	ROM_LOAD( "mb_9b_alt_pregunt_espana_10-10-92.bin", 0x180000, 0x020000, CRC(eeaf0a6d) SHA1(4e4ce87b9e4e552bab9c41c767fe5f4b32e0eb73) ) // questions
	ROM_RELOAD(                                        0x1A0000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x1c0000 tt 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END


// Italian sets

/* The internal ROM for the (v1) Italian sets wasn't trojaned, instead it came from a ROM from the Gaelco archives.
   So, marked as BAD_DUMP on these sets as a precaution, although it could very well be correct */

ROM_START( mastboyi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mboy_programa_italia_18-3-92_27c512.bin", 0x0000, 0x4000, BAD_DUMP CRC(4137cf86) SHA1(1746b92a90f69fc9cf23c9ce59593d6a3254a17e) )
	ROM_IGNORE(                                          0xc000 )
	ROM_LOAD( "3-mem-a.ic77",                    0x4000, 0x4000, CRC(3ee33282) SHA1(26371e3bb436869461e9870409b69aa9fb1845d6) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                                0x4000, 0x4000 )
	ROM_CONTINUE(                                0x4000, 0x4000 )
	ROM_CONTINUE(                                0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "4.ic91", 0x00000, 0x10000, CRC(858d7b27) SHA1(b0ddf49df5665003f3616d67f7fc27408433483b) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "1-mem-c.ic75", 0x00000, 0x40000, CRC(7c7b1cc5) SHA1(73ad7bdb61d1f99ce09ef3a5a3ae0f1e72364eee) ) // 99% gfx
	ROM_LOAD( "2-mem-b.ic76", 0x40000, 0x20000, CRC(87015c18) SHA1(a16bf2707ce847da0923662796195b75719a6d77) ) // data
	ROM_RELOAD(               0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  TEMPO LIBERO - HOBBY GIOCHI 011091 5457 0300
    SCIENZA - NATURA            011091 5657 0400
    SPORT - MONDIALI-90         011091 5999 0212
    MUSICA - AUTORI CANZONI     011091 5496 0314 */
	ROM_LOAD( "5-rom.ic95",   0x080000, 0x020000, CRC(adc07f12) SHA1(2e0b46ac5884ad459bc354f56ff384ff1932f147) )
	ROM_RELOAD(               0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SPORT- GENERALE             011091 5294 0713
    SPETTACOLO - CINE-TV        011091 5657 0400 */
	ROM_LOAD( "6-rom.ic96",   0x0c0000, 0x020000, CRC(2c52cb1e) SHA1(d58f21c09bd3983497f74ab6c5a37977d9e30f0c) )
	ROM_RELOAD(               0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SCIENZA - STORIA            011091 5430 0375
    SCIENZA - GEOGRAFIA         011091 5010 0565 */
	ROM_LOAD( "7-rom.ic97",   0x100000, 0x020000, CRC(7818408f) SHA1(2a69688b6cda5baf2a45966dd86f10b2fcd54b66) )
	ROM_RELOAD(               0x120000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x140000 to 0x17ffff EMPTY
	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

// Only one of the question ROMs differs from 'mastboyi' (minor wording / spelling changes in most cases).
ROM_START( mastboyia )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mboy_programa_italia_18-3-92_27c512.bin", 0x0000, 0x4000, BAD_DUMP CRC(4137cf86) SHA1(1746b92a90f69fc9cf23c9ce59593d6a3254a17e) )
	ROM_IGNORE(                                          0xc000 )
	ROM_LOAD( "3-mem-a.ic77",                    0x4000, 0x4000, CRC(3ee33282) SHA1(26371e3bb436869461e9870409b69aa9fb1845d6) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                                0x4000, 0x4000 )
	ROM_CONTINUE(                                0x4000, 0x4000 )
	ROM_CONTINUE(                                0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "4.ic91", 0x00000, 0x10000, CRC(858d7b27) SHA1(b0ddf49df5665003f3616d67f7fc27408433483b) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "1-mem-c.ic75", 0x00000, 0x40000, CRC(7c7b1cc5) SHA1(73ad7bdb61d1f99ce09ef3a5a3ae0f1e72364eee) ) // 99% gfx
	ROM_LOAD( "2-mem-b.ic76", 0x40000, 0x20000, CRC(87015c18) SHA1(a16bf2707ce847da0923662796195b75719a6d77) ) // data
	ROM_RELOAD(               0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  TEMPO LIBERO - HOBBY GIOCHI 011091 5457 0300
    SCIENZA - NATURA            011091 5657 0400
    SPORT - MONDIALI-90         011091 5999 0212
    MUSICA - AUTORI CANZONI     011091 5496 0314 */
	ROM_LOAD( "5-alt.ic95",   0x080000, 0x020000, CRC(efa442fa) SHA1(5211ea122083120028348418e33cb71b4ce52b8f) )
	ROM_RELOAD(               0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SPORT- GENERALE             011091 5294 0713
    SPETTACOLO - CINE-TV        011091 5657 0400 */
	ROM_LOAD( "6-rom.ic96",   0x0c0000, 0x020000, CRC(2c52cb1e) SHA1(d58f21c09bd3983497f74ab6c5a37977d9e30f0c) )
	ROM_RELOAD(               0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SCIENZA - STORIA            011091 5430 0375
    SCIENZA - GEOGRAFIA         011091 5010 0565 */
	ROM_LOAD( "7-rom.ic97",   0x100000, 0x020000, CRC(7818408f) SHA1(2a69688b6cda5baf2a45966dd86f10b2fcd54b66) )
	ROM_RELOAD(               0x120000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x140000 to 0x17ffff EMPTY
	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( mastboyib )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_ia.bin", 0x0000, 0x4000, CRC(a0556932) SHA1(0fc3cd8ff497002f22fcb236c8b77ec840bf1bb3) ) // game code is internal to the CPU!
	ROM_LOAD( "mboy_italia_tablas_27-oct-92_27c512.bin", 0x4000, 0x4000, CRC(21e1fd0a) SHA1(71ab677f6af5cb2c4a40a156289f5af29c6c4a9a) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                                        0x4000, 0x4000 )
	ROM_CONTINUE(                                        0x4000, 0x4000 )
	ROM_CONTINUE(                                        0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "figuras_mboy_italia_18-3-92_27c512.bin", 0x00000, 0x10000, CRC(388beade) SHA1(2161ac884d5537293e2dd9786b3556bbc8ebdce6) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "figuras_mboy_italia_18-3-92_27c020.bin",             0x00000, 0x40000, CRC(cecbde31) SHA1(5fbe5fbceb9864ede0cf9c593b66df1c9ff88822) ) // 99% gfx
	ROM_LOAD( "montajes_parlante_mboy_italia_20-ene-92_27c010.bin", 0x40000, 0x20000, CRC(439ad40a) SHA1(9235bb1c63f852a26a9467fec8c029a2974ef51b) ) // data
	ROM_RELOAD(               0x60000, 0x20000 ) // 128kb ROMs are mirrored

/* SPORT - GENERALE       011092 7515 1329 */
	ROM_LOAD( "15_domande-rom.ic92", 0x080000, 0x020000, CRC(45dd77e3) SHA1(856fbd1b7f888e1768abceb2465d5bb97a685332) )
	ROM_RELOAD(                      0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - STORIA       011092 6361 0808
   MUSICA - GENERALE      011092 6875 0719 */
	ROM_LOAD( "16_domande-rom.ic93", 0x0c0000, 0x020000, CRC(31ececb2) SHA1(a62b1ecdedf8c587afefef4a7d5cdc9746abb093) )
	ROM_RELOAD(                      0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - GEOGRAFIA    011092 6756 0565
   SPETTACOLOS - CINE-TV  011092 6171 0918 */
	ROM_LOAD( "17_domande-rom.ic94", 0x100000, 0x020000, CRC(bdce54df) SHA1(b30a3adcdeba26f91f7de8e174f54a158d173dba) )
	ROM_RELOAD(                      0x120000, 0x020000 ) // 128kb ROMs are mirrored

/* TEMPO LIBERO - CULTURA 011092 6913 0424
   SCIENZA - NATURA       011092 6969 0400
   TEMPO LIBERO - HOBBY   011092 6569 0300
   SPORT - WC_90          011092 6072 0212
   SCIENZA - SESSUOLOGIA  011092 6098 0276 */
	ROM_LOAD( "18_domande-rom.ic95", 0x140000, 0x020000, CRC(3ea4dd86) SHA1(6db92010ab6d6adbdf6bea9b257423bb7607c7f2) )
	ROM_RELOAD(                      0x160000, 0x020000 ) // 128kb ROMs are mirrored

/* SPETTACOLOS - FUMETTI  011092 6938 0496
   TEMPO LIBERO - PAROLE  011092 6075 0219 */
	ROM_LOAD( "19_domande-rom.ic96", 0x180000, 0x020000, CRC(146c46f9) SHA1(a6b09ffb98146ed2eb67a9d43465abc076758d60) )
	ROM_RELOAD(                      0x1a0000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

// No internal ROM on the MCU, all the code is on the EPROM.
ROM_START( mastboyic )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mboy_italia_programa_tablas_27-oct-92_27c512.bin", 0x0000, 0x8000, CRC(1d14ea03) SHA1(c521d3a640db72e350452ab293e04c56271825de) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE( 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "figuras_mboy_italia_18-3-92_27c512.bin", 0x00000, 0x10000, CRC(388beade) SHA1(2161ac884d5537293e2dd9786b3556bbc8ebdce6) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "figuras_mboy_italia_18-3-92_27c020.bin",             0x00000, 0x40000, CRC(cecbde31) SHA1(5fbe5fbceb9864ede0cf9c593b66df1c9ff88822) ) // 99% gfx
	ROM_LOAD( "montajes_parlante_mboy_italia_20-ene-92_27c010.bin", 0x40000, 0x20000, CRC(439ad40a) SHA1(9235bb1c63f852a26a9467fec8c029a2974ef51b) ) // data
	ROM_RELOAD(               0x60000, 0x20000 ) // 128kb ROMs are mirrored

/* SPORT - GENERALE       011092 7515 1329 */
	ROM_LOAD( "15_domande-rom.ic92", 0x080000, 0x020000, CRC(45dd77e3) SHA1(856fbd1b7f888e1768abceb2465d5bb97a685332) )
	ROM_RELOAD(                      0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - STORIA       011092 6361 0808
   MUSICA - GENERALE      011092 6875 0719 */
	ROM_LOAD( "16_domande-rom.ic93", 0x0c0000, 0x020000, CRC(31ececb2) SHA1(a62b1ecdedf8c587afefef4a7d5cdc9746abb093) )
	ROM_RELOAD(                      0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - GEOGRAFIA    011092 6756 0565
   SPETTACOLOS - CINE-TV  011092 6171 0918 */
	ROM_LOAD( "17_domande-rom.ic94", 0x100000, 0x020000, CRC(bdce54df) SHA1(b30a3adcdeba26f91f7de8e174f54a158d173dba) )
	ROM_RELOAD(                      0x120000, 0x020000 ) // 128kb ROMs are mirrored

/* TEMPO LIBERO - CULTURA 011092 6913 0424
   SCIENZA - NATURA       011092 6969 0400
   TEMPO LIBERO - HOBBY   011092 6569 0300
   SPORT - WC_90          011092 6072 0212
   SCIENZA - SESSUOLOGIA  011092 6098 0276 */
	ROM_LOAD( "18_domande-rom.ic95", 0x140000, 0x020000, CRC(3ea4dd86) SHA1(6db92010ab6d6adbdf6bea9b257423bb7607c7f2) )
	ROM_RELOAD(                      0x160000, 0x020000 ) // 128kb ROMs are mirrored

/* SPETTACOLOS - FUMETTI  011092 6938 0496
   TEMPO LIBERO - PAROLE  011092 6075 0219 */
	ROM_LOAD( "19_domande-rom.ic96", 0x180000, 0x020000, CRC(146c46f9) SHA1(a6b09ffb98146ed2eb67a9d43465abc076758d60) )
	ROM_RELOAD(                      0x1a0000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( mastboyid )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mboy_programa_italia_18-3-92_27c512.bin",        0x0000, 0x4000, CRC(4137cf86) SHA1(1746b92a90f69fc9cf23c9ce59593d6a3254a17e) )
	ROM_IGNORE(                                                         0xc000 )
	ROM_LOAD( "mboy_tablas_sin_play_italia_18-3-92_27c512.bin", 0x4000, 0x4000, CRC(20450b48) SHA1(296999780b62e54687fff22e2acea94733a4ffea) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                                               0x4000, 0x4000 )
	ROM_CONTINUE(                                               0x4000, 0x4000 )
	ROM_CONTINUE(                                               0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "4.ic91", 0x00000, 0x10000, CRC(858d7b27) SHA1(b0ddf49df5665003f3616d67f7fc27408433483b) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "1-mem-c.ic75", 0x00000, 0x40000, CRC(7c7b1cc5) SHA1(73ad7bdb61d1f99ce09ef3a5a3ae0f1e72364eee) ) // 99% gfx
	ROM_LOAD( "2-mem-b.ic76", 0x40000, 0x20000, CRC(87015c18) SHA1(a16bf2707ce847da0923662796195b75719a6d77) ) // data
	ROM_RELOAD(               0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  TEMPO LIBERO - HOBBY GIOCHI 011091 5457 0300
    SCIENZA - NATURA            011091 5657 0400
    SPORT - MONDIALI-90         011091 5999 0212
    MUSICA - AUTORI CANZONI     011091 5496 0314 */
	ROM_LOAD( "5-rom.ic95",   0x080000, 0x020000, CRC(adc07f12) SHA1(2e0b46ac5884ad459bc354f56ff384ff1932f147) )
	ROM_RELOAD(               0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SPORT- GENERALE             011091 5294 0713
    SPETTACOLO - CINE-TV        011091 5657 0400 */
	ROM_LOAD( "6-rom.ic96",   0x0c0000, 0x020000, CRC(2c52cb1e) SHA1(d58f21c09bd3983497f74ab6c5a37977d9e30f0c) )
	ROM_RELOAD(               0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SCIENZA - STORIA            011091 5430 0375
    SCIENZA - GEOGRAFIA         011091 5010 0565 */
	ROM_LOAD( "7-rom.ic97",   0x100000, 0x020000, CRC(7818408f) SHA1(2a69688b6cda5baf2a45966dd86f10b2fcd54b66) )
	ROM_RELOAD(               0x120000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x140000 to 0x17ffff EMPTY
	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( mastboyiol )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_ia.bin", 0x0000, 0x4000, CRC(a0556932) SHA1(0fc3cd8ff497002f22fcb236c8b77ec840bf1bb3) ) // game code is internal to the CPU!
	ROM_LOAD( "3a_i.bin",        0x4000, 0x4000, CRC(b0c424f8) SHA1(b1f71298acd5b6fea4088a7c9db9f32cb06c54d6) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                0x4000, 0x4000 )
	ROM_CONTINUE(                0x4000, 0x4000 )
	ROM_CONTINUE(                0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "4a_i.bin", 0x00000, 0x10000, CRC(388beade) SHA1(2161ac884d5537293e2dd9786b3556bbc8ebdce6) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "1a_i.bin", 0x00000, 0x40000, CRC(67d07e14) SHA1(ce198facd05948010f104f52e30ce8103ec4fff9) ) // 99% gfx
	ROM_LOAD( "2a_i_bin", 0x40000, 0x20000, CRC(c8840f36) SHA1(38393988ef1ab440043aa9728fd8687ed67f3910) ) // data
	ROM_RELOAD(           0x60000, 0x20000 ) // 128kb ROMs are mirrored

/* SPORT - GENERALE       011092 7515 1329 */
	ROM_LOAD( "5a_i_preg_ita_0_18-oct-92.bin", 0x080000, 0x020000, CRC(45dd77e3) SHA1(856fbd1b7f888e1768abceb2465d5bb97a685332) )
	ROM_RELOAD(                                0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - STORIA       011092 6361 0808
   MUSICA - GENERALE      011092 6875 0719 */
	ROM_LOAD( "6a_i_preg_ita_1_18-oct-92.bin", 0x0c0000, 0x020000, CRC(31ececb2) SHA1(a62b1ecdedf8c587afefef4a7d5cdc9746abb093) )
	ROM_RELOAD(                                0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - GEOGRAFIA    011092 6756 0565
   SPETTACOLOS - CINE-TV  011092 6171 0918 */
	ROM_LOAD( "7a_i_preg_ita_2_18-oct-92.bin", 0x100000, 0x020000, CRC(bdce54df) SHA1(b30a3adcdeba26f91f7de8e174f54a158d173dba) )
	ROM_RELOAD(                                0x120000, 0x020000 ) // 128kb ROMs are mirrored

/* TEMPO LIBERO - CULTURA 011092 6913 0424
   SCIENZA - NATURA       011092 6969 0400
   TEMPO LIBERO - HOBBY   011092 6569 0300
   SPORT - WC_90          011092 6072 0212
   SCIENZA - SESSUOLOGIA  011092 6098 0276 */
	ROM_LOAD( "8a_i_preg_ita_3_18-oct-92.bin", 0x140000, 0x020000, CRC(3ea4dd86) SHA1(6db92010ab6d6adbdf6bea9b257423bb7607c7f2) )
	ROM_RELOAD(                                0x160000, 0x020000 ) // 128kb ROMs are mirrored

/* SPETTACOLOS - FUMETTI  011092 6938 0496
   TEMPO LIBERO - PAROLE  011092 6075 0219 */
	ROM_LOAD( "9a_i_preg_ita_4_18-oct-92.bin", 0x180000, 0x020000, CRC(146c46f9) SHA1(a6b09ffb98146ed2eb67a9d43465abc076758d60) )
	ROM_RELOAD(                                0x1a0000, 0x020000 ) // 128kb ROMs are mirrored

// Missing the Italian questions about the Olympic Games ??
	//ROM_LOAD( "olympic_questions_it.bin",      0x180000, 0x020000, NO_DUMP ) // questions about Olympic Games
	//ROM_RELOAD(                                0x1A0000, 0x020000 ) // 128kb ROMs are mirrored

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

// No internal ROM on the MCU, all the code is on the EPROM.
ROM_START( mastboyitst )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mb_test.bin", 0x0000, 0x8000, CRC(bd6f5c2e) SHA1(e5822f480c24f43c1f0a455174d4187e8db5a8a8) ) // data (1ST AND 2ND HALF IDENTICAL)
	ROM_CONTINUE( 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "4.ic91", 0x00000, 0x10000, CRC(858d7b27) SHA1(b0ddf49df5665003f3616d67f7fc27408433483b) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "1-mem-c.ic75", 0x00000, 0x40000, CRC(7c7b1cc5) SHA1(73ad7bdb61d1f99ce09ef3a5a3ae0f1e72364eee) ) // 99% gfx
	ROM_LOAD( "2-mem-b.ic76", 0x40000, 0x20000, CRC(87015c18) SHA1(a16bf2707ce847da0923662796195b75719a6d77) ) // data
	ROM_RELOAD(               0x60000, 0x20000 ) // 128kb ROMs are mirrored

/*  TEMPO LIBERO - HOBBY GIOCHI 011091 5457 0300
    SCIENZA - NATURA            011091 5657 0400
    SPORT - MONDIALI-90         011091 5999 0212
    MUSICA - AUTORI CANZONI     011091 5496 0314 */
	ROM_LOAD( "5-rom.ic95",   0x080000, 0x020000, CRC(adc07f12) SHA1(2e0b46ac5884ad459bc354f56ff384ff1932f147) )
	ROM_RELOAD(               0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SPORT- GENERALE             011091 5294 0713
    SPETTACOLO - CINE-TV        011091 5657 0400 */
	ROM_LOAD( "6-rom.ic96",   0x0c0000, 0x020000, CRC(2c52cb1e) SHA1(d58f21c09bd3983497f74ab6c5a37977d9e30f0c) )
	ROM_RELOAD(               0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/*  SCIENZA - STORIA            011091 5430 0375
    SCIENZA - GEOGRAFIA         011091 5010 0565 */
	ROM_LOAD( "7-rom.ic97",   0x100000, 0x020000, CRC(7818408f) SHA1(2a69688b6cda5baf2a45966dd86f10b2fcd54b66) )
	ROM_RELOAD(               0x120000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x140000 to 0x17ffff EMPTY
	//                  0x180000 to 0x1bffff EMPTY
	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

/* Gaelco had an internal version of this set with no internal ROM for the MCU, having the first 0x4000 bytes of the EPROM with actual code, and this hash:
   CRC(06465aa5) SHA1(c2958197cf5d0f36efb1654d9d0f7c660768f4d1) */
ROM_START( mastboyiv2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_it_v2.bin",                        0x0000, 0x4000, CRC(55a499cc) SHA1(3bd7a175de00f7b102b990bb09b040adf08520f6) ) // game code is internal to the CPU!
	ROM_LOAD( "mboy_3_italia_tablas_27-oct-92_27c512.bin", 0x4000, 0x4000, CRC(5fa0f502) SHA1(05d7464b2fec2705bc38b80b09e58ee5d08ee974) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                                          0x4000, 0x4000 )
	ROM_CONTINUE(                                          0x4000, 0x4000 )
	ROM_CONTINUE(                                          0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "14.ic91", 0x00000, 0x10000, CRC(388beade) SHA1(2161ac884d5537293e2dd9786b3556bbc8ebdce6) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "11_mem-c.ic75", 0x00000, 0x40000, CRC(4030846a) SHA1(8782bae43d506f8b9e584087d9325a88f7020f4f) ) // 99% gfx
	ROM_LOAD( "12_mem-b.ic76", 0x40000, 0x20000, CRC(a38293eb) SHA1(07be32df7dd689b6262f0da2e4e0500bf29d4428) ) // data
	ROM_RELOAD(                0x60000, 0x20000 ) // 128kb ROMs are mirrored

/* SPORT - GENERALE       011092 7515 1329 */
	ROM_LOAD( "15_domande-rom.ic92", 0x080000, 0x020000, CRC(45dd77e3) SHA1(856fbd1b7f888e1768abceb2465d5bb97a685332) )
	ROM_RELOAD(                      0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - STORIA       011092 6361 0808
   MUSICA - GENERALE      011092 6875 0719 */
	ROM_LOAD( "16_domande-rom.ic93", 0x0c0000, 0x020000, CRC(31ececb2) SHA1(a62b1ecdedf8c587afefef4a7d5cdc9746abb093) )
	ROM_RELOAD(                      0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - GEOGRAFIA    011092 6756 0565
   SPETTACOLOS - CINE-TV  011092 6171 0918 */
	ROM_LOAD( "17_domande-rom.ic94", 0x100000, 0x020000, CRC(bdce54df) SHA1(b30a3adcdeba26f91f7de8e174f54a158d173dba) )
	ROM_RELOAD(                      0x120000, 0x020000 ) // 128kb ROMs are mirrored

/* TEMPO LIBERO - CULTURA 011092 6913 0424
   SCIENZA - NATURA       011092 6969 0400
   TEMPO LIBERO - HOBBY   011092 6569 0300
   SPORT - WC_90          011092 6072 0212
   SCIENZA - SESSUOLOGIA  011092 6098 0276 */
	ROM_LOAD( "18_domande-rom.ic95", 0x140000, 0x020000, CRC(3ea4dd86) SHA1(6db92010ab6d6adbdf6bea9b257423bb7607c7f2) )
	ROM_RELOAD(                      0x160000, 0x020000 ) // 128kb ROMs are mirrored

/* SPETTACOLOS - FUMETTI  011092 6938 0496
   TEMPO LIBERO - PAROLE  011092 6075 0219 */
	ROM_LOAD( "19_domande-rom.ic96", 0x180000, 0x020000, CRC(146c46f9) SHA1(a6b09ffb98146ed2eb67a9d43465abc076758d60) )
	ROM_RELOAD(                      0x1a0000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END

// PCB dated 03/02/92
ROM_START( mastboyiv2a)
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hd647180_it_v2.bin", 0x0000, 0x4000, CRC(55a499cc) SHA1(3bd7a175de00f7b102b990bb09b040adf08520f6) ) // game code is internal to the CPU!
	ROM_LOAD( "13_mem-a.ic77",      0x4000, 0x4000, CRC(06465aa5) SHA1(c2958197cf5d0f36efb1654d9d0f7c660768f4d1) ) // sound data? (+ 1 piece of) 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                   0x4000, 0x4000 )
	ROM_CONTINUE(                   0x4000, 0x4000 )
	ROM_CONTINUE(                   0x4000, 0x4000 ) // only the last 16kb matters

	ROM_REGION( 0x10000, "vrom", ROMREGION_INVERT ) // ROM accessed by the video chip
	ROM_LOAD( "14.ic91", 0x00000, 0x10000, CRC(388beade) SHA1(2161ac884d5537293e2dd9786b3556bbc8ebdce6) )

	ROM_REGION( 0x200000, "bankedrom", 0 ) // question data - 6 sockets
	ROM_LOAD( "11_mem-c.ic75", 0x00000, 0x40000, CRC(4030846a) SHA1(8782bae43d506f8b9e584087d9325a88f7020f4f) ) // 99% gfx
	ROM_LOAD( "12_mem-b.ic76", 0x40000, 0x20000, CRC(a38293eb) SHA1(07be32df7dd689b6262f0da2e4e0500bf29d4428) ) // data
	ROM_RELOAD(                0x60000, 0x20000 ) // 128kb ROMs are mirrored

/* SPORT - GENERALE       011092 7515 1329 */
	ROM_LOAD( "15_domande-rom.ic92", 0x080000, 0x020000, CRC(45dd77e3) SHA1(856fbd1b7f888e1768abceb2465d5bb97a685332) )
	ROM_RELOAD(                      0x0a0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - STORIA       011092 6361 0808
   MUSICA - GENERALE      011092 6875 0719 */
	ROM_LOAD( "16_domande-rom.ic93", 0x0c0000, 0x020000, CRC(31ececb2) SHA1(a62b1ecdedf8c587afefef4a7d5cdc9746abb093) )
	ROM_RELOAD(                      0x0e0000, 0x020000 ) // 128kb ROMs are mirrored

/* SCIENZA - GEOGRAFIA    011092 6756 0565
   SPETTACOLOS - CINE-TV  011092 6171 0918 */
	ROM_LOAD( "17_domande-rom.ic94", 0x100000, 0x020000, CRC(bdce54df) SHA1(b30a3adcdeba26f91f7de8e174f54a158d173dba) )
	ROM_RELOAD(                      0x120000, 0x020000 ) // 128kb ROMs are mirrored

/* TEMPO LIBERO - CULTURA 011092 6913 0424
   SCIENZA - NATURA       011092 6969 0400
   TEMPO LIBERO - HOBBY   011092 6569 0300
   SPORT - WC_90          011092 6072 0212
   SCIENZA - SESSUOLOGIA  011092 6098 0276 */
	ROM_LOAD( "18_domande-rom.ic95", 0x140000, 0x020000, CRC(3ea4dd86) SHA1(6db92010ab6d6adbdf6bea9b257423bb7607c7f2) )
	ROM_RELOAD(                      0x160000, 0x020000 ) // 128kb ROMs are mirrored

/* SPETTACOLOS - FUMETTI  011092 6938 0496
   TEMPO LIBERO - PAROLE  011092 6075 0219 */
	ROM_LOAD( "19_domande-rom.ic96", 0x180000, 0x020000, CRC(146c46f9) SHA1(a6b09ffb98146ed2eb67a9d43465abc076758d60) )
	ROM_RELOAD(                      0x1a0000, 0x020000 ) // 128kb ROMs are mirrored

	//                  0x1c0000 to 0x1fffff EMPTY

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8-25.ic32", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic49", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8-25.ic84", 0x000, 0x117, NO_DUMP )
ROM_END


} // anonymous namespace

// Every known PCB has the MCU sub-PCB silkscreened as "REV- A". The dates on the comments are from the program ROM sticker.

GAME( 1991, mastboy,     0,       mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy (Spain, set 1, checksum E49B)", MACHINE_SUPPORTS_SAVE ) // 27-Nov-1992
GAME( 1991, mastboya,    mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy (Spain, set 2, checksum E5AD)", MACHINE_SUPPORTS_SAVE ) // No text on license box at boot
GAME( 1991, mastboyb,    mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy (Spain, set 3, checksum 6070)", MACHINE_SUPPORTS_SAVE ) // 27-Nov-1992
GAME( 1991, mastboyc,    mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy (Spain, set 4, checksum CC6D)", MACHINE_SUPPORTS_SAVE ) // 23-Oct-1992
GAME( 1991, mastboyd,    mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy (Spain, set 5, checksum DF11)", MACHINE_SUPPORTS_SAVE ) // 19-Feb-1992

GAME( 1992, mastboyol,   mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy Olympic (Spain, set 1, checksum 7309)", MACHINE_SUPPORTS_SAVE ) // 18-Oct-1992
GAME( 1992, mastboyola,  mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy Olympic (Spain, set 2, checksum 741B)", MACHINE_SUPPORTS_SAVE ) // 18-Oct-1992. No text on license box at boot

GAME( 1991, mastboyv2,   mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy Version II (Spain, set 1, checksum 5A0B)", MACHINE_SUPPORTS_SAVE ) // 17-Oct-1992
GAME( 1991, mastboyv2a,  mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy Version II (Spain, set 2, checksum 5B1D)", MACHINE_SUPPORTS_SAVE ) // 17-Oct-1992. No text on license box at boot

// There were specific Gaelco Master Boy PCBs for the Italian market, silkcreened in Italian instead of in Spanish ("DOMANDE ROMS", "MASTER-BOY VERSIONE", etc.).

GAME( 1991, mastboyi,    mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy (Italy, set 1, checksum E7C5)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mastboyia,   mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy (Italy, set 2, checksum E7C5)", MACHINE_SUPPORTS_SAVE ) // Same checksum as 'mastboyi', only the questions are different
GAME( 1991, mastboyib,   mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy (Italy, set 3, checksum 6918)", MACHINE_SUPPORTS_SAVE ) // 27-Oct-1992
GAME( 1991, mastboyic,   mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy (Italy, set 4, checksum 680C)", MACHINE_SUPPORTS_SAVE ) // 27-Oct-1992
GAME( 1991, mastboyid,   mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy (Italy, set 4, checksum E84C)", MACHINE_SUPPORTS_SAVE ) // 18-Mar-1992

GAME( 1991, mastboyiol,  mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy Olympic (Italy, set 1, checksum 77A8)", MACHINE_SUPPORTS_SAVE ) // 18-Oct-1992

GAME( 1991, mastboyitst, mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco", "Master Boy (Italy, test version)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, mastboyiv2,  mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy Version II (Italy, set 1, checksum 618F)", MACHINE_SUPPORTS_SAVE ) // 27-Oct-1992
GAME( 1992, mastboyiv2a, mastboy, mastboy, mastboy, mastboy_state, empty_init, ROT0, "Gaelco (Playmark license)", "Master Boy Version II (Italy, set 2, checksum 629B)", MACHINE_SUPPORTS_SAVE ) // 27-Oct-1992. No text on license box at boot
