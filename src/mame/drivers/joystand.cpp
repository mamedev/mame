// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Joy Stand Private
Yuvo 1999 (31st October, 1999)

driver by Luca Elia

This is a sticker machine with camera, printer and light pen.

PCB Layout
----------

Main board

JSP-NM515
|---------------------------------------------------------|
|TA8201A     POWER-IN     TD62083  TD62083     CONTROL    |
|     JRC3404             D71055   D71055            T531A|
|SP   JRC3404                     CN10                    |
|VR2  JRC3404      JSP001 JSP-FRM         JSP003B TC551001|
|VR1            M6295     JSP-MAP                         |
|CAM-IN JRC2235    JSP002  C46M1 TMP68301 JSP004B TC551001|
|PRN-IN JRC2233 YM2413                                    |
|VR3    JRC3404                 JSP-XCT                   |
|VR4                            16MHz            32.767kHz|
|VR5  SONY_A1585Q       M548262 M548262             M6242B|
|     3.579545MHz       M548262 M548262            3V_BATT|
|       JRC2240  D6951  M548262 M548262                   |
|RGB-OUT         D6951  M548262 M548262               JACK|
|                D6951                  XC3042A           |
|       SONY_CXA1645M           KM68257 KM68257   JRC2903 |
|SV-OUT          D6901                            LIGHTPEN|
|       JRC2244  D6901  XC3030A  XC3030A   XC3030A        |
|       JRC2244  D6901            CN11            PRN_CONT|
|---------------------------------------------------------|
Notes:
      Main CPU is Toshiba TMP68301 @ 16MHz
      No custom chips, using only Xilinx XC30xx FPGA
      Sound Oki M6295 @ 1MHz [16/16], YM2413 @ 3.579545MHz
      PALs type PALCE16V8H
      EPROMs are 27C040/27C020
      CN10/11 - Connector for sub-board
      Many other connectors for camera, printer, lightpen etc.


Sub board

JSP-NS515
MODEL-NP001
9.10.31
|-------------------------------|
|                J2             |
|                               |
|      JSP-SUB                  |
|                               |
|                  |-|      |-| |
|JSP005 JSP007A    | |      | | |
|                  | |      | | |
|                  | |      | | |
|                  | |      | | |
|JSP006 JSP008A    | |      | | |
|                  | |      | | |
|                  |-|      |-| |
|                J1             |
|-------------------------------|


Cart board

NS514-F040DD
|---------------------|
|  U1 U2 U3 U4 U5 U6  |
|                     |
| U7 U8 U9 U10 U11 U12|
|                     |
-|                  |-|
 |------------------|
 Notes:
       U* - TMS29F040 (TSOP32, x12)


Notes:

- To unlock some hidden items in test mode, go in the option menu and move:
  left, right, left, left

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "machine/tmp68301.h"
#include "sound/okim6295.h"
#include "sound/ym2413.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

class joystand_state : public driver_device
{
public:
	joystand_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_bg15_palette(*this, "bg15_palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_eeprom(*this, "eeprom"),
		m_cart_u1(*this, "cart.u1"),
		m_cart_u2(*this, "cart.u2"),
		m_cart_u3(*this, "cart.u3"),
		m_cart_u4(*this, "cart.u4"),
		m_cart_u5(*this, "cart.u5"),
		m_cart_u6(*this, "cart.u6"),
		m_cart_u7(*this, "cart.u7"),
		m_cart_u8(*this, "cart.u8"),
		m_cart_u9(*this, "cart.u9"),
		m_cart_u10(*this, "cart.u10"),
		m_cart_u11(*this, "cart.u11"),
		m_cart_u12(*this, "cart.u12"),
		m_oki(*this, "oki"),
		m_bg1_ram(*this, "bg1_ram"),
		m_bg2_ram(*this, "bg2_ram"),
		m_bg15_0_ram(*this, "bg15_0_ram"),
		m_bg15_1_ram(*this, "bg15_1_ram"),
		m_scroll(*this, "scroll"),
		m_enable(*this, "enable"),
		m_outputs(*this, "outputs")
	{ }

	void joystand(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// devices
	required_device<tmp68301_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_bg15_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<intelfsh8_device> m_cart_u1;
	required_device<intelfsh8_device> m_cart_u2;
	required_device<intelfsh8_device> m_cart_u3;
	required_device<intelfsh8_device> m_cart_u4;
	required_device<intelfsh8_device> m_cart_u5;
	required_device<intelfsh8_device> m_cart_u6;
	required_device<intelfsh8_device> m_cart_u7;
	required_device<intelfsh8_device> m_cart_u8;
	required_device<intelfsh8_device> m_cart_u9;
	required_device<intelfsh8_device> m_cart_u10;
	required_device<intelfsh8_device> m_cart_u11;
	required_device<intelfsh8_device> m_cart_u12;
	intelfsh8_device *m_cart_flash[12];
	required_device<okim6295_device> m_oki;

	// memory pointers
	required_shared_ptr<uint16_t> m_bg1_ram;
	required_shared_ptr<uint16_t> m_bg2_ram;
	required_shared_ptr<uint16_t> m_bg15_0_ram;
	required_shared_ptr<uint16_t> m_bg15_1_ram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_enable;
	required_shared_ptr<uint16_t> m_outputs;

	// tilemaps
	tilemap_t *m_bg1_tmap;
	tilemap_t *m_bg2_tmap;
	DECLARE_WRITE16_MEMBER(bg1_w);
	DECLARE_WRITE16_MEMBER(bg2_w);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);

	// r5g5b5 layers
	bitmap_rgb32 m_bg15_bitmap[2];
	DECLARE_WRITE16_MEMBER(bg15_0_w);
	DECLARE_WRITE16_MEMBER(bg15_1_w);
	static const rgb_t BG15_TRANSPARENT;
	void draw_bg15_tile(address_space &space, int x, int y, uint16_t code);
	void draw_bg15_tilemap();
	bool bg15_tiles_dirty;

	// eeprom
	DECLARE_READ16_MEMBER(eeprom_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);

	// cart
	DECLARE_READ16_MEMBER(cart_r);
	DECLARE_WRITE16_MEMBER(cart_w);

	// misc
	DECLARE_READ16_MEMBER(fpga_r);
	DECLARE_WRITE16_MEMBER(oki_bank_w);
	DECLARE_READ16_MEMBER(e00000_r);
	DECLARE_READ16_MEMBER(e00020_r);
	DECLARE_WRITE16_MEMBER(outputs_w);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// machine
	INTERRUPT_GEN_MEMBER(joystand_interrupt);
	void joystand_map(address_map &map);
};

const rgb_t joystand_state::BG15_TRANSPARENT = 0x99999999;

/***************************************************************************

    Tilemaps

***************************************************************************/

TILE_GET_INFO_MEMBER(joystand_state::get_bg1_tile_info)
{
	uint32_t code = (m_bg1_ram[tile_index * 2 + 0] << 16) | m_bg1_ram[tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0, code & 0x00ffffff, code >> 24, 0);
}

TILE_GET_INFO_MEMBER(joystand_state::get_bg2_tile_info)
{
	uint32_t code = (m_bg2_ram[tile_index * 2 + 0] << 16) | m_bg2_ram[tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0, code & 0x00ffffff, code >> 24, 0);
}

WRITE16_MEMBER(joystand_state::bg1_w)
{
	COMBINE_DATA(&m_bg1_ram[offset]);
	m_bg1_tmap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(joystand_state::bg2_w)
{
	COMBINE_DATA(&m_bg2_ram[offset]);
	m_bg2_tmap->mark_tile_dirty(offset/2);
}

/***************************************************************************

    r5g5b5 Layers

***************************************************************************/

// pixel-based
WRITE16_MEMBER(joystand_state::bg15_0_w)
{
	uint16_t val = COMBINE_DATA(&m_bg15_0_ram[offset]);
	m_bg15_bitmap[0].pix32(offset >> 9, offset & 0x1ff) = (val & 0x8000) ? BG15_TRANSPARENT : m_bg15_palette->pen_color(val & 0x7fff);
}

// tile-based
void joystand_state::draw_bg15_tile(address_space &space, int x, int y, uint16_t code)
{
	x *= 16;
	y *= 16;
	int srcaddr = 0x800000 + (code % (0x800 * 6)) * 16 * 16 * 2;

	for (int ty = 0; ty < 16; ++ty)
	{
		for (int tx = 0; tx < 16; ++tx)
		{
			uint16_t val = space.read_word(srcaddr + ty * 16 * 2 + tx * 2);
			m_bg15_bitmap[1].pix32(y + ty , x + tx) = (val & 0x8000) ? BG15_TRANSPARENT : m_bg15_palette->pen_color(val & 0x7fff);
		}
	}
}

void joystand_state::draw_bg15_tilemap()
{
	if (!bg15_tiles_dirty)
		return;

	bg15_tiles_dirty = false;

	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t *src = m_bg15_1_ram + 2/2;
	for (int y = 0; y < 0x10; ++y)
	{
		for (int x = 0; x < 0x20; ++x)
		{
			draw_bg15_tile(space, x, y, *src);
			src += 8/2;
		}
		src += 0x100/2;
	}
}

WRITE16_MEMBER(joystand_state::bg15_1_w)
{
	uint16_t code = COMBINE_DATA(&m_bg15_1_ram[offset]);
	if ((offset & 0x83) == 0x01)
		draw_bg15_tile(space, (offset/4) & 0x1f, offset/0x100, code);
}

/***************************************************************************

    Screen Update

***************************************************************************/

void joystand_state::video_start()
{
	m_bg1_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(joystand_state::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x20);
	m_bg2_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(joystand_state::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x40);

	m_bg1_tmap->set_transparent_pen(0xf);
	m_bg2_tmap->set_transparent_pen(0xf);

	for (auto & elem : m_bg15_bitmap)
	{
		elem.allocate(0x200, 0x200);
		elem.fill(BG15_TRANSPARENT);
	}

	bg15_tiles_dirty = true;
}

uint32_t joystand_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (machine().input().code_pressed(KEYCODE_A))  msk |= 4;
		if (machine().input().code_pressed(KEYCODE_S))  msk |= 8;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	m_bg1_tmap->set_scrollx(0, 0);
	m_bg1_tmap->set_scrolly(0, 0);

	m_bg2_tmap->set_scrollx(0, m_scroll[0] - 0xa);
	m_bg2_tmap->set_scrolly(0, m_scroll[1]);

	draw_bg15_tilemap();

	bitmap.fill(m_palette->black_pen(), cliprect);
	if (layers_ctrl & 4)    copybitmap_trans(bitmap, m_bg15_bitmap[0], 0, 0, 1, 0, cliprect, BG15_TRANSPARENT);
	if (layers_ctrl & 8)    copybitmap_trans(bitmap, m_bg15_bitmap[1], 0, 0, 0, 0, cliprect, BG15_TRANSPARENT);
	if (layers_ctrl & 1)    m_bg1_tmap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 2)    m_bg2_tmap->draw(screen, bitmap, cliprect, 0, 0);

	popmessage("S0: %04X S1: %04X EN: %04X OUT: %04X", m_scroll[0], m_scroll[1], m_enable[0], m_outputs[0]);
	return 0;
}

/***************************************************************************

    Memory Maps

***************************************************************************/

READ16_MEMBER(joystand_state::fpga_r)
{
	return 0xffff;
}

WRITE16_MEMBER(joystand_state::oki_bank_w)
{
	if (ACCESSING_BITS_0_7)
		m_oki->set_rom_bank((data >> 6) & 3);
}

READ16_MEMBER(joystand_state::eeprom_r)
{
	// mask 0x0020 ? (active low)
	// mask 0x0040 ? ""
	return (m_eeprom->do_read() & 1) << 3;
}
WRITE16_MEMBER(joystand_state::eeprom_w)
{
	if (ACCESSING_BITS_8_15)
	{
		// latch the data bit
		m_eeprom->di_write ( (data & 0x0004) ? ASSERT_LINE : CLEAR_LINE );

		// reset line asserted: reset.
		m_eeprom->cs_write ( (data & 0x0001) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write( (data & 0x0002) ? ASSERT_LINE : CLEAR_LINE );

		// mask 0x1000 ?
	}
}

WRITE16_MEMBER(joystand_state::outputs_w)
{
	COMBINE_DATA(&m_outputs[0]);
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0,            BIT(data, 0)); // coin counter 1
		machine().bookkeeping().coin_counter_w(1,            BIT(data, 1)); // coin counter 2

		output().set_value("blocker",             BIT(data, 2));
		output().set_value("error_lamp",          BIT(data, 3)); // counter error
		output().set_value("photo_lamp",          BIT(data, 4)); // during photo
	}
	if (ACCESSING_BITS_8_15)
	{
		output().set_value("ok_button_led",       BIT(data, 8));
		output().set_value("cancel_button_led",   BIT(data, 9));
	}
}

// carts

// copy slot
READ16_MEMBER(joystand_state::e00000_r)
{
	return ioport("COPY")->read();
}
// master slot
READ16_MEMBER(joystand_state::e00020_r)
{
	return ioport("MASTER")->read();
}

READ16_MEMBER(joystand_state::cart_r)
{
	int which = offset / 0x80000;
	int addr  = offset & 0x7ffff;
	return (m_cart_flash[which * 2 + 0]->read(addr) << 8) | m_cart_flash[which * 2 + 1]->read(addr);
}

WRITE16_MEMBER(joystand_state::cart_w)
{
	int which = offset / 0x80000;
	int addr  = offset & 0x7ffff;

	if (ACCESSING_BITS_0_7)
		m_cart_flash[which * 2 + 1]->write(addr, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_cart_flash[which * 2 + 0]->write(addr, data >> 8);

	bg15_tiles_dirty = true;
}

void joystand_state::joystand_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x200000, 0x200003).w("ym2413", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x200009, 0x200009).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200010, 0x200011).portr("IN0"); // r/w
	map(0x200012, 0x200013).ram().w(FUNC(joystand_state::outputs_w)).share("outputs"); // r/w
	map(0x200014, 0x200015).rw(FUNC(joystand_state::fpga_r), FUNC(joystand_state::oki_bank_w)); // r/w
//  AM_RANGE(0x200016, 0x200017) // write $9190 at boot

	map(0x400000, 0x47ffff).ram().w(FUNC(joystand_state::bg15_0_w)).share("bg15_0_ram"); // r5g5b5 200x200 pixel-based
	map(0x480000, 0x4fffff).ram(); // more rgb layers? (writes at offset 0)
	map(0x500000, 0x57ffff).ram(); // ""
	map(0x580000, 0x5fffff).ram(); // ""

	map(0x600000, 0x603fff).ram().w(FUNC(joystand_state::bg2_w)).share("bg2_ram");
	map(0x604000, 0x605fff).ram().w(FUNC(joystand_state::bg1_w)).share("bg1_ram");
	map(0x606000, 0x607fff).ram().w(FUNC(joystand_state::bg15_1_w)).share("bg15_1_ram"); // r5g5b5 200x200 tile-based
	map(0x608000, 0x609fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x60c000, 0x60c003).ram().share("scroll"); // write
	map(0x60c00c, 0x60c00d).ram().share("enable"); // write

	map(0x800000, 0xdfffff).rw(FUNC(joystand_state::cart_r), FUNC(joystand_state::cart_w)); // r/w (cart flash)
//  AM_RANGE(0xe00080, 0xe00081) // write (bit 0 = cart? bit 1 = ? bit 3 = ?)
	map(0xe00000, 0xe00001).r(FUNC(joystand_state::e00000_r)); // copy slot
	map(0xe00020, 0xe00021).r(FUNC(joystand_state::e00020_r)); // master slot

	map(0xe80040, 0xe8005f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
}


static INPUT_PORTS_START( joystand )
	// Cart status:
	// mask 0x1000 -> cart flash addressing (0 = sequential, 1 = interleaved even/odd)
	// mask 0x6000 == 0 -> cart present?
	// mask 0x8000 -> cart ready?

	PORT_START("MASTER")
	PORT_CONFNAME( 0x1000, 0x1000, "Master Flash Addressing" )
	PORT_CONFSETTING(      0x1000, "Interleaved" )
	PORT_CONFSETTING(      0x0000, "Sequential" )
	PORT_CONFNAME( 0x2000, 0x0000, "Master Slot Sense 1" )
	PORT_CONFSETTING(      0x2000, "Empty" )
	PORT_CONFSETTING(      0x0000, "Cart" )
	PORT_CONFNAME( 0x4000, 0x0000, "Master Slot Sense 2" )
	PORT_CONFSETTING(      0x4000, "Empty" )
	PORT_CONFSETTING(      0x0000, "Cart" )
	PORT_BIT( 0x8fff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COPY")
	PORT_CONFNAME( 0x1000, 0x1000, "Copy Flash Addressing" )
	PORT_CONFSETTING(      0x1000, "Interleaved" )
	PORT_CONFSETTING(      0x0000, "Sequential" )
	PORT_CONFNAME( 0x2000, 0x2000, "Copy Slot Sense 1" )
	PORT_CONFSETTING(      0x2000, "Empty" )
	PORT_CONFSETTING(      0x0000, "Cart" )
	PORT_CONFNAME( 0x4000, 0x4000, "Copy Slot Sense 2" )
	PORT_CONFSETTING(      0x4000, "Empty" )
	PORT_CONFSETTING(      0x0000, "Cart" )
	PORT_BIT( 0x8fff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) // up
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) // down
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) // left
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // right
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) // ok
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) // cancel
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1          ) // coin
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1       ) // service
	PORT_SERVICE_NO_TOGGLE( 0x0400, IP_ACTIVE_LOW       ) // test
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
INPUT_PORTS_END


static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4) },
	{ STEP8(0, 4*8) },
	8*8*4
};

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP16(0, 8*16) },
	16*16*8
};

static GFXDECODE_START( gfx_joystand )
	GFXDECODE_ENTRY( "tiles",    0, layout_8x8x4,   0, 0x100 )
	GFXDECODE_ENTRY( "cart.u5",  0, layout_16x16x8, 0,  0x10 )
	GFXDECODE_ENTRY( "cart.u6",  0, layout_16x16x8, 0,  0x10 )
	GFXDECODE_ENTRY( "cart.u3",  0, layout_16x16x8, 0,  0x10 )
	GFXDECODE_ENTRY( "cart.u4",  0, layout_16x16x8, 0,  0x10 )
	GFXDECODE_ENTRY( "cart.u1",  0, layout_16x16x8, 0,  0x10 )
	GFXDECODE_ENTRY( "cart.u2",  0, layout_16x16x8, 0,  0x10 )
GFXDECODE_END


void joystand_state::machine_start()
{
	m_cart_flash[0]  = m_cart_u11;      m_cart_flash[1]  = m_cart_u5;
	m_cart_flash[2]  = m_cart_u12;      m_cart_flash[3]  = m_cart_u6;
	m_cart_flash[4]  = m_cart_u9;       m_cart_flash[5]  = m_cart_u3;
	m_cart_flash[6]  = m_cart_u10;      m_cart_flash[7]  = m_cart_u4;
	m_cart_flash[8]  = m_cart_u7;       m_cart_flash[9]  = m_cart_u1;
	m_cart_flash[10] = m_cart_u8;       m_cart_flash[11] = m_cart_u2;
}

void joystand_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(joystand_state::joystand_interrupt)
{
	// VBlank is connected to INT1 (external interrupts pin 1)
	m_maincpu->external_interrupt_1();
}

void joystand_state::joystand(machine_config &config)
{
	// basic machine hardware
	TMP68301(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &joystand_state::joystand_map);
	m_maincpu->set_vblank_int("screen", FUNC(joystand_state::joystand_interrupt));
	m_maincpu->in_parallel_callback().set(FUNC(joystand_state::eeprom_r));
	m_maincpu->out_parallel_callback().set(FUNC(joystand_state::eeprom_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(joystand_state::screen_update));
	screen.set_size(0x200, 0x100);
	screen.set_visarea(0x40, 0x40+0x178-1, 0x10, 0x100-1);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_joystand);

	PALETTE(config, m_bg15_palette, palette_device::RGB_555);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym2413", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.80);

	OKIM6295(config, m_oki, XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // pin 7 not verified

	// cart
	TMS_29F040(config, "cart.u1");
	TMS_29F040(config, "cart.u2");
	TMS_29F040(config, "cart.u3");
	TMS_29F040(config, "cart.u4");
	TMS_29F040(config, "cart.u5");
	TMS_29F040(config, "cart.u6");
	TMS_29F040(config, "cart.u7");
	TMS_29F040(config, "cart.u8");
	TMS_29F040(config, "cart.u9");
	TMS_29F040(config, "cart.u10");
	TMS_29F040(config, "cart.u11");
	TMS_29F040(config, "cart.u12");

	// devices
	EEPROM_93C46_16BIT(config, "eeprom");
	MSM6242(config, "rtc", XTAL(32'768));
}


/***************************************************************************

    Machine driver(s)

***************************************************************************/

ROM_START( joystand )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "yuvo_jsp003b.ic3",  0x00000, 0x40000, CRC(0c85bc77) SHA1(ad8ec80b02e82cf43e3f0732cc6e5468c6d21297) )
	ROM_LOAD16_BYTE( "yuvo_jsp004b.ic63", 0x00001, 0x40000, CRC(333396e5) SHA1(fc605890676efed476b67abcd1fcb8d509324be2) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD16_BYTE( "yuvo_jsp005.2j",  0x000000, 0x80000, CRC(98caff66) SHA1(e201bb0119bc6560b7def40d42d2ef5b788ca3d4) )
	ROM_LOAD16_BYTE( "yuvo_jsp006.4j",  0x000001, 0x80000, CRC(6c9f8048) SHA1(3fd6effa83b0e429b97d55041697f1b5ee6eafe2) )
	ROM_LOAD16_BYTE( "yuvo_jsp007a.2g", 0x100000, 0x40000, CRC(ccfd5b72) SHA1(29bf14c888731d63f5d6705d0efb840f1de0fc91) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "yuvo_jsp008a.4g", 0x100001, 0x40000, CRC(fdaf369c) SHA1(488741b9f2c5ccd27ee1aa5120834ec8b161d6b1) ) // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "cart.u1", 0 )
	ROM_LOAD( "jsp.u1",  0x00000, 0x80000, CRC(5478b779) SHA1(5d76645de2833cefb20374480572f06ef496ce30) )
	ROM_REGION( 0x80000, "cart.u2", 0 )
	ROM_LOAD( "jsp.u2",  0x00000, 0x80000, CRC(adba9522) SHA1(574e925b35ef3f732989f712caf3f92e16106c22) )
	ROM_REGION( 0x80000, "cart.u3", 0 )
	ROM_LOAD( "jsp.u3",  0x00000, 0x80000, CRC(6e293f82) SHA1(e29099c5337c5e7b4776da01a3bd45141b4900b9) )
	ROM_REGION( 0x80000, "cart.u4", 0 )
	ROM_LOAD( "jsp.u4",  0x00000, 0x80000, CRC(4caab540) SHA1(5cd88dc93c57d3ae9a6b3773222d8f6001b74634) )
	ROM_REGION( 0x80000, "cart.u5", 0 )
	ROM_LOAD( "jsp.u5",  0x00000, 0x80000, CRC(2cfee501) SHA1(2f07179accca0181d20bb0af797194a8ddad4f7a) )
	ROM_REGION( 0x80000, "cart.u6", 0 )
	ROM_LOAD( "jsp.u6",  0x00000, 0x80000, CRC(6069d711) SHA1(e969dcc4b5da6951b4140a78fa7cda350167ca66) )
	ROM_REGION( 0x80000, "cart.u7", 0 )
	ROM_LOAD( "jsp.u7",  0x00000, 0x80000, CRC(9f58df4d) SHA1(e4933087204624c021420bf632a6ddfd7b26179c) )
	ROM_REGION( 0x80000, "cart.u8", 0 )
	ROM_LOAD( "jsp.u8",  0x00000, 0x80000, CRC(829ddce6) SHA1(614ac45d55abe487aaa0e5ca7354926caaa03346) )
	ROM_REGION( 0x80000, "cart.u9", 0 )
	ROM_LOAD( "jsp.u9",  0x00000, 0x80000, CRC(e5ee5d8d) SHA1(ea6ea2fe4fc8b9eaf556453b430c85434ddf1570) )
	ROM_REGION( 0x80000, "cart.u10", 0 )
	ROM_LOAD( "jsp.u10", 0x00000, 0x80000, CRC(97234b84) SHA1(06a5dc290e925f5d6a8bade89d970964f32c9945) )
	ROM_REGION( 0x80000, "cart.u11", 0 )
	ROM_LOAD( "jsp.u11", 0x00000, 0x80000, CRC(8b138563) SHA1(8a2092d80d02ac685014540837b9aa38dfe0eb47) )
	ROM_REGION( 0x80000, "cart.u12", 0 )
	ROM_LOAD( "jsp.u12", 0x00000, 0x80000, CRC(10001cab) SHA1(5de49061f9ab81a4dc7e3405132ecec35a63248d) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "yuvo_jsp001.ic14", 0x00000, 0x80000, CRC(bf2b4557) SHA1(932b96f4b3553e9d52509d678c7c2d4dcfc32cd7) )
	ROM_LOAD( "yuvo_jsp002.ic13", 0x80000, 0x80000, CRC(0eb6db96) SHA1(e5f88f5357709def987f807d1a2d21514b5aa107) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x117, "pld", 0 )
	ROM_LOAD( "jsp-frm.ic100", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "jsp-map.ic4",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "jsp-sub.1f",    0x000, 0x117, NO_DUMP )
	ROM_LOAD( "jsp-xct.ic5",   0x000, 0x117, NO_DUMP )
ROM_END

GAME( 1997, joystand, 0, joystand, joystand, joystand_state, empty_init, ROT0, "Yuvo", "Joy Stand Private", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
