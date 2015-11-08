// license:BSD-3-Clause
// copyright-holders:David Haywood, Mirko Buffoni
/*
Happy Jackie (c) 1993 IGS.
Video Slot machine game for amusement only.

Driver by David Haywood and Mirko Buffoni
*/
/*

Anno    199x
Produttore  IGS
N.revisione

CPU

1x Z0840006PSC (main)
2x D8255AC
1x unknown AMT001
1x unknown IGS002
1x UM3567 (sound)
1x oscillator 12.000MHz
1x oscillator 3.579645

ROMs

2x D27128A (1,3)
1x MBM27128 (2)
3x 27C010 (4,5,6)
1x D27512 (7sv)
1x MBM27C512 (v110)
1x unknown (DIP20 mil300)(jack3)
3x PEEL18CV8PC (read protected)
1x TIBPAL16L8 (read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector (payout system)
1x trimmer (volume)
1x pushbutton
1x battery
5x 8x2 switches dip

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2413intf.h"


class jackie_state : public driver_device
{
public:
	jackie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_bg_scroll2(*this, "bg_scroll2"),
		m_bg_scroll(*this, "bg_scroll"),
		m_reel1_ram(*this, "reel1_ram"),
		m_reel2_ram(*this, "reel2_ram"),
		m_reel3_ram(*this, "reel3_ram"),
		m_fg_tile_ram(*this, "fg_tile_ram"),
		m_fg_color_ram(*this, "fg_color_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_bg_scroll2;
	required_shared_ptr<UINT8> m_bg_scroll;
	required_shared_ptr<UINT8> m_reel1_ram;
	required_shared_ptr<UINT8> m_reel2_ram;
	required_shared_ptr<UINT8> m_reel3_ram;
	required_shared_ptr<UINT8> m_fg_tile_ram;
	required_shared_ptr<UINT8> m_fg_color_ram;

	int m_exp_bank;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;
	int m_irq_enable;
	int m_nmi_enable;
	int m_bg_enable;
	int m_hopper;
	UINT8 m_out[3];
	UINT16 m_unk_reg[3][5];

	DECLARE_WRITE8_MEMBER(fg_tile_w);
	DECLARE_WRITE8_MEMBER(fg_color_w);
	DECLARE_WRITE8_MEMBER(bg_scroll_w);
	DECLARE_WRITE8_MEMBER(reel1_ram_w);
	DECLARE_WRITE8_MEMBER(reel2_ram_w);
	DECLARE_WRITE8_MEMBER(reel3_ram_w);
	DECLARE_WRITE8_MEMBER(unk_reg1_lo_w);
	DECLARE_WRITE8_MEMBER(unk_reg2_lo_w);
	DECLARE_WRITE8_MEMBER(unk_reg3_lo_w);
	DECLARE_WRITE8_MEMBER(unk_reg1_hi_w);
	DECLARE_WRITE8_MEMBER(unk_reg2_hi_w);
	DECLARE_WRITE8_MEMBER(unk_reg3_hi_w);
	DECLARE_WRITE8_MEMBER(nmi_and_coins_w);
	DECLARE_WRITE8_MEMBER(lamps_w);
	DECLARE_READ8_MEMBER(igs_irqack_r);
	DECLARE_WRITE8_MEMBER(igs_irqack_w);
	DECLARE_READ8_MEMBER(expram_r);

	void unk_reg_lo_w( int offset, UINT8 data, int reg );
	void unk_reg_hi_w( int offset, UINT8 data, int reg );
	void show_out();

	DECLARE_CUSTOM_INPUT_MEMBER(hopper_r);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel3_tile_info);

	DECLARE_DRIVER_INIT(jackie);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
};




TILE_GET_INFO_MEMBER(jackie_state::get_fg_tile_info)
{
	int code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	int tile = code & 0x1fff;
	SET_TILE_INFO_MEMBER(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

WRITE8_MEMBER(jackie_state::fg_tile_w)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(jackie_state::fg_color_w)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}




WRITE8_MEMBER(jackie_state::bg_scroll_w)
{
	m_bg_scroll[offset] = data;
}


WRITE8_MEMBER(jackie_state::reel1_ram_w)
{
	m_reel1_ram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(jackie_state::get_reel1_tile_info)
{
	int code = m_reel1_ram[tile_index];
	SET_TILE_INFO_MEMBER(1, code, 0, 0);
}



WRITE8_MEMBER(jackie_state::reel2_ram_w)
{
	m_reel2_ram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(jackie_state::get_reel2_tile_info)
{
	int code = m_reel2_ram[tile_index];
	SET_TILE_INFO_MEMBER(1, code, 0, 0);
}


WRITE8_MEMBER(jackie_state::reel3_ram_w)
{
	m_reel3_ram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(jackie_state::get_reel3_tile_info)
{
	int code = m_reel3_ram[tile_index];
	SET_TILE_INFO_MEMBER(1, code, 0, 0);
}

void jackie_state::video_start()
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jackie_state::get_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jackie_state::get_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jackie_state::get_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jackie_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8,  8,  64, 32);
	m_fg_tilemap->set_transparent_pen(0);
}


UINT32 jackie_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i,j;
	int startclipmin = 0;
	const rectangle &visarea = screen.visible_area();

	bitmap.fill(m_palette->black_pen(), cliprect);

	for (i=0;i < 0x40;i++)
	{
		m_reel1_tilemap->set_scrolly(i, m_bg_scroll[i+0x000]);
		m_reel2_tilemap->set_scrolly(i, m_bg_scroll[i+0x040]);
		m_reel3_tilemap->set_scrolly(i, m_bg_scroll[i+0x080]);
	}

	for (j=0; j < 0x100-1; j++)
	{
		rectangle clip;
		int rowenable = m_bg_scroll2[j];

		/* draw top of screen */
		clip.set(visarea.min_x, visarea.max_x, startclipmin, startclipmin+1);

		if (rowenable==0)
		{
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
		}

		startclipmin+=1;
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void jackie_state::machine_start()
{
	save_item(NAME(m_exp_bank));
	// save_item(NAME(m_irq_enable)); //always 1?
	save_item(NAME(m_nmi_enable));
	// save_item(NAME(m_bg_enable)); //always 1?
	save_item(NAME(m_hopper));
	save_item(NAME(m_out));
	save_item(NAME(m_unk_reg));
}

void jackie_state::machine_reset()
{
	m_irq_enable    =   1;
	m_nmi_enable    =   0;
	m_hopper        =   0;
	m_bg_enable =   1;
}


void jackie_state::show_out()
{
#ifdef MAME_DEBUG
//  popmessage("%02x %02x %02x", m_out[0], m_out[1], m_out[2]);
	popmessage("520: %04x %04x %04x %04x %04x\n560: %04x %04x %04x %04x %04x\n5A0: %04x %04x %04x %04x %04x",
		m_unk_reg[0][0],m_unk_reg[0][1],m_unk_reg[0][2],m_unk_reg[0][3],m_unk_reg[0][4],
		m_unk_reg[1][0],m_unk_reg[1][1],m_unk_reg[1][2],m_unk_reg[1][3],m_unk_reg[1][4],
		m_unk_reg[2][0],m_unk_reg[2][1],m_unk_reg[2][2],m_unk_reg[2][3],m_unk_reg[2][4]
	);
#endif
}

void jackie_state::unk_reg_lo_w( int offset, UINT8 data, int reg )
{
	m_unk_reg[reg][offset] &= 0xff00;
	m_unk_reg[reg][offset] |= data;
	show_out();
}

WRITE8_MEMBER(jackie_state::unk_reg1_lo_w){ unk_reg_lo_w( offset, data, 0 ); }
WRITE8_MEMBER(jackie_state::unk_reg2_lo_w){ unk_reg_lo_w( offset, data, 1 ); }
WRITE8_MEMBER(jackie_state::unk_reg3_lo_w){ unk_reg_lo_w( offset, data, 2 ); }

void jackie_state::unk_reg_hi_w( int offset, UINT8 data, int reg )
{
	m_unk_reg[reg][offset] &= 0xff;
	m_unk_reg[reg][offset] |= data << 8;
	show_out();
}

WRITE8_MEMBER(jackie_state::unk_reg1_hi_w){ unk_reg_hi_w( offset, data, 0 ); }
WRITE8_MEMBER(jackie_state::unk_reg2_hi_w){ unk_reg_hi_w( offset, data, 1 ); }
WRITE8_MEMBER(jackie_state::unk_reg3_hi_w){ unk_reg_hi_w( offset, data, 2 ); }

WRITE8_MEMBER(jackie_state::nmi_and_coins_w)
{
	coin_counter_w(machine(), 0,        data & 0x01);   // coin_a
	coin_counter_w(machine(), 1,        data & 0x04);   // coin_c
	coin_counter_w(machine(), 2,        data & 0x08);   // key in
	coin_counter_w(machine(), 3,        data & 0x10);   // coin m_out mech

	set_led_status(machine(), 6,        data & 0x20);   // led for coin m_out / m_hopper active

	m_exp_bank   = (data & 0x02) ? 1 : 0;       // expram bank number
	m_nmi_enable = data & 0x80;     // nmi enable?

	m_out[0] = data;
	show_out();
}

WRITE8_MEMBER(jackie_state::lamps_w)
{
/*
    - Lbits -
    7654 3210
    =========
    ---- --x-  Hold1 lamp.
    --x- ----  Hold2 lamp.
    ---x ----  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---- -x--  Hold5 lamp.
    ---- ---x  Start lamp.
*/
	output_set_lamp_value(1, (data >> 1) & 1);      /* Lamp 1 - HOLD 1 */
	output_set_lamp_value(2, (data >> 5) & 1);      /* Lamp 2 - HOLD 2  */
	output_set_lamp_value(3, (data >> 4) & 1);      /* Lamp 3 - HOLD 3 */
	output_set_lamp_value(4, (data >> 3) & 1);      /* Lamp 4 - HOLD 4 */
	output_set_lamp_value(5, (data >> 2) & 1);      /* Lamp 5 - HOLD 5 */
	output_set_lamp_value(6, (data & 1));           /* Lamp 6 - START */

	m_hopper            =   (~data)& 0x80;

	m_out[1] = data;
	show_out();
}

READ8_MEMBER(jackie_state::igs_irqack_r)
{
	m_irq_enable = 1;
	return 0;
}

WRITE8_MEMBER(jackie_state::igs_irqack_w)
{
//  m_maincpu->set_input_line(0, CLEAR_LINE);
	m_out[2] = data;
	show_out();
}

READ8_MEMBER(jackie_state::expram_r)
{
	UINT8 *rom = memregion("gfx3")->base();

	offset += m_exp_bank * 0x8000;
//  logerror("PC %06X: %04x = %02x\n",space.device().safe_pc(),offset,rom[offset]);
	return rom[offset];
}


static ADDRESS_MAP_START( jackie_prg_map, AS_PROGRAM, 8, jackie_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_REGION("maincpu", 0xf000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jackie_io_map, AS_IO, 8, jackie_state )
	AM_RANGE(0x0520, 0x0524) AM_WRITE(unk_reg1_lo_w)
	AM_RANGE(0x0d20, 0x0d24) AM_WRITE(unk_reg1_hi_w)
	AM_RANGE(0x0560, 0x0564) AM_WRITE(unk_reg2_lo_w)
	AM_RANGE(0x0d60, 0x0d64) AM_WRITE(unk_reg2_hi_w)
	AM_RANGE(0x05a0, 0x05a4) AM_WRITE(unk_reg3_lo_w)
	AM_RANGE(0x0da0, 0x0da4) AM_WRITE(unk_reg3_hi_w)
	AM_RANGE(0x1000, 0x1107) AM_RAM AM_SHARE("bg_scroll2")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x2800, 0x2fff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("DSW1")           /* DSW1 */
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("DSW2")           /* DSW2 */
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("DSW3")           /* DSW3 */
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("DSW4")           /* DSW4 */
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("DSW5")           /* DSW5 */
	AM_RANGE(0x5080, 0x5080) AM_WRITE(nmi_and_coins_w)
	AM_RANGE(0x5081, 0x5081) AM_READ_PORT("SERVICE")
	AM_RANGE(0x5082, 0x5082) AM_READ_PORT("COINS")
	AM_RANGE(0x5090, 0x5090) AM_READ_PORT("BUTTONS1")
	AM_RANGE(0x5091, 0x5091) AM_WRITE(lamps_w )
	AM_RANGE(0x50a0, 0x50a0) AM_READ_PORT("BUTTONS2")
	AM_RANGE(0x50b0, 0x50b1) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE(0x50c0, 0x50c0) AM_READ(igs_irqack_r) AM_WRITE(igs_irqack_w)
	AM_RANGE(0x6000, 0x60ff) AM_RAM_WRITE(bg_scroll_w ) AM_SHARE("bg_scroll")
	AM_RANGE(0x6800, 0x69ff) AM_RAM_WRITE(reel1_ram_w )  AM_SHARE("reel1_ram")
	AM_RANGE(0x6a00, 0x6bff) AM_RAM_WRITE(reel2_ram_w )  AM_SHARE("reel2_ram")
	AM_RANGE(0x6c00, 0x6dff) AM_RAM_WRITE(reel3_ram_w )  AM_SHARE("reel3_ram")
	AM_RANGE(0x7000, 0x77ff) AM_RAM_WRITE(fg_tile_w )  AM_SHARE("fg_tile_ram")
	AM_RANGE(0x7800, 0x7fff) AM_RAM_WRITE(fg_color_w ) AM_SHARE("fg_color_ram")
	AM_RANGE(0x8000, 0xffff) AM_READ(expram_r)
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER(jackie_state::hopper_r)
{
	if (m_hopper) return !(m_screen->frame_number()%10);
	return machine().input().code_pressed(KEYCODE_H);
}

static INPUT_PORTS_START( jackie )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWB:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "45%" )
	PORT_DIPSETTING(    0x0e, "50%" )
	PORT_DIPSETTING(    0x0d, "55%" )
	PORT_DIPSETTING(    0x0c, "60%" )
	PORT_DIPSETTING(    0x0b, "65%" )
	PORT_DIPSETTING(    0x0a, "70%" )
	PORT_DIPSETTING(    0x09, "75%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x07, "83%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "88%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x03, "92%" )
	PORT_DIPSETTING(    0x02, "94%" )
	PORT_DIPSETTING(    0x01, "96%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPNAME( 0x20, 0x00, "Double Up Rate" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0xC0, 0x00, "Max Bet" ) PORT_DIPLOCATION("SWC:2,1")
	PORT_DIPSETTING(    0xC0, "1" )
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("SERVICE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9) PORT_NAME("Attendent")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF,jackie_state,hopper_r, (void *)0 ) PORT_NAME("HPSW")    // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )   // test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Clear")   // pays out
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Togglemode") // Used
	PORT_BIT( 0xC0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Stop")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS2")  // OK
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Small / Right Hammer")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Take/Left Hammer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Big / Center Hammer")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


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

static GFXDECODE_START( jackie )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x32x6, 0, 16 )
GFXDECODE_END

DRIVER_INIT_MEMBER(jackie_state,jackie)
{
	int A;
	UINT8 *rom = memregion("maincpu")->base();

	for (A = 0;A < 0xf000;A++)
	{
		rom[A] = rom[A] ^ 0x21;

		if (((A & 0x0080) == 0x0000) && ((A & 0x0008) == 0x0000)) rom[A] = rom[A] ^ 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
	memset( &rom[0xf000], 0, 0x1000);

	// Patch trap
	rom[0x7e86] = 0xc3;
}

TIMER_DEVICE_CALLBACK_MEMBER(jackie_state::irq)
{
	int scanline = param;

	if((scanline % 32) != 0)
		return;

	if((scanline % 64) == 32 && m_irq_enable)
		m_maincpu->set_input_line(0, HOLD_LINE);
	else if ((scanline % 64) == 0 && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( jackie, jackie_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz / 2)
	MCFG_CPU_PROGRAM_MAP(jackie_prg_map)
	MCFG_CPU_IO_MAP(jackie_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", jackie_state, irq, "screen", 0, 1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jackie_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jackie)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


ROM_START( jackie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jackiev110.u23",   0x0000, 0x10000, CRC(1b78a619) SHA1(a6eb6b6e544efa55225f2e947483614afb6ece3b) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "6.u6",  0x00000, 0x20000, CRC(d2ed60a9) SHA1(40e2280384aa5c9e72e87a3b9e673172ff695676) )
	ROM_LOAD( "5.u5",  0x20000, 0x20000, CRC(dc01fe7c) SHA1(683834ce2f13a923c0467209b93fef693d9c3e38) )
	ROM_LOAD( "4.u4",  0x40000, 0x20000, CRC(38a42dcd) SHA1(8cc08ff4143281d9022210d6577146d725df9044) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.u3",  0x00000, 0x4000, CRC(c69e962b) SHA1(492427ad1ac959cdf22d23439e0eb5932b60ec88) )
	ROM_LOAD( "2.u2",  0x10000, 0x4000, CRC(8900ffba) SHA1(065cf1810ec9738718e4c94613f726e85ba4314d) )
	ROM_LOAD( "1.u1",  0x20000, 0x4000, CRC(071d20f0) SHA1(77c87486803dccaa63732ff959c223b1313820e3) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "jackie7sv.u22",   0x0000, 0x10000, CRC(8b4eb6da) SHA1(480784917dfaf9a0343c1d56eb590b32bf5e94fd) )

	ROM_REGION( 0x10000, "misc", 0 )
	ROM_LOAD( "16l8.u31",   0x0000, 0x104, BAD_DUMP CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "18cv8.u14",  0x0000, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
	ROM_LOAD( "18cv8.u8",   0x0000, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
	ROM_LOAD( "18cv8.u9",   0x0000, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
ROM_END


GAME( 1993,  jackie,   0,        jackie,   jackie, jackie_state, jackie,  ROT0, "IGS",    "Happy Jackie (v110U)", MACHINE_SUPPORTS_SAVE )
