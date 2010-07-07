/*************************************************************************************************************

                                            -= Astro Corp. CGA Hardware =-

                                       driver by   Luca Elia (l.elia@tin.it)

CPU:    68000
GFX:    ASTRO V01 or V02 or V06
SOUND:  OKI M6295 (AD-65)
OTHER:  EEPROM, Battery

 512 sprites, each made of N x M tiles. Tiles are 16x16x8

-------------------------------------------------------------------------------------------------------------------
Year + Game          PCB ID         CPU                Video      Chips                                   Notes
-------------------------------------------------------------------------------------------------------------------
00  Show Hand        CHE-B50-4002A  68000              ASTRO V01  ASTRO 0001B   (28 pins), pLSI1016-60LJ
00  Wang Pai Dui J.  CHE-B50-4002A  68000              ASTRO V01  MDT2020AP MCU (28 pins), pLSI1016
02  Skill Drop GA    None           JX-1689F1028N      ASTRO V02  pLSI1016-60LJ
05? Zoo              M1.1           ASTRO V102PX-005?  ASTRO V06  ASTRO F02 2005-02-18                    Encrypted
05? Win Win Bingo    M1.2           ASTRO V102PX-006?  ASTRO V06  ASTRO F02 2005-09-17                    Encrypted
-------------------------------------------------------------------------------------------------------------------

To do:

- Find source of level 2 interrupt

*************************************************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

class astrocorp_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, astrocorp_state(machine)); }

	astrocorp_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *   spriteram;
	UINT16 *   paletteram;
	size_t     spriteram_size;

	/* video-related */
	bitmap_t * bitmap;
	UINT16     screen_enable;
	UINT16     draw_sprites;
};

/***************************************************************************
                                Video
***************************************************************************/

static VIDEO_START( astrocorp )
{
	astrocorp_state *state = (astrocorp_state *)machine->driver_data;

	state->bitmap = machine->primary_screen->alloc_compatible_bitmap();

	state_save_register_global_bitmap(machine, state->bitmap);
	state_save_register_global       (machine, state->screen_enable);
	state_save_register_global       (machine, state->draw_sprites);
}

/***************************************************************************
                              Sprites Format

    Offset:    Bits:                  Value:

    0          f--- ---- ---- ----    Show This Sprite
               -edc ba98 7654 3210    X

    1                                 Code

    2          f--- ---- ---- ----    -
               -edc ba98 7654 3210    Y

    3          fedc ba98 ---- ----    X Size
               ---- ---- 7654 3210    Y Size

    If the first two words are zero, the sprite list ends

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	astrocorp_state *state = (astrocorp_state *)machine->driver_data;
	UINT16 *source = state->spriteram;
	UINT16 *finish = state->spriteram + state->spriteram_size / 2;

	for ( ; source < finish; source += 8 / 2 )
	{
		int x, y;
		int xwrap, ywrap;

		int	sx		= source[ 0x0/2 ];
		int	code	= source[ 0x2/2 ];
		int	sy		= source[ 0x4/2 ];
		int	attr	= source[ 0x6/2 ];

		int dimx	= (attr >> 8) & 0xff;
		int dimy	= (attr >> 0) & 0xff;

		if (!sx && !code)
			return;

		if (!(sx & 0x8000))
			continue;

		sx = (sx & 0x1fff) - (sx & 0x2000);
		sy = (sy & 0x1fff) - (sy & 0x2000);

		for (y = 0 ; y < dimy ; y++)
		{
			for (x = 0 ; x < dimx ; x++)
			{
				for (ywrap = 0 ; ywrap <= 0x100 ; ywrap += 0x100)
				{
					for (xwrap = 0 ; xwrap <= 0x200 ; xwrap += 0x200)
					{
						drawgfx_transpen(bitmap,cliprect, machine->gfx[0],
								code, 0,
								0, 0,
								sx + x * 16 - xwrap, sy + y * 16 - ywrap, 0xff);
					}
				}
				code++;
			}
		}
	}
}

static VIDEO_UPDATE(astrocorp)
{
	astrocorp_state *state = (astrocorp_state *)screen->machine->driver_data;

	if (state->screen_enable & 1)
	{
		bitmap_fill(bitmap,cliprect,screen->machine->pens[0xff]);
		copybitmap(bitmap, state->bitmap, 0,0,0,0, cliprect);
	}
	else
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	return 0;
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

static WRITE16_HANDLER( astrocorp_draw_sprites_w )
{
	astrocorp_state *state = (astrocorp_state *)space->machine->driver_data;

	UINT16 old = state->draw_sprites;
	UINT16 now = COMBINE_DATA(&state->draw_sprites);

	if (!old && now)
		draw_sprites(space->machine, state->bitmap, &space->machine->primary_screen->visible_area());
}

static WRITE16_HANDLER( astrocorp_eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		input_port_write(space->machine, "EEPROMOUT", data, 0xff);
	}
}

static WRITE16_DEVICE_HANDLER( astrocorp_sound_bank_w )
{
	if (ACCESSING_BITS_8_15)
	{
		okim6295_device *oki = downcast<okim6295_device *>(device);
		oki->set_bank_base(0x40000 * ((data >> 8) & 1));
//      logerror("CPU #0 PC %06X: OKI bank %08X\n", cpu_get_pc(space->cpu), data);
	}
}

static WRITE16_DEVICE_HANDLER( skilldrp_sound_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		okim6295_device *oki = downcast<okim6295_device *>(device);
		oki->set_bank_base(0x40000 * (data & 1));
//      logerror("CPU #0 PC %06X: OKI bank %08X\n", cpu_get_pc(space->cpu), data);
	}
}

static WRITE16_HANDLER( showhand_outputs_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0,	(data & 0x0004));	// coin counter
		set_led_status(space->machine, 0,	(data & 0x0008));	// you win
		if ((data & 0x0010)) increment_dispensed_tickets(space->machine, 1); // coin out
		set_led_status(space->machine, 1,	(data & 0x0020));	// coin/hopper jam
	}
	if (ACCESSING_BITS_8_15)
	{
		set_led_status(space->machine, 2,	(data & 0x0100));	// bet
		set_led_status(space->machine, 3,	(data & 0x0800));	// start
		set_led_status(space->machine, 4,	(data & 0x1000));	// ? select/choose
		set_led_status(space->machine, 5,	(data & 0x2000));	// ? select/choose
		set_led_status(space->machine, 6,	(data & 0x4000));	// look
	}
//  popmessage("%04X",data);
}

static WRITE16_HANDLER( skilldrp_outputs_w )
{
	// key in          (0001)
	// coin in         (0002)
	// key out         (0004)
	// coin out        (0008)
	// hopper?         (0010)
	// error  lamp     (0020)
	// ticket motor?   (0080)
	// select lamp     (0100)
	// take   lamp     (0400)
	// bet    lamp     (0800)
	// start  lamp     (1000)
	// win / test lamp (4000)
	// ticket?         (8000)

	// account    (5d20 4d20 4520 4420 4020 4000)

	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0,	(data & 0x0001));	// key in  |
		coin_counter_w(space->machine, 0,	(data & 0x0002));	// coin in |- manual shows 1 in- and 1 out- counter
		coin_counter_w(space->machine, 1,	(data & 0x0004));	// key out |
		ticket_dispenser_w(space->machine->device("hopper"), 0, (data & 0x0008)<<4);	// hopper motor?
		//                                  (data & 0x0010)     // hopper?
		set_led_status(space->machine, 0,	(data & 0x0020));	// error lamp (coin/hopper jam: "call attendant")
		ticket_dispenser_w(space->machine->device("ticket"), 0, data & 0x0080);	// ticket motor?
	}
	if (ACCESSING_BITS_8_15)
	{
		// lamps:
		set_led_status(space->machine, 1,	(data & 0x0100));	// select
		set_led_status(space->machine, 2,	(data & 0x0400));	// take
		set_led_status(space->machine, 3,	(data & 0x0800));	// bet
		set_led_status(space->machine, 4,	(data & 0x1000));	// start
		set_led_status(space->machine, 5,	(data & 0x4000));	// win / test
		set_led_status(space->machine, 6,	(data & 0x8000));	// ticket?
	}

//  popmessage("%04X",data);
}

static WRITE16_HANDLER( astrocorp_enable_w )
{
	astrocorp_state *state = (astrocorp_state *)space->machine->driver_data;
	COMBINE_DATA(&state->screen_enable);
//  popmessage("%04X",data);
	if (state->screen_enable & (~1))
		logerror("CPU #0 PC %06X: screen enable = %04X\n", cpu_get_pc(space->cpu), state->screen_enable);
}

static READ16_HANDLER( astrocorp_unk_r )
{
	return 0xffff;	// bit 3?
}

// 5-6-5 Palette: BBBBB-GGGGGG-RRRRR
static WRITE16_HANDLER( astrocorp_palette_w )
{
	astrocorp_state *state = (astrocorp_state *)space->machine->driver_data;
	COMBINE_DATA(&state->paletteram[offset]);
	palette_set_color_rgb(space->machine, offset,
		pal5bit((state->paletteram[offset] >>  0) & 0x1f),
		pal6bit((state->paletteram[offset] >>  5) & 0x3f),
		pal5bit((state->paletteram[offset] >> 11) & 0x1f)
	);
}

static ADDRESS_MAP_START( showhand_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x050000, 0x050fff ) AM_RAM AM_BASE_SIZE_MEMBER(astrocorp_state, spriteram, spriteram_size)
	AM_RANGE( 0x052000, 0x052001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x054000, 0x054001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x058000, 0x058001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x05a000, 0x05a001 ) AM_WRITE(showhand_outputs_w)
	AM_RANGE( 0x05e000, 0x05e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x060000, 0x0601ff ) AM_RAM_WRITE(astrocorp_palette_w) AM_BASE_MEMBER(astrocorp_state, paletteram)
	AM_RANGE( 0x070000, 0x073fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	// battery
	AM_RANGE( 0x080000, 0x080001 ) AM_DEVWRITE("oki", astrocorp_sound_bank_w)
	AM_RANGE( 0x0a0000, 0x0a0001 ) AM_WRITE(astrocorp_enable_w)
	AM_RANGE( 0x0d0000, 0x0d0001 ) AM_READ(astrocorp_unk_r) AM_DEVWRITE8("oki", okim6295_w, 0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( showhanc_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x060000, 0x0601ff ) AM_RAM_WRITE(astrocorp_palette_w) AM_BASE_MEMBER(astrocorp_state, paletteram)
	AM_RANGE( 0x070000, 0x070001 ) AM_DEVWRITE("oki", astrocorp_sound_bank_w)
	AM_RANGE( 0x080000, 0x080fff ) AM_RAM AM_BASE_SIZE_MEMBER(astrocorp_state, spriteram, spriteram_size)
	AM_RANGE( 0x082000, 0x082001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x084000, 0x084001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x088000, 0x088001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x08a000, 0x08a001 ) AM_WRITE(showhand_outputs_w)
	AM_RANGE( 0x08e000, 0x08e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x090000, 0x093fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	// battery
	AM_RANGE( 0x0a0000, 0x0a0001 ) AM_WRITE(astrocorp_enable_w)
	AM_RANGE( 0x0e0000, 0x0e0001 ) AM_READ(astrocorp_unk_r) AM_DEVWRITE8("oki", okim6295_w, 0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( skilldrp_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x03ffff ) AM_ROM
	AM_RANGE( 0x200000, 0x200fff ) AM_RAM AM_BASE_SIZE_MEMBER(astrocorp_state, spriteram, spriteram_size)
	AM_RANGE( 0x202000, 0x202001 ) AM_WRITE(astrocorp_draw_sprites_w)
	AM_RANGE( 0x204000, 0x204001 ) AM_READ_PORT("INPUTS")
	AM_RANGE( 0x208000, 0x208001 ) AM_WRITE(astrocorp_eeprom_w)
	AM_RANGE( 0x20a000, 0x20a001 ) AM_WRITE(skilldrp_outputs_w)
	AM_RANGE( 0x20e000, 0x20e001 ) AM_READ_PORT("EEPROMIN")
	AM_RANGE( 0x380000, 0x3801ff ) AM_RAM_WRITE(astrocorp_palette_w) AM_BASE_MEMBER(astrocorp_state, paletteram)
	AM_RANGE( 0x400000, 0x400001 ) AM_WRITE(astrocorp_enable_w)
	AM_RANGE( 0x500000, 0x507fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	// battery
	AM_RANGE( 0x580000, 0x580001 ) AM_DEVWRITE("oki", skilldrp_sound_bank_w)
	AM_RANGE( 0x600000, 0x600001 ) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( showhand )
	PORT_START("INPUTS")	// 54000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1     )	PORT_IMPULSE(1)	// coin
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON5   )	PORT_NAME("Payout")	PORT_CODE(KEYCODE_F1) // payout (must be 0 on startup)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// ?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START2    )	PORT_NAME("Bet / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON3   )	PORT_NAME("Look / Small")
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW )	// settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// ?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_SPECIAL   )	// coin sensor
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_BUTTON2   )	PORT_NAME("Yes / Big")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_BUTTON4   )	PORT_NAME("Hold1")	// HOLD1 in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_BUTTON1   )	PORT_NAME("Select")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_START1    )	PORT_NAME("Start / Take")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SERVICE1  )	PORT_NAME("Reset Settings")	// when 1 in test mode: reset settings (must be 0 on startup)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// ?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN2     )	// key in
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SPECIAL   )	// coin sensor

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static INPUT_PORTS_START( showhanc )
	PORT_START("INPUTS")	// 84000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1     )	PORT_IMPULSE(1)	// coin
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON5   )	PORT_NAME("Payout")	PORT_CODE(KEYCODE_F1) // payout (must be 0 on startup)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START2    )	PORT_NAME("Bet / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1   )	PORT_NAME("Select")
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW )	// settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START1    )	PORT_NAME("Start / Take")	// HOLD1 in test mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_BUTTON3   )	PORT_NAME("Look / Small / Exit")	// HOLD5 in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON4   )	PORT_NAME("Hold2")	// HOLD2 in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_BUTTON2   )	PORT_NAME("Yes / Big")	// HOLD4 in test mode
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SERVICE1  )	PORT_NAME("Reset Settings")	// when 1 in test mode: reset settings (must be 0 on startup)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL   )	// must be 0 for inputs to work
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN2     )	PORT_IMPULSE(1)	// key in (shows an error)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static INPUT_PORTS_START( skilldrp )
	PORT_START("INPUTS")	// 204000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         )	PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_GAMBLE_TAKE   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START3        )	PORT_NAME("Select / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START1        )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_OTHER         )	PORT_CODE(KEYCODE_T) PORT_NAME("Ticket Out")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_START2        )	PORT_NAME("Bet")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("ticket", ticket_dispenser_line_r)	// ticket sw
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK   )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("hopper", ticket_dispenser_line_r)	// hopper sw
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_16x16x8 =
{
	16, 16,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( astrocorp )
	GFXDECODE_ENTRY("sprites", 0, layout_16x16x8, 0, 1)
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static const UINT16 showhand_default_eeprom[15] =	{0x0001,0x0007,0x000a,0x0003,0x0000,0x0009,0x0003,0x0000,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000};

static MACHINE_DRIVER_START( showhand )

	/* driver data */
	MDRV_DRIVER_DATA(astrocorp_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_20MHz / 2)
	MDRV_CPU_PROGRAM_MAP(showhand_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_EEPROM_93C46_ADD("eeprom")
	MDRV_EEPROM_DATA(showhand_default_eeprom, sizeof(showhand_default_eeprom))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58.846)	// measured on pcb
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(astrocorp)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(astrocorp)
	MDRV_VIDEO_UPDATE(astrocorp)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_20MHz/20, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( showhanc )
	MDRV_IMPORT_FROM( showhand )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(showhanc_map)
MACHINE_DRIVER_END


static INTERRUPT_GEN( skilldrp_irq )
{
	switch (cpu_getiloops(device))
	{
		case 0:	cpu_set_input_line(device, 4, HOLD_LINE);	break;	// sprites, sound, i/o
		case 1:	cpu_set_input_line(device, 2, HOLD_LINE);	break;	// palette
	}
}

static MACHINE_DRIVER_START( skilldrp )

	/* driver data */
	MDRV_DRIVER_DATA(astrocorp_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)	// JX-1689F1028N GRX586.V5
	MDRV_CPU_PROGRAM_MAP(skilldrp_map)
	MDRV_CPU_VBLANK_INT_HACK(skilldrp_irq, 2)

	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_EEPROM_93C46_ADD("eeprom")

	MDRV_TICKET_DISPENSER_ADD("ticket", 200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW )
	MDRV_TICKET_DISPENSER_ADD("hopper", 200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58.846)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 0x100)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x200-1, 0, 0xf0-1)

	MDRV_GFXDECODE(astrocorp)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(astrocorp)
	MDRV_VIDEO_UPDATE(astrocorp)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_24MHz/24, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Show Hand
(C) 2000? Astro Corp.

PCB CHE-B50-4002A 88 94V-0 0002

CPU     1x MC68HC000FN12 (main)
        1x pLSI1016-60LJ (main)
        1x ASTRO V01 0005 (custom)
        1x AD-65 (equivalent to OKI6295)(sound)
        1x ASTRO 0001B (custom)
        1x oscillator 20.000MHz
        1x oscillator 25.601712MHz

ROMs    2x 27C512 (1,2)
        2x M27C801 (3,4)
        1x M27C4001 (5)
        1x 93C46 (not dumped)

Note    1x 28x2 JAMMA edge connector
        1x 18x2 edge connector
        1x 10x2 edge connector
        1x pushbutton
        1x trimmer (volume)
        1x 2x2 switches dip

Hardware info by f205v

***************************************************************************/

ROM_START( showhand )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-8.even.u16", 0x00000, 0x10000, CRC(cf34bf0d) SHA1(72ad7ca63ef89451b2572d64cccfa764b9d9b353) )
	ROM_LOAD16_BYTE( "2-8.odd.u17",  0x00001, 0x10000, CRC(dd031c36) SHA1(198d0e685dd2d824a04c787f8a17c173efa272d9) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "4.even.u26", 0x000000, 0x100000, CRC(8a706e42) SHA1(989688ee3a5e4fc11fb502e43c9d6012488982ee) )
	ROM_LOAD16_BYTE( "3.odd.u26",  0x000001, 0x100000, CRC(a624b750) SHA1(fc5b09f8a10cba5fb2474e1edd62a0400177a5ad) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5", 0x00000, 0x80000, CRC(e6987122) SHA1(fb3e7c2399057c64b5c496a393f6f22a1e54c844) )
ROM_END

/***************************************************************************

Show Hand
Astro Corp, 199?

PCB Layout
----------
CHE-B50-4002A
|----------------------------------------|
|     LATTICE    JAMMA   SW   VOL UPC1242|
|     PLSI1016           U26             |
|                6264                 U43|
|                        U25    M6295    |
|     68000                              |
|                U17    |-----|          |
|                       |ASTRO|          |
|                U16    |V01  |          |
|                       |-----|MDT2020AP/3V
|                6264           20MHz    |
|DSW(2)                      26.601712MHz|
|93C46           6116  6116    KM681000  |
|BATTERY         6116  6116    KM681000  |
|----------------------------------------|
Notes:
      68000 clock - 10.000MHz [20/2]
      M6295 clock - 1.000MHz [20/20], pin 7 HIGH
      VSync - 58.846Hz
      HSync - 15.354kHz

Hardware info by Guru

***************************************************************************/

ROM_START( showhanc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1even.u16", 0x00000, 0x10000, CRC(d1295bdb) SHA1(bb035ee89b21368fb11c3b9cd23164b68feb84bd) )
	ROM_LOAD16_BYTE( "2odd.u17",  0x00001, 0x10000, CRC(bbca78e7) SHA1(a163569acad8d6b8821602ce24013fc46887aba9) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "4even.u26", 0x00000, 0x100000, CRC(285375e0) SHA1(63b47105f0751c65e528139074f5b342450495ba) )
	ROM_LOAD16_BYTE( "3odd.u25",  0x00001, 0x100000, CRC(b93e3a91) SHA1(5192375c32518532e08bddfe000efdee587e1ecc) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5.u43", 0x00000, 0x80000, CRC(d6b70f02) SHA1(5a94680594c1f06196fe3bcf7faf56e2ed576f01) )
ROM_END

/***************************************************************************

Skill Drop Georgia

"Sep 13 2002 09:17:54" in code rom and AAI276832 on sticker.

No specific PCB model or numer....

 Astro V02 0022 160pin PQFP ("ASTRO02" silkscreened under chip)
 JX-1689F1028N GRC586.V5 (68K core, has direct connection to program roms)
 Lattice IspLSI 1016 60LJ socketted FPGA
 OKI 6295 clone chip (AD-65 or U6295)

EEPROM Atmel 93C46
Battery 3.6V
OSC    24.000MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                                                     |
+-+             +------+                                |
|      ULN2003A |IspLSI| UT6264CPC     ROM#4*           |
|  ULN2003A     | 1016 |             +---------+        |
|8              +------++----------+ |ROM#6 U26|        |
|                       |ROM#2  U20| +---------+  +----+|
|L        +-------+     +----------+   ROM#3*     |6295||
|i        | JX-   |                               +----+|
|n        | 1689F | +--------------+                    |
|e        | 1028N | |  ROM#7 U100  |                    |
|r        +-------+ +--------------+                    |
|                                    +----------+   +---+
|C                      +----------+ |          |   |   |
|o                      |ROM#1  U21| |  Astro   |   | R |
|n                      +----------+ |  V02     |   | O |
|n                                   |  0022    |   | M |
|e                                   +----------+   | # |
|c                       UT6264CPC                  | 5 |
|t                                                  |   |
|o                                                  +---+
|r     93C46              6116      6116      RAM1      |
|                                                  24MHz|
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#1 & ROM#2 are 32pin sockets
ROM#7 is a 40pin socket
ROM#3 & ROM#4 at U27 & U25 are optional unpopulated 32pin sockets and overlap with ROM#6
ROM#6 is 29F1610MC flash rom

RAM1 are SEC KM681000BLG-7L RAM chips

***************************************************************************/

ROM_START( skilldrp )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "7-skill_drop_g1.0s.u100", 0x00000, 0x40000, CRC(f968b783) SHA1(1d693b1d460e659ca94aae8625ea26e120053f84) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(4fdac800) SHA1(bcafceb6c34866c474714347e23f9e819b5fcfa6) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-skill_drop", 0x00000, 0x80000, CRC(a479e06d) SHA1(ee690d39188b8a43652c4aa5bf8267c1f6632d2f) ) /* No chip location just "ROM#5" silkscreened under socket */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "skilldrp.u6", 0x00, 0x80, CRC(57886a3d) SHA1(bad8fa2ec2262ccb5ef8ec50959aec3f3bf8b90b) )
ROM_END

/***************************************************************************

Win Win Bingo

ASTRO M1.2 PCB:
 Astro V06 0430 160pin PQFP ("ASTRO02" silkscreened under chip)
   Boards are known to have either Astro V06 0430 or Astro V07 0610
 Astro V102PX-006 at U10 (68K core, has direct connection to program roms)
 Astro F02 2005-09-17 socketted FPGA type chip (used for encryption?)
 OKI 6295 clone chip (AD-65 or U6295)

RAM:
 2 x HMI HM62C64P-70
 4 x HMI HM6116P-70
 2 x BSI BS62LV1023SC-70

EEPROM 93C46
Battery 3.6V
OSC    24MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                    24MHz        +---------+         |
+-+          +-------+              |ROM#4 U30|         |
|            | Astro |   HM62C64P   +---------+         |
|            |  F2   | +----------+ +---------+         |
|8           |       | |ROM#2  U25| |ROM#3 U26|         |
|   ULN2003A +-------+ +----------+ +---------+         |
|L                                                      |
|i  ULN2003A +------+                                   |
|n           |Astro |      ROM#7*                 +----+|
|e           |V102PX|                             |6295||
|r           +------+                             +----+|
|                                    +----------+       |
|C                     +----------+  |          |   +---+
|o                     |ROM#1  U26|  |  Astro   |   |   |
|n                     +----------+  |  V06     |   | R |
|n                                   |  0430    |   | O |
|e                                   +----------+   | M |
|c                       HM62C64P                   | # |
|t                                               J1 | 5 |
|o            93C46                                 |   |
|r                        6116      6116      RAM1  +---+
|                                                       |
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#1 & ROM#2 are SYNCMOS F29C51001T
ROM#3 is a 29F1610MC flash rom
ROM#4 is a 29F1610MC flash rom (optionally populated based on game)
ROM#5 is a MX 27C4000PC-12

RAM1 are BSI BS62LV1023SC-70 RAM chips

J1 is an 2 pin connector, unknown purpose

***************************************************************************/

ROM_START( winbingo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u31", 0x00000, 0x20000, CRC(964cfdb5) SHA1(01109466e07f5e906be300bc69310171d34f2e6c) )
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(4ebeec72) SHA1(c603265e6319cff94a0c75017a12c6d86787f906) )

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASE )
	ROM_LOAD( "mxf291610mc.u26",          0x000000, 0x200000, CRC(ad1f61e7) SHA1(845aa01d49c50bcadaed16d76c0dd9131a425b46) )
	ROM_LOAD( "mx29f1610mc.u30.bad.dump", 0x200000, 0x0a0000, BAD_DUMP CRC(6da439c5) SHA1(6afc0c800fe57b9b34ca317f4d1c040b11d3d988) ) /* Should be same as U30 below?? */

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000pc.u35", 0x00000, 0x80000, CRC(445d81c0) SHA1(cacb9c262740c31ea42f406e9f960a1edd1b3ead) )
ROM_END

ROM_START( winbingoa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u31", 0x00000, 0x20000, CRC(c33676c3) SHA1(9f5b7d05d187cf59948a572f80c55cb8fa1f656f) )
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(43c7b2d8) SHA1(16ee79c34b7c485dfccecdf3e0ae9f18f8a20150) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(ad1f61e7) SHA1(845aa01d49c50bcadaed16d76c0dd9131a425b46) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(31613d99) SHA1(1c720f8d981c3e9cb9d9b3b27eb95e7f72ccfc93) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000pc.u35", 0x00000, 0x80000, CRC(e48ed57d) SHA1(11995b90e70e010b292ba9db2da0af4ebf795c1a) )
ROM_END

/***************************************************************************

Zoo by Astro

ASTRO M1.1 PCB:
 Astro V06 0430 160pin PQFP ("ASTRO02" silkscreened under chip)
 Astro V102PX-005 T042652846 at U10 (68K core, has direct connection to program roms)
 Astro F02 2005-02-18 socketted FPGA type chip (used for encryption?)
 OKI 6295 clone chip (AD-65 or U6295)

RAM:
 2 x KTC KT76C88-70LL
 4 x HM6116L-70
 2 x BSI BS62LV1025SC-70

EEPROM 93C46
Battery 3.6V
OSC    26.824MHz

PC1 is a push button for test mode
VR1 is for sound volume


ZOO Z0.02.D at both U25 & U26 are Winbond W27C512 roms and are program code
5 ZOO is a MX 27C4000 rom and is the sample rom
29F1610mc at U26 (yes "U26" is present twice on the PCB) are the graphics

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                   26.824MHz     +---------+         |
+-+          +-------+              |ROM#4 U30|         |
|            | Astro |   KT76C88    +---------+         |
|            |  F2   | +----------+ +---------+         |
|8           |       | |ROM#2  U25| |ROM#3 U26|         |
|   ULN2003A +-------+ +----------+ +---------+         |
|L                                                      |
|i  ULN2003A +------+                                   |
|n           |Astro |      ROM#7*                 +----+|
|e           |V102PX|                             |6295||
|r           +------+                             +----+|
|                                    +----------+       |
|C                     +----------+  |          |   +---+
|o                     |ROM#1  U26|  |  Astro   |   |   |
|n                     +----------+  |  V06     |   | R |
|n                                   |  0430    |   | O |
|e                                   +----------+   | M |
|c                       KT76C88                    | # |
|t                                               J1 | 5 |
|o            93C46                                 |   |
|r                        6116      6116      RAM1  +---+
|                                                       |
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#3 is a 29F1610MC flash rom
ROM#4 is a 29F1610MC flash rom (optionally populated based on game)

RAM1 are BSI BS62LV1025SC-70 RAM chips

J1 is an 2 pin connector, unknown purpose

***************************************************************************/

ROM_START( zoo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "zoo_zo.02.d.u25", 0x00000, 0x10000, CRC(8566aa21) SHA1(319192e2074f3bdda6001d8e9a4b97e98826d7ce) )
	ROM_LOAD16_BYTE( "zoo_zo.02.d.u26", 0x00001, 0x10000, CRC(1a3be45a) SHA1(877be4c9e8d5e7c4644e7bcb9a6729443ed772a4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "29f1610mc.u26", 0x000000, 0x200000, CRC(f5cfd915) SHA1(ec869b47d0762102509dcfc1349d94340037fad5) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_zoo", 0x00000, 0x80000, CRC(b0c9f7aa) SHA1(99345ba0f8da3907f26c9bd29d70135f3ab7cd60) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "zoo_93c46", 0x00, 0x80, CRC(0053fcc4) SHA1(e67a495f9586dd3946f79d50506fba1ae913f6ec) )
ROM_END



static DRIVER_INIT( showhand )
{
#if 0
	UINT16 *rom = (UINT16*)memory_region(machine, "maincpu");

	rom[0x0a1a/2] = 0x6000;	// hopper jam

	rom[0x1494/2] = 0x4e71;	// enable full test mode
	rom[0x1496/2] = 0x4e71;	// ""
	rom[0x1498/2] = 0x4e71;	// ""

	rom[0x12f6/2] = 0x6000;	// rom error
	rom[0x4916/2] = 0x6000;	// rom error
#endif
}

static DRIVER_INIT( showhanc )
{
#if 0
	UINT16 *rom = (UINT16*)memory_region(machine, "maincpu");

	rom[0x14d4/2] = 0x4e71;	// enable full test mode
	rom[0x14d6/2] = 0x4e71;	// ""
	rom[0x14d8/2] = 0x4e71;	// ""

	rom[0x139c/2] = 0x6000;	// rom error
#endif
}

GAME( 2000,  showhand,  0,        showhand, showhand, showhand, ROT0, "Astro Corp.", "Show Hand (Italy)",     GAME_SUPPORTS_SAVE )
GAME( 2000,  showhanc,  showhand, showhanc, showhanc, showhanc, ROT0, "Astro Corp.", "Wang Pai Dui Jue",      GAME_SUPPORTS_SAVE )
GAME( 2002,  skilldrp,  0,        skilldrp, skilldrp, 0,        ROT0, "Astro Corp.", "Skill Drop Georgia",    GAME_SUPPORTS_SAVE )
GAME( 2005?, winbingo,  0,        showhand, showhand, 0,        ROT0, "Astro Corp.", "Win Win Bingo (set 1)", GAME_NOT_WORKING )
GAME( 2005?, winbingoa, winbingo, showhand, showhand, 0,        ROT0, "Astro Corp.", "Win Win Bingo (set 2)", GAME_NOT_WORKING )
GAME( 2005?, zoo,       0,        showhand, showhand, 0,        ROT0, "Astro Corp.", "Zoo",                   GAME_NOT_WORKING )
