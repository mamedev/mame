/*****************************************************************************

Champion Poker by IGS   (documented by Mirko Buffoni)
---

Memory Layout (refers to CSK227IT.  Others may have different addresses)

ROM:        0000-efff
RAM:        f000-ffff

---

I/O Ports

Palette:    2000-27ff   (low byte)
            2800-2fff   (high byte)
VideoRAM:   7000-77ff
ColorRAM:   7800-7fff
DSW1-5:     4000-4004   (see input ports section below)
InputPorts: 50a0        (unused in this game)
            5081-5082   (Coins and Keyboard)
            5091        (Keyboard)
Expansion:  8000-ffff   (R)     Used to read from an expansion rom

Unknown:    5080        (RW)    (possibly related to ticket/hopper)
            5090-5091   (RW)    (possibly related to eprom counters)
            50b0-50b1   (W)     (OPL2 compatible chip)
            5083        (W)     (used only at reset, maybe)
            1000-10ff   (W) ???
            6000-67ff   (W) ???
            6800-6fff   (W)     Expansion video layer (used with ability)

---

Timing:

Game is synchronized with VBLANK. It uses IRQ & NMI interrupts.
During a frame, there must be 4 IRQs and 4 NMIs in order to play
to the correct speed.

---

Notes about palette:
Charset is 6 bit depth (thus 64 colors of granularity)
Colortable is made up of 2 entries of 64 bytes for each palette,
splitted, and colorinfo is stored to form the following word:

xBBBBBGGGGGRRRRR    (Bit 15 is never used)

---

FIX:  csk227it has video issues, as after Ability game, bg_tilemap is not reset
	so there must be some bg_enable command which I couldn't find, or rom is
	from a beta version which has transparency issues.  This doesn't happen with
	csk234it or New Champion Skill.
	Insert credits with Key-In and press Pay-out to play ability game, and wait
	for attract-mode to show cubes (not cards), which are transparent and reveal
	background tilemap.

*****************************************************************************/


#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/2413intf.h"
#include "igspoker.lh"





static int nmi_enable, bg_enable, hopper;

static MACHINE_RESET( igs )
{
	nmi_enable	=	0;
	hopper		=	0;
	bg_enable	=	1;
}

static INTERRUPT_GEN( igs_interrupt )
{
	if (cpu_getiloops(device) % 2) {
		cpu_set_input_line(device, 0, HOLD_LINE);
	} else {
		if (nmi_enable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static READ8_HANDLER( igs_irqack_r )
{
	return 0;
}

static WRITE8_HANDLER( igs_irqack_w )
{
//	cpu_set_input_line(space->machine->cpu[0], 0, CLEAR_LINE);
}



static UINT8   *fg_tile_ram, *fg_color_ram, *bg_tile_ram;
static tilemap *fg_tilemap, *bg_tilemap;

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = bg_tile_ram[tile_index];
	SET_TILE_INFO(1 + (tile_index & 3), code, 0, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = fg_tile_ram[tile_index] | (fg_color_ram[tile_index] << 8);
	int tile = code & 0x1fff;
	SET_TILE_INFO(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

static WRITE8_HANDLER( bg_tile_w )
{
	bg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static WRITE8_HANDLER( fg_tile_w )
{
	fg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static WRITE8_HANDLER( fg_color_w )
{
	fg_color_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static VIDEO_START(igs_video)
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,	8,  32,	64, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,	8,  8,	64, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static VIDEO_UPDATE(igs_video)
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	// FIX: CSK227IT must have some way to disable background, or wrong gfx?
	if (bg_enable) tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}

static UINT8 out[3];

static void show_out(void)
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x", out[0], out[1]);
#endif
}

static WRITE8_HANDLER( igs_nmi_and_coins_w )
{
	coin_counter_w(0,		data & 0x01);	// coin_a
	coin_counter_w(1,		data & 0x04);	// coin_c
	coin_counter_w(2,		data & 0x08);	// key in
	coin_counter_w(3,		data & 0x10);	// coin out mech

	set_led_status(6,		data & 0x20);	// led for coin out / hopper active

	nmi_enable = data & 0x80;     // nmi enable?

	out[0] = data;
	show_out();
}

static WRITE8_HANDLER( igs_lamps_w )
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
	output_set_lamp_value(1, (data >> 1) & 1);		/* Lamp 1 - HOLD 1 */
	output_set_lamp_value(2, (data >> 5) & 1);		/* Lamp 2 - HOLD 2  */
	output_set_lamp_value(3, (data >> 4) & 1);		/* Lamp 3 - HOLD 3 */
	output_set_lamp_value(4, (data >> 3) & 1);		/* Lamp 4 - HOLD 4 */
	output_set_lamp_value(5, (data >> 2) & 1);		/* Lamp 5 - HOLD 5 */
	output_set_lamp_value(6, (data & 1));			/* Lamp 6 - START */

	hopper			=	(~data)& 0x80;

	out[1] = data;
	show_out();
}


static size_t protection_res = 0;

static READ8_HANDLER( custom_io_r )
{
	return protection_res;
}

static WRITE8_HANDLER( custom_io_w )
{
	switch (data)
	{
		case 0x00: protection_res = input_port_read(space->machine, "BUTTONS1"); break;
		case 0x20: protection_res = 0x49; break;
		case 0x21: protection_res = 0x47; break;
		case 0x22: protection_res = 0x53; break;
		case 0x24: protection_res = 0x41; break;
		case 0x25: protection_res = 0x41; break;
		case 0x26: protection_res = 0x7f; break;
		case 0x27: protection_res = 0x41; break;
		case 0x28: protection_res = 0x41; break;
		case 0x2a: protection_res = 0x3e; break;
		case 0x2b: protection_res = 0x41; break;
		case 0x2c: protection_res = 0x49; break;
		case 0x2d: protection_res = 0xf9; break;
		case 0x2e: protection_res = 0x0a; break;
		case 0x30: protection_res = 0x26; break;
		case 0x31: protection_res = 0x49; break;
		case 0x32: protection_res = 0x49; break;
		case 0x33: protection_res = 0x49; break;
		case 0x34: protection_res = 0x32; break;
		default:
			protection_res = data;
	}
}

static CUSTOM_INPUT( hopper_r )
{
	if (hopper) return !(video_screen_get_frame_number(field->port->machine->primary_screen)%10);
	return input_code_pressed(KEYCODE_H);
}

static ADDRESS_MAP_START( igspoker_prg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_REGION("main", 0xf000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( igspoker_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE( &paletteram )
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE( &paletteram_2 )
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("DSW1")			/* DSW1 */
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("DSW2")			/* DSW2 */
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("DSW3")			/* DSW3 */
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("DSW4")			/* DSW4 */
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("DSW5")			/* DSW5 */
	AM_RANGE(0x5080, 0x5080) AM_WRITE(igs_nmi_and_coins_w)
	AM_RANGE(0x5081, 0x5081) AM_READ_PORT("SERVICE")			/* Services */
	AM_RANGE(0x5082, 0x5082) AM_READ_PORT("COINS")			/* Coing & Kbd */
	AM_RANGE(0x5090, 0x5090) AM_WRITE(custom_io_w)
	AM_RANGE(0x5091, 0x5091) AM_READ(custom_io_r) AM_WRITE( igs_lamps_w )			/* Keyboard */
	AM_RANGE(0x50a0, 0x50a0) AM_READ_PORT("BUTTONS2")			/* Not connected */
	AM_RANGE(0x50b0, 0x50b0) AM_WRITE(ym2413_register_port_0_w)
	AM_RANGE(0x50b1, 0x50b1) AM_WRITE(ym2413_data_port_0_w)
	AM_RANGE(0x50c0, 0x50c0) AM_READ(igs_irqack_r) AM_WRITE(igs_irqack_w)
	AM_RANGE(0x6800, 0x6fff) AM_RAM_WRITE( bg_tile_w )  AM_BASE( &bg_tile_ram )
	AM_RANGE(0x7000, 0x77ff) AM_RAM_WRITE( fg_tile_w )  AM_BASE( &fg_tile_ram )
	AM_RANGE(0x7800, 0x7fff) AM_RAM_WRITE( fg_color_w ) AM_BASE( &fg_color_ram )
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("gfx3", 0)
ADDRESS_MAP_END


/* MB: 05 Jun 99  Input ports and Dip switches are all verified! */

static INPUT_PORTS_START( cpoker )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1:1" )
	PORT_DIPSETTING(    0x40, "10:1" )
	PORT_DIPSETTING(    0x20, "100:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )		/* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Anytime Key-in" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM( hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_Q) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold1/High/Low")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold5/Bet")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold4/Take")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold3/W-Up")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold2/Red/Black")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( csk227 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:6,7")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x18, "Max Bet" ) PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1500" )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x00, "7500" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:8,7")
	PORT_DIPSETTING(    0x03, "200" )
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "800" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )		/* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Ability" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Payout Select" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, "Ticket" )
	PORT_DIPSETTING(    0x00, "Hopper" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x07, 0x07, "Key Out Rate" ) PORT_DIPLOCATION("SWE:8,7,6")
	PORT_DIPSETTING(    0x07, "1:1" )
	PORT_DIPSETTING(    0x06, "10:1" )
	PORT_DIPSETTING(    0x05, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x03, "100:1" )		/* Bits 1-0 are all equivalents */
	PORT_DIPNAME( 0x08, 0x00, "Card Select" ) PORT_DIPLOCATION("SWE:5")
	PORT_DIPSETTING(    0x08, "Poker" )
	PORT_DIPSETTING(    0x00, "Tetris" )
	PORT_DIPNAME( 0x70, 0x70, "Ticket Rate" ) PORT_DIPLOCATION("SWE:4,3,2")
	PORT_DIPSETTING(    0x70, "1:1" )
	PORT_DIPSETTING(    0x60, "5:1" )
	PORT_DIPSETTING(    0x50, "10:1" )
	PORT_DIPSETTING(    0x40, "20:1" )
	PORT_DIPSETTING(    0x30, "25:1" )
	PORT_DIPSETTING(    0x20, "50:1" )
	PORT_DIPSETTING(    0x10, "100:1" )
	PORT_DIPSETTING(    0x00, "200:1" )
	PORT_DIPNAME( 0x80, 0x80, "Win Table" ) PORT_DIPLOCATION("SWE:1")
	PORT_DIPSETTING(    0x80, "Change" )
	PORT_DIPSETTING(    0x00, "Fixed" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM( hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold5/Bet")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 	 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 	 )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_CODE(KEYCODE_Q) PORT_NAME("Key In")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_CODE(KEYCODE_W) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold4/Take")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold3/W-Up")

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold1/High/Low")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold2/Red/Black")

	PORT_START("BUTTONS2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END


static INPUT_PORTS_START( csk234 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1:1" )
	PORT_DIPSETTING(    0x40, "10:1" )
	PORT_DIPSETTING(    0x20, "100:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )		/* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x40, "Card Select" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, "Poker" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Anytime Key-in" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xC0, IP_ACTIVE_LOW, IPT_UNUSED )			/* Joker and Royal Flush are always enabled */

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Hopper" ) PORT_DIPLOCATION("SWE:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Payout Select" ) PORT_DIPLOCATION("SWE:7")
	PORT_DIPSETTING(    0x02, "Hopper" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x0c, 0x0c, "Ticket Rate" ) PORT_DIPLOCATION("SWE:6,5")
	PORT_DIPSETTING(    0x0c, "10:1" )
	PORT_DIPSETTING(    0x08, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x10, 0x00, "Ability" ) PORT_DIPLOCATION("SWE:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM( hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_Q) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold1/High/Low")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold5/Bet")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold4/Take")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold3/W-Up")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold2/Red/Black")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


static INPUT_PORTS_START( igs_ncs )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "W-UP Limit" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1500" )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x00, "7500" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "W-UP Pool" ) PORT_DIPLOCATION("SWC:8,7")
	PORT_DIPSETTING(    0x03, "200" )
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "800" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )		/* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x40, "Ability Pay" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, "All" )
	PORT_DIPSETTING(    0x00, "1/Time" )
	PORT_DIPNAME( 0x80, 0x80, "Ability" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x07, 0x07, "Key Out Rate" ) PORT_DIPLOCATION("SWE:8,7,6")
	PORT_DIPSETTING(    0x07, "1:1" )
	PORT_DIPSETTING(    0x06, "10:1" )
	PORT_DIPSETTING(    0x05, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x03, "100:1" )		/* latest 4 is 100 for ON/OFF */
	PORT_DIPNAME( 0x08, 0x08, "Card Select" ) PORT_DIPLOCATION("SWE:5")
	PORT_DIPSETTING(    0x08, "Poker" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPNAME( 0x70, 0x70, "Ticket Rate" ) PORT_DIPLOCATION("SWE:4,3,2")
	PORT_DIPSETTING(    0x70, "1:1" )
	PORT_DIPSETTING(    0x60, "5:1" )
	PORT_DIPSETTING(    0x50, "10:1" )
	PORT_DIPSETTING(    0x40, "20:1" )
	PORT_DIPSETTING(    0x30, "25:1" )
	PORT_DIPSETTING(    0x20, "50:1" )
	PORT_DIPSETTING(    0x10, "100:1" )
	PORT_DIPSETTING(    0x00, "200:1" )
	PORT_DIPNAME( 0x80, 0x00, "Oddstab Fixed" ) PORT_DIPLOCATION("SWE:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM( hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_Q) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold1/High/Low")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold5/Bet")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold4/Take")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold3/W-Up")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold2/Red/Black")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8, 8,	/* 8*8 characters */
	RGN_FRAC(1, 3),
	6,		/* 6 bits per pixel */
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2	/* every char takes 32 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	8, 32,   /* 8*32 characters */
	RGN_FRAC(1, 3),
	6,      /* 6 bits per pixel */
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( igspoker )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, charlayout2,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, charlayout2,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, charlayout2,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, charlayout2,  0, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( igspoker )

	/* basic machine hardware */
	MDRV_CPU_ADD("main",Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(igspoker_prg_map,0)
	MDRV_CPU_IO_MAP(igspoker_io_map,0)
	MDRV_CPU_VBLANK_INT_HACK(igs_interrupt,8)

	MDRV_MACHINE_RESET(igs)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 32*8-1)

	MDRV_GFXDECODE(igspoker)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(igs_video)
	MDRV_VIDEO_UPDATE(igs_video)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ym", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( csk227it )

	MDRV_IMPORT_FROM(igspoker)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( csk234it )

	MDRV_IMPORT_FROM(igspoker)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( igs_ncs )

	MDRV_IMPORT_FROM(igspoker)

MACHINE_DRIVER_END



/*  ROM Regions definition
 */

ROM_START( cpoker )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "v220i1.bin",  0x0000, 0x8000, CRC(b7cae556) SHA1(bb43ee48634879029ed1a7cd4133d7f12413e2ac) )
	ROM_LOAD( "v220i2.bin",  0x8000, 0x8000, CRC(8245e42c) SHA1(b7e7b9f643e6dc2f4d5aaf7d50d0a9154ed9a4e7) )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "220i1.bin",  0x40000, 0x20000, CRC(9c4c0af1) SHA1(7a9808b3093b23bde7ecc7405689b2a28ae34e61) )
	ROM_LOAD( "220i2.bin",  0x20000, 0x20000, CRC(331fa4b8) SHA1(ddac57251fa5dfecc0988a2ca01eec016ef47f20) )
	ROM_LOAD( "220i3.bin",  0x00000, 0x20000, CRC(bd2f797c) SHA1(5ca5adae44490dd109f630213a09a68c12f9bd1a) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 | ROMREGION_DISPOSE )

	ROM_REGION( 0x10000, "gfx3", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "220i7.bin",   0x0000, 0x8000, CRC(8a2ff310) SHA1(a415a99dbb1448b4b2b94e17a3973e6347e3be18) )
ROM_END

ROM_START( csk227it )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "v227i.bin",   0x0000, 0x10000, CRC(df1ebf49) SHA1(829c7575d3d3780557405b3a61859901df6dbe4f) )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "6.227",  0x00000, 0x20000, CRC(e9aad93b) SHA1(72116759cd8ddd9828f534e8f8a3f9f96ad2e002) )
	ROM_LOAD( "5.227",  0x20000, 0x20000, CRC(e4c4c8da) SHA1(0442b0de68f3b69e613506348e00c3cf9139edcf) )
	ROM_LOAD( "4.227",  0x40000, 0x20000, CRC(afb365dd) SHA1(930a4cd516258e703a75afc25ef6b2655b8b696a) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(fcb115ac) SHA1(a9f2b9762413840669cd44f8e54b47a7c4350d11) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(848343a3) SHA1(b12f9bc2feb470d2fa8b085621fa60c0895109d4) )
	ROM_LOAD( "1.bin",  0x20000, 0x10000, CRC(921ad5de) SHA1(b06ab2e63b31361dcb0367110f47bf2453ecdca6) )

	ROM_REGION( 0x10000, "gfx3", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.227",   0x0000, 0x10000, CRC(a10786ad) SHA1(82f5f81808ca70d67a2710cc66fbbf78588b33b5) )
ROM_END

ROM_START( csk234it )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "v234it.bin",   0x0000, 0x10000, CRC(344b7059)  SHA1(990cb84e35c0c50d3be9fbb76a11395114dc6c9b) )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "6.234",  0x00000, 0x20000, CRC(23b855a4) SHA1(8217bac61ad09483d8789113cf394d0e525ab28a) )
	ROM_LOAD( "5.234",  0x20000, 0x20000, CRC(189039d7) SHA1(146fd1ddb23ceaa4192e0382b0ab82f5cfbdabfe) )
	ROM_LOAD( "4.234",  0x40000, 0x20000, CRC(c82b0ffc) SHA1(5ebd7da76d402b7111cbe9012cfa3b8a8ff1a86e) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(fcb115ac) SHA1(a9f2b9762413840669cd44f8e54b47a7c4350d11) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(848343a3) SHA1(b12f9bc2feb470d2fa8b085621fa60c0895109d4) )
	ROM_LOAD( "1.bin",  0x20000, 0x10000, CRC(921ad5de) SHA1(b06ab2e63b31361dcb0367110f47bf2453ecdca6) )

	ROM_REGION( 0x10000, "gfx3", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.234",   0x0000, 0x10000, CRC(ae6dd4ad) SHA1(4772d5c150d64d1ef3b68e16214f594eea0b3c1b) )
ROM_END


/*

Stelle e Cubi

-- most of the roms on this seem to be the wrong size / missing data
   but its appears to be a hack based on Champion Skill

1x Z84c0006
1x 12mhz OSC
1x U6295 sound chip
1x Actel FPGA (gfx chip)

ROMs
Note    1x Battery
5x banks of dipswitch


--

This doesn't attempt to decode the gfx.

*/
ROM_START( stellecu )
	ROM_REGION( 0x20000, "main", 0 )
	/* there is data at 0x18000 which is probably mapped somewhere */
	ROM_LOAD( "u35.bin",   0x0000, 0x20000, CRC(914b7c59) SHA1(3275b5016524467199f32d653c757bfe4f9cfc60) )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	/* seems to be missing half the gfx */
	ROM_LOAD( "u23.bin",   0x0000, 0x40000, BAD_DUMP CRC(9d95757d) SHA1(f7f44d684f1f3a5b1e9c0a82f4377c6d79eb4214) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_DISPOSE )
	/* seems to be missing half the gfx */
	ROM_LOAD( "u25.bin",   0x0000, 0x40000, BAD_DUMP CRC(63094010) SHA1(a781f1c529167dd0ab411c66b72105fc19e32f02) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	/* missing sample tables at start of rom */
	ROM_LOAD( "u15.bin",   0x0000, 0x40000, BAD_DUMP CRC(72e3e9c1) SHA1(6a8fb93059bee5a4e4b4deb9fee4b5869e53983b) )
ROM_END

/*  Decode a simple PAL encryption
 */

static DRIVER_INIT( cpoker )
{
	int A;
	UINT8 *rom = memory_region(machine, "main");


	for (A = 0;A < 0x10000;A++)
	{
		rom[A] ^= 0x21;
		if ((A & 0x0030) == 0x0010) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}

static DRIVER_INIT( cpokert )
{
	UINT8 *rom = memory_region(machine, "main");
	int i;

	/* decrypt the program ROM */
	for (i = 0;i < 0x10000;i++)
	{
		if((i & 0x200) && (i & 0x80))
		{
			rom[i] ^= ((~i & 2) >> 1);
		}
		else
		{
			rom[i] ^= 0x01;
		}

		if((i & 0x30) != 0x10)
		{
			rom[i] ^= 0x20;
		}

		if((i & 0x900) == 0x900 && ((i & 0xc0) == 0x40 || (i & 0xc0) == 0xc0))
		{
			rom[i] ^= 0x02;
		}
	}
}

static DRIVER_INIT( cska )
{
	int A;
	UINT8 *rom = memory_region(machine, "main");


	for (A = 0;A < 0x10000;A++)
	{
		if ((A & 0x0020) == 0x0000) rom[A] ^= 0x01;
		if ((A & 0x0020) == 0x0020) rom[A] ^= 0x21;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0028) == 0x0028) rom[A] ^= 0x20;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}


static DRIVER_INIT( igs_ncs )
{
	int A;
	UINT8 *rom = memory_region(machine, "main");


	for (A = 0;A < 0x10000;A++)
	{
		rom[A] ^= 0x21;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0140) == 0x0100) rom[A] ^= 0x20;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}


/*

1x ZILOG Z0840006PSC-Z80CPU (main)
1x YM2413 (sound)
1x NEC D8255AC (label: ORIGINAL BY IGS 102986)
1x oscillator 12.000MHz (main)
1x oscillator 3.579545MHz (sound)

1x custom QFP80 label AMT001
1x custom QFP80 label IGS002
1x custom DIP40 label IGS003 (under chip label 8255)

ROMs

3x MX27C1000DC (4,5,6)
1x NM27C256Q (7)
1x 27C512 (200)
2x PEEL18CV8P (8,9)
1x PAL16L8ACN (31)
2x PEEL18CV8P (12,14) <-> UNREADABLE, protected!

Note

1x 10x2 edge connector (con1) (looks like a coin payout)
1x 36x2 edge connector (con2)
1x pushbutton (sw6)
5x 8 switches dips (sw1-5)
1x trimmer (volume)
----------------------
IGS PCB NO-T0039-8

*/

ROM_START( cpokert )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "champingv-200g.u23", 0x00000, 0x10000, CRC(696cb684) SHA1(ce9e5bed83d0bd3b115f556cc89e3293ac6b69c3) )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "cpoker6.u6", 0x00000, 0x20000, CRC(f3e61b24) SHA1(b18998defb6e51daef4ac5a5865674565ffb9029) )
	ROM_LOAD( "cpoker5.u5", 0x20000, 0x20000, CRC(a68b305f) SHA1(f872d2bf7ab194145dffe6b254ae0ad66aa6a497) )
	ROM_LOAD( "cpoker4.u4", 0x40000, 0x20000, CRC(860be7c9) SHA1(41bc58713076276aeefc44c7ea903549692b0224) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", ROMREGION_DISPOSE )
	ROM_COPY( "gfx1", 0, 0, 0x60000 )

	ROM_REGION( 0x10000, "gfx3", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "cpoker7.u22", 0x0000, 0x8000, CRC(dae3ecda) SHA1(c881e143ec600c5a931f26cd097da6353e1da7c3) )

	// convert them to the pld format
	ROM_REGION( 0x2000, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "ag-u31.u31", 0x00000, 0x000b60, CRC(fd36baf2) SHA1(caac8bf47bc958395f97b6191569196efe3b3eaa) )
	ROM_LOAD( "ag-u8.u8",   0x00000, 0x0015e2, CRC(c0308c63) SHA1(16819a5c147fef38a235675fa4442da9fa8a6618) )
	ROM_LOAD( "ag-u9.u9",   0x00000, 0x0015e2, CRC(2e8039a3) SHA1(e39635ee9485a5ccd28526f1af7ec2e3294b0aec) )
ROM_END


ROM_START( igs_ncs )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "v.bin",   0x0000, 0x10000, CRC(8077724b)  SHA1(1f6e01d5838e6ec4f91b07637c281a3f59631a51) )

	ROM_REGION( 0x60000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "6.bin",  0x00000, 0x20000, CRC(d8e88148) SHA1(5f5c06d947027ef76026e8834f2090b96652006c) )
	ROM_LOAD( "5.bin",  0x20000, 0x20000, CRC(96c8a71c) SHA1(202d04850df9dfbd405c4b5372ef1b39850ac7f7) )
	ROM_LOAD( "4.bin",  0x40000, 0x20000, CRC(5480eae8) SHA1(93e35e8ba7d282cb93d51498420341a4e95acf78) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(fcb115ac) SHA1(a9f2b9762413840669cd44f8e54b47a7c4350d11) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(848343a3) SHA1(b12f9bc2feb470d2fa8b085621fa60c0895109d4) )
	ROM_LOAD( "1.bin",  0x20000, 0x10000, CRC(921ad5de) SHA1(b06ab2e63b31361dcb0367110f47bf2453ecdca6) )

	ROM_REGION( 0x10000, "gfx3", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.bin",   0x0000, 0x10000, CRC(678e412c) SHA1(dba031d3576d098d314d6589dd1aeda44d17c650) )
ROM_END


/* New Champion Skill by IGS
 -- the dump MAY be incomplete, there were 3 empty positions on the PCB near
    the gfx roms

Chips of Note

IGS 003C (near chip with TEST OK E0069281 label)
IGS 002
IGA 001A


'file'
KC8255A
9941
(near CPU roms)

UM3567 9946

5x 8 switch dips

Clocks
3.579545Mhz (near sound)
12Mhz


--- what is the CPU, it looks like either Z80 or Z180 based
 -- CPU rom is lightly encrypted (usual IGS style, some xors)

*/

DRIVER_INIT( igs_ncs2 )
{
	UINT8 *src = (UINT8 *) (memory_region(machine, "main"));
	int i;

	for(i = 0; i < 0x10000; i++)
	{
		/* bit 0 xor layer */
		if(i & 0x200)
		{
			if(i & 0x80)
			{
				if(~i & 0x02)
				{
					src[i] ^= 0x01;
				}
			}
			else
			{
				src[i] ^= 0x01;
			}
		}
		else
		{
			src[i] ^= 0x01;
		}

		/* bit 1 xor layer */
		if(i & 0x800)
		{
			if(i & 0x100)
			{
				if(i & 0x40)
				{
					src[i] ^= 0x02;
				}
			}
		}

		/* bit 5 xor layer */
		if(i & 0x100)
		{
			if(i & 0x40)
			{
				src[i] ^= 0x20;
			}
		}
		else
		{
			src[i] ^= 0x20;
		}
	}

}

ROM_START( igs_ncs2 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ncs_v100n.u20", 0x00000, 0x10000, CRC(2bb91de5) SHA1(b0b7b3b9cee1ce4da10cf78ef1c8079f3d9cafbf) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ncs_v100n.u50", 0x00000, 0x40000, CRC(ff2bb3dc) SHA1(364c948504003b4230fbdac74227842c802d4c12) )
	ROM_LOAD( "ncs_v100n.u51", 0x40000, 0x40000, CRC(f8530313) SHA1(b21d6de7d5d4b902008ceea7e1227545e0d1701b) )
	ROM_LOAD( "ncs_v100n.u52", 0x80000, 0x40000, CRC(2fa5b6df) SHA1(5bfc651297440f73692079f1806b1e40b457b7b8) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASEFF | ROMREGION_DISPOSE )
	// looks like these are needed for pre-game screens, sockets were empty
	ROM_LOAD( "ncs_v100n.u55", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "ncs_v100n.u56", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "ncs_v100n.u57", 0x20000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "ncs_v100n.u21", 0x00000, 0x10000, CRC(678e412c) SHA1(dba031d3576d098d314d6589dd1aeda44d17c650) )
ROM_END



GAMEL( 1993?, cpoker,   0,        igspoker, cpoker,  cpoker,  ROT0, "IGS",    "Champion Poker (v220I)",                    0, layout_igspoker )
GAMEL( 1993?, cpokert,  cpoker,   igspoker, cpoker,  cpokert, ROT0, "Tuning", "Champion Poker (v200G)",                    0,	layout_igspoker )
GAMEL( 198?,  csk227it, 0,        csk227it, csk227,  cska,    ROT0, "IGS",    "Champion Skill (with Ability)",             0,	layout_igspoker ) /* SU 062 */
GAMEL( 198?,  csk234it, csk227it, csk234it, csk234,  cska,    ROT0, "IGS",    "Champion Skill (Ability, Poker & Symbols)", 0,	layout_igspoker ) /* SU 062 */
GAMEL( 198?,  igs_ncs,  0,        igs_ncs,  igs_ncs, igs_ncs, ROT0, "IGS",    "New Champion Skill (v100n)",                0,	layout_igspoker ) /* SU 062 */

GAMEL( 2000, igs_ncs2,  0,        igs_ncs,  igs_ncs, igs_ncs2, ROT0, "IGS",   "New Champion Skill (v100n 2000)",           GAME_IMPERFECT_GRAPHICS, layout_igspoker )

GAMEL( 1998, stellecu,  0,        csk234it, csk234, 0,       ROT0, "Sure",   "Stelle e Cubi (Italy)",                     GAME_NOT_WORKING,	layout_igspoker )




