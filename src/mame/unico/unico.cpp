// license:BSD-3-Clause
// copyright-holders: Luca Elia

/***************************************************************************

                              -= Unico Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU         :   M68000 or M68EC020
Sound Chips :   OKI M6295 (AD-65) + YM3812 (K-666) or YM2151
Video Chips :   3 x Actel A1020B (Square 84 Pin Socketed) [Or A40MX04-F]
                MACH211 (Square 44 Pin Socketed) [Or MACH210-15JC]


---------------------------------------------------------------------------
Year + Game         PCB             Notes
---------------------------------------------------------------------------
97  Burglar X       ?
98  Zero Point      ZPM1001A/B      Has Light Guns.
99  Zero Point 2    UZP21001A/B     Has Light Guns.
---------------------------------------------------------------------------


***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class burglarx_state : public driver_device
{
public:
	burglarx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_leds(*this, "led%u", 0U),
		m_vram(*this, "vram", 0xc000, ENDIANNESS_BIG),
		m_scroll(*this, "scroll", 22, ENDIANNESS_BIG),
		m_spriteram(*this, "spriteram", 0x800, ENDIANNESS_BIG)
	{ }

	void burglarx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	static rgb_t unico_R6G6B6X(uint32_t raw);
	uint16_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scroll_r(offs_t offset);
	void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void okibank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<okim6295_device> m_oki;
	output_finder<2> m_leds;

private:
	memory_share_creator<uint16_t> m_vram;
	memory_share_creator<uint16_t> m_scroll;
	tilemap_t *m_tilemap[3]{};
	int m_sprites_scrolldx = 0;
	int m_sprites_scrolldy = 0;
	memory_share_creator<uint16_t> m_spriteram;
};

class zeropnt_state : public burglarx_state
{
public:
	zeropnt_state(const machine_config &mconfig, device_type type, const char *tag) :
		burglarx_state(mconfig, type, tag),
		m_okibank(*this, "okibank"),
		m_screen(*this, "screen"),
		m_gun_axes(*this, { "Y0", "X0", "Y1", "X1" })
	{ }

	void zeropnt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_memory_bank m_okibank;

	required_device<screen_device> m_screen;

	enum { Y0, X0, Y1, X1 }; // gun axis indices

	template <uint8_t Which> uint16_t gunx_msb_r();
	template <uint8_t Which> uint16_t guny_msb_r();

	void oki_map(address_map &map) ATTR_COLD;

private:
	required_ioport_array<4> m_gun_axes;

	void okibank_leds_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
};

class zeropnt2_state : public zeropnt_state
{
public:
	zeropnt2_state(const machine_config &mconfig, device_type type, const char *tag) :
		zeropnt_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom")
	{ }

	void zeropnt2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	template <uint8_t Which> uint32_t gunx_msb_r();
	template <uint8_t Which> uint32_t guny_msb_r();
	void okibank(uint8_t data);
	void leds_w(uint8_t data);

	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void program_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W / E       Shows Layer 0 / 1 / 2
        A               Shows Sprites

        Keys can be used together!


    [ 3 Scrolling Layers ]

        Tile Size:              16 x 16 x 8
        Layer Size (tiles):     64 x 64

    [ 512 Sprites ]

        Sprites are made of 16 x 16 x 8 tiles. Size can vary from 1 to
        16 tiles horizontally, while their height is always 1 tile.
        There seems to be 4 levels of priority (wrt layers) for each
        sprite, following this simple scheme:

        [if we denote the three layers with 0-3 (0 being the backmost)
         and the sprite with S]

        Sprite Priority         Order (back -> front)
                0                   S 0 1 2
                1                   0 S 1 2
                2                   0 1 S 2
                3                   0 1 2 S

***************************************************************************/

/***************************************************************************

                                    Palette

    Byte:   0   1   2   3
    Gun:    R   G   B   0

    6 Bits x Gun

***************************************************************************/

rgb_t burglarx_state::unico_R6G6B6X(uint32_t raw)
{
	int const red   = (raw >> 24) & 0xfc;
	int const green = (raw >> 16) & 0xfc;
	int const blue  = (raw >>  8) & 0xfc;

	return rgb_t(red | (red >> 6), green | (green >> 6), blue | (blue >> 6));
}

/***************************************************************************

                                Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 7--- ----
                ---- ---- -6-- ----     Flip Y
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color

***************************************************************************/


TILE_GET_INFO_MEMBER(burglarx_state::get_tile_info)
{
	uint16_t const *vram = (uint16_t *)tilemap.user_data();
	uint16_t const code = vram[2 * tile_index + 0];
	uint16_t const attr = vram[2 * tile_index + 1];
	tileinfo.set(1, code, attr & 0x1f, TILE_FLIPYX(attr >> 5));
}

uint16_t burglarx_state::vram_r(offs_t offset) { return m_vram[offset]; }

void burglarx_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const tile = ((offset / 0x2000) + 1) % 3;
	COMBINE_DATA(&m_vram[offset]);
	m_tilemap[tile]->mark_tile_dirty((offset & 0x1fff) / 2);
}


uint16_t burglarx_state::scroll_r(offs_t offset) { return m_scroll[offset]; }
void burglarx_state::scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_scroll[offset]); }
uint16_t burglarx_state::spriteram_r(offs_t offset) { return m_spriteram[offset]; }
void burglarx_state::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)  { COMBINE_DATA(&m_spriteram[offset]); }


/***************************************************************************


                            Video Hardware Init


***************************************************************************/


void burglarx_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(burglarx_state::get_tile_info)), TILEMAP_SCAN_ROWS, 0x10, 0x10, 0x40, 0x40);

	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(burglarx_state::get_tile_info)), TILEMAP_SCAN_ROWS, 0x10, 0x10, 0x40, 0x40);

	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(burglarx_state::get_tile_info)), TILEMAP_SCAN_ROWS, 0x10, 0x10, 0x40, 0x40);

	m_tilemap[0]->set_user_data(&m_vram[0x8000 / 2]);
	m_tilemap[1]->set_user_data(&m_vram[0x0000 / 2]);
	m_tilemap[2]->set_user_data(&m_vram[0x4000 / 2]);

	m_sprites_scrolldx = -0x3f;
	m_sprites_scrolldy = -0x0e;

	m_tilemap[0]->set_scrolldx(-0x32, 0);
	m_tilemap[1]->set_scrolldx(-0x30, 0);
	m_tilemap[2]->set_scrolldx(-0x2e, 0);

	m_tilemap[0]->set_scrolldy(-0x0f, 0);
	m_tilemap[1]->set_scrolldy(-0x0f, 0);
	m_tilemap[2]->set_scrolldy(-0x0f, 0);

	m_tilemap[0]->set_transparent_pen(0x00);
	m_tilemap[1]->set_transparent_pen(0x00);
	m_tilemap[2]->set_transparent_pen(0x00);
}


/***************************************************************************

                                Sprites Drawing


        0.w                             X

        2.w                             Y

        4.w                             Code

        6.w     fe-- ---- ---- ----
                --dc ---- ---- ----     Priority
                ---- ba98 ---- ----     Number of tiles along X, minus 1
                ---- ---- 7--- ----
                ---- ---- -6-- ----     Flip Y?
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color


***************************************************************************/

void burglarx_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Draw them backwards, for pdrawgfx
	for (int offs = (0x800 - 8) / 2; offs >= 0 ; offs -= 8 / 2)
	{
		int sx = m_spriteram[offs + 0];
		int sy = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 2];
		int const attr = m_spriteram[offs + 3];

		int const flipx = attr & 0x020;
		int const flipy = attr & 0x040;   // not sure

		int const dimx = ((attr >> 8) & 0xf) + 1;

		int const priority = ((attr >> 12) & 0x3);
		int pri_mask;

		switch (priority)
		{
			case 0:     pri_mask = 0xfe;    break;  // below all
			case 1:     pri_mask = 0xf0;    break;  // above layer 0
			case 2:     pri_mask = 0xfc;    break;  // above layer 1
			default:
			case 3:     pri_mask = 0x00;            // above all
		}

		sx += m_sprites_scrolldx;
		sy += m_sprites_scrolldy;

		sx = (sx & 0x1ff) - (sx & 0x200);
		sy = (sy & 0x1ff) - (sy & 0x200);

		int startx, endx, incx;

		if (flipx)  { startx = sx + (dimx - 1) * 16; endx = sx - 16; incx = -16; }
		else { startx = sx; endx = sx + dimx * 16; incx = +16; }

		for (int x = startx ; x != endx ; x += incx)
		{
			m_gfxdecode->gfx(0)->prio_transpen(bitmap, cliprect,
						code++,
						attr & 0x1f,
						flipx, flipy,
						x, sy,
						screen.priority(),
						pri_mask, 0x00);
		}
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

uint32_t burglarx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	m_tilemap[0]->set_scrollx(0, m_scroll[0x00]);
	m_tilemap[0]->set_scrolly(0, m_scroll[0x01]);

	m_tilemap[1]->set_scrollx(0, m_scroll[0x05]);
	m_tilemap[1]->set_scrolly(0, m_scroll[0x0a]);

	m_tilemap[2]->set_scrollx(0, m_scroll[0x04]);
	m_tilemap[2]->set_scrolly(0, m_scroll[0x02]);

#ifdef MAME_DEBUG
if ( machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	// The background color is the first of the last palette
	bitmap.fill(0x1f00, cliprect);
	screen.priority().fill(0, cliprect);

	if (layers_ctrl & 1)    m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);
	if (layers_ctrl & 2)    m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 2);
	if (layers_ctrl & 4)    m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 4);

	// Sprites are drawn last, using pdrawgfx
	if (layers_ctrl & 8)    draw_sprites(screen, bitmap, cliprect);

	return 0;
}


/***************************************************************************


                                Memory Maps


***************************************************************************/

/***************************************************************************
                                Burglar X
***************************************************************************/

void burglarx_state::okibank_w(uint8_t data)
{
	m_oki->set_rom_bank(data & 1);
}

void burglarx_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0xff0000, 0xffffff).ram();
	map(0x800000, 0x800001).portr("INPUTS");
	map(0x800018, 0x800019).portr("SYSTEM");
	map(0x80001a, 0x80001b).portr("DSW1");
	map(0x80001c, 0x80001d).portr("DSW2");
	map(0x800030, 0x800031).nopw(); // ? 0
	map(0x80010c, 0x800121).rw(FUNC(burglarx_state::scroll_r), FUNC(burglarx_state::scroll_w));
	map(0x800189, 0x800189).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80018a, 0x80018a).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x80018c, 0x80018c).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x80018e, 0x80018e).w(FUNC(burglarx_state::okibank_w));
	map(0x8001e0, 0x8001e1).nopw(); // IRQ Ack
	map(0x904000, 0x90ffff).rw(FUNC(burglarx_state::vram_r), FUNC(burglarx_state::vram_w)); // Layers 1, 2, 0
	map(0x920000, 0x923fff).ram(); // ? 0
	map(0x930000, 0x9307ff).rw(FUNC(burglarx_state::spriteram_r), FUNC(burglarx_state::spriteram_w));
	map(0x940000, 0x947fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}



/***************************************************************************
                                Zero Point
***************************************************************************/

void zeropnt_state::okibank_leds_w(uint8_t data)
{
	/* Banked sound samples. The 3rd quarter of the ROM
	   contains garbage. Indeed, only banks 0&1 are used */

	m_okibank->set_entry(data & 1);

	machine().bookkeeping().coin_counter_w(0, data & 0x10);
	m_leds[0] = BIT(data, 3); // Start 1
	m_leds[1] = BIT(data, 2); // Start 2
}

// Light Gun - need to wiggle the input slightly otherwise fire doesn't work
template <uint8_t Which>
uint16_t zeropnt_state::gunx_msb_r()
{
	int x = m_gun_axes[Which]->read();

	x = x * 384 / 256; // On screen pixel X
	if (x < 0x160) x = 0x30 + (x * 0xd0 / 0x15f);
	else x = ((x - 0x160) * 0x20) / 0x1f;

	return ((x & 0xff) ^ (m_screen->frame_number() & 1)) << 8;
}

template <uint8_t Which>
uint16_t zeropnt_state::guny_msb_r()
{
	int y = m_gun_axes[Which]->read();

	y = 0x18 + ((y * 0xe0) / 0xff);

	return ((y & 0xff) ^ (m_screen->frame_number() & 1)) << 8;
}

void zeropnt_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0xef0000, 0xefffff).ram();
	map(0x800030, 0x800031).nopw(); // ? 0
	map(0x800018, 0x800019).portr("INPUTS");
	map(0x80001a, 0x80001b).portr("DSW1");
	map(0x80001c, 0x80001d).portr("DSW2");
	map(0x80010c, 0x800121).rw(FUNC(zeropnt_state::scroll_r), FUNC(zeropnt_state::scroll_w));
	map(0x800170, 0x800171).r(FUNC(zeropnt_state::guny_msb_r<Y0>));
	map(0x800174, 0x800175).r(FUNC(zeropnt_state::gunx_msb_r<X0>));
	map(0x800178, 0x800179).r(FUNC(zeropnt_state::guny_msb_r<Y1>));
	map(0x80017c, 0x80017d).r(FUNC(zeropnt_state::gunx_msb_r<X1>));
	map(0x800189, 0x800189).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80018a, 0x80018a).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x80018c, 0x80018c).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x80018e, 0x80018e).w(FUNC(zeropnt_state::okibank_leds_w));
	map(0x8001e0, 0x8001e1).nopw(); // ? IRQ Ack
	map(0x904000, 0x90ffff).rw(FUNC(zeropnt_state::vram_r), FUNC(zeropnt_state::vram_w)); // Layers 1, 2, 0
	map(0x920000, 0x923fff).ram(); // ? 0
	map(0x930000, 0x9307ff).rw(FUNC(zeropnt_state::spriteram_r), FUNC(zeropnt_state::spriteram_w));
	map(0x940000, 0x947fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void zeropnt_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}


/***************************************************************************
                                Zero Point 2
***************************************************************************/

template <uint8_t Which> uint32_t zeropnt2_state::gunx_msb_r() { return (zeropnt_state::gunx_msb_r<Which>() - 0x0800) << 16; }
template <uint8_t Which> uint32_t zeropnt2_state::guny_msb_r() { return (zeropnt_state::guny_msb_r<Which>() + 0x0800) << 16; }

void zeropnt2_state::okibank(uint8_t data)
{
	m_okibank->set_entry((data & 3) % 4);
}

void zeropnt2_state::leds_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	m_leds[0] = BIT(data, 7); // Start 1
	m_leds[1] = BIT(data, 6); // Start 2
}

void zeropnt2_state::eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (data & ~0xfe00000)
		logerror("%s - Unknown EEPROM bit written %04X\n",machine().describe_context(),data);

	if (ACCESSING_BITS_24_31)
	{
		// latch the bit
		m_eeprom->di_write((data & 0x04000000) >> 26);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x01000000) ? ASSERT_LINE : CLEAR_LINE);

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

void zeropnt2_state::program_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x800018, 0x80001b).portr("SYSTEM");
	map(0x800025, 0x800025).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x800028, 0x80002f).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask32(0x00ff0000);
	map(0x800031, 0x800031).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x800034, 0x800034).w(FUNC(zeropnt2_state::okibank));
	map(0x800039, 0x800039).w(FUNC(zeropnt2_state::leds_w));
	map(0x80010c, 0x800123).rw(FUNC(zeropnt2_state::scroll_r), FUNC(zeropnt2_state::scroll_w));
	map(0x800140, 0x800143).r(FUNC(zeropnt2_state::guny_msb_r<Y0>));
	map(0x800144, 0x800147).r(FUNC(zeropnt2_state::gunx_msb_r<X0>));
	map(0x800148, 0x80014b).r(FUNC(zeropnt2_state::guny_msb_r<Y1>));
	map(0x80014c, 0x80014f).r(FUNC(zeropnt2_state::gunx_msb_r<X1>));
	map(0x800150, 0x800153).portr("DSW1");
	map(0x800154, 0x800157).portr("DSW2");
	map(0x80015c, 0x80015f).portr("BUTTONS");
	map(0x8001e0, 0x8001e3).nopw(); // ? IRQ Ack
	map(0x8001f0, 0x8001f3).w(FUNC(zeropnt2_state::eeprom_w));
	map(0x904000, 0x90ffff).rw(FUNC(zeropnt2_state::vram_r), FUNC(zeropnt2_state::vram_w)); // Layers 1, 2, 0
	map(0x920000, 0x923fff).ram(); // ? 0
	map(0x930000, 0x9307ff).rw(FUNC(zeropnt2_state::spriteram_r), FUNC(zeropnt2_state::spriteram_w));
	map(0x940000, 0x947fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xfe0000, 0xffffff).ram();
}


/***************************************************************************


                                Input Ports


***************************************************************************/

/***************************************************************************
                                Burglar X
***************************************************************************/

static INPUT_PORTS_START( burglarx )
	PORT_START("INPUTS")    // $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $800019.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // $80001a.b
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )                    PORT_DIPLOCATION("DIP-A:1")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("DIP-A:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown A-3" )            PORT_DIPLOCATION("DIP-A:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("DIP-A:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown A-5" )            PORT_DIPLOCATION("DIP-A:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )       PORT_DIPLOCATION("DIP-A:6,7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")  // $80001c.b
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Bonus_Life ) )    PORT_DIPLOCATION("DIP-B:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0300, "A" )
	PORT_DIPSETTING(      0x0100, "B" )
	PORT_DIPSETTING(      0x0000, "C" )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown B-3" )            PORT_DIPLOCATION("DIP-B:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Energy" )                 PORT_DIPLOCATION("DIP-B:4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0800, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("DIP-B:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )         PORT_DIPLOCATION("DIP-B:7,8")
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
INPUT_PORTS_END



/***************************************************************************
                                Zero Point
***************************************************************************/

static INPUT_PORTS_START( zeropnt )
	PORT_START("INPUTS")    // $800018.w
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("DSW1")  // $80001a.b
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-A:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("DIP-A:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-A:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("DIP-A:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-A:!5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coinage ) )       PORT_DIPLOCATION("DIP-A:!6,!7,!8")
	PORT_DIPSETTING(      0xe000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")  // $80001c.b
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-B:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-B:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-B:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unused ) )        PORT_DIPLOCATION("DIP-B:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x0000, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("DIP-B:!5,!6")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Lives ) )         PORT_DIPLOCATION("DIP-B:!7,!8")
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0xc000, "5" )

	PORT_START("Y0")    // $800170.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("X0")    // $800174.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("Y1")    // $800178.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("X1")    // $80017c.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)
INPUT_PORTS_END



/***************************************************************************
                                Zero Point 2
***************************************************************************/

static INPUT_PORTS_START( zeropnt2 )
	PORT_START("SYSTEM")    // $800019.b
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00040000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xff000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")  // $80001a.b
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x01000000, 0x01000000, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("DIP-A:1")
	PORT_DIPSETTING(          0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, "1 Coin to Continue" )     PORT_DIPLOCATION("DIP-A:2") // Used when coinage is NOT 1 Coin 1 Credit
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )        // Must insert number of coins as per coinage setting to continue
	PORT_DIPSETTING(          0x02000000, DEF_STR( On ) )         // Only 1 coin is required to continue regardless of the coinage setting
	PORT_DIPNAME( 0x0c000000, 0x0c000000, "Gun Reloading" )          PORT_DIPLOCATION("DIP-A:3,4")
	PORT_DIPSETTING(          0x08000000, DEF_STR(No) )
	PORT_DIPSETTING(          0x04000000, DEF_STR(Yes) )
	PORT_DIPSETTING(          0x0c000000, "Factory Setting" )
	PORT_DIPSETTING(          0x00000000, "unused?" )
	PORT_DIPNAME( 0x10000000, 0x10000000, DEF_STR( Language ) )      PORT_DIPLOCATION("DIP-A:5")
	PORT_DIPSETTING(          0x10000000, DEF_STR( English ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0xe0000000, 0xe0000000, DEF_STR( Coinage ) )       PORT_DIPLOCATION("DIP-A:6,7,8")
	PORT_DIPSETTING(          0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(          0x20000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x40000000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x60000000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0xe0000000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0xc0000000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0xa0000000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x80000000, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")  // $80001c.b
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x01000000, 0x01000000, "Korean Language" )        PORT_DIPLOCATION("DIP-B:1")
	PORT_DIPSETTING(          0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x00000000, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("DIP-B:2")
	PORT_DIPSETTING(          0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c000000, 0x1c000000, DEF_STR( Lives ) )         PORT_DIPLOCATION("DIP-B:3,4,5")
	PORT_DIPSETTING(          0x10000000, "2" )
	PORT_DIPSETTING(          0x0c000000, "3" )
	PORT_DIPSETTING(          0x1c000000, "4" )
	PORT_DIPSETTING(          0x18000000, "5" )
	PORT_DIPSETTING(          0x14000000, "6" )
	PORT_DIPSETTING(          0x08000000, "4 (duplicate)" )
	PORT_DIPSETTING(          0x04000000, "4 (duplicate)" )
	PORT_DIPSETTING(          0x00000000, "4 (duplicate)" )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DIP-B:6")
	PORT_DIPSETTING(          0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0000000, 0xc0000000, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("DIP-B:7,8")
	PORT_DIPSETTING(          0x80000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0xc0000000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x40000000, DEF_STR( Harder ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Hardest ) )

	PORT_START("BUTTONS")   // $80015c.b
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("Y0")    // $800140.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("X0")    // $800144.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("Y1")    // $800148.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("X1")    // $80014c.b
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)
INPUT_PORTS_END



/***************************************************************************


                            Graphics Layouts


***************************************************************************/

// 16x16x8
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{   RGN_FRAC(3,4)+8,    RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,    RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,    RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,    RGN_FRAC(0,4)+0 },
	{   STEP8(0,1),         STEP8(16,1)     },
	{   STEP16(0,16*2)                      },
	16*16*2
};

static GFXDECODE_START( gfx_unico )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x8, 0x0, 0x20 )
	GFXDECODE_ENTRY( "tiles",   0, layout_16x16x8, 0x0, 0x20 )
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/


void burglarx_state::machine_start()
{
	m_leds.resolve();
}


/***************************************************************************
                                Burglar X
***************************************************************************/

void burglarx_state::burglarx(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // 16MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &burglarx_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(burglarx_state::irq2_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 224);
	screen.set_visarea(0, 384-1, 0, 224-1);
	screen.set_screen_update(FUNC(burglarx_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_unico);
	PALETTE(config, m_palette).set_format(4, &burglarx_state::unico_R6G6B6X, 8192);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(14'318'181) / 4)); // 3.579545 MHz
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.40, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.40, 1);

	OKIM6295(config, m_oki, 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.80, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.80, 1);
}



/***************************************************************************
                                Zero Point
***************************************************************************/

void zeropnt_state::machine_start()
{
	burglarx_state::machine_start();
	m_okibank->configure_entries(0, 4, memregion("oki")->base() + 0x20000, 0x20000);
}

void zeropnt_state::zeropnt(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // 16MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &zeropnt_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(zeropnt_state::irq2_line_hold));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(384, 224);
	m_screen->set_visarea(0, 384-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(zeropnt_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_unico);
	PALETTE(config, m_palette).set_format(4, &zeropnt_state::unico_R6G6B6X, 8192);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(14'318'181) / 4)); // 3.579545 MHz
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.40, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.40, 1);

	OKIM6295(config, m_oki, 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 verified
	m_oki->set_addrmap(0, &zeropnt_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.80, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.80, 1);
}



/***************************************************************************
                                Zero Point 2
***************************************************************************/

void zeropnt2_state::machine_start()
{
	burglarx_state::machine_start();
	m_okibank->configure_entries(0, 4, memregion("oki1")->base() + 0x20000, 0x20000);
}

void zeropnt2_state::zeropnt2(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, 32_MHz_XTAL / 2); // 16MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &zeropnt2_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(zeropnt2_state::irq2_line_hold));

	EEPROM_93C46_8BIT(config, "eeprom");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(384, 224);
	m_screen->set_visarea(0, 384-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(zeropnt2_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_unico);
	PALETTE(config, m_palette).set_format(4, &zeropnt2_state::unico_R6G6B6X, 8192);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(14'318'181) / 4).add_route(0, "speaker", 0.70, 0).add_route(1, "speaker", 0.70, 1); // 3.579545 MHz

	okim6295_device &oki1(OKIM6295(config, "oki1", 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki1.set_addrmap(0, &zeropnt2_state::oki_map);
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.40, 0);
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.40, 1);

	okim6295_device &oki2(OKIM6295(config, "oki2", XTAL(14'318'181)/4, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki2.add_route(ALL_OUTPUTS, "speaker", 0.20, 0);
	oki2.add_route(ALL_OUTPUTS, "speaker", 0.20, 1);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/



/***************************************************************************

                                Burglar X

by Unico

68000-16MHz , MACH210-15JC, 3 x A1020B
14.31818 MHz, 32.000 MHz

***************************************************************************/

ROM_START( burglarx )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bx-rom2.pgm", 0x000000, 0x080000, CRC(f81120c8) SHA1(f0240cf9aceb755e3c920bc3bcae0a9de29fd8c1) )
	ROM_LOAD16_BYTE( "bx-rom3.pgm", 0x000001, 0x080000, CRC(080b4e82) SHA1(7eb08a7ea7684297e879123ae7ddc88d7fc1b87b) )

	// Notice the weird ROMs order? Pretty much bit scrambling
	ROM_REGION( 0x400000, "sprites", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD16_BYTE( "bx-rom4",  0x000000, 0x080000, CRC(f74ce31f) SHA1(bafe247a2fdc918318ccf7b11f0406c78909dcaa) )
	ROM_LOAD16_BYTE( "bx-rom10", 0x000001, 0x080000, CRC(6f56ca23) SHA1(5cfedda8d9fe4b575932a6a136d7b525d96e5454) )
	ROM_LOAD16_BYTE( "bx-rom9",  0x100000, 0x080000, CRC(33f29d79) SHA1(287d8412842887af5a5c7a0f5e5736a741c3c7db) )
	ROM_LOAD16_BYTE( "bx-rom8",  0x100001, 0x080000, CRC(24367092) SHA1(dc21d043a793cbc9fe94085c7884d684f1f80d74) )
	ROM_LOAD16_BYTE( "bx-rom7",  0x200000, 0x080000, CRC(aff6bdea) SHA1(3c050ec5e1bbc93b15435c7a6e66bade9a07445e) )
	ROM_LOAD16_BYTE( "bx-rom6",  0x200001, 0x080000, CRC(246afed2) SHA1(fcf08e968f11549546c47c1a67013c2427e0aad3) )
	ROM_LOAD16_BYTE( "bx-rom11", 0x300000, 0x080000, CRC(898d176a) SHA1(4c85948b7e639743d0f1676fdc463267f550f97c) )
	ROM_LOAD16_BYTE( "bx-rom5",  0x300001, 0x080000, CRC(fdee1423) SHA1(319610435b3dea61276d412e2bf6a3f32809ae19) )

	ROM_REGION( 0x400000, "tiles", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD16_BYTE( "bx-rom14", 0x000000, 0x080000, CRC(30413373) SHA1(37bbc4d2943a32ee9f6bb268c823ffe162fe92a2) )
	ROM_LOAD16_BYTE( "bx-rom18", 0x000001, 0x080000, CRC(8e7fc99f) SHA1(81141e3c9111944aae97d27e5631b11eaf6f8734) )
	ROM_LOAD16_BYTE( "bx-rom19", 0x100000, 0x080000, CRC(d40eabcd) SHA1(e41d5e921a1648d6d4907f18e0256dbe3a01e9d3) )
	ROM_LOAD16_BYTE( "bx-rom15", 0x100001, 0x080000, CRC(78833c75) SHA1(93bd2e9ba98d99e36b99765ff576df4ca347daf3) )
	ROM_LOAD16_BYTE( "bx-rom17", 0x200000, 0x080000, CRC(f169633f) SHA1(3bb707110286890a740ef607fb2addeeaadedb08) )
	ROM_LOAD16_BYTE( "bx-rom12", 0x200001, 0x080000, CRC(71eb160f) SHA1(4fc8caabc5ee6c7771c76e704ffba675cf997dae) )
	ROM_LOAD16_BYTE( "bx-rom13", 0x300000, 0x080000, CRC(da34bbb5) SHA1(455c2412135b89670c2ecda9fd02f4da9b891ee4) )
	ROM_LOAD16_BYTE( "bx-rom16", 0x300001, 0x080000, CRC(55b28ef9) SHA1(48615d53ac955ba6aca86ad4f8b61f4d2675d840) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bx-rom1.snd", 0x000000, 0x080000, CRC(8ae67138) SHA1(3ea44f805a1f978e0a1c1bb7f45507379b147bc0) ) // 2 x 40000
ROM_END


/***************************************************************************

                                Zero Point

(C) 1998 Unico

PCB Number: ZPM1001A
CPU: 68HC000P16
SND: K-664/K-666 & AD-65 (YM3014/YM3812 & M6295)
OSC: 14.31818MHz, 32.000MHz
RAM: 62256 x 5, 6116 x 8, 84256 x 2
DIPS: 2 x 8 position

Other Chips: 3 x Actel A1020B (square 84 pin socketed, Same video chip as Power Instinct and Blomby Car)
             MACH211 (square 44 pin socketed)

There is a small gun interface board (Number ZPT1001B) located near the 68000 which contains
another Actel A1020B chip, a 74HC14 TTL chip and a 4.9152MHz OSC.

ROMS:
zero2.BIN  \
zero3.BIN  / Main Program 4M Mask ROMs
zero1.BIN  -- Sound MX27C4000
zpobjz01.BIN -\
zpobjz02.BIN   \
zpobjz03.BIN    \
zpobjz04.BIN     \
zpscrz05.BIN      - GFX,16M Mask ROMs
zpscrz06.BIN     /
zpscrz07.BIN    /
zpscrz08.BIN  -/

***************************************************************************/

ROM_START( zeropnt )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "unico_2.rom2", 0x000000, 0x080000, CRC(1e599509) SHA1(5a562a3c85700126b95fbdf21ef8c0ddd35d9037) )
	ROM_LOAD16_BYTE( "unico_3.rom3", 0x000001, 0x080000, CRC(588aeef7) SHA1(0dfa22c9e7b1fe493c16160b1ac76fa4d3bb2e68) )

	ROM_REGION( 0x800000, "sprites", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "unico_zpobj_z01.bin", 0x000000, 0x200000, CRC(1f2768a3) SHA1(75c83458afc527dda47bfbd86a8e9c5ded7a5444) )
	ROM_LOAD( "unico_zpobj_z02.bin", 0x200000, 0x200000, CRC(de34f33a) SHA1(b77c7d508942176585afaeeaea2f34f60326eeb1) )
	ROM_LOAD( "unico_zpobj_z03.bin", 0x400000, 0x200000, CRC(d7a657f7) SHA1(f1f9e6a01eef4d0c8c4b2e161136cc4438d770e2) )
	ROM_LOAD( "unico_zpobj_z04.bin", 0x600000, 0x200000, CRC(3aec2f8d) SHA1(6fb1cfabfb0bddf688d3bfb60f7538209efbd8f1) )

	ROM_REGION( 0x800000, "tiles", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "unico_zpscr_z06.bin", 0x000000, 0x200000, CRC(e1e53cf0) SHA1(b440e09f6229d486d1a8be476ac8a17adde1ff7e) )
	ROM_LOAD( "unico_zpscr_z05.bin", 0x200000, 0x200000, CRC(0d7d4850) SHA1(43f87d0461fe022b68b4e57e6c9542bcd78e301b) )
	ROM_LOAD( "unico_zpscr_z07.bin", 0x400000, 0x200000, CRC(bb178f32) SHA1(1354f4d90a8cec58d1f2b6809985776b309b96a8) )
	ROM_LOAD( "unico_zpscr_z08.bin", 0x600000, 0x200000, CRC(672f02e5) SHA1(8e8b28a8b2293950764d453a3c385d7083eb5a57) )

	ROM_REGION( 0x80000 * 2, "oki", 0 )
	ROM_LOAD( "unico_1.rom1", 0x000000, 0x080000, CRC(fd2384fa) SHA1(8ae83665fe952c5d03bd62d2abb507c351cf0fb5) )
ROM_END


ROM_START( zeropnta )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "unico2.rom2", 0x000000, 0x080000, CRC(285fbca3) SHA1(61f8d48388a666ed9300c0688fbf844e316b8892) )
	ROM_LOAD16_BYTE( "unico3.rom3", 0x000001, 0x080000, CRC(ad7b3129) SHA1(d814b5d9336d011386aa0b316b11225e5ea799fc) )

	ROM_REGION( 0x800000, "sprites", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "unico_zpobj_z01.bin", 0x000000, 0x200000, CRC(1f2768a3) SHA1(75c83458afc527dda47bfbd86a8e9c5ded7a5444) )
	ROM_LOAD( "unico_zpobj_z02.bin", 0x200000, 0x200000, CRC(de34f33a) SHA1(b77c7d508942176585afaeeaea2f34f60326eeb1) )
	ROM_LOAD( "unico_zpobj_z03.bin", 0x400000, 0x200000, CRC(d7a657f7) SHA1(f1f9e6a01eef4d0c8c4b2e161136cc4438d770e2) )
	ROM_LOAD( "unico_zpobj_z04.bin", 0x600000, 0x200000, CRC(3aec2f8d) SHA1(6fb1cfabfb0bddf688d3bfb60f7538209efbd8f1) )

	ROM_REGION( 0x800000, "tiles", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "unico_zpscr_z06.bin", 0x000000, 0x200000, CRC(e1e53cf0) SHA1(b440e09f6229d486d1a8be476ac8a17adde1ff7e) )
	ROM_LOAD( "unico_zpscr_z05.bin", 0x200000, 0x200000, CRC(0d7d4850) SHA1(43f87d0461fe022b68b4e57e6c9542bcd78e301b) )
	ROM_LOAD( "unico_zpscr_z07.bin", 0x400000, 0x200000, CRC(bb178f32) SHA1(1354f4d90a8cec58d1f2b6809985776b309b96a8) )
	ROM_LOAD( "unico_zpscr_z08.bin", 0x600000, 0x200000, CRC(672f02e5) SHA1(8e8b28a8b2293950764d453a3c385d7083eb5a57) )

	ROM_REGION( 0x80000 * 2, "oki", 0 )
	ROM_LOAD( "unico_1.rom1", 0x000000, 0x080000, CRC(fd2384fa) SHA1(8ae83665fe952c5d03bd62d2abb507c351cf0fb5) )
ROM_END


ROM_START( zeropntj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "unico_2.bin", 0x000000, 0x080000, CRC(098d9756) SHA1(c98ae2774d2eff7d0ea66887c57d4b55d6939ad8) )
	ROM_LOAD16_BYTE( "unico_3.bin", 0x000001, 0x080000, CRC(58e105f3) SHA1(6069ec030d6ce11ec4b9514f366197068f1220ee) )

	ROM_REGION( 0x800000, "sprites", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "unico_zpobj_z01.bin", 0x000000, 0x200000, CRC(1f2768a3) SHA1(75c83458afc527dda47bfbd86a8e9c5ded7a5444) )
	ROM_LOAD( "unico_4.bin",         0x200000, 0x200000, CRC(529c36ee) SHA1(3c1d1b94b9cf84fd07689cd5fbfe86820bb878e6) ) // EPROM Containing graphics data for Japanese text
	ROM_LOAD( "unico_zpobj_z03.bin", 0x400000, 0x200000, CRC(d7a657f7) SHA1(f1f9e6a01eef4d0c8c4b2e161136cc4438d770e2) )
	ROM_LOAD( "unico_zpobj_z04.bin", 0x600000, 0x200000, CRC(3aec2f8d) SHA1(6fb1cfabfb0bddf688d3bfb60f7538209efbd8f1) )

	ROM_REGION( 0x800000, "tiles", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "unico_zpscr_z06.bin", 0x000000, 0x200000, CRC(e1e53cf0) SHA1(b440e09f6229d486d1a8be476ac8a17adde1ff7e) )
	ROM_LOAD( "unico_zpscr_z05.bin", 0x200000, 0x200000, CRC(0d7d4850) SHA1(43f87d0461fe022b68b4e57e6c9542bcd78e301b) )
	ROM_LOAD( "unico_zpscr_z07.bin", 0x400000, 0x200000, CRC(bb178f32) SHA1(1354f4d90a8cec58d1f2b6809985776b309b96a8) )
	ROM_LOAD( "unico_zpscr_z08.bin", 0x600000, 0x200000, CRC(672f02e5) SHA1(8e8b28a8b2293950764d453a3c385d7083eb5a57) )

	ROM_REGION( 0x80000 * 2, "oki", 0 )
	ROM_LOAD( "unico_1.rom1", 0x000000, 0x080000, CRC(fd2384fa) SHA1(8ae83665fe952c5d03bd62d2abb507c351cf0fb5) )
ROM_END

/***************************************************************************

                                    Zero Point 2

(c) 1999 Unico

PCB Number: UZP21001A
CPU: MC68EC020FG16
SND: YM3012/YM2151 & AD-65 x 2 (OKI M6295)
OSC: 3.579545MHz (near AD-65), 32.000MHz, 40.000MHz (near 68020)
RAM: 62256B x 9

Other Chips: 3 x Actel A40MX04-F (square 84 pin socketed)
             MACH211 (square 44 pin socketed)

There is a small gun interface board (Number UZP21001B) located above the 68020 which contains:

   OSC: 4.9152MHz
  DIPS: 2 x 8 position
EEPROM: ST 93C46
 OTHER: Actel A40MX04-F chip
        74HC14 TTL chip
        4-pin gun header x 2

ROMS:
D0-D15.3  \ Main Program 8M Mask ROMs
D16-D31.4 /
uzp2-1.bin - Sound 27C040
uzp2-2.bin - Sound 27C020

A0-A1ZP.205  -\
A2-A3ZP.206    \
A6-A7ZP.207     \
A4-A5ZP.208      \
DB0DB1ZP.209      - GFX,32M Mask ROMs
DB2DB3ZP.210     /
DB4DB5ZP.211    /
DB6DB7ZP.212  -/

                     Zero Point 2 board JAMMA Pinout

                          Main Jamma Connector
            Solder Side          |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
                             | E | 5 |
             +12             | F | 6 |             +12
------------ KEY ------------| H | 7 |------------ KEY -----------
                             | J | 8 |      Coin Counter # 1
       Player 2 Lamp         | K | 9 |       Player 1 Lamp
        Speaker (L)          | L | 10|        Speaker (R)
                             | M | 11|
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
       Service Switch        | R | 14|        Video GND
                             | S | 15|        Test Switch
        Coin Switch 2        | T | 16|         Coin Switch 1
       Player 2 Start        | U | 17|        Player 1 Start
                             | V | 18|
                             | W | 19|
                             | X | 20|
                             | Y | 21|
                             | Z | 22|
                             | a | 23|
                             | b | 24|
                             | c | 25|
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND



SPECIAL NOTICE - Sound wiring change:

For cabinets with one speaker:
    JAMMA pin 10 goes to speaker (+)
    Run a ground to the negative side of speaker.
For cabinets with two speakers:
    JAMMA pin 10 goes to right speaker (+)
    JAMMA pin L goes to left speaker (+)
    Run a ground to the negative side of each speaker.

Using Original Unico Light Guns & connectors:

 1PLAY: Left (Red) Gun Connector Pinout*

   1| Gun OPTO - White Wire
   2| +5 Volts - Red Wire
   3| Trigger  - Green Wire
   4| Ground   - Black Wire

 2PLAY: Right (Blue) Gun Connector Pinout*

   1| Gun OPTO - White Wire
   2| +5 Volts - Red Wire
   3| Trigger  - Green Wire
   4| Ground   - Black Wire

* This is not the same as the HAPP Controls' 4-pin standard


DIPSW-A
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
    Free Play         |   Off    |off|                           |*
                      |   On     |on |                           |
------------------------------------------------------------------
  1 Coin to Continue  |   Off    |   |off|                       |*
                      |   On     |   |on |                       |
------------------------------------------------------------------
                      | Factory  |       |off|off|               |*
   Gun Loading Mode   |Not Reload|       |on |off|               |
                      | Reload   |       |off|on |               |
------------------------------------------------------------------
      Language        | English  |               |off|           |*
                      | Japanese |               |on |           |
------------------------------------------------------------------
                      | 1cn/1pl  |                   |off|off|off|*
                      | 1cn/2pl  |                   |on |off|off|
                      | 1cn/3pl  |                   |off|on |off|
        Coinage       | 1cn/4pl  |                   |on |on |off|
                      | 2cn/1pl  |                   |off|off|on |
                      | 3cn/1pl  |                   |on |off|on |
                      | 4cn/1pl  |                   |off|on |on |
                      | 5cn/1pl  |                   |on |on |on |
------------------------------------------------------------------

DIPSW-B
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
   Korean Language    |   Off    |off|                           |*
                      |   On     |on |                           |
------------------------------------------------------------------
     Demo Sounds      |   Off    |   |off|                       |
                      |   On     |   |on |                       |*
------------------------------------------------------------------
                      |    4     |       |off|off|off|           |*
   Player's Heart     |    5     |       |on |off|off|           |
       (Lives)        |    6     |       |off|on |off|           |
                      |    2     |       |on |on |off|           |
                      |    3     |       |off|off|on |           |
------------------------------------------------------------------
  Not Used / Always Off                              |off|       |*
------------------------------------------------------------------
                      |  Normal  |                       |off|off|*
      Difficulty      |   Easy   |                       |on |off|
        Level         |   Hard   |                       |off|on |
                      |  V.Hard  |                       |on |on |
------------------------------------------------------------------

* Denotes Factory Defaults


BrianT

***************************************************************************/

ROM_START( zeropnt2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68020 code
	ROM_LOAD32_WORD_SWAP( "d16-d31.4", 0x000000, 0x100000, CRC(48314fdb) SHA1(a5bdb6a3f520587ff5e73438dc414cfdff34167b) )
	ROM_LOAD32_WORD_SWAP( "d0-d15.3",  0x000002, 0x100000, CRC(5ec4151e) SHA1(f7c857bdb6a92f76f09a089b37def7e6cf24b65a) )

	ROM_REGION( 0x1000000, "sprites", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "db0db1zp.209", 0x000000, 0x400000, CRC(474b460c) SHA1(72104b7a00cb6d62b3cee2cfadc928669ca948c4) )
	ROM_LOAD( "db2db3zp.210", 0x400000, 0x400000, CRC(0a1d0a88) SHA1(b0a6ba9eba539fff417557c9af60d408c2912491) )
	ROM_LOAD( "db4db5zp.211", 0x800000, 0x400000, CRC(227169dc) SHA1(b03d8d46714e5aa3631fde7d65466334dafdc341) )
	ROM_LOAD( "db6db7zp.212", 0xc00000, 0x400000, CRC(a6306cdb) SHA1(da48c5981b72b87df40602e03e56a40a24728262) )

	ROM_REGION( 0x1000000, "tiles", ROMREGION_INVERT ) // 16x16x8
	ROM_LOAD( "a0-a1zp.205", 0x000000, 0x400000, CRC(f7ca9c0e) SHA1(541139b617ff34c378a506cf88fe97234c93ee20) )
	ROM_LOAD( "a2-a3zp.206", 0x400000, 0x400000, CRC(0581c8fe) SHA1(9bbffc9c758bbaba2b43a63811b725e51996268a) )
	ROM_LOAD( "a4-a5zp.208", 0x800000, 0x400000, CRC(ddd091ef) SHA1(c1751aef2546a35f2fdbfeca9647a88fd3e65cdd) )
	ROM_LOAD( "a6-a7zp.207", 0xc00000, 0x400000, CRC(3fd46113) SHA1(326684b92c258bde318693cd9b3a7660aed3cd6f) )

	ROM_REGION( 0x80000 * 2, "oki1", 0 )
	ROM_LOAD( "uzp2-1.bin", 0x000000, 0x080000, CRC(ed0966ed) SHA1(a43b9c493f94d1fb11e1b189caaf37d3d792c730) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "uzp2-2.bin", 0x000000, 0x040000, CRC(db8cb455) SHA1(6723b4018208d554bd1bf1e0640b72d2f4f47302) )
ROM_END

} // anonymous namespace


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1997, burglarx, 0,       burglarx, burglarx, burglarx_state, empty_init, ROT0, "Unico", "Burglar X" ,         MACHINE_SUPPORTS_SAVE )
GAME( 1998, zeropnt,  0,       zeropnt,  zeropnt,  zeropnt_state,  empty_init, ROT0, "Unico", "Zero Point (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, zeropnta, zeropnt, zeropnt,  zeropnt,  zeropnt_state,  empty_init, ROT0, "Unico", "Zero Point (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, zeropntj, zeropnt, zeropnt,  zeropnt,  zeropnt_state,  empty_init, ROT0, "Unico", "Zero Point (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, zeropnt2, 0,       zeropnt2, zeropnt2, zeropnt2_state, empty_init, ROT0, "Unico", "Zero Point 2",       MACHINE_SUPPORTS_SAVE )
