/***************************************************************************
Super Poker (IGS)
Driver by Mirko Buffoni

TODO:
- Understand how to reset NVRAM
- Map DSW (Operator mode doesn't help)
- Map Leds and Coin counters
***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


/***************************************************************************
                                Video Hardware
***************************************************************************/

class spoker_state : public driver_device
{
public:
	spoker_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8   *bg_tile_ram;
	tilemap_t *bg_tilemap;

	UINT8 *fg_tile_ram;
	UINT8 *fg_color_ram;
	tilemap_t *fg_tilemap;

	int video_enable;
	int nmi_enable, hopper;
	UINT8 igs_magic[2];
	UINT8 out[3];
};

static WRITE8_HANDLER( bg_tile_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	state->bg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	spoker_state *state = machine->driver_data<spoker_state>();
	int code = state->bg_tile_ram[tile_index];
	SET_TILE_INFO(1 + (tile_index & 3), code & 0xff, 0, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	spoker_state *state = machine->driver_data<spoker_state>();
	int code = state->fg_tile_ram[tile_index] | (state->fg_color_ram[tile_index] << 8);
	SET_TILE_INFO(0, code, (4*(code >> 14)+3), 0);
}

static WRITE8_HANDLER( fg_tile_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	state->fg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

static WRITE8_HANDLER( fg_color_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	state->fg_color_ram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

static VIDEO_START(spoker)
{
	spoker_state *state = machine->driver_data<spoker_state>();

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,	8,  32,	128, 8);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,	8,  8,	128, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static VIDEO_UPDATE(spoker)
{
	spoker_state *state = screen->machine->driver_data<spoker_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static CUSTOM_INPUT( hopper_r )
{
	running_machine *machine = field->port->machine;
	spoker_state *state = machine->driver_data<spoker_state>();

	if (state->hopper) return !(machine->primary_screen->frame_number()%10);
	return input_code_pressed(machine, KEYCODE_H);
}

static void show_out(UINT8 *out)
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x", out[0], out[1], out[2]);
#endif
}

static WRITE8_HANDLER( spoker_nmi_and_coins_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	if ((state->nmi_enable ^ data) & (~0xdd))
	{
		logerror("PC %06X: nmi_and_coins = %02x\n",cpu_get_pc(space->cpu),data);
//      popmessage("%02x",data);
	}

	coin_counter_w(space->machine, 0,		data & 0x01);	// coin_a
	coin_counter_w(space->machine, 1,		data & 0x04);	// coin_c
	coin_counter_w(space->machine, 2,		data & 0x08);	// key in
	coin_counter_w(space->machine, 3,		data & 0x10);	// coin out mech

	set_led_status(space->machine, 6,		data & 0x40);	// led for coin out / hopper active

	state->nmi_enable = data;	//  data & 0x80     // nmi enable?

	state->out[0] = data;
	show_out(state->out);
}

static WRITE8_HANDLER( spoker_video_and_leds_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	set_led_status(space->machine, 4,	  data & 0x01);	// start?
	set_led_status(space->machine, 5,	  data & 0x04);	// l_bet?

	state->video_enable	=	  data & 0x40;
	state->hopper			=	(~data)& 0x80;

	state->out[1] = data;
	show_out(state->out);
}

static WRITE8_HANDLER( spoker_leds_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	set_led_status(space->machine, 0, data & 0x01);	// stop_1
	set_led_status(space->machine, 1, data & 0x02);	// stop_2
	set_led_status(space->machine, 2, data & 0x04);	// stop_3
	set_led_status(space->machine, 3, data & 0x08);	// stop
	// data & 0x10?

	state->out[2] = data;
	show_out(state->out);
}

static WRITE8_HANDLER( spoker_magic_w )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	state->igs_magic[offset] = data;

	if (offset == 0)
		return;

	switch(state->igs_magic[0])
	{
		case 0x01:
			break;

		default:
//          popmessage("magic %x <- %04x",igs_magic[0],data);
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", cpu_get_pc(space->cpu), state->igs_magic[0], data);
	}
}

static READ8_HANDLER( spoker_magic_r )
{
	spoker_state *state = space->machine->driver_data<spoker_state>();

	switch(state->igs_magic[0])
	{
		case 0x00:
			if ( !(state->igs_magic[1] & 0x01) )	return input_port_read(space->machine, "DSW1");
			if ( !(state->igs_magic[1] & 0x02) )	return input_port_read(space->machine, "DSW2");
			if ( !(state->igs_magic[1] & 0x04) )	return input_port_read(space->machine, "DSW3");
			if ( !(state->igs_magic[1] & 0x08) )	return input_port_read(space->machine, "DSW4");
			if ( !(state->igs_magic[1] & 0x10) )	return input_port_read(space->machine, "DSW5");
			logerror("%06x: warning, reading dsw with igs_magic[1] = %02x\n", cpu_get_pc(space->cpu), state->igs_magic[1]);
			break;

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", cpu_get_pc(space->cpu), state->igs_magic[0]);
	}

	return 0;
}




static ADDRESS_MAP_START( spoker_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0f3ff ) AM_ROM
	AM_RANGE( 0x0f400, 0x0ffff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( spoker_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs

	AM_RANGE( 0x2000, 0x23ff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE_GENERIC( paletteram )
	AM_RANGE( 0x2400, 0x27ff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE_GENERIC( paletteram2 )

	AM_RANGE( 0x3000, 0x33ff ) AM_RAM_WRITE( bg_tile_w ) AM_BASE_MEMBER( spoker_state,bg_tile_ram )

	AM_RANGE( 0x5000, 0x5fff ) AM_RAM_WRITE( fg_tile_w )  AM_BASE_MEMBER( spoker_state,fg_tile_ram )

	AM_RANGE( 0x6480, 0x6480 ) AM_WRITE( spoker_nmi_and_coins_w )

	AM_RANGE( 0x6481, 0x6481 ) AM_READ_PORT( "SERVICE" )
	AM_RANGE( 0x6482, 0x6482 ) AM_READ_PORT( "COINS" )
	AM_RANGE( 0x6490, 0x6490 ) AM_READ_PORT( "BUTTONS1" )
	AM_RANGE( 0x6491, 0x6491 ) AM_WRITE( spoker_video_and_leds_w )
	AM_RANGE( 0x6492, 0x6492 ) AM_WRITE( spoker_leds_w )
	AM_RANGE( 0x64a0, 0x64a0 ) AM_READ_PORT( "BUTTONS2" )

	AM_RANGE( 0x64b0, 0x64b1 ) AM_DEVWRITE( "ymsnd", ym2413_w )

	AM_RANGE( 0x64c0, 0x64c0 ) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)

	AM_RANGE( 0x64d0, 0x64d1 ) AM_READWRITE( spoker_magic_r, spoker_magic_w )	// DSW1-5

	AM_RANGE( 0x7000, 0x7fff ) AM_RAM_WRITE( fg_color_w ) AM_BASE_MEMBER( spoker_state,fg_color_ram )
ADDRESS_MAP_END




/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( spoker )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
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
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Memory Clear")	// stats, memory
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM( hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
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

static GFXDECODE_START( spoker )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_8x8x6,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, layout_8x32x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, layout_8x32x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, layout_8x32x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6,  0, 16 )
GFXDECODE_END




/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_RESET( spoker )
{
	spoker_state *state = machine->driver_data<spoker_state>();

	state->nmi_enable		=	0;
	state->hopper			=	0;
	state->video_enable	=	1;
}

static INTERRUPT_GEN( spoker_interrupt )
{
	spoker_state *state = device->machine->driver_data<spoker_state>();

	 if (state->nmi_enable & 0x80)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( spoker, spoker_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)	/* HD64180RP8, 8 MHz? */
	MCFG_CPU_PROGRAM_MAP(spoker_map)
	MCFG_CPU_IO_MAP(spoker_portmap)
	MCFG_CPU_VBLANK_INT("screen",spoker_interrupt)

	MCFG_MACHINE_RESET(spoker)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)

	MCFG_GFXDECODE(spoker)
	MCFG_PALETTE_LENGTH(0x400)

	MCFG_VIDEO_START(spoker)
	MCFG_VIDEO_UPDATE(spoker)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_12MHz / 12, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static DRIVER_INIT( spk116it )
{
	int A;
	UINT8 *rom = machine->region("maincpu")->base();


	for (A = 0;A < 0x10000;A++)
	{
		rom[A] ^= 0x02;
		if ((A & 0x0208) == 0x0208) rom[A] ^= 0x20;
		if ((A & 0x0228) == 0x0008) rom[A] ^= 0x20;
		if ((A & 0x04A0) == 0x04A0) rom[A] ^= 0x02;
		if ((A & 0x1208) == 0x1208) rom[A] ^= 0x01;
	}
}


ROM_START( spk115it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v.bin",   0x0000, 0x10000, CRC(df52997b) SHA1(72a76e84aeedfdebd4c6cb47809117a28b5d3892) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6.bin",  0x80000, 0x40000, CRC(f9b027f8) SHA1(c4686a4024062482f9864e0445087e32899fc775) )
	ROM_LOAD( "5.bin",  0x40000, 0x40000, CRC(baca51b6) SHA1(c97322c814729332378b6304a79062fea385ca97) )
	ROM_LOAD( "4.bin",  0x00000, 0x40000, CRC(1172c790) SHA1(43f1d019ecae5c605722e3fe77ae2f022b01260b) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END


ROM_START( spk116it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v.bin",   0x0000, 0x10000, CRC(e44e943a)  SHA1(78e32d07e2be9a452be10735641cbcf269068c55) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6.bin",  0x80000, 0x40000, CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )
	ROM_LOAD( "5.bin",  0x40000, 0x40000, CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "4.bin",  0x00000, 0x40000, CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

GAME( 1993?, spk116it, 0,        spoker, spoker,  spk116it, ROT0, "IGS", "Super Poker (v116IT)", 0 )
GAME( 1993?, spk115it, spk116it, spoker, spoker,  spk116it, ROT0, "IGS", "Super Poker (v115IT)", 0 )
