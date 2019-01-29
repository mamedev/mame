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

CPU     :   Kawasaki KL5C80A120FP (Z80 Compatible High Speed Microcontroller)
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

- KL5C80 emulation is needed to consolidate the sammymdl games in one memory map and to run the BIOS
- Remove ROM patches: gegege checks the EEPROM output after reset, and wants a timed 0->1 transition or locks up while
  saving setting in service mode. Using a reset_delay of 7 works, unless when "play style" is set
  to "coin" (it probably changes the number of reads from port $C0).
  I guess the reset_delay mechanism should be implemented with a timer in eeprom.c.
- pyenaget intro: when the theater scrolls out to the left, the train should scroll in from the right,
  with no visible gaps. It currently leaves the screen empty instead, for several seconds.
- tdoboon: no smoke from hit planes as shown in the video? Tiles are present (f60-125f) and used in demo mode.
- dashhero does not acknowledge the button bashing correctly, it's very hard to win (a slower pace works better!)

Notes:

- "BACKUP RAM NG" error: in test mode, choose "SET MODE" -> "RAM CLEAR" and keep the button pressed for long.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
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


class sigmab98_state : public driver_device
{
public:
	sigmab98_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		// Required devices
		m_maincpu(*this,"maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		// Optional devices
		m_buffered_spriteram(*this, "spriteram"),
		m_nvramdev(*this, "nvram"),
		m_eeprom(*this, "eeprom"),
		m_hopper(*this, "hopper"),
		m_hopper_small(*this, "hopper_small"),
		m_hopper_large(*this, "hopper_large"),
		// Shared pointers
		m_nvram(*this, "nvram"),
		m_spriteram(*this, "spriteram"),
		m_vregs(*this, "vregs"),
		m_vtable(*this, "vtable"),
		m_leds(*this, "led%u", 0U)
	{ }

	void sigmab98(machine_config &config);
	void pyenaget(machine_config &config);
	void dodghero(machine_config &config);
	void dashhero(machine_config &config);
	void gegege(machine_config &config);
	void haekaka(machine_config &config);
	void gocowboy(machine_config &config);
	void tdoboon(machine_config &config);
	void animalc(machine_config &config);
	void sammymdl(machine_config &config);
	void itazuram(machine_config &config);

	void init_dodghero();
	void init_b3rinsya();
	void init_tbeastw2();
	void init_dashhero();
	void init_gegege();
	void init_pepsiman();
	void init_itazuram();
	void init_animalc();
	void init_ucytokyu();
	void init_haekaka();
	void init_gocowboy();

	uint32_t screen_update_sigmab98(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	DECLARE_WRITE8_MEMBER(gegege_regs_w);
	DECLARE_READ8_MEMBER(gegege_regs_r);
	DECLARE_WRITE8_MEMBER(gegege_regs2_w);
	DECLARE_READ8_MEMBER(gegege_regs2_r);

	DECLARE_WRITE8_MEMBER(dodghero_regs_w);
	DECLARE_READ8_MEMBER(dodghero_regs_r);
	DECLARE_WRITE8_MEMBER(dodghero_regs2_w);
	DECLARE_READ8_MEMBER(dodghero_regs2_r);

	DECLARE_WRITE8_MEMBER(dashhero_regs2_w);
	DECLARE_READ8_MEMBER(dashhero_regs2_r);

	DECLARE_WRITE8_MEMBER(vregs_w);
	DECLARE_READ8_MEMBER(vregs_r);
	DECLARE_READ8_MEMBER(d013_r);
	DECLARE_READ8_MEMBER(d021_r);

	DECLARE_WRITE8_MEMBER(c4_w);
	DECLARE_WRITE8_MEMBER(c6_w);
	DECLARE_WRITE8_MEMBER(c8_w);

	DECLARE_WRITE8_MEMBER(animalc_rombank_w);
	DECLARE_READ8_MEMBER(animalc_rombank_r);
	DECLARE_WRITE8_MEMBER(animalc_rambank_w);
	DECLARE_READ8_MEMBER(animalc_rambank_r);

	DECLARE_READ8_MEMBER(unk_34_r);
	DECLARE_WRITE8_MEMBER(vblank_w);
	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_READ8_MEMBER(sammymdl_coin_counter_r);
	DECLARE_WRITE8_MEMBER(sammymdl_coin_counter_w);
	DECLARE_READ8_MEMBER(sammymdl_leds_r);
	DECLARE_WRITE8_MEMBER(sammymdl_leds_w);
	DECLARE_WRITE8_MEMBER(sammymdl_hopper_w);
	DECLARE_READ8_MEMBER(sammymdl_coin_hopper_r);

	DECLARE_WRITE8_MEMBER(gocowboy_rombank_w);
	DECLARE_READ8_MEMBER(gocowboy_rombank_r);
	DECLARE_WRITE8_MEMBER(gocowboy_rambank_w);
	DECLARE_READ8_MEMBER(gocowboy_rambank_r);
	DECLARE_WRITE8_MEMBER(gocowboy_4400_w);
	DECLARE_READ8_MEMBER(gocowboy_4400_r);
	DECLARE_WRITE8_MEMBER(gocowboy_dc00_w);
	DECLARE_READ8_MEMBER(gocowboy_dc00_r);
	DECLARE_WRITE8_MEMBER(gocowboy_leds_w);

	DECLARE_WRITE8_MEMBER(haekaka_rombank_w);
	DECLARE_READ8_MEMBER(haekaka_rombank_r);
	DECLARE_WRITE8_MEMBER(haekaka_rambank_w);
	DECLARE_READ8_MEMBER(haekaka_rambank_r);
	DECLARE_READ8_MEMBER(haekaka_vblank_r);
	DECLARE_READ8_MEMBER(haekaka_b000_r);
	DECLARE_WRITE8_MEMBER(haekaka_b000_w);
	DECLARE_WRITE8_MEMBER(haekaka_leds_w);
	DECLARE_WRITE8_MEMBER(haekaka_coin_counter_w);

	DECLARE_WRITE8_MEMBER(itazuram_rombank_w);
	DECLARE_READ8_MEMBER(itazuram_rombank_r);
	DECLARE_WRITE8_MEMBER(itazuram_rambank_w);
	DECLARE_READ8_MEMBER(itazuram_rambank_r);
	DECLARE_WRITE8_MEMBER(itazuram_nvram_palette_w);
	DECLARE_WRITE8_MEMBER(itazuram_palette_w);
	DECLARE_READ8_MEMBER(itazuram_palette_r);

	DECLARE_WRITE8_MEMBER(tdoboon_rombank_w);
	DECLARE_READ8_MEMBER(tdoboon_rombank_r);
	DECLARE_WRITE8_MEMBER(tdoboon_rambank_w);
	DECLARE_READ8_MEMBER(tdoboon_rambank_r);
	DECLARE_READ8_MEMBER(tdoboon_c000_r);
	DECLARE_WRITE8_MEMBER(tdoboon_c000_w);

	void show_outputs();
	void show_3_outputs();
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(sammymdl_eeprom_r);
	DECLARE_WRITE8_MEMBER(sammymdl_eeprom_w);

	DECLARE_MACHINE_RESET(sigmab98);
	DECLARE_MACHINE_RESET(sammymdl);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank_sammymdl);
	INTERRUPT_GEN_MEMBER(sigmab98_vblank_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(sammymdl_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_mask);

	void animalc_io(address_map &map);
	void animalc_map(address_map &map);
	void dashhero_io_map(address_map &map);
	void dodghero_io_map(address_map &map);
	void dodghero_mem_map(address_map &map);
	void gegege_io_map(address_map &map);
	void gegege_mem_map(address_map &map);
	void gocowboy_io(address_map &map);
	void gocowboy_map(address_map &map);
	void haekaka_io(address_map &map);
	void haekaka_map(address_map &map);
	void itazuram_io(address_map &map);
	void itazuram_map(address_map &map);
	void pyenaget_io(address_map &map);
	void tdoboon_io(address_map &map);
	void tdoboon_map(address_map &map);

	virtual void machine_start() override { m_leds.resolve(); }
	virtual void video_start() override;

	// Required devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	// Optional devices
	optional_device<buffered_spriteram8_device> m_buffered_spriteram;   // not on sammymdl?
	optional_device<nvram_device> m_nvramdev; // battery backed RAM (should be required, but dashhero breaks with it)
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<ticket_dispenser_device> m_hopper;
	optional_device<ticket_dispenser_device> m_hopper_small;
	optional_device<ticket_dispenser_device> m_hopper_large;
	// Shared pointers
	required_shared_ptr<uint8_t> m_nvram;
	optional_shared_ptr<uint8_t> m_spriteram; // optional as some games allocate it themselves (due to banking)
	optional_shared_ptr<uint8_t> m_vregs;     // optional as some games allocate it themselves (due to banking)
	optional_shared_ptr<uint8_t> m_vtable;    // optional as some games allocate it themselves (due to banking)
	output_finder<8> m_leds;

	std::vector<uint8_t> m_paletteram;

	std::unique_ptr<bitmap_ind16> m_sprite_bitmap;
	bool new_sprite_chip; // KY-10510 has a slightly different sprite format than KY-3211

	uint8_t m_reg;
	uint8_t m_rombank;
	uint8_t m_reg2;
	uint8_t m_rambank;

	uint8_t m_vblank_vector;
	uint8_t m_timer0_vector;
	uint8_t m_timer1_vector;

	uint8_t m_c0;
	uint8_t m_c4;
	uint8_t m_c6;
	uint8_t m_c8;
	uint8_t m_vblank;
	uint8_t m_out[3];
};


class lufykzku_state : public sigmab98_state
{
public:
	lufykzku_state(const machine_config &mconfig, device_type type, const char *tag) :
		sigmab98_state(mconfig, type, tag),
		m_watchdog(*this, "watchdog_mb3773"),
		m_dsw_shifter(*this, "ttl165_%u", 1U),
		m_dsw_bit(0)
	{
		new_sprite_chip = true;
	}

	required_device<mb3773_device> m_watchdog;
	required_device_array<ttl165_device, 2> m_dsw_shifter;

	int m_dsw_bit;
	DECLARE_WRITE_LINE_MEMBER(dsw_w);

	DECLARE_WRITE8_MEMBER(lufykzku_regs_w);
	DECLARE_READ8_MEMBER(lufykzku_regs_r);
	DECLARE_WRITE8_MEMBER(lufykzku_c4_w);
	DECLARE_WRITE8_MEMBER(lufykzku_c6_w);
	DECLARE_READ8_MEMBER(lufykzku_c8_r);
	DECLARE_WRITE8_MEMBER(lufykzku_c8_w);
	DECLARE_WRITE8_MEMBER(lufykzku_watchdog_w);

	DECLARE_MACHINE_RESET(lufykzku);
	void init_lufykzku();

	TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_irq);

	void lufykzku(machine_config &config);
	void lufykzku_io_map(address_map &map);
	void lufykzku_mem_map(address_map &map);
};


/***************************************************************************

    Video

***************************************************************************/

void sigmab98_state::video_start()
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

void sigmab98_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_mask)
{
	uint8_t *end      =   (m_buffered_spriteram ? m_buffered_spriteram->buffer() : m_spriteram) - 0x10;
	uint8_t *s        =   end + 0x1000;

	for ( ; s != end; s -= 0x10 )
	{
		if ( (s[ 0x01 ] & (new_sprite_chip ? 0x0c : 0x04)) == 0)
			continue;

		if ( ((1 << (s[ 0x01 ] & 0x03)) & pri_mask) == 0 )
			continue;

		int color   =   s[ 0x00 ] & (new_sprite_chip ? 0xff : 0xf);

		int gfx     =   (s[ 0x01 ] & (new_sprite_chip ? 0x08 : 0x40)) ? 1 : 0;
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
		int x0, x1, dx, flipx = new_sprite_chip ? s[ 0x04 ] & 0x04 : s[ 0x01 ] & 0x10;
		int y0, y1, dy, flipy = new_sprite_chip ? s[ 0x06 ] & 0x04 : s[ 0x01 ] & 0x08;

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
		uint16_t *src = &m_sprite_bitmap->pix16(0);
		uint16_t *dst = &bitmap.pix16(0);

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

uint32_t sigmab98_state::screen_update_sigmab98(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

WRITE8_MEMBER(sigmab98_state::vregs_w)
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

READ8_MEMBER(sigmab98_state::vregs_r)
{
	switch (offset)
	{
		default:
			if (!machine().side_effects_disabled())
				logerror("%s: unknown video reg read: %02x\n", machine().describe_context(), offset);
			return m_vregs[offset];
	}
}

READ8_MEMBER(sigmab98_state::d013_r)
{
	// bit 5 must go 0->1 (vblank?)
	// bit 2 must be set (sprite buffered? triggered by pulsing bit 3 of port C6?)
//  return (m_screen->vblank() ? 0x20 : 0x00) | 0x04;
	return (m_screen->vblank() ? 0x20 : 0x01) | 0x04;
//  return machine().rand();
}
READ8_MEMBER(sigmab98_state::d021_r)
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

// rombank
WRITE8_MEMBER(sigmab98_state::dodghero_regs_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	switch ( m_reg )
	{
		case 0x1f:
			m_rombank = data;
			if (data >= 0x18)
				logerror("%s: unknown rom bank = %02x\n", machine().describe_context(), data);
			else
				membank("rombank")->set_entry(data * 2);
			break;

		case 0x9f:
			m_rombank = data;
			if (data >= 0x18)
				logerror("%s: unknown rom bank2 = %02x\n", machine().describe_context(), data);
			else
				membank("rombank")->set_entry(data * 2 + 1);
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}
READ8_MEMBER(sigmab98_state::dodghero_regs_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x1f:
		case 0x9f:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

// rambank
WRITE8_MEMBER(sigmab98_state::dodghero_regs2_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0x37:
			m_rambank = data;
			switch (data)
			{
				case 0x32:
					membank("rambank")->set_entry(0);
					break;
				case 0x36:
					membank("rambank")->set_entry(1);
					break;
				default:
					logerror("%s: unknown ram bank = %02x\n", machine().describe_context(), data);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}
READ8_MEMBER(sigmab98_state::dodghero_regs2_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x37:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}

void sigmab98_state::dodghero_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xa7ff).bankr("rombank");

	map(0xa800, 0xb7ff).ram().share("spriteram");

	map(0xc800, 0xc9ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0xd001, 0xd07f).ram().share("vtable");

	map(0xd800, 0xdfff).bankrw("rambank");    // not used, where is it mapped?

	map(0xd800, 0xd821).rw(FUNC(sigmab98_state::vregs_r), FUNC(sigmab98_state::vregs_w)).share("vregs");
	map(0xd813, 0xd813).r(FUNC(sigmab98_state::d013_r));
	map(0xd821, 0xd821).r(FUNC(sigmab98_state::d021_r));

	map(0xe000, 0xefff).ram().share("nvram"); // battery backed RAM

	map(0xf000, 0xffff).ram();
}

void sigmab98_state::dodghero_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0xa0, 0xa1).rw(FUNC(sigmab98_state::dodghero_regs_r), FUNC(sigmab98_state::dodghero_regs_w));
//  AM_RANGE( 0xa2, 0xa3 )
	map(0xa4, 0xa5).rw(FUNC(sigmab98_state::dodghero_regs2_r), FUNC(sigmab98_state::dodghero_regs2_w));

	map(0xc0, 0xc0).portr("EEPROM").w(FUNC(sigmab98_state::eeprom_w));
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(sigmab98_state::c4_w));
	map(0xc6, 0xc6).w(FUNC(sigmab98_state::c6_w));
	map(0xc8, 0xc8).w(FUNC(sigmab98_state::c8_w));
}

/***************************************************************************
                        GeGeGe no Kitarou Youkai Slot
***************************************************************************/

// rombank
WRITE8_MEMBER(sigmab98_state::gegege_regs_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	switch ( m_reg )
	{
		case 0x1f:
			m_rombank = data;
			if (data >= 0x18)
				logerror("%s: unknown rom bank = %02x\n", machine().describe_context(), data);
			else
				membank("rombank")->set_entry(data);
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}
READ8_MEMBER(sigmab98_state::gegege_regs_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x1f:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

// rambank
WRITE8_MEMBER(sigmab98_state::gegege_regs2_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0xb5:
			m_rambank = data;
			switch (data)
			{
				case 0x32:
					membank("rambank")->set_entry(0);
					break;
				case 0x36:
					membank("rambank")->set_entry(1);
					break;
				default:
					logerror("%s: unknown ram bank = %02x\n", machine().describe_context(), data);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}
READ8_MEMBER(sigmab98_state::gegege_regs2_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0xb5:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}


// Outputs

void sigmab98_state::show_outputs()
{
#ifdef MAME_DEBUG
	popmessage("0: %02X  4: %02X  6: %02X  8: %02X", m_c0, m_c4, m_c6, m_c8);
#endif
}

// Port c0
WRITE8_MEMBER(sigmab98_state::eeprom_w)
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
WRITE8_MEMBER(sigmab98_state::c4_w)
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
WRITE8_MEMBER(sigmab98_state::c6_w)
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
WRITE8_MEMBER(sigmab98_state::c8_w)
{
	m_hopper->motor_w((!(data & 0x02) && (data & 0x01)) ? 0 : 1);

	m_c8 = data;
	show_outputs();
}

void sigmab98_state::gegege_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("rombank");

	map(0xa000, 0xafff).ram().share("spriteram");

	map(0xc000, 0xc1ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0xc800, 0xc87f).ram().share("vtable");

	map(0xd000, 0xd021).rw(FUNC(sigmab98_state::vregs_r), FUNC(sigmab98_state::vregs_w)).share("vregs");
	map(0xd013, 0xd013).r(FUNC(sigmab98_state::d013_r));
	map(0xd021, 0xd021).r(FUNC(sigmab98_state::d021_r));

	map(0xd800, 0xdfff).bankrw("rambank");

	map(0xe000, 0xefff).ram().share("nvram"); // battery backed RAM

	map(0xf000, 0xffff).ram();
}

void sigmab98_state::gegege_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0xa0, 0xa1).rw(FUNC(sigmab98_state::gegege_regs_r), FUNC(sigmab98_state::gegege_regs_w));
//  AM_RANGE( 0xa2, 0xa3 )
	map(0xa4, 0xa5).rw(FUNC(sigmab98_state::gegege_regs2_r), FUNC(sigmab98_state::gegege_regs2_w));

	map(0xc0, 0xc0).portr("EEPROM").w(FUNC(sigmab98_state::eeprom_w));
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(sigmab98_state::c4_w));
	map(0xc6, 0xc6).w(FUNC(sigmab98_state::c6_w));
	map(0xc8, 0xc8).w(FUNC(sigmab98_state::c8_w));

	map(0xe5, 0xe5).nopr();   // during irq
}

/***************************************************************************
                           Minna Ganbare! Dash Hero
***************************************************************************/

// rambank
WRITE8_MEMBER(sigmab98_state::dashhero_regs2_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0x75:
		case 0xb5:
		case 0xf5:
			m_rambank = data;
			switch (data)
			{
				case 0x32:
					membank("rambank")->set_entry(0);
					break;
				case 0x34:
					membank("rambank")->set_entry(1);
					break;
				case 0x36:
					membank("rambank")->set_entry(2);
					break;
				case 0x39:
					membank("rambank")->set_entry(3);
					break;
				default:
					logerror("%s: unknown ram bank = %02x\n", machine().describe_context(), data);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}
READ8_MEMBER(sigmab98_state::dashhero_regs2_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x75:
		case 0xb5:
		case 0xf5:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}

void sigmab98_state::dashhero_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0xa0, 0xa1).rw(FUNC(sigmab98_state::gegege_regs_r), FUNC(sigmab98_state::gegege_regs_w));
	//  AM_RANGE( 0xa2, 0xa3 )
	map(0xa4, 0xa5).rw(FUNC(sigmab98_state::dashhero_regs2_r), FUNC(sigmab98_state::dashhero_regs2_w));

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

// romrambank
WRITE8_MEMBER(lufykzku_state::lufykzku_regs_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	switch ( m_reg )
	{
		case 0x1f: // 8000
			m_rombank = data;
			switch (data)
			{
				case 0x00: // (8000) ROM
					membank("romrambank")->set_entry(0);
					break;

				case 0x78: // (80000) NVRAM
					membank("romrambank")->set_entry(1);
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}

READ8_MEMBER(lufykzku_state::lufykzku_regs_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x00:
		case 0x78:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

WRITE_LINE_MEMBER(lufykzku_state::dsw_w)
{
	m_dsw_bit = state;
}

// Port c0
WRITE8_MEMBER(lufykzku_state::lufykzku_watchdog_w)
{
	m_watchdog->write_line_ck(BIT(data, 7));
}

// Port c4
WRITE8_MEMBER(lufykzku_state::lufykzku_c4_w)
{
	machine().bookkeeping().coin_lockout_w(1, (~data) & 0x20); // 100 yen lockout
//  machine().bookkeeping().coin_lockout_w(2, (~data) & 0x40); // (unused coin lockout)
	machine().bookkeeping().coin_lockout_w(0, (~data) & 0x80); // medal lockout

	m_c4 = data;
	show_outputs();
}

// Port c6
WRITE8_MEMBER(lufykzku_state::lufykzku_c6_w)
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
READ8_MEMBER(lufykzku_state::lufykzku_c8_r)
{
	return 0xbf | (m_dsw_bit ? 0x40 : 0);
}

WRITE8_MEMBER(lufykzku_state::lufykzku_c8_w)
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
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankrw("romrambank").share("nvram"); // ROM | NVRAM

	map(0xc000, 0xcfff).ram().share("spriteram");

	map(0xd000, 0xefff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // more palette entries

	map(0xf000, 0xf021).rw(FUNC(lufykzku_state::vregs_r), FUNC(lufykzku_state::vregs_w)).share("vregs");
	map(0xf013, 0xf013).r(FUNC(lufykzku_state::d013_r));
	map(0xf021, 0xf021).r(FUNC(lufykzku_state::d021_r));

	map(0xf400, 0xf47f).ram().share("vtable");

	map(0xfc00, 0xffff).ram();
}

void lufykzku_state::lufykzku_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("oki", FUNC(okim9810_device::write));
	map(0x01, 0x01).w("oki", FUNC(okim9810_device::tmp_register_w));

	map(0xa2, 0xa3).rw(FUNC(lufykzku_state::lufykzku_regs_r), FUNC(lufykzku_state::lufykzku_regs_w));

	map(0xc0, 0xc0).portr("COIN").w(FUNC(lufykzku_state::lufykzku_watchdog_w)); // bit 7 -> watchdog
	map(0xc2, 0xc2).portr("BUTTON");
	map(0xc4, 0xc4).portr("PAYOUT").w(FUNC(lufykzku_state::lufykzku_c4_w)); // bit 7 = medal lock, bit 6 = coin3, bit 5 = yen
	map(0xc6, 0xc6).w(FUNC(lufykzku_state::lufykzku_c6_w));
	map(0xc8, 0xc8).rw(FUNC(lufykzku_state::lufykzku_c8_r), FUNC(lufykzku_state::lufykzku_c8_w)); // 0xc8 bit 6 read (eeprom?)
}


/***************************************************************************
                                 Animal Catch
***************************************************************************/

// rombank
WRITE8_MEMBER(sigmab98_state::animalc_rombank_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	uint8_t *rom = memregion("maincpu")->base();
	switch ( m_reg )
	{
		case 0x0f:
			m_rombank = data;
			switch (data)
			{
				case 0x10:  membank("rombank")->set_base(rom + 0x400 + 0x4000); break;
				case 0x14:  membank("rombank")->set_base(rom + 0x400 + 0x8000); break;
				case 0x18:  membank("rombank")->set_base(rom + 0x400 + 0xc000); break;
				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}
READ8_MEMBER(sigmab98_state::animalc_rombank_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x0f:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

// rambank
WRITE8_MEMBER(sigmab98_state::animalc_rambank_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	int bank = 0;
	switch ( m_reg2 )
	{
		case 0x1f:
			m_rambank = data;
			switch (data)
			{
				case 0x58:  bank = 0;   break;
				case 0x62:  bank = 1;   break;
				case 0x63:  bank = 2;   break;
				case 0x64:  bank = 3;   break;
				case 0x65:  bank = 4;   break;
				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", machine().describe_context(), data, m_reg2);
					return;
			}
			membank("rambank")->set_entry(bank);
			if ( (bank == 1) || (bank == 2) || (bank == 3) )
				membank("sprbank")->set_entry(bank-1);
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}
READ8_MEMBER(sigmab98_state::animalc_rambank_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x1f:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}


READ8_MEMBER(sigmab98_state::sammymdl_eeprom_r)
{
	return m_eeprom->do_read() ? 0x80 : 0;
}

WRITE8_MEMBER(sigmab98_state::sammymdl_eeprom_w)
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

READ8_MEMBER(sigmab98_state::unk_34_r)
{
	// mask 0x01?
	return 0x01;
}

READ8_MEMBER(sigmab98_state::vblank_r)
{
	// mask 0x04 must be set before writing sprite list
	// mask 0x10 must be set or irq/00 hangs?
	return  m_vblank | 0x14;
}

WRITE8_MEMBER(sigmab98_state::vblank_w)
{
	m_vblank = (m_vblank & ~0x03) | (data & 0x03);
}

WRITE_LINE_MEMBER(sigmab98_state::screen_vblank_sammymdl)
{
	// rising edge
	if (state)
	{
		m_vblank &= ~0x01;
	}
}

void sigmab98_state::show_3_outputs()
{
#ifdef MAME_DEBUG
	popmessage("COIN: %02X  LED: %02X  HOP: %02X", m_out[0], m_out[1], m_out[2]);
#endif
}
// Port 31
READ8_MEMBER(sigmab98_state::sammymdl_coin_counter_r)
{
	return m_out[0];
}
WRITE8_MEMBER(sigmab98_state::sammymdl_coin_counter_w)
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
READ8_MEMBER(sigmab98_state::sammymdl_leds_r)
{
	return m_out[1];
}
WRITE8_MEMBER(sigmab98_state::sammymdl_leds_w)
{
	m_leds[0] = BIT(data, 0);   // button

	m_out[1] = data;
	show_3_outputs();
}

// Port b0
// 02 hopper enable?
// 01 hopper motor on (active low)?
WRITE8_MEMBER(sigmab98_state::sammymdl_hopper_w)
{
	m_hopper->motor_w((!(data & 0x01) && (data & 0x02)) ? 0 : 1);

	m_out[2] = data;
	show_3_outputs();
}

READ8_MEMBER(sigmab98_state::sammymdl_coin_hopper_r)
{
	uint8_t ret = ioport("COIN")->read();

//  if ( !m_hopper->read(0) )
//      ret &= ~0x01;

	return ret;
}

void sigmab98_state::animalc_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0x8fff).bankrw("rambank").share("nvram");

	map(0x9000, 0x9fff).ram();
	map(0xa000, 0xafff).ram();
	map(0xb000, 0xbfff).bankrw("sprbank");

	map(0xd000, 0xd1ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd800, 0xd87f).ram().share("vtable");

	map(0xe000, 0xe021).rw(FUNC(sigmab98_state::vregs_r), FUNC(sigmab98_state::vregs_w)).share("vregs");
	map(0xe011, 0xe011).nopw();  // IRQ Enable? Screen disable?
	map(0xe013, 0xe013).rw(FUNC(sigmab98_state::vblank_r), FUNC(sigmab98_state::vblank_w));    // IRQ Ack?

	map(0xfe00, 0xffff).ram();   // High speed internal RAM
}

void sigmab98_state::animalc_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw(FUNC(sigmab98_state::animalc_rombank_r), FUNC(sigmab98_state::animalc_rombank_w));
	map(0x04, 0x05).rw(FUNC(sigmab98_state::animalc_rambank_r), FUNC(sigmab98_state::animalc_rambank_w));

	map(0x2c, 0x2c).rw(FUNC(sigmab98_state::sammymdl_eeprom_r), FUNC(sigmab98_state::sammymdl_eeprom_w));
	map(0x2e, 0x2e).r(FUNC(sigmab98_state::sammymdl_coin_hopper_r));
	map(0x30, 0x30).portr("BUTTON");
	map(0x31, 0x31).rw(FUNC(sigmab98_state::sammymdl_coin_counter_r), FUNC(sigmab98_state::sammymdl_coin_counter_w));
	map(0x32, 0x32).rw(FUNC(sigmab98_state::sammymdl_leds_r), FUNC(sigmab98_state::sammymdl_leds_w));
	map(0x34, 0x34).r(FUNC(sigmab98_state::unk_34_r));
	map(0x90, 0x90).w("oki", FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).w(FUNC(sigmab98_state::sammymdl_hopper_w));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                               Go Go Cowboy
***************************************************************************/

// rombank
WRITE8_MEMBER(sigmab98_state::gocowboy_rombank_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	switch ( m_reg )
	{
		case 0x50: // 4400
			m_rombank = data;
			switch (data)
			{
				case 0x13: // (17800) ROM
				case 0x15: // (19800) ROM
					break;
				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		case 0x90: // 4400
			m_rombank = data;
			switch (data)
			{
				case 0x0f: // (13c00) ROM
					break;
				case 0x17: // (1bc00) ROM
					break;
				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		case 0xd0: // 4400
			m_rombank = data;
			switch (data)
			{
				case 0x0f: // (14000) ROM
					break;

				case 0x5b: // (60000) clears 4400-c3ff
					break;

				case 0x5d: // (62000) copies 1404 bytes: 4400 <-> e6c6
					break;

				case 0x6b: // (70000) SPRITERAM + (72000) PALETTERAM + (72800) VTABLE + (73000) VREGS
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}

READ8_MEMBER(sigmab98_state::gocowboy_rombank_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x50:
		case 0x90:
		case 0xd0:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

READ8_MEMBER(sigmab98_state::gocowboy_4400_r)
{
	switch (m_rombank)
	{
		// ROM
		case 0x0f:
			return memregion("maincpu")->base()[offset + 0x3800 + (m_reg >> 6) * 0x400];
			break;

		case 0x13:
			return memregion("maincpu")->base()[offset + 0x7c00];
			break;

		case 0x15:
			return memregion("maincpu")->base()[offset + 0x9c00];
			break;

		case 0x17:
			return memregion("maincpu")->base()[offset + 0xc000];
			break;

		case 0x5b: // (60000) clears 4400-c3ff
			return m_nvram[offset];
			break;

		case 0x5d: // (62000) copies 1404 bytes: 4400 <-> e6c6
			if (offset + 0x2000 < 0x80000)
				return m_nvram[offset + 0x2000];
			break;

		case 0x6b: // (70000) SPRITERAM + (72000) PALETTERAM + (72800) VTABLE + (73000) VREGS
			if (offset < 0x1000)
				return m_spriteram[offset];
			else if ((offset >= 0x2000) && (offset < 0x2200))
				return m_paletteram[offset - 0x2000];
			else if ((offset >= 0x2800) && (offset < 0x2880))
				return m_vtable[offset - 0x2800];
			else if (offset >= 0x3000 && offset <= 0x3021)
				return vregs_r(space, offset - 0x3000);
			break;
	}

	logerror("%s: unknown read from %02x with rombank = %02x\n", machine().describe_context(), offset+0x4400, m_rombank);
	return 0x00;
}

WRITE8_MEMBER(sigmab98_state::gocowboy_4400_w)
{
	switch (m_rombank)
	{
		case 0x5b: // (60000) clears 4400-c3ff
			m_nvram[offset] = data;
			return;

		case 0x5d: // (62000) copies 1404 bytes: 4400 <-> e6c6
			if (offset + 0x2000 < 0x80000)
			{
				m_nvram[offset + 0x2000] = data;
				return;
			}
			break;

		case 0x6b: // (70000) SPRITERAM + (72000) PALETTERAM + (72800) VTABLE + (73000) VREGS
			if (offset < 0x1000)
			{
				m_spriteram[offset] = data;
				return;
			}
			else if ((offset >= 0x2000) && (offset < 0x2200))
			{
				m_palette->write8(space, offset-0x2000, data);
				return;
			}
			else if ((offset >= 0x2800) && (offset < 0x2880))
			{
				m_vtable[offset-0x2800] = data;
				return;
			}
			else if (offset >= 0x3000 && offset <= 0x3021)
			{
				vregs_w(space, offset - 0x3000, data);
				return;
			}
			break;
	}

	logerror("%s: unknown write to %02x = %02x with rombank = %02x\n", machine().describe_context(), offset + 0x4400, data, m_rombank);
}

// rambank
WRITE8_MEMBER(sigmab98_state::gocowboy_rambank_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0x76: // dc00
			m_rambank = data;
			switch (data)
			{
				case 0x52: // (60000) NVRAM
					break;

				case 0x64: // (72000) PALETTERAM
					break;

				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", machine().describe_context(), data, m_reg2);
					return;
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}

READ8_MEMBER(sigmab98_state::gocowboy_rambank_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x76:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}

READ8_MEMBER(sigmab98_state::gocowboy_dc00_r)
{
	switch (m_rambank)
	{
		case 0x52: // (60000) NVRAM
			return m_nvram[offset];

		case 0x64: // (72000) PALETTERAM
			return m_paletteram[offset];
	}

	logerror("%s: unknown read from %02x with rombank = %02x\n", machine().describe_context(), offset + 0xdc00, m_rambank);
	return 0;
}

WRITE8_MEMBER(sigmab98_state::gocowboy_dc00_w)
{
	switch (m_rambank)
	{
		case 0x52: // (60000) NVRAM
			m_nvram[offset] = data;
			return;

		case 0x64: // (72000) PALETTERAM
			m_palette->write8(space, offset, data);
			return;
	}

	logerror("%s: unknown write to %02x = %02x with rambank = %02x\n", machine().describe_context(), offset + 0xdc00, data, m_rambank);
}

void sigmab98_state::gocowboy_map(address_map &map)
{
	map(0x0000, 0x43ff).rom();

	map(0x4400, 0xdbff).rw(FUNC(sigmab98_state::gocowboy_4400_r), FUNC(sigmab98_state::gocowboy_4400_w));    // SPRITERAM + PALETTERAM + VTABLE + VREGS | NVRAM

	map(0xdc00, 0xfbff).rw(FUNC(sigmab98_state::gocowboy_dc00_r), FUNC(sigmab98_state::gocowboy_dc00_w)).share("nvram");  // PALETTERAM | NVRAM

	map(0xfe00, 0xffff).ram();   // High speed internal RAM
}


WRITE8_MEMBER(sigmab98_state::gocowboy_leds_w)
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

void sigmab98_state::gocowboy_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw(FUNC(sigmab98_state::gocowboy_rombank_r), FUNC(sigmab98_state::gocowboy_rombank_w));
	map(0x04, 0x05).rw(FUNC(sigmab98_state::gocowboy_rambank_r), FUNC(sigmab98_state::gocowboy_rambank_w));

	map(0x2c, 0x2c).rw(FUNC(sigmab98_state::sammymdl_eeprom_r), FUNC(sigmab98_state::sammymdl_eeprom_w));
	map(0x2e, 0x2e).r(FUNC(sigmab98_state::sammymdl_coin_hopper_r));
	map(0x30, 0x30).portr("BUTTON");
	map(0x31, 0x31).rw(FUNC(sigmab98_state::sammymdl_coin_counter_r), FUNC(sigmab98_state::sammymdl_coin_counter_w));
	map(0x32, 0x32).rw(FUNC(sigmab98_state::sammymdl_leds_r), FUNC(sigmab98_state::gocowboy_leds_w));
	map(0x90, 0x90).rw("oki", FUNC(okim9810_device::read), FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).nopw();
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                             Hae Hae Ka Ka Ka
***************************************************************************/

// rombank
WRITE8_MEMBER(sigmab98_state::haekaka_rombank_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	switch ( m_reg )
	{
		case 0x2b:
			m_rombank = data;
			switch (data)
			{
				case 0x10:  // ROM
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
				case 0x16:
				case 0x17:
				case 0x18:
				case 0x19:
				case 0x1a:
				case 0x1b:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:

				case 0x65:  // SPRITERAM
				case 0x67:  // PALETTERAM + VTABLE + VREGS
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}
READ8_MEMBER(sigmab98_state::haekaka_rombank_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x2b:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

// rambank
WRITE8_MEMBER(sigmab98_state::haekaka_rambank_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0x33:
			m_rambank = data;
			switch (data)
			{
				case 0x53:
					break;

				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", machine().describe_context(), data, m_reg2);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}
READ8_MEMBER(sigmab98_state::haekaka_rambank_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x33:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}

READ8_MEMBER(sigmab98_state::haekaka_vblank_r)
{
	return m_screen->vblank() ? 0 : 0x1c;
}

READ8_MEMBER(sigmab98_state::haekaka_b000_r)
{
	switch (m_rombank)
	{
		case 0x10:  // ROM
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			return memregion("maincpu")->base()[offset + 0xb400 + 0x1000 * (m_rombank-0x10)];

		case 0x65:  // SPRITERAM
			if (offset < 0x1000)
				return m_spriteram[offset];

		case 0x67:  // PALETTERAM + VTABLE + VREGS
			if (offset < 0x200)
				return m_paletteram[offset];
			else if ((offset >= 0x800) && (offset < 0x880))
			{
				return m_vtable[offset-0x800];
			}
			else if (offset >= (0xc000-0xb000) && offset <= (0xc021-0xb000))
			{
				if (offset == (0xc013-0xb000))
					return haekaka_vblank_r(space, 0);
				return vregs_r(space, offset-(0xc000-0xb000));
			}
			break;
	}

	logerror("%s: unknown read from %02x with rombank = %02x\n", machine().describe_context(), offset+0xb000, m_rombank);
	return 0x00;
}

WRITE8_MEMBER(sigmab98_state::haekaka_b000_w)
{
	switch (m_rombank)
	{
		case 0x65:  // SPRITERAM
			if (offset < 0x1000)
			{
				m_spriteram[offset] = data;
				return;
			}
			break;

		case 0x67:  // PALETTERAM + VTABLE + VREGS
			if (offset < 0x200)
			{
				m_palette->write8(space, offset, data);
				return;
			}
			else if ((offset >= 0x800) && (offset < 0x880))
			{
				m_vtable[offset-0x800] = data;
				return;
			}
			else if (offset >= (0xc000-0xb000) && offset <= (0xc021-0xb000))
			{
				vregs_w(space, offset-(0xc000-0xb000), data);
				return;
			}
			break;
	}

	logerror("%s: unknown write to %02x = %02x with rombank = %02x\n", machine().describe_context(), offset+0xb000, data, m_rombank);
}

WRITE8_MEMBER(sigmab98_state::haekaka_leds_w)
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

WRITE8_MEMBER(sigmab98_state::haekaka_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0,   data & 0x01 );   // medal out
//                                 data & 0x02 ?
//                                 data & 0x04 ?
//                                 data & 0x10 ?

	m_out[0] = data;
	show_3_outputs();
}

void sigmab98_state::haekaka_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xb000, 0xcfff).rw(FUNC(sigmab98_state::haekaka_b000_r), FUNC(sigmab98_state::haekaka_b000_w));
	map(0xd000, 0xefff).ram().share("nvram");
	map(0xfe00, 0xffff).ram();   // High speed internal RAM
}

void sigmab98_state::haekaka_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw(FUNC(sigmab98_state::haekaka_rombank_r), FUNC(sigmab98_state::haekaka_rombank_w));
	map(0x04, 0x05).rw(FUNC(sigmab98_state::haekaka_rambank_r), FUNC(sigmab98_state::haekaka_rambank_w));

	map(0x2c, 0x2c).rw(FUNC(sigmab98_state::sammymdl_eeprom_r), FUNC(sigmab98_state::sammymdl_eeprom_w));
	map(0x2e, 0x2e).r(FUNC(sigmab98_state::sammymdl_coin_hopper_r));
	map(0x30, 0x30).portr("BUTTON");
	map(0x31, 0x31).rw(FUNC(sigmab98_state::sammymdl_coin_counter_r), FUNC(sigmab98_state::haekaka_coin_counter_w));
	map(0x32, 0x32).rw(FUNC(sigmab98_state::sammymdl_leds_r), FUNC(sigmab98_state::haekaka_leds_w));
	map(0x90, 0x90).w("oki", FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).w(FUNC(sigmab98_state::sammymdl_hopper_w));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                              Itazura Monkey
***************************************************************************/

// rombank
WRITE8_MEMBER(sigmab98_state::itazuram_rombank_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	uint8_t *rom = memregion("maincpu")->base();
	switch ( m_reg )
	{
		case 0x0d:
			m_rombank = data;
			switch (data)
			{
				case 0x11:  // 3800 IS ROM
					membank("rombank0")->set_base(rom + 0x4c00);
					membank("rombank1")->set_base(rom + 0x5c00);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
					membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		case 0x4d:
			m_rombank = data;
			switch (data)
			{
//              case 0x0f:  // demo mode, after title

				case 0x14:  // 3800 IS ROM
					membank("rombank0")->set_base(rom + 0x8000);
					membank("rombank1")->set_base(rom + 0x9000);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
					membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		case 0x8d:
			m_rombank = data;
			switch (data)
			{
				case 0x0f:  // 3800 IS ROM
					membank("rombank0")->set_base(rom + 0x3400);
					membank("rombank1")->set_base(rom + 0x4400);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
					membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch
					break;

				case 0x12:  // 3800 IS ROM
					membank("rombank0")->set_base(rom + 0x6400);
					membank("rombank1")->set_base(rom + 0x7400);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
					membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch
					break;

				// used in test mode (code at 2cc4):
//              case 0x5c:  membank("rombank")->set_base(rom + 0x400 + 0x0000);    break;  // 3800 IS RAM! (8000 bytes)

				case 0x5e:  // 3800 IS RAM! (1404 bytes)
					membank("rombank0")->set_base(m_spriteram + 0x1000*1);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*1);
					membank("rombank1")->set_base(m_spriteram + 0x1000*2);
					membank("sprbank1")->set_base(m_spriteram + 0x1000*2);
					break;

				case 0x6c:  // 3800 IS RAM! (1000 bytes) - SPRITERAM
					membank("rombank0")->set_base(m_spriteram);
					membank("sprbank0")->set_base(m_spriteram);
//                  membank("sprbank1")->set_base(m_spriteram + 0x1000*4);    // scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		case 0xcd:
			m_rombank = data;
			switch (data)
			{
				case 0x14:  // 3800 IS ROM
					membank("rombank0")->set_base(rom + 0x8800);
					membank("rombank1")->set_base(rom + 0x9800);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
					membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch
					break;

				case 0x16:  // 3800 IS ROM
					membank("rombank0")->set_base(rom + 0xa800);
					membank("rombank1")->set_base(rom + 0xb800);
					membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
					membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}
READ8_MEMBER(sigmab98_state::itazuram_rombank_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		// FIXME: different registers
		case 0x0d:
		case 0x4d:
		case 0x8d:
		case 0xcd:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

// rambank
WRITE8_MEMBER(sigmab98_state::itazuram_rambank_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0x76:
			m_rambank = data;
			switch (data)
			{
				case 0x52:  membank("palbank")->set_base(m_nvram);          break;
				case 0x64:  membank("palbank")->set_base(&m_paletteram[0]); break;
				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", machine().describe_context(), data, m_reg2);
					return;
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}

READ8_MEMBER(sigmab98_state::itazuram_rambank_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x76:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}

WRITE8_MEMBER(sigmab98_state::itazuram_nvram_palette_w)
{
	if (m_rambank == 0x64)
	{
		m_palette->write8(space, offset, data);
	}
	else if (m_rambank == 0x52)
	{
		m_nvram[offset] = data;
	}
	else
	{
		logerror("%s: itazuram_nvram_palette_w offset = %03x with unknown bank = %02x\n", machine().describe_context(), offset, m_rambank);
	}
}

WRITE8_MEMBER(sigmab98_state::itazuram_palette_w)
{
	if (m_rombank == 0x6c)
	{
		if (offset < 0x200)
			m_palette->write8(space, offset, data);
	}
	else
	{
		logerror("%s: itazuram_palette_w offset = %03x with unknown bank = %02x\n", machine().describe_context(), offset, m_rombank);
	}
}

READ8_MEMBER(sigmab98_state::itazuram_palette_r)
{
	return m_paletteram[offset];
}

void sigmab98_state::itazuram_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x3800, 0x47ff).bankr("rombank0").bankw("sprbank0");
	map(0x4800, 0x57ff).bankr("rombank1").bankw("sprbank1");

	map(0x5800, 0x59ff).rw(FUNC(sigmab98_state::itazuram_palette_r), FUNC(sigmab98_state::itazuram_palette_w));
	map(0x6000, 0x607f).ram().share("vtable");

	map(0x6800, 0x6821).rw(FUNC(sigmab98_state::vregs_r), FUNC(sigmab98_state::vregs_w)).share("vregs");
	map(0x6811, 0x6811).nopw();  // IRQ Enable? Screen disable?
	map(0x6813, 0x6813).nopw();  // IRQ Ack?
	map(0xdc00, 0xfdff).bankr("palbank").w(FUNC(sigmab98_state::itazuram_nvram_palette_w)).share("nvram");    // nvram | paletteram

	map(0xfe00, 0xffff).ram();   // High speed internal RAM
}

void sigmab98_state::itazuram_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw(FUNC(sigmab98_state::itazuram_rombank_r), FUNC(sigmab98_state::itazuram_rombank_w));
	map(0x04, 0x05).rw(FUNC(sigmab98_state::itazuram_rambank_r), FUNC(sigmab98_state::itazuram_rambank_w));

	map(0x2c, 0x2c).rw(FUNC(sigmab98_state::sammymdl_eeprom_r), FUNC(sigmab98_state::sammymdl_eeprom_w));
	map(0x2e, 0x2e).r(FUNC(sigmab98_state::sammymdl_coin_hopper_r));
	map(0x30, 0x30).portr("BUTTON");
	map(0x31, 0x31).rw(FUNC(sigmab98_state::sammymdl_coin_counter_r), FUNC(sigmab98_state::sammymdl_coin_counter_w));
	map(0x32, 0x32).rw(FUNC(sigmab98_state::sammymdl_leds_r), FUNC(sigmab98_state::sammymdl_leds_w));
	map(0x90, 0x90).w("oki", FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).w(FUNC(sigmab98_state::sammymdl_hopper_w));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}

/***************************************************************************
                             Pye-nage Taikai
***************************************************************************/

void sigmab98_state::pyenaget_io(address_map &map)
{
	haekaka_io(map);
	map(0x31, 0x31).rw(FUNC(sigmab98_state::sammymdl_coin_counter_r), FUNC(sigmab98_state::sammymdl_coin_counter_w));
}

/***************************************************************************
                             Taihou de Doboon
***************************************************************************/

// rombank
WRITE8_MEMBER(sigmab98_state::tdoboon_rombank_w)
{
	if (offset == 0)
	{
		m_reg = data;
		return;
	}

	switch ( m_reg )
	{
		case 0x2f:
			m_rombank = data;
			switch (data)
			{
				case 0x10:  // ROM
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
				case 0x16:
				case 0x17:
				case 0x18:
				case 0x19:
				case 0x1a:
				case 0x1b:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:

				case 0x64:  // SPRITERAM
				case 0x66:  // PALETTERAM + VTABLE
				case 0x67:  // VREGS
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", machine().describe_context(), data, m_reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", machine().describe_context(), m_reg, data);
	}
}
READ8_MEMBER(sigmab98_state::tdoboon_rombank_r)
{
	if (offset == 0)
		return m_reg;

	switch ( m_reg )
	{
		case 0x2f:
			return m_rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", machine().describe_context(), m_reg);
			return 0x00;
	}
}

// rambank
WRITE8_MEMBER(sigmab98_state::tdoboon_rambank_w)
{
	if (offset == 0)
	{
		m_reg2 = data;
		return;
	}

	switch ( m_reg2 )
	{
		case 0x33:
			m_rambank = data;
			switch (data)
			{
				case 0x53:
					break;

				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", machine().describe_context(), data, m_reg2);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", machine().describe_context(), m_reg2, data);
	}
}
READ8_MEMBER(sigmab98_state::tdoboon_rambank_r)
{
	if (offset == 0)
		return m_reg2;

	switch ( m_reg2 )
	{
		case 0x33:
			return m_rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", machine().describe_context(), m_reg2);
			return 0x00;
	}
}

READ8_MEMBER(sigmab98_state::tdoboon_c000_r)
{
	switch (m_rombank)
	{
		case 0x10:  // ROM
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			return memregion("maincpu")->base()[offset + 0xc400 + 0x1000 * (m_rombank-0x10)];

		case 0x64:  // SPRITERAM
			if (offset < 0x1000)
				return m_spriteram[offset];
			break;

		case 0x66:  // PALETTERAM + VTABLE
			if (offset < 0x200)
				return m_paletteram[offset];
			else if ((offset >= 0x800) && (offset < 0x880))
			{
				return m_vtable[offset-0x800];
			}
			break;

		case 0x67:  // VREGS
			if (offset >= (0xc000-0xc000) && offset <= (0xc021-0xc000))
			{
				if (offset == (0xc013-0xc000))
					return haekaka_vblank_r(space, 0);
				return vregs_r(space, offset-(0xc000-0xc000));
			}
			break;
	}

	logerror("%s: unknown read from %02x with rombank = %02x\n", machine().describe_context(), offset+0xc000, m_rombank);
	return 0x00;
}

WRITE8_MEMBER(sigmab98_state::tdoboon_c000_w)
{
	switch (m_rombank)
	{
		case 0x64:  // SPRITERAM
			if (offset < 0x1000)
			{
				m_spriteram[offset] = data;
				return;
			}
			break;

		case 0x66:  // PALETTERAM + VTABLE
			if (offset < 0x200)
			{
				m_palette->write8(space, offset, data);
				return;
			}
			else if ((offset >= 0x800) && (offset < 0x880))
			{
				m_vtable[offset-0x800] = data;
				return;
			}
			break;

		case 0x67:  // VREGS
			if (offset >= (0xc000-0xc000) && offset <= (0xc021-0xc000))
			{
				vregs_w(space, offset-(0xc000-0xc000), data);
				return;
			}
			break;
	}

	logerror("%s: unknown write to %02x = %02x with rombank = %02x\n", machine().describe_context(), offset+0xc000, data, m_rombank);
}

void sigmab98_state::tdoboon_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).rw(FUNC(sigmab98_state::tdoboon_c000_r), FUNC(sigmab98_state::tdoboon_c000_w));
	map(0xd000, 0xefff).ram().share("nvram");
	map(0xfe00, 0xffff).ram();   // High speed internal RAM
}

void sigmab98_state::tdoboon_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw(FUNC(sigmab98_state::tdoboon_rombank_r), FUNC(sigmab98_state::tdoboon_rombank_w));
	map(0x04, 0x05).rw(FUNC(sigmab98_state::tdoboon_rambank_r), FUNC(sigmab98_state::tdoboon_rambank_w));

	map(0x2c, 0x2c).rw(FUNC(sigmab98_state::sammymdl_eeprom_r), FUNC(sigmab98_state::sammymdl_eeprom_w));
	map(0x2e, 0x2e).r(FUNC(sigmab98_state::sammymdl_coin_hopper_r));
	map(0x30, 0x30).portr("BUTTON");
	map(0x31, 0x31).rw(FUNC(sigmab98_state::sammymdl_coin_counter_r), FUNC(sigmab98_state::sammymdl_coin_counter_w));
	map(0x32, 0x32).rw(FUNC(sigmab98_state::sammymdl_leds_r), FUNC(sigmab98_state::sammymdl_leds_w));
	map(0x90, 0x90).w("oki", FUNC(okim9810_device::write));
	map(0x91, 0x91).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x92, 0x92).r("oki", FUNC(okim9810_device::read));
	map(0xb0, 0xb0).w(FUNC(sigmab98_state::sammymdl_hopper_w));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));  // 1
}


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout sigmab98_16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6, 4*9,4*8, 4*11,4*10, 4*13,4*12, 4*15,4*14 },
	{ STEP16(0,16*4) },
	16*16*4
};

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
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x4_layout, 0, 0x100/16  )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x8_layout, 0, 0x100/256 )
GFXDECODE_END

// Larger palette
static GFXDECODE_START( gfx_lufykzku )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x4_layout, 0, 0x1000/16 )
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

MACHINE_RESET_MEMBER(sigmab98_state,sigmab98)
{
	m_rombank = 0;
	membank("rombank")->set_entry(0);

	m_rambank = 0;
	membank("rambank")->set_entry(0);
}

INTERRUPT_GEN_MEMBER(sigmab98_state::sigmab98_vblank_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x5a);
}

MACHINE_CONFIG_START(sigmab98_state::sigmab98)
	MCFG_DEVICE_ADD("maincpu", Z80, 10000000)  // !! TAXAN KY-80, clock @X1? !!
	MCFG_DEVICE_PROGRAM_MAP(gegege_mem_map)
	MCFG_DEVICE_IO_MAP(gegege_io_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", sigmab98_state,  sigmab98_vblank_interrupt)

	MCFG_MACHINE_RESET_OVERRIDE(sigmab98_state, sigmab98)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	EEPROM_93C46_16BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)                    // ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // game reads vblank state
	MCFG_SCREEN_SIZE(0x140, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0,0x140-1, 0,0xf0-1)
	MCFG_SCREEN_UPDATE_DRIVER(sigmab98_state, screen_update_sigmab98)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sigmab98);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	BUFFERED_SPRITERAM8(config, m_buffered_spriteram);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("ymz", YMZ280B, 16934400)    // clock @X2?
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::dodghero)
	sigmab98(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( dodghero_mem_map )
	MCFG_DEVICE_IO_MAP( dodghero_io_map )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::gegege)
	sigmab98(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( gegege_mem_map )
	MCFG_DEVICE_IO_MAP( gegege_io_map )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::dashhero)
	sigmab98(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( gegege_mem_map )
	MCFG_DEVICE_IO_MAP( dashhero_io_map )

	config.device_remove("nvram"); // FIXME: does not survive between sessions otherwise
MACHINE_CONFIG_END


/***************************************************************************
                           Banpresto Medal Games
***************************************************************************/

MACHINE_RESET_MEMBER(lufykzku_state,lufykzku)
{
	m_rombank = 0;
	membank("romrambank")->set_entry(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(lufykzku_state::lufykzku_irq)
{
	int scanline = param;

	if (scanline == 240)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vblank_vector);
	else if (scanline == 128)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer0_vector);
	else if ((scanline % 8) == 0)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer1_vector); // this needs to be called often or the state of the door is not read at boot (at least 5 times before bb9 is called)
}

MACHINE_CONFIG_START(lufykzku_state::lufykzku)
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(20'000'000) / 2)  // !! TAXAN KY-80, clock @X1? !!
	MCFG_DEVICE_PROGRAM_MAP(lufykzku_mem_map)
	MCFG_DEVICE_IO_MAP(lufykzku_io_map)
	TIMER(config, "scantimer").configure_scanline(FUNC(lufykzku_state::lufykzku_irq), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(lufykzku_state, lufykzku)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);   // battery backed RAM
	// No EEPROM

	MCFG_DEVICE_ADD("watchdog_mb3773", MB3773, 0)
	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	// 2 x 8-bit parallel/serial converters
	TTL165(config, m_dsw_shifter[0]);
	m_dsw_shifter[0]->data_callback().set_ioport("DSW2");
	m_dsw_shifter[0]->qh_callback().set(m_dsw_shifter[1], FUNC(ttl165_device::serial_w));

	TTL165(config, m_dsw_shifter[1]);
	m_dsw_shifter[1]->data_callback().set_ioport("DSW1");
	m_dsw_shifter[1]->qh_callback().set(FUNC(lufykzku_state::dsw_w));

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)                    // ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // game reads vblank state
	MCFG_SCREEN_SIZE(0x140, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0,0x140-1, 0,0xf0-1)
	MCFG_SCREEN_UPDATE_DRIVER(sigmab98_state, screen_update_sigmab98)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lufykzku);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	//BUFFERED_SPRITERAM8(config, m_buffered_spriteram); // same as sammymdl?

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MCFG_DEVICE_ADD("oki", OKIM9810, XTAL(4'096'000))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_CONFIG_END


/***************************************************************************
                             Sammy Medal Games
***************************************************************************/

MACHINE_RESET_MEMBER(sigmab98_state,sammymdl)
{
	m_maincpu->set_state_int(Z80_PC, 0x400);  // code starts at 400 ??? (000 = cart header)
}

TIMER_DEVICE_CALLBACK_MEMBER(sigmab98_state::sammymdl_irq)
{
	int scanline = param;

	if (scanline == 240)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_vblank_vector);

	if (scanline == 128)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer0_vector);

	if (scanline == 32)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_timer1_vector);
}

MACHINE_CONFIG_START(sigmab98_state::sammymdl)
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(20'000'000) / 2)    // !! KL5C80A120FP @ 10MHz? (actually 4 times faster than Z80) !!
	MCFG_DEVICE_PROGRAM_MAP( animalc_map )
	MCFG_DEVICE_IO_MAP( animalc_io )
	TIMER(config, "scantimer").configure_scanline(FUNC(sigmab98_state::sammymdl_irq), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(sigmab98_state, sammymdl )

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);   // battery backed RAM
	EEPROM_93C46_8BIT(config, "eeprom");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)                    // ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // game reads vblank state
	MCFG_SCREEN_SIZE(0x140, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xf0-1)
	MCFG_SCREEN_UPDATE_DRIVER(sigmab98_state, screen_update_sigmab98)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, sigmab98_state, screen_vblank_sammymdl))
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sigmab98);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000 + 1);
	m_palette->set_endianness(ENDIANNESS_BIG);

	//BUFFERED_SPRITERAM8(config, m_buffered_spriteram); // not on sammymdl?

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("oki", OKIM9810, XTAL(4'096'000))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::animalc)
	sammymdl(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( animalc_map )
	MCFG_DEVICE_IO_MAP( animalc_io )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::gocowboy)
	sammymdl(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( gocowboy_map )
	MCFG_DEVICE_IO_MAP( gocowboy_io )

	config.device_remove("hopper");
	TICKET_DISPENSER(config, m_hopper_small, attotime::from_msec(1000), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );
	TICKET_DISPENSER(config, m_hopper_large, attotime::from_msec(1000), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW );
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::haekaka)
	sammymdl(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( haekaka_map )
	MCFG_DEVICE_IO_MAP( haekaka_io )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::itazuram)
	sammymdl(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( itazuram_map )
	MCFG_DEVICE_IO_MAP( itazuram_io )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::pyenaget)
	sammymdl(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( haekaka_map )
	MCFG_DEVICE_IO_MAP( pyenaget_io )
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sigmab98_state::tdoboon)
	sammymdl(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP( tdoboon_map )
	MCFG_DEVICE_IO_MAP( tdoboon_io )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0,0x140-1, 0+4,0xf0+4-1)
MACHINE_CONFIG_END


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

void sigmab98_state::init_dodghero()
{
	// ROM banks
	uint8_t *rom = memregion("maincpu")->base();
	membank("rombank")->configure_entries(0, 0x18*2, rom + 0x8000, 0x800);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 2);
	membank("rambank")->configure_entries(0, 2, bankedram, 0x800);
}

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

	// ROM banks
	membank("rombank")->configure_entries(0, 0x18, rom + 0x8000, 0x1000);
	membank("rombank")->set_entry(0);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 2);
	membank("rambank")->configure_entries(0, 2, bankedram, 0x800);
	membank("rambank")->set_entry(0);
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

	// ROM banks
	membank("rombank")->configure_entries(0, 0x18, rom + 0x8000, 0x1000);
	membank("rombank")->set_entry(0);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 2);
	membank("rambank")->configure_entries(0, 2, bankedram, 0x800);
	membank("rambank")->set_entry(0);
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

	// ROM banks
	membank("rombank")->configure_entries(0, 0x18, rom + 0x8000, 0x1000);
	membank("rombank")->set_entry(0);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 2);
	membank("rambank")->configure_entries(0, 2, bankedram, 0x800);
	membank("rambank")->set_entry(0);
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

	// ROM banks
	membank("rombank")->configure_entries(0, 0x18, rom + 0x8000, 0x1000);
	membank("rombank")->set_entry(0);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 2);
	membank("rambank")->configure_entries(0, 2, bankedram, 0x800);
	membank("rambank")->set_entry(0);
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

	// ROM banks
	membank("rombank")->configure_entries(0, 0x18, rom + 0x8000, 0x1000);
	membank("rombank")->set_entry(0);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 2);
	membank("rambank")->configure_entries(0, 2, bankedram, 0x800);
	membank("rambank")->set_entry(0);
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

	// ROM banks
	membank("rombank")->configure_entries(0, 0x18, rom + 0x8000, 0x1000);
	membank("rombank")->set_entry(0);

	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x800 * 4);
	membank("rambank")->configure_entries(0, 4, bankedram, 0x800);
	membank("rambank")->set_entry(0);
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
	m_nvram.allocate(0x4000);
	memset(m_nvram, 0, 0x4000);
	m_nvramdev->set_base(m_nvram, 0x4000);

	// ROM/RAM banks
	uint8_t *rom = memregion("maincpu")->base();

	membank("romrambank")->configure_entry(0, rom + 0x8000);
	membank("romrambank")->configure_entry(1, m_nvram);
	membank("romrambank")->set_entry(0);

	m_vblank_vector = 0xfa; // nop
	m_timer0_vector = 0xfc; // write coin counters/lockout, drive hopper
	m_timer1_vector = 0xfe; // read inputs and hopper sensor, handle coin in

	m_gfxdecode->gfx(1)->set_granularity(16);
}


/***************************************************************************

  Sammy Medal Games

  PCB:

    Sammy AM3AHF-01 SC MAIN PCB VER2 (Etched)
    MAIN PCB VER2 VM12-6001-0 (Sticker)

  CPU:

    KAWASAKI KL5C80A120FP (@U1) - Z80 Compatible High Speed Microcontroller
    XTAL 20 MHz  (@X1)
    MX29F040TC-12 VM1211L01 (@U2) - 4M-bit [512kx8] CMOS Equal Sector Flash Memory
    BSI BS62LV256SC-70      (@U4) - Very Low Power/Voltage CMOS SRAM 32K X 8 bit

  Video:

    TAXAN KY-3211 ? (@U17)
    M548262-60 (@U18) - 262144-Word x 8-Bit Multiport DRAM
    XTAL 27 MHz (@X3)

  Sound:

    OKI M9810B (@U11)
    XTAL 4.09 MHz (@X2)
    Trimmer (@VR1)
    Toshiba TA7252AP (@U16) - 5.9W Audio Power Amplifier

  Other:

    Xilinx XC9536 VM1212F01 (@U5) - In-System Programmable CPLD
    MX29F0??C (@U3) - Empty 32 Pin ROM Socket
    M5295A (@U8) - Watchdog Timer (Near CUT-DEBUG MODE Jumper)
    M93C46MN6T (@U11?) - Serial EEPROM
    Cell Battery (@BAT)
    25 Pin Edge Connector
    56 Pin Cartridge Connector
    6 Pin Connector

***************************************************************************/

#define SAMMYMDL_BIOS                                                                                               \
	ROM_REGION( 0x80000, "mainbios", 0 )                                                                            \
	ROM_SYSTEM_BIOS( 0, "v5", "IPL Ver. 5.0" )                                                                      \
	ROM_LOAD( "vm1211l01.u2", 0x000000, 0x080000, CRC(c3c74dc5) SHA1(07352e6dba7514214e778ba39e1ca773e4698858) )

ROM_START( sammymdl )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )

	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_COPY( "mainbios", 0x000000, 0x0000, 0x40000 )

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

void sigmab98_state::init_animalc()
{
	// RAM banks
	uint8_t *bankedram = auto_alloc_array(machine(), uint8_t, 0x1000 * 5);
	membank("rambank")->configure_entry(0, m_nvram);
	membank("rambank")->configure_entries(1, 4, bankedram, 0x1000);
	membank("rambank")->set_entry(0);

	m_spriteram.allocate(0x1000 * 5);
	memset(m_spriteram, 0, 0x1000 * 5);
	membank("sprbank")->configure_entries(0, 5, m_spriteram, 0x1000);
	membank("sprbank")->set_entry(0);

	m_vblank_vector = 0x00; // increment counter
	m_timer0_vector = 0x1c; // read hopper state
	m_timer1_vector = 0x1e; // drive hopper motor
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

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "mainbios", 0x00fc00, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "em701l01.u016", 0x000000, 0x200000, CRC(c1f07320) SHA1(734717140e66ddcf0bded1489156c51cdaf1b50c) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "st93c46.u6", 0x00, 0x80, CRC(1af2d376) SHA1(afbe953f1a9ff0152fe1092a83482695dbe5e75d) )

	ROM_REGION( 0x5cde, "pld", 0 )
	ROM_LOAD( "vm1212f01.u5.jed", 0x0000, 0x5cde, CRC(b86a1825) SHA1(cc2e633fb8a24cfc93291a778b0964089f6b8ac7) )
ROM_END

void sigmab98_state::init_gocowboy()
{
	// RAM banks
	m_paletteram.resize(0x200);
	memset(&m_paletteram[0], 0, 0x200);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	m_nvram.allocate(0x8000);
	memset(m_nvram, 0, 0x8000);
	m_nvramdev->set_base(m_nvram, 0x8000);

	m_spriteram.allocate(0x1000);
	memset(m_spriteram, 0, 0x1000);

	m_vregs.allocate(0x22);
	memset(m_vregs, 0, 0x22);

	m_vtable.allocate(0x80);
	memset(m_vtable, 0, 0x80);

	m_rombank = 0x6b;
	m_rambank = 0x52;

	m_vblank_vector = 0x00;
	m_timer0_vector = 0x02;
	m_timer1_vector = 0x16;
}

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

void sigmab98_state::init_itazuram()
{
	// ROM banks
	uint8_t *rom = memregion("maincpu")->base();
	membank("rombank0")->set_base(rom + 0x3400);
	membank("rombank1")->set_base(rom + 0x4400);
	m_rombank = 0x0f;

	// RAM banks
	m_paletteram.resize(0x3000);
	memset(&m_paletteram[0], 0, 0x3000);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);
	membank("palbank")->set_base(&m_paletteram[0]);
	m_rambank = 0x64;

	m_spriteram.allocate(0x1000 * 5);
	memset(m_spriteram, 0, 0x1000 * 5);
	membank("sprbank0")->set_base(m_spriteram + 0x1000*4);  // scratch
	membank("sprbank1")->set_base(m_spriteram + 0x1000*4);  // scratch

	m_vblank_vector = 0x00;
	m_timer0_vector = 0x02;
	m_timer1_vector = 0x16;
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

void sigmab98_state::init_haekaka()
{
	// RAM banks
	m_paletteram.resize(0x200);
	memset(&m_paletteram[0], 0, 0x200);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	m_spriteram.allocate(0x1000);
	memset(m_spriteram, 0, 0x1000);

	m_vregs.allocate(0x22);
	memset(m_vregs, 0, 0x22);

	m_vtable.allocate(0x80);
	memset(m_vtable, 0, 0x80);

	m_rombank = 0x65;
	m_rambank = 0x53;

	m_vblank_vector = 0x04;
	m_timer0_vector = 0x1a;
	m_timer1_vector = 0x1c;
}

/***************************************************************************

    Game Drivers

***************************************************************************/

// Sigma Medal Games
GAME( 1997, dodghero, 0,        dodghero, sigma_1b, sigmab98_state, init_dodghero, ROT0, "Sigma",             "Minna Atsumare! Dodge Hero",           0 )
GAME( 1997, sushimar, 0,        dodghero, sigma_3b, sigmab98_state, init_dodghero, ROT0, "Sigma",             "Itazura Daisuki! Sushimaru Kun",       0 )
GAME( 1997, gegege,   0,        gegege,   sigma_1b, sigmab98_state, init_gegege,   ROT0, "Sigma / Banpresto", "GeGeGe no Kitarou Youkai Slot",        0 )
GAME( 1997, b3rinsya, 0,        gegege,   sigma_5b, sigmab98_state, init_b3rinsya, ROT0, "Sigma",             "Burning Sanrinsya - Burning Tricycle", 0 ) // 1997 in the rom
GAME( 1997, pepsiman, 0,        gegege,   sigma_3b, sigmab98_state, init_pepsiman, ROT0, "Sigma",             "PEPSI Man",                            0 )
GAME( 1998, tbeastw2, 0,        gegege,   sigma_3b, sigmab98_state, init_tbeastw2, ROT0, "Sigma / Transformer Production Company / Takara", "Transformers Beast Wars II", 0 ) // 1997 in the rom
GAME( 1997, ucytokyu, 0,        gegege,   sigma_js, sigmab98_state, init_ucytokyu, ROT0, "Sigma",             "Uchuu Tokkyuu Medalian",               0 ) // Banpresto + others in the ROM
GAME( 2000, dashhero, 0,        dashhero, sigma_1b, sigmab98_state, init_dashhero, ROT0, "Sigma",             "Minna Ganbare! Dash Hero",             MACHINE_NOT_WORKING ) // 1999 in the rom
// Banpresto Medal Games
GAME( 2001, lufykzku, 0,        lufykzku, lufykzku, lufykzku_state, init_lufykzku, ROT0, "Banpresto / Eiichiro Oda / Shueisha - Fuji TV - Toho Animation", "Otakara Itadaki Luffy Kaizoku-Dan! (Japan, v1.02)", 0 )
// Sammy Medal Games:
GAME( 2000, sammymdl, 0,        sammymdl, sammymdl, sigmab98_state, init_animalc,  ROT0, "Sammy",             "Sammy Medal Game System Bios",         MACHINE_IS_BIOS_ROOT )
GAME( 2000, animalc,  sammymdl, animalc,  sammymdl, sigmab98_state, init_animalc,  ROT0, "Sammy",             "Animal Catch",                         0 )
GAME( 2000, itazuram, sammymdl, itazuram, sammymdl, sigmab98_state, init_itazuram, ROT0, "Sammy",             "Itazura Monkey",                       0 )
GAME( 2000, pyenaget, sammymdl, pyenaget, sammymdl, sigmab98_state, init_haekaka,  ROT0, "Sammy",             "Pye-nage Taikai",                      0 )
GAME( 2000, tdoboon,  sammymdl, tdoboon,  haekaka,  sigmab98_state, init_haekaka,  ROT0, "Sammy",             "Taihou de Doboon",                     0 )
GAME( 2001, haekaka,  sammymdl, haekaka,  haekaka,  sigmab98_state, init_haekaka,  ROT0, "Sammy",             "Hae Hae Ka Ka Ka",                     0 )
GAME( 2003, gocowboy, sammymdl, gocowboy, gocowboy, sigmab98_state, init_gocowboy, ROT0, "Sammy",             "Go Go Cowboy (English, prize)",        0 )
