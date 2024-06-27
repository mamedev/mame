// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                     -= Sigma B-98 Hardware / Sammy Medal Games =-

                                                 driver by Luca Elia

CPU     :   TAXAN KY-80 (Yamaha)
Video   :   TAXAN KY-3211 or KY-10510
Sound   :   YMZ280B or OKI M9811
NVRAM   :   93C46 and/or battery backed RAM

Graphics are made of sprites only.
Each sprite is composed of X x Y tiles and can be zoomed / shrunk and rotated.
Tiles can be 16x16x4 or 16x16x8.

Some videos:
https://www.youtube.com/channel/UCYFiPd3FukrmNJa3pfIKf-Q/search?query=sigma
https://www.youtube.com/channel/UCYFiPd3FukrmNJa3pfIKf-Q/search?query=sammy
https://www.youtube.com/user/analysis08/search?query=sammy
http://www.nicozon.net/watch/sm14334996

Dumped games:                           ROMs:    Video:

1997 Minna Atsumare! Dodge Hero         B9802   https://youtu.be/2eXDQnKCT6A
1997 Itazura Daisuki! Sushimaru Kun     B9803   https://youtu.be/nhvbZ71KWr8
1997 GeGeGe no Kitarou Youkai Slot      B9804
1997 Burning Sanrinsya                  B9805
1997 PEPSI Man                          B9806   https://youtu.be/p3cbZ67m4lo
1998 Transformers Beast Wars II         B9808
1997 Uchuu Tokkyuu Medalian             B9809   https://youtu.be/u813kBOZbwI
2000 Minna Ganbare! Dash Hero           B9811

2001 Otakara Itadaki Luffy Kaizoku-Dan! KA108   https://youtu.be/D_4bWx3tTPw

--------------------------------------------------------------------------------------

Sammy Kids Medal Series

CPU     :   Kawasaki KL5C80A12CFP (Z80 Compatible High Speed Microcontroller)
Video   :   TAXAN KY-3211
Sound   :   OKI M9810B
NVRAM   :   93C46 and battery backed RAM

Cartridge based system. Carts contain just some 16Mb flash eeproms.

Info from Tatsuya Fujita:

According to some news articles for the AOU show 2002 and 2003 the correct system name
is "Treasure Fall" (despite the cart label is "Treasure Hall").

Dumped games:

2000 Animal Catch                 https://youtu.be/U4L5EwWbxqw
2000 Itazura Monkey               https://youtu.be/GHxiqUQRpV8
2000 Pye-nage Taikai              https://youtu.be/oL2OIbrv-KI
2000 Taihou de Doboon             https://youtu.be/loPP3jt0Ob0
2001 Hae Hae Ka Ka Ka             https://youtu.be/37IxYCg0tic
2003 Go Go Cowboy (EN, prize)     https://youtu.be/rymtzmSXjuA

Games with the same cabinet, or in the Treasure Fall series, which might be on the same hardware:

1999 Shatekids                    https://youtu.be/aWzYlvm6uIs
1999 Otakara Locomo               https://youtu.be/J0NwMWO3SdY
1999 Dokidoki Kingyosukui         https://youtu.be/Z0tOjG_tteU
2000 Otoshicha Ottotto            https://youtu.be/AybhPHTFvMo
2001 Mushitori Meijin
2001 Morino Dodgeball Senshuken   https://youtu.be/k98KIRjTYbY
2001 Waiwai Wanage                https://youtu.be/4GmwPTk_Er4
2001 Zarigani Tsuri               https://youtu.be/NppRdebkUaQ
2001 Kotekitai Slot               https://youtu.be/IohrnGIma4A
2002 Shateki Yokochou             https://youtu.be/LPZLWP1x5o8
2002 Ipponzuri Slot
2002 Karateman                    https://youtu.be/EIrVHEAv3Sc
2002 One-touchable
2002 Perfect Goal (screenless)    https://youtu.be/ilneyp-8dBI
2003 Go Go Cowboy (JP, medal)     https://youtu.be/qYDw2sxNRqE
2003 Gun Kids
2003 Kurukuru Train               https://youtu.be/Ef7TQX4C9fA
2003 Safari Kingdom (screenless)
2003 Zakuzaku Kaizokudan
2004 Animal Punch
2004 Dotabata Zaurus              https://youtu.be/Gxt6klOYZ9A
2004 Excite Hockey (screenless)
2004 Fishing Battle (screenless)
2004 Home Run Derby (screenless)
2004 Ninchuu Densetsu             https://youtu.be/_ifev_CJROs
2004 Outer Space (screenless)
2004 Pretty Witch Pointe          https://youtu.be/lYAwfHyywfA

Original list from:
http://www.tsc-acnet.com/index.php?sort=8&action=cataloglist&s=1&mode=3&genre_id=40&freeword=%25A5%25B5%25A5%25DF%25A1%25BC

--------------------------------------------------------------------------------------

To Do:

- Consolidate the sammymdl games in one memory map and run the BIOS without ROM patches
- Remove ROM patches: gegege checks the EEPROM output after reset, and wants a timed 0->1 transition or locks up while
  saving setting in service mode. Using a reset_delay of 7 works, unless when "play style" is set
  to "coin" (it probably changes the number of reads from port $C0).
  I guess the reset_delay mechanism should be implemented with a timer in eeprom.c.
- pyenaget intro: when the theater scrolls out to the left, the train should scroll in from the right,
  with no visible gaps. It currently leaves a small gap.
- tdoboon: no smoke from hit planes as shown in the video? Tiles are present (f60-125f) and used in demo mode.
- dashhero does not acknowledge the button bashing correctly, it's very hard to win (a slower pace works better!)
- dodghero and sushimar often write zeroes to 81XX1 and 00XX1 for some reason (maybe just sloppy coding?)

Notes:

- "BACKUP RAM NG" error: in test mode, choose "SET MODE" -> "RAM CLEAR" and keep the button pressed for long.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/kl5c80a12.h"
#include "cpu/z80/ky80.h"
#include "machine/74165.h"
#include "machine/eepromser.h"
#include "machine/mb3773.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim9810.h"
#include "sound/ymz280b.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sigmab98_base_state : public driver_device
{
public:
	sigmab98_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_buffered_spriteram(*this, "spriteram")
		, m_spriteram(*this, "spriteram")
		, m_vregs(*this, "vregs")
		, m_vtable(*this, "vtable")
		, m_new_sprite_chip(false)
		, m_vblank(0)
	{ }

protected:
	virtual void video_start() override;

	// TODO: unify these handlers
	void vregs_w(offs_t offset, uint8_t data);
	uint8_t vregs_r(offs_t offset);
	uint8_t d013_r();
	uint8_t d021_r();
	void vblank_w(uint8_t data);
	uint8_t vblank_r();
	uint8_t haekaka_vblank_r();

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_mask);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	[[maybe_unused]] void screen_vblank_sammymdl(int state);

	// Required devices
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	// Optional devices
	optional_device<buffered_spriteram8_device> m_buffered_spriteram;   // not on sammymdl?

	// Shared pointers
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vregs;
	required_shared_ptr<uint8_t> m_vtable;

	std::unique_ptr<bitmap_ind16> m_sprite_bitmap;

	bool m_new_sprite_chip; // KY-10510 has a slightly different sprite format than KY-3211

	uint8_t m_vblank;
};


class sigmab98_state : public sigmab98_base_state
{
public:
	sigmab98_state(const machine_config &mconfig, device_type type, const char *tag)
		: sigmab98_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_hopper(*this, "hopper")
		, m_eeprom(*this, "eeprom")
		, m_leds(*this, "led%u", 0U)
	{ }

	void sigmab98(machine_config &config);
	void dodghero(machine_config &config);

	void init_b3rinsya();
	void init_tbeastw2();
	void init_dashhero();
	void init_gegege();
	void init_pepsiman();
	void init_ucytokyu();

protected:
	void c4_w(uint8_t data);
	void c6_w(uint8_t data);
	void c8_w(uint8_t data);

	void show_outputs();
	void eeprom_w(uint8_t data);

	INTERRUPT_GEN_MEMBER(sigmab98_vblank_interrupt);

	void dodghero_mem_map(address_map &map);
	void gegege_io_map(address_map &map);
	void gegege_mem_map(address_map &map);

	virtual void machine_start() override { m_leds.resolve(); }

	// Required devices
	required_device<ky80_device> m_maincpu;
	required_device<ticket_dispenser_device> m_hopper;

	// Optional devices
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	output_finder<4> m_leds;

	uint8_t m_c0 = 0;
	uint8_t m_c4 = 0;
	uint8_t m_c6 = 0;
	uint8_t m_c8 = 0;
};


class lufykzku_state : public sigmab98_state
{
public:
	lufykzku_state(const machine_config &mconfig, device_type type, const char *tag)
		: sigmab98_state(mconfig, type, tag)
		, m_watchdog(*this, "watchdog_mb3773")
		, m_dsw_shifter(*this, "ttl165_%u", 1U)
		, m_dsw_bit(0)
	{
		m_new_sprite_chip = true;
	}

	void init_lufykzku();
	void init_rockman();
	void lufykzku(machine_config &config);
	void rockman(machine_config& config);

private:
	required_device<mb3773_device> m_watchdog;
	required_device_array<ttl165_device, 2> m_dsw_shifter;

	int m_dsw_bit;
	void dsw_w(int state);

	void lufykzku_c4_w(uint8_t data);
	void lufykzku_c6_w(uint8_t data);
	uint8_t lufykzku_c8_r();
	void lufykzku_c8_w(uint8_t data);
	void lufykzku_watchdog_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(rockman_timer_irq);

	void lufykzku_io_map(address_map &map);
	void lufykzku_mem_map(address_map &map);

	uint8_t m_vblank_vector = 0;
	uint8_t m_timer0_vector = 0;
	uint8_t m_timer1_vector = 0;
};


class sammymdl_state : public sigmab98_base_state
{
public:
	sammymdl_state(const machine_config &mconfig, device_type type, const char *tag)
		: sigmab98_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kp69(*this, "maincpu:kp69")
		, m_eeprom(*this, "eeprom")
		, m_hopper(*this, "hopper")
		, m_hopper_small(*this, "hopper_small")
		, m_hopper_large(*this, "hopper_large")
		, m_leds(*this, "led%u", 0U)
	{ }

	void haekaka(machine_config &config);
	void gocowboy(machine_config &config);
	void tdoboon(machine_config &config);
	void animalc(machine_config &config);
	void sammymdl(machine_config &config);
	void itazuram(machine_config &config);
	void pyenaget(machine_config &config);

	void init_itazuram();
	void init_animalc();
	void init_haekaka();

protected:
	virtual void machine_start() override { m_leds.resolve(); }

private:
	TIMER_DEVICE_CALLBACK_MEMBER(gocowboy_int);

	void coin_counter_w(uint8_t data);
	void leds_w(uint8_t data);
	void hopper_w(uint8_t data);
	uint8_t coin_hopper_r();

	void gocowboy_leds_w(uint8_t data);

	void haekaka_leds_w(uint8_t data);
	void haekaka_coin_counter_w(uint8_t data);

	void show_3_outputs();
	uint8_t eeprom_r();
	void eeprom_w(uint8_t data);

	void animalc_io(address_map &map);
	void animalc_map(address_map &map);
	void gocowboy_io(address_map &map);
	void gocowboy_map(address_map &map);
	void haekaka_map(address_map &map);
	void itazuram_map(address_map &map);
	void tdoboon_map(address_map &map);

	// Required devices
	required_device<kl5c80a12_device> m_maincpu;
	required_device<kp69_device> m_kp69;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	// Optional devices
	optional_device<ticket_dispenser_device> m_hopper;
	optional_device<ticket_dispenser_device> m_hopper_small;
	optional_device<ticket_dispenser_device> m_hopper_large;

	output_finder<8> m_leds;

	uint8_t m_out[3];
};


/***************************************************************************

    Video

***************************************************************************/

void sigmab98_base_state::video_start()
{
	m_sprite_bitmap = std::make_unique<bitmap_ind16>(512, 512);
}

/***************************************************************************

    Sprites (Older chip: TAXAN KY-3211. Newer chip: KY-10510)

    Offset:     Bits:         Value:

    0           7654 ----     Color (High, newer chip only?)
                ---- 3210     Color
    1           7--- ----
                -6-- ----     256 Color Sprite (older chip)
                --5- ----
                ---4 ----     Flip X (older chip)
                ---- 3---     Flip Y (older chip) / 256 Color Sprite (newer chip)
                ---- -2--     Draw Sprite
                ---- --10     Priority (0 = Front .. 3 = Back)
    2                         Tile Code (High)
    3                         Tile Code (Low)
    4           7654 3---     Number of X Tiles - 1
                ---- -2--     Flip X (newer chip)
                ---- --10     X (High)
    5                         X (Low)
    6           7654 3---     Number of Y Tiles - 1
                ---- -2--     Flip Y (newer chip)
                ---- --10     Y (High)
    7                         Y (Low)
    8                         Destination Delta X, Scaled by Shrink Factor << 8 (High)
    9                         Destination Delta X, Scaled by Shrink Factor << 8 (Low)
    a                         Destination Delta Y, Scaled by Shrink Factor << 8 (High)
    b                         Destination Delta Y, Scaled by Shrink Factor << 8 (Low)
    c           7654 3---
                ---- -210     Source X (High)
    d                         Source X (Low)
    e           7654 3---
                ---- -210     Source Y (High)
    f                         Source Y (Low)

    Sprites rotation examples:
    logo in dashhero, pepsiman https://youtu.be/p3cbZ67m4lo?t=1m24s, tdoboon https://youtu.be/loPP3jt0Ob0

***************************************************************************/

inline int integer_part(int x)
{
//  return x >> 16;
	return (x + 0x8000) >> 16;
}

void sigmab98_base_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_mask)
{
	uint8_t *end      =   (m_buffered_spriteram ? m_buffered_spriteram->buffer() : m_spriteram) - 0x10;
	uint8_t *s        =   end + 0x1000;

	for ( ; s != end; s -= 0x10 )
	{
		if ( (s[ 0x01 ] & (m_new_sprite_chip ? 0x0c : 0x04)) == 0)
			continue;

		if ( ((1 << (s[ 0x01 ] & 0x03)) & pri_mask) == 0 )
			continue;

		int color   =   s[ 0x00 ] & (m_new_sprite_chip ? 0xff : 0xf);

		int gfx     =   (s[ 0x01 ] & (m_new_sprite_chip ? 0x08 : 0x40)) ? 1 : 0;
		int code    =   s[ 0x02 ] * 256 + s[ 0x03 ];

		int nx      =   ((s[ 0x04 ] & 0xf8) >> 3) + 1;
		int dstx    =   (s[ 0x04 ] & 0x03) * 256 + s[ 0x05 ];

		int ny      =   ((s[ 0x06 ] & 0xf8) >> 3) + 1;
		int dsty    =   (s[ 0x06 ] & 0x03) * 256 + s[ 0x07 ];

		int dstdx   =   (s[ 0x08 ] & 0xff) * 256 + s[ 0x09 ];   // 0x100 = no zoom, 0x200 = 50% zoom
		int dstdy   =   (s[ 0x0a ] & 0xff) * 256 + s[ 0x0b ];   // ""

		int srcx    =   (s[ 0x0c ] & 0xff) * 256 + s[ 0x0d ];
		int srcy    =   (s[ 0x0e ] & 0xff) * 256 + s[ 0x0f ];

		// Sign extend the position
		dstx = (dstx  & 0x01ff) - (dstx  & 0x0200); // or 0x3ff/0x400?
		dsty = (dsty  & 0x01ff) - (dsty  & 0x0200);

		// Flipping
		int x0, x1, dx, flipx = m_new_sprite_chip ? s[ 0x04 ] & 0x04 : s[ 0x01 ] & 0x10;
		int y0, y1, dy, flipy = m_new_sprite_chip ? s[ 0x06 ] & 0x04 : s[ 0x01 ] & 0x08;

		if ( flipx )    {   x0 = nx - 1;    x1 = -1;    dx = -1;    }
		else            {   x0 = 0;         x1 = nx;    dx = +1;    }

		if ( flipy )    {   y0 = ny - 1;    y1 = -1;    dy = -1;    }
		else            {   y0 = 0;         y1 = ny;    dy = +1;    }

		// Draw the sprite directly to screen if no zoom/rotation/offset is required
		if (dstdx == 0x100 && !dstdy && !srcx && !srcy)
		{
			for (int y = y0; y != y1; y += dy)
			{
				for (int x = x0; x != x1; x += dx)
				{
					m_gfxdecode->gfx(gfx)->transpen(bitmap, cliprect,
											code++, color,
											flipx, flipy,
											dstx + x * 16, dsty + y * 16, 0);
				}
			}

			continue;
		}

		// First draw the sprite in a buffer without zoom/rotation/offset, nor transparency
		rectangle sprite_cliprect(0, nx * 16 - 1, 0, ny * 16 - 1);
		for (int y = y0; y != y1; y += dy)
		{
			for (int x = x0; x != x1; x += dx)
			{
				m_gfxdecode->gfx(gfx)->opaque(*m_sprite_bitmap, sprite_cliprect,
										code++, color,
										flipx, flipy,
										x * 16, y * 16);
			}
		}

		// Sign extend the transformation values
		dstdx   =   (dstdx & 0x7fff) - (dstdx & 0x8000);
		dstdy   =   (dstdy & 0x7fff) - (dstdy & 0x8000);
		srcx    =   (srcx  & 0x7fff) - (srcx  & 0x8000);
		srcy    =   (srcy  & 0x7fff) - (srcy  & 0x8000);
		dstdy   =   -dstdy;

		// Use fixed point values (16.16), for accuracy
		dstx  <<=   16;
		dsty  <<=   16;

		// Source delta (equal for x and y)
		int z = int( sqrt(dstdx * dstdx + dstdy * dstdy) + 0.5 );   // dest delta vector is scaled by the source delta!?
		if (!z)
			z = 0x100;
		int srcdzz = z << 8;

		// Destination x and y deltas
		int dstdxx = (dstdx << 16) / z; // dest x delta for source x increments
		int dstdyx = (dstdy << 16) / z; // dest y delta for source x increments

		int dstdxy = -dstdyx;           // dest x delta for source y increments (orthogonal to the above vector)
		int dstdyy = dstdxx;            // dest y delta for source y increments

		// Transform the source offset in a destination offset (negate, scale and rotate it)
		srcx = (-srcx << 8) / z;
		srcy = (-srcy << 8) / z;

		dstx += srcx * dstdxx;
		dsty += srcx * dstdyx;

		dstx += srcy * dstdxy;
		dsty += srcy * dstdyy;

		// Supersampling (2x2) to avoid gaps in the destination
		srcdzz /= 2;
		dstdxx /= 2;
		dstdyx /= 2;
		dstdxy /= 2;
		dstdyy /= 2;

		// Transform the source image while drawing to the screen
		uint16_t const *const src = &m_sprite_bitmap->pix(0);
		uint16_t *const dst = &bitmap.pix(0);

		int src_rowpixels = m_sprite_bitmap->rowpixels();
		int dst_rowpixels = bitmap.rowpixels();

		uint16_t penmask = gfx ? 0xff : 0x0f;

		// Scan source image top to bottom
		srcy = 0;
		for (;;)
		{
			int dstx_prev = dstx;
			int dsty_prev = dsty;

			int fy = integer_part(srcy);
			if (fy > sprite_cliprect.max_y)
				break;
			if (fy >= sprite_cliprect.min_y)
			{
				// left to right
				srcx = 0;
				for (;;)
				{
					int fx = integer_part(srcx);
					if (fx > sprite_cliprect.max_x)
						break;
					if (fx >= sprite_cliprect.min_x)
					{
						int px = integer_part(dstx);
						int py = integer_part(dsty);

						if (px >= cliprect.min_x && px <= cliprect.max_x && py >= cliprect.min_y && py <= cliprect.max_y)
						{
							uint16_t pen = src[fy * src_rowpixels + fx];
							if (pen & penmask)
								dst[py * dst_rowpixels + px] = pen;
						}
					}

					// increment source x and dest x,y
					srcx += srcdzz;

					dstx += dstdxx;
					dsty += dstdyx;
				}
			}

			// increment source y and dest x,y
			srcy += srcdzz;

			dstx = dstx_prev;
			dsty = dsty_prev;
			dstx += dstdxy;
			dsty += dstdyy;
		}
	}
}

uint32_t sigmab98_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
		if (machine().input().code_pressed(KEYCODE_R))  msk |= 8;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	bitmap.fill(m_palette->pens()[0x1000], cliprect);

	// Draw from priority 3 (bottom, converted to a bitmask) to priority 0 (top)
	draw_sprites(bitmap, cliprect, layers_ctrl & 8);
	draw_sprites(bitmap, cliprect, layers_ctrl & 4);
	draw_sprites(bitmap, cliprect, layers_ctrl & 2);
	draw_sprites(bitmap, cliprect, layers_ctrl & 1);

	return 0;
}

/***************************************************************************

    Video Regs

    Offset:     Bits:         Value:

    01                        Screen Width / 2 - 1
    03
    05
    07
    09                        Screen Height - 1
    0b
    0d
    0f
    11
    13           76-- ----
                 --5- ----    VBlank?
                 ---4 3---
                 ---- -2--    Sprites Buffered?
                 ---- --10
    15
    17
    19
    1b                        Background Color (Low)
    1d                        Background Color (High)
    1f
    21

***************************************************************************/

void sigmab98_base_state::vregs_w(offs_t offset, uint8_t data)
{
	m_vregs[offset] = data;

	switch (offset)
	{
		case 0x1b:  // background color
		case 0x1d:
		{
			int x = (m_vregs[0x1d] << 8) + m_vregs[0x1b];
			int r = (x >> 10) & 0x1f;
			int g = (x >>  5) & 0x1f;
			int b = (x >>  0) & 0x1f;
			m_palette->set_pen_color(0x1000, pal5bit(r), pal5bit(g), pal5bit(b));
			break;
		}
//      default:
//          logerror("%s: unknown video reg written: %02x = %02x\n", machine().describe_context(), offset, data);
	}
}

uint8_t sigmab98_base_state::vregs_r(offs_t offset)
{
	switch (offset)
	{
		default:
			if (!machine().side_effects_disabled())
				logerror("%s: unknown video reg read: %02x\n", machine().describe_context(), offset);
			return m_vregs[offset];
	}
}

uint8_t sigmab98_base_state::d013_r()
{
	// bit 5 must go 0->1 (vblank?)
	// bit 2 must be set (sprite buffered? triggered by pulsing bit 3 of port C6?)
//  return (m_screen->vblank() ? 0x20 : 0x00) | 0x04;
	return (m_screen->vblank() ? 0x20 : 0x01) | 0x04;
//  return machine().rand();
}
uint8_t sigmab98_base_state::d021_r()
{
	// bit 5 must be 0?
	return 0;
//  return machine().rand();
}


/***************************************************************************

    Memory Maps

***************************************************************************/

/***************************************************************************
                          Minna Atsumare! Dodge Hero
***************************************************************************/

void sigmab98_state::dodghero_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().nopw();

	map(0x40000, 0x41fff).ram().share("nvram"); // battery backed RAM

	map(0x80000, 0x80fff).ram().share("spriteram");
	map(0x81000, 0x811ff).nopw();
	map(0x82000, 0x821ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0x82800, 0x8287f).ram().share("vtable");

	map(0x83000, 0x83021).rw(FUNC(sigmab98_state::vregs_r), FUNC(sigmab98_state::vregs_w)).share("vregs");
	map(0x83013, 0x83013).r(FUNC(sigmab98_state::d013_r));
	map(0x83021, 0x83021).r(FUNC(sigmab98_state::d021_r));
}

/***************************************************************************
                        GeGeGe no Kitarou Youkai Slot
***************************************************************************/

// Outputs

void sigmab98_state::show_outputs()
{
#ifdef MAME_DEBUG
	popmessage("0: %02X  4: %02X  6: %02X  8: %02X", m_c0, m_c4, m_c6, m_c8);
#endif
}

// Port c0
void sigmab98_state::eeprom_w(uint8_t data)
{
	// latch the bit
	m_eeprom->di_write((data & 0x40) >> 6);

	// reset line asserted: reset.
//  if ((m_c0 ^ data) & 0x20)
		m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	m_c0 = data;
	//show_outputs(state);
}

// Port c4
// 10 led?
void sigmab98_state::c4_w(uint8_t data)
{
	m_leds[0] = BIT(data, 4);

	m_c4 = data;
	show_outputs();
}

// Port c6
// 03 lockout (active low, 02 is cleared when reaching 99 credits)
// 04 pulsed on coin in
// 08 buffer sprites?
// 10 led?
// 20 led? (starts blinking after coin in)
void sigmab98_state::c6_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, (~data) & 0x02);

	machine().bookkeeping().coin_counter_w(0,   data  & 0x04);

	if ((data & 0x08) && !(m_c6 & 0x08))
		m_buffered_spriteram->copy();

	m_leds[1] = BIT(data, 4);
	m_leds[2] = BIT(data, 5);

	m_c6 = data;
	show_outputs();
}

// Port c8
// 01 hopper enable?
// 02 hopper motor on (active low)?
void sigmab98_state::c8_w(uint8_t data)
{
	m_hopper->motor_w((!(data & 0x02) && (data & 0x01)) ? 0 : 1);

	m_c8 = data;
	show_outputs();
}

void sigmab98_state::gegege_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();

	map(0x40000, 0x47fff).ram().share("nvram"); // battery backed RAM

	map(0x80000, 0x80fff).ram().share("spriteram");

	map(0x82000, 0x821ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0x82800, 0x8287f).ram().share("vtable");

	map(0x83000, 0x83021).rw(FUNC(sigmab98_state::vregs_r), FUNC(sigmab98_state::vregs_w)).share("vregs");
	map(0x83013, 0x83013).r(FUNC(sigmab98_state::d013_r));
	map(0x83021, 0x83021).r(FUNC(sigmab98_state::d021_r));
}

void sigmab98_state::gegege_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0xc0, 0xc0).portr("EEPROM").w(FUNC(sigmab98_state::eeprom_w));
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(sigmab98_state::c4_w));
	map(0xc6, 0xc6).w(FUNC(sigmab98_state::c6_w));
	map(0xc8, 0xc8).w(FUNC(sigmab98_state::c8_w));

	map(0xe5, 0xe5).nopr();   // during irq
}


/***************************************************************************
                       Otakara Itadaki Luffy Kaizoku-Dan!
***************************************************************************/

void lufykzku_state::dsw_w(int state)
{
	m_dsw_bit = state;
}

// Port c0
void lufykzku_state::lufykzku_watchdog_w(uint8_t data)
{
	m_watchdog->write_line_ck(BIT(data, 7));
}

// Port c4
void lufykzku_state::lufykzku_c4_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(1, (~data) & 0x20); // 100 yen lockout
//  machine().bookkeeping().coin_lockout_w(2, (~data) & 0x40); // (unused coin lockout)
	machine().bookkeeping().coin_lockout_w(0, (~data) & 0x80); // medal lockout

	m_c4 = data;
	show_outputs();
}

// Port c6
void lufykzku_state::lufykzku_c6_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(1, data & 0x01); // 100 yen in
//  machine().bookkeeping().coin_counter_w(2, data & 0x02); // (unused coin in)
	machine().bookkeeping().coin_counter_w(0, data & 0x04); // medal in
	machine().bookkeeping().coin_counter_w(3, data & 0x08); // medal out
	m_leds[0] = BIT(data, 4); // button led
//  m_leds[1] = BIT(data, 5); // (unused button led)
//  m_leds[2] = BIT(data, 6); // (unused button led)
//  m_leds[3] = BIT(data, 7); // (unused button led)

	m_c6 = data;
	show_outputs();
}

// Port c8
uint8_t lufykzku_state::lufykzku_c8_r()
{
	return 0xbf | (m_dsw_bit ? 0x40 : 0);
}

void lufykzku_state::lufykzku_c8_w(uint8_t data)
{
	// bit 0? on payout button
	// bit 1? when ending payment
	m_hopper->motor_w(( (data & 0x01) && !(data & 0x02)) ? 0 : 1);

	m_dsw_shifter[0]->shift_load_w(BIT(data, 4));
	m_dsw_shifter[1]->shift_load_w(BIT(data, 4));
	m_dsw_shifter[0]->clock_w(BIT(data, 5));
	m_dsw_shifter[1]->clock_w(BIT(data, 5));

	m_c8 = data;
	show_outputs();
}

void lufykzku_state::lufykzku_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x80000, 0x83fff).ram().share("nvram");

	map(0xc0000, 0xc0fff).ram().share("spriteram");
	map(0xc1000, 0xc2fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // more palette entries
	map(0xc3000, 0xc3021).rw(FUNC(lufykzku_state::vregs_r), FUNC(lufykzku_state::vregs_w)).share("vregs");
	map(0xc3013, 0xc3013).r(FUNC(lufykzku_state::d013_r));
	map(0xc3021, 0xc3021).r(FUNC(lufykzku_state::d021_r));

	map(0xc3400, 0xc347f).ram().share("vtable");
}

void lufykzku_state::lufykzku_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("oki", FUNC(okim9810_device::write));
	map(0x01, 0x01).w("oki", FUNC(okim9810_device::tmp_register_w));

	map(0xc0, 0xc0).portr("COIN").w(FUNC(lufykzku_state::lufykzku_watchdog_w)); // bit 7 -> watchdog
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(lufykzku_state::lufykzku_c4_w)); // bit 7 = medal lock, bit 6 = coin3, bit 5 = yen
	map(0xc6, 0xc6).w(FUNC(lufykzku_state::lufykzku_c6_w));
	map(0xc8, 0xc8).rw(FUNC(lufykzku_state::lufykzku_c8_r), FUNC(lufykzku_state::lufykzku_c8_w)); // 0xc8 bit 6 read (eeprom?)
}


/***************************************************************************
                                 Animal Catch
***************************************************************************/

uint8_t sammymdl_state::eeprom_r()
{
	return m_eeprom->do_read() ? 0x80 : 0;
}

void sammymdl_state::eeprom_w(uint8_t data)
{
	// latch the bit
	m_eeprom->di_write((data & 0x40) >> 6);

	// reset line asserted: reset.
	m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	if (data & 0x8f)
		logerror("%s: unknown eeeprom bits written %02x\n", machine().describe_context(), data);
}

uint8_t sigmab98_base_state::vblank_r()
{
	// mask 0x04 must be set before writing sprite list
	// mask 0x10 must be set or irq/00 hangs?
	return  (m_vblank & ~0x01) | 0x14;
}

void sigmab98_base_state::vblank_w(uint8_t data)
{
	m_vblank = (m_vblank & ~0x03) | (data & 0x03);
}

void sigmab98_base_state::screen_vblank_sammymdl(int state)
{
	// rising edge
	if (state)
	{
		m_vblank &= ~0x01;
	}
}

void sammymdl_state::show_3_outputs()
{
#ifdef MAME_DEBUG
	popmessage("COIN: %02X  LED: %02X  HOP: %02X", m_out[0], m_out[1], m_out[2]);
#endif
}
// Port 31
void sammymdl_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,   data  & 0x01 );  // coin1 in
	machine().bookkeeping().coin_counter_w(1,   data  & 0x02 );  // coin2 in
	machine().bookkeeping().coin_counter_w(2,   data  & 0x04 );  // medal in

//  machine().bookkeeping().coin_lockout_w(1, (~data) & 0x08 ); // coin2 lockout?
//  machine().bookkeeping().coin_lockout_w(0, (~data) & 0x10 ); // coin1 lockout
//  machine().bookkeeping().coin_lockout_w(2, (~data) & 0x20 ); // medal lockout?

//  data & 0x80? (gocowboy)

	m_out[0] = data;
	show_3_outputs();
}

// Port 32
void sammymdl_state::leds_w(uint8_t data)
{
	m_leds[0] = BIT(data, 0);   // button

	m_out[1] = data;
	show_3_outputs();
}

// Port b0
// 02 hopper enable?
// 01 hopper motor on (active low)?
void sammymdl_state::hopper_w(uint8_t data)
{
	m_hopper->motor_w((!(data & 0x01) && (data & 0x02)) ? 0 : 1);

	m_out[2] = data;
	show_3_outputs();
}

uint8_t sammymdl_state::coin_hopper_r()
{
	uint8_t ret = ioport("COIN")->read();

//  if ( !m_hopper->read(0) )
//      ret &= ~0x01;

	return ret;
}

void sammymdl_state::animalc_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("mainbios", 0);
	map(0x10000, 0x2ffff).rom().region("maincpu", 0x400);
	map(0x60000, 0x63fff).ram().share("nvram");
	map(0x6a000, 0x6ffff).ram();
	map(0x70000, 0x70fff).ram().share("spriteram");
	map(0x72000, 0x721ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x72800, 0x7287f).ram().share("vtable");
	map(0x73000, 0x73021).rw(FUNC(sammymdl_state::vregs_r), FUNC(sammymdl_state::vregs_w)).share("vregs");
	map(0x73011, 0x73011).nopw();  // IRQ Enable? Screen disable?
	map(0x73013, 0x73013).rw(FUNC(sammymdl_state::vblank_r), FUNC(sammymdl_state::vblank_w));    // IRQ Ack?
}

void sammymdl_state::animalc_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x90, 0x90).w("oki", FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).w(FUNC(sammymdl_state::hopper_w));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                               Go Go Cowboy
***************************************************************************/

void sammymdl_state::gocowboy_map(address_map &map)
{
	map(0x00000, 0x5ffff).rom().region("mainbios", 0);
	map(0x60000, 0x67fff).ram().share("nvram");
	map(0x70000, 0x70fff).ram().share("spriteram");
	map(0x72000, 0x721ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x72800, 0x7287f).ram().share("vtable");
	map(0x73000, 0x73021).rw(FUNC(sammymdl_state::vregs_r), FUNC(sammymdl_state::vregs_w)).share("vregs");
}


void sammymdl_state::gocowboy_leds_w(uint8_t data)
{
	m_leds[0] = BIT(data, 0);   // button
	m_leds[1] = BIT(data, 1);   // coin lockout? (after coining up, but not for service coin)
	m_leds[2] = BIT(data, 2);   // ? long after a prize is not collected
	m_leds[3] = BIT(data, 3);   // ? "don't forget the large prizes"

	// 10 hopper enable?
	// 20 hopper motor on (active low)?
	m_hopper_small->motor_w((!(data & 0x20) && (data & 0x10)) ? 0 : 1);
	m_hopper_large->motor_w((!(data & 0x80) && (data & 0x40)) ? 0 : 1);

	m_out[1] = data;
	show_3_outputs();
}

void sammymdl_state::gocowboy_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x90, 0x90).rw("oki", FUNC(okim9810_device::read), FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).nopw();
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                             Hae Hae Ka Ka Ka
***************************************************************************/

uint8_t sigmab98_base_state::haekaka_vblank_r()
{
	return m_screen->vblank() ? 0 : 0x1c;
}

void sammymdl_state::haekaka_leds_w(uint8_t data)
{
	// All used
	m_leds[0] = BIT(data, 0);
	m_leds[1] = BIT(data, 1);
	m_leds[2] = BIT(data, 2);
	m_leds[3] = BIT(data, 3);
	m_leds[4] = BIT(data, 4);
	m_leds[5] = BIT(data, 5);
	m_leds[6] = BIT(data, 6);
	m_leds[7] = BIT(data, 7);

	m_out[1] = data;
	show_3_outputs();
}

void sammymdl_state::haekaka_coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,   data & 0x01 );   // medal out
//                                 data & 0x02 ?
//                                 data & 0x04 ?
//                                 data & 0x10 ?

	m_out[0] = data;
	show_3_outputs();
}

void sammymdl_state::haekaka_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("mainbios", 0);
	map(0x10000, 0x2ffff).rom().region("maincpu", 0x400);
	map(0x60000, 0x61fff).ram().share("nvram");
	map(0x70000, 0x70fff).ram().share("spriteram");
	map(0x72000, 0x721ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x72800, 0x7287f).ram().share("vtable");
	map(0x73000, 0x73021).rw(FUNC(sammymdl_state::vregs_r), FUNC(sammymdl_state::vregs_w)).share("vregs");
	map(0x73013, 0x73013).r(FUNC(sammymdl_state::haekaka_vblank_r));
}

/***************************************************************************
                              Itazura Monkey
***************************************************************************/

void sammymdl_state::itazuram_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("mainbios", 0);
	map(0x10000, 0x2ffff).rom().region("maincpu", 0x400);
	map(0x60000, 0x63fff).ram().share("nvram");
	map(0x70000, 0x70fff).ram().share("spriteram");
	map(0x72000, 0x721ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x72800, 0x7287f).ram().share("vtable");
	map(0x73000, 0x73021).rw(FUNC(sammymdl_state::vregs_r), FUNC(sammymdl_state::vregs_w)).share("vregs");
	map(0x73011, 0x73011).nopw();  // IRQ Enable? Screen disable?
	map(0x73013, 0x73013).r(FUNC(sammymdl_state::haekaka_vblank_r)).nopw();  // IRQ Ack?
}

/***************************************************************************
                             Taihou de Doboon
***************************************************************************/

void sammymdl_state::tdoboon_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("mainbios", 0);
	map(0x10000, 0x2ffff).rom().region("maincpu", 0x400);
	map(0x60000, 0x61fff).ram().share("nvram");
	map(0x70000, 0x70fff).ram().share("spriteram");
	map(0x72000, 0x721ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x72800, 0x7287f).ram().share("vtable");
	map(0x73000, 0x73021).rw(FUNC(sammymdl_state::vregs_r), FUNC(sammymdl_state::vregs_w)).share("vregs");
	map(0x73013, 0x73013).r(FUNC(sammymdl_state::haekaka_vblank_r));
}


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout sigmab98_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( gfx_sigmab98 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb,  0, 0x100/16  )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x8_layout, 0, 0x100/256 )
GFXDECODE_END

// Larger palette
static GFXDECODE_START( gfx_lufykzku )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb,  0, 0x1000/16 )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x8_layout, 0, 0x1000/16 )
GFXDECODE_END


/***************************************************************************

    Input Ports

***************************************************************************/

/***************************************************************************
                             Sigma B-98 Games
***************************************************************************/

// 1 button (plus bet and payout)
static INPUT_PORTS_START( sigma_1b )
	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )                       // Related to d013. Must be 0
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM  ) PORT_VBLANK("screen") // Related to d013. Must be 0
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(5)   // ? (coin error, pulses mask 4 of port c6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(5) PORT_NAME("Medal")    // coin/medal in (coin error)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_1)  // bet / select in test menu
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PAYOUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )  // pay out / change option in test menu
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

// 3 buttons (plus bet and payout)
static INPUT_PORTS_START( sigma_3b )
	PORT_INCLUDE( sigma_1b )

	PORT_MODIFY("BUTTON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 )
INPUT_PORTS_END

// 5 buttons (plus bet and payout)
static INPUT_PORTS_START( sigma_5b )
	PORT_INCLUDE( sigma_1b )

	PORT_MODIFY("BUTTON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3 )

	PORT_MODIFY("PAYOUT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON5 )
INPUT_PORTS_END

// Joystick (plus bet and payout)
static INPUT_PORTS_START( sigma_js )
	PORT_INCLUDE( sigma_1b )

	PORT_MODIFY("BUTTON")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("PAYOUT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )
INPUT_PORTS_END


/***************************************************************************
                           Banpresto Medal Games
***************************************************************************/

static INPUT_PORTS_START( lufykzku )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1   ) PORT_IMPULSE(2) // medal in
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2   ) PORT_IMPULSE(2) // 100 yen in
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )                 // (unused coin in)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // sw1 (button)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("PAYOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE       ) // test sw
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1      ) // service sw (service coin, press to go down in service mode)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // payout sw
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_DOOR   ) // "maintenance panel abnormality" when not high on boot (in that case, when it goes high the game resets)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN       )

	// Dips read serially via port 0xc8 bit 6
	PORT_START("DSW1") // stored at fc14
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW1:1" )
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW1:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DSW1:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "DSW1:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "DSW1:6" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:7,8") // Advertize Voice
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xc0, "On (1/3 Of The Loops)" )
	PORT_DIPSETTING(    0x80, "On (1/2 Of The Loops)" )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_START("DSW2") // stored at fc15
	PORT_DIPNAME( 0x07, 0x03, "Payout" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x08, "Win Wave" ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x00, "Big"   )
	PORT_DIPNAME( 0xf0, 0xa0, "¥100 Medals" ) PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0xf0,  "5" )
	PORT_DIPSETTING(    0xe0,  "6" )
	PORT_DIPSETTING(    0xd0,  "7" )
	PORT_DIPSETTING(    0xc0,  "8" )
	PORT_DIPSETTING(    0xb0,  "9" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0x90, "11" )
	PORT_DIPSETTING(    0x80, "12" )
	PORT_DIPSETTING(    0x70, "13" )
	PORT_DIPSETTING(    0x60, "14" )
	PORT_DIPSETTING(    0x50, "15" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x30, "17" )
	PORT_DIPSETTING(    0x20, "18" )
	PORT_DIPSETTING(    0x10, "19" )
	PORT_DIPSETTING(    0x00, "20" )
INPUT_PORTS_END


/***************************************************************************
                             Sammy Medal Games
***************************************************************************/

static INPUT_PORTS_START( sammymdl )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // freeze (itazuram)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(5)   // coin1 in
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(5)   // coin2 in
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN3   ) PORT_IMPULSE(5) PORT_NAME("Medal")    // medal in
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE )   // test sw
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( haekaka )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5) PORT_NAME( "Medal" )   // medal in ("chacker")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE  )  // test sw
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1  )  // button
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE1 )  // service coin / set in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
INPUT_PORTS_END

static INPUT_PORTS_START( gocowboy )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) // shoot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(20) // coin
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper_small", ticket_dispenser_device, line_r) // 1/2' pay sensor (small)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper_large", ticket_dispenser_device, line_r) // 3/4' pay sensor (large)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Switch") // capsule test (pressed while booting) / next in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Switch") // reset backup ram (pressed while booting) / previous in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE  )                           // test mode (keep pressed in game) / select in test mode / service coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

/***************************************************************************

    Machine Drivers

***************************************************************************/

/***************************************************************************
                             Sigma B-98 Games
***************************************************************************/

INTERRUPT_GEN_MEMBER(sigmab98_state::sigmab98_vblank_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x5a); // Z80
}

void sigmab98_state::sigmab98(machine_config &config)
{
	KY80(config, m_maincpu, 20000000);  // !! TAXAN KY-80, clock @X1? !!
	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab98_state::gegege_mem_map);
	m_maincpu->set_addrmap(AS_IO, &sigmab98_state::gegege_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sigmab98_state::sigmab98_vblank_interrupt));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	EEPROM_93C46_16BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);                    // ?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_size(0x140, 0x100);
	m_screen->set_visarea(0,0x140-1, 0,0xf0-1);
	m_screen->set_screen_update(FUNC(sigmab98_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sigmab98);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	BUFFERED_SPRITERAM8(config, m_buffered_spriteram);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));    // clock @X2?
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}

void sigmab98_state::dodghero(machine_config &config)
{
	sigmab98(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab98_state::dodghero_mem_map);
}


/***************************************************************************
                           Banpresto Medal Games
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_state::lufykzku_irq)
{
	int scanline = param;

	if (scanline == 240)
	{
		if (m_vblank_vector) m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vblank_vector); // Z80
	}
	else if (scanline == 128)
	{
		if (m_timer0_vector) m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer0_vector); // Z80
	}
	else if ((scanline % 8) == 0)
	{
		if (m_timer1_vector) m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer1_vector); // Z80 - this needs to be called often or the state of the door is not read at boot (at least 5 times before bb9 is called)
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_state::rockman_timer_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xfc);
}

void lufykzku_state::lufykzku(machine_config &config)
{
	KY80(config, m_maincpu, XTAL(20'000'000));  // !! TAXAN KY-80, clock @X1? !!
	m_maincpu->set_addrmap(AS_PROGRAM, &lufykzku_state::lufykzku_mem_map);
	m_maincpu->set_addrmap(AS_IO, &lufykzku_state::lufykzku_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(lufykzku_state::lufykzku_irq), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);   // battery backed RAM (TC55257DFL-70L)
	// No EEPROM

	MB3773(config, m_watchdog, 0);
	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	// 2 x 8-bit parallel/serial converters
	TTL165(config, m_dsw_shifter[0]);
	m_dsw_shifter[0]->data_callback().set_ioport("DSW2");
	m_dsw_shifter[0]->qh_callback().set(m_dsw_shifter[1], FUNC(ttl165_device::serial_w));

	TTL165(config, m_dsw_shifter[1]);
	m_dsw_shifter[1]->data_callback().set_ioport("DSW1");
	m_dsw_shifter[1]->qh_callback().set(FUNC(lufykzku_state::dsw_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);                    // ?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_size(0x140, 0x100);
	m_screen->set_visarea(0,0x140-1, 0,0xf0-1);
	m_screen->set_screen_update(FUNC(lufykzku_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lufykzku);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	//BUFFERED_SPRITERAM8(config, m_buffered_spriteram); // same as sammymdl?

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "lspeaker", 0.80);
	oki.add_route(1, "rspeaker", 0.80);
}

void lufykzku_state::rockman(machine_config& config)
{
	lufykzku(config);
	TIMER(config, "unktimer").configure_periodic(FUNC(lufykzku_state::rockman_timer_irq), attotime::from_hz(100));
}

/***************************************************************************
                             Sammy Medal Games
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(sammymdl_state::gocowboy_int)
{
	int scanline = param;

	// TODO: what really triggers these?
	if (scanline == 240)
	{
		m_kp69->ir_w<0>(1);
		m_kp69->ir_w<0>(0);
	}

	if (scanline == 128)
	{
		m_kp69->ir_w<1>(1);
		m_kp69->ir_w<1>(0);
	}
}

void sammymdl_state::sammymdl(machine_config &config)
{
	KL5C80A12(config, m_maincpu, XTAL(20'000'000));    // !! KL5C80A12CFP @ 10MHz? (actually 4 times faster than Z80) !!
	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::animalc_map);
	m_maincpu->set_addrmap(AS_IO, &sammymdl_state::animalc_io);
	m_maincpu->in_p0_callback().set(FUNC(sammymdl_state::eeprom_r));
	m_maincpu->out_p0_callback().set(FUNC(sammymdl_state::eeprom_w));
	m_maincpu->in_p1_callback().set(FUNC(sammymdl_state::coin_hopper_r));
	m_maincpu->in_p2_callback().set_ioport("BUTTON");
	m_maincpu->out_p3_callback().set(FUNC(sammymdl_state::coin_counter_w));
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::leds_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);   // battery backed RAM
	EEPROM_93C46_8BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);                    // ?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_size(0x140, 0x100);
	m_screen->set_visarea(0, 0x140-1, 0, 0xf0-1);
	m_screen->set_screen_update(FUNC(sammymdl_state::screen_update));
	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<0>));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sigmab98);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	//BUFFERED_SPRITERAM8(config, m_buffered_spriteram); // not on sammymdl?

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "lspeaker", 0.80);
	oki.add_route(1, "rspeaker", 0.80);
}

void sammymdl_state::animalc(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::animalc_map);
}

void sammymdl_state::gocowboy(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::gocowboy_map);
	m_maincpu->set_addrmap(AS_IO, &sammymdl_state::gocowboy_io);
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::gocowboy_leds_w));

	TIMER(config, "scantimer").configure_scanline(FUNC(sammymdl_state::gocowboy_int), "screen", 0, 1);

	config.device_remove("hopper");
	TICKET_DISPENSER(config, m_hopper_small, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );
	TICKET_DISPENSER(config, m_hopper_large, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	m_screen->screen_vblank().set_nop();
}

void sammymdl_state::haekaka(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::haekaka_map);
	m_maincpu->out_p3_callback().set(FUNC(sammymdl_state::haekaka_coin_counter_w));
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::haekaka_leds_w));

	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<2>));
}

void sammymdl_state::itazuram(machine_config &config)
{
	sammymdl(config);

	TIMER(config, "scantimer").configure_scanline(FUNC(sammymdl_state::gocowboy_int), "screen", 0, 1);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::itazuram_map);

	m_screen->screen_vblank().set_nop();
}

void sammymdl_state::pyenaget(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::haekaka_map);
	m_maincpu->out_p4_callback().set(FUNC(sammymdl_state::haekaka_leds_w));

	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<2>));
}

void sammymdl_state::tdoboon(machine_config &config)
{
	sammymdl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sammymdl_state::tdoboon_map);

	m_screen->screen_vblank().set(m_kp69, FUNC(kp69_device::ir_w<2>));
	m_screen->set_visarea(0,0x140-1, 0+4,0xf0+4-1);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

  Minna Atsumare! Dodge Hero

***************************************************************************/

ROM_START( dodghero )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9802-1d.ic7", 0x00000, 0x20000, CRC(093492e3) SHA1(d4dd104dc2410d97add532eea031cbc1ede3b0b1) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9802-2.ic12",  0x00000, 0x80000, CRC(bb810ab8) SHA1(02bb1bb9b6dd0d24401c8a8c579f5ebcba963d8f) )
	ROM_LOAD( "b9802-3a.ic13", 0x80000, 0x80000, CRC(8792e487) SHA1(c5ed8059cd40a00656016b33762a04b9bedd7f06) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "b9802-5.ic16", 0x00000, 0x80000, CRC(4840bdbd) SHA1(82f286ef848df9b6dcb5ff3b3aaa11d8e93e995b) )
	ROM_LOAD( "b9802-6.ic26", 0x80000, 0x80000, CRC(d83d8537) SHA1(9a5afdc68417db828a09188d653552452930b136) )
ROM_END

/***************************************************************************

  Itazura Daisuki! Sushimaru Kun

***************************************************************************/

ROM_START( sushimar )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9803-1c.ic7", 0x00000, 0x20000, CRC(8ad3b7be) SHA1(14d8cec6723f230d4167de91b5b1103fe40755bc) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9803-2.ic12",  0x00000, 0x80000, CRC(cae710a4) SHA1(c0511412d8feaa032b8bcd72074522d1b90f22b2) )
	ROM_LOAD( "b9803-03.ic13", 0x80000, 0x80000, CRC(f69f37f6) SHA1(546045b50dbc3ef45fc4dd1c7f2f6a23dfdc53d8) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9803-5a.ic16", 0x00000, 0x80000, CRC(da3f36aa) SHA1(0caffbe6726afd41763f25378f8820724aa7bbce) )
ROM_END

/***************************************************************************

  GeGeGe no Kitarou Youkai Slot

  (C) 1997 Banpresto, Sigma

  PCB:

    (c) 1997 Sigma B-98-1 MAIN PCB
    970703 (Sticker)

  CPU:

    TAXAN KY-80 YAMAHA 9650 AZGC (@IC1)
    XTAL ?? (@X1)

  Video:

    TAXAN KY-3211 9722 AZGC (@IC11)
    XTAL 27.000 MHz (@XOSC1)
    M548262-60 (@IC24) - 262144-Word x 8-Bit Multiport DRAM

  Sound:

    YAMAHA YMZ280B-F (@IC14)
    XTAL ?? (@X2)
    Trimmer

  Other:

    93C46AN EEPROM (@IC5)
    MAX232CPE (@IC6)
    Battery (@BAT)

***************************************************************************/

ROM_START( gegege )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9804-1.ic7", 0x00000, 0x20000, CRC(f8b4f855) SHA1(598bd9f91123e9ab539ce3f33779bff2d072e731) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9804-2.ic12", 0x00000, 0x80000, CRC(4211079d) SHA1(d601c623fb909f1346fd02b8fb37b67956e2cd4e) )
	ROM_LOAD( "b9804-3.ic13", 0x80000, 0x80000, CRC(54aeb2aa) SHA1(ccf939111f6288a889846d51bab47ff4e992c542) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9804-5.ic16", 0x00000, 0x80000, CRC(ddd7984c) SHA1(3558c495776671ffd3cd5c665b87827b3959b360) )
ROM_END

void sigmab98_state::init_gegege()
{
	uint8_t *rom = memregion("maincpu")->base();

	// Related to d013
//  rom[0x0bdd] = 0xc9;

//  rom[0x0bf9] = 0xc9;

//  rom[0x0dec] = 0x00;
//  rom[0x0ded] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Burning Sanrinsya - Burning Tricycle

***************************************************************************/

ROM_START( b3rinsya )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9805-1c.ic7", 0x00000, 0x20000, CRC(a8cde2f4) SHA1(74d1f3f1710084d788a71dec0366f2c3f756fdf8) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9805-2.ic12", 0x00000, 0x80000, CRC(7ec2e957) SHA1(1eb9095663d4f8f8f0c77f151918af1978332b3d) )
	ROM_LOAD( "b9805-3.ic13", 0x80000, 0x80000, CRC(449d0848) SHA1(63e91e4be8b58a6ebf1777ed5a9c23416bacba48) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9805-5.ic16", 0x00000, 0x80000, CRC(f686f886) SHA1(ab68d12c5cb3a9fbc8a178739f39a2ff3104a0a1) )
ROM_END

void sigmab98_state::init_b3rinsya()
{
	uint8_t *rom = memregion("maincpu")->base();

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  PEPSI Man

***************************************************************************/

ROM_START( pepsiman )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9806-1a.ic7", 0x00000, 0x20000, CRC(3152fe90) SHA1(98a8ae1bd3a4381cec11ba8b3e9cdad71c7bd05a) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9806-2.ic12", 0x00000, 0x80000, CRC(82f650ea) SHA1(c0b214fdc39329e2136707bc195d470d4b613509) )
	ROM_LOAD( "b9806-3.ic13", 0x80000, 0x80000, CRC(07dc548e) SHA1(9419c0cac289a9894cce1a10924f40e146e2ff8a) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9806-5.ic16", 0x00000, 0x80000, CRC(6d405dfb) SHA1(e65ffe1279680097894754e379d7ad638657eb49) )
ROM_END

void sigmab98_state::init_pepsiman()
{
	uint8_t *rom = memregion("maincpu")->base();

	// Related to d013
//  rom[0x058a] = 0xc9;

//  rom[0x05a6] = 0xc9;

//  rom[0xa00e] = 0x00;
//  rom[0xa00f] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Transformers Beast Wars II

***************************************************************************/

ROM_START( tbeastw2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9808-1b.ic7", 0x00000, 0x20000, CRC(65f7e079) SHA1(d421da3c99d62d3228e1b9c1cfb2de51f0fcc56e) )

	ROM_REGION( 0x180000, "sprites", 0 )
	ROM_LOAD( "b9808-2.ic12", 0x000000, 0x80000, CRC(dda5c2d2) SHA1(1bb21e7251df93b0f502b716e958d81f4e4e46dd) )
	ROM_LOAD( "b9808-3.ic13", 0x080000, 0x80000, CRC(80df49c6) SHA1(14342be3a176cdf015c0ac07a4f1c109862c67aa) )
	ROM_LOAD( "b9808-4.ic17", 0x100000, 0x80000, CRC(d90961ea) SHA1(c2f226a528238eafc1ba37200da4ee6ce9b54325) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "b9808-5.ic16", 0x00000, 0x80000, CRC(762c6d5f) SHA1(0d4e35b7f346c8cc0c49163474f34c1fc462998a) )
	ROM_LOAD( "b9808-6.ic26", 0x80000, 0x80000, CRC(9ed759c9) SHA1(963db80b8a107ce9292bbc776ba91bc76ad82d5b) )
ROM_END

void sigmab98_state::init_tbeastw2()
{
	uint8_t *rom = memregion("maincpu")->base();

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Uchuu Tokkyuu Medalian

***************************************************************************/

ROM_START( ucytokyu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9809-1.ic7", 0x00000, 0x20000, CRC(5be6adff) SHA1(7248157111be2ae23df7d51f5d071cc3b9fd79b4) )

	ROM_REGION( 0x180000, "sprites", 0 )
	ROM_LOAD( "b9809-2.ic12", 0x000000, 0x80000, CRC(18f342b3) SHA1(09d62bb3597259e0fbae2c0f4ed163685a4a9dd9) )
	ROM_LOAD( "b9809-3.ic13", 0x080000, 0x80000, CRC(88a2a52a) SHA1(0dd10d4fa88d1a54150729026495a70dbe67bae0) )
	ROM_LOAD( "b9809-4.ic17", 0x100000, 0x80000, CRC(ea74eacd) SHA1(279fa1d2bc7bfafbafecd0e0758a47345ca95140) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "b9809-5.ic16", 0x00000, 0x80000, CRC(470006e6) SHA1(34c82fae7364eb5288de5c8128d72d7e5772c526) )
	ROM_LOAD( "b9809-6.ic26", 0x80000, 0x80000, CRC(4e2d5fdf) SHA1(af1357b0f6a407890ecad26a18d2b4e223802693) )
ROM_END

void sigmab98_state::init_ucytokyu()
{
	uint8_t *rom = memregion("maincpu")->base();

	// Related to d013
//  rom[0x0bfa] = 0xc9;

//  rom[0x0c16] = 0xc9;

//  rom[0xa43a] = 0x00;
//  rom[0xa43b] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;
}

/***************************************************************************

  Minna Ganbare! Dash Hero

***************************************************************************/

ROM_START( dashhero )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b098111-0101.ic7", 0x00000, 0x20000, CRC(46488393) SHA1(898bafbf5273b368cf963d863fb93e9fa0da816f) )

	ROM_REGION( 0x180000, "sprites", 0 )
	ROM_LOAD( "b98114-0100.ic12", 0x000000, 0x80000, CRC(067625ea) SHA1(f9dccfc85adbb840da7512db0c88f554b453d2d2) )
	ROM_LOAD( "b98115-0100.ic13", 0x080000, 0x80000, CRC(d6f0b89d) SHA1(33b5f2f6529fd9a145ccb1b4deffabf5fa0d46cb) )
	ROM_LOAD( "b98116-0100.ic17", 0x100000, 0x80000, CRC(c0dbe953) SHA1(a75e202a0c1be988b8fd7d4ee23ebc82f6110e5f) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b098112-0100.ic16", 0x00000, 0x80000, CRC(26e5d6f5) SHA1(6fe6a26e51097886db58a6619b12a73cd21e7130) )
ROM_END

void sigmab98_state::init_dashhero()
{
	uint8_t *rom = memregion("maincpu")->base();

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8162] = 0x00;
	rom[0x8163] = 0x00;
}


/***************************************************************************

  お宝いただき ルフィ海賊団! (Otakara Itadaki Luffy Kaizoku-Dan!)
  "From TV animation One Piece"
  (C) Eiichiro Oda / Shueisha - Fuji TV - Toho Animation
  (C) Banpresto 2001 Made In Japan

  PCB: BPSC-2001M-A

  Chips:
    TAXAN KY-80 YAMAHA  :  CPU
    TAXAN KY-10510      :  Graphics, slightly different sprite format than KY-3211
    OKI M9811           :  Sound, 4-channel version of OKI M9810
    MB3773 (@IC22?)     :  Power Supply Monitor with Watch Dog Timer and Reset (SOIC8)

  Misc:
    2 x DSW8 (@DSW1-2, KSD08S)
    2 x 74HC165A (8-bit parallel-in/serial out shift register)
    Cell battery (@BT1)
    Potentiometer (@VR1)
    Connectors: CN1 (52? pins), CN2 (10 pins), CN3 (6 pins), CN4 (5 pins)
    XTALs: 25.000 MHz (@OSC1?), 20.00 MHz (@X1), 15.00 MHz (@X2), 4.096 MHz (@X3)
    Jumpers: ホッパー        JP1 / JP2  =  12V / 24V     (hopper voltage)
             ウォッチドック  JP3        =                (watchdog enable?)
             PRG ロム       JP4 / JP5  =  4M? / 1 or 2M (rom size)

  Eproms:
    IC1 ST 27C1001
    IC2 ST 27C4001
    IC3 ST 27C800

***************************************************************************/

ROM_START( lufykzku )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ka-102_p1__ver1.02.ic1", 0x000000, 0x020000, CRC(5d4f3405) SHA1(e2d982465ba847e9883dbb1c9a95c24eeae4611d) ) // hand-written label, 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "ka-102_g1__ver1.00.ic3", 0x000000, 0x100000, CRC(abaca89d) SHA1(ad42eb8536373fb4fcff8f04ffe9b67dc62e0423) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "ka-102_s1__ver1.00.ic2", 0x000000, 0x080000, CRC(65f800d5) SHA1(03afe2f7a0731e7c3bc7c86e1a0dcaea0796e87f) )
ROM_END

void lufykzku_state::init_lufykzku()
{
	m_vblank_vector = 0xfa; // nop
	m_timer0_vector = 0xfc; // write coin counters/lockout, drive hopper
	m_timer1_vector = 0xfe; // read inputs and hopper sensor, handle coin in

	m_gfxdecode->gfx(1)->set_granularity(16);
}

void lufykzku_state::init_rockman()
{
	m_vblank_vector = 0xfa; // nop
	m_timer0_vector = 0;
	m_timer1_vector = 0xfe;

	m_gfxdecode->gfx(1)->set_granularity(16);
}

// Banpresto BPSC-2001M-A PCB, same as lufykzku
ROM_START( mnrockman )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ka-108_p1_ver1.02.ic1", 0x000000, 0x020000, CRC(727edf2f) SHA1(51a5f89a9ba64e16a1f46cc1145efa792ebb6401) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "ka-108_g1_ver1.00.ic3", 0x000000, 0x200000, CRC(ef79a6de) SHA1(50cbe7665e80b58a6bb0b20bae2deeca2e29c9da) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "ka-108_s1_ver1.00.ic2", 0x000000, 0x080000, CRC(828dd3bd) SHA1(9788a30199d81f6db54f5409fcb146098a29e6aa) )
ROM_END

/***************************************************************************

  Sammy Medal Games

  PCB:

    Sammy AM3AHF-01 SC MAIN PCB VER2 (Etched)
    MAIN PCB VER2 VM12-6001-0 (Sticker)

  CPU:

    KAWASAKI KL5C80A12CFP (@U1) - Z80 Compatible High Speed Microcontroller
    XTAL 20 MHz  (@X1)
    MX29F040TC-12 VM1211L01 (@U2) - 4M-bit [512kx8] CMOS Equal Sector Flash Memory
    BSI BS62LV256SC-70      (@U4) - Very Low Power/Voltage CMOS SRAM 32K X 8 bit
    (or Toshiba TC55257DFL-70L)

  Video:

    TAXAN KY-3211 (@U17)
    OKI M548262-60 (@U18) - 262144-Word x 8-Bit Multiport DRAM
    XTAL 27 MHz (@X3)

  Sound:

    OKI M9810B (@U11)
    XTAL 4.09 MHz (@X2)
    Trimmer (@VR1)
    Toshiba TA7252AP (@U16) - 5.9W Audio Power Amplifier

  Other:

    Xilinx XC9536 VM1212F01 (@U5) - In-System Programmable CPLD
    MX29F0?TC (@U3) - Empty 32 Pin DIP Socket
    M5295A (@U8) - Watchdog Timer (Near CUT-DEBUG MODE Jumper)
    M93C46MN6T (@U6) - Serial EEPROM
    RTC8564 (@U7) - not populated
    SN75C1168 (@U10) - Dual RS-422 Transceiver
    Cell Battery (@BAT)
    25 Pin Edge Connector
    56 Pin Cartridge Connector
    6 Pin Connector - +5V (1), GND (2), TCLK (3), TDO (4), TDI (5), TMS (6)

  On Go Go Cowboy, U2 and U10 are unpopulated, but U3 is occupied by a
  ST M27C4001-10F1 EPROM.

***************************************************************************/

#define SAMMYMDL_BIOS                                                                                                           \
	ROM_REGION( 0x80000, "mainbios", 0 )                                                                                        \
	ROM_SYSTEM_BIOS( 0, "v5", "IPL Ver. 5.0" ) /* (c)2003 */                                                                    \
	ROMX_LOAD( "vm1211l01.u2",  0x000000, 0x080000, CRC(c3c74dc5) SHA1(07352e6dba7514214e778ba39e1ca773e4698858), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "v4", "IPL Ver. 4.0" ) /* (c)2000, ROM patches not correct for this BIOS */                             \
	ROMX_LOAD( "mt201l04.u012", 0x000000, 0x080000, CRC(c8c6d25f) SHA1(5be39fa72b65f2e455ccc146dbab58d24ab46505), ROM_BIOS(1) )

ROM_START( sammymdl )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )

	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_COPY( "mainbios", 0x00fc00, 0x0000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASEFF )
ROM_END

/***************************************************************************

  Animal Catch ( VX2002L02 ANIMALCAT 200011211536 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  1 x MX29F1610ATC-12 (@U021)
  1 x MX29F1610TC-12  (@U016)

***************************************************************************/

ROM_START( animalc )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(84cf123b) SHA1(d8b425c93ff1a560e3f92c70d7eb93a05c3581af) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(4ae14ff9) SHA1(1273d15ea642452fecacff572655cd3ab47a5884) )   // 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

ROM_START( gunkids )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(3d989a45) SHA1(f4e1dc13bfe9ef8bf733735b6647946dda6962f2) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(5e356b68) SHA1(0e4e28b02dcb5ff7d2a7139c5cdf31cbd08167f4) )
ROM_END

void sammymdl_state::init_animalc()
{
	uint8_t *rom = memregion("mainbios")->base();

	// video timing loops
	rom[0x015d9] = 0x00;
	rom[0x015da] = 0x00;
	rom[0x01605] = 0x00;
	rom[0x01606] = 0x00;
	rom[0x01750] = 0x00;
	rom[0x01751] = 0x00;

	// force jump out of BIOS loop
	rom[0x005ac] = 0xc3;
}

/***************************************************************************

  Go Go Cowboy

  Cart:

    ガムボール     <- gumball
    with プライズ  <- with prize
    EM5701L01
    EM5701L01

  PCB:

    Sammy AM3AHF-01 SC MAIN PCB VER2 (Etched)
    SC MAIN PCB VER2(GUM) 5049-6001-0 (Sticker)

***************************************************************************/

ROM_START( gocowboy )
	ROM_REGION( 0x80000, "mainbios", 0 )
	ROM_LOAD( "go_go_cowboy_gpt_2c9c.u3", 0x00000, 0x80000, CRC(ad9b1de6) SHA1(24809ec3a579d28189a98190db70a33217e4f8bc) ) // uses custom BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "em702l01.u021", 0x000000, 0x200000, CRC(4c4289fe) SHA1(517b5a1e9d91e7ed322b4792d863e7abda835d4a) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "em701l01.u016", 0x000000, 0x200000, CRC(c1f07320) SHA1(734717140e66ddcf0bded1489156c51cdaf1b50c) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "st93c46.u6", 0x00, 0x80, CRC(1af2d376) SHA1(afbe953f1a9ff0152fe1092a83482695dbe5e75d) )

	ROM_REGION( 0x5cde, "pld", 0 )
	ROM_LOAD( "vm1212f01.u5.jed", 0x0000, 0x5cde, CRC(b86a1825) SHA1(cc2e633fb8a24cfc93291a778b0964089f6b8ac7) )
ROM_END

/***************************************************************************

  Itazura Monkey ( VX1902L02 ITZRMONKY 200011211639 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

***************************************************************************/

ROM_START( itazuram )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx2002l01.u021", 0x00000, 0x200000, CRC(ddbdd2f3) SHA1(91f67a938929be0261442e066e3d2c03b5e9f06a) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx2001l01.u016", 0x00000, 0x200000, CRC(9ee95222) SHA1(7154d43ef312a48a882207ca37e1c61e8b215a9b) )
ROM_END

void sammymdl_state::init_itazuram()
{
	uint8_t *rom = memregion("mainbios")->base();

	// force jump out of BIOS loop
	rom[0x005ac] = 0xc3;
}

/***************************************************************************

  Taihou de Doboon ( EM4210L01 PUSHERDBN 200203151028 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

  The flash roms found in the cart are labeled:

  vx1801l01.u016
  vx1802l01.u021

  which correspond to "Pye-nage Taikai". But they were reflashed with the
  software noted in the sticker on the back of the cart (rom names reflect that):

  System: Treasure Hall
  Soft: Taihou de Doboon
  2003.02.14
  Char Rev: EM4209L01
  Pro  Rev: EM4210L01

***************************************************************************/

ROM_START( tdoboon )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "em4210l01.u021", 0x00000, 0x200000, CRC(3523e314) SHA1(d07c5d17d3f285be4cde810547f427e84f98968f) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "em4209l01.u016", 0x00000, 0x200000, CRC(aca220fa) SHA1(7db441add16af554700e597fd9926b6ccd19d628) )   // 1xxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

/***************************************************************************

  Pye-nage Taikai ( VX1802L01 PAINAGETK 200011021216 SAMMY CORP. AM. )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

***************************************************************************/

ROM_START( pyenaget )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "vx1802l01.u021", 0x00000, 0x200000, CRC(7a22a657) SHA1(2a98085862fd958209253c5401e41eae4f7c06ea) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx1801l01.u016", 0x00000, 0x200000, CRC(c4607403) SHA1(f4f4699442afccc5ed4354447f91b1bee36ae3e5) )
ROM_END

/***************************************************************************

  Hae Hae Ka Ka Ka ( EM4208L01 PUSHERHAEHAE 200203151032 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

  The flash roms found in the cart are labeled:

  vx1801l01.u016
  vx1802l01.u021

  which correspond to "Pye-nage Taikai". But they were reflashed with the
  software noted in the sticker on the back of the cart (rom names reflect that):

  System: Treasure Hall
  Soft: Hae Hae Ka Ka Ka
  2003.02.14
  Char Rev: EM4207L01
  Pro  Rev: EM4208L01

***************************************************************************/

ROM_START( haekaka )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "em4208l01.u021", 0x00000, 0x200000, CRC(d23bb748) SHA1(38d5b6c4b2cd470b3a68574aeca3f9fa9032245e) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "em4207l01.u016", 0x00000, 0x200000, CRC(3876961c) SHA1(3d842c1f63ea5aa7e799967928b86c5fabb4e65e) )
ROM_END

void sammymdl_state::init_haekaka()
{
	uint8_t *rom = memregion("mainbios")->base();

	// force jump out of BIOS loop
	rom[0x005ac] = 0xc3;
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// Sigma Medal Games
GAME( 1997, dodghero, 0,        dodghero, sigma_1b, sigmab98_state, empty_init,    ROT0, "Sigma",             "Minna Atsumare! Dodge Hero",           0 )
GAME( 1997, sushimar, 0,        dodghero, sigma_3b, sigmab98_state, empty_init,    ROT0, "Sigma",             "Itazura Daisuki! Sushimaru Kun",       0 )
GAME( 1997, gegege,   0,        sigmab98, sigma_1b, sigmab98_state, init_gegege,   ROT0, "Sigma / Banpresto", "GeGeGe no Kitarou Youkai Slot",        0 )
GAME( 1997, b3rinsya, 0,        sigmab98, sigma_5b, sigmab98_state, init_b3rinsya, ROT0, "Sigma",             "Burning Sanrinsya - Burning Tricycle", 0 ) // 1997 in the rom
GAME( 1997, pepsiman, 0,        sigmab98, sigma_3b, sigmab98_state, init_pepsiman, ROT0, "Sigma",             "PEPSI Man",                            0 )
GAME( 1998, tbeastw2, 0,        sigmab98, sigma_3b, sigmab98_state, init_tbeastw2, ROT0, "Sigma / Transformer Production Company / Takara", "Transformers Beast Wars II", 0 ) // 1997 in the rom
GAME( 1997, ucytokyu, 0,        sigmab98, sigma_js, sigmab98_state, init_ucytokyu, ROT0, "Sigma",             "Uchuu Tokkyuu Medalian",               0 ) // Banpresto + others in the ROM
GAME( 2000, dashhero, 0,        sigmab98, sigma_1b, sigmab98_state, init_dashhero, ROT0, "Sigma",             "Minna Ganbare! Dash Hero",             MACHINE_NOT_WORKING ) // 1999 in the rom
// Banpresto Medal Games
GAME( 2001, lufykzku, 0,        lufykzku, lufykzku, lufykzku_state, init_lufykzku, ROT0, "Banpresto / Eiichiro Oda / Shueisha - Fuji TV - Toho Animation", "Otakara Itadaki Luffy Kaizoku-Dan! (Japan, v1.02)", 0 )
GAME( 2002, mnrockman,0,        rockman,  lufykzku, lufykzku_state, init_rockman,  ROT0, "Banpresto / Capcom / Shogakukan / ShoPro / TV Tokyo", "Medal Network: Rockman EXE", 0 )
// Sammy Medal Games:
GAME( 2000, sammymdl, 0,        sammymdl, sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Sammy Medal Game System BIOS",         MACHINE_IS_BIOS_ROOT )
GAME( 2000, animalc,  sammymdl, animalc,  sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Animal Catch",                         0 )
GAME( 2000, itazuram, sammymdl, itazuram, sammymdl, sammymdl_state, init_itazuram, ROT0, "Sammy",             "Itazura Monkey",                       0 )
GAME( 2000, pyenaget, sammymdl, pyenaget, sammymdl, sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Pye-nage Taikai",                      0 )
GAME( 2000, tdoboon,  sammymdl, tdoboon,  haekaka,  sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Taihou de Doboon",                     0 )
GAME( 2001, haekaka,  sammymdl, haekaka,  haekaka,  sammymdl_state, init_haekaka,  ROT0, "Sammy",             "Hae Hae Ka Ka Ka",                     0 )
GAME( 2002, gunkids,  sammymdl, animalc,  sammymdl, sammymdl_state, init_animalc,  ROT0, "Sammy",             "Hayauchi Gun Kids",                    0 )
GAME( 2003, gocowboy, 0,        gocowboy, gocowboy, sammymdl_state, empty_init,    ROT0, "Sammy",             "Go Go Cowboy (English, prize)",        0 )
