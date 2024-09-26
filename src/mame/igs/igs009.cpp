// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood, Roberto Fresca
/***************************************************************************

                          -= IGS009 Based Games =-

                   driver by Luca Elia and David Haywood

CPU     :   Z180
Sound   :   M6295 + YM2413
Video   :   IGS009
NVRAM   :   Battery for main RAM

- The hardware is similar to other IGS002 + IGS003 based boards.
  The interesting part is the background tilemap, that is designed specifically
  for simulating the nine reels of a slot machine.

---------------------------------------------------------------------------
Year  Game                         Manufacturer    Notes
---------------------------------------------------------------------------
1997  Jingle Bell (US, V157US)     IGS             patched protection
1997  Jingle Bell (EU, V155UE)     IGS             patched protection
1997  Jingle Bell (EU, V153UE)     IGS             patched protection
1995  Jingle Bell (EU, V141UE)     IGS             patched protection
1995? Jingle Bell (Italy, V133I)   IGS             patched protection
1998  Grand Prix '98               Romtec
---------------------------------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class igs009_state : public driver_device
{
public:
	igs009_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_bg_scroll(*this, "bg_scroll.%u", 0U)
		, m_reel_ram(*this, "reel_ram.%u", 0U)
		, m_fg_tile_ram(*this, "fg_tile_ram")
		, m_fg_color_ram(*this, "fg_color_ram")
		, m_leds(*this, "led%u", 0U)
	{ }

	void gp98(machine_config &config);
	void jingbell(machine_config &config);

	void init_jingbell();
	void init_jingbelli();
	void init_animalhjb();

	int hopper_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	template<uint8_t Reel> void reel_ram_w(offs_t offset, uint8_t data);
	void bg_scroll_w(offs_t offset, uint8_t data);
	void fg_tile_w(offs_t offset, uint8_t data);
	void fg_color_w(offs_t offset, uint8_t data);
	void nmi_and_coins_w(uint8_t data);
	void video_and_leds_w(uint8_t data);
	void leds_w(uint8_t data);
	void magic_w(offs_t offset, uint8_t data);
	uint8_t magic_r();

	void show_out();
	void vblank_irq(int state);

	template<uint8_t Reel> TILE_GET_INFO_MEMBER(get_jingbell_reel_tile_info);
	template<uint8_t Reel> TILE_GET_INFO_MEMBER(get_gp98_reel_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void decrypt_jingbell();

	DECLARE_VIDEO_START(gp98);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gp98_portmap(address_map &map) ATTR_COLD;
	void jingbell_map(address_map &map) ATTR_COLD;
	void jingbell_portmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 2> m_bg_scroll;
	required_shared_ptr_array<uint8_t, 4> m_reel_ram;
	required_shared_ptr<uint8_t> m_fg_tile_ram;
	required_shared_ptr<uint8_t> m_fg_color_ram;
	output_finder<7> m_leds;

	tilemap_t *m_reel_tilemap[4]{};
	tilemap_t *m_fg_tilemap = nullptr;
	int m_video_enable = 0;
	int m_nmi_enable = 0;
	int m_hopper = 0;
	uint8_t m_out[3]{};
	uint8_t m_igs_magic[2]{};
};


/***************************************************************************
                                Video Hardware
***************************************************************************/


template<uint8_t Reel>
void igs009_state::reel_ram_w(offs_t offset, uint8_t data)
{
	m_reel_ram[Reel][offset] = data;
	m_reel_tilemap[Reel]->mark_tile_dirty(offset);
}

template<uint8_t Reel>
TILE_GET_INFO_MEMBER(igs009_state::get_jingbell_reel_tile_info)
{
	int code = m_reel_ram[Reel][tile_index];

	tileinfo.set(0,
			(code)+(((tile_index+1)&0x3)*0x100),
			(code & 0x80) ? 0xc : 0,
			0);
}

template<uint8_t Reel>
TILE_GET_INFO_MEMBER(igs009_state::get_gp98_reel_tile_info)
{
	int code = m_reel_ram[Reel][tile_index];

	tileinfo.set(0,
			(code*4)+(tile_index&0x3),
			0,
			0);
}

void igs009_state::bg_scroll_w(offs_t offset, uint8_t data)
{
	m_bg_scroll[0][offset] = data;
//  bg_tilemap->set_scrolly(offset,data);
}


TILE_GET_INFO_MEMBER(igs009_state::get_fg_tile_info)
{
	int code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	tileinfo.set(1, code, (4*(code >> 14)+3), 0);
}

void igs009_state::fg_tile_w(offs_t offset, uint8_t data)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void igs009_state::fg_color_w(offs_t offset, uint8_t data)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void igs009_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,  8,  0x80,0x20);
	m_fg_tilemap->set_transparent_pen(0);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_jingbell_reel_tile_info<0>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_jingbell_reel_tile_info<1>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_jingbell_reel_tile_info<2>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_jingbell_reel_tile_info<3>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);

	m_reel_tilemap[0]->set_scroll_cols(128);
	m_reel_tilemap[1]->set_scroll_cols(128);
	m_reel_tilemap[2]->set_scroll_cols(128);
	m_reel_tilemap[3]->set_scroll_cols(128);
}


VIDEO_START_MEMBER(igs009_state,gp98)
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,  8,  0x80,0x20);
	m_fg_tilemap->set_transparent_pen(0);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_gp98_reel_tile_info<0>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_gp98_reel_tile_info<1>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_gp98_reel_tile_info<2>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igs009_state::get_gp98_reel_tile_info<3>)),TILEMAP_SCAN_ROWS,8,32, 128, 8);

	m_reel_tilemap[0]->set_scroll_cols(128);
	m_reel_tilemap[1]->set_scroll_cols(128);
	m_reel_tilemap[2]->set_scroll_cols(128);
	m_reel_tilemap[3]->set_scroll_cols(128);
}


uint32_t igs009_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = m_video_enable ? -1 : 0;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 2;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	if (layers_ctrl & 1)
	{
		int startclipmin = 0;
		const rectangle &visarea = screen.visible_area();

		for (int i= 0;i < 0x80;i++)
		{
			m_reel_tilemap[0]->set_scrolly(i, m_bg_scroll[0][i]*2);
			m_reel_tilemap[1]->set_scrolly(i, m_bg_scroll[0][i+0x80]*2);
			m_reel_tilemap[2]->set_scrolly(i, m_bg_scroll[0][i+0x100]*2);
			m_reel_tilemap[3]->set_scrolly(i, m_bg_scroll[0][i+0x180]*2);
		}

		for (int zz=0;zz<0x80-8;zz++) // -8 because of visible area (2*8 = 16)
		{
			rectangle clip;
			int rowenable = m_bg_scroll[1][zz];

			// draw top of screen
			clip.set(visarea.min_x, visarea.max_x, startclipmin, startclipmin+2);

			bitmap.fill(m_palette->pen(rowenable), clip);

			if (rowenable==0)
			{ // 0 and 1 are the same? or is there a global switchoff?
				m_reel_tilemap[0]->draw(screen, bitmap, clip, 0,0);
			}
			else if (rowenable==1)
			{
				m_reel_tilemap[1]->draw(screen, bitmap, clip, 0,0);
			}
			else if (rowenable==2)
			{
				m_reel_tilemap[2]->draw(screen, bitmap, clip, 0,0);
			}
			else if (rowenable==3)
			{
				m_reel_tilemap[3]->draw(screen, bitmap, clip, 0,0);
			}


			startclipmin+=2;
		}

	}
	else                    bitmap.fill(m_palette->black_pen(), cliprect);


	if (layers_ctrl & 2)    m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/


int igs009_state::hopper_r()
{
	return m_hopper && !(m_screen->frame_number()%10);
}


void igs009_state::show_out()
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x", m_out[0], m_out[1], m_out[2]);
#endif
}

void igs009_state::nmi_and_coins_w(uint8_t data)
{
	if ((m_nmi_enable ^ data) & (~0xdd))
	{
		logerror("PC %06X: nmi_and_coins = %02x\n",m_maincpu->pc(),data);
//      popmessage("%02x",data);
	}

	machine().bookkeeping().coin_counter_w(0,        data & 0x01);   // coin_a
	machine().bookkeeping().coin_counter_w(1,        data & 0x04);   // coin_c
	machine().bookkeeping().coin_counter_w(2,        data & 0x08);   // key in
	machine().bookkeeping().coin_counter_w(3,        data & 0x10);   // coin out mech

	m_leds[6] = BIT(data, 6);   // led for coin out / m_hopper active

	m_nmi_enable = data;    //  data & 0x80     // nmi enable?

	m_out[0] = data;
	show_out();
}

void igs009_state::video_and_leds_w(uint8_t data)
{
	m_leds[4] = BIT(data, 0); // start?
	m_leds[5] = BIT(data, 2); // l_bet?

	m_video_enable  =     data & 0x40;
	m_hopper            =   (~data)& 0x80;

	m_out[1] = data;
	show_out();
}

void igs009_state::leds_w(uint8_t data)
{
	m_leds[0] = BIT(data, 0);  // stop_1
	m_leds[1] = BIT(data, 1);  // stop_2
	m_leds[2] = BIT(data, 2);  // stop_3
	m_leds[3] = BIT(data, 3);  // stop
	// data & 0x10?

	m_out[2] = data;
	show_out();
}


void igs009_state::magic_w(offs_t offset, uint8_t data)
{
	m_igs_magic[offset] = data;

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		case 0x01:
			break;

		default:
//          popmessage("magic %x <- %04x",m_igs_magic[0],data);
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", m_maincpu->pc(), m_igs_magic[0], data);
	}
}

uint8_t igs009_state::magic_r()
{
	switch(m_igs_magic[0])
	{
		case 0x00:
			if ( !(m_igs_magic[1] & 0x01) ) return ioport("DSW1")->read();
			if ( !(m_igs_magic[1] & 0x02) ) return ioport("DSW2")->read();
			if ( !(m_igs_magic[1] & 0x04) ) return ioport("DSW3")->read();
			if ( !(m_igs_magic[1] & 0x08) ) return ioport("DSW4")->read();
			if ( !(m_igs_magic[1] & 0x10) ) return ioport("DSW5")->read();
			logerror("%06x: warning, reading dsw with igs_magic[1] = %02x\n", m_maincpu->pc(), m_igs_magic[1]);
			break;

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", m_maincpu->pc(), m_igs_magic[0]);
	}

	return 0;
}


void igs009_state::jingbell_map(address_map &map)
{
	map(0x00000, 0x0f3ff).rom();
	map(0x0f400, 0x0ffff).ram().share("nvram");
}

void igs009_state::jingbell_portmap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs

	map(0x1000, 0x11ff).ram().w(FUNC(igs009_state::bg_scroll_w)).share(m_bg_scroll[0]);

	map(0x2000, 0x23ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");

	map(0x3000, 0x33ff).ram().w(FUNC(igs009_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0x3400, 0x37ff).ram().w(FUNC(igs009_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0x3800, 0x3bff).ram().w(FUNC(igs009_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0x3c00, 0x3fff).ram().w(FUNC(igs009_state::reel_ram_w<3>)).share(m_reel_ram[3]);

	map(0x4000, 0x407f).ram().share(m_bg_scroll[1]);

	map(0x5000, 0x5fff).ram().w(FUNC(igs009_state::fg_tile_w)).share(m_fg_tile_ram);

	map(0x6480, 0x6483).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    // NMI and coins (w), service (r), coins (r)
	map(0x6490, 0x6493).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    // buttons 1 (r), video and leds (w), leds (w)

	map(0x64a0, 0x64a0).portr("BUTTONS2");

	map(0x64b0, 0x64b1).w("ymsnd", FUNC(ym2413_device::write));

	map(0x64c0, 0x64c0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x64d0, 0x64d1).rw(FUNC(igs009_state::magic_r), FUNC(igs009_state::magic_w));    // DSW1-5

	map(0x7000, 0x7fff).ram().w(FUNC(igs009_state::fg_color_w)).share(m_fg_color_ram);

	map(0x8000, 0xffff).rom().region("data", 0);
}


void igs009_state::gp98_portmap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs

	map(0x1000, 0x11ff).ram().w(FUNC(igs009_state::bg_scroll_w)).share(m_bg_scroll[0]);

	map(0x2000, 0x23ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");

	map(0x3000, 0x33ff).ram().w(FUNC(igs009_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0x3400, 0x37ff).ram().w(FUNC(igs009_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0x3800, 0x3bff).ram().w(FUNC(igs009_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0x3c00, 0x3fff).ram().w(FUNC(igs009_state::reel_ram_w<3>)).share(m_reel_ram[3]);

	map(0x4000, 0x407f).ram().share(m_bg_scroll[1]);

	map(0x5000, 0x5fff).ram().w(FUNC(igs009_state::fg_tile_w)).share(m_fg_tile_ram);

	// seems to lack PPI devices...
	map(0x6480, 0x6480).w(FUNC(igs009_state::nmi_and_coins_w));
	map(0x6481, 0x6481).portr("SERVICE");
	map(0x6482, 0x6482).portr("COINS");
	map(0x6490, 0x6490).portr("BUTTONS1");
	map(0x6491, 0x6491).w(FUNC(igs009_state::video_and_leds_w));
	map(0x6492, 0x6492).w(FUNC(igs009_state::leds_w));
	map(0x64a0, 0x64a0).portr("BUTTONS2");

	map(0x64b0, 0x64b1).w("ymsnd", FUNC(ym2413_device::write));

	map(0x64c0, 0x64c0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x64d0, 0x64d1).rw(FUNC(igs009_state::magic_r), FUNC(igs009_state::magic_w));    // DSW1-5

	map(0x7000, 0x7fff).ram().w(FUNC(igs009_state::fg_color_w)).share(m_fg_color_ram);

	map(0x8000, 0xffff).rom().region("data", 0);
}


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( jingbell )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "W-Up Bonus" )        PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Min Bet" )           PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x08, 0x08, "Spin Speed" )        PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_DIPNAME( 0x10, 0x00, "Strip Girl" )        PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Payout Mode" )       PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0xc0, 0xc0, "Player's Panel" )    PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, "Type A" )
	PORT_DIPSETTING(    0xc0, "Type A" )
	PORT_DIPSETTING(    0x80, "Type B" )
	PORT_DIPSETTING(    0x40, "Type C" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate (%)" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "55" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x03, "75" )
	PORT_DIPSETTING(    0x02, "80" )
	PORT_DIPSETTING(    0x01, "85" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPNAME( 0x38, 0x38, "W-Up Chance (%)" )   PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x38, "93" )
	PORT_DIPSETTING(    0x30, "94" )
	PORT_DIPSETTING(    0x28, "95" )
	PORT_DIPSETTING(    0x20, "96" )
	PORT_DIPSETTING(    0x18, "97" )
	PORT_DIPSETTING(    0x10, "98" )
	PORT_DIPSETTING(    0x08, "99" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Key In Limit" )      PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "1k" )
	PORT_DIPSETTING(    0x80, "3k" )
	PORT_DIPSETTING(    0x40, "5k" )
	PORT_DIPSETTING(    0x00, "10k" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Rate" )       PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x05, "10" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x03, "50" )
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x01, "200" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x38, 0x38, "Coin 1 Rate" )       PORT_DIPLOCATION("DSW3:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x10, "25" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "System Limit" )      PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0xc0, "5k" )
	PORT_DIPSETTING(    0x80, "10k" )
	PORT_DIPSETTING(    0x40, "30k" )
	PORT_DIPSETTING(    0x00, "Unlimited" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Min Play For Fever" )    PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPNAME( 0x02, 0x02, "Max Bet" )           PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x1c, 0x1c, "Coin 2 Rate" )       PORT_DIPLOCATION("DSW4:3,4,5")
	PORT_DIPSETTING(    0x1c, "1" )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" )      PORT_DIPLOCATION("DSW4:6,7")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x80, 0x80, "Play Line" )         PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x00, "16" )

// These are from the manual for v201us - DSW1-DSW4 match but DSW5 doesn't seem to match or actually do anything
	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Play" )      PORT_DIPLOCATION("DSW5:1,2")
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPSETTING(    0x01, "32" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x04, 0x04, "Skill Stop" )        PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPNAME( 0x08, 0x00, "Hands Count" )       PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x08, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x30, 0x00, "Hands Coin Rate" )   PORT_DIPLOCATION("DSW5:5,6")
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPNAME( 0x40, 0x40, "Hands Coin Value" )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPNAME( 0x80, 0x80, "Unused" )            PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0x80, "Off" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )      PORT_NAME("Memory Clear")    // stats, memory
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(igs009_state, hopper_r)  // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )   // test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   PORT_NAME("Records")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1      ) PORT_NAME("Start / Half D-Up Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW  ) PORT_NAME("Small")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1     ) PORT_NAME("Left Bet / 2X D-Up Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2     ) PORT_NAME("Right Bet / D-Up Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{   RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6 =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{   RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( gfx_jingbell )
	GFXDECODE_ENTRY( "reels", 0, layout_8x32x6, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, layout_8x8x6,  0, 16 )
GFXDECODE_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+1, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+1, RGN_FRAC(0,3)+0, RGN_FRAC(0,3)+1 },
	{ 8,10,12,14, 0, 2, 4, 6, },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout tiles8x32_layout =
{
	8,32,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+1, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+1, RGN_FRAC(0,3)+0, RGN_FRAC(0,3)+1 },
	{ 8,10,12,14, 0, 2, 4, 6, },
	{ STEP32(0,16) },
	32*16
};

static GFXDECODE_START( gfx_gp98 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32_layout, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

void igs009_state::machine_start()
{
	m_leds.resolve();


	save_item(NAME(m_video_enable));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_hopper));
	save_item(NAME(m_out));
	save_item(NAME(m_igs_magic));
}

void igs009_state::machine_reset()
{
	m_nmi_enable    =   0;
	m_hopper        =   0;
	m_video_enable  =   1;
}

void igs009_state::vblank_irq(int state)
{
	if (state && BIT(m_nmi_enable, 7))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void igs009_state::jingbell(machine_config &config)
{
	// basic machine hardware
	HD64180RP(config, m_maincpu, XTAL(12'000'000));   // HD64180RP8, 8 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &igs009_state::jingbell_map);
	m_maincpu->set_addrmap(AS_IO, &igs009_state::jingbell_portmap);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.out_pa_callback().set(FUNC(igs009_state::nmi_and_coins_w));
	ppi0.in_pb_callback().set_ioport("SERVICE");
	ppi0.in_pc_callback().set_ioport("COINS");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("BUTTONS1");
	ppi1.out_pb_callback().set(FUNC(igs009_state::video_and_leds_w));
	ppi1.out_pc_callback().set(FUNC(igs009_state::leds_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-16-1);
	m_screen->set_screen_update(FUNC(igs009_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(igs009_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jingbell);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(12'000'000) / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void igs009_state::gp98(machine_config &config)
{
	jingbell(config);
	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &igs009_state::gp98_portmap);

	m_gfxdecode->set_info(gfx_gp98);

	MCFG_VIDEO_START_OVERRIDE(igs009_state,gp98)
}


/***************************************************************************

  Jingle Bell
  IGS, 1997.

  English versions.

  1x HD64180RP8 (u18)
  2x NEC D8255AC-2 (u19, u20)
  1x custom IGS 009 F56D 246 (u22)
  1x YM2413 (u45)
  1x AR17961-AP0640 (u46)(sound equivalent to OKI M6295)

  4x 27C512 (1, 2, 3, V)
  3x 27C2001 (4, 5, 6)
  1x 27C256 (7)
  1x 27C1001 (sp)

  1x UMC UM6264B-10L (u42)
  2x UMC UM6164DK-12 (u1, u2)

  1x crystal 12.000 MHz.
  1x crystal 3.579545 MHz.

  1x 38x2 edge connector.
  2x 10x2 edge connectors.
  1x switch.
  1x 3.6 V. lithium battery.
  5x 8 DIP switches banks.

***************************************************************************/

/* Jingle Bells (V157 US)
   Original IGS.
   For amusement.
*/
ROM_START( jbell157us )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v157_us.u44", 0x00000, 0x10000, CRC(37f95b60) SHA1(db2022802ce553ce7a0e8403408f3194b2f10007) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "27c256_v157_us.u43", 0x0000, 0x8000, CRC(a7affa15) SHA1(f9d33e32b57ad267d383e075663994e0af0b3016) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "27c512_v157_us.u17", 0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "27c512_v157_us.u16", 0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "27c512_v157_us.u15", 0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "27c2001_v157_us.u25", 0x00000, 0x40000, CRC(daa56ce5) SHA1(4f14a8efac16b03bd14dd26d586bcb8d5bef65c1) )
	ROM_LOAD( "27c2001_v157_us.u24", 0x40000, 0x40000, CRC(b10b38e1) SHA1(397b2d899e47c6249fbbb6e6262d0390d9b796e6) )
	ROM_LOAD( "27c2001_v157_us.u23", 0x80000, 0x40000, CRC(a3304b5a) SHA1(bf51cb1f728758d50ce27275aa19ef649f6b34b9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx29f001t_v157_us_sp.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )
ROM_END

/* Jingle Bells (V155 EU)
   Original IGS.
   For amusement.
*/
ROM_START( jbell155ue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v155.u44", 0x00000, 0x10000, CRC(0813d12b) SHA1(4c35b656d1e54619bbfd26cbc017eba336e6abda) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "27c256_v155.u43", 0x0000, 0x8000, CRC(a7affa15) SHA1(f9d33e32b57ad267d383e075663994e0af0b3016) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "27c512_v155.u17", 0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "27c512_v155.u16", 0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "27c512_v155.u15", 0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "27c2001_v155.u25", 0x00000, 0x40000, CRC(daa56ce5) SHA1(4f14a8efac16b03bd14dd26d586bcb8d5bef65c1) )
	ROM_LOAD( "27c2001_v155.u24", 0x40000, 0x40000, CRC(b10b38e1) SHA1(397b2d899e47c6249fbbb6e6262d0390d9b796e6) )
	ROM_LOAD( "27c2001_v155.u23", 0x80000, 0x40000, CRC(a3304b5a) SHA1(bf51cb1f728758d50ce27275aa19ef649f6b34b9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c1001_v155_sp.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )
ROM_END

/* Jingle Bells (V153 EU)
   Original IGS.
   For amusement.
*/
ROM_START( jbell153ue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v153.u44", 0x00000, 0x10000, CRC(fd3bc092) SHA1(a92dc9bc15d7a4816887d66d46cea925d230c5b8) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "27c256_v153.u43", 0x0000, 0x8000, CRC(a7affa15) SHA1(f9d33e32b57ad267d383e075663994e0af0b3016) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "27c512_v153.u17", 0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "27c512_v153.u16", 0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "27c512_v153.u15", 0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "27c2001_v153.u25", 0x00000, 0x40000, CRC(daa56ce5) SHA1(4f14a8efac16b03bd14dd26d586bcb8d5bef65c1) )
	ROM_LOAD( "27c2001_v153.u24", 0x40000, 0x40000, CRC(b10b38e1) SHA1(397b2d899e47c6249fbbb6e6262d0390d9b796e6) )
	ROM_LOAD( "27c2001_v153.u23", 0x80000, 0x40000, CRC(a3304b5a) SHA1(bf51cb1f728758d50ce27275aa19ef649f6b34b9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c1001_v153_sp.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )
ROM_END

/* Jingle Bells (V141 EU)
   Original IGS.
   For amusement.
*/
ROM_START( jbell141ue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v141.u44", 0x00000, 0x10000, CRC(6997a903) SHA1(991508d85e55a3c6f811070352e06ae6bf65cc2e) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "27c256_v141.u43", 0x0000, 0x8000, CRC(a7affa15) SHA1(f9d33e32b57ad267d383e075663994e0af0b3016) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "27c512_v141.u17", 0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "27c512_v141.u16", 0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "27c512_v141.u15", 0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "27c2001_v141.u25", 0x00000, 0x40000, CRC(f53bac7e) SHA1(f4375da0780fba59fcb65e24a33099af35e4d286) )
	ROM_LOAD( "27c2001_v141.u24", 0x40000, 0x40000, CRC(bddd6001) SHA1(2a6395e9593352d3ea2d477a7f41805f389c9c50) )
	ROM_LOAD( "27c2001_v141.u23", 0x80000, 0x40000, CRC(e8322c75) SHA1(c3385538fb673a4ab14c315ce8bce792eb264ec7) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c1001_v141_sp.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )
ROM_END

/***************************************************************************

Jingle Bell
Italy, V133I
(C) IGS ("COPYRIGHT 1995" in ROM, "FEB. 23 1998" on sticker)

CPU:
    1x HD64180RP8 (u18)(main)
    2x NEC D8255AC (u19,u20)(main)
    1x custom IGS009-F56D246 (u22)
    1x U3567HX881 (u45)(sound equivalent to ym2413)
    1x AR17961-AP0848 (u46)(sound equivalent to m6295)
    1x oscillator 12.000
    1x oscillator 3.579545

ROMs:
    3x M27C512 (1,2,3)
    1x LE27C2001F (4)
    2x MX27C2000 (5,6)
    1x D27256 (7)
    1x MX27C512 (v)
    1x NM27C010 (sp)
    2x PALCE16V8H (read protected)
    1x PALCE22V10H (read protected)
    1x PALCE22V10H (dumped)

Notes:
    1x 38x2 edge connector
    1x 10x2 edge connector
    1x pushbutton
    1x battery
    5x 8x2 switches dip

12/02/2008 f205v

***************************************************************************/

ROM_START( jbell133i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jinglev133i.u44", 0x00000, 0x10000, CRC(df60dc39) SHA1(ff57afd50c045b621395353fdc50ffd1e1b65e9e) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "jingle133i7.u43", 0x0000, 0x8000, CRC(a7affa15) SHA1(f9d33e32b57ad267d383e075663994e0af0b3016) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "jingle133i1.u17", 0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "jingle133i2.u16", 0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "jingle133i3.u15", 0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "jingle133i4.u25", 0x00000, 0x40000, CRC(7aa1d344) SHA1(141e27df93cb35ab852d9022e0b08bd596f1186b) )
	ROM_LOAD( "jingle133i5.u24", 0x40000, 0x40000, CRC(021261d1) SHA1(5b23f9bd818193c343f9f4c9317955b17efb8cfa) )
	ROM_LOAD( "jingle133i6.u23", 0x80000, 0x40000, CRC(c40228fd) SHA1(4dc05337d64ed2b8d66fc5f0ca8ffbf96799f768) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "jingle133isp.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )

	ROM_REGION( 0x2dd, "plds",0 )
	ROM_LOAD( "palce16v8h-ch-jin-u12v.u12", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h-ch-jin-u33v.u33", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce22v10h-ajbu24.u24",     0x000, 0x2dd, CRC(6310f441) SHA1(b610e170ccca1fcb06a57f718ece1408b696ba9c) )
	ROM_LOAD( "palce22v10h-ch-jin-u27.u27", 0x000, 0x2dd, NO_DUMP )
ROM_END

/* Animal House.
   Recording from the real hardware: https://youtu.be/ibd8_nsklTY */
ROM_START( animalhjb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.u44",     0x00000, 0x10000, CRC(d11d191f) SHA1(113e22d69a2b8ceb213d72ec8cee021b1a5507e5) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "tms27c256.u43",  0x00000, 0x08000, CRC(af67f687) SHA1(8f43a693358612880389b238ec7040f78b0164bb) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "am27c512.u17",   0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "am27c512.u16",   0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "am27c512.u15",   0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )
	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "at27c020.u25",   0x00000, 0x40000, CRC(5f8abeaf) SHA1(99c4a795cb9b4d94867c12ca99cba04f9c05e129) )
	ROM_LOAD( "at27c020.u24",   0x40000, 0x40000, CRC(58efe5a8) SHA1(fb4cba3965052e5cdd24a7e93966c597855caa68) )
	ROM_LOAD( "at27c020.u23",   0x80000, 0x40000, CRC(e6249be1) SHA1(f49383c587061a8a5531381dc80c8ebd7c94d61d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "tms27c010a.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )

	ROM_REGION( 0x2dd, "plds", 0 )
	ROM_LOAD( "gal16v8d.u12",   0x00000, 0x00117, CRC(b59340ed) SHA1(8678f2efedf7d17aec18b6ffeb0d4ef6b943df3c) )
	ROM_LOAD( "gal16v8d.u33",   0x00000, 0x00117, CRC(83547e35) SHA1(a8d3622905cbd54af39c01070048f07be1b0257a) )
	ROM_LOAD( "pal22v10.u26",   0x00000, 0x002dd, CRC(808381b5) SHA1(15802edd7d28ca4a73fb4d7757c80040393daf8d) )
	ROM_LOAD( "pal22v10.u27",   0x00000, 0x002dd, CRC(848a42fc) SHA1(2cfeecbf934b81e9fbf3018efc00cc431a14b266) )
ROM_END

void igs009_state::decrypt_jingbell()
{
	uint8_t *rom  = (uint8_t *)memregion("maincpu")->base();
	size_t size = memregion("maincpu")->bytes();

	for (int i=0; i<size; i++)
	{
		uint8_t x = rom[i];
		if (i & 0x0080)
		{
			if ((i & 0x0420) == 0x0420) x ^= 0x20;
			else                        x ^= 0x22;
		}
		else
		{
			if (i & 0x0200) x ^= 0x02;
			else            x ^= 0x22;
		}

		if ((i & 0x1208) == 0x1208) x ^= 0x01;

		rom[i] = x;
	}
}

void igs009_state::init_jingbelli()
{
	decrypt_jingbell();

	// protection patch
	uint8_t *rom  = (uint8_t *)memregion("maincpu")->base();
	rom[0x01f19] = 0x18;
}

void igs009_state::init_animalhjb()
{
	decrypt_jingbell();

	// protection patch
	uint8_t *rom  = (uint8_t *)memregion("maincpu")->base();
	rom[0x01f21] = 0x18;
}

void igs009_state::init_jingbell()
{
	decrypt_jingbell();

	// protection patch
	uint8_t *rom  = (uint8_t *)memregion("maincpu")->base();
	rom[0x0e753] = 0x18;
}

/***************************************************************************

Grand Prix '98

PCB Layout
----------

|-----------------------------------------|
|                 YM2413       DSW2  DSW4 |
|                 3.579545MHz             |
|   Z180                       DSW1  DSW3 |
|                 PAL                     |
|     PRG                                 |
|   12MHz                                 |
|J                   51        |-------|  |
|A    6264                     |PLCC84 |  |
|M                   50        |FPGA   |  |
|M                             |       |  |
|A                   49        |-------|  |
|                                         |
|                                         |
|                                         |
|                   6264                  |
|                                         |
|          62256                          |
|-----------------------------------------|
Z180 @ 12MHz
YM2413 @ 3.579545MHz
VSync 60Hz
HSync 15.35kHz

***************************************************************************/

ROM_START( gp98 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prg", 0x00000, 0x20000, CRC(1c02b8cc) SHA1(b8a29cbd96581f8ae1c1028279b8ee703be29f5f) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_COPY( "maincpu", 0x18000, 0x00000, 0x8000 )

	ROM_REGION( 0x180000, "tempgfx", 0 ) // 6bpp (2bpp per rom) font at tile # 0x4000
	ROM_LOAD( "em-03.u49", 0x000000, 0x80000, CRC(f92c510d) SHA1(f8dc4d7d1fdc6f62fcdd86caf8fd703db4b5fb18) )
	ROM_LOAD( "em-02.u50", 0x080000, 0x80000, CRC(48f6190d) SHA1(b430131a258b4e2fc178ac0e3e3f0010a82eac65) )
	ROM_LOAD( "em-01.u51", 0x100000, 0x80000, CRC(30a2ef85) SHA1(38ea637acd83b175eccd2969ef21879265b88992) )

	ROM_REGION( 0xc0000, "reels", 0 )
	ROM_COPY( "tempgfx", 0x000000, 0x00000, 0x40000 )
	ROM_COPY( "tempgfx", 0x080000, 0x40000, 0x40000 )
	ROM_COPY( "tempgfx", 0x100000, 0x80000, 0x40000 )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_COPY( "tempgfx", 0x040000, 0x00000, 0x40000 )
	ROM_COPY( "tempgfx", 0x0c0000, 0x40000, 0x40000 )
	ROM_COPY( "tempgfx", 0x140000, 0x80000, 0x40000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	// no OKI on this
ROM_END

// Real PCB recording for reference: https://youtu.be/ydMbv90kIXQ
ROM_START( gp98a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27c010a.u2",   0x000000, 0x20000, CRC(6f22bad1) SHA1(17df88ef3dbf2f44fd4f6f3a056e71db323217c7) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_COPY( "maincpu",      0x018000, 0x00000, 0x8000 )

	ROM_REGION( 0x180000, "tempgfx", 0 ) // 6bpp (2bpp per rom) font at tile # 0x4000
	ROM_LOAD( "m27c4001.u49", 0x000000, 0x80000, CRC(f92c510d) SHA1(f8dc4d7d1fdc6f62fcdd86caf8fd703db4b5fb18) )
	ROM_LOAD( "m27c4001.u50", 0x080000, 0x80000, CRC(48f6190d) SHA1(b430131a258b4e2fc178ac0e3e3f0010a82eac65) )
	ROM_LOAD( "m27c4001.u51", 0x100000, 0x80000, CRC(30a2ef85) SHA1(38ea637acd83b175eccd2969ef21879265b88992) )

	ROM_REGION( 0xc0000, "reels", 0 )
	ROM_COPY( "tempgfx",      0x000000, 0x00000, 0x40000 )
	ROM_COPY( "tempgfx",      0x080000, 0x40000, 0x40000 )
	ROM_COPY( "tempgfx",      0x100000, 0x80000, 0x40000 )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_COPY( "tempgfx",      0x040000, 0x00000, 0x40000 )
	ROM_COPY( "tempgfx",      0x0c0000, 0x40000, 0x40000 )
	ROM_COPY( "tempgfx",      0x140000, 0x80000, 0x40000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	// no OKI on this

	ROM_REGION( 0x26e, "plds", 0 )
	ROM_LOAD( "palce20v8.u7", 0x000000, 0x00157, NO_DUMP )
	ROM_LOAD( "gal16v8d.u8",  0x000157, 0x00117, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR   NAME        PARENT  MACHINE   INPUT     STATE          INIT            ROT   COMPANY           FULLNAME                                 FLAGS
GAME( 1997,  jbell157us, jbell,  jingbell, jingbell, igs009_state,  init_jingbell,  ROT0, "IGS",            "Jingle Bell (US, V157US)",              MACHINE_SUPPORTS_SAVE )
GAME( 1997,  jbell155ue, jbell,  jingbell, jingbell, igs009_state,  init_jingbell,  ROT0, "IGS",            "Jingle Bell (EU, V155UE)",              MACHINE_SUPPORTS_SAVE ) // Shows V154UE in test mode!
GAME( 1997,  jbell153ue, jbell,  jingbell, jingbell, igs009_state,  init_jingbell,  ROT0, "IGS",            "Jingle Bell (EU, V153UE)",              MACHINE_SUPPORTS_SAVE )
GAME( 1995,  jbell141ue, jbell,  jingbell, jingbell, igs009_state,  init_jingbelli, ROT0, "IGS",            "Jingle Bell (EU, V141UE)",              MACHINE_SUPPORTS_SAVE )
GAME( 1995?, jbell133i,  jbell,  jingbell, jingbell, igs009_state,  init_jingbelli, ROT0, "IGS",            "Jingle Bell (Italy, V133I)",            MACHINE_SUPPORTS_SAVE )
GAME( 1995?, animalhjb,  jbell,  jingbell, jingbell, igs009_state,  init_animalhjb, ROT0, "bootleg",        "Animal House (bootleg of Jingle Bell)", MACHINE_SUPPORTS_SAVE )
GAME( 1998,  gp98,       0,      gp98,     jingbell, igs009_state,  empty_init,     ROT0, "Romtec Co. Ltd", "Grand Prix '98 (V100K, set 1)",         MACHINE_SUPPORTS_SAVE )
GAME( 1998,  gp98a,      gp98,   gp98,     jingbell, igs009_state,  empty_init,     ROT0, "Romtec Co. Ltd", "Grand Prix '98 (V100K, set 2)",         MACHINE_SUPPORTS_SAVE ) // "V100K JINGLEBELL" string on program ROM
