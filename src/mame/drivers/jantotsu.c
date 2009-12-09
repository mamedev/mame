/*******************************************************************************************

4nin-uchi Mahjong Jantotsu (c) 1983 Sanritsu

driver by David Haywood and Angelo Salese

Notes:
-The 1st/2nd Player tiles on hand are actually shown on different screen sides.The Service
 Mode is for adjusting these screens (to not let the human opponent to read your tiles).

TODO:
-Video buffering? If you coin up, you can see the "credit 1" msg that gets build into the
 video bitmaps...
-According to the flyer, color bitplanes might be wrong on the A-N mahjong charset, might be
 a BTANB however...
-I need schematics / pcb photos (component + solder sides) to understand if the background
 color is hard-wired to the DIP-Switches or there's something else wrong.

============================================================================================
Debug cheats:

c01b-c028 player-1 tiles
c02b-c038 "right" computer tiles
c03b-c048 "up" computer tiles / player-2 tiles
c04b-c058 "left" computer tiles

============================================================================================
MSM samples table (I'm assuming that F is left-right players, M2 is up / player 2 and M1 is player 1):
start |end   |sample    |possible data triggers
------------------------|----------------------
jat-43 (0x0000-0x1fff)
0x0000|0x06ff|rii'chi M1| 0x00
0x0700|0x0bff|rii'chi M2| 0x07
0x0c00|0x0fff|nagare?   |
0x1000|0x13ff|ron M     |
0x1400|0x17ff|kore?     | 0x14
0x1800|0x1bff|kan M1    | 0x18
0x1c00|0x1fff|kan M2    |
jat-42 (0x4000-0x5fff)
0x4000|0x42ff|koi?      |
0x4300|0x4ef0|ippatsu M |
0x5000|0x52ff|pon M     |
0x5300|0x57ff|tsumo M2  | 0x53
0x5800|0x5dff|rii'chi F | 0x58
jat-41 (0x8000-0x9fff)
0x8000|0x87ff|ippatsu F |
0x8800|0x8fff|ryutoku?  | 0x88
0x9000|0x95ff|otoyo?    | 0x90
0x9600|0x9eff|yatta     | 0x96
jat-40 (0xc000-0xdfff)
0xc000|0xc8ff|odaii?    | 0xc0
0xc900|0xccff|tsumo F   | 0xc9
0xcd00|0xcfff|tsumo M1  |
0xd000|0xd3ff|ron F     | 0xd0
0xd400|0xd7ff|pon F     | 0xd4
0xd800|0xdbff|kan F     |
0xdc00|0xdfff|chi       |
------------------------------------------------


============================================================================================

4nin-uchi mahjong Jantotsu
(c)1983 Sanritsu

C2-00159_A

CPU: Z80
Sound: SN76489ANx2 MSM5205
OSC: 18.432MHz

ROMs:
JAT-00.2B (2764)
JAT-01.3B
JAT-02.4B
JAT-03.2E
JAT-04.3E
JAT-05.4E

JAT-40.6B
JAT-41.7B
JAT-42.8B
JAT-43.9B

JAT-60.10P (7051) - color PROM

This game requires special control panel.
Standard mahjong control panel + these 3 buttons.
- 9syu nagare
- Continue Yes
- Continue No

dumped by sayu

*******************************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/msm5205.h"

typedef struct _jantotsu_state jantotsu_state;
struct _jantotsu_state
{
	/* video-related */
	UINT8    *bitmap;
	UINT8    vram_bank, col_bank;

	/* sound-related */
	UINT32   adpcm_pos;
	UINT8    adpcm_idle;
	int      adpcm_data;
	UINT8    adpcm_trigger;

	/* misc */
	UINT8    mux_data;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static VIDEO_START(jantotsu)
{
	jantotsu_state *state = (jantotsu_state *)machine->driver_data;

	state->bitmap = auto_alloc_array(machine, UINT8, 0x8000);
	state_save_register_global_pointer(machine, state->bitmap, 0x8000);
}

static VIDEO_UPDATE(jantotsu)
{
	jantotsu_state *state = (jantotsu_state *)screen->machine->driver_data;
	int x, y, i;
	int count = 0;

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 256; x += 8)
		{
			int pen[4], color;

			for (i = 0; i < 8; i++)
			{
				pen[0] = (state->bitmap[count + 0x0000]) >> (7 - i);
				pen[1] = (state->bitmap[count + 0x2000]) >> (7 - i);
				pen[2] = (state->bitmap[count + 0x4000]) >> (7 - i);
				pen[3] = (state->bitmap[count + 0x6000]) >> (7 - i);

				color  = ((pen[0] & 1) << 0);
				color |= ((pen[1] & 1) << 1);
				color |= ((pen[2] & 1) << 2);
				color |= ((pen[3] & 1) << 3);
				color |= state->col_bank;

				if ((x + i) <= video_screen_get_visible_area(screen)->max_x && (y + 0) < video_screen_get_visible_area(screen)->max_y)
					*BITMAP_ADDR32(bitmap, y, x + i) = screen->machine->pens[color];
			}

			count++;
		}
	}

	return 0;
}

/* banked vram */
static READ8_HANDLER( jantotsu_bitmap_r )
{
	jantotsu_state *state = (jantotsu_state *)space->machine->driver_data;
	return state->bitmap[offset + ((state->vram_bank & 3) * 0x2000)];
}

static WRITE8_HANDLER( jantotsu_bitmap_w )
{
	jantotsu_state *state = (jantotsu_state *)space->machine->driver_data;
	state->bitmap[offset + ((state->vram_bank & 3) * 0x2000)] = data;
}

static WRITE8_HANDLER( bankaddr_w )
{
	jantotsu_state *state = (jantotsu_state *)space->machine->driver_data;

	state->vram_bank = ((data & 0xc0) >> 6); // top 2 bits?

	// looks like the top 2 bits and bottom 2 bits are used..
	// multiple buffers? different read / write ?

	//  printf("%02x\n",data & 0x03);
}

static PALETTE_INIT( jantotsu )
{
	int	bit0, bit1, bit2, r, g, b;
	int	i;

	for (i = 0; i < 0x20; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/*Multiplexer is mapped as 6-bits reads,bits 6 & 7 are always connected to the coin mechs.*/
static READ8_HANDLER( jantotsu_mux_r )
{
	jantotsu_state *state = (jantotsu_state *)space->machine->driver_data;
	UINT8 coin_port = input_port_read(space->machine, "COINS");

	//  printf("%02x\n", state->mux_data);

	switch (state->mux_data)
	{
		case 0x01: return input_port_read(space->machine, "PL1_1") | coin_port;
		case 0x02: return input_port_read(space->machine, "PL1_2") | coin_port;
		case 0x04: return input_port_read(space->machine, "PL1_3") | coin_port;
		case 0x08: return input_port_read(space->machine, "PL1_4") | coin_port;
		case 0x10: return input_port_read(space->machine, "PL2_1") | coin_port;
		case 0x20: return input_port_read(space->machine, "PL2_2") | coin_port;
		case 0x40: return input_port_read(space->machine, "PL2_3") | coin_port;
		case 0x80: return input_port_read(space->machine, "PL2_4") | coin_port;
	}

	return coin_port;
}

static WRITE8_HANDLER( jantotsu_mux_w )
{
	jantotsu_state *state = (jantotsu_state *)space->machine->driver_data;
	state->mux_data = ~data;
}

/*If bits 6 & 7 doesn't return 0x80,the game hangs until this bit is set,
  so I'm guessing that these bits can't be read by the z80 at all but directly
  hard-wired to the video chip. However I need the schematics / pcb snaps and/or
  a side-by-side test (to know if the background colors really works) to be sure. */
static READ8_HANDLER( jantotsu_dsw2_r )
{
	return (input_port_read(space->machine, "DSW2") & 0x3f) | 0x80;
}

static WRITE8_DEVICE_HANDLER( jan_adpcm_w )
{
	jantotsu_state *state = (jantotsu_state *)device->machine->driver_data;

	switch (offset)
	{
		case 0:
			state->adpcm_pos = (data & 0xff) * 0x100;
			state->adpcm_idle = 0;
			msm5205_reset_w(device, 0);
			/* I don't think that this will ever happen, it's there just to be sure
               (i.e. I'll probably never do a "nagare" in my entire life ;-) ) */
			if(data & 0x20)
				popmessage("ADPCM called with data = %02x, contact MAMEdev", data);
//          printf("%02x 0\n", data);
			break;
		/*same write as port 2? MSM sample ack? */
		case 1:
//          state->adpcm_idle = 1;
//          msm5205_reset_w(device, 1);
//          printf("%02x 1\n", data);
			break;
	}
}

static void jan_adpcm_int( const device_config *device )
{
	jantotsu_state *state = (jantotsu_state *)device->machine->driver_data;

	if (state->adpcm_pos >= 0x10000 || state->adpcm_idle)
	{
		//state->adpcm_idle = 1;
		msm5205_reset_w(device, 1);
		state->adpcm_trigger = 0;
	}
	else
	{
		UINT8 *ROM = memory_region(device->machine, "adpcm");

		state->adpcm_data = ((state->adpcm_trigger ? (ROM[state->adpcm_pos] & 0x0f) : (ROM[state->adpcm_pos] & 0xf0) >> 4));
		msm5205_data_w(device, state->adpcm_data & 0xf);
		state->adpcm_trigger ^= 1;
		if (state->adpcm_trigger == 0)
		{
			state->adpcm_pos++;
			if ((ROM[state->adpcm_pos] & 0xff) == 0x70)
				state->adpcm_idle = 1;
		}
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( jantotsu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(jantotsu_bitmap_r, jantotsu_bitmap_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jantotsu_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1") AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0x01, 0x01) AM_READ(jantotsu_dsw2_r) AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("adpcm", jan_adpcm_w)
	AM_RANGE(0x04, 0x04) AM_READWRITE(jantotsu_mux_r, jantotsu_mux_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(bankaddr_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jantotsu )
	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPNAME( 0x1c, 0x10, "Player vs. CPU timer decrement speed") PORT_DIPLOCATION("SW1:3,4,5") //in msecs I suppose
	PORT_DIPSETTING(    0x00, "30")
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x04, "70")
	PORT_DIPSETTING(    0x0c, "90" )
	PORT_DIPSETTING(    0x10, "110" )
	PORT_DIPSETTING(    0x18, "130" )
	PORT_DIPSETTING(    0x14, "150" )
	PORT_DIPSETTING(    0x1c, "170" )
	PORT_DIPNAME( 0xe0, 0x80, "Player vs. Player timer decrement speed" ) PORT_DIPLOCATION("SW1:6,7,8") //in msecs I suppose
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x40, "120" )
	PORT_DIPSETTING(    0x20, "140" )
	PORT_DIPSETTING(    0x60, "160" )
	PORT_DIPSETTING(    0x80, "180" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xa0, "220" )
	PORT_DIPSETTING(    0xe0, "240" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Play BGM") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Background Color" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "Purple" )
	PORT_DIPSETTING(    0x80, "Green" )
	PORT_DIPSETTING(    0x40, "Blue" )
	PORT_DIPSETTING(    0x00, "Black" )

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Mahjong Nagare") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Continue Button Yes") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Continue Button No") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Mahjong Nagare") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Continue Button Yes") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Continue Button No") PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
INPUT_PORTS_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/

static const msm5205_interface msm5205_config =
{
	jan_adpcm_int,	/* interrupt function */
	MSM5205_S64_4B	/* 6 KHz */
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( jantotsu )
{
	jantotsu_state *state = (jantotsu_state *)machine->driver_data;

	state_save_register_global(machine, state->vram_bank);
	state_save_register_global(machine, state->mux_data);
	state_save_register_global(machine, state->adpcm_pos);
	state_save_register_global(machine, state->adpcm_idle);
	state_save_register_global(machine, state->adpcm_data);
	state_save_register_global(machine, state->adpcm_trigger);
}

static MACHINE_RESET( jantotsu )
{
	jantotsu_state *state = (jantotsu_state *)machine->driver_data;

	/*Load hard-wired background color.*/
	state->col_bank = (input_port_read(machine, "DSW2") & 0xc0) >> 3;

	state->vram_bank = 0;
	state->mux_data = 0;
	state->adpcm_pos = 0;
	state->adpcm_idle = 1;
	state->adpcm_data = 0;
	state->adpcm_trigger = 0;
}

static MACHINE_DRIVER_START( jantotsu )

	/* driver data */
	MDRV_DRIVER_DATA(jantotsu_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,18432000/4)
	MDRV_CPU_PROGRAM_MAP(jantotsu_map)
	MDRV_CPU_IO_MAP(jantotsu_io)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_MACHINE_START(jantotsu)
	MDRV_MACHINE_RESET(jantotsu)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_PALETTE_INIT(jantotsu)
	MDRV_PALETTE_LENGTH(0x20)

	MDRV_VIDEO_START(jantotsu)
	MDRV_VIDEO_UPDATE(jantotsu)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76489A, 18432000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn2", SN76489A, 18432000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("adpcm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jantotsu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jat-00.2b", 0x00000, 0x02000, CRC(bed10f86) SHA1(c5ac845b32fa295b0ff205b1401bcc071d604d6e) )
	ROM_LOAD( "jat-01.3b", 0x02000, 0x02000, CRC(cc9312e9) SHA1(b08d38cc58d92378305c015c5a001b4da072188b) )
	ROM_LOAD( "jat-02.4b", 0x04000, 0x02000, CRC(292969f1) SHA1(e7cb93b67296e84ea012fcceb72df712c7e0f135) )
	ROM_LOAD( "jat-03.2e", 0x06000, 0x02000, CRC(7452ff63) SHA1(b258674e77eee5548a065419a3092877957dec66) )
	ROM_LOAD( "jat-04.3e", 0x08000, 0x02000, CRC(734e029f) SHA1(75aa13397847b4db32c41aaa6ff2ac82f16bd7a2) )
	ROM_LOAD( "jat-05.4e", 0x0a000, 0x02000, CRC(1a725e1a) SHA1(1d39d607850f47b9389f41147d4570da8814f639) )

	ROM_REGION( 0x10000, "adpcm", ROMREGION_ERASE00 )
	ROM_LOAD( "jat-43.9b", 0x00000, 0x02000, CRC(3c1d843c) SHA1(7a836e66cad4e94916f0d80a439efde49306a0e1) )
	ROM_LOAD( "jat-42.8b", 0x04000, 0x02000, CRC(3ac3efbf) SHA1(846faea7c7c01fb7500aa33a70d4b54e878c0e41) )
	ROM_LOAD( "jat-41.7b", 0x08000, 0x02000, CRC(ce08ed71) SHA1(8554e5e7ec178f57bed5fbdd5937e3a35f72c454) )
	ROM_LOAD( "jat-40.6b", 0x0c000, 0x02000, CRC(2275253e) SHA1(64e9415faf2775c6b9ab497dce7fda8c4775192e) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jat-60.10p", 0x00, 0x20,  CRC(65528ae0) SHA1(6e3bf27d10ec14e3c6a494667b03b68726fcff14) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, jantotsu,  0,    jantotsu, jantotsu,  0, ROT270, "Sanritsu", "4nin-uchi Mahjong Jantotsu", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
