// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
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

----------------------------------------------------------------------
Year  Game                Manufacturer    Notes
----------------------------------------------------------------------
1995? Jingle Bell         IGS
1998  Grand Prix '98      Romtec          1 reel gfx rom is bad
----------------------------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


class igs009_state : public driver_device
{
public:
	igs009_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_bg_scroll(*this, "bg_scroll"),
		m_reel1_ram(*this, "reel1_ram"),
		m_reel2_ram(*this, "reel2_ram"),
		m_reel3_ram(*this, "reel3_ram"),
		m_reel4_ram(*this, "reel4_ram"),
		m_bg_scroll2(*this, "bg_scroll2"),
		m_fg_tile_ram(*this, "fg_tile_ram"),
		m_fg_color_ram(*this, "fg_color_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_bg_scroll;
	required_shared_ptr<UINT8> m_reel1_ram;
	required_shared_ptr<UINT8> m_reel2_ram;
	required_shared_ptr<UINT8> m_reel3_ram;
	required_shared_ptr<UINT8> m_reel4_ram;
	required_shared_ptr<UINT8> m_bg_scroll2;
	required_shared_ptr<UINT8> m_fg_tile_ram;
	required_shared_ptr<UINT8> m_fg_color_ram;

	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;
	tilemap_t *m_reel4_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_video_enable;
	int m_nmi_enable;
	int m_hopper;
	UINT8 m_out[3];
	UINT8 m_igs_magic[2];

	DECLARE_WRITE8_MEMBER(reel1_ram_w);
	DECLARE_WRITE8_MEMBER(reel2_ram_w);
	DECLARE_WRITE8_MEMBER(reel3_ram_w);
	DECLARE_WRITE8_MEMBER(reel4_ram_w);
	DECLARE_WRITE8_MEMBER(bg_scroll_w);
	DECLARE_WRITE8_MEMBER(fg_tile_w);
	DECLARE_WRITE8_MEMBER(fg_color_w);
	DECLARE_WRITE8_MEMBER(nmi_and_coins_w);
	DECLARE_WRITE8_MEMBER(video_and_leds_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(magic_w);
	DECLARE_READ8_MEMBER(magic_r);

	void show_out();
	DECLARE_CUSTOM_INPUT_MEMBER(hopper_r);
	INTERRUPT_GEN_MEMBER(interrupt);

	TILE_GET_INFO_MEMBER(get_jingbell_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_gp98_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_jingbell_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_gp98_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_jingbell_reel3_tile_info);
	TILE_GET_INFO_MEMBER(get_gp98_reel3_tile_info);
	TILE_GET_INFO_MEMBER(get_jingbell_reel4_tile_info);
	TILE_GET_INFO_MEMBER(get_gp98_reel4_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	DECLARE_DRIVER_INIT(jingbell);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(gp98);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/***************************************************************************
                                Video Hardware
***************************************************************************/



WRITE8_MEMBER(igs009_state::reel1_ram_w)
{
	m_reel1_ram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(igs009_state::get_jingbell_reel1_tile_info)
{
	int code = m_reel1_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code)+(((tile_index+1)&0x3)*0x100),
			(code & 0x80) ? 0xc : 0,
			0);
}


TILE_GET_INFO_MEMBER(igs009_state::get_gp98_reel1_tile_info)
{
	int code = m_reel1_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code*4)+(tile_index&0x3),
			0,
			0);
}


WRITE8_MEMBER(igs009_state::reel2_ram_w)
{
	m_reel2_ram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(igs009_state::get_jingbell_reel2_tile_info)
{
	int code = m_reel2_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code)+(((tile_index+1)&0x3)*0x100),
			(code & 0x80) ? 0xc : 0,
			0);
}

TILE_GET_INFO_MEMBER(igs009_state::get_gp98_reel2_tile_info)
{
	int code = m_reel2_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code*4)+(tile_index&0x3),
			0,
			0);
}



WRITE8_MEMBER(igs009_state::reel3_ram_w)
{
	m_reel3_ram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(igs009_state::get_jingbell_reel3_tile_info)
{
	int code = m_reel3_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code)+(((tile_index+1)&0x3)*0x100),
			(code & 0x80) ? 0xc : 0,
			0);
}

TILE_GET_INFO_MEMBER(igs009_state::get_gp98_reel3_tile_info)
{
	int code = m_reel3_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code*4)+(tile_index&0x3),
			0,
			0);
}



WRITE8_MEMBER(igs009_state::reel4_ram_w)
{
	m_reel4_ram[offset] = data;
	m_reel4_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(igs009_state::get_jingbell_reel4_tile_info)
{
	int code = m_reel4_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code)+(((tile_index+1)&0x3)*0x100),
			(code & 0x80) ? 0xc : 0,
			0);
}

TILE_GET_INFO_MEMBER(igs009_state::get_gp98_reel4_tile_info)
{
	int code = m_reel4_ram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code*4)+(tile_index&0x3),
			0,
			0);
}







WRITE8_MEMBER(igs009_state::bg_scroll_w)
{
	m_bg_scroll[offset] = data;
//  bg_tilemap->set_scrolly(offset,data);
}


TILE_GET_INFO_MEMBER(igs009_state::get_fg_tile_info)
{
	int code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	SET_TILE_INFO_MEMBER(1, code, (4*(code >> 14)+3), 0);
}

WRITE8_MEMBER(igs009_state::fg_tile_w)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(igs009_state::fg_color_w)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void igs009_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8,  8,  0x80,0x20);
	m_fg_tilemap->set_transparent_pen(0);

	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_jingbell_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_jingbell_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_jingbell_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel4_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_jingbell_reel4_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);

	m_reel1_tilemap->set_scroll_cols(128);
	m_reel2_tilemap->set_scroll_cols(128);
	m_reel3_tilemap->set_scroll_cols(128);
	m_reel4_tilemap->set_scroll_cols(128);
}


VIDEO_START_MEMBER(igs009_state,gp98)
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8,  8,  0x80,0x20);
	m_fg_tilemap->set_transparent_pen(0);

	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_gp98_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_gp98_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_gp98_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);
	m_reel4_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(igs009_state::get_gp98_reel4_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 128, 8);

	m_reel1_tilemap->set_scroll_cols(128);
	m_reel2_tilemap->set_scroll_cols(128);
	m_reel3_tilemap->set_scroll_cols(128);
	m_reel4_tilemap->set_scroll_cols(128);
}


UINT32 igs009_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
		int zz,i;
		int startclipmin = 0;
		const rectangle &visarea = screen.visible_area();


		for (i= 0;i < 0x80;i++)
		{
			m_reel1_tilemap->set_scrolly(i, m_bg_scroll[i]*2);
			m_reel2_tilemap->set_scrolly(i, m_bg_scroll[i+0x80]*2);
			m_reel3_tilemap->set_scrolly(i, m_bg_scroll[i+0x100]*2);
			m_reel4_tilemap->set_scrolly(i, m_bg_scroll[i+0x180]*2);
		}




		for (zz=0;zz<0x80-8;zz++) // -8 because of visible area (2*8 = 16)
		{
			rectangle clip;
			int rowenable = m_bg_scroll2[zz];

			/* draw top of screen */
			clip.set(visarea.min_x, visarea.max_x, startclipmin, startclipmin+2);

			bitmap.fill(m_palette->pen(rowenable), clip);

			if (rowenable==0)
			{ // 0 and 1 are the same? or is there a global switchoff?
				m_reel1_tilemap->draw(screen, bitmap, clip, 0,0);
			}
			else if (rowenable==1)
			{
				m_reel2_tilemap->draw(screen, bitmap, clip, 0,0);
			}
			else if (rowenable==2)
			{
				m_reel3_tilemap->draw(screen, bitmap, clip, 0,0);
			}
			else if (rowenable==3)
			{
				m_reel4_tilemap->draw(screen, bitmap, clip, 0,0);
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


CUSTOM_INPUT_MEMBER(igs009_state::hopper_r)
{
	return m_hopper && !(m_screen->frame_number()%10);
}


void igs009_state::show_out()
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x", m_out[0], m_out[1], m_out[2]);
#endif
}

WRITE8_MEMBER(igs009_state::nmi_and_coins_w)
{
	if ((m_nmi_enable ^ data) & (~0xdd))
	{
		logerror("PC %06X: nmi_and_coins = %02x\n",space.device().safe_pc(),data);
//      popmessage("%02x",data);
	}

	coin_counter_w(machine(), 0,        data & 0x01);   // coin_a
	coin_counter_w(machine(), 1,        data & 0x04);   // coin_c
	coin_counter_w(machine(), 2,        data & 0x08);   // key in
	coin_counter_w(machine(), 3,        data & 0x10);   // coin m_out mech

	set_led_status(machine(), 6,        data & 0x40);   // led for coin m_out / m_hopper active

	m_nmi_enable = data;    //  data & 0x80     // nmi enable?

	m_out[0] = data;
	show_out();
}

WRITE8_MEMBER(igs009_state::video_and_leds_w)
{
	set_led_status(machine(), 4,      data & 0x01); // start?
	set_led_status(machine(), 5,      data & 0x04); // l_bet?

	m_video_enable  =     data & 0x40;
	m_hopper            =   (~data)& 0x80;

	m_out[1] = data;
	show_out();
}

WRITE8_MEMBER(igs009_state::leds_w)
{
	set_led_status(machine(), 0, data & 0x01);  // stop_1
	set_led_status(machine(), 1, data & 0x02);  // stop_2
	set_led_status(machine(), 2, data & 0x04);  // stop_3
	set_led_status(machine(), 3, data & 0x08);  // stop
	// data & 0x10?

	m_out[2] = data;
	show_out();
}


WRITE8_MEMBER(igs009_state::magic_w)
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
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", space.device().safe_pc(), m_igs_magic[0], data);
	}
}

READ8_MEMBER(igs009_state::magic_r)
{
	switch(m_igs_magic[0])
	{
		case 0x00:
			if ( !(m_igs_magic[1] & 0x01) ) return ioport("DSW1")->read();
			if ( !(m_igs_magic[1] & 0x02) ) return ioport("DSW2")->read();
			if ( !(m_igs_magic[1] & 0x04) ) return ioport("DSW3")->read();
			if ( !(m_igs_magic[1] & 0x08) ) return ioport("DSW4")->read();
			if ( !(m_igs_magic[1] & 0x10) ) return ioport("DSW5")->read();
			logerror("%06x: warning, reading dsw with igs_magic[1] = %02x\n", space.device().safe_pc(), m_igs_magic[1]);
			break;

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", space.device().safe_pc(), m_igs_magic[0]);
	}

	return 0;
}




static ADDRESS_MAP_START( jingbell_map, AS_PROGRAM, 8, igs009_state )
	AM_RANGE( 0x00000, 0x0f3ff ) AM_ROM
	AM_RANGE( 0x0f400, 0x0ffff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( jingbell_portmap, AS_IO, 8, igs009_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs

	AM_RANGE( 0x1000, 0x11ff ) AM_RAM_WRITE(bg_scroll_w ) AM_SHARE("bg_scroll")

	AM_RANGE( 0x2000, 0x23ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x2400, 0x27ff ) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")

	AM_RANGE( 0x3000, 0x33ff ) AM_RAM_WRITE(reel1_ram_w )  AM_SHARE("reel1_ram")
	AM_RANGE( 0x3400, 0x37ff ) AM_RAM_WRITE(reel2_ram_w )  AM_SHARE("reel2_ram")
	AM_RANGE( 0x3800, 0x3bff ) AM_RAM_WRITE(reel3_ram_w )  AM_SHARE("reel3_ram")
	AM_RANGE( 0x3c00, 0x3fff ) AM_RAM_WRITE(reel4_ram_w )  AM_SHARE("reel4_ram")

	AM_RANGE( 0x4000, 0x407f ) AM_RAM AM_SHARE("bg_scroll2")

	AM_RANGE( 0x5000, 0x5fff ) AM_RAM_WRITE(fg_tile_w )  AM_SHARE("fg_tile_ram")

	AM_RANGE( 0x6480, 0x6480 ) AM_WRITE(nmi_and_coins_w )

	AM_RANGE( 0x6481, 0x6481 ) AM_READ_PORT( "SERVICE" )
	AM_RANGE( 0x6482, 0x6482 ) AM_READ_PORT( "COINS" )
	AM_RANGE( 0x6490, 0x6490 ) AM_READ_PORT( "BUTTONS1" )
	AM_RANGE( 0x6491, 0x6491 ) AM_WRITE(video_and_leds_w )
	AM_RANGE( 0x6492, 0x6492 ) AM_WRITE(leds_w )
	AM_RANGE( 0x64a0, 0x64a0 ) AM_READ_PORT( "BUTTONS2" )

	AM_RANGE( 0x64b0, 0x64b1 ) AM_DEVWRITE("ymsnd", ym2413_device, write)

	AM_RANGE( 0x64c0, 0x64c0 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE( 0x64d0, 0x64d1 ) AM_READWRITE(magic_r, magic_w )    // DSW1-5

	AM_RANGE( 0x7000, 0x7fff ) AM_RAM_WRITE(fg_color_w ) AM_SHARE("fg_color_ram")

	AM_RANGE( 0x8000, 0xffff ) AM_ROM AM_REGION("data", 0)
ADDRESS_MAP_END


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

// These are from the manual for v201us - DSW1-DSW4 match but DSW5 doesn't seem to match or actuallly do anything
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF,igs009_state,hopper_r, (void *)nullptr )  // hopper sensor
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
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
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
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( jingbell )
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

static GFXDECODE_START( gp98 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32_layout, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

void igs009_state::machine_start()
{
	save_item(NAME(m_video_enable));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_hopper));
	save_item(NAME(m_out));
	save_item(NAME(m_igs_magic));
}

void igs009_state::machine_reset()
{
	m_nmi_enable        =   0;
	m_hopper            =   0;
	m_video_enable  =   1;
}

INTERRUPT_GEN_MEMBER(igs009_state::interrupt)
{
		if (m_nmi_enable & 0x80)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( jingbell, igs009_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)   /* HD64180RP8, 8 MHz? */
	MCFG_CPU_PROGRAM_MAP(jingbell_map)
	MCFG_CPU_IO_MAP(jingbell_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", igs009_state, interrupt)


	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs009_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jingbell)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_12MHz / 12, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gp98, jingbell )
	MCFG_GFXDECODE_MODIFY("gfxdecode", gp98)

	MCFG_VIDEO_START_OVERRIDE(igs009_state,gp98)
MACHINE_CONFIG_END


/***************************************************************************

Jingle Bell
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

ROM_START( jingbell )
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
	ROM_LOAD( "palce16v8h-ch-jin-u12v.u12", 0x000, 0x117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v8h-ch-jin-u33v.u33", 0x000, 0x117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce22v10h-ajbu24.u24",     0x000, 0x2dd, CRC(6310f441) SHA1(b610e170ccca1fcb06a57f718ece1408b696ba9c) )
	ROM_LOAD( "palce22v10h-ch-jin-u27.u27", 0x000, 0x2dd, BAD_DUMP CRC(5c4e9024) SHA1(e9d1e4df3d79c21f4ce053a84bb7b7a43d650f91) )
ROM_END

DRIVER_INIT_MEMBER(igs009_state,jingbell)
{
	int i;
	UINT8 *rom  = (UINT8 *)memregion("maincpu")->base();
	size_t size = memregion("maincpu")->bytes();

	for (i=0; i<size; i++)
	{
		UINT8 x = rom[i];
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

	// protection patch
	rom[0x01f19] = 0x18;
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
	ROM_LOAD( "49", 0x000000, 0x80000, BAD_DUMP CRC(a9d9367d) SHA1(91c74740fc8394f1e1cd68feb8c993afd2042d70) )
	ROM_LOAD( "50", 0x080000, 0x80000, CRC(48f6190d) SHA1(b430131a258b4e2fc178ac0e3e3f0010a82eac65) )
	ROM_LOAD( "51", 0x100000, 0x80000, CRC(30a2ef85) SHA1(38ea637acd83b175eccd2969ef21879265b88992) )

	ROM_REGION( 0xc0000, "reels", 0 )
	ROM_COPY( "tempgfx", nullptr, 0x00000, 0x40000 )
	ROM_COPY( "tempgfx", 0x080000, 0x40000, 0x40000 )
	ROM_COPY( "tempgfx", 0x100000, 0x80000, 0x40000 )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_COPY( "tempgfx", 0x040000, 0x00000, 0x40000 )
	ROM_COPY( "tempgfx", 0x0c0000, 0x40000, 0x40000 )
	ROM_COPY( "tempgfx", 0x140000, 0x80000, 0x40000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	// no OKI on this
ROM_END

GAME( 1995?, jingbell, 0, jingbell, jingbell, igs009_state, jingbell, ROT0, "IGS",            "Jingle Bell (Italy, V133I)", MACHINE_SUPPORTS_SAVE )
GAME( 1998,  gp98,     0, gp98,     jingbell, driver_device, 0,        ROT0, "Romtec Co. Ltd", "Grand Prix '98 (V100K)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
