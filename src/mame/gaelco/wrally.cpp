// license:BSD-3-Clause
// copyright-holders: Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna

/***************************************************************************

World Rally (c) 1993 Gaelco (Designed & Developed by Zigurat. Produced by Gaelco)

Driver by Manuel Abadia, Mike Coates, Nicola Salmoria and Miguel Angel Horna

Thanks to GAELCO SA for the DS5002FP code and information about the encryption

REF.930217  (also REF.930705)
+-------------------------------------------------+
|       C1                                  6116  |
|  VOL  C2                                  6116  |
|          30MHz                            6116  |
|    M6295                    +----------+  6116  |
|     1MHz                    |TMS       |        |
|      MS6264A                |TPC1020AFN|        |
|J     MS6264A                |   -084C  |    H8  |
|A     +------------+         +----------+        |
|M     |DS5002FP Box|         +----------+        |
|M     +------------+         |TMS       |    H12 |
|A            MS6264A         |TPC1020AFN|        |
|             MS6264A         |   -084C  |        |
|                             +----------+        |
|SW1                                  PAL  MS6264A|
|     24MHz    MC68000P12                  MS6264A|
|SW2           C22                    6116        |
|      PAL     C23                    6116        |
+-------------------------------------------------+

Main PCB components:
====================

CPUs related:
=============
* 1xDS5002FP @ D12 (Dallas security processor @ 12 MHz)
* 1xHM62256ALFP-8T (32KB NVSRAM) @ C11 (encrypted DS5002FP program code)
* 1xLithium cell
* 2xMS6264A-20NC (8KB SRAM) @ D14 & D15 (shared memory between M68000 & DS5002FP)
* 4x74LS157 (Quad 2 input multiplexer) @ F14, F15, F16 & F17 (used to select M68000 or DS5002FP address bus)
* 4x74LS245 (Octal bus transceiver) @ C14, C15, C16 & C17 (used to store shared RAM data)
* 2x74LS373 (Octal tristate latch) @ D16 & D17 (used by DS5002FP to access data from shared RAM)
* 1xMC68000P12 @ C20 (Motorola 68000 @ 12 MHz)
* 1xOSC24MHz @ B20
* 2xM27C4001 @ C22 & C23 (M68000 program ROMs)
* 1xPAL20L8 @ B23 (handles 1st level M68000 memory map)
    0 -> DTACK  M68000 data ack
    1 -> SELACT
    2 -> Input/sound (see below)
    3 -> ACTEXT
    4 -> SELMOV
    5 -> CSW
    6 -> CSR
    7 -> EXT

* 1x74LS138 (3 to 8 line decoder) @ B13 (handles 2nd level M68000 memory map)
    0 -> IN0    DIPSW #1 & #2
    1 -> IN1    Joystick 1P & 2P, COINSW, STARTSW
    2 -> IN2    Wheel input
    3 -> -
    4 -> IN4    TESTSW & SERVICESW
    5 -> OUT (see below)
    6 -> CSBAN  OKIM6295 bankswitch
    7 -> CSSON  OKIM6295 R/W

* 1x74LS259 (8 bit addressable latches) @A7 (handles 3rd level M68000 memory map)
    0 -> Coin lockout 1
    1 -> Coin lockout 2
    2 -> Coin counter 1
    3 -> Coin counter 2
    4 -> Sound muting
    5 -> flip screen
    6 -> ENA/D?
    7 -> CKA/D?

* 1x16AS @ B15
    0 -> OE
    1 -> XSRL   Shared RAM @ D14
    2 -> XSRH   Shared RAM @ D15
    3 -> SAD    Shared Access with DS5002FP
    4 -> SRE    Shared Access with M68000
    5 -> TRANS
    6 -> XLD
    7 -> XHI

Sound related:
==============
* 1xOKIM6295 @ C6
* 1xOSC1MHz @ C7
* 2xM27C4001 @ C1 & C3 (OKI ADPCM samples)
* 1xPAL16R4 @ E2 (handles OKI ROM banking)

Graphics related:
=================
* 1xOSC30MHz @ D5
* 2xTPC1020AFN-84C (FPGA) @ G8 & G13 (GFX processing)
* 2xMS6264A-20NC (8KB SRAM) @ I16 & I17 (Video RAM)
* 4xUM6116BK-25 (2KB SRAM) @ H1, H2, H4 & H5
* 2xUM6116BK-25 (2KB SRAM) @ H22 & H23

Palette related:
================
* 2xMS6264A-20NC (8KB SRAM) @ C8 & C9 (palette RAM (xxxxBBBBRRRRGGGG))
* 2x74HCT273 (octal D-Type flip-flop with clear) @ B8 & B9 (connected to RGB output)

Controls related: (added by Mirko Mattioli)
=================
When optical wheel is selected (via dipswitch), then gear shift (low/high) is enabled.
On the real PCB the optical wheel encoder is connected to 74LS169 ICs (@A16 and @A17)
via a flip-flop IC mounted in the steering wheel assembly. As a result, the output
of the flip-flop generates a signal that contains the information about the steering
direction; this signal is routed to pin #1 (U/D) at ICs A16 and A17 (high when turn
left and low when turn right). The second signal of the optical encoder goes directly
to pin #2 (CLK) at ICs A16 and A17 and it is a clock for the 74LS169 ICs; this clock
frequency is proportional to the movements of the steering wheel: fast movements
produces a high clock frequency, slow movements a low freq.

PCB: REF.930217

The PCB has a layout that can either use the 4 ROM set of I7, I9, I11 & I13 or larger
 ROMs at H8 & H12 for graphics as well as the ability to use different size sound sample
 ROMs at C1 & C3

***************************************************************************/

#include "emu.h"

#include "gaelco_ds5002fp.h"
#include "gaelco_wrally_sprites.h"
#include "gaelcrpt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74259.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wrally_state : public driver_device
{
public:
	wrally_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprites(*this, "sprites"),
		m_okibank(*this, "okibank"),
		m_vramcrypt(*this, "vramcrypt"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_analog(*this, "ANALOG%u", 0U),
		m_tilemap{ nullptr, nullptr }
	{
	}

	void wrally(machine_config &config);

	template <int N> int analog_bit_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t shareram_r(offs_t offset);
	void shareram_w(offs_t offset, uint8_t data);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void okim6295_bankswitch_w(uint8_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	template <uint8_t Which> void coin_lockout_w(int state);

	void adc_clk(int state);
	void adc_en(int state);

	template <int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_hostmem_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<gaelco_wrally_sprites_device> m_sprites;
	required_memory_bank m_okibank;
	required_device<gaelco_vram_encryption_device> m_vramcrypt;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;

	required_ioport_array<2> m_analog;

	tilemap_t *m_tilemap[2]{};
	uint8_t m_analog_ports[2]{};
};


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (64*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | --xxxxxx xxxxxxxx | code
      0  | xx------ -------- | not used?
      1  | -------- ---xxxxx | color
      1  | -------- --x----- | priority
      1  | -------- -x------ | flip y
      1  | -------- x------- | flip x
      1  | ---xxxxx -------- | data used to handle collisions, speed, etc
      1  | xxx----- -------- | not used?
*/

template <int Layer>
TILE_GET_INFO_MEMBER(wrally_state::get_tile_info)
{
	int const data = m_videoram[(Layer * 0x2000 / 2) + (tile_index << 1)];
	int const data2 = m_videoram[(Layer * 0x2000 / 2) + (tile_index << 1) + 1];
	int const code = data & 0x3fff;

	tileinfo.category = (data2 >> 5) & 0x01;

	tileinfo.set(0, code, data2 & 0x1f, TILE_FLIPYX((data2 >> 6) & 0x03));
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void wrally_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wrally_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wrally_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);

	m_tilemap[0]->set_transmask(0, 0xff01, 0x00ff); // this layer is split in two (pens 1..7, pens 8-15)
	m_tilemap[1]->set_transparent_pen(0);
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t wrally_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprites->draw_sprites(cliprect, m_spriteram, flip_screen());

	// set scroll registers
	if (!flip_screen()) {
		m_tilemap[0]->set_scrolly(0, m_vregs[0]);
		m_tilemap[0]->set_scrollx(0, m_vregs[1] + 4);
		m_tilemap[1]->set_scrolly(0, m_vregs[2]);
		m_tilemap[1]->set_scrollx(0, m_vregs[3]);
	} else {
		m_tilemap[0]->set_scrolly(0, 248 - m_vregs[0]);
		m_tilemap[0]->set_scrollx(0, 1024 - m_vregs[1] - 4);
		m_tilemap[1]->set_scrolly(0, 248 - m_vregs[2]);
		m_tilemap[1]->set_scrollx(0, 1024 - m_vregs[3]);
	}

	// draw tilemaps + sprites
	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER0,0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER1,0);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER0,0);

	m_sprites->mix_sprites(bitmap, cliprect, 0);

	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER1,0);

	m_sprites->mix_sprites(bitmap, cliprect, 1);

	return 0;
}


void wrally_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);

	save_item(NAME(m_analog_ports));
}

/***************************************************************************

    World Rally memory handlers

***************************************************************************/

void wrally_state::shareram_w(offs_t offset, uint8_t data)
{
	// why isn't there address map functionality for this?
	util::big_endian_cast<uint8_t>(m_shareram.target())[offset] = data;
}

uint8_t wrally_state::shareram_r(offs_t offset)
{
	// why isn't there address map functionality for this?
	return util::big_endian_cast<uint8_t const>(m_shareram.target())[offset];
}

void wrally_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = m_vramcrypt->gaelco_decrypt(*m_maincpu, offset, data);
	COMBINE_DATA(&m_videoram[offset]);

	m_tilemap[(offset & 0x1fff) >> 12]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);
}

void wrally_state::okim6295_bankswitch_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}

template <uint8_t Which>
void wrally_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

template <uint8_t Which>
void wrally_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(Which, !state);
}

// the following methods have been pilfered from gaelco2.cpp (wrally2). They seem to work fine for wrally, too
template <int N>
int wrally_state::analog_bit_r()
{
	return (m_analog_ports[N] >> 7) & 0x01;
}

void wrally_state::adc_clk(int state)
{
	// a zero/one combo is written here to clock the next analog port bit
	if (!state)
	{
		m_analog_ports[0] <<= 1;
		m_analog_ports[1] <<= 1;
	}
}

void wrally_state::adc_en(int state)
{
	// a zero is written here to read the analog ports, and a one is written when finished
	if (!state)
	{
		m_analog_ports[0] = m_analog[0]->read();
		m_analog_ports[1] = m_analog[1]->read();
	}
}


void wrally_state::mcu_hostmem_map(address_map &map)
{
	map(0x0000, 0xffff).mask(0x3fff).rw(FUNC(wrally_state::shareram_r), FUNC(wrally_state::shareram_w)); // shared RAM with the main CPU
}


void wrally_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram().w(FUNC(wrally_state::vram_w)).share(m_videoram);                // Encrypted
	map(0x108000, 0x108007).ram().share(m_vregs);
	map(0x10800c, 0x10800d).nopw();                                                               // CLR INT Video
	map(0x200000, 0x203fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x440000, 0x440fff).ram().share(m_spriteram);
	map(0x700000, 0x700001).portr("DSW");
	map(0x700002, 0x700003).portr("P1_P2");
	map(0x700004, 0x700005).portr("WHEEL");
	map(0x700008, 0x700009).portr("SYSTEM");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(wrally_state::okim6295_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x70006a, 0x70006b).nopr();
	map(0x70007a, 0x70007b).nopr();
	map(0xfec000, 0xfeffff).ram().share(m_shareram);                                              // Work RAM (shared with DS5002FP)
}


void wrally_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( wrally )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Number of Joysticks" )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0018, 0x0018, "Control Configuration" )    PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0018, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0010, "Pot Wheel" )
	PORT_DIPSETTING(      0x0000, "Optical Wheel" )
	PORT_DIPSETTING(      0x0008, "invalid" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) )         PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )          PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )          PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Credit configuration" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x4000, "Start 1C/Continue 1C" )
	PORT_DIPSETTING(      0x0000, "Start 2C/Continue 1C" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Gear Shift") PORT_TOGGLE PORT_REVERSE
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0018)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("WHEEL")
	PORT_BIT( 0xff00, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_RIGHT) PORT_CODE_INC(KEYCODE_LEFT) PORT_REVERSE PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0000)
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")   // pot wheel player 1
	PORT_BIT( 0xff, 0x8A, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("P1 Pot Wheel") PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0010)

	PORT_START("ANALOG1")   // pot wheel player 2
	PORT_BIT( 0xff, 0x8A, IPT_PADDLE_V ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("P2 Pot Wheel") PORT_CONDITION("DSW", 0x0018, EQUALS, 0x0010)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 ) // Go to test mode NOW
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(wrally_state, analog_bit_r<0>)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(wrally_state, analog_bit_r<1>)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout wrally_tilelayout16 =
{
	16,16,                                  // 16x16 tiles
	RGN_FRAC(1,2),                          // number of tiles
	4,                                      // 4 bpp
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ STEP8(0,1), STEP8(16*16,1) },
	{ STEP16(0,16) },
	64*8
};

static GFXDECODE_START( gfx_wrally )
	GFXDECODE_ENTRY( "gfx", 0x000000, wrally_tilelayout16, 0, 64*8 )
GFXDECODE_END


void wrally_state::wrally(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);        // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &wrally_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wrally_state::irq6_line_hold));

	gaelco_ds5002fp_device &ds5002(GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2)); // verified on PCB
	ds5002.set_addrmap(0, &wrally_state::mcu_hostmem_map);
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");

	GAELCO_VRAM_ENCRYPTION(config, m_vramcrypt);
	m_vramcrypt->set_params(0x1f, 0x522a);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // Not accurate
	screen.set_size(64*16, 32*16);
	screen.set_visarea(8, 24*16-8-1, 16, 16*16-8-1);
	screen.set_screen_update(FUNC(wrally_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wrally);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024*8);

	GAELCO_WRALLY_SPRITES(config, m_sprites, 0);
	m_sprites->set_gfxdecode_tag("gfxdecode");
	m_sprites->set_screen_tag("screen");

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(wrally_state::coin_lockout_w<0>));
	m_outlatch->q_out_cb<1>().set(FUNC(wrally_state::coin_lockout_w<1>));
	m_outlatch->q_out_cb<2>().set(FUNC(wrally_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(wrally_state::coin_counter_w<1>));
	m_outlatch->q_out_cb<4>().set_nop();                                // Sound muting
	m_outlatch->q_out_cb<5>().set(FUNC(wrally_state::flip_screen_set));
	m_outlatch->q_out_cb<6>().set(FUNC(wrally_state::adc_en));  // ENA/D, for pot wheel
	m_outlatch->q_out_cb<7>().set(FUNC(wrally_state::adc_clk)); // CKA/D,      "

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // verified on PCB
	oki.set_addrmap(0, &wrally_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( wrally )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "worldr17.c23", 0x000000, 0x080000, CRC(050f5629) SHA1(74fc2cd5114f3bc4b2429f1d8d7eeb1658f9f179) ) // Only difference compared to set 2 is how the Dallas DS5002FP
	ROM_LOAD16_BYTE( "worldr16.c22", 0x000001, 0x080000, CRC(9e0d126c) SHA1(369360b7ec2c3497af3bf62b4eba24c3d9f94675) ) // Power failure shows on screen, IE: "Tension  baja"

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x88 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD16_BYTE( "worldr21.i13", 0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "worldr20.i11", 0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "worldr19.i09", 0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "worldr18.i07", 0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "worldr14.c01",   0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "worldr15.c03",   0x080000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, "plds", 0 ) // PALs and GALs
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x02e5, BAD_DUMP CRC(a1c780ed) SHA1(91dc230d94c992c4c4516554c0faeced41e1e34e) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x02e5, BAD_DUMP CRC(a39efdc6) SHA1(bf86f764665531639076dfcc72583457f1cbf806) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallya )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x080000, CRC(8b7d93c3) SHA1(ce4163eebc5d4a0c1266d650523b1ffc702d1b87) ) // Only difference compared to set 1 is how the Dallas DS5002FP
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x080000, CRC(56da43b6) SHA1(02db8f969ed5e7f5e5356c45c0312faf5f000335) ) // power failure shows on screen, IE: "Power Failure"

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x88 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD16_BYTE( "worldr21.i13", 0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "worldr20.i11", 0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "worldr19.i09", 0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "worldr18.i07", 0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "worldr14.c01",   0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "worldr15.c03",   0x080000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, "plds", 0 ) // PALs and GALs
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x02e5, BAD_DUMP CRC(a1c780ed) SHA1(91dc230d94c992c4c4516554c0faeced41e1e34e) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x02e5, BAD_DUMP CRC(a39efdc6) SHA1(bf86f764665531639076dfcc72583457f1cbf806) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallyb )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "rally_c23.c23", 0x000000, 0x080000, CRC(ddd6f833) SHA1(f12f82c412fa93f46020d50c2620974ae2fb502b) )
	ROM_LOAD16_BYTE( "rally_c22.c22", 0x000001, 0x080000, CRC(59a0d35c) SHA1(7c6f376a53c1e6d793cbfb16861ee3298ee013a1) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x88 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "rally h-12.h12", 0x000000, 0x100000, CRC(3353dc00) SHA1(db3b1686751dcaa231d66c08b5be81fcfe299ad9) ) // Same data, different layout
	ROM_LOAD( "rally h-8.h8",   0x100000, 0x100000, CRC(58dcd024) SHA1(384ff296d3c7c8e0c4469231d1940de3cea89fc2) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "sound c-1.c1", 0x000000, 0x100000, CRC(2d69c9b8) SHA1(328cb3c928dc6921c0c3f0277f59bca6c747c504) ) // Same data in a single ROM

	ROM_REGION( 0x0514, "plds", 0 ) // PALs and GALs
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x02e5, BAD_DUMP CRC(a1c780ed) SHA1(91dc230d94c992c4c4516554c0faeced41e1e34e) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x02e5, BAD_DUMP CRC(a39efdc6) SHA1(bf86f764665531639076dfcc72583457f1cbf806) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallyc )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "rally c23.c23", 0x000000, 0x080000, CRC(fbd57c94) SHA1(05036f076e6d8e765c04515e9d822c6006c1a378) )
	ROM_LOAD16_BYTE( "rally c22.c22", 0x000001, 0x080000, CRC(db73e0af) SHA1(6c1a6ee3d5dda76c3491159087ab9f7d49fa7dad) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x88 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD16_BYTE( "rally i13.i13", 0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "rally i11.i11", 0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "rally i9.i9",   0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "rally i7.i7",   0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "rally c1.c1",   0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "rally c3.c3",   0x080000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, "plds", 0 ) // PALs and GALs
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x02e5, BAD_DUMP CRC(a1c780ed) SHA1(91dc230d94c992c4c4516554c0faeced41e1e34e) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x02e5, BAD_DUMP CRC(a39efdc6) SHA1(bf86f764665531639076dfcc72583457f1cbf806) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallyat ) // Board Marked 930217, Atari License
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "rally.c23", 0x000000, 0x080000, CRC(366595ad) SHA1(e16341ed9eacf9b729c28184268150ea9b62f185) ) // North & South America only...
	ROM_LOAD16_BYTE( "rally.c22", 0x000001, 0x080000, CRC(0ad4ec6f) SHA1(991557cf25fe960b1c586e990e6019befe5a11d0) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "wrdallas.bin", 0x00000, 0x8000, CRC(547d1768) SHA1(c58d1edd072d796be0663fb265f4739ec006b688) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x88 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "rally h-12.h12", 0x000000, 0x100000, CRC(3353dc00) SHA1(db3b1686751dcaa231d66c08b5be81fcfe299ad9) ) // Same data, different layout
	ROM_LOAD( "rally h-8.h8",   0x100000, 0x100000, CRC(58dcd024) SHA1(384ff296d3c7c8e0c4469231d1940de3cea89fc2) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "sound c-1.c1", 0x000000, 0x100000, CRC(2d69c9b8) SHA1(328cb3c928dc6921c0c3f0277f59bca6c747c504) ) // Same data in a single ROM

	ROM_REGION( 0x0514, "plds", 0 ) // PALs and GALs
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x02e5, BAD_DUMP CRC(a1c780ed) SHA1(91dc230d94c992c4c4516554c0faeced41e1e34e) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x02e5, BAD_DUMP CRC(a39efdc6) SHA1(bf86f764665531639076dfcc72583457f1cbf806) ) // Bruteforced but verified (as GAL22V10)
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

} // anonymous namespace


GAME( 1993, wrally,   0,      wrally, wrally, wrally_state, empty_init, ROT0, "Gaelco",                 "World Rally (Version 1.0, Checksum 0E56)", MACHINE_SUPPORTS_SAVE ) // Dallas DS5002FP power failure shows as: "Tension baja"
GAME( 1993, wrallya,  wrally, wrally, wrally, wrally_state, empty_init, ROT0, "Gaelco",                 "World Rally (Version 1.0, Checksum 3873)", MACHINE_SUPPORTS_SAVE ) // Dallas DS5002FP power failure shows as: "Power Failure"
GAME( 1993, wrallyb,  wrally, wrally, wrally, wrally_state, empty_init, ROT0, "Gaelco",                 "World Rally (Version 1.0, Checksum 8AA2)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, wrallyc,  wrally, wrally, wrally, wrally_state, empty_init, ROT0, "Gaelco",                 "World Rally (Version 1.0, Checksum E586)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, wrallyat, wrally, wrally, wrally, wrally_state, empty_init, ROT0, "Gaelco (Atari license)", "World Rally (US, 930217)",                 MACHINE_SUPPORTS_SAVE )
