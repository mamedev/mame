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
		m_maincpu(*this,"maincpu")
		{ }

	int m_exp_bank;
	UINT8 *m_fg_tile_ram;
	UINT8 *m_fg_color_ram;
	tilemap_t *m_fg_tilemap;
	UINT8 *m_bg_scroll;
	UINT8 *m_bg_scroll2;
	tilemap_t *m_reel1_tilemap;
	UINT8 *m_reel1_ram;
	tilemap_t *m_reel2_tilemap;
	UINT8 *m_reel2_ram;
	tilemap_t *m_reel3_tilemap;
	UINT8 *m_reel3_ram;
	int m_irq_enable;
	int m_nmi_enable;
	int m_bg_enable;
	int m_hopper;
	UINT8 m_out[3];
	UINT16 m_unk_reg[3][5];

	required_device<cpu_device> m_maincpu;
};




static TILE_GET_INFO( get_fg_tile_info )
{
	jackie_state *state = machine.driver_data<jackie_state>();
	int code = state->m_fg_tile_ram[tile_index] | (state->m_fg_color_ram[tile_index] << 8);
	int tile = code & 0x1fff;
	SET_TILE_INFO(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

static WRITE8_HANDLER( fg_tile_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_fg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset);
}

static WRITE8_HANDLER( fg_color_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_fg_color_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset);
}




static WRITE8_HANDLER( bg_scroll_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_bg_scroll[offset] = data;
}


static WRITE8_HANDLER( jackie_reel1_ram_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_reel1_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m_reel1_tilemap,offset);
}

static TILE_GET_INFO( get_jackie_reel1_tile_info )
{
	jackie_state *state = machine.driver_data<jackie_state>();
	int code = state->m_reel1_ram[tile_index];
	SET_TILE_INFO(1, code, 0, 0);
}



static WRITE8_HANDLER( jackie_reel2_ram_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_reel2_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m_reel2_tilemap,offset);
}

static TILE_GET_INFO( get_jackie_reel2_tile_info )
{
	jackie_state *state = machine.driver_data<jackie_state>();
	int code = state->m_reel2_ram[tile_index];
	SET_TILE_INFO(1, code, 0, 0);
}


static WRITE8_HANDLER( jackie_reel3_ram_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_reel3_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m_reel3_tilemap,offset);
}

static TILE_GET_INFO( get_jackie_reel3_tile_info )
{
	jackie_state *state = machine.driver_data<jackie_state>();
	int code = state->m_reel3_ram[tile_index];
	SET_TILE_INFO(1, code, 0, 0);
}

static VIDEO_START(jackie)
{
	jackie_state *state = machine.driver_data<jackie_state>();
	state->m_reel1_tilemap = tilemap_create(machine,get_jackie_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel2_tilemap = tilemap_create(machine,get_jackie_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel3_tilemap = tilemap_create(machine,get_jackie_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(state->m_reel1_tilemap, 64);
	tilemap_set_scroll_cols(state->m_reel2_tilemap, 64);
	tilemap_set_scroll_cols(state->m_reel3_tilemap, 64);

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,	8,  8,	64, 32);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);
}


static SCREEN_UPDATE(jackie)
{
	jackie_state *state = screen->machine().driver_data<jackie_state>();
	int i,j;
	int startclipmin = 0;
	const rectangle &visarea = screen->visible_area();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	for (i=0;i < 0x40;i++)
	{
		tilemap_set_scrolly(state->m_reel1_tilemap, i, state->m_bg_scroll[i+0x000]);
		tilemap_set_scrolly(state->m_reel2_tilemap, i, state->m_bg_scroll[i+0x040]);
		tilemap_set_scrolly(state->m_reel3_tilemap, i, state->m_bg_scroll[i+0x080]);
	}

	for (j=0; j < 0x100-1; j++)
	{
		rectangle clip;
		int rowenable = state->m_bg_scroll2[j];

		/* draw top of screen */
		clip.min_x = visarea.min_x;
		clip.max_x = visarea.max_x;
		clip.min_y = startclipmin;
		clip.max_y = startclipmin+1;

		if (rowenable==0)
		{
			tilemap_draw(bitmap,&clip,state->m_reel1_tilemap,0,0);
		}
		else if (rowenable==1)
		{
			tilemap_draw(bitmap,&clip,state->m_reel2_tilemap,0,0);
		}
		else if (rowenable==2)
		{
			tilemap_draw(bitmap,&clip,state->m_reel3_tilemap,0,0);
		}
		else if (rowenable==3)
		{
		}

		startclipmin+=1;
	}

	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	return 0;
}



static MACHINE_RESET( jackie )
{
	jackie_state *state = machine.driver_data<jackie_state>();
	state->m_irq_enable	=	1;
	state->m_nmi_enable	=	0;
	state->m_hopper		=	0;
	state->m_bg_enable	=	1;
}


static void show_out(jackie_state *state)
{
#ifdef MAME_DEBUG
//  popmessage("%02x %02x %02x", state->m_out[0], state->m_out[1], state->m_out[2]);
	popmessage("520: %04x %04x %04x %04x %04x\n560: %04x %04x %04x %04x %04x\n5A0: %04x %04x %04x %04x %04x",
		state->m_unk_reg[0][0],state->m_unk_reg[0][1],state->m_unk_reg[0][2],state->m_unk_reg[0][3],state->m_unk_reg[0][4],
		state->m_unk_reg[1][0],state->m_unk_reg[1][1],state->m_unk_reg[1][2],state->m_unk_reg[1][3],state->m_unk_reg[1][4],
		state->m_unk_reg[2][0],state->m_unk_reg[2][1],state->m_unk_reg[2][2],state->m_unk_reg[2][3],state->m_unk_reg[2][4]
	);
#endif
}

static void jackie_unk_reg_lo_w( address_space *space, int offset, UINT8 data, int reg )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_unk_reg[reg][offset] &= 0xff00;
	state->m_unk_reg[reg][offset] |= data;
	show_out(state);
}

static WRITE8_HANDLER( jackie_unk_reg1_lo_w ) { jackie_unk_reg_lo_w( space, offset, data, 0 ); }
static WRITE8_HANDLER( jackie_unk_reg2_lo_w ) { jackie_unk_reg_lo_w( space, offset, data, 1 ); }
static WRITE8_HANDLER( jackie_unk_reg3_lo_w ) { jackie_unk_reg_lo_w( space, offset, data, 2 ); }

static void jackie_unk_reg_hi_w( address_space *space, int offset, UINT8 data, int reg )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_unk_reg[reg][offset] &= 0xff;
	state->m_unk_reg[reg][offset] |= data << 8;
	show_out(state);
}

static WRITE8_HANDLER( jackie_unk_reg1_hi_w ) { jackie_unk_reg_hi_w( space, offset, data, 0 ); }
static WRITE8_HANDLER( jackie_unk_reg2_hi_w ) { jackie_unk_reg_hi_w( space, offset, data, 1 ); }
static WRITE8_HANDLER( jackie_unk_reg3_hi_w ) { jackie_unk_reg_hi_w( space, offset, data, 2 ); }

static WRITE8_HANDLER( jackie_nmi_and_coins_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	coin_counter_w(space->machine(), 0,		data & 0x01);	// coin_a
	coin_counter_w(space->machine(), 1,		data & 0x04);	// coin_c
	coin_counter_w(space->machine(), 2,		data & 0x08);	// key in
	coin_counter_w(space->machine(), 3,		data & 0x10);	// coin state->m_out mech

	set_led_status(space->machine(), 6,		data & 0x20);	// led for coin state->m_out / state->m_hopper active

	state->m_exp_bank   = (data & 0x02) ? 1 : 0;		// expram bank number
	state->m_nmi_enable = data & 0x80;     // nmi enable?

	state->m_out[0] = data;
	show_out(state);
}

static WRITE8_HANDLER( jackie_lamps_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
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
	output_set_lamp_value(1, (data >> 1) & 1);		/* Lamp 1 - HOLD 1 */
	output_set_lamp_value(2, (data >> 5) & 1);		/* Lamp 2 - HOLD 2  */
	output_set_lamp_value(3, (data >> 4) & 1);		/* Lamp 3 - HOLD 3 */
	output_set_lamp_value(4, (data >> 3) & 1);		/* Lamp 4 - HOLD 4 */
	output_set_lamp_value(5, (data >> 2) & 1);		/* Lamp 5 - HOLD 5 */
	output_set_lamp_value(6, (data & 1));			/* Lamp 6 - START */

	state->m_hopper			=	(~data)& 0x80;

	state->m_out[1] = data;
	show_out(state);
}

static READ8_HANDLER( igs_irqack_r )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	state->m_irq_enable = 1;
	return 0;
}

static WRITE8_HANDLER( igs_irqack_w )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
//  cputag_set_input_line(space->machine(), "maincpu", 0, CLEAR_LINE);
	state->m_out[2] = data;
	show_out(state);
}

static READ8_HANDLER( expram_r )
{
	jackie_state *state = space->machine().driver_data<jackie_state>();
	UINT8 *rom = space->machine().region("gfx3")->base();

	offset += state->m_exp_bank * 0x8000;
//  logerror("PC %06X: %04x = %02x\n",cpu_get_pc(&space->device()),offset,rom[offset]);
	return rom[offset];
}


static ADDRESS_MAP_START( jackie_prg_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_REGION("maincpu", 0xf000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jackie_io_map, AS_IO, 8 )
	AM_RANGE(0x0520, 0x0524) AM_WRITE(jackie_unk_reg1_lo_w)
	AM_RANGE(0x0d20, 0x0d24) AM_WRITE(jackie_unk_reg1_hi_w)
	AM_RANGE(0x0560, 0x0564) AM_WRITE(jackie_unk_reg2_lo_w)
	AM_RANGE(0x0d60, 0x0d64) AM_WRITE(jackie_unk_reg2_hi_w)
	AM_RANGE(0x05a0, 0x05a4) AM_WRITE(jackie_unk_reg3_lo_w)
	AM_RANGE(0x0da0, 0x0da4) AM_WRITE(jackie_unk_reg3_hi_w)
	AM_RANGE(0x1000, 0x1107) AM_RAM AM_BASE_MEMBER(jackie_state, m_bg_scroll2 )
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE_GENERIC( paletteram )
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE_GENERIC( paletteram2 )
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("DSW1")			/* DSW1 */
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("DSW2")			/* DSW2 */
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("DSW3")			/* DSW3 */
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("DSW4")			/* DSW4 */
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("DSW5")			/* DSW5 */
	AM_RANGE(0x5080, 0x5080) AM_WRITE(jackie_nmi_and_coins_w)
	AM_RANGE(0x5081, 0x5081) AM_READ_PORT("SERVICE")
	AM_RANGE(0x5082, 0x5082) AM_READ_PORT("COINS")
	AM_RANGE(0x5090, 0x5090) AM_READ_PORT("BUTTONS1")
	AM_RANGE(0x5091, 0x5091) AM_WRITE( jackie_lamps_w )
	AM_RANGE(0x50a0, 0x50a0) AM_READ_PORT("BUTTONS2")
	AM_RANGE(0x50b0, 0x50b1) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x50c0, 0x50c0) AM_READ(igs_irqack_r) AM_WRITE(igs_irqack_w)
	AM_RANGE(0x6000, 0x60ff) AM_RAM_WRITE( bg_scroll_w ) AM_BASE_MEMBER(jackie_state, m_bg_scroll )
	AM_RANGE(0x6800, 0x69ff) AM_RAM_WRITE( jackie_reel1_ram_w )  AM_BASE_MEMBER(jackie_state, m_reel1_ram )
	AM_RANGE(0x6a00, 0x6bff) AM_RAM_WRITE( jackie_reel2_ram_w )  AM_BASE_MEMBER(jackie_state, m_reel2_ram )
	AM_RANGE(0x6c00, 0x6dff) AM_RAM_WRITE( jackie_reel3_ram_w )  AM_BASE_MEMBER(jackie_state, m_reel3_ram )
	AM_RANGE(0x7000, 0x77ff) AM_RAM_WRITE( fg_tile_w )  AM_BASE_MEMBER(jackie_state, m_fg_tile_ram )
	AM_RANGE(0x7800, 0x7fff) AM_RAM_WRITE( fg_color_w ) AM_BASE_MEMBER(jackie_state, m_fg_color_ram )
	AM_RANGE(0x8000, 0xffff) AM_READ(expram_r)
ADDRESS_MAP_END

static CUSTOM_INPUT( hopper_r )
{
	jackie_state *state = field.machine().driver_data<jackie_state>();
	if (state->m_hopper) return !(field.machine().primary_screen->frame_number()%10);
	return field.machine().input().code_pressed(KEYCODE_H);
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM( hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )	// test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Clear")	// pays out
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Togglemode")	// Used
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

	PORT_START("BUTTONS2")	// OK
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Small / Right Hammer")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)	PORT_NAME("Take/Left Hammer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3)	PORT_NAME("W-Up")
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

static DRIVER_INIT( jackie )
{

	int A;
	UINT8 *rom = machine.region("maincpu")->base();

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

static TIMER_DEVICE_CALLBACK( jackie_irq )
{
	jackie_state *state = timer.machine().driver_data<jackie_state>();
	int scanline = param;

	if((scanline % 32) != 0)
		return;

	if((scanline % 64) == 32 && state->m_irq_enable)
		device_set_input_line(state->m_maincpu, 0, HOLD_LINE);
	else if	((scanline % 64) == 0 && state->m_nmi_enable)
		device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( jackie, jackie_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz / 2)
	MCFG_CPU_PROGRAM_MAP(jackie_prg_map)
	MCFG_CPU_IO_MAP(jackie_io_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", jackie_irq, "screen", 0, 1)

	MCFG_MACHINE_RESET(jackie)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE(jackie)

	MCFG_GFXDECODE(jackie)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(jackie)

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


GAME( 1993,  jackie,   0,        jackie,   jackie, jackie,  ROT0, "IGS",    "Happy Jackie (v110U)",       0 )
