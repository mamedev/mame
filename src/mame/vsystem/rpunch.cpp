// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Rabbit Punch / Rabio Lepus

    driver by Aaron Giles

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    000000-03FFFF   R     xxxxxxxx xxxxxxxx   Program ROM
    040000-04FFFF   R/W   xxxxxxxx xxxxxxxx   Bitmap RAM (512x256 pixels)
                    R/W   xxxx---- --------      (leftmost pixel)
                    R/W   ----xxxx --------      (2nd pixel)
                    R/W   -------- xxxx----      (3rd pixel)
                    R/W   -------- ----xxxx      (rightmost pixel)
    060000-060FFF   R/W   xxxxxxxx xxxxxxxx   Sprite RAM (512 entries x 4 words)
                    R/W   -------x xxxxxxxx      (0: horizontal position)
                    R/W   xxx----- --------      (1: color index)
                    R/W   ---x---- --------      (1: horizontal flip)
                    R/W   -----xxx xxxxxxxx      (1: image number)
                    R/W   -------x xxxxxxxx      (2: Y position)
                    R/W   -------- --------      (3: not used)
    080000-081FFF   R/W   xxxxxxxx xxxxxxxx   Background 1 RAM (64x64 tiles)
                    R/W   xxx----- --------      (color index)
                    R/W   ---xxxxx xxxxxxxx      (image number)
    082000-083FFF   R/W   xxxxxxxx xxxxxxxx   Background 2 RAM (64x64 tiles)
                    R/W   xxx----- --------      (color index)
                    R/W   ---xxxxx xxxxxxxx      (image number)
    0A0000-0A01FF   R/W   -xxxxxxx xxxxxxxx   Background 1 palette RAM (256 entries)
                    R/W   -xxxxx-- --------      (red component)
                    R/W   ------xx xxx-----      (green component)
                    R/W   -------- ---xxxxx      (blue component)
    0A0200-0A03FF   R/W   -xxxxxxx xxxxxxxx   Background 2 palette RAM (256 entries)
    0A0400-0A05FF   R/W   -xxxxxxx xxxxxxxx   Bitmap palette RAM (256 entries)
    0A0600-0A07FF   R/W   -xxxxxxx xxxxxxxx   Sprite palette RAM (256 entries)
    0C0000            W   -------x xxxxxxxx   Background 1 vertical scroll
    0C0002            W   -------x xxxxxxxx   Background 1 horizontal scroll
    0C0004            W   -------x xxxxxxxx   Background 2 vertical scroll
    0C0006            W   -------x xxxxxxxx   Background 2 horizontal scroll
    0C0008            W   -------- ????????   Video controller data (CRTC)
    0C000C            W   ---xxx-- -xxxxxxx   Video flags
                      W   ---x---- --------      (flip screen)
                      W   ----x--- --------      (background 2 image bank)
                      W   -----x-- --------      (background 1 image bank)
                      W   -------- -x------      (sprite palette bank)
                      W   -------- --x-----      (background 2 palette bank)
                      W   -------- ---x----      (background 1 palette bank)
                      W   -------- ----xxxx      (bitmap palette bank)
    0C000E            W   -------- xxxxxxxx   Sound communications
    0C0010            W   -------- --xxxxxx   Number of active sprite entries
    0C0012            W   -------- --xxxxxx   Split point of sprite priority
    0C0018          R     -xxxx-xx --xxxxxx   Player 1 input port
                    R     -x------ --------      (2 player start)
                    R     --x----- --------      (1 player start)
                    R     ---x---- --------      (coin 1)
                    R     ----x--- --------      (coin 2)
                    R     ------x- --------      (test switch)
                    R     -------x --------      (service coin)
                    R     -------- --x-----      (punch button)
                    R     -------- ---x----      (missile button)
                    R     -------- ----xxxx      (joystick right/left/down/up)
    0C001A          R     -xxxx-xx --xxxxxx   Player 2 input port
    0C001C          R     xxxxxxxx xxxxxxxx   DIP switches
                    R     x------- --------      (flip screen)
                    R     -x------ --------      (continues allowed)
                    R     --x----- --------      (demo sounds)
                    R     ---x---- --------      (extended play)
                    R     ----x--- --------      (laser control)
                    R     -----x-- --------      (number of lives)
                    R     ------xx --------      (difficulty)
                    R     -------- xxxx----      (coinage 2)
                    R     -------- ----xxxx      (coinage 1)
    0C001E          R     -------- -------x   Sound busy flag
    0C0028            W   -------- ????????   Video controller register select (CRTC)
    0FC000-0FFFFF   R/W   xxxxxxxx xxxxxxxx   Program RAM
    ========================================================================
    Interrupts:
        IRQ1 = VBLANK
    ========================================================================


    ========================================================================
    SOUND CPU
    ========================================================================
    0000-EFFF   R     xxxxxxxx   Program ROM
    F000-F001   R/W   xxxxxxxx   YM2151 communications
    F200        R     xxxxxxxx   Sound command input
    F400          W   x------x   UPD7759 control
                  W   x-------      (/RESET line)
                  W   -------x      (ROM bank select)
    F600          W   xxxxxxxx   UPD7759 data/trigger
    F800-FFFF   R/W   xxxxxxxx   Program RAM
    ========================================================================
    Interrupts:
        IRQ = YM2151 IRQ or'ed with the sound command latch flag
    ========================================================================

***************************************************************************/


#include "emu.h"

// mame
#include "vsystem_gga.h"

// devices
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/upd7759.h"
#include "sound/ymopm.h"

// emu
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"
#include "speaker.h"


namespace {


class rpunch_state : public driver_device
{
public:
	rpunch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gga(*this, "gga"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_hi_bits(*this, "SERVICE")
	{ }

	void rpunch(machine_config &config);
	void svolleybl(machine_config &config);

	ioport_value hi_bits_r() { return m_hi_bits->read(); }

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<vsystem_gga_device> m_gga;

	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u16> m_spriteram;

	required_ioport m_hi_bits;

	tilemap_t *m_background[2]{};

	void set_sprite_palette(int val) { m_sprite_palette = val; }
	void set_sprite_xoffs(int val) { m_sprite_xoffs = val; }

	virtual void machine_start() override ATTR_COLD;

	void svolley_map(address_map &map) ATTR_COLD;
	void rpunch_map(address_map &map) ATTR_COLD;
	void svolleybl_main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void svolleybl_sound_map(address_map &map) ATTR_COLD;

private:
	// hardware configuration
	int m_sprite_palette = 0;
	int m_sprite_xoffs = 0;

	u8 m_upd_rom_bank = 0;
	u16 m_videoflags = 0;
	u8 m_sprite_pri = 0;
	u8 m_sprite_num = 0;
	std::unique_ptr<bitmap_ind16> m_pixmap;
	emu_timer *m_crtc_timer = nullptr;

	u16 sound_busy_r();
	u8 pixmap_r(offs_t offset);
	void pixmap_w(offs_t offset, u8 data);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void videoreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void scrollreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void gga_w(offs_t offset, u8 data);
	void gga_data_w(offs_t offset, u8 data);
	void sprite_ctrl_w(offs_t offset, u8 data);
	void upd_control_w(u8 data);
	void upd_data_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(crtc_interrupt_gen);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
};


class svolley_state : public rpunch_state
{
public:
	svolley_state(const machine_config &mconfig, device_type type, const char *tag) :
		rpunch_state(mconfig, type, tag)
	{
	}

	void svolley(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
};


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(rpunch_state::get_bg0_tile_info)
{
	const u16 data = m_videoram[tile_index];
	int code;
	if (m_videoflags & 0x0400)  code = (data & 0x0fff) | 0x2000;
	else                        code = (data & 0x1fff);

	tileinfo.set(0,
			code,
			((m_videoflags & 0x0010) >> 1) | ((data >> 13) & 7),
			0);
}

TILE_GET_INFO_MEMBER(rpunch_state::get_bg1_tile_info)
{
	const u16 data = m_videoram[0x1000 | tile_index];
	int code;
	if (m_videoflags & 0x0800)  code = (data & 0x0fff) | 0x2000;
	else                        code = (data & 0x1fff);

	tileinfo.set(1,
			code,
			((m_videoflags & 0x0020) >> 2) | ((data >> 13) & 7),
			0);
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

TIMER_CALLBACK_MEMBER(rpunch_state::crtc_interrupt_gen)
{
	m_maincpu->set_input_line(1, HOLD_LINE);
	if (param != 0)
		m_crtc_timer->adjust(m_screen->frame_period() / param, 0, m_screen->frame_period() / param);
}


/*************************************
 *
 *  Write handlers
 *
 *************************************/

u8 rpunch_state::pixmap_r(offs_t offset)
{
	const int sy = offset >> 8;
	const int sx = (offset & 0xff) << 1;

	return ((m_pixmap->pix(sy & 0xff, sx & ~1) & 0xf) << 4) | (m_pixmap->pix(sy & 0xff, sx |  1) & 0xf);
}

void rpunch_state::pixmap_w(offs_t offset, u8 data)
{
	const int sy = offset >> 8;
	const int sx = (offset & 0xff) << 1;

	m_pixmap->pix(sy & 0xff, sx & ~1) = ((data & 0xf0) >> 4);
	m_pixmap->pix(sy & 0xff, sx |  1) = (data & 0x0f);
}

void rpunch_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int tmap = offset >> 12;
	const int tile_index = offset & 0xfff;
	COMBINE_DATA(&m_videoram[offset]);
	m_background[tmap]->mark_tile_dirty(tile_index);
}


void rpunch_state::videoreg_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int oldword = m_videoflags;
	COMBINE_DATA(&m_videoflags);

	if (m_videoflags != oldword)
	{
		// invalidate tilemaps
		if ((oldword ^ m_videoflags) & 0x0410)
			m_background[0]->mark_all_dirty();
		if ((oldword ^ m_videoflags) & 0x0820)
			m_background[1]->mark_all_dirty();
	}
}


void rpunch_state::scrollreg_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
		switch (offset)
		{
			case 0:
				m_background[0]->set_scrolly(0, data & 0x1ff);
				break;

			case 1:
				m_background[0]->set_scrollx(0, data & 0x1ff);
				break;

			case 2:
				m_background[1]->set_scrolly(0, data & 0x1ff);
				break;

			case 3:
				m_background[1]->set_scrollx(0, data & 0x1ff);
				break;
		}
}


void rpunch_state::gga_w(offs_t offset, u8 data)
{
	m_gga->write(offset >> 5, data & 0xff);
}


void rpunch_state::gga_data_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		// only register we know about....
		case 0x0b:
			m_crtc_timer->adjust(m_screen->time_until_vblank_start(), (data == 0xc0) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", offset, data);
			break;
	}
}


void rpunch_state::sprite_ctrl_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_sprite_num = data & 0x3f;
		logerror("Number of sprites = %02X\n", data & 0x3f);
	}
	else
	{
		m_sprite_pri = data & 0x3f;
		logerror("Sprite priority point = %02X\n", data & 0x3f);
	}
}


/*************************************
 *
 *  Sprite routines
 *
 *************************************/

void rpunch_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw the sprites
	int offs = m_sprite_num;
	while (offs > 0)
	{
		offs--;
		const u32 ram = offs << 2;
		u32 pri = 0; // above everything except direct mapped bitmap
		// this seems like the most plausible explanation
		if (offs < m_sprite_pri)
			pri |= GFX_PMASK_2; // behind foreground

		const u16 data1 = m_spriteram[ram + 1];
		const u32 code = data1 & 0x7ff;

		const u16 data0 = m_spriteram[ram + 0];
		const u16 data2 = m_spriteram[ram + 2];
		int x = (data2 & 0x1ff) + 8;
		int y = 513 - (data0 & 0x1ff);
		const bool xflip = data1 & 0x1000;
		const bool yflip = data1 & 0x0800;
		const u32 color = ((data1 >> 13) & 7) | ((m_videoflags & 0x0040) >> 3);

		if (x > cliprect.max_x) x -= 512;
		if (y > cliprect.max_y) y -= 512;

		m_gfxdecode->gfx(2)->prio_transpen(bitmap,cliprect,
				code, color + (m_sprite_palette / 16), xflip, yflip, x + m_sprite_xoffs, y,
				m_screen->priority(), pri, 15);
	}
}


/*************************************
 *
 *  Bitmap routines
 *
 *************************************/

void rpunch_state::draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u32 colourbase = 512 + ((m_videoflags & 0x000f) << 4);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const src = &m_pixmap->pix(y & 0xff);
		u16 *const dst = &bitmap.pix(y);
		for(int x = cliprect.min_x / 4; x <= cliprect.max_x; x++)
		{
			const u16 pix = src[(x + 4) & 0x1ff];
			if ((pix & 0xf) != 0xf)
				dst[x] = colourbase + pix;
		}
	}
}


/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

u32 rpunch_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_background[0]->draw(screen, bitmap, cliprect, 0,1);
	m_background[1]->draw(screen, bitmap, cliprect, 0,2);
	draw_sprites(bitmap, cliprect);
	draw_bitmap(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void rpunch_state::machine_start()
{
	m_videoflags = 0;

	// allocate tilemaps for the backgrounds
	m_background[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rpunch_state::get_bg0_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);
	m_background[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rpunch_state::get_bg1_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);

	// configure the tilemaps
	m_background[1]->set_transparent_pen(15);

	m_pixmap = std::make_unique<bitmap_ind16>(512, 256);

	const rectangle pixmap_rect(0,511,0,255);
	m_pixmap->fill(0xf, pixmap_rect);

	// reset the timer
	m_crtc_timer = timer_alloc(FUNC(rpunch_state::crtc_interrupt_gen), this);

	save_item(NAME(m_upd_rom_bank));
	save_item(NAME(m_videoflags));
	save_item(NAME(m_sprite_pri));
	save_item(NAME(m_sprite_num));
	save_item(NAME(*m_pixmap));
}

void svolley_state::machine_start()
{
	rpunch_state::machine_start();

	m_background[0]->set_scrolldx(8, 0); // aligns middle net sprite with bg as shown in reference
}


/*************************************
 *
 *  Sound I/O
 *
 *************************************/

u16 rpunch_state::sound_busy_r()
{
	return m_soundlatch->pending_r();
}


/*************************************
 *
 *  UPD7759 controller
 *
 *************************************/

void rpunch_state::upd_control_w(u8 data)
{
	m_upd7759->set_rom_bank(BIT(data, 0));
	m_upd7759->reset_w(BIT(data, 7));
}


void rpunch_state::upd_data_w(u8 data)
{
	m_upd7759->port_w(data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void rpunch_state::svolley_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x000000, 0x03ffff).rom();
	map(0x060000, 0x060fff).ram().share(m_spriteram);
	map(0x080000, 0x083fff).ram().w(FUNC(rpunch_state::videoram_w)).share(m_videoram);
	map(0x0a0000, 0x0a07ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0c0000, 0x0c0007).w(FUNC(rpunch_state::scrollreg_w));
	map(0x0c0009, 0x0c0009).select(0x20).w(FUNC(rpunch_state::gga_w));
	map(0x0c000c, 0x0c000d).w(FUNC(rpunch_state::videoreg_w));
	map(0x0c000f, 0x0c000f).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0x0c0010, 0x0c0013).w(FUNC(rpunch_state::sprite_ctrl_w)).umask16(0x00ff);
	map(0x0c0018, 0x0c0019).portr("P1");
	map(0x0c001a, 0x0c001b).portr("P2");
	map(0x0c001c, 0x0c001d).portr("DSW");
	map(0x0c001e, 0x0c001f).r(FUNC(rpunch_state::sound_busy_r));
	map(0x0fc000, 0x0fffff).ram();
}

void rpunch_state::rpunch_map(address_map &map)
{
	svolley_map(map);
	map(0x040000, 0x04ffff).rw(FUNC(rpunch_state::pixmap_r), FUNC(rpunch_state::pixmap_w)); // mirrored?
}

void rpunch_state::svolleybl_main_map(address_map &map)
{
	svolley_map(map);

	// TODO: sound latch hook up is incomplete
	map(0x090000, 0x090fff).ram(); // ?
	map(0x0c000e, 0x0c000f).unmapw();
	map(0x0c001e, 0x0c001f).unmapr();
	map(0x0b0001, 0x0b0001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void rpunch_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf200, 0xf200).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf400, 0xf400).w(FUNC(rpunch_state::upd_control_w));
	map(0xf600, 0xf600).w(FUNC(rpunch_state::upd_data_w));
	map(0xf800, 0xffff).ram();
}

void rpunch_state::svolleybl_sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe800, 0xe800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	//map(0xf000, 0xf000) // OKI M5205?
	//map(0xf400, 0xf400) // OKI M5205?
	map(0xf800, 0xffff).ram();
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( rpunch )
	PORT_START("P1")    // c0018 lower 8 bits
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(rpunch_state, hi_bits_r)

	PORT_START("P2")    // c001a lower 8 bits
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(rpunch_state, hi_bits_r)

	PORT_START("SERVICE")   // c0018/c001a upper 8 bits
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) // Hold F2 at bootup
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )  // Freeze game
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")   // c001c DIP switches
	PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(      0x000d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, "6 Coins/4 Credits")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00f0, 0x0000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(      0x00d0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0050, "6 Coins/4 Credits")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )


	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPNAME( 0x0800, 0x0000, "Laser" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(      0x0000, "Manual" )
	PORT_DIPSETTING(      0x0800, "Semi-Automatic" )
	PORT_DIPNAME( 0x1000, 0x0000, "Extended Play" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(      0x0000, "500000 points" )
	PORT_DIPSETTING(      0x1000, DEF_STR( None ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Continues ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rabiolep )
	PORT_INCLUDE( rpunch )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPNAME( 0x0800, 0x0000, "Laser" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(      0x0000, "Semi-Automatic" )
	PORT_DIPSETTING(      0x0800, "Manual" )
	PORT_DIPNAME( 0x1000, 0x0000, "Extended Play" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(      0x0000, "500000 points" )
	PORT_DIPSETTING(      0x1000, "300000 points" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



static INPUT_PORTS_START( svolley )
	PORT_INCLUDE( rpunch )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(      0x0100, "2 min/1 min" )
	PORT_DIPSETTING(      0x0000, "3 min/1.5 min" )
	PORT_DIPNAME( 0x0600, 0x0000, "2P Starting Score" ) PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(      0x0600, "0-0" )
	PORT_DIPSETTING(      0x0400, "5-5" )
	PORT_DIPSETTING(      0x0000, "7-7" )
	PORT_DIPSETTING(      0x0200, "9-9" )
	PORT_DIPNAME( 0x1800, 0x0000, "1P Starting Score" ) PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(      0x1000, "9-11" )
	PORT_DIPSETTING(      0x1800, "10-10" )
	PORT_DIPSETTING(      0x0800, "10-11" )
	PORT_DIPSETTING(      0x0000, "11-11" )
	PORT_SERVICE_DIPLOC( 0x2000, IP_ACTIVE_HIGH, "SWB:6" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout splayout =
{
	16,32,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP4(3*4,-4), STEP4(7*4,-4), STEP4(11*4,-4), STEP4(15*4,-4) },
	{ STEP32(0,16*4) },
	16*32*4
};


static GFXDECODE_START( gfx_rpunch )
	GFXDECODE_ENTRY( "tiles1",  0, gfx_8x8x4_packed_lsb,   0, 16 )
	GFXDECODE_ENTRY( "tiles2",  0, gfx_8x8x4_packed_lsb, 256, 16 )
	GFXDECODE_ENTRY( "sprites", 0, splayout,               0, 64 )
GFXDECODE_END


static const gfx_layout bootleg_tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,8) },
	{ STEP8(0,1) },
	{ STEP8(0,8*4) },
	8*8*4
};

static const gfx_layout bootleg_sprite_layout =
{
	16,32,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,8) },
	{ STEP8(0,1),  STEP8(32*8*4,1) },
	{ STEP32(0,8*4) },
	16*32*4
};

static GFXDECODE_START( gfx_svolleybl )
	GFXDECODE_ENTRY( "tiles1",  0, bootleg_tile_layout,   0, 16 )
	GFXDECODE_ENTRY( "tiles2",  0, bootleg_tile_layout, 256, 16 )
	GFXDECODE_ENTRY( "sprites", 0, bootleg_sprite_layout, 0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static constexpr XTAL MASTER_CLOCK = XTAL(16'000'000);
static constexpr XTAL VIDEO_CLOCK = XTAL(13'333'000);

void rpunch_state::rpunch(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &rpunch_state::rpunch_map);

	Z80(config, m_audiocpu, MASTER_CLOCK/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &rpunch_state::sound_map);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set("soundirq", FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline(m_audiocpu, 0);

	// video hardware
	set_sprite_palette(0x300);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(304, 224);
	m_screen->set_visarea(8, 303-8, 0, 223-8);
	m_screen->set_screen_update(FUNC(rpunch_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rpunch);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_GGA(config, m_gga, VIDEO_CLOCK/2); // verified from rpunch schematics
	m_gga->write_cb().set(FUNC(rpunch_state::gga_data_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", MASTER_CLOCK/4));
	ymsnd.irq_handler().set("soundirq", FUNC(input_merger_device::in_w<1>));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	UPD7759(config, m_upd7759).add_route(ALL_OUTPUTS, "mono", 0.50);
}


// the main differences between Super Volleyball and Rabbit Punch are
// the lack of direct-mapped bitmap and a different palette base for sprites
void svolley_state::svolley(machine_config &config)
{
	rpunch(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &svolley_state::svolley_map);

	set_sprite_palette(0x080);
	set_sprite_xoffs(-4); // aligns middle net sprite with bg as shown in reference
}


// FIXME: copy/paste of above for now, bootleg hardware, things need verifying
void rpunch_state::svolleybl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &rpunch_state::svolleybl_main_map);

	Z80(config, m_audiocpu, MASTER_CLOCK/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &rpunch_state::svolleybl_sound_map);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	// video hardware
	set_sprite_palette(0x080);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(304, 224);
	m_screen->set_visarea(8, 303-8, 0, 223-8);
	m_screen->set_screen_update(FUNC(rpunch_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_svolleybl);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_GGA(config, m_gga, VIDEO_CLOCK/2);
	m_gga->write_cb().set(FUNC(rpunch_state::gga_data_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", MASTER_CLOCK/4));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	// TODO: OKI M5205
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// VS7-0102 PCB (CPU Board) with VS7-0101 PCB (Video/ROM Board?)
ROM_START( rpunch )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rpunch.20", 0x00000, 0x08000, CRC(a2028d59) SHA1(d304811853ad68b3977edb90b94f3e2c7507be82) )
	ROM_LOAD16_BYTE( "rpunch.21", 0x00001, 0x08000, CRC(1cdb13d3) SHA1(a51b2bbbd7b4553500b65aa6a20fabe03432e6ca) )
	ROM_LOAD16_BYTE( "rpunch.2",  0x10000, 0x08000, CRC(9b9729bb) SHA1(3fcf8df9787137d331c062bc4ee97d865af29c78) )
	ROM_LOAD16_BYTE( "rpunch.3",  0x10001, 0x08000, CRC(5704a688) SHA1(3c447cd98ecd25798dbeea328e3ccbaad384c8b1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rpunch.92", 0x00000, 0x10000, CRC(5e1870e3) SHA1(0ab33f39144ed72d805341d869f61764610d3df6) )

	ROM_REGION( 0x80000, "tiles1", ROMREGION_ERASEFF )
	ROM_LOAD( "rl_c13.bin", 0x00000, 0x40000, CRC(7c8403b0) SHA1(2fb92860a41f3331076c73b2b010e175cb4929ca) )
	ROM_LOAD( "rl_c10.bin", 0x40000, 0x08000, CRC(312eb260) SHA1(31faa90fde54fbc6c110bee7b4690a30beaec469) )
	ROM_LOAD( "rl_c12.bin", 0x48000, 0x08000, CRC(bea85219) SHA1(4036bdad921dd3555a2dc6bb12e9ffa615de70ca) )
	ROM_FILL(               0x50000, 0x10000, 0xff )

	ROM_REGION( 0x80000, "tiles2", ROMREGION_ERASEFF )
	ROM_LOAD( "rl_a10.bin", 0x00000, 0x40000, CRC(c2a77619) SHA1(9b1e85fb18833c3b96a6c58b8714984f60a90afc) )
	ROM_LOAD( "rl_a13.bin", 0x40000, 0x08000, CRC(a39c2c16) SHA1(d8d55eb58d3fc79f982f535ec85f69593fe9d883) )
	ROM_LOAD( "rpunch.54",  0x48000, 0x08000, CRC(e2969747) SHA1(8da996fc2e2e3d281f293d0ccaf35ebdb9379d48) )

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rl_4g.bin", 0x00000, 0x20000, CRC(c5cb4b7a) SHA1(2b6be85800ab62b000a0b01cff8af689b25c4c65) )
	ROM_LOAD16_BYTE( "rl_4h.bin", 0x00001, 0x20000, CRC(8a4d3c99) SHA1(29fca0ea60ac040fc0b0f65b9d2069fcffe039bd) )
	ROM_LOAD16_BYTE( "rl_1g.bin", 0x40000, 0x08000, CRC(74d41b2e) SHA1(cd690df46242dd54061bf5f6464203e1f29ce6d8) )
	ROM_LOAD16_BYTE( "rl_1h.bin", 0x40001, 0x08000, CRC(7dcb32bb) SHA1(3401464494447b681456d81199e8e74837419a39) )
	ROM_LOAD16_BYTE( "rpunch.85", 0x50000, 0x08000, CRC(60b88a2c) SHA1(b10aba06a5d88d0f27041f9e356aebf9f8a230df) )
	ROM_LOAD16_BYTE( "rpunch.86", 0x50001, 0x08000, CRC(91d204f6) SHA1(68b5fb29ea5404597adada1a197ad853e79ada1c) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "rl_f18.bin", 0x00000, 0x20000, CRC(47840673) SHA1(ffe20f8772a987f5dd06a3f348a1e3cfed26e19e) )
//  ROM_LOAD( "rpunch.91",  0x00000, 0x0f000, CRC(7512cc59) SHA1(1eb151a1a9cdb7922d2cb422dff41d452238a2b8) )
ROM_END

ROM_START( rabiolep )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rl_e2.bin", 0x00000, 0x08000, CRC(7d936a12) SHA1(4f472ccbceb13b4d0b14c686d151faa8d4c466c7) )
	ROM_LOAD16_BYTE( "rl_d2.bin", 0x00001, 0x08000, CRC(d8d85429) SHA1(dc52c30c18026300a4625bddc987a00396bcd079) )
	ROM_LOAD16_BYTE( "rl_e4.bin", 0x10000, 0x08000, CRC(5bfaee12) SHA1(c918b1843ad96410f41915574cf12949658c6e90) )
	ROM_LOAD16_BYTE( "rl_d4.bin", 0x10001, 0x08000, CRC(e64216bf) SHA1(851ebab8d7cddb60ab0a8a290df151b35d605e3e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rl_f20.bin", 0x00000, 0x10000, CRC(a6f50351) SHA1(3152d4ed100b0dfaf0da4ee79cd9e0f1692335e0) )

	ROM_REGION( 0x80000, "tiles1", ROMREGION_ERASEFF )
	ROM_LOAD( "rl_c13.bin", 0x00000, 0x40000, CRC(7c8403b0) SHA1(2fb92860a41f3331076c73b2b010e175cb4929ca) )
	ROM_LOAD( "rl_c10.bin", 0x40000, 0x08000, CRC(312eb260) SHA1(31faa90fde54fbc6c110bee7b4690a30beaec469) )
	ROM_LOAD( "rl_c12.bin", 0x48000, 0x08000, CRC(bea85219) SHA1(4036bdad921dd3555a2dc6bb12e9ffa615de70ca) )
	ROM_FILL(               0x50000, 0x10000, 0xff )

	ROM_REGION( 0x80000, "tiles2", ROMREGION_ERASEFF )
	ROM_LOAD( "rl_a10.bin", 0x00000, 0x40000, CRC(c2a77619) SHA1(9b1e85fb18833c3b96a6c58b8714984f60a90afc) )
	ROM_LOAD( "rl_a13.bin", 0x40000, 0x08000, CRC(a39c2c16) SHA1(d8d55eb58d3fc79f982f535ec85f69593fe9d883) )
	ROM_LOAD( "rl_a12.bin", 0x48000, 0x08000, CRC(970b0e32) SHA1(a1d4025ee4470a41aa047c6f06ca7aa98a1f7ffd) )

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rl_4g.bin", 0x00000, 0x20000, CRC(c5cb4b7a) SHA1(2b6be85800ab62b000a0b01cff8af689b25c4c65) )
	ROM_LOAD16_BYTE( "rl_4h.bin", 0x00001, 0x20000, CRC(8a4d3c99) SHA1(29fca0ea60ac040fc0b0f65b9d2069fcffe039bd) )
	ROM_LOAD16_BYTE( "rl_1g.bin", 0x40000, 0x08000, CRC(74d41b2e) SHA1(cd690df46242dd54061bf5f6464203e1f29ce6d8) )
	ROM_LOAD16_BYTE( "rl_1h.bin", 0x40001, 0x08000, CRC(7dcb32bb) SHA1(3401464494447b681456d81199e8e74837419a39) )
	ROM_LOAD16_BYTE( "rl_2g.bin", 0x50000, 0x08000, CRC(744903b4) SHA1(ba931a7f6bea8cebab8314551ed34896316b6661) )
	ROM_LOAD16_BYTE( "rl_2h.bin", 0x50001, 0x08000, CRC(09649e75) SHA1(a650561a11970fbcbc4610fc67cb9f54fa3145a6) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "rl_f18.bin", 0x00000, 0x20000, CRC(47840673) SHA1(ffe20f8772a987f5dd06a3f348a1e3cfed26e19e) )
ROM_END

// VS-68K-2 (H2) PCB
ROM_START( svolley )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sps_13.bin", 0x00000, 0x10000, CRC(2fbc5dcf) SHA1(fba4d353948f29b75b4db464509f7e606703f9dc) )
	ROM_LOAD16_BYTE( "sps_11.bin", 0x00001, 0x10000, CRC(51b025c9) SHA1(4eae91ee9fe893083d0ff5fed28d847dba49b244) )
	ROM_LOAD16_BYTE( "sps_14.bin", 0x20000, 0x08000, CRC(e7630122) SHA1(d200afe5134030be615f112af0ab54ac3b349eca) )
	ROM_LOAD16_BYTE( "sps_12.bin", 0x20001, 0x08000, CRC(b6b24910) SHA1(2e4cf80a8eb1fcd9448405ff881bb99ae4ce8909) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sps_17.bin", 0x00000, 0x10000, CRC(48b89688) SHA1(1f39d979a852f5237a7d95231e86a28cdc1f4d65) )

	ROM_REGION( 0x80000, "tiles1", ROMREGION_ERASEFF )
	ROM_LOAD( "sps_02.bin", 0x00000, 0x10000, CRC(1a0abe75) SHA1(49251c5e377f9317471f7df26ac2c6b8cfa51007) )
	ROM_LOAD( "sps_03.bin", 0x10000, 0x10000, CRC(36279075) SHA1(6c4cf3fab9eb764cb8bc10ab4f8aa54d0afb65d9) )
	ROM_LOAD( "sps_04.bin", 0x20000, 0x10000, CRC(7cede7d9) SHA1(9c7e3a9b7dd8d390b327d52ced35b03b8c1fd5ee) )
	ROM_LOAD( "sps_01.bin", 0x30000, 0x08000, CRC(6425e6d7) SHA1(b6c81155c22072d1de88ca23d58bd9621139dc6c) )
	ROM_LOAD( "sps_10.bin", 0x40000, 0x08000, CRC(a12b1589) SHA1(ecaa941f29c028ca94fcd1d86edfd69884e61d2c) )

	ROM_REGION( 0x80000, "tiles2", ROMREGION_ERASEFF )
	ROM_LOAD( "sps_05.bin", 0x00000, 0x10000, CRC(b0671d12) SHA1(defc71b6d7c31c74a58789a1620a506f36b85837) )
	ROM_LOAD( "sps_06.bin", 0x10000, 0x10000, CRC(c231957e) SHA1(b56afd41969bd865ad3ca16fb51e39030aeb1943) )
	ROM_LOAD( "sps_07.bin", 0x20000, 0x10000, CRC(904b7709) SHA1(9b66a565cd599928b666baad9f97c50f35ffcc37) )
	ROM_LOAD( "sps_08.bin", 0x30000, 0x10000, CRC(5430ffac) SHA1(163311d96f2f7e1ecb0901d0be73ac357b01bf6a) )
	ROM_LOAD( "sps_09.bin", 0x40000, 0x10000, CRC(414a6278) SHA1(baa9dc9ab0dd3c5f27c128de23053edcddf45ad0) )

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sps_20.bin", 0x00000, 0x10000, CRC(c9e7206d) SHA1(af5b2f49387a3b46c6693f4782aa0e587f17ab25) )
	ROM_LOAD16_BYTE( "sps_23.bin", 0x00001, 0x10000, CRC(7b15c805) SHA1(b55d67956ca10c172f244edce4dc0a8bd155b3ce) )
	ROM_LOAD16_BYTE( "sps_19.bin", 0x20000, 0x08000, CRC(8ac2f232) SHA1(6ccd003d7e6fb933241e58964e682bd0fcc37b35) )
	ROM_LOAD16_BYTE( "sps_22.bin", 0x20001, 0x08000, CRC(fcc754e3) SHA1(84a4083262095d099bca4d5c29829527d981130f) )
	ROM_LOAD16_BYTE( "sps_18.bin", 0x30000, 0x08000, CRC(4d6c8f0c) SHA1(27f58a53cd6aef071c685eda532e4909ea915c8d) )
	ROM_RELOAD(0x60000, 0x8000) // these mirrors are needed for the player hints to display
	ROM_LOAD16_BYTE( "sps_21.bin", 0x30001, 0x08000, CRC(9dd28b42) SHA1(5f49456ee49ed7df59629d02a9da57eac370c388) )
	ROM_RELOAD(0x60001, 0x8000)

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "sps_16.bin", 0x00000, 0x20000, CRC(456d0f36) SHA1(3d1bdc5c79b41a7b33932d6a8b838f01cea9d4ed) )
	ROM_LOAD( "sps_15.bin", 0x20000, 0x10000, CRC(f33f415f) SHA1(1dd465d9b3009754a7d53400562a53dacff364fc) )
ROM_END

ROM_START( svolleyk )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a14.bin", 0x00000, 0x10000, CRC(dbab3bf9) SHA1(b329245666fb5082871d661714332b10cf49fe41) )
	ROM_LOAD16_BYTE( "a11.bin", 0x00001, 0x10000, CRC(92afd56f) SHA1(6edb421af6051de640c3ed9bb75bd9e7f609ce14) )
	ROM_LOAD16_BYTE( "a15.bin", 0x20000, 0x08000, CRC(d8f89c4a) SHA1(454b42277457d3ebdefb04fc91f6bdc1e0d28439) )
	ROM_LOAD16_BYTE( "a12.bin", 0x20001, 0x08000, CRC(de3dd5cb) SHA1(ea997b91ff45513e69fe76d71da40c8f9dc112f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sps_17.bin", 0x00000, 0x10000, CRC(48b89688) SHA1(1f39d979a852f5237a7d95231e86a28cdc1f4d65) )

	ROM_REGION( 0x80000, "tiles1", ROMREGION_ERASEFF )
	ROM_LOAD( "sps_02.bin", 0x00000, 0x10000, CRC(1a0abe75) SHA1(49251c5e377f9317471f7df26ac2c6b8cfa51007) )
	ROM_LOAD( "sps_03.bin", 0x10000, 0x10000, CRC(36279075) SHA1(6c4cf3fab9eb764cb8bc10ab4f8aa54d0afb65d9) )
	ROM_LOAD( "sps_04.bin", 0x20000, 0x10000, CRC(7cede7d9) SHA1(9c7e3a9b7dd8d390b327d52ced35b03b8c1fd5ee) )
	ROM_LOAD( "sps_01.bin", 0x30000, 0x08000, CRC(6425e6d7) SHA1(b6c81155c22072d1de88ca23d58bd9621139dc6c) )
	ROM_LOAD( "sps_10.bin", 0x40000, 0x08000, CRC(a12b1589) SHA1(ecaa941f29c028ca94fcd1d86edfd69884e61d2c) )

	ROM_REGION( 0x80000, "tiles2", ROMREGION_ERASEFF )
	ROM_LOAD( "sps_05.bin", 0x00000, 0x10000, CRC(b0671d12) SHA1(defc71b6d7c31c74a58789a1620a506f36b85837) )
	ROM_LOAD( "sps_06.bin", 0x10000, 0x10000, CRC(c231957e) SHA1(b56afd41969bd865ad3ca16fb51e39030aeb1943) )
	ROM_LOAD( "sps_07.bin", 0x20000, 0x10000, CRC(904b7709) SHA1(9b66a565cd599928b666baad9f97c50f35ffcc37) )
	ROM_LOAD( "sps_08.bin", 0x30000, 0x10000, CRC(5430ffac) SHA1(163311d96f2f7e1ecb0901d0be73ac357b01bf6a) )
	ROM_LOAD( "sps_09.bin", 0x40000, 0x10000, CRC(414a6278) SHA1(baa9dc9ab0dd3c5f27c128de23053edcddf45ad0) )
	ROM_LOAD( "a09.bin",    0x50000, 0x08000, CRC(dd92dfe1) SHA1(08c956e11d567a215ec3cdaf6ef75fa9a886513a) ) // contains Korea, GB and Spain flags

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sps_20.bin", 0x00000, 0x10000, CRC(c9e7206d) SHA1(af5b2f49387a3b46c6693f4782aa0e587f17ab25) )
	ROM_LOAD16_BYTE( "sps_23.bin", 0x00001, 0x10000, CRC(7b15c805) SHA1(b55d67956ca10c172f244edce4dc0a8bd155b3ce) )
	ROM_LOAD16_BYTE( "sps_19.bin", 0x20000, 0x08000, CRC(8ac2f232) SHA1(6ccd003d7e6fb933241e58964e682bd0fcc37b35) )
	ROM_LOAD16_BYTE( "sps_22.bin", 0x20001, 0x08000, CRC(fcc754e3) SHA1(84a4083262095d099bca4d5c29829527d981130f) )
	ROM_LOAD16_BYTE( "sps_18.bin", 0x30000, 0x08000, CRC(4d6c8f0c) SHA1(27f58a53cd6aef071c685eda532e4909ea915c8d) )
	ROM_RELOAD(0x60000, 0x8000)
	ROM_LOAD16_BYTE( "sps_21.bin", 0x30001, 0x08000, CRC(9dd28b42) SHA1(5f49456ee49ed7df59629d02a9da57eac370c388) )
	ROM_RELOAD(0x60001, 0x8000)

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "sps_16.bin", 0x00000, 0x20000, CRC(456d0f36) SHA1(3d1bdc5c79b41a7b33932d6a8b838f01cea9d4ed) )
	ROM_LOAD( "sps_15.bin", 0x20000, 0x10000, CRC(f33f415f) SHA1(1dd465d9b3009754a7d53400562a53dacff364fc) )
ROM_END

ROM_START( svolleyu )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "svb-du8.137", 0x00000, 0x10000, CRC(ffd5d261) SHA1(cf1a6897b88481d6dc7ffca647f85b91b04c2242) )
	ROM_LOAD16_BYTE( "svb-du5.136", 0x00001, 0x10000, CRC(c1e943f5) SHA1(dda0db7dcf61038fd14d1717ee036e0e5b92253d) )
	ROM_LOAD16_BYTE( "svb-du9.127", 0x20000, 0x08000, CRC(70e04a2e) SHA1(0dec1e21a7bf611caf3487a31965654ce1fe2989) )
	ROM_LOAD16_BYTE( "svb-du6.126", 0x20001, 0x08000, CRC(acb3872b) SHA1(a720efe3f248c6a8c8f95c5c90bfe7f1e8537911) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sps_17.bin", 0x00000, 0x10000, CRC(48b89688) SHA1(1f39d979a852f5237a7d95231e86a28cdc1f4d65) )

	ROM_REGION( 0x80000, "tiles1", ROMREGION_ERASEFF )
	ROM_LOAD( "sps_02.bin", 0x00000, 0x10000, CRC(1a0abe75) SHA1(49251c5e377f9317471f7df26ac2c6b8cfa51007) )
	ROM_LOAD( "sps_03.bin", 0x10000, 0x10000, CRC(36279075) SHA1(6c4cf3fab9eb764cb8bc10ab4f8aa54d0afb65d9) )
	ROM_LOAD( "sps_04.bin", 0x20000, 0x10000, CRC(7cede7d9) SHA1(9c7e3a9b7dd8d390b327d52ced35b03b8c1fd5ee) )
	ROM_LOAD( "sps_01.bin", 0x30000, 0x08000, CRC(6425e6d7) SHA1(b6c81155c22072d1de88ca23d58bd9621139dc6c) )
	ROM_LOAD( "sps_10.bin", 0x40000, 0x08000, CRC(a12b1589) SHA1(ecaa941f29c028ca94fcd1d86edfd69884e61d2c) )

	ROM_REGION( 0x80000, "tiles2", ROMREGION_ERASEFF )
	ROM_LOAD( "sps_05.bin", 0x00000, 0x10000, CRC(b0671d12) SHA1(defc71b6d7c31c74a58789a1620a506f36b85837) )
	ROM_LOAD( "sps_06.bin", 0x10000, 0x10000, CRC(c231957e) SHA1(b56afd41969bd865ad3ca16fb51e39030aeb1943) )
	ROM_LOAD( "sps_07.bin", 0x20000, 0x10000, CRC(904b7709) SHA1(9b66a565cd599928b666baad9f97c50f35ffcc37) )
	ROM_LOAD( "sps_08.bin", 0x30000, 0x10000, CRC(5430ffac) SHA1(163311d96f2f7e1ecb0901d0be73ac357b01bf6a) )
	ROM_LOAD( "sps_09.bin", 0x40000, 0x10000, CRC(414a6278) SHA1(baa9dc9ab0dd3c5f27c128de23053edcddf45ad0) )
//  ROM_LOAD( "a09.bin",    0x50000, 0x08000, CRC(dd92dfe1) SHA1(08c956e11d567a215ec3cdaf6ef75fa9a886513a) ) // not on this set?

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sps_20.bin", 0x00000, 0x10000, CRC(c9e7206d) SHA1(af5b2f49387a3b46c6693f4782aa0e587f17ab25) )
	ROM_LOAD16_BYTE( "sps_23.bin", 0x00001, 0x10000, CRC(7b15c805) SHA1(b55d67956ca10c172f244edce4dc0a8bd155b3ce) )
	ROM_LOAD16_BYTE( "sps_19.bin", 0x20000, 0x08000, CRC(8ac2f232) SHA1(6ccd003d7e6fb933241e58964e682bd0fcc37b35) )
	ROM_LOAD16_BYTE( "sps_22.bin", 0x20001, 0x08000, CRC(fcc754e3) SHA1(84a4083262095d099bca4d5c29829527d981130f) )
	ROM_LOAD16_BYTE( "sps_18.bin", 0x30000, 0x08000, CRC(4d6c8f0c) SHA1(27f58a53cd6aef071c685eda532e4909ea915c8d) )
	ROM_RELOAD(0x60000, 0x8000)
	ROM_LOAD16_BYTE( "sps_21.bin", 0x30001, 0x08000, CRC(9dd28b42) SHA1(5f49456ee49ed7df59629d02a9da57eac370c388) )
	ROM_RELOAD(0x60001, 0x8000)

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "sps_16.bin", 0x00000, 0x20000, CRC(456d0f36) SHA1(3d1bdc5c79b41a7b33932d6a8b838f01cea9d4ed) )
	ROM_LOAD( "sps_15.bin", 0x20000, 0x10000, CRC(f33f415f) SHA1(1dd465d9b3009754a7d53400562a53dacff364fc) )
ROM_END

 // VS-68K-2 (H2) PCB, same PCB as the svolley set but this has some MASK ROMs.
 // Shows for use in Japan disclaimer but has English text and makes you play as the USA like svolleyu, but no Data East license.
ROM_START( svolleyua )
	ROM_REGION( 0x40000, "maincpu", 0 ) // differs
	ROM_LOAD16_BYTE( "h0.ic137", 0x00000, 0x10000, CRC(58cfa5d7) SHA1(2e07121b301d4371c0b2bc865276772bf5b923e5) ) // 27512
	ROM_LOAD16_BYTE( "l0.ic136", 0x00001, 0x10000, CRC(b1f5a54c) SHA1(71b9c9b5cca93c65a87de10030e64005fe3f605c) ) // 27512
	ROM_LOAD16_BYTE( "h1.ic127", 0x20000, 0x08000, CRC(3c9721ff) SHA1(4a23401667af01b5255d86e2acb66fd47bd40600) ) // 27256
	ROM_LOAD16_BYTE( "l1.ic126", 0x20001, 0x08000, CRC(55cfabce) SHA1(c35d9cbf54f5103703a169b4764e063f55f5e6ef) ) // 27256

	ROM_REGION( 0x10000, "audiocpu", 0 ) // same
	ROM_LOAD( "1.ic112", 0x00000, 0x10000, CRC(48b89688) SHA1(1f39d979a852f5237a7d95231e86a28cdc1f4d65) ) // 27512

	ROM_REGION( 0x80000, "tiles1", ROMREGION_ERASEFF ) // same but split differently
	ROM_LOAD16_WORD_SWAP( "lh532099.ic46", 0x00000, 0x40000, CRC(9428cc36) SHA1(d915dc6ee16cca695fd8b2ff3049a9cc7f601e34) ) // 2m MASK ROM, 27C020 compatible
	ROM_LOAD16_WORD(      "7.ic35",        0x40000, 0x10000, CRC(83b20b91) SHA1(043cb6035d762fca3955221edee284fbcf0f8d1e) ) // 27512, double sized but second half 0x00 filled

	ROM_REGION( 0x80000, "tiles2", ROMREGION_ERASEFF ) // same but split differently
	ROM_LOAD16_WORD_SWAP( "lh53200a.ic47", 0x00000, 0x40000, CRC(75930468) SHA1(be5889b969ede70a8e39932e91fd6bd24b9f9df2) ) // 2m MASK ROM, 27C020 compatible
	ROM_LOAD16_WORD(      "10.ic36",       0x40000, 0x10000, CRC(414a6278) SHA1(baa9dc9ab0dd3c5f27c128de23053edcddf45ad0) ) // 27512

	ROM_REGION( 0x80000, "sprites", ROMREGION_ERASEFF ) // same but split differently
	ROM_LOAD16_WORD( "lh5320h7.ic51", 0x00000, 0x40000, CRC(152ff5b6) SHA1(8574f59757d198b197284efabb2befe3a7e41d45) ) // 2m MASK ROM, 27C020 compatible
	ROM_LOAD16_BYTE( "s0.ic18",       0x60000, 0x08000, CRC(4d6c8f0c) SHA1(27f58a53cd6aef071c685eda532e4909ea915c8d) ) // 27256
	ROM_LOAD16_BYTE( "s1.ic43",       0x60001, 0x08000, CRC(9dd28b42) SHA1(5f49456ee49ed7df59629d02a9da57eac370c388) ) // 27256, S1 was not readable but since S0 matches it's almost certain this does too

	ROM_REGION( 0x40000, "upd", 0 )  // same but split differently
	ROM_LOAD( "4.ic114", 0x00000, 0x10000, CRC(c4effee6) SHA1(84dece09139f804dcf8a07c2089cec84515eba16) ) // 27512
	ROM_LOAD( "3.ic123", 0x10000, 0x10000, CRC(5a818eb4) SHA1(a4596a9aedda0b362546e4de7cbbdf1eb1fd7ade) ) // 27512
	ROM_LOAD( "2.ic133", 0x20000, 0x10000, CRC(f33f415f) SHA1(1dd465d9b3009754a7d53400562a53dacff364fc) ) // 27512
ROM_END


ROM_START( svolleybl )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "4-prg.bin", 0x00001, 0x10000, CRC(eefaa208) SHA1(2a0417e170de3212f45be64719bb1eb0c6d33c59) )
	ROM_LOAD16_BYTE( "6-prg.bin", 0x00000, 0x10000, CRC(da7d2e81) SHA1(ca78a661876ddbcb0e7599edcc819558afb76930) )
	ROM_LOAD16_BYTE( "5-prg.bin", 0x20000, 0x08000, CRC(e7630122) SHA1(d200afe5134030be615f112af0ab54ac3b349eca) ) // these 2 match program ROMs from svolley
	ROM_LOAD16_BYTE( "3-prg.bin", 0x20001, 0x08000, CRC(b6b24910) SHA1(2e4cf80a8eb1fcd9448405ff881bb99ae4ce8909) )

	ROM_REGION( 0x080000, "tiles1", 0 )
	ROM_LOAD32_BYTE( "7.bin",        0x000000, 0x010000, CRC(9596a4c0) SHA1(1f233bb2fa662fb8cd9c0db478e392ca26d9484b) )
	ROM_LOAD32_BYTE( "10.bin",       0x000001, 0x010000, CRC(a05249e6) SHA1(8671e0c980ba87ea14895176fb5c8a48bb4c932e) )
	ROM_LOAD32_BYTE( "13.bin",       0x000002, 0x010000, CRC(429159f3) SHA1(4395413c4ab4a1fd322a1af6f2b93bb62b044223) )
	ROM_LOAD32_BYTE( "16.bin",       0x000003, 0x010000, CRC(f5436c8d) SHA1(d29508cc5ee43d7b072112c6d95c36ee0328e5fb) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD32_BYTE( "8.bin",        0x000000, 0x010000, CRC(451ebd75) SHA1(67d5a9fadf3c8a39d59e7b21cb8633dd19886f76) )
	ROM_LOAD32_BYTE( "11.bin",       0x000001, 0x010000, CRC(0983987a) SHA1(c334276774ffdee0023ea6287e98e0e6e372fb80) )
	ROM_LOAD32_BYTE( "14.bin",       0x000002, 0x010000, CRC(4babf749) SHA1(1d5055e825b9efc17a200f4e04e6fa326397f7cc) )
	ROM_LOAD32_BYTE( "17.bin",       0x000003, 0x010000, CRC(f82f9664) SHA1(678fd8f3abc39ccb4ef32e9d6ef481d7d751aecb) )
	ROM_LOAD32_BYTE( "9.bin",        0x040000, 0x008000, CRC(3291e3e0) SHA1(dcc358bf66e4c65992d4376c203b811928068cf3) )
	ROM_LOAD32_BYTE( "12.bin",       0x040001, 0x008000, CRC(40aedad9) SHA1(cbf50eae4ccbc06213a5c227409e1dade7180572) )
	ROM_LOAD32_BYTE( "15.bin",       0x040002, 0x008000, CRC(911104d7) SHA1(66b48c34da2cc17faeffa1d36f5b6b7e15c2033b) )
	ROM_LOAD32_BYTE( "18.bin",       0x040003, 0x008000, CRC(07265de1) SHA1(bad7f1b168640a7d90b0d4d9c255ba98fa4c6fa8) )

	ROM_REGION( 0x080000, "sprites", ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "19.bin",       0x000000, 0x010000, CRC(12a67e3f) SHA1(c77b264eae0f55af36728b6e5e5e1fec3d366eb1) )
	ROM_LOAD32_BYTE( "20.bin",       0x000001, 0x010000, CRC(31828996) SHA1(b324902b9fff0bab1daa3af5136b96d50d12956f) )
	ROM_LOAD32_BYTE( "21.bin",       0x000002, 0x010000, CRC(51cbe0d6) SHA1(d60b2a297d7e994c60db28e8ba60b0664e01f61d) )
	ROM_LOAD32_BYTE( "22.bin",       0x000003, 0x010000, CRC(c289bfc0) SHA1(4a8929c5f304a1d203cad04c72fc6e96764dc858) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 Sound CPU
	ROM_LOAD( "2-snd.bin", 0x00000, 0x10000, CRC(e3065b1d) SHA1(c4a3a95ba7f43cdf1b0c574f41de06d007ad2bd8) ) // matches 1.ic140 from spikes91
	ROM_LOAD( "1-snd.bin", 0x10000, 0x08000, CRC(009d7157) SHA1(2cdda7094c7476289d75a78ee25b34fa3b3225c0) ) // matches 2.ic141 from spikes91, when halved
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    year  name       parent    machine    input     class          init        rot   company                                      description                                 flags
GAME( 1987, rabiolep,  0,        rpunch,    rabiolep, rpunch_state,  empty_init, ROT0, "V-System Co.",                              "Rabio Lepus (Japan)",                      MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1987, rpunch,    rabiolep, rpunch,    rpunch,   rpunch_state,  empty_init, ROT0, "V-System Co. (Bally/Midway/Sente license)", "Rabbit Punch (US)",                        MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1989, svolley,   0,        svolley,   svolley,  svolley_state, empty_init, ROT0, "V-System Co.",                              "Super Volleyball (Japan)",                 MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1989, svolleyk,  svolley,  svolley,   svolley,  svolley_state, empty_init, ROT0, "V-System Co.",                              "Super Volleyball (Korea)",                 MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1989, svolleyu,  svolley,  svolley,   svolley,  svolley_state, empty_init, ROT0, "V-System Co. (Data East license)",          "Super Volleyball (US, Data East license)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1989, svolleyua, svolley,  svolley,   svolley,  svolley_state, empty_init, ROT0, "V-System Co.",                              "Super Volleyball (US)",                    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

// video registers are changed, and there's some kind of RAM at 090xxx, possible a different sprite scheme for the bootleg (even if the original is intact)
// the sound system seems to be ripped from the later Power Spikes (see aerofgt.cpp)
GAME( 1991, svolleybl, svolley,  svolleybl, svolley,  rpunch_state,  empty_init, ROT0, "bootleg",  "Super Volleyball (bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_NO_COCKTAIL ) // aka 1991 Spikes?
