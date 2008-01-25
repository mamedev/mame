/*
    Sega System16 Hardware
    major cleanup in progress - still a lot to do!

    see video/system16.c for more information

Changes:

04/28/04  Charles MacDonald
- Added MSM5205 sample playback to tturfbl.

03/17/04
- Added correctly dumped ROM set for eswat to replace the old one. Game is encrypted and unplayable.
- Moved Ace Attacker here from System 18 driver. Game is encrypted and unplayable.
- Added sound support for tturf, tturfu, tturfbl (no samples), fpointbl, fpointbj
- Fixed toryumon RAM test
- Cleaned up timscannr, toryumon drivers
03/11/04
- Cleaned up riotcity, aurail, altbeast, bayroute drivers
- Added missing coin control to sys16_coinctrl_w
- Removed 'extra' RAM in some drivers and replaced with sys16_tilebank_w

To do:
- tturf and tturfu have some invalid data written to the UPD7759. I think this causes the 'pop' sound when level 1 starts.
- tturfbl has some serious problems with tilemap scrolling
- bayroute has a bad sprite in the door of the level 1 boss
- altbeast writes to mirrored sprite RAM at $441000
- tturfu has bad single sprite frame when main character walks
- dduxbl has some 'stuck' sprites

Notes:
- toryumon RAM test accesses mirrored work RAM. Maybe there's a better way to support this than using AM_MASK.
- I separated the fpoint and fpointbl,fpointbj machine drivers as the latter two have different sound hardware,
  but the original does not. I think this creates a dependancy where fpointbl.zip needs flpoint.001 from fpointbj.zip,
  as fpointbl uses fpoint ROMs (it's parent), it's own, and the sound ROM from fpointbj.
  So add fpoint.001 to fpointbl.zip for it to work.
  I made fpointbl the parent of fpointbj so it would use the proper memory map for the sound hardware.
*/

/***************************************************************************/
/*
  ASTORMBL
          3. In the ending, the 3 heroes are floating into a half bubble. (see picture).
          Also colour problems during ending as well.
          4. In the later Shooting gallery stage (like inside the car shop and the factory (mission 3)),
          there is some garbage graphics (sprite of death monsters that appear where they should not)

    working:
        Alex Kidd
        Alien Storm (bootleg)
        Alien Syndrome
        Altered Beast (Ver 1)
        Altered Beast (Ver 2)   (No Sound)
        Atomic Point            (No Sound)
        Aurail                  (Speech quality sounds poor)
        Aurail (317-0168)
        Bay Route
        Body Slam
        Dump Matsumoto (Japan, Body Slam)
        Dynamite Dux (bootleg)
        Enduro Racer (bootleg)
        Enduro Racer (custom bootleg)
        E-Swat (bootleg)
        Fantasy Zone (Old Ver.)
        Fantasy Zone (New Ver.)
        Flash Point  (bootleg)
        Golden Axe (Ver 1)
        Golden Axe (Ver 2)
        Hang-on
        Heavyweight Champ: some minor graphics glitches
        Major League: No game over.
        Moonwalker (bootleg): Music Speed varies
        Outrun (set 1)
        Outrun (set 2)
        Outrun (custom bootleg)
        Passing Shot (bootleg)
        Passing Shot (4 player bootleg)
        Quartet: Glitch on highscore list
        Quartet (Japan): Glitch on highscore list
        Quartet 2: Glitch on highscore list
        Riot City
        SDI
        Shadow Dancer
        Shadow Dancer (Japan)
        Shinobi
        Shinobi (Sys16A Bootleg?)
        Space Harrier
        Super Hangon (bootleg)
        Tetris (bootleg)
        Time Scanner
        Toryumon
        Tough Turf (Japan)          (No Sound)
        Tough Turf (US)             (No Sound)
        Tough Turf (bootleg)    (No Speech Roms)
        Wonderboy 3 - Monster Lair
        Wonderboy 3 - Monster Lair (bootleg)
        Wrestle War

    not really working:
        Shadow Dancer (bootleg)

    protected:
        Alex Kidd (jpn?)
        Alien Syndrome
        Alien Syndrome
        Alien Syndrome (Japan)
        Alien Storm
        Alien Storm (2 Player)
        Bay Route (317-0116)
        Bay Route (protected bootleg 1)
        Bay Route (protected bootleg 2)
        Enduro Racer
        E-Swat
        Flash Point
        Golden Axe (Ver 1 317-0121 Japan)
        Golden Axe (Ver 2 317-0110)
        Golden Axe (Ver 2 317-0122)
        Golden Axe (protected bootleg)
        Jyuohki (Japan, altered beast)
        Moonwalker (317-0158)
        Moonwalker (317-0159)
        Passing Shot (317-0080)
        Shinobi (Sys16B 317-0049)
        Shinobi (Sys16A 317-0050)
        SDI (Japan, old version)
        Super Hangon
        Tetris (Type A)
        Tetris (Type B 317-0092)
        Wonderboy 3 - Monster Lair (317-0089)

    protected (No driver):
        Ace Attacker
        Action Fighter
        Bloxeed
        Clutch Hitter
        Cotton (Japan)
        Cotton
        DD Crew
        Dunk Shot
        Excite League
        Laser Ghost
        MVP
        Ryukyu
        Super Leagu
        Turbo Outrun
        Turbo Outrun (Set 2)
*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/i8039/i8039.h"
#include "system16.h"
#include "cpu/m68000/m68000.h"
#include "machine/fd1094.h"
#include "sound/msm5205.h"
#include "sound/2151intf.h"
#include "sound/upd7759.h"

void fd1094_machine_init(void);
void fd1094_driver_init(void (*set_decrypted)(UINT8 *));

static void sys16_video_config(void (*update)(void), int sprxoffs, const int *bank)
{
	static const int bank_default[16] =
	{
		0x0,0x1,0x2,0x3,
		0x4,0x5,0x6,0x7,
		0x8,0x9,0xa,0xb,
		0xc,0xd,0xe,0xf
	};

	sys16_update_proc = update;
	sys16_sprxoffset = sprxoffs;
	sys16_obj_bank = bank ? bank : bank_default;
}

/***************************************************************************/

// 7751 emulation
WRITE8_HANDLER( sys16_7751_audio_8255_w );
 READ8_HANDLER( sys16_7751_audio_8255_r );
 READ8_HANDLER( sys16_7751_sh_rom_r );
 READ8_HANDLER( sys16_7751_sh_t1_r );
 READ8_HANDLER( sys16_7751_sh_command_r );
WRITE8_HANDLER( sys16_7751_sh_dac_w );
WRITE8_HANDLER( sys16_7751_sh_busy_w );
WRITE8_HANDLER( sys16_7751_sh_offset_a0_a3_w );
WRITE8_HANDLER( sys16_7751_sh_offset_a4_a7_w );
WRITE8_HANDLER( sys16_7751_sh_offset_a8_a11_w );
WRITE8_HANDLER( sys16_7751_sh_rom_select_w );

/***************************************************************************/

static UINT16 coinctrl;

static WRITE16_HANDLER( sys16_3d_coinctrl_w )
{
	if( ACCESSING_LSB )
{
		coinctrl = data&0xff;
		sys16_refreshenable = coinctrl & 0x10;
		coin_counter_w(0,coinctrl & 0x01);
		/* bit 6 is also used (0 in fantzone) */

		/* Hang-On, Super Hang-On, Space Harrier, Enduro Racer */
		set_led_status(0,coinctrl & 0x04);

		/* Space Harrier */
		set_led_status(1,coinctrl & 0x08);
	}
}

static INTERRUPT_GEN( sys16_interrupt )
{
	cpunum_set_input_line(machine, 0, 4, HOLD_LINE); /* Interrupt vector 4, used by VBlank */
}



/***************************************************************************/
/*
    Tough Turf (Datsu bootleg) sound emulation

    Memory map

    0000-7fff : ROM (fixed, tt014d68 0000-7fff)
    8000-bfff : ROM (banked)
    e000      : Bank control
    e800      : Sound command latch
    f000      : MSM5205 sample data buffer
    f800-ffff : Work RAM

    Interrupts

    IRQ = Read sound command from $E800
    NMI = Copy data from fixed/banked ROM to $F000

    Bank control values

    00 = tt014d68 8000-bfff
    01 = tt014d68 c000-ffff
    02 = tt0246ff 0000-3fff
    03 = tt0246ff 4000-7fff
    04 = tt0246ff 8000-bfff

    The sample sound codes in the sound test are OK, but in-game sample playback is bad.
    There seems to be more data in the high bits of the ROM bank control word which may be related.
*/

static int sample_buffer = 0;
static int sample_select = 0;

static WRITE8_HANDLER( tturfbl_msm5205_data_w )
{
	sample_buffer = data;
}

static void tturfbl_msm5205_callback(int data)
{
	MSM5205_data_w(0, (sample_buffer >> 4) & 0x0F);
	sample_buffer <<= 4;
	sample_select ^= 1;
	if(sample_select == 0)
		cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

static const struct MSM5205interface tturfbl_msm5205_interface =
{
	tturfbl_msm5205_callback,
	MSM5205_S48_4B
};


static UINT8 *tturfbl_soundbank_ptr = NULL;		/* Pointer to currently selected portion of ROM */

static READ8_HANDLER( tturfbl_soundbank_r )
{
	if(tturfbl_soundbank_ptr) return tturfbl_soundbank_ptr[offset & 0x3fff];
	return 0x80;
}

static WRITE8_HANDLER( tturfbl_soundbank_w )
{
	UINT8 *mem = memory_region(REGION_CPU2);

	switch(data)
	{
		case 0:
			tturfbl_soundbank_ptr = &mem[0x18000]; /* tt014d68 8000-bfff */
			break;
		case 1:
			tturfbl_soundbank_ptr = &mem[0x1C000]; /* tt014d68 c000-ffff */
			break;
		case 2:
			tturfbl_soundbank_ptr = &mem[0x20000]; /* tt0246ff 0000-3fff */
			break;
		case 3:
			tturfbl_soundbank_ptr = &mem[0x24000]; /* tt0246ff 4000-7fff */
			break;
		case 4:
			tturfbl_soundbank_ptr = &mem[0x28000]; /* tt0246ff 8000-bfff */
			break;
		case 8:
			tturfbl_soundbank_ptr = mem;
			break;
		default:
			tturfbl_soundbank_ptr = NULL;
			logerror("Invalid bank setting %02X (%04X)\n", data, activecpu_get_pc());
			break;
	}
}

static ADDRESS_MAP_START( tturfbl_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(tturfbl_soundbank_r)
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tturfbl_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(MWA8_NOP) /* ROM bank */
	AM_RANGE(0xe000, 0xe000) AM_WRITE(tturfbl_soundbank_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(tturfbl_msm5205_data_w)
	AM_RANGE(0xf800, 0xffff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tturfbl_sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x01, 0x01) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x80, 0x80) AM_READ(MRA8_NOP)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tturfbl_sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x80, 0x80) AM_WRITE(MWA8_NOP)
ADDRESS_MAP_END

/*******************************************************************************/

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xf800, 0xffff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x01, 0x01) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0xc0, 0xc0) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2151_data_port_0_w)
ADDRESS_MAP_END


// 7759
static ADDRESS_MAP_START( sound_readmem_7759, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xdfff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END


static WRITE8_HANDLER( upd7759_bank_w ) //*
{
	int offs, size = memory_region_length(REGION_CPU2) - 0x10000;

	upd7759_reset_w(0, data & 0x40);
	offs = 0x10000 + (data * 0x4000) % size;
	memory_set_bankptr(1, memory_region(REGION_CPU2) + offs);
}


static ADDRESS_MAP_START( sound_writeport_7759, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(upd7759_bank_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(upd7759_0_port_w)
ADDRESS_MAP_END


static WRITE16_HANDLER( sound_command_w )
{
	if( ACCESSING_LSB )
{
		soundlatch_w( 0,data&0xff );
		cpunum_set_input_line(Machine, 1, 0, HOLD_LINE );
	}
}

static WRITE16_HANDLER( sound_command_nmi_w )
{
	if( ACCESSING_LSB )
{
		soundlatch_w( 0,data&0xff );
		cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
	}
}

//static UINT16 coinctrl;


static WRITE16_HANDLER( sys16_coinctrl_w )
{
	if( ACCESSING_LSB )
{
		coinctrl = data&0xff;
		sys16_refreshenable = coinctrl & 0x20;
		set_led_status(1,coinctrl & 0x08);
		set_led_status(0,coinctrl & 0x04);
		coin_counter_w(1,coinctrl & 0x02);
		coin_counter_w(0,coinctrl & 0x01);
		/* bit 6 is also used (1 most of the time; 0 in dduxbl, sdi, wb3;
           tturf has it normally 1 but 0 after coin insertion) */
		/* eswat sets bit 4 */
	}
}

/***************************************************************************/

static MACHINE_DRIVER_START( system16 )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 10000000)
	MDRV_CPU_VBLANK_INT(sys16_interrupt,1)

	MDRV_CPU_ADD_TAG("sound", Z80, 4000000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(sys16)
	MDRV_PALETTE_LENGTH(2048*ShadowColorsMultiplier)

	MDRV_VIDEO_START(system16)
	MDRV_VIDEO_UPDATE(system16)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD_TAG("2151", YM2151, 4000000)
	MDRV_SOUND_ROUTE(0, "left", 0.32)
	MDRV_SOUND_ROUTE(1, "right", 0.32)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( system16_7759 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16)

	MDRV_CPU_MODIFY("sound")
	MDRV_CPU_PROGRAM_MAP(sound_readmem_7759,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport_7759)

	/* sound hardware */
	MDRV_SOUND_ADD_TAG("7759", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_CONFIG(sys16_upd7759_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.48)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.48)
MACHINE_DRIVER_END



/***************************************************************************/

static WRITE16_HANDLER( sys16_tilebank_w )
{
	if(ACCESSING_LSB)
	{
		switch(offset & 1)
		{
			case 0:
				sys16_tile_bank0 = data & 0x0F;
				break;
			case 1:
				sys16_tile_bank1 = data & 0x0F;
				break;
		}
	}
}

static void set_tile_bank( int data )
{
	sys16_tile_bank1 = data&0xf;
	sys16_tile_bank0 = (data>>4)&0xf;
}

#if 0
static void set_tile_bank18( int data )
{
	sys16_tile_bank0 = data&0xf;
	sys16_tile_bank1 = (data>>4)&0xf;
}
#endif

static void set_fg_page( int data )
{
	sys16_fg_page[0] = data>>12;
	sys16_fg_page[1] = (data>>8)&0xf;
	sys16_fg_page[2] = (data>>4)&0xf;
	sys16_fg_page[3] = data&0xf;
}

static void set_bg_page( int data )
{
	sys16_bg_page[0] = data>>12;
	sys16_bg_page[1] = (data>>8)&0xf;
	sys16_bg_page[2] = (data>>4)&0xf;
	sys16_bg_page[3] = data&0xf;
}





/***************************************************************************/
/***************************************************************************/
/***************************************************************************/




/***************************************************************************/


/***************************************************************************/


/***************************************************************************/

/***************************************************************************/


/***************************************************************************/


/***************************************************************************/



/***************************************************************************/

static INPUT_PORTS_START( aliensyn )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "127 (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, "Timer" )
	PORT_DIPSETTING(    0x00, "120" )
	PORT_DIPSETTING(    0x10, "130" )
	PORT_DIPSETTING(    0x20, "140" )
	PORT_DIPSETTING(    0x30, "150" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

/****************************************************************************/


/***************************************************************************/


static ADDRESS_MAP_START( bayroute_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x500000, 0x503fff) AM_READ(SYS16_MRA16_WORKINGRAM)
	AM_RANGE(0x600000, 0x600fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x700000, 0x70ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x710000, 0x710fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x800000, 0x800fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0x901002, 0x901003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0x901006, 0x901007) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0x901000, 0x901001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0x902002, 0x902003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0x902000, 0x902001) AM_READ(input_port_4_word_r) // dip2
ADDRESS_MAP_END

static ADDRESS_MAP_START( bayroute_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x100003) AM_WRITE(MWA16_NOP) // tilebank control?
	AM_RANGE(0x500000, 0x503fff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
	AM_RANGE(0x600000, 0x600fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x700000, 0x70ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x710000, 0x710fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x800000, 0x800fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0x900000, 0x900001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xff0006, 0xff0007) AM_WRITE(sound_command_w)
	AM_RANGE(0xff0020, 0xff003f) AM_WRITE(MWA16_NOP) // config regs
ADDRESS_MAP_END

/***************************************************************************/

static void bayroute_update_proc( void )
{
	set_fg_page( sys16_textram[0x740] );
	set_bg_page( sys16_textram[0x741] );
	sys16_fg_scrolly = sys16_textram[0x748];
	sys16_bg_scrolly = sys16_textram[0x749];
	sys16_fg_scrollx = sys16_textram[0x74c];
	sys16_bg_scrollx = sys16_textram[0x74d];
}

static MACHINE_RESET( bayroute )
{
	static const int bank[16] = {
		0,0,0,0,
		0,0,0,3,
		0,0,0,2,
		0,1,0,0
	};
	sys16_obj_bank = bank;
	sys16_update_proc = bayroute_update_proc;
	sys16_spritesystem = sys16_sprite_shinobi;
}

static DRIVER_INIT( bayrtbl1 )
{
	MACHINE_RESET_CALL(sys16_onetime);
}
/***************************************************************************/

static INPUT_PORTS_START( bayroute )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "Unlimited (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xc0, "A" )
	PORT_DIPSETTING(    0x80, "B" )
	PORT_DIPSETTING(    0x40, "C" )
	PORT_DIPSETTING(    0x00, "D" )

INPUT_PORTS_END

/***************************************************************************/

static MACHINE_DRIVER_START( bayroute )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16_7759)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(bayroute_readmem,bayroute_writemem)

	MDRV_MACHINE_RESET(bayroute)
MACHINE_DRIVER_END

/***************************************************************************

   Body Slam

***************************************************************************/


/***************************************************************************/


/***************************************************************************/


/***************************************************************************/


/***************************************************************************/

static ADDRESS_MAP_START( dduxbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41004, 0xc41005) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dduxbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x3f0000, 0x3fffff) AM_WRITE(sys16_tilebank_w)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xc40006, 0xc40007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc46000, 0xc4603f) AM_WRITE(SYS16_MWA16_EXTRAM2) AM_BASE(&sys16_extraram2)
	AM_RANGE(0xfe0020, 0xfe003f) AM_WRITE(MWA16_NOP) // config regs
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void dduxbl_update_proc( void )
{
	sys16_fg_scrollx = (sys16_extraram2[0x0018/2] ^ 0xffff) & 0x01ff;
	sys16_bg_scrollx = (sys16_extraram2[0x0008/2] ^ 0xffff) & 0x01ff;
	sys16_fg_scrolly = sys16_extraram2[0x0010/2] & 0x00ff;
	sys16_bg_scrolly = sys16_extraram2[0x0000/2];

	{
		UINT8 lu = sys16_extraram2[0x0020/2] & 0xff;
		UINT8 ru = sys16_extraram2[0x0022/2] & 0xff;
		UINT8 ld = sys16_extraram2[0x0024/2] & 0xff;
		UINT8 rd = sys16_extraram2[0x0026/2] & 0xff;

		if (lu==4 && ld==4 && ru==5 && rd==5)
		{ // fix a bug in chicago round (un-tested in MAME)
			int vs=(*(UINT16 *)(&sys16_workingram[0x36ec]));
			sys16_bg_scrolly = vs & 0xff;
			sys16_fg_scrolly = vs & 0xff;
			if (vs >= 0x100)
			{
				lu=0x26; ru=0x37;
				ld=0x04; rd=0x15;
			} else {
				ld=0x26; rd=0x37;
				lu=0x04; ru=0x15;
			}
		}
		sys16_fg_page[0] = ld&0xf;
		sys16_fg_page[1] = rd&0xf;
		sys16_fg_page[2] = lu&0xf;
		sys16_fg_page[3] = ru&0xf;

		sys16_bg_page[0] = ld>>4;
		sys16_bg_page[1] = rd>>4;
		sys16_bg_page[2] = lu>>4;
		sys16_bg_page[3] = ru>>4;
	}
}

static MACHINE_RESET( dduxbl )
{
	sys16_patch_code( 0x1eb2e, 0x01 );
	sys16_patch_code( 0x1eb2f, 0x01 );
	sys16_patch_code( 0x1eb3c, 0x00 );
	sys16_patch_code( 0x1eb3d, 0x00 );
	sys16_patch_code( 0x23132, 0x01 );
	sys16_patch_code( 0x23133, 0x01 );
	sys16_patch_code( 0x23140, 0x00 );
	sys16_patch_code( 0x23141, 0x00 );
	sys16_patch_code( 0x24a9a, 0x01 );
	sys16_patch_code( 0x24a9b, 0x01 );
	sys16_patch_code( 0x24aa8, 0x00 );
	sys16_patch_code( 0x24aa9, 0x00 );
}

static DRIVER_INIT( dduxbl )
{
	static const int bank[16] = { //*
		0,0,0,0,
		0,0,0,4,
		0,0,0,3,
		0,2,0,0
	};

	MACHINE_RESET_CALL(sys16_onetime);
	sys16_video_config(dduxbl_update_proc, -0x48, bank);
}
/***************************************************************************/

static INPUT_PORTS_START( ddux )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "150000" )
	PORT_DIPSETTING(    0x60, "200000" )
	PORT_DIPSETTING(    0x20, "300000" )
	PORT_DIPSETTING(    0x00, "400000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

/***************************************************************************/

static MACHINE_DRIVER_START( dduxbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(dduxbl_readmem,dduxbl_writemem)

	MDRV_MACHINE_RESET(dduxbl)
MACHINE_DRIVER_END

/***************************************************************************/



static ADDRESS_MAP_START( eswat_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x418fff) AM_READ(SYS16_MRA16_TEXTRAM) //*
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41006, 0xc41007) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static int eswat_tilebank0;

static WRITE16_HANDLER( eswat_tilebank0_w )
{
	if( ACCESSING_LSB )
	{
		eswat_tilebank0 = data&0xff;
	}
}



static ADDRESS_MAP_START( eswatbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x3e2000, 0x3e2001) AM_WRITE(eswat_tilebank0_w)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x418fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc42006, 0xc42007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xc80000, 0xc80001) AM_WRITE(MWA16_NOP)
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END


/***************************************************************************/

static void eswatbl_update_proc( void )
{
	sys16_fg_scrollx = sys16_textram[0x8008/2] ^ 0xffff;
	sys16_bg_scrollx = sys16_textram[0x8018/2] ^ 0xffff;
	sys16_fg_scrolly = sys16_textram[0x8000/2];
	sys16_bg_scrolly = sys16_textram[0x8010/2];

	set_fg_page( sys16_textram[0x8020/2] );
	set_bg_page( sys16_textram[0x8028/2] );

	sys16_tile_bank1 = (sys16_textram[0x8030/2])&0xf;
	sys16_tile_bank0 = eswat_tilebank0;
}

static MACHINE_RESET( eswatbl )
{
	static const int bank[16] = {
		0,1,	4,5,
		8,9,	12,13,
		2,3,	6,7,
		10,11,	14,15
	};
	sys16_obj_bank = bank;
	sys16_sprxoffset = -0x23c;

	sys16_patch_code( 0x3897, 0x11 );

	sys16_update_proc = eswatbl_update_proc;
}


static DRIVER_INIT( eswatbl )
{
	MACHINE_RESET_CALL(sys16_onetime);
	sys16_rowscroll_scroll=0x8000;
	sys18_splittab_fg_x=&sys16_textram[0x0f80];
}

/***************************************************************************/

static INPUT_PORTS_START( eswat )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Display Flip" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Time" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
INPUT_PORTS_END

/***************************************************************************/

static MACHINE_DRIVER_START( eswatbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16_7759)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(eswat_readmem,eswatbl_writemem)

	MDRV_MACHINE_RESET(eswatbl)
MACHINE_DRIVER_END

/***************************************************************************/


/***************************************************************************/


/***************************************************************************/


/***************************************************************************/


static ADDRESS_MAP_START( fpointbl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
//  AM_RANGE(0x02002e, 0x020049) AM_READ(fp_io_service_dummy_r)
	AM_RANGE(0x600006, 0x600007) AM_WRITE(sound_command_w)
	AM_RANGE(0x601000, 0x601001) AM_READ(input_port_0_word_r) // service
	AM_RANGE(0x601002, 0x601003) AM_READ(input_port_1_word_r) // player1
	AM_RANGE(0x601004, 0x601005) AM_READ(input_port_2_word_r) // player2
	AM_RANGE(0x600000, 0x600001) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0x600002, 0x600003) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0x400000, 0x40ffff) AM_READWRITE(SYS16_MRA16_TILERAM, SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_READWRITE(SYS16_MRA16_TEXTRAM, SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_READWRITE(SYS16_MRA16_SPRITERAM, SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
//  AM_RANGE(0x44302a, 0x44304d) AM_READ(fp_io_service_dummy_r)
	AM_RANGE(0x840000, 0x840fff) AM_READWRITE(SYS16_MRA16_PALETTERAM, SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
//  AM_RANGE(0xfe0006, 0xfe0007) AM_WRITE(sound_command_w) // original
//  AM_RANGE(0xfe003e, 0xfe003f) AM_READ(fp_io_service_dummy_r)
	AM_RANGE(0xffc000, 0xffffff) AM_READWRITE(SYS16_MRA16_WORKINGRAM, SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void fpoint_update_proc( void )
{
	set_fg_page( sys16_textram[0x740] );
	set_bg_page( sys16_textram[0x741] );
	sys16_fg_scrolly = sys16_textram[0x748];
	sys16_bg_scrolly = sys16_textram[0x749];
	sys16_fg_scrollx = sys16_textram[0x74c];
	sys16_bg_scrollx = sys16_textram[0x74d];
}

static MACHINE_RESET( fpointbl )
{
	sys16_patch_code( 0x454, 0x33 );
	sys16_patch_code( 0x455, 0xf8 );
	sys16_patch_code( 0x456, 0xe0 );
	sys16_patch_code( 0x457, 0xe2 );
	sys16_patch_code( 0x8ce8, 0x16 );
	sys16_patch_code( 0x8ce9, 0x66 );
	sys16_patch_code( 0x17687, 0x00 );
	sys16_patch_code( 0x7bed, 0x04 );

	sys16_patch_code( 0x7ea8, 0x61 );
	sys16_patch_code( 0x7ea9, 0x00 );
	sys16_patch_code( 0x7eaa, 0x84 );
	sys16_patch_code( 0x7eab, 0x16 );
	sys16_patch_code( 0x2c0, 0xe7 );
	sys16_patch_code( 0x2c1, 0x48 );
	sys16_patch_code( 0x2c2, 0xe7 );
	sys16_patch_code( 0x2c3, 0x49 );
	sys16_patch_code( 0x2c4, 0x04 );
	sys16_patch_code( 0x2c5, 0x40 );
	sys16_patch_code( 0x2c6, 0x00 );
	sys16_patch_code( 0x2c7, 0x10 );
	sys16_patch_code( 0x2c8, 0x4e );
	sys16_patch_code( 0x2c9, 0x75 );

	sys16_update_proc = fpoint_update_proc;
}

static DRIVER_INIT( fpointbl )
{
	MACHINE_RESET_CALL(sys16_onetime);
	sys16_video_config(fpoint_update_proc, -0xb8, NULL);
}
/***************************************************************************/

static INPUT_PORTS_START( fpoint )
	SYS16_SERVICE

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Clear round allowed" ) /* Use button 3 */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	SYS16_COINAGE
INPUT_PORTS_END


static INPUT_PORTS_START( fpointbj )
	SYS16_SERVICE

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) /* not used according to manual */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) /* not used according to manual */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) /* not used according to manual */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) /* not used according to manual */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2 Cell Move Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	SYS16_COINAGE
INPUT_PORTS_END

/***************************************************************************/

/*
    Flash Point (Datsu bootlegs = fpointbl, fpointbj)
    Has sound latch at $E000 instead of I/O ports $C0-FF
*/
static ADDRESS_MAP_START( fpointbl_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static MACHINE_DRIVER_START( fpointbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(fpointbl_map,0)

	MDRV_CPU_MODIFY("sound")
	MDRV_CPU_PROGRAM_MAP(fpointbl_sound_readmem,sound_writemem)

	MDRV_MACHINE_RESET(fpointbl)
MACHINE_DRIVER_END

/***************************************************************************/

static READ16_HANDLER( ga_io_players_r ) {
	return (readinputport(0) << 8) | readinputport(1);
}
static READ16_HANDLER( ga_io_service_r )
{
	return (input_port_2_word_r(0,0) << 8) | (sys16_workingram[0x2c96/2] & 0x00ff);
}

static ADDRESS_MAP_START( goldnaxe_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x110000, 0x110fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x140000, 0x140fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0x1f0000, 0x1f0003) AM_READ(SYS16_MRA16_EXTRAM)
	AM_RANGE(0x200000, 0x200fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41006, 0xc41007) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xffecd0, 0xffecd1) AM_READ(ga_io_players_r)
	AM_RANGE(0xffec96, 0xffec97) AM_READ(ga_io_service_r)
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static WRITE16_HANDLER( ga_sound_command_w )
{
	COMBINE_DATA( &sys16_workingram[(0xecfc-0xc000)/2] );
	if( ACCESSING_MSB )
{
		soundlatch_w( 0,data>>8 );
		cpunum_set_input_line(Machine, 1, 0, HOLD_LINE );
	}
}

static WRITE16_HANDLER( goldnaxe_prot_w )
{
	sys16_workingram[(0xecd8 - 0xc000)/2] = 0x048c;
	sys16_workingram[(0xecda - 0xc000)/2] = 0x159d;
	sys16_workingram[(0xecdc - 0xc000)/2] = 0x26ae;
	sys16_workingram[(0xecde - 0xc000)/2] = 0x37bf;
	COMBINE_DATA( &sys16_workingram[(0xec1c-0xc000)/2] );
}

static ADDRESS_MAP_START( goldnaxe_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x110000, 0x110fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x140000, 0x140fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0x1f0000, 0x1f0003) AM_WRITE(SYS16_MWA16_EXTRAM) AM_BASE(&sys16_extraram)
	AM_RANGE(0x200000, 0x200fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xc43000, 0xc43001) AM_WRITE(MWA16_NOP) // ?
//  AM_RANGE(0xfe0006, 0xfe0007) AM_WRITE(MWA16_NOP) I think this is the real sound out
	AM_RANGE(0xffec1c, 0xffec1d) AM_WRITE(goldnaxe_prot_w)// how does this really work?
	AM_RANGE(0xffecfc, 0xffecfd) AM_WRITE(ga_sound_command_w)// probably just a buffer
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram) /* fails SCRATCH RAM test because of hacks */
//  AM_RANGE(0xfffc00, 0xffffff) AM_WRITE(MWA15_NOP) /* 0x400 bytes; battery backed up */
ADDRESS_MAP_END

/***************************************************************************/

static void goldnaxe_update_proc( void )
{
	set_fg_page( sys16_textram[0x740] );
	set_bg_page( sys16_textram[0x741] );
	sys16_fg_scrolly = sys16_textram[0x748];
	sys16_bg_scrolly = sys16_textram[0x749];
	sys16_fg_scrollx = sys16_textram[0x74c];
	sys16_bg_scrollx = sys16_textram[0x74d];

	set_tile_bank( sys16_workingram[0x2c94/2] );
}

static MACHINE_RESET( goldnaxe )
{
	static const int bank[16] = {
		0,1,4,5,
		8,9,0,0,
		2,3,6,7,
		10,11,0,0
	};
	sys16_obj_bank = bank;

// protection patch; no longer needed
//  sys16_patch_code( 0x3CB2, 0x60 );
//  sys16_patch_code( 0x3CB3, 0x1e );

	sys16_sprxoffset = -0xb8;
	sys16_update_proc = goldnaxe_update_proc;
}



static DRIVER_INIT( goldnabl )
{
	MACHINE_RESET_CALL(sys16_onetime);
}

/***************************************************************************/

static INPUT_PORTS_START( goldnaxe )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Credits needed" )
	PORT_DIPSETTING(    0x01, "1 to start, 1 to continue" )
	PORT_DIPSETTING(    0x00, "2 to start, 1 to continue" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Energy Meter" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/

static MACHINE_DRIVER_START( goldnaxe )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16_7759)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(goldnaxe_readmem,goldnaxe_writemem)

	MDRV_MACHINE_RESET(goldnaxe)
MACHINE_DRIVER_END

/***************************************************************************/



/***************************************************************************/


/***************************************************************************/


/***************************************************************************/


/***************************************************************************/


/***************************************************************************/

static ADDRESS_MAP_START( passsht_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41004, 0xc41005) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( passsht_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc42006, 0xc42007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

static int passht4b_io1_val;
static int passht4b_io2_val;
static int passht4b_io3_val;

static READ16_HANDLER( passht4b_service_r )
{
	UINT16 val=input_port_2_word_r(offset,0);
	if(!(readinputport(0) & 0x40)) val&=0xef;
	if(!(readinputport(1) & 0x40)) val&=0xdf;
	if(!(readinputport(5) & 0x40)) val&=0xbf;
	if(!(readinputport(6) & 0x40)) val&=0x7f;

	passht4b_io3_val=(readinputport(0)<<4) | (readinputport(5)&0xf);
	passht4b_io2_val=(readinputport(1)<<4) | (readinputport(6)&0xf);

	passht4b_io1_val=0xff;

	// player 1 buttons
	if(!(readinputport(0) & 0x10)) passht4b_io1_val &=0xfe;
	if(!(readinputport(0) & 0x20)) passht4b_io1_val &=0xfd;
	if(!(readinputport(0) & 0x80)) passht4b_io1_val &=0xfc;

	// player 2 buttons
	if(!(readinputport(1) & 0x10)) passht4b_io1_val &=0xfb;
	if(!(readinputport(1) & 0x20)) passht4b_io1_val &=0xf7;
	if(!(readinputport(1) & 0x80)) passht4b_io1_val &=0xf3;

	// player 3 buttons
	if(!(readinputport(5) & 0x10)) passht4b_io1_val &=0xef;
	if(!(readinputport(5) & 0x20)) passht4b_io1_val &=0xdf;
	if(!(readinputport(5) & 0x80)) passht4b_io1_val &=0xcf;

	// player 4 buttons
	if(!(readinputport(6) & 0x10)) passht4b_io1_val &=0xbf;
	if(!(readinputport(6) & 0x20)) passht4b_io1_val &=0x7f;
	if(!(readinputport(6) & 0x80)) passht4b_io1_val &=0x3f;

	return val;
}

static READ16_HANDLER( passht4b_io1_r ) {	return passht4b_io1_val;}
static READ16_HANDLER( passht4b_io2_r ) {	return passht4b_io2_val;}
static READ16_HANDLER( passht4b_io3_r ) {	return passht4b_io3_val;}

static ADDRESS_MAP_START( passht4b_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41000, 0xc41001) AM_READ(passht4b_service_r)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(passht4b_io1_r)
	AM_RANGE(0xc41004, 0xc41005) AM_READ(passht4b_io2_r)
	AM_RANGE(0xc41006, 0xc41007) AM_READ(passht4b_io3_r)
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xc43000, 0xc43001) AM_READ(input_port_0_word_r) // player1        // test mode only
	AM_RANGE(0xc43002, 0xc43003) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc43004, 0xc43005) AM_READ(input_port_5_word_r) // player3
	AM_RANGE(0xc43006, 0xc43007) AM_READ(input_port_6_word_r) // player4
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( passht4b_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc42006, 0xc42007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc4600a, 0xc4600b) AM_WRITE(sys16_coinctrl_w)	/* coin counter doesn't work */
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void passsht_update_proc( void )
{
	sys16_fg_scrollx = sys16_workingram[0x34be/2];
	sys16_bg_scrollx = sys16_workingram[0x34c2/2];
	sys16_fg_scrolly = sys16_workingram[0x34bc/2];
	sys16_bg_scrolly = sys16_workingram[0x34c0/2];

	set_fg_page( sys16_textram[0x0ff6/2] );
	set_bg_page( sys16_textram[0x0ff4/2] );
}

static void passht4b_update_proc( void )
{
	sys16_fg_scrollx = sys16_workingram[0x34ce/2];
	sys16_bg_scrollx = sys16_workingram[0x34d2/2];
	sys16_fg_scrolly = sys16_workingram[0x34cc/2];
	sys16_bg_scrolly = sys16_workingram[0x34d0/2];

	set_fg_page( sys16_textram[0x0ff6/2] );
	set_bg_page( sys16_textram[0x0ff4/2] );
}

static MACHINE_RESET( passsht )
{
	sys16_sprxoffset = -0x48;
	sys16_spritesystem = sys16_sprite_passshot;

	// fix name entry
	sys16_patch_code( 0x13a8,0xc0);

	sys16_update_proc = passsht_update_proc;
}

static MACHINE_RESET( passht4b )
{
	sys16_sprxoffset = -0xb8;
	sys16_spritesystem = sys16_sprite_passshot;

	// fix name entry
	sys16_patch_code( 0x138a,0xc0);

	sys16_update_proc = passht4b_update_proc;
}

static DRIVER_INIT( passsht )
{
	MACHINE_RESET_CALL(sys16_onetime);
}

static DRIVER_INIT( passht4b )
{
	MACHINE_RESET_CALL(sys16_onetime);
}

/***************************************************************************/

static INPUT_PORTS_START( passsht )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Initial Point" )
	PORT_DIPSETTING(    0x06, "2000" )
	PORT_DIPSETTING(    0x0a, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPSETTING(    0x0e, "5000" )
	PORT_DIPSETTING(    0x08, "6000" )
	PORT_DIPSETTING(    0x04, "7000" )
	PORT_DIPSETTING(    0x02, "8000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x30, 0x30, "Point Table" )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( passht4b )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Initial Point" )
	PORT_DIPSETTING(    0x06, "2000" )
	PORT_DIPSETTING(    0x0a, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPSETTING(    0x0e, "5000" )
	PORT_DIPSETTING(    0x08, "6000" )
	PORT_DIPSETTING(    0x04, "7000" )
	PORT_DIPSETTING(    0x02, "8000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x30, 0x30, "Point Table" )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START_TAG("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)

	PORT_START_TAG("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

INPUT_PORTS_END

/***************************************************************************/

static MACHINE_DRIVER_START( passsht )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16_7759)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(passsht_readmem,passsht_writemem)

	MDRV_MACHINE_RESET(passsht)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( passht4b )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16_7759)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(passht4b_readmem,passht4b_writemem)

	MDRV_MACHINE_RESET(passht4b)
MACHINE_DRIVER_END

/***************************************************************************/


/***************************************************************************/


/***************************************************************************/


/***************************************************************************/

static INPUT_PORTS_START( shinobi )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "240 (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Enemy's Bullet Speed" )
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )

INPUT_PORTS_END

/***************************************************************************/


/***************************************************************************/

static ADDRESS_MAP_START( shinobl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41006, 0xc41007) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shinobl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sound_command_nmi_w)
	AM_RANGE(0xc40002, 0xc40003) AM_WRITE(sys16_3d_coinctrl_w)
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void shinobl_update_proc( void )
{
	set_bg_page( sys16_textram[0x74e] );
	set_fg_page( sys16_textram[0x74f] );
	sys16_fg_scrolly = sys16_textram[0x792] & 0x00ff;
	sys16_bg_scrolly = sys16_textram[0x793] & 0x01ff;
	sys16_fg_scrollx = sys16_textram[0x7fc] & 0x01ff;
	sys16_bg_scrollx = sys16_textram[0x7fd] & 0x01ff;
}

static MACHINE_RESET( shinobl )
{
	static const int bank[16] = {
		0,2,4,6,
		1,3,5,7
	};
	sys16_obj_bank = bank;
	sys16_spritesystem = sys16_sprite_quartet2;
	sys16_sprxoffset = -0xbc;
	sys16_fgxoffset = sys16_bgxoffset = 7;
	sys16_tilebank_switch=0x2000;

	sys16_update_proc = shinobl_update_proc;
}

static DRIVER_INIT( shinobl )
{
	shinobl_kludge = 1;
	MACHINE_RESET_CALL(sys16_onetime);
}


/***************************************************************************/



static MACHINE_DRIVER_START( shinob2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(shinobl_readmem,shinobl_writemem)

	MDRV_MACHINE_RESET(shinobl)
MACHINE_DRIVER_END


/***************************************************************************/



/* bootleg has extra ram for regs? */

static ADDRESS_MAP_START( tetrisbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x418000, 0x41803f) AM_READ(SYS16_MRA16_EXTRAM2)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41006, 0xc41007) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xc80000, 0xc80001) AM_READ(MRA16_NOP)
	AM_RANGE(0xffc000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetrisbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x418000, 0x41803f) AM_WRITE(SYS16_MWA16_EXTRAM2) AM_BASE(&sys16_extraram2)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xc42006, 0xc42007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc43034, 0xc43035) AM_WRITE(MWA16_NOP)
	AM_RANGE(0xc80000, 0xc80001) AM_WRITE(MWA16_NOP)
	AM_RANGE(0xffc000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/



static void tetris_bootleg_update_proc( void )
{
	sys16_fg_scrolly = sys16_textram[0x748];
	sys16_bg_scrolly = sys16_textram[0x749];
	sys16_fg_scrollx = sys16_textram[0x74c];
	sys16_bg_scrollx = sys16_textram[0x74d];

	set_fg_page( sys16_extraram2[0x38/2] );
	set_bg_page( sys16_extraram2[0x28/2] );
}


static MACHINE_RESET( tetrisbl )
{
//  sys16_patch_code( 0xba6, 0x4e );
//  sys16_patch_code( 0xba7, 0x71 );

	sys16_sprxoffset = -0x40;
	sys16_update_proc = tetris_bootleg_update_proc;
}


static DRIVER_INIT( tetrisbl )
{
	MACHINE_RESET_CALL(sys16_onetime);
}


/***************************************************************************/

static INPUT_PORTS_START( tetris )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE /* unconfirmed */

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )	// from the code it looks like some kind of difficulty
	PORT_DIPSETTING(    0x0c, "A" )					// level, but all 4 levels points to the same place
	PORT_DIPSETTING(    0x08, "B" )					// so it doesn't actually change anything!!
	PORT_DIPSETTING(    0x04, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/


static MACHINE_DRIVER_START( tetrisbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(tetrisbl_readmem,tetrisbl_writemem)

	MDRV_MACHINE_RESET(tetrisbl)
MACHINE_DRIVER_END

/***************************************************************************/

static READ16_HANDLER( tt_io_player1_r )
{ return input_port_0_r( offset ) << 8; }
static READ16_HANDLER( tt_io_player2_r )
{ return input_port_1_r( offset ) << 8; }
static READ16_HANDLER( tt_io_service_r )
{ return input_port_2_r( offset ) << 8; }



/***************************************************************************/

/*
    This game has a MCU which does the following:
    - Get Z80 sound command out of work RAM and write to Z80 sound command register
    - Read input ports and store to work RAM

    The routine which stores the sound code in RAM looks like this:

    ; D0 = sound command
    movem.l    d0-d1/a0, -(a7)
    lea        $2001d6, a0         ; base of 16-byte circular buffer
    move.w     $2001d4, d1         ; get buffer index
    move.b     d0, (a0, d1.w)      ; write sound command to buffer
    addq.w     #1, d1              ; next buffer index
    andi.w     #$000f, d1          ; wrap buffer index
    move.w     d1, $2001d4         ; save buffer index
    addq.w     #1, $2001d2         ; bump 'sound code written' flag
    movem.l    (a7)+, d0-d1/a0
    rts

    Most likely the MCU reads $2001D2 and copies the sound byte from $2001D6+$2001D4 to the sound command register.
    In tturfbl, a JSR is inserted over the first LEA instruction to a subroutine which copies D0 to the sound command
    register at $600007, and restores a0 to $2001D6 before returning.

    If the circular buffer is to prioritize sound requests, then this effect is lost in tturfbl. If it's just to
    be tricky, tturfbl handles it correctly.
*/


/***************************************************************************/

static INPUT_PORTS_START( tturf )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Continues ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "Unlimited" )
	PORT_DIPSETTING(    0x03, "Unlimited" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x20, "Starting Energy" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x30, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bonus Energy" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END


/***************************************************************************/

static ADDRESS_MAP_START( tturfbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x2001e6, 0x2001e7) AM_READ(tt_io_service_r)
	AM_RANGE(0x2001e8, 0x2001e9) AM_READ(tt_io_player1_r)
	AM_RANGE(0x2001ea, 0x2001eb) AM_READ(tt_io_player2_r)
	AM_RANGE(0x200000, 0x203fff) AM_READ(SYS16_MRA16_EXTRAM)
	AM_RANGE(0x300000, 0x300fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x500000, 0x500fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0x600002, 0x600003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0x600000, 0x600001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0x601002, 0x601003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0x601004, 0x601005) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0x601000, 0x601001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0x602002, 0x602003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0x602000, 0x602001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xc46000, 0xc4601f) AM_READ(SYS16_MRA16_EXTRAM3)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tturfbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x200000, 0x203fff) AM_WRITE(SYS16_MWA16_EXTRAM) AM_BASE(&sys16_extraram)
	AM_RANGE(0x300000, 0x300fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x500000, 0x500fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0x600000, 0x600001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0x600006, 0x600007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc44000, 0xc44001) AM_WRITE(MWA16_NOP)
	AM_RANGE(0xc46000, 0xc4601f) AM_WRITE(SYS16_MWA16_EXTRAM3) AM_BASE(&sys16_extraram3)
	AM_RANGE(0xff0020, 0xff003f) AM_WRITE(MWA16_NOP) // config regs
ADDRESS_MAP_END

/***************************************************************************/

static void tturfbl_update_proc( void )
{
	sys16_fg_scrollx = sys16_textram[0x74c] & 0x01ff;
	sys16_bg_scrollx = sys16_textram[0x74d/2] & 0x01ff;
	sys16_fg_scrolly = sys16_textram[0x748];
	sys16_bg_scrolly = sys16_textram[0x749];


	{
		int data1,data2;

		data1 = sys16_textram[0x740];
		data2 = sys16_textram[0x741];

		sys16_fg_page[3] = data1>>12;
		sys16_bg_page[3] = (data1>>8)&0xf;
		sys16_fg_page[1] = (data1>>4)&0xf;
		sys16_bg_page[1] = data1&0xf;

		sys16_fg_page[2] = data2>>12;
		sys16_bg_page[2] = (data2>>8)&0xf;
		sys16_fg_page[0] = (data2>>4)&0xf;
		sys16_bg_page[0] = data2&0xf;
	}
}

static MACHINE_RESET( tturfbl )
{
	static const int bank[16] = {
		0,0,0,0,
		0,0,0,3,
		0,0,0,2,
		0,1,0,0
	};
	sys16_obj_bank = bank;
	sys16_sprxoffset = -0x48;

	sys16_update_proc = tturfbl_update_proc;
}

static DRIVER_INIT( tturfbl )
{
	UINT8 *mem;

	MACHINE_RESET_CALL(sys16_onetime);

	mem = memory_region(REGION_CPU2);
	memcpy(mem, mem+0x10000, 0x8000);

}
/***************************************************************************/
// sound ??
static MACHINE_DRIVER_START( tturfbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16_7759)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(tturfbl_readmem,tturfbl_writemem)

	MDRV_CPU_MODIFY("sound")
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(tturfbl_sound_readmem,tturfbl_sound_writemem)
	MDRV_CPU_IO_MAP(tturfbl_sound_readport,tturfbl_sound_writeport)

	MDRV_SOUND_REMOVE("7759")
	MDRV_SOUND_ADD_TAG("5205", MSM5205, 220000)
	MDRV_SOUND_CONFIG(tturfbl_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.80)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.80)

	MDRV_MACHINE_RESET(tturfbl)
MACHINE_DRIVER_END


/***************************************************************************/


/***************************************************************************/





/***************************************************************************/

static INPUT_PORTS_START( wb3b )
	SYS16_JOY1
	SYS16_JOY2
	SYS16_SERVICE
	SYS16_COINAGE

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )		//??
	PORT_DIPSETTING(    0x10, "5000/10000/18000/30000" )
	PORT_DIPSETTING(    0x00, "5000/15000/30000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Allow Round Select" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )			// no collision though
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/


/***************************************************************************/

static ADDRESS_MAP_START( wb3bbl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x40ffff) AM_READ(SYS16_MRA16_TILERAM)
	AM_RANGE(0x410000, 0x410fff) AM_READ(SYS16_MRA16_TEXTRAM)
	AM_RANGE(0x440000, 0x440fff) AM_READ(SYS16_MRA16_SPRITERAM)
	AM_RANGE(0x840000, 0x840fff) AM_READ(SYS16_MRA16_PALETTERAM)
	AM_RANGE(0xc41002, 0xc41003) AM_READ(input_port_0_word_r) // player1
	AM_RANGE(0xc41004, 0xc41005) AM_READ(input_port_1_word_r) // player2
	AM_RANGE(0xc41000, 0xc41001) AM_READ(input_port_2_word_r) // service
	AM_RANGE(0xc42002, 0xc42003) AM_READ(input_port_3_word_r) // dip1
	AM_RANGE(0xc42000, 0xc42001) AM_READ(input_port_4_word_r) // dip2
	AM_RANGE(0xc46000, 0xc4601f) AM_READ(SYS16_MRA16_EXTRAM3)
	AM_RANGE(0xff0000, 0xffffff) AM_READ(SYS16_MRA16_WORKINGRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wb3bbl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x3f0000, 0x3fffff) AM_WRITE(sys16_tilebank_w)
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(SYS16_MWA16_TILERAM) AM_BASE(&sys16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_WRITE(SYS16_MWA16_TEXTRAM) AM_BASE(&sys16_textram)
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SYS16_MWA16_SPRITERAM) AM_BASE(&sys16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_WRITE(SYS16_MWA16_PALETTERAM) AM_BASE(&paletteram16)
	AM_RANGE(0xc40000, 0xc40001) AM_WRITE(sys16_coinctrl_w)
	AM_RANGE(0xc42006, 0xc42007) AM_WRITE(sound_command_w)
	AM_RANGE(0xc44000, 0xc44001) AM_WRITE(MWA16_NOP)
	AM_RANGE(0xc46000, 0xc4601f) AM_WRITE(SYS16_MWA16_EXTRAM3) AM_BASE(&sys16_extraram3)
	AM_RANGE(0xff0000, 0xffffff) AM_WRITE(SYS16_MWA16_WORKINGRAM) AM_BASE(&sys16_workingram)
ADDRESS_MAP_END

/***************************************************************************/

static void wb3bbl_update_proc( void )
{

	sys16_fg_scrollx = sys16_workingram[0xc030/2];
	sys16_bg_scrollx = sys16_workingram[0xc038/2];
	sys16_fg_scrolly = sys16_workingram[0xc032/2];
	sys16_bg_scrolly = sys16_workingram[0xc03c/2];

	set_fg_page( sys16_textram[0x0ff6/2] );
	set_bg_page( sys16_textram[0x0ff4/2] );
}

static MACHINE_RESET( wb3bbl )
{
	static const int bank[16] = {
		2,0,
		1,0,
		3,0,
		0,3,
		0,0,
		0,2,
		0,1,
		0,0
	};
	sys16_obj_bank = bank;
#if 1
	sys16_patch_code( 0x17058, 0x4e );
	sys16_patch_code( 0x17059, 0xb9 );
	sys16_patch_code( 0x1705a, 0x00 );
	sys16_patch_code( 0x1705b, 0x00 );
	sys16_patch_code( 0x1705c, 0x09 );
	sys16_patch_code( 0x1705d, 0xdc );
	sys16_patch_code( 0x1705e, 0x4e );
	sys16_patch_code( 0x1705f, 0xf9 );
	sys16_patch_code( 0x17060, 0x00 );
	sys16_patch_code( 0x17061, 0x01 );
	sys16_patch_code( 0x17062, 0x70 );
	sys16_patch_code( 0x17063, 0xe0 );
	sys16_patch_code( 0x1a3a, 0x31 );
	sys16_patch_code( 0x1a3b, 0x7c );
	sys16_patch_code( 0x1a3c, 0x80 );
	sys16_patch_code( 0x1a3d, 0x00 );
	sys16_patch_code( 0x23df8, 0x14 );
	sys16_patch_code( 0x23df9, 0x41 );
	sys16_patch_code( 0x23dfa, 0x10 );
	sys16_patch_code( 0x23dfd, 0x14 );
	sys16_patch_code( 0x23dff, 0x1c );
#endif
	sys16_update_proc = wb3bbl_update_proc;
}

static DRIVER_INIT( wb3bbl )
{
	MACHINE_RESET_CALL(sys16_onetime);
}

/***************************************************************************/

static MACHINE_DRIVER_START( wb3bbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(system16)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(wb3bbl_readmem,wb3bbl_writemem)

	MDRV_MACHINE_RESET(wb3bbl)
MACHINE_DRIVER_END



/*****************************************************************************/


ROM_START( bayrtbl1 )
	ROM_REGION( 0xc0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "b4.bin", 0x000000, 0x10000, CRC(eb6646ae) SHA1(073bc0a3868e70785f44e497a949cd9e3b591a33) )
	ROM_LOAD16_BYTE( "b2.bin", 0x000001, 0x10000, CRC(ecd9cd0e) SHA1(177c38ca02c4e87d6adcae77ce4e9237938d23a9) )
	/* empty 0x20000-0x80000*/
	ROM_LOAD16_BYTE( "br.5a",  0x080000, 0x10000, CRC(9d6fd183) SHA1(5ae78d33c0e929886d84a25c0fbd62ab45dcbff4) )
	ROM_LOAD16_BYTE( "br.2a",  0x080001, 0x10000, CRC(5ca1e3d2) SHA1(51ce67ed0a0054f9c9c4ac56c5775716c44d74b1) )
	ROM_LOAD16_BYTE( "b8.bin", 0x0a0000, 0x10000, CRC(e7ca0331) SHA1(b255939576a84f4d266f31a7fde818e04ff35b24) )
	ROM_LOAD16_BYTE( "b6.bin", 0x0a0001, 0x10000, CRC(2bc748a6) SHA1(9ab760377fde24cecb703726ee3e59ee23d60a3a) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "bs16.bin", 0x00000, 0x10000, CRC(a8a5b310) SHA1(8883e1ed48a3e0f7b4c36d83579f93e84e28568c) )
	ROM_LOAD( "bs14.bin", 0x10000, 0x10000, CRC(6bc4d0a8) SHA1(90b9a61c7a140291d72554857ce26d54ebf03fc2) )
	ROM_LOAD( "bs12.bin", 0x20000, 0x10000, CRC(c1f967a6) SHA1(8eb6bbd9e17dc531830bc798b8485c8ea999e56e) )

	ROM_REGION( 0x80000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "br_obj0o.1b", 0x00001, 0x10000, CRC(098a5e82) SHA1(c5922f418773bc3629071e584457839d67a370e9) )
	ROM_LOAD16_BYTE( "br_obj0e.5b", 0x00000, 0x10000, CRC(85238af9) SHA1(39989a8d9b60c6d55272b5e2c213341a563dd993) )
	ROM_LOAD16_BYTE( "br_obj1o.2b", 0x20001, 0x10000, CRC(cc641da1) SHA1(28f8a6502702cb9e2cc7f3e98f6c5d201f462fa3) )
	ROM_LOAD16_BYTE( "br_obj1e.6b", 0x20000, 0x10000, CRC(d3123315) SHA1(16a87caed1cabb080d4f35935910b38797344ca5) )
	ROM_LOAD16_BYTE( "br_obj2o.3b", 0x40001, 0x10000, CRC(84efac1f) SHA1(41c43d70dc7ae7e361d6fa12c5790ea7ebf13ca8) )
	ROM_LOAD16_BYTE( "br_obj2e.7b", 0x40000, 0x10000, CRC(b73b12cb) SHA1(e8265ae90aabf1ee0522dbc6541a0f82fec97c7a) )
	ROM_LOAD16_BYTE( "br_obj3o.4b", 0x60001, 0x10000, CRC(a2e238ac) SHA1(c854774c0ffd1ccf6e46591a8fa3c80a4630e007) )
	ROM_LOAD16_BYTE( "bs7.bin",     0x60000, 0x10000, CRC(0c91abcc) SHA1(d25608f3cbacd1bd169f1a2247f007ac8bc8dda0) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12459.a10", 0x00000, 0x08000, CRC(3e1d29d0) SHA1(fe3d985983e5132e8a26a02a3f2d8d420cbf1a49) )
	ROM_LOAD( "mpr12460.a11", 0x10000, 0x20000, CRC(0bae570d) SHA1(05fa4a3405666342ab66e696a7344cca97569f19) )
	ROM_LOAD( "mpr12461.a12", 0x30000, 0x20000, CRC(b03b8b46) SHA1(b0283ac377d464f3d9374a992192ec6c515a3c2f) )
ROM_END

ROM_START( bayrtbl2 )
	ROM_REGION( 0xc0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "br_04", 0x000000, 0x10000, CRC(2e33ebfc) SHA1(f6b5a4bd28d302abd6b1e5a9ec6f2a8b57ff213e) )
	ROM_LOAD16_BYTE( "br_06", 0x000001, 0x10000, CRC(3db42313) SHA1(e1c874ebf83e1a458cefaa038fbe89a9804ca30d) )
	/* empty 0x20000-0x80000*/
	ROM_LOAD16_BYTE( "br_03", 0x080000, 0x20000, CRC(285d256b) SHA1(73eac0131d14f0d7fe2a06cb2e0e57dcf4779cf9) )
	ROM_LOAD16_BYTE( "br_05", 0x080001, 0x20000, CRC(552e6384) SHA1(2770b0c9d961671576e09ada2ebd7bb486f24547) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "br_15",    0x00000, 0x10000, CRC(050079a9) SHA1(4b356eddec2f500fb0dcc20af6b7aed2f9ef0c02) )
	ROM_LOAD( "br_16",    0x10000, 0x10000, CRC(fc371928) SHA1(b36866c95bdc440aae999a90ecf3bbaed11d4351) )
	ROM_LOAD( "bs12.bin", 0x20000, 0x10000, CRC(c1f967a6) SHA1(8eb6bbd9e17dc531830bc798b8485c8ea999e56e) )

	ROM_REGION( 0x080000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "br_11",       0x00001, 0x10000, CRC(65232905) SHA1(cb195a0ce8bff9d1d3e31678060b3aaccfefcd2d) )
	ROM_LOAD16_BYTE( "br_obj0e.5b", 0x00000, 0x10000, CRC(85238af9) SHA1(39989a8d9b60c6d55272b5e2c213341a563dd993) )
	ROM_LOAD16_BYTE( "br_obj1o.2b", 0x20001, 0x10000, CRC(cc641da1) SHA1(28f8a6502702cb9e2cc7f3e98f6c5d201f462fa3) )
	ROM_LOAD16_BYTE( "br_obj1e.6b", 0x20000, 0x10000, CRC(d3123315) SHA1(16a87caed1cabb080d4f35935910b38797344ca5) )
	ROM_LOAD16_BYTE( "br_obj2o.3b", 0x40001, 0x10000, CRC(84efac1f) SHA1(41c43d70dc7ae7e361d6fa12c5790ea7ebf13ca8) )
	ROM_LOAD16_BYTE( "br_09",       0x40000, 0x10000, CRC(05e9b840) SHA1(7cc1c9ac7b85f1e1bdb68215b5e83eae3ee5ba2a) )
	ROM_LOAD16_BYTE( "br_14",       0x60001, 0x10000, CRC(4c4a177b) SHA1(a9dfd7e56b0a21a0f7750d8ec4631901ad182609) )
	ROM_LOAD16_BYTE( "bs7.bin",     0x60000, 0x10000, CRC(0c91abcc) SHA1(d25608f3cbacd1bd169f1a2247f007ac8bc8dda0) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "br_01", 0x00000, 0x10000, CRC(b87156ec) SHA1(bdfef2ab5a4d3cac4077c92ce1ef4604b4c11cf8) )
	ROM_LOAD( "br_02", 0x10000, 0x10000, CRC(ef63991b) SHA1(4221741780f88c80b3213ddca949bee7d4c1469a) )
ROM_END

// sys16B

ROM_START( dduxbl )
	ROM_REGION( 0x0c0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "dduxb03.bin", 0x000000, 0x20000, CRC(e7526012) SHA1(a1798008bfa1ce9b87dc330f3817b1978052fcfd) )
	ROM_LOAD16_BYTE( "dduxb05.bin", 0x000001, 0x20000, CRC(459d1237) SHA1(55e9c0dc341c919d58cc789203642c397d7ac65e) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "dduxb02.bin", 0x080000, 0x20000, CRC(d8ed3132) SHA1(a9d5ad8f79fb635cc234a99fad398688a5f15926) )
	ROM_LOAD16_BYTE( "dduxb04.bin", 0x080001, 0x20000, CRC(30c6cb92) SHA1(2e17c74eeb37c9731fc2e365cc0114f7383c0106) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "dduxb14.bin", 0x00000, 0x10000, CRC(664bd135) SHA1(674b06e01c2c8f5b8057dd24d470330c3f140473) )
	ROM_LOAD( "dduxb15.bin", 0x10000, 0x10000, CRC(ce0d2b30) SHA1(e60521c46f1650c9bdc76f2ceb91a6d61aaa0a09) )
	ROM_LOAD( "dduxb16.bin", 0x20000, 0x10000, CRC(6de95434) SHA1(7bed2a0261cf6c2fbb3756633f05f0bb2173977c) )

	ROM_REGION( 0xa0000, REGION_GFX2, 0 ) //* sprites */
	ROM_LOAD16_BYTE( "dduxb10.bin", 0x00001, 0x010000, CRC(0be3aee5) SHA1(48fc779b7398abbb82cd0d0d28705ece75b3c4e3) )
	ROM_RELOAD( 0x20001, 0x010000 )
	ROM_LOAD16_BYTE( "dduxb06.bin", 0x00000, 0x010000, CRC(b0079e99) SHA1(9bb4d3fa804a3d05a6e06b45a1280d7064e96ac6) )
	ROM_RELOAD( 0x20000, 0x010000 )
	ROM_LOAD16_BYTE( "dduxb11.bin", 0x40001, 0x010000, CRC(cfb2af18) SHA1(1ad18f933a7b797f0364d1f4a6c8549351b4c9a6) )
	ROM_LOAD16_BYTE( "dduxb07.bin", 0x40000, 0x010000, CRC(0217369c) SHA1(b6ec2fa1279a27a602d79e1073c54193745ea816) )
	ROM_LOAD16_BYTE( "dduxb12.bin", 0x60001, 0x010000, CRC(28ce9b15) SHA1(1640df9c8f21893c0647ad2f4210c714a06e6f37) )
	ROM_LOAD16_BYTE( "dduxb08.bin", 0x60000, 0x010000, CRC(8844f336) SHA1(18c1baaad3bcc658d4a6d03de8c97378b5284e88) )
	ROM_LOAD16_BYTE( "dduxb13.bin", 0x80001, 0x010000, CRC(efe57759) SHA1(69b8969b20ab9480df2735bd2bcd527069196bd7) )
	ROM_LOAD16_BYTE( "dduxb09.bin", 0x80000, 0x010000, CRC(6b64f665) SHA1(df07fcf2bbec6fa78f89b95272762aebd6f3ec0e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "dduxb01.bin", 0x0000, 0x8000, CRC(0dbef0d7) SHA1(8b9afb2fcb946cec467b1e691c267194b503f841) )
ROM_END

// sys16B


ROM_START( eswatbl )
	ROM_REGION( 0x080000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "eswat_c.rom", 0x000000, 0x10000, CRC(1028cc81) SHA1(24b4cd182419a44f3d6afa1c4273353024eb278f) )
	ROM_LOAD16_BYTE( "eswat_f.rom", 0x000001, 0x10000, CRC(f7b2d388) SHA1(8131ba8f4fa01751b9993c3c6c218c9bd3adb328) )
	ROM_LOAD16_BYTE( "eswat_b.rom", 0x020000, 0x10000, CRC(87c6b1b5) SHA1(a9f29e29a9c0e3daf272dce263a5fd7866642c77) )
	ROM_LOAD16_BYTE( "eswat_e.rom", 0x020001, 0x10000, CRC(937ddf9a) SHA1(9fc73f93e9c4221a4dc778593edc02cb405b2f78) )
	ROM_LOAD16_BYTE( "eswat_a.rom", 0x040000, 0x08000, CRC(2af4fc62) SHA1(f7b1539a5ab9560bd49dfecf44699abccfb649be) )
	ROM_LOAD16_BYTE( "eswat_d.rom", 0x040001, 0x08000, CRC(b4751e19) SHA1(57c9687dc864c163d13dbb89057cd42684a199cd) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr12624.b11", 0x00000, 0x40000, CRC(375a5ec4) SHA1(42b9116bdc0e0a5b1dd667ac1856b4c2252829ba) ) // ic19
	ROM_LOAD( "mpr12625.b12", 0x40000, 0x40000, CRC(3b8c757e) SHA1(0b66e8446d059a12e47e2a6fe8f0a333245bb95c) ) // ic20
	ROM_LOAD( "mpr12626.b13", 0x80000, 0x40000, CRC(3efca25c) SHA1(0d866bf53a16b52719f73081e933f4db27d72ece) ) // ic21

	ROM_REGION( 0x180000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr12618.b1", 0x000001, 0x40000, CRC(0d1530bf) SHA1(bb8626cd98761c1c20cee117d00315c85621ba6a) ) // ic9
	ROM_LOAD16_BYTE( "mpr12621.b4", 0x000000, 0x40000, CRC(18ff0799) SHA1(5417223378aef16ee2b4f438d1f8f11a23fe7265) ) // ic12
	ROM_LOAD16_BYTE( "mpr12619.b2", 0x080001, 0x40000, CRC(32069246) SHA1(4913009bc72bf4f8b171b14fe06457f5784cab15) ) // ic10
	ROM_LOAD16_BYTE( "mpr12622.b5", 0x080000, 0x40000, CRC(a3dfe436) SHA1(640ccc552114d403f35d441574d2f3e4f1d4a8f9) ) // ic13
	ROM_LOAD16_BYTE( "mpr12620.b3", 0x100001, 0x40000, CRC(f6b096e0) SHA1(695ad1adbdc29f4d614645867e16de038cf92709) ) // ic11
	ROM_LOAD16_BYTE( "mpr12623.b6", 0x100000, 0x40000, CRC(6773fef6) SHA1(91e646ea447be02254d060daf255d26afe0cc79e) ) // ic14

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12617.a13",  0x00000, 0x08000, CRC(7efecf23) SHA1(2b87af7cfaab5942a3f7b38c987fcba01d3475ab) ) // ic8
	ROM_LOAD( "mpr12616.a11", 0x10000, 0x40000, CRC(254347c2) SHA1(bf2d83a69a5be375c7e42e9f7d6e65c1095a354c) ) // ic6
ROM_END

// sys16B


ROM_START( fpointbl )
	ROM_REGION( 0x020000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "flpoint.003", 0x000000, 0x10000, CRC(4d6df514) SHA1(168aa1629ab7152ba1984605155406b236954a2c) )
	ROM_LOAD16_BYTE( "flpoint.002", 0x000001, 0x10000, CRC(4dff2ee8) SHA1(bd157d8c168d45e7490a05d5e1e901d9bdda9599) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "flpoint.006", 0x00000, 0x10000, CRC(c539727d) SHA1(56674effe1d273128dddd2ff9e02974ec10f3fff) )
	ROM_LOAD( "flpoint.005", 0x10000, 0x10000, CRC(82c0b8b0) SHA1(e1e2e721cb8ad53df33065582dc90edeba9c3cab) )
	ROM_LOAD( "flpoint.004", 0x20000, 0x10000, CRC(522426ae) SHA1(90fd0a19b30a8a61dc4cfa66a64115596333dcc6) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "12596.bin", 0x00001, 0x010000, CRC(4a4041f3) SHA1(4c52b30223d8aa80ccdbb196098cb17e64ad6583) )
	ROM_LOAD16_BYTE( "12597.bin", 0x00000, 0x010000, CRC(6961e676) SHA1(7639d2da086b57a9a8d6100fdacf40d97d7c4772) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "flpoint.001", 0x0000, 0x8000, CRC(c5b8e0fe) SHA1(6cf8c67151d8604326fc6dbf976c0635b452a844) )	// bootleg rom doesn't work, but should be correct!
ROM_END

ROM_START( fpointbj )
	ROM_REGION( 0x020000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "boot2.003", 0x000000, 0x10000, CRC(6c00d1b0) SHA1(fd0c47b8ca010a64d3ef91980f93854ebc98fbda) )
	ROM_LOAD16_BYTE( "boot2.002", 0x000001, 0x10000, CRC(c1fcd704) SHA1(697bef464e53fb9891ed15ee2d6210107b693b20) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "flpoint.006", 0x00000, 0x10000, CRC(c539727d) SHA1(56674effe1d273128dddd2ff9e02974ec10f3fff) )
	ROM_LOAD( "flpoint.005", 0x10000, 0x10000, CRC(82c0b8b0) SHA1(e1e2e721cb8ad53df33065582dc90edeba9c3cab) )
	ROM_LOAD( "flpoint.004", 0x20000, 0x10000, CRC(522426ae) SHA1(90fd0a19b30a8a61dc4cfa66a64115596333dcc6) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "12596.bin", 0x00001, 0x010000, CRC(4a4041f3) SHA1(4c52b30223d8aa80ccdbb196098cb17e64ad6583) )
	ROM_LOAD16_BYTE( "12597.bin", 0x00000, 0x010000, CRC(6961e676) SHA1(7639d2da086b57a9a8d6100fdacf40d97d7c4772) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "flpoint.001", 0x0000, 0x8000, CRC(c5b8e0fe) SHA1(6cf8c67151d8604326fc6dbf976c0635b452a844) )	// bootleg rom doesn't work, but should be correct!

	/* stuff below isn't used but loaded because it was on the board .. */
	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s129.1",  0x0000, 0x0100, CRC(a7c22d96) SHA1(160deae8053b09c09328325246598b3518c7e20b) )
	ROM_LOAD( "82s123.2",  0x0100, 0x0020, CRC(58bcf8bd) SHA1(e4d3d179b08c0f3424a6bec0f15058fb1b56f8d8) )

	ROM_REGION( 0x0600, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "fpointbj_gal16v8_1.bin", 0x0000, 0x0117, CRC(ba7f292c) SHA1(c383700d05663c9c5f29d5d04d16b05cd6adceb8) )
	ROM_LOAD( "fpointbj_gal16v8_3.bin", 0x0200, 0x0117, CRC(ce1ab1e1) SHA1(dcfc0015d8595ee6cb6bb02c95655161a7f3b017) )
	ROM_LOAD( "fpointbj_gal20v8.bin", 0x0400, 0x019d, NO_DUMP ) /* Protected */
ROM_END

/* This is a frankenset, it has original and bootleg roms of different sizes and names mixed together, it's
   highly unlikely that the original bootleg PCB was like this */
ROM_START( goldnabl )
	ROM_REGION( 0x0c0000, REGION_CPU1, 0 ) /* 68000 code */
// protected code
	ROM_LOAD16_BYTE( "ga6.a22", 0x00000, 0x10000, CRC(f95b459f) SHA1(dadf66d63454ed62fefa521d4fed249d28c63778) )
	ROM_LOAD16_BYTE( "ga4.a20", 0x00001, 0x10000, CRC(83eabdf5) SHA1(1effef966f513fbdec2026d535658e17ef7dea51) )
	ROM_LOAD16_BYTE( "ga11.a27",0x20000, 0x10000, CRC(f4ef9349) SHA1(3ffa335e74ffbc10f80387268da659643c566897) )
	ROM_LOAD16_BYTE( "ga8.a24", 0x20001, 0x10000, CRC(37a65839) SHA1(6e8055d91b840afd8526041d3752c0a55eaebe0c) )
	/* emtpy 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "epr12521.a8", 0x80000, 0x20000, CRC(5001d713) SHA1(68cf3f48d6e440e5b800503a211adda02107d956) )
	ROM_LOAD16_BYTE( "epr12519.a6", 0x80001, 0x20000, CRC(4438ca8e) SHA1(0af53d64f06abf41f4c46540d28d5f008a4835a3) )

	ROM_REGION( 0x60000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "ga33.b16", 0x00000, 0x10000, CRC(84587263) SHA1(3a88c8578a477a487a0a214a367042b9739f39eb) )
	ROM_LOAD( "ga32.b15", 0x10000, 0x10000, CRC(63d72388) SHA1(ba0a582b1daf3a1e316237efbad17fcc0381643f) )
	ROM_LOAD( "ga31.b14", 0x20000, 0x10000, CRC(f8b6ae4f) SHA1(55132c98955107e4b247992f7917a6ce588460a7) )
	ROM_LOAD( "ga30.b13", 0x30000, 0x10000, CRC(e29baf4f) SHA1(3761cb2217599fe3f2f860f9395930b96ec52f47) )
	ROM_LOAD( "ga29.b12", 0x40000, 0x10000, CRC(22f0667e) SHA1(2d11b2ce105a3db9c914942cace85aff17deded9) )
	ROM_LOAD( "ga28.b11", 0x50000, 0x10000, CRC(afb1a7e4) SHA1(726fded9db72a881128b43f449d2baf450131f63) )

	ROM_REGION( 0x1c0000, REGION_GFX2, 0 ) /* sprites */
	/* wrong! */
	ROM_LOAD16_BYTE( "ga34.b17", 		0x000001, 0x10000, CRC(28ba70c8) SHA1(a6f33e1404928b6d1006943494646d6cfbd60a4b) )
	ROM_LOAD16_BYTE( "ga35.b18", 		0x010000, 0x10000, CRC(2ed96a26) SHA1(edcf915243e6f92d31cdfc53965438f6b6bff51d) )
	ROM_LOAD16_BYTE( "ga23.a14", 		0x020001, 0x10000, CRC(84dccc5b) SHA1(10263d98d663f1170c3203066f391075a1d64ff5) )
	ROM_LOAD16_BYTE( "ga18.a9",  		0x030000, 0x10000, CRC(de346006) SHA1(65aa489373b6d2cccbb024f13fc190a7cae86274) )
	ROM_LOAD16_BYTE( "mpr12379.b4", 	0x040001, 0x40000, CRC(1a0e8c57) SHA1(674f1ae7db632876fff346e76786801ae19d9799) )
	ROM_LOAD16_BYTE( "ga36.b19", 		0x080000, 0x10000, CRC(101d2fff) SHA1(1de1390c5f55f192491053c8aac31be3389aab2b) )
	ROM_LOAD16_BYTE( "ga37.b20", 		0x090001, 0x10000, CRC(677e64a6) SHA1(e3d0d31097017c6cb1a7f41292783f18ce13b41c) )
	ROM_LOAD16_BYTE( "ga19.a10", 		0x0a0000, 0x10000, CRC(11794d05) SHA1(eef52d7a644dbcc5f983222f163445a725286a32) )
	ROM_LOAD16_BYTE( "ga20.a11", 		0x0b0001, 0x10000, CRC(ad1c1c90) SHA1(155f17593cfab1a117bb755b1edd0c473d455f91) )
	ROM_LOAD16_BYTE( "mpr12381.b5",	0x0c0000, 0x40000, CRC(81ba6ecc) SHA1(7f59e4d86a192b97e92729371b78c3f1c784a0b5) )
	ROM_LOAD16_BYTE( "mpr12382.b3",	0x100001, 0x40000, CRC(81601c6f) SHA1(604bc5613c6c734a06860303ba36d61bb54508a0) )
	ROM_LOAD16_BYTE( "mpr12383.b6",	0x140000, 0x40000, CRC(5dbacf7a) SHA1(236866fb94672b13cbb2cb479324e61de87eeb34) )

	ROM_REGION( 0x30000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12390",     0x00000, 0x08000, CRC(399fc5f5) SHA1(6f290b36dc71ff4759598e2a9c185a8945a3c9e7) )
	ROM_LOAD( "mpr12384.a11", 0x10000, 0x20000, CRC(6218d8e7) SHA1(5a745c750efb4a61716f99befb7ed14cc84e9973) )
ROM_END

/* Golden Axe (bootleg) made in Italy?
   PCB: GENSYS-1/I

 CPUs

 1x SCN68000CAN64-KGQ7551-8931KE (main - upper board)(IC40)
 1x oscillator 20.000MHz (upper board - near main CPU)(XL2)
 1x STZ8400BB1-Z80BCPU-28911 (sound - upper board)(IC44)
 2x YAMAHA YM2203C (sound - upper board)IC27, IC28)
 1x OKI M5205 (sound - upper board)(IC16)
 2x YAMAHA Y3014B (DAC - sound - upper board)(IC19, IC20)
 1x crystal resonator CSB398P (upper board - near sound CPUs)(XL1)
 1x oscillator 24MHz (lower board near connectors to upper board)(XL1)

 ROMs
 8x TMS27C512-20JL (from 1 to 8 - upper board) (CPU)
 26x ST M27512FI (from 9 to 34 - upper board) (Sound + Tilemap)
 6x ST M27512FI (from 35 to 40 - lower board) (Sprites)

*/


ROM_START( goldnab2 )
	ROM_REGION( 0x0c0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ic39.1", 0x00000, 0x10000, CRC(71489c9a) SHA1(ec25463a2c63027d2635f0e27f5a971d68938fe3) )
	ROM_LOAD16_BYTE( "ic53.5", 0x00001, 0x10000, CRC(7a98512d) SHA1(f022a047d23051211e2de761b6c2289438b8f3cc) )
	ROM_LOAD16_BYTE( "ic38.2", 0x20000, 0x10000, CRC(8c8c95b1) SHA1(2b3897b8af587e6397dca0f3409a803432be07aa) )
	ROM_LOAD16_BYTE( "ic52.6", 0x20001, 0x10000, CRC(4f58f15b) SHA1(4dcd7e8cfd8fe5fe9f9259abf53fd0d4c64b5abd) )
	/* emtpy 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "ic37.3", 0x80000, 0x10000, CRC(056879fd) SHA1(fc0a910c1dc4cf535ad589f482f57379798b5524) )
	ROM_LOAD16_BYTE( "ic51.7", 0x80001, 0x10000, CRC(8c77d958) SHA1(f7c5abfec2a9d1724c83b1de2cd994bb4c42857c) )
	ROM_LOAD16_BYTE( "ic36.4", 0xa0000, 0x10000, CRC(b69ab892) SHA1(9b426058a80abb8dd3d6c0c55574fdc841889a72) )
	ROM_LOAD16_BYTE( "ic50.8", 0xa0001, 0x10000, CRC(3cf2f725) SHA1(1f620fcebe8533cba50736ae1d97c095abf1bc25) )

	ROM_REGION( 0x60000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "ic4.35",  0x00000, 0x10000, CRC(84587263) SHA1(3a88c8578a477a487a0a214a367042b9739f39eb) )
	ROM_LOAD( "ic18.38", 0x10000, 0x10000, CRC(63d72388) SHA1(ba0a582b1daf3a1e316237efbad17fcc0381643f) )
	ROM_LOAD( "ic3.36",  0x20000, 0x10000, CRC(f8b6ae4f) SHA1(55132c98955107e4b247992f7917a6ce588460a7) )
	ROM_LOAD( "ic17.39", 0x30000, 0x10000, CRC(e29baf4f) SHA1(3761cb2217599fe3f2f860f9395930b96ec52f47) )
	ROM_LOAD( "ic2.37",  0x40000, 0x10000, CRC(22f0667e) SHA1(2d11b2ce105a3db9c914942cace85aff17deded9) )
	ROM_LOAD( "ic16.40", 0x50000, 0x10000, CRC(afb1a7e4) SHA1(726fded9db72a881128b43f449d2baf450131f63) )

	ROM_REGION( 0x1c0000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "ic73.34", 		0x000001, 0x10000, CRC(28ba70c8) SHA1(a6f33e1404928b6d1006943494646d6cfbd60a4b) ) // mpr12378.b1  [1/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic74.33", 		0x020001, 0x10000, CRC(2ed96a26) SHA1(edcf915243e6f92d31cdfc53965438f6b6bff51d) ) // mpr12378.b1  [2/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic79.28", 		0x100001, 0x10000, CRC(84dccc5b) SHA1(10263d98d663f1170c3203066f391075a1d64ff5) ) // mpr12378.b1  [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic80.27",  		0x120001, 0x10000, CRC(3d41b995) SHA1(913d2a0c9a2ac6d36589966d7eb5537120ea6ff0) ) // mpr12378.b1  [4/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic58.22", 		0x000000, 0x10000, CRC(ede51fe0) SHA1(c05a2b51095a322bac5a67ee8886aecc186cbdfe) ) // mpr12379.b4  [1/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic59.21", 		0x020000, 0x10000, CRC(30989141) SHA1(f6e7ae258deedec11b1f80c19575c884aac26c56) ) // mpr12379.b4  [2/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic64.16", 		0x100000, 0x10000, CRC(38e4c92a) SHA1(8b8a596da0726bc02c412a68e5163770fe3e62e4) ) // mpr12379.b4  [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic65.15",  		0x120000, 0x10000, CRC(9fb1f9b6) SHA1(a193ff48f2bfbb4afbc322ae33cdf360afcb681e) ) // mpr12379.b4  [4/4]      IDENTICAL
 	ROM_LOAD16_BYTE( "ic75.32", 		0x040001, 0x10000, CRC(101d2fff) SHA1(1de1390c5f55f192491053c8aac31be3389aab2b) ) // mpr12380.b2  [1/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic76.31", 		0x060001, 0x10000, CRC(677e64a6) SHA1(e3d0d31097017c6cb1a7f41292783f18ce13b41c) ) // mpr12380.b2  [2/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic81.26", 		0x140001, 0x10000, CRC(a34e84d1) SHA1(5aa8836ef1bdf97e399899c911967840e9873bfb) ) // mpr12380.b2  [3/4]      99.992371%
	ROM_LOAD16_BYTE( "ic82.25", 		0x160001, 0x10000, CRC(ad1c1c90) SHA1(155f17593cfab1a117bb755b1edd0c473d455f91) ) // mpr12380.b2  [4/4]      IDENTICAL
  	ROM_LOAD16_BYTE( "ic60.20", 		0x040000, 0x10000, CRC(5853000d) SHA1(db7adf1de74c66f667ea7ccc41702576de081ff5) ) // mpr12381.b5  [1/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic61.19", 		0x060000, 0x10000, CRC(697b3276) SHA1(a8aeb2cfaca9368d5bfa14a67de36dbadd4a0585) ) // mpr12381.b5  [2/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic66.14", 		0x140000, 0x10000, CRC(753b01e8) SHA1(0180cf893d63e60e14e2fb0f5836b302f08f0228) ) // mpr12381.b5  [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic67.13", 		0x160000, 0x10000, CRC(5dd6d8db) SHA1(270886aebf4549c5cf456b1e90927b759a53e2e1) ) // mpr12381.b5  [4/4]      IDENTICAL
  	ROM_LOAD16_BYTE( "ic77.30", 		0x080001, 0x10000, CRC(a9519afe) SHA1(0550f596ef080db25241d88242d57edb3c7fb685) ) // mpr12382.b3  [1/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic78.29", 		0x0a0001, 0x10000, CRC(74df9232) SHA1(61286c83eb6f31983c0ed24ca2151c58a95238f1) ) // mpr12382.b3  [2/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic83.24", 		0x180001, 0x10000, CRC(a8c8c784) SHA1(8e998019b4dbf509179d41eb2b446647db3d00a6) ) // mpr12382.b3  [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic84.23", 		0x1a0001, 0x10000, CRC(cc3a922c) SHA1(3c96459d95961d463909b6b0e8c77916c3a018e3) ) // mpr12382.b3  [4/4]      IDENTICAL
   	ROM_LOAD16_BYTE( "ic62.18", 		0x080000, 0x10000, CRC(ac6ad195) SHA1(3c3fe38047698e28cae9193438fe0b85941476c5) ) // mpr12383.b6  [1/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic63.17", 		0x0a0000, 0x10000, CRC(03a905c4) SHA1(c526a744021a392c860ded37afd54a78e9f47778) ) // mpr12383.b6  [2/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic68.12", 		0x180000, 0x10000, CRC(cba013c7) SHA1(a0658aaf7893bc9fb8f0435cab9f77ceb1fb4e1d) ) // mpr12383.b6  [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "ic69.11", 		0x1a0000, 0x10000, CRC(bea4d237) SHA1(46e51e89b4ee1e2701da1004758d7da547a2e4c2) ) // mpr12383.b6  [4/4]      IDENTICAL

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* sound CPU + samples */
	ROM_LOAD( "ic45.10", 0x00000, 0x10000, CRC(804dfba8) SHA1(650ca61f78eb4d0aa256b554fb1570330166cc24) )
	ROM_LOAD( "ic46.9",  0x10000, 0x10000, CRC(634dd54e) SHA1(ac4ab0ad9c1ac634ff4dfb83232b0fa5cfb26fdd) )
ROM_END


// pre16
ROM_START( mjleague )
	ROM_REGION( 0x030000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7404.09b", 0x000000, 0x8000, CRC(ec1655b5) SHA1(5c1df364fa9733daa4478c5f88298089e4963c33) )
	ROM_LOAD16_BYTE( "epr-7401.06b", 0x000001, 0x8000, CRC(2befa5e0) SHA1(0a1681a4c7d62a5754ba6f3845436b4d08324246) )
	ROM_LOAD16_BYTE( "epr-7405.10b", 0x010000, 0x8000, CRC(7a4f4e38) SHA1(65a22097dd933e83f326bd64b3863915897780a6) )
	ROM_LOAD16_BYTE( "epr-7402.07b", 0x010001, 0x8000, CRC(b7bef762) SHA1(214450e0b094f99ef38dec2a3e5cbdb0b30e917d) )
	ROM_LOAD16_BYTE( "epra7406.11b", 0x020000, 0x8000, CRC(bb743639) SHA1(5d99638a79f02ce14374d3b1f3d9fbfc5c13c6e1) )
	ROM_LOAD16_BYTE( "epra7403.08b", 0x020001, 0x8000, CRC(d86250cf) SHA1(fb5dabb7b9b9fe0bbe93e28c60311c7b3256107a) )	// Fails memory test. Bad rom?

	ROM_REGION( 0x18000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr-7051.09a", 0x00000, 0x08000, CRC(10ca255a) SHA1(ccf58ffcac2f7fbdbfbdf32601a1b97f359cbd91) )
	ROM_LOAD( "epr-7052.10a", 0x08000, 0x08000, CRC(2550db0e) SHA1(28f8d68f43d26f12793fe295c205cc86adc4e96a) )
	ROM_LOAD( "epr-7053.11a", 0x10000, 0x08000, CRC(5bfea038) SHA1(01dc6e14cc7bba9f7930e68573c441fa2841f49a) )

	ROM_REGION( 0x50000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7055.05a", 0x000001, 0x8000, CRC(1fb860bd) SHA1(4a4155d0352dfae9e402a2b2f1558ef17b1303b4) )
	ROM_LOAD16_BYTE( "epr-7059.02b", 0x000000, 0x8000, CRC(3d14091d) SHA1(36208415b2012b6e948fefa15b0f7041748066be) )
	ROM_LOAD16_BYTE( "epr-7056.06a", 0x010001, 0x8000, CRC(b35dd968) SHA1(e306b5e38acf583d7b2089302622ad25ae5564b0) )
	ROM_LOAD16_BYTE( "epr-7060.03b", 0x010000, 0x8000, CRC(61bb3757) SHA1(5c87cf23be22b84e3dae746527ca057d870d6397) )
	ROM_LOAD16_BYTE( "epr-7057.07a", 0x020001, 0x8000, CRC(3e5a2b6f) SHA1(d3dbafb4acb916e02c978a156008bd75ba122fb7) )
	ROM_LOAD16_BYTE( "epr-7061.04b", 0x020000, 0x8000, CRC(c808dad5) SHA1(9b65acc8dc23b16e56327298188d1a6ab48b2b5d) )
	ROM_LOAD16_BYTE( "epr-7058.08a", 0x030001, 0x8000, CRC(b543675f) SHA1(35ffc9295a8849a18fabe156fdbc9801ea2179cd) )
	ROM_LOAD16_BYTE( "epr-7062.05b", 0x030000, 0x8000, CRC(9168eb47) SHA1(daaa7836e627a0679e65373d8f20a9383ba4c905) )
//  ROM_LOAD16_BYTE( "epr-7055.05a", 0x040001, 0x8000, CRC(1fb860bd) SHA1(4a4155d0352dfae9e402a2b2f1558ef17b1303b4) ) loaded twice??
//  ROM_LOAD16_BYTE( "epr-7059.02b", 0x040000, 0x8000, CRC(3d14091d) SHA1(36208415b2012b6e948fefa15b0f7041748066be) ) loaded twice??

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "eprc7054.01b", 0x00000, 0x8000, CRC(4443b744) SHA1(73359a6e9d62b382dee47fea31b9e17eb26a0321) )

	ROM_REGION( 0x1000, REGION_CPU3, 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, REGION_SOUND1, 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7063.01a", 0x00000, 0x8000, CRC(45d8908a) SHA1(e61f81f953c1a744ded36fed3b55774e4747af29) )
	ROM_LOAD( "epr-7065.02a", 0x08000, 0x8000, CRC(8c8f8cff) SHA1(fca5a916a8b25800ee5e8771e2ced0ed9bd737f4) )
	ROM_LOAD( "epr-7064.03a", 0x10000, 0x8000, CRC(159f6636) SHA1(66fa3f3e95a6ef3d3ff4ded09c05ab1131d9fbbb) )
	ROM_LOAD( "epr-7066.04a", 0x18000, 0x8000, CRC(f5cfa91f) SHA1(c85d68cbcd03fe1436bed12235c033610acc11ee) )
ROM_END

ROM_START( passht4b )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "pas4p.3", 0x000000, 0x10000, CRC(2d8bc946) SHA1(35d3e529d4815543d9876fd0545c3d686467abaa) )
	ROM_LOAD16_BYTE( "pas4p.4", 0x000001, 0x10000, CRC(e759e831) SHA1(dd5727dc28010cb988e4951723171171eb645ce8) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "pas4p.11", 0x00000, 0x10000, CRC(da20fbc9) SHA1(21dc8143f4d1cebae4f86e83495fa84e5293ba48) )
	ROM_LOAD( "pas4p.12", 0x10000, 0x10000, CRC(bebb9211) SHA1(4f56048f6f70b63f74a4c0d64456213d36ce5b26) )
	ROM_LOAD( "pas4p.13", 0x20000, 0x10000, CRC(e37506c3) SHA1(e6fbf15d58f321a3d052fefbe5a1901e4a1734ae) )

	ROM_REGION( 0x60000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "opr11862.b1",  0x00001, 0x10000, CRC(b6e94727) SHA1(0838e034f1f10d9cd1312c8c94b5c57387c0c271) )
	ROM_LOAD16_BYTE( "opr11865.b5",  0x00000, 0x10000, CRC(17e8d5d5) SHA1(ac1074b0a705be13c6e3391441e6cfec1d2b3f8a) )
	ROM_LOAD16_BYTE( "opr11863.b2",  0x20001, 0x10000, CRC(3e670098) SHA1(2cfc83f4294be30cd868738886ac546bd8489962) )
	ROM_LOAD16_BYTE( "opr11866.b6",  0x20000, 0x10000, CRC(50eb71cc) SHA1(463b4917ca19c7f4ad2c2845caa104d5e4a2dda3) )
	ROM_LOAD16_BYTE( "opr11864.b3",  0x40001, 0x10000, CRC(05733ca8) SHA1(1dbc7c99450ebe6a9fd8c0244fd3cb38b74984ef) )
	ROM_LOAD16_BYTE( "opr11867.b7",  0x40000, 0x10000, CRC(81e49697) SHA1(a70fa409e3555ad6c8f28930a7026fdf2deb8c65) )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "pas4p.1",  0x00000, 0x08000, CRC(e60fb017) SHA1(21298036eab55c74427f1c2e3a9623d41bca4849) )
	ROM_LOAD( "pas4p.2",  0x10000, 0x10000, CRC(092e016e) SHA1(713638749efa9dce19c547b84308236110bc85fe) )
ROM_END

ROM_START( passshtb )
	ROM_REGION( 0x020000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "pass3_2p.bin", 0x000000, 0x10000, CRC(26bb9299) SHA1(11bacf86dfdd8bcfbfb61f0ebc59890325c48adc) )
	ROM_LOAD16_BYTE( "pass4_2p.bin", 0x000001, 0x10000, CRC(06ac6d5d) SHA1(2dd71a8a956404326797de8beed7bca016c9919e) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr11854.b9",  0x00000, 0x10000, CRC(d31c0b6c) SHA1(610d04988da70c30300cc5614817eda9d2204f39) )
	ROM_LOAD( "opr11855.b10", 0x10000, 0x10000, CRC(b78762b4) SHA1(d594ef846bd7fed8da91a89906b39c1a2867a1fe) )
	ROM_LOAD( "opr11856.b11", 0x20000, 0x10000, CRC(ea49f666) SHA1(36ccd32cdcbb7fcc300628bb59c220ec3c324d82) )

	ROM_REGION( 0x60000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "opr11862.b1",  0x00001, 0x10000, CRC(b6e94727) SHA1(0838e034f1f10d9cd1312c8c94b5c57387c0c271) )
	ROM_LOAD16_BYTE( "opr11865.b5",  0x00000, 0x10000, CRC(17e8d5d5) SHA1(ac1074b0a705be13c6e3391441e6cfec1d2b3f8a) )
	ROM_LOAD16_BYTE( "opr11863.b2",  0x20001, 0x10000, CRC(3e670098) SHA1(2cfc83f4294be30cd868738886ac546bd8489962) )
	ROM_LOAD16_BYTE( "opr11866.b6",  0x20000, 0x10000, CRC(50eb71cc) SHA1(463b4917ca19c7f4ad2c2845caa104d5e4a2dda3) )
	ROM_LOAD16_BYTE( "opr11864.b3",  0x40001, 0x10000, CRC(05733ca8) SHA1(1dbc7c99450ebe6a9fd8c0244fd3cb38b74984ef) )
	ROM_LOAD16_BYTE( "opr11867.b7",  0x40000, 0x10000, CRC(81e49697) SHA1(a70fa409e3555ad6c8f28930a7026fdf2deb8c65) )

	ROM_REGION( 0x30000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr11857.a7",  0x00000, 0x08000, CRC(789edc06) SHA1(8c89c94e503513c287807d187de78a7fbd75a7cf) )
	ROM_LOAD( "epr11858.a8",  0x10000, 0x08000, CRC(08ab0018) SHA1(0685f80a7d403208c9cfffea3f2035324f3924fe) )
	ROM_LOAD( "epr11859.a9",  0x18000, 0x08000, CRC(8673e01b) SHA1(e79183ab30e683fdf61ced2e9dbe010567c324cb) )
	ROM_LOAD( "epr11860.a10", 0x20000, 0x08000, CRC(10263746) SHA1(1f981fb185c6a9795208ecdcfba36cf892a99ed5) )
	ROM_LOAD( "epr11861.a11", 0x28000, 0x08000, CRC(38b54a71) SHA1(68ec4ef5b115844214ff2213be1ce6678904fbd2) )
ROM_END

/* Shinobi bootleg by 'Beta' (7751 replaced by what? Sample rom is different, but no extra sound CPU rom present, missing?) */
/* otherwise it seems to run fine on System 16A */
ROM_START( shinoblb )
	ROM_REGION( 0x040000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "4.3k", 0x000000, 0x10000, CRC(c178a39c) SHA1(05ff1679cdfc3618df8b3fabdeab64b1f2299aa3) )
	ROM_LOAD16_BYTE( "2.3n", 0x000001, 0x10000, CRC(5ad8ebf2) SHA1(b22e0c8d4b27c553abface17c625e207d19417ab) )
	ROM_LOAD16_BYTE( "5.2k", 0x020000, 0x10000, CRC(a2a620bd) SHA1(f8b135ce14d6c5eac5e40ddfd5ad2f1e6f2bc7a6) )
	ROM_LOAD16_BYTE( "3.2n", 0x020001, 0x10000, CRC(a3ceda52) SHA1(97a1c52a162fb1d43b3f8f16613b70ce582a8d26) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "8.3b", 0x00000, 0x10000, CRC(46627e7d) SHA1(66bb5b22a2100e7b9df303007a837bc2d52cf7ba) )
	ROM_LOAD( "7.4b", 0x10000, 0x10000, CRC(87d0f321) SHA1(885b38eaff2dcaeab4eeaa20cc8a2885d520abd6) )
	ROM_LOAD( "6.5b", 0x20000, 0x10000, CRC(efb4af87) SHA1(0b8a905023e1bc808fd2b1c3cfa3778cde79e659) )

	ROM_REGION16_BE( 0x080000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "9.6r",  0x00001, 0x08000, CRC(611f413a) SHA1(180f83216e2dfbfd77b0fb3be83c3042954d12df) )
	ROM_CONTINUE(             0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "13.8r", 0x00000, 0x08000, CRC(5eb00fc1) SHA1(97e02eee74f61fabcad2a9e24f1868cafaac1d51) )
	ROM_CONTINUE(             0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "10.6q", 0x10001, 0x08000, CRC(3c0797c0) SHA1(df18c7987281bd9379026c6cf7f96f6ae49fd7f9) )
	ROM_CONTINUE(             0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "14.8q", 0x10000, 0x08000, CRC(25307ef8) SHA1(91ffbe436f80d583524ee113a8b7c0cf5d8ab286) )
	ROM_CONTINUE(             0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "11.6p", 0x20001, 0x08000, CRC(c29ac34e) SHA1(b5e9b8c3233a7d6797f91531a0d9123febcf1660) )
	ROM_CONTINUE(             0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "15.8p", 0x20000, 0x08000, CRC(04a437f8) SHA1(ea5fed64443236e3404fab243761e60e2e48c84c) )
	ROM_CONTINUE(             0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "12.6n", 0x30001, 0x08000, CRC(41f41063) SHA1(5cc461e9738dddf9eea06831fce3702d94674163) )
	ROM_CONTINUE(             0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "16.8n", 0x30000, 0x08000, CRC(b6e1fd72) SHA1(eb86e4bf880bd1a1d9bcab3f2f2e917bcaa06172) )
	ROM_CONTINUE(             0x70000, 0x08000 )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "1.5s", 0x0000, 0x8000, CRC(dd50b745) SHA1(52e1977569d3713ad864d607170c9a61cd059a65) )

	ROM_REGION( 0x08000, REGION_SOUND1, 0 ) /* sound samples (played by what?, not the same as the original) */
	ROM_LOAD( "17.6u", 0x0000, 0x8000, CRC(b7a6890c) SHA1(6431df82c7dbe454cabc6084c1a677ebb42ae4b3) )
ROM_END

/* Shinobi bootleg by 'Datsu' - Sound hardware is different */
ROM_START( shinobld )
	ROM_REGION( 0x040000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "12.bin", 0x000001, 0x10000, CRC(757a0c71) SHA1(8f476b0fd5f5dd480489af09b99585c58a4801fc) )
	ROM_LOAD16_BYTE( "14.bin", 0x000000, 0x10000, CRC(a65870b2) SHA1(3e9b4aa694bf86ef9db4756ebaa3d8d87d7f269a) )
	ROM_LOAD16_BYTE( "13.bin", 0x020001, 0x10000, CRC(c4334bcd) SHA1(ea1dd23ca6fbf632d8e10bbb9ced6515a69bd14a) )
	ROM_LOAD16_BYTE( "15.bin", 0x020000, 0x10000, CRC(b70a6ec1) SHA1(79db41c36d6a053bcdc355b46b19ae938a7755a9) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "9.bin",  0x10000, 0x10000, CRC(565e11c6) SHA1(e063400b3d0470b932d75da0be9cd4b446189dea) )
	ROM_LOAD( "10.bin", 0x20000, 0x10000, CRC(7cc40b6c) SHA1(ffad7eef7ab2ff9a2e49a8d71b5785a61fa3c675) )
	ROM_LOAD( "11.bin", 0x00000, 0x10000, CRC(0f6c7b1c) SHA1(defc76592c285b3396e89a3cff7a73f3a948117f) )

	ROM_REGION( 0x080000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "5.bin", 0x00001, 0x08000, CRC(611f413a) SHA1(180f83216e2dfbfd77b0fb3be83c3042954d12df) )
	ROM_CONTINUE(             0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "3.bin", 0x00000, 0x08000, CRC(5eb00fc1) SHA1(97e02eee74f61fabcad2a9e24f1868cafaac1d51) )
	ROM_CONTINUE(             0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "8.bin", 0x10001, 0x08000, CRC(3c0797c0) SHA1(df18c7987281bd9379026c6cf7f96f6ae49fd7f9) )
	ROM_CONTINUE(             0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "2.bin", 0x10000, 0x08000, CRC(25307ef8) SHA1(91ffbe436f80d583524ee113a8b7c0cf5d8ab286) )
	ROM_CONTINUE(             0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "6.bin", 0x30001, 0x08000, CRC(c29ac34e) SHA1(b5e9b8c3233a7d6797f91531a0d9123febcf1660) )
	ROM_CONTINUE(             0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "4.bin", 0x30000, 0x08000, CRC(04a437f8) SHA1(ea5fed64443236e3404fab243761e60e2e48c84c) )
	ROM_CONTINUE(             0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "7.bin", 0x40001, 0x08000, CRC(41f41063) SHA1(5cc461e9738dddf9eea06831fce3702d94674163) )
	ROM_CONTINUE(             0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "1.bin", 0x40000, 0x08000, CRC(b6e1fd72) SHA1(eb86e4bf880bd1a1d9bcab3f2f2e917bcaa06172) )
	ROM_CONTINUE(             0x70000, 0x08000 )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU + data */
	ROM_LOAD( "16.bin", 0x0000, 0x10000, CRC(52c8364e) SHA1(01d30b82f92498d155d2e31d43d58dff0285cce3) )
ROM_END


// sys16B

// sys16A custom

/*


*/

// sys16B
ROM_START( tetrisbl )
	ROM_REGION( 0x040000, REGION_CPU1, ROMREGION_ERASEFF ) /* 68000 code */
	ROM_LOAD16_BYTE( "rom2.bin", 0x000000, 0x10000, CRC(4d165c38) SHA1(04706b1977ae18bd09bafaf8ea65f8e5f32e04b8) )
	ROM_LOAD16_BYTE( "rom1.bin", 0x000001, 0x10000, CRC(1e912131) SHA1(8f53504ac08942ee340489d84eab825e654d0a2c) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr12165.b9",  0x00000, 0x10000, CRC(62640221) SHA1(c311d3847a981d0e1609f9b3d80481565d32d78c) )
	ROM_LOAD( "epr12166.b10", 0x10000, 0x10000, CRC(9abd183b) SHA1(621b017cb34973f9227be383e26b5cd41aea9422) )
	ROM_LOAD( "epr12167.b11", 0x20000, 0x10000, CRC(2495fd4e) SHA1(2db94ead9223a67238a97e724668076fc43e5534) )

	ROM_REGION( 0x020000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "obj0-o.rom", 0x00001, 0x10000, CRC(2fb38880) SHA1(0e1b601bbda78d1887951c1f7e752531c281bc83) )
	ROM_LOAD16_BYTE( "obj0-e.rom", 0x00000, 0x10000, CRC(d6a02cba) SHA1(d80000f92e754e89c6ca7b7273feab448fc9a061) )

	ROM_REGION( 0x40000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12168.a7", 0x0000, 0x8000, CRC(bd9ba01b) SHA1(fafa7dc36cc057a50ae4cdf7a35f3594292336f4) )
ROM_END

// sys16B
ROM_START( tturfbl )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "tt042197.rom", 0x00000, 0x10000, CRC(deee5af1) SHA1(0caba775021dc7e28ac6b7af8eac4f49d3102c83) )
	ROM_LOAD16_BYTE( "tt06c794.rom", 0x00001, 0x10000, CRC(90e6a95a) SHA1(014a0ae5cebcba9cc99e6ccde4ad5d938fab915c) )
	ROM_LOAD16_BYTE( "tt030be3.rom", 0x20000, 0x10000, CRC(100264a2) SHA1(d1ea4bf93f5472901ce95200f546ce9b58936aea) )
	ROM_LOAD16_BYTE( "tt05ef8a.rom", 0x20001, 0x10000, CRC(f787a948) SHA1(512b8cb2f5e9795171951e02c07cae957db41334) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "tt1574b3.rom", 0x00000, 0x10000, CRC(e9e630da) SHA1(e8471dedbb25475e4814d78b56f579fe9110461e) )
	ROM_LOAD( "tt16cf44.rom", 0x10000, 0x10000, CRC(4c467735) SHA1(8338b6605cbe2e076da0b3e3a47630409a79f002) )
	ROM_LOAD( "tt17d59e.rom", 0x20000, 0x10000, CRC(60c0f2fe) SHA1(3fea4ed757d47628f59ff940e40cb86b3b5b443b) )

	ROM_REGION( 0x80000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "12279.1b", 0x00001, 0x10000, CRC(7a169fb1) SHA1(1ec6da0d2cfcf727e61f61c847fd8b975b64f944) )
	ROM_LOAD16_BYTE( "12283.5b", 0x00000, 0x10000, CRC(ae0fa085) SHA1(ae9af92d4dd0c8a0f064d24e647522b588fbd7f7) )
	ROM_LOAD16_BYTE( "12278.2b", 0x20001, 0x10000, CRC(961d06b7) SHA1(b1a9dea63785bfa2c0e7b931387b91dfcd27d79b) )
	ROM_LOAD16_BYTE( "12282.6b", 0x20000, 0x10000, CRC(e8671ee1) SHA1(a3732938c370f1936d867aae9c3d1e9bbfb57ede) )
	ROM_LOAD16_BYTE( "12277.3b", 0x40001, 0x10000, CRC(f16b6ba2) SHA1(00cc04c7b5aad82d51d2d252e1e57bcdc5e2c9e3) )
	ROM_LOAD16_BYTE( "12281.7b", 0x40000, 0x10000, CRC(1ef1077f) SHA1(8ce6fd7d32a20b93b3f91aaa43fe22720da7236f) )
	ROM_LOAD16_BYTE( "12276.4b", 0x60001, 0x10000, CRC(838bd71f) SHA1(82d9d127438f5e1906b1cf40bf3b4727f2ee5685) )
	ROM_LOAD16_BYTE( "12280.8b", 0x60000, 0x10000, CRC(639a57cb) SHA1(84fd8b96758d38f9e1ba1a3c2cf8099ec0452784) )

	ROM_REGION( 0x30000, REGION_CPU2, 0 ) //* sound CPU */
	ROM_LOAD( "tt014d68.rom", 0x10000, 0x10000, CRC(d4aab1d9) SHA1(94885896d59da1ecabe2377a194fcf61eaae3765) )
	ROM_LOAD( "tt0246ff.rom", 0x20000, 0x10000, CRC(bb4bba8f) SHA1(b182a7e1d0425e93c2c1b93472aafd30a6af6907) )
ROM_END


// sys16B
ROM_START( wb3bbl )
	ROM_REGION( 0x040000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "wb3_03", 0x000000, 0x10000, CRC(0019ab3b) SHA1(89d49a437690fa6e0c35bb9f1450042f89504714) )
	ROM_LOAD16_BYTE( "wb3_05", 0x000001, 0x10000, CRC(196e17ee) SHA1(71e4345b2c3d1612a3d424c9310fad1e23c8a9f7) )
	ROM_LOAD16_BYTE( "wb3_02", 0x020000, 0x10000, CRC(c87350cb) SHA1(55a8cb68d70b6060dd9a55e281e216ce3917ea5b) )
	ROM_LOAD16_BYTE( "wb3_04", 0x020001, 0x10000, CRC(565d5035) SHA1(e28a132f1a4ce9466945e231c54502178748af98) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "wb3_14", 0x00000, 0x10000, CRC(d3f20bca) SHA1(0a87f709f8e2a913473512ede408e2cbc535443f) )
	ROM_LOAD( "wb3_15", 0x10000, 0x10000, CRC(96ff9d52) SHA1(791a9da4860e0d42fba98f80a3c6725ad8c73e33) )
	ROM_LOAD( "wb3_16", 0x20000, 0x10000, CRC(afaf0d31) SHA1(d4309329a0a543250788146b63b27ff058c02fc3) )

	ROM_REGION( 0x080000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr12093.b4", 0x000001, 0x010000, CRC(4891e7bb) SHA1(1be04fcabe9bfa8cf746263a5bcca67902a021a0) )
	ROM_LOAD16_BYTE( "epr12097.b8", 0x000000, 0x010000, CRC(e645902c) SHA1(497cfcf6c25cc2e042e16dbcb1963d2223def15a) )
	ROM_LOAD16_BYTE( "epr12091.b2", 0x020001, 0x010000, CRC(8409a243) SHA1(bcbb9510a6499d8147543d6befa5a49f4ac055d9) )
	ROM_LOAD16_BYTE( "epr12095.b6", 0x020000, 0x010000, CRC(e774ec2c) SHA1(a4aa15ec7be5539a740ad02ff720458018dbc536) )
	ROM_LOAD16_BYTE( "epr12090.b1", 0x040001, 0x010000, CRC(aeeecfca) SHA1(496124b170a725ad863c741d4e021ab947511e4c) )
	ROM_LOAD16_BYTE( "epr12094.b5", 0x040000, 0x010000, CRC(615e4927) SHA1(d23f164973afa770714e284a77ddf10f18cc596b) )
	ROM_LOAD16_BYTE( "epr12092.b3", 0x060001, 0x010000, CRC(5c2f0d90) SHA1(e0fbc0f841e4607ad232931368b16e81440a75c4) )
	ROM_LOAD16_BYTE( "epr12096.b7", 0x060000, 0x010000, CRC(0cd59d6e) SHA1(caf754a461feffafcfe7bfc6e89da76c4db257c5) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12127.a10", 0x0000, 0x8000, CRC(0bb901bb) SHA1(c81b198df8e3b0ec568032c76addf0d1a1711194) )
ROM_END


/* pre-System16 */
/*          rom       parent    machine   inp       init */
/* Alien Syndrome */

/* System16A */
/*          rom       parent    machine   inp       init */
GAME( 1987, shinoblb,   shinobi,  shinob2,  shinobi,  shinobl,  ROT0,   "[Sega] (Beta bootleg)", "Shinobi (Beta bootleg)", 0 )
GAME( 1987, shinobld,   shinobi,  shinob2,  shinobi,  shinobl,  ROT0,   "[Sega] (Datsu bootleg)", "Shinobi (Datsu bootleg)", 0 )

/* System16B */
/*          rom       parent    machine   inp       init */

GAME( 1989, bayrtbl1, bayroute, bayroute, bayroute, bayrtbl1, ROT0,   "bootleg", "Bay Route (bootleg set 1)", GAME_NOT_WORKING )
GAME( 1989, bayrtbl2, bayroute, bayroute, bayroute, bayrtbl1, ROT0,   "bootleg", "Bay Route (bootleg set 2)", GAME_NOT_WORKING )
GAME( 1989, dduxbl,   ddux,     dduxbl,   ddux,     dduxbl,   ROT0,   "bootleg", "Dynamite Dux (bootleg)", 0 )
GAME( 1989, eswatbl,  eswat,    eswatbl,  eswat,    eswatbl,  ROT0,   "bootleg", "E-Swat - Cyber Police (bootleg)", GAME_IMPERFECT_SOUND )
GAME( 1989, fpointbl, fpoint,   fpointbl, fpoint,   fpointbl, ROT0,   "bootleg", "Flash Point (World, bootleg)", 0 )
GAME( 1989, fpointbj, fpoint,   fpointbl, fpointbj, fpointbl, ROT0,   "bootleg", "Flash Point (Japan, bootleg)", 0 )

GAME( 1989, goldnabl, goldnaxe, goldnaxe, goldnaxe, goldnabl, ROT0,   "bootleg", "Golden Axe (bootleg, encrypted)", GAME_NOT_WORKING )
GAME( 1989, goldnab2, goldnaxe, goldnaxe, goldnaxe, goldnabl, ROT0,   "bootleg", "Golden Axe (bootleg)", GAME_NOT_WORKING )

GAME( 1988, passshtb, passsht,  passsht,  passsht,  passsht,  ROT270, "bootleg", "Passing Shot (2 Players) (bootleg)", GAME_IMPERFECT_SOUND )
GAME( 1988, passht4b, passsht,  passht4b, passht4b, passht4b, ROT270, "bootleg", "Passing Shot (4 Players) (bootleg)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 1988, tetrisbl, tetris,   tetrisbl, tetris,   tetrisbl, ROT0,   "bootleg", "Tetris (bootleg)", 0 )
GAME( 1989, tturfbl,  tturf,    tturfbl,  tturf,    tturfbl,  ROT0,   "bootleg", "Tough Turf (bootleg)", GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1988, wb3bbl,   wb3,      wb3bbl,   wb3b,     wb3bbl,   ROT0,   "bootleg", "Wonder Boy III - Monster Lair (bootleg)", 0 )
