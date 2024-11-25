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

TODO:
- Support cartridge slot select function
- Requires camera, printer, lightpen emulation
- Identify x180ii and emulate the differences

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/tmp68301.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

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
		m_cart_flash(*this, "cart.u%u", 1U),
		m_cartflash_bankdev(*this, "cartflash_bankdev"),
		m_oki(*this, "oki"),
		m_bg1_ram(*this, "bg1_ram"),
		m_bg2_ram(*this, "bg2_ram"),
		m_bg15_0_ram(*this, "bg15_0_ram"),
		m_bg15_1_ram(*this, "bg15_1_ram"),
		m_scroll(*this, "scroll"),
		m_enable(*this, "enable"),
		m_outputs(*this, "outputs"),
		m_blocker(*this, "blocker"),
		m_error_lamp(*this, "error_lamp"),
		m_photo_lamp(*this, "photo_lamp"),
		m_ok_button_led(*this, "ok_button_led"),
		m_cancel_button_led(*this, "cancel_button_led")
	{ }

	void joystand(machine_config &config);
	void x180ii(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<tmp68301_device> m_maincpu;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_bg15_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device_array<intelfsh8_device, 12> m_cart_flash;
	optional_device<address_map_bank_device> m_cartflash_bankdev;
	required_device<okim6295_device> m_oki;

	// memory pointers
	required_shared_ptr<uint16_t> m_bg1_ram;
	required_shared_ptr<uint16_t> m_bg2_ram;
	optional_shared_ptr<uint16_t> m_bg15_0_ram;
	optional_shared_ptr<uint16_t> m_bg15_1_ram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_enable;
	required_shared_ptr<uint16_t> m_outputs;

	// I/O
	output_finder<> m_blocker;
	output_finder<> m_error_lamp;
	output_finder<> m_photo_lamp;
	output_finder<> m_ok_button_led;
	output_finder<> m_cancel_button_led;

	// tilemaps
	tilemap_t *m_bg1_tmap = nullptr;
	tilemap_t *m_bg2_tmap = nullptr;
	void bg1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);

	// r5g5b5 layers
	bitmap_rgb32 m_bg15_bitmap[2]{};
	void bg15_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg15_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	static const rgb_t BG15_TRANSPARENT;
	void draw_bg15_tile(int x, int y, uint16_t code);
	void draw_bg15_tilemap();
	bool bg15_tiles_dirty = false;

	// eeprom
	uint16_t eeprom_r();
	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// cart
	void cart_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// misc
	uint16_t fpga_r();
	void oki_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t e00000_r();
	uint16_t e00020_r();
	void outputs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_x180ii(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_VIDEO_START(x180ii);

	// machine
	INTERRUPT_GEN_MEMBER(joystand_interrupt);
	void joystand_map(address_map &map) ATTR_COLD;
	void x180ii_map(address_map &map) ATTR_COLD;
	void cart_map(address_map &map) ATTR_COLD;
};

const rgb_t joystand_state::BG15_TRANSPARENT = 0x99999999;

/***************************************************************************

    Tilemaps

***************************************************************************/

TILE_GET_INFO_MEMBER(joystand_state::get_bg1_tile_info)
{
	uint32_t code = (m_bg1_ram[tile_index * 2 + 0] << 16) | m_bg1_ram[tile_index * 2 + 1];
	tileinfo.set(0, code & 0x00ffffff, code >> 24, 0);
}

TILE_GET_INFO_MEMBER(joystand_state::get_bg2_tile_info)
{
	uint32_t code = (m_bg2_ram[tile_index * 2 + 0] << 16) | m_bg2_ram[tile_index * 2 + 1];
	tileinfo.set(0, code & 0x00ffffff, code >> 24, 0);
}


void joystand_state::bg1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg1_ram[offset]);
	m_bg1_tmap->mark_tile_dirty(offset/2);
}

void joystand_state::bg2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg2_ram[offset]);
	m_bg2_tmap->mark_tile_dirty(offset/2);
}

/***************************************************************************

    r5g5b5 Layers

***************************************************************************/

// pixel-based
void joystand_state::bg15_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t val = COMBINE_DATA(&m_bg15_0_ram[offset]);
	m_bg15_bitmap[0].pix(offset >> 9, offset & 0x1ff) = (val & 0x8000) ? BG15_TRANSPARENT : m_bg15_palette->pen_color(val & 0x7fff);
}

// tile-based
void joystand_state::draw_bg15_tile(int x, int y, uint16_t code)
{
	address_space &space = m_cartflash_bankdev->space(0);

	x *= 16;
	y *= 16;
	int srcaddr = (code % (0x800 * 6)) * 16 * 16 * 2;

	for (int ty = 0; ty < 16; ++ty)
	{
		for (int tx = 0; tx < 16; ++tx)
		{
			uint16_t val = space.read_word(srcaddr + ty * 16 * 2 + tx * 2);
			m_bg15_bitmap[1].pix(y + ty , x + tx) = (val & 0x8000) ? BG15_TRANSPARENT : m_bg15_palette->pen_color(val & 0x7fff);
		}
	}
}

void joystand_state::draw_bg15_tilemap()
{
	if (!bg15_tiles_dirty)
		return;

	bg15_tiles_dirty = false;

	uint16_t *src = m_bg15_1_ram + 2/2;
	for (int y = 0; y < 0x10; ++y)
	{
		for (int x = 0; x < 0x20; ++x)
		{
			draw_bg15_tile(x, y, *src);
			src += 8/2;
		}
		src += 0x100/2;
	}
}

void joystand_state::bg15_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t code = COMBINE_DATA(&m_bg15_1_ram[offset]);
	if ((offset & 0x83) == 0x01)
		draw_bg15_tile((offset/4) & 0x1f, offset/0x100, code);
}

/***************************************************************************

    Screen Update

***************************************************************************/

void joystand_state::video_start()
{
	m_bg1_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(joystand_state::get_bg1_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x20);
	m_bg2_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(joystand_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x40);

	m_bg1_tmap->set_transparent_pen(0xf);
	m_bg2_tmap->set_transparent_pen(0xf);

	for (auto & elem : m_bg15_bitmap)
	{
		elem.allocate(0x200, 0x200);
		elem.fill(BG15_TRANSPARENT);
	}

	bg15_tiles_dirty = true;
}

VIDEO_START_MEMBER(joystand_state, x180ii)
{
	m_bg1_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(joystand_state::get_bg1_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x20);
	m_bg2_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(joystand_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 0x40, 0x40);

	m_bg1_tmap->set_transparent_pen(0xf);
	m_bg2_tmap->set_transparent_pen(0xf);
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

uint32_t joystand_state::screen_update_x180ii(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

	bitmap.fill(m_palette->black_pen(), cliprect);
	if (layers_ctrl & 1)    m_bg1_tmap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 2)    m_bg2_tmap->draw(screen, bitmap, cliprect, 0, 0);

	popmessage("S0: %04X S1: %04X EN: %04X OUT: %04X", m_scroll[0], m_scroll[1], m_enable[0], m_outputs[0]);
	return 0;
}

/***************************************************************************

    Memory Maps

***************************************************************************/

uint16_t joystand_state::fpga_r()
{
	return 0xffff;
}

void joystand_state::oki_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_oki->set_rom_bank((data >> 6) & 3);
}

uint16_t joystand_state::eeprom_r()
{
	// mask 0x0020 ? (active low)
	// mask 0x0040 ? ""
	return (m_eeprom->do_read() & 1) << 3;
}
void joystand_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

void joystand_state::outputs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_outputs[0]);
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0,            BIT(data, 0)); // coin counter 1
		machine().bookkeeping().coin_counter_w(1,            BIT(data, 1)); // coin counter 2

		m_blocker = BIT(data, 2);
		m_error_lamp = BIT(data, 3); // counter error
		m_photo_lamp = BIT(data, 4); // during photo
	}
	if (ACCESSING_BITS_8_15)
	{
		m_ok_button_led = BIT(data, 8);
		m_cancel_button_led = BIT(data, 9);
	}
}

// carts

// copy slot
uint16_t joystand_state::e00000_r()
{
	return ioport("COPY")->read();
}
// master slot
uint16_t joystand_state::e00020_r()
{
	return ioport("MASTER")->read();
}

void joystand_state::cart_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_cartflash_bankdev->write16(offset, data, mem_mask);
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
//  map(0x200016, 0x200017) // write $9190 at boot

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

	map(0x800000, 0xdfffff).r(m_cartflash_bankdev, FUNC(address_map_bank_device::read16)).w(FUNC(joystand_state::cart_w)); // r/w (cart flash)
//  map(0xe00080, 0xe00081) // write (bit 0 = cart? bit 1 = ? bit 3 = ?)
	map(0xe00000, 0xe00001).r(FUNC(joystand_state::e00000_r)); // copy slot
	map(0xe00020, 0xe00021).r(FUNC(joystand_state::e00020_r)); // master slot

	map(0xe80040, 0xe8005f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
}

void joystand_state::cart_map(address_map &map)
{
	map(0x000000, 0x0fffff).rw(m_cart_flash[10], FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x000000, 0x0fffff).rw(m_cart_flash[4],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);

	map(0x100000, 0x1fffff).rw(m_cart_flash[11], FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x100000, 0x1fffff).rw(m_cart_flash[5],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);

	map(0x200000, 0x2fffff).rw(m_cart_flash[8],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x200000, 0x2fffff).rw(m_cart_flash[2],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);

	map(0x300000, 0x3fffff).rw(m_cart_flash[9],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x300000, 0x3fffff).rw(m_cart_flash[3],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);

	map(0x400000, 0x4fffff).rw(m_cart_flash[6],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x400000, 0x4fffff).rw(m_cart_flash[0],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);

	map(0x500000, 0x5fffff).rw(m_cart_flash[7],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x500000, 0x5fffff).rw(m_cart_flash[1],  FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
}

void joystand_state::x180ii_map(address_map &map) // TODO: verify everything
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x200000, 0x200003).w("ym2413", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x200009, 0x200009).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200010, 0x200011).portr("IN0"); // r/w
	map(0x200012, 0x200013).ram().w(FUNC(joystand_state::outputs_w)).share("outputs"); // r/w
	map(0x200014, 0x200015).rw(FUNC(joystand_state::fpga_r), FUNC(joystand_state::oki_bank_w)); // r/w
//  map(0x200016, 0x200017) // write $9190 at boot

	map(0x400000, 0x47ffff).ram();
	map(0x480000, 0x4fffff).ram(); // more rgb layers? (writes at offset 0)
	map(0x500000, 0x57ffff).ram(); // ""
	map(0x580000, 0x5fffff).ram(); // ""

	map(0x600000, 0x603fff).ram().w(FUNC(joystand_state::bg2_w)).share("bg2_ram");
	map(0x604000, 0x605fff).ram().w(FUNC(joystand_state::bg1_w)).share("bg1_ram");
	map(0x606000, 0x607fff).ram(); // still writes here
	map(0x608000, 0x609fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x60c000, 0x60c003).ram().share("scroll"); // write
	map(0x60c00c, 0x60c00d).ram().share("enable"); // write

	//  map(0xe00080, 0xe00081) // write (bit 0 = cart? bit 1 = ? bit 3 = ?)
	//map(0xe00000, 0xe00001).r(FUNC(joystand_state::e00000_r)); // copy slot
	//map(0xe00020, 0xe00021).r(FUNC(joystand_state::e00020_r)); // master slot
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
	GFXDECODE_ENTRY( "tiles",    0, gfx_8x8x4_packed_msb, 0, 0x100 )
	GFXDECODE_ENTRY( "cart.u5",  0, layout_16x16x8,       0,  0x10 )
	GFXDECODE_ENTRY( "cart.u6",  0, layout_16x16x8,       0,  0x10 )
	GFXDECODE_ENTRY( "cart.u3",  0, layout_16x16x8,       0,  0x10 )
	GFXDECODE_ENTRY( "cart.u4",  0, layout_16x16x8,       0,  0x10 )
	GFXDECODE_ENTRY( "cart.u1",  0, layout_16x16x8,       0,  0x10 )
	GFXDECODE_ENTRY( "cart.u2",  0, layout_16x16x8,       0,  0x10 )
GFXDECODE_END

static GFXDECODE_START( gfx_x180ii )
	GFXDECODE_ENTRY( "tiles", 0,        gfx_8x8x4_packed_msb, 0, 0x100 )
	GFXDECODE_ENTRY( "tiles", 0x200000, layout_16x16x8,       0, 0x10 ) // wrong
GFXDECODE_END


void joystand_state::machine_start()
{
	m_blocker.resolve();
	m_error_lamp.resolve();
	m_photo_lamp.resolve();
	m_ok_button_led.resolve();
	m_cancel_button_led.resolve();
}


void joystand_state::joystand(machine_config &config)
{
	// basic machine hardware
	TMP68301(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &joystand_state::joystand_map);
	m_maincpu->parallel_r_cb().set(FUNC(joystand_state::eeprom_r));
	m_maincpu->parallel_w_cb().set(FUNC(joystand_state::eeprom_w));

	ADDRESS_MAP_BANK(config, m_cartflash_bankdev).set_map(&joystand_state::cart_map).set_options(ENDIANNESS_BIG, 16, 24, 0x800000); // TODO: address bit per carts?
	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(joystand_state::screen_update));
	screen.set_size(0x200, 0x100);
	screen.set_visarea(0x40, 0x40+0x178-1, 0x10, 0x100-1);
	screen.screen_vblank().set_inputline(m_maincpu, 1);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_joystand);

	PALETTE(config, m_bg15_palette, palette_device::RGB_555);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym2413", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.80);

	OKIM6295(config, m_oki, XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // pin 7 not verified

	// cart
	for (int i = 0; i < 12; i++)
		TMS_29F040(config, m_cart_flash[i]);

	// devices
	EEPROM_93C46_16BIT(config, "eeprom");
	MSM6242(config, "rtc", XTAL(32'768));
}

void joystand_state::x180ii(machine_config &config)
{
	// basic machine hardware
	TMP68303(config, m_maincpu, XTAL(16'000'000)); // TMP68303F
	m_maincpu->set_addrmap(AS_PROGRAM, &joystand_state::x180ii_map);
	m_maincpu->parallel_r_cb().set(FUNC(joystand_state::eeprom_r));
	m_maincpu->parallel_w_cb().set(FUNC(joystand_state::eeprom_w));

	I8255(config, "ppi0");

	I8255(config, "ppi1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify this
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(joystand_state::screen_update_x180ii));
	screen.set_size(0x200, 0x100);
	screen.set_visarea(0x40, 0x40+0x178-1, 0x10, 0x100-1);
	screen.screen_vblank().set_inputline(m_maincpu, 1);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_x180ii);

	MCFG_VIDEO_START_OVERRIDE(joystand_state, x180ii)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym2413", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.80);

	OKIM6295(config, m_oki, XTAL(16'000'000) / 16, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.50); // clock supplied by pin 15 of the XCT GAL, to be verified

	// devices
	EEPROM_93C46_16BIT(config, "eeprom");
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

ROM_START( x180 ) // YUVO PCC180C PCB + JSR-1A REV.B riser PCB. Similar to the joystand one, even most IC locations match. It has 2x D71055C.
	ROM_REGION( 0x100000, "maincpu", 0 ) // on riser PCB
	ROM_LOAD16_BYTE( "msvol1a.even.u5", 0x00000, 0x80000, CRC(f79c476c) SHA1(c49a8ccbe494d39635fda58ba379cea6714a1ec2) )
	ROM_LOAD16_BYTE( "msvol1b.odd.u6",  0x00001, 0x80000, CRC(dd46fd51) SHA1(60832cb90d5335cd55422e56c8482ac7c88f70ff) )

	ROM_REGION( 0x600000, "tiles", 0 ) // on riser PCB
	ROM_LOAD( "e28f016sa.u1", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD( "e28f016sa.u2", 0x200000, 0x200000, NO_DUMP )
	ROM_LOAD( "e28f016sa.u3", 0x400000, 0x200000, NO_DUMP )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "x180-sej1.ver1.00.ic14", 0x00000, 0x80000, CRC(86a0801b) SHA1(a252ed786bf51b963feb6ff253303ea3b67d8fcf) )
	ROM_LOAD( "x180-sej2.ver1.00.ic13", 0x80000, 0x80000, CRC(92f73edb) SHA1(541a671d0e1648d8ddb42abe0e851ea9c68c718f) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "93c46-x16.ic16", 0x00, 0x80, NO_DUMP )

	ROM_REGION( 0x117, "pld", 0 )
	ROM_LOAD( "map.ic4", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "xct.ic5", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( x180ii ) // YUVO PCC180C PCB. Similar to the joystand one, even most IC locations match. It has 2x D71055C.
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "x180ii-mpj-e.ver1.00.ic3",  0x00000, 0x80000, CRC(20343837) SHA1(44306e93d3c333f9e418c42d44433fe5654cad40) )
	ROM_LOAD16_BYTE( "x180ii-mpj-0.ver1.00.ic63", 0x00001, 0x80000, CRC(0d43c32a) SHA1(ab09c2ed61a80704b6d2029f9a2e98e93a5ec5e6) )

	ROM_REGION( 0x600000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "x180ii-chrm3.ver1.00.ic30", 0x000000, 0x200000, CRC(74b79688) SHA1(d3792aa8aa88a50a3b5530ed2e2077bd03d23aee) )
	ROM_LOAD16_WORD_SWAP( "x180ii-chrm2.ver1.00.ic32", 0x200000, 0x200000, CRC(6198a681) SHA1(0eae28c72e7b737788cb62767fa7713ed9499206) )
	ROM_LOAD16_WORD_SWAP( "x180ii-chrm1.ver1.00.ic33", 0x400000, 0x200000, CRC(9b477d6d) SHA1(e6541334298729fe60d158c5e86e84294bcc409c) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "x180-sej1.ver1.00.ic14", 0x00000, 0x80000, CRC(86a0801b) SHA1(a252ed786bf51b963feb6ff253303ea3b67d8fcf) )
	ROM_LOAD( "x180-sej2.ver1.00.ic13", 0x80000, 0x80000, CRC(92f73edb) SHA1(541a671d0e1648d8ddb42abe0e851ea9c68c718f) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "93c46-x16.ic16", 0x00, 0x80, CRC(7ce71435) SHA1(ce9a008f85aae5a8209300f879d8c56af512f44f) )

	ROM_REGION( 0x117, "pld", 0 )
	ROM_LOAD( "map.ic4", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "xct.ic5", 0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1997, joystand, 0, joystand, joystand, joystand_state, empty_init, ROT0, "Yuvo", "Joy Stand Private",                   MACHINE_NOT_WORKING | MACHINE_NODEVICE_PRINTER | MACHINE_SUPPORTS_SAVE )
GAME( 1997, x180,     0, x180ii,   joystand, joystand_state, empty_init, ROT0, "Yuvo", "unknown Yuvo Joy Stand game (set 1)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_PRINTER | MACHINE_SUPPORTS_SAVE ) // has Joy Stand sample in Oki ROMs
GAME( 1997, x180ii,   0, x180ii,   joystand, joystand_state, empty_init, ROT0, "Yuvo", "unknown Yuvo Joy Stand game (set 2)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_PRINTER | MACHINE_SUPPORTS_SAVE ) // has Joy Stand sample in Oki ROMs
