/***************************************************************************

Century CVS System

MAIN BOARD:

             FLAG LOW               |   FLAG HIGH
------------------------------------+-----------------------------------
1C00-1FFF    SYSTEM RAM             |   SYSTEM RAM
                                    |
                                    |
1800-1BFF    ATTRIBUTE RAM 32X28    |   SCREEN RAM 32X28
1700         2636 1                 |   CHARACTER RAM 1 256BYTES OF 1K*
1600         2636 2                 |   CHARACTER RAM 2 256BYTES OF 1K*
1500         2636 3                 |   CHARACTER RAM 3 256BYTES OF 1K*
1400         BULLET RAM             |   PALETTE RAM 16BYTES
                                    |
                                    |
0000-13FF    PROGRAM ROM'S          |   PROGRAM ROM'S

* Note the upper two address lines are latched using an I/O read. The I/O map only has
  space for 128 character bit maps

The CPU CANNOT read the character PROMs
        ------

                         CVS I/O MAP
                         -----------
ADR14 ADR13 | READ                                      | WRITE
------------+-------------------------------------------+-----------------------------
  1     0   | COLLISION RESET                           | D0 = STARS ON
            |                                           | D1 = SHADE BRIGHTER TO RIGHT
            |                                           | D2 = SCREEN ROTATE
 read/write |                                           | D3 = SHADE BRIGHTER TO LEFT
   Data     |                                           | D4 = LAMP 1 (CN1 1)
            |                                           | D5 = LAMP 2 (CN1 2)
            |                                           | D6 = SHADE BRIGHTER TO BOTTOM
            |                                           | D7 = SHADE BRIGHTER TO TOP
------------+-------------------------------------------+------------------------------
  X     1   | A0-2: 0 STROBE CN2 1                      | VERTICAL SCROLL OFFSET
            |       1 STROBE CN2 11                     |
            |       2 STROBE CN2 2                      |
            |       3 STROBE CN2 3                      |
            |       4 STROBE CN2 4                      |
            |       5 STROBE CN2 12                     |
            |       6 STROBE DIP SW3                    |
            |       7 STROBE DIP SW2                    |
            |                                           |
 read/write | A4-5: CHARACTER PROM/RAM SELECTION MODE   |
  extended  | There are 256 characters in total. The    |
            | higher ones can be user defined in RAM.   |
            | The split between PROM and RAM characters |
            | is variable.                              |
            |                ROM              RAM       |
            | A4 A5 MODE  CHARACTERS       CHARACTERS   |
            | 0  0   0    224 (0-223)      32  (224-255)|
            | 0  1   1    192 (0-191)      64  (192-255)|
            | 1  0   2    256 (0-255)      0            |
            | 1  1   3    128 (0-127)      128 (128-255)|
            |                                           |
            |                                           |
            | A6-7: SELECT CHARACTER RAM's              |
            |       UPPER ADDRESS BITS A8-9             |
            |       (see memory map)                    |
------------+-------------------------------------------+-------------------------------
  0     0   | COLLISION DATA BYTE:                      | SOUND CONTROL PORT
            | D0 = OBJECT 1 AND 2                       |
            | D1 = OBJECT 2 AND 3                       |
 read/write | D2 = OBJECT 1 AND 3                       |
  control   | D3 = ANY OBJECT AND BULLET                |
            | D4 = OBJECT 1 AND CP1 OR CP2              |
            | D5 = OBJECT 2 AND CP1 OR CP2              |
            | D6 = OBJECT 3 AND CP1 OR CP2              |
            | D7 = BULLET AND CP1 OR CP2                |
------------+-------------------------------------------+-------------------------------

Driver by
    Mike Coates

Hardware Info
 Malcolm & Darren

***************************************************************************/

#include "driver.h"
#include "cpu/s2650/s2650.h"
#include "sound/dac.h"
#include "sound/5110intf.h"
#include "sound/tms5110.h"
#include "video/s2636.h"
#include "cvs.h"


#define VERBOSE 1
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CVS_MAIN_CPU_INDEX	(0)
#define CVS_AUDIO_CPU_INDEX	(1)

#define CVS_REGION_SPEECH	(REGION_SOUND1)



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT8 *cvs_color_ram;
UINT8 *cvs_video_ram;
UINT8 *cvs_bullet_ram;
UINT8 *cvs_palette_ram;
UINT8 *cvs_character_ram;

static UINT8 *cvs_4_bit_dac_data;

static UINT8 character_banking_mode;
static UINT16 character_ram_page_start;
static UINT16 speech_rom_bit_address;



UINT8 cvs_get_character_banking_mode(void)
{
	return character_banking_mode;
}



/*************************************
 *
 *  Multiplexed memory access
 *
 *************************************/

READ8_HANDLER( cvs_video_or_color_ram_r )
{
	if (!activecpu_get_reg(S2650_FO))
		return cvs_color_ram[offset];
	else
		return cvs_video_ram[offset];
}

WRITE8_HANDLER( cvs_video_or_color_ram_w )
{
	if (!activecpu_get_reg(S2650_FO))
		cvs_color_ram[offset] = data;
	else
		cvs_video_ram[offset] = data;
}


READ8_HANDLER( cvs_bullet_ram_or_palette_r )
{
	if (!activecpu_get_reg(S2650_FO))
		return cvs_bullet_ram[offset];
	else
		return cvs_palette_ram[offset & 0x0f];
}

WRITE8_HANDLER( cvs_bullet_ram_or_palette_w )
{
	if (!activecpu_get_reg(S2650_FO))
		cvs_bullet_ram[offset] = data;
	else
		cvs_palette_ram[offset & 0x0f] = data;
}


READ8_HANDLER( cvs_s2636_1_or_character_ram_r )
{
	if (!activecpu_get_reg(S2650_FO))
		return s2636_1_ram[offset];
	else
		return cvs_character_ram[(0 * 0x800) | 0x400 | character_ram_page_start | offset];
}

WRITE8_HANDLER( cvs_s2636_1_or_character_ram_w )
{
	if (!activecpu_get_reg(S2650_FO))
		s2636_1_ram[offset] = data;
	else
		cvs_character_ram[(0 * 0x800) | 0x400 | character_ram_page_start | offset] = data;
}


READ8_HANDLER( cvs_s2636_2_or_character_ram_r )
{
	if (!activecpu_get_reg(S2650_FO))
		return s2636_2_ram[offset];
	else
		return cvs_character_ram[(1 * 0x800) | 0x400 | character_ram_page_start | offset];
}

WRITE8_HANDLER( cvs_s2636_2_or_character_ram_w )
{
	if (!activecpu_get_reg(S2650_FO))
		s2636_2_ram[offset] = data;
	else
		cvs_character_ram[(1 * 0x800) | 0x400 | character_ram_page_start | offset] = data;
}


READ8_HANDLER( cvs_s2636_3_or_character_ram_r )
{
	if (!activecpu_get_reg(S2650_FO))
		return s2636_3_ram[offset];
	else
		return cvs_character_ram[(2 * 0x800) | 0x400 | character_ram_page_start | offset];
}

WRITE8_HANDLER( cvs_s2636_3_or_character_ram_w )
{
	if (!activecpu_get_reg(S2650_FO))
		s2636_3_ram[offset] = data;
	else
		cvs_character_ram[(2 * 0x800) | 0x400 | character_ram_page_start | offset] = data;
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( cvs_main_cpu_interrupt )
{
	cpunum_set_input_line_vector(CVS_MAIN_CPU_INDEX, 0, 0x03);
	cpunum_set_input_line(CVS_MAIN_CPU_INDEX, 0, PULSE_LINE);

	cvs_scroll_stars();
}


static void cvs_audio_cpu_interrupt(void)
{
	cpunum_set_input_line_vector(CVS_AUDIO_CPU_INDEX, 0, 0x03);
	cpunum_set_input_line(CVS_AUDIO_CPU_INDEX, 0, HOLD_LINE);
}



/*************************************
 *
 *  Input port access
 *
 *************************************/

static READ8_HANDLER( cvs_input_r )
{
	UINT8 ret = 0;

	/* the upper 4 bits of the address is used to select the character banking attributes */
	character_banking_mode = (offset >> 4) & 0x03;
	character_ram_page_start = (offset << 2) & 0x300;

	/* the lower 4 (or 3?) bits select the port to read */
	switch (offset & 0x0f)	/* might be 0x07 */
	{
	case 0x00:  ret = input_port_0_r(0); break;
	case 0x02:  ret = input_port_1_r(0); break;
	case 0x03:  ret = input_port_2_r(0); break;
	case 0x04:  ret = input_port_3_r(0); break;
	case 0x06:  ret = input_port_4_r(0); break;
	case 0x07:  ret = input_port_5_r(0); break;
	default:    logerror("%04x : CVS: Reading unmapped input port 0x%02x\n", activecpu_get_pc(), offset & 0x0f); break;
	}

	return ret;
}



/*************************************
 *
 *  4-bit DAC
 *
 *************************************/

static WRITE8_HANDLER( cvs_4_bit_dac_data_w )
{
	/* merge D7 of each byte into D0-D3 */
	UINT8 dac_value = ((cvs_4_bit_dac_data[0] >> 7) << 0) |
					  ((cvs_4_bit_dac_data[1] >> 7) << 1) |
					  ((cvs_4_bit_dac_data[2] >> 7) << 2) |
					  ((cvs_4_bit_dac_data[3] >> 7) << 3);

	/* scale up to a full byte and output */
	DAC_1_data_w(0, (dac_value << 4) | dac_value);
}



/*************************************
 *
 *  Speech hardware
 *
 *************************************/

static void speech_execute_command(UINT8 command)
{
	/* reset */
	if (command == 0x3f)
	{
		tms5110_CTL_w(0, TMS5110_CMD_RESET);

		tms5110_PDC_w(0,0);
		tms5110_PDC_w(0,1);
		tms5110_PDC_w(0,0);

		tms5110_PDC_w(0,0);
		tms5110_PDC_w(0,1);
		tms5110_PDC_w(0,0);

		tms5110_PDC_w(0,0);
		tms5110_PDC_w(0,1);
		tms5110_PDC_w(0,0);

		speech_rom_bit_address = 0;

		LOG(("%04x : CVS: Speech reset\n", activecpu_get_pc()));
	}
	/* start */
	else
	{
		tms5110_CTL_w(0, TMS5110_CMD_SPEAK);

		tms5110_PDC_w(0, 0);
		tms5110_PDC_w(0, 1);
		tms5110_PDC_w(0, 0);

		speech_rom_bit_address = command * 0x80 * 8;

		LOG(("%04x : CVS: Speech command = %02x (%04x)\n", activecpu_get_pc(), command, speech_rom_bit_address >> 3));
	}
}


static int speech_rom_read_bit(void)
{
	UINT8 *ROM = memory_region(CVS_REGION_SPEECH);
    int bit;

	/* before reading the bit, clamp the address to the region length */
	speech_rom_bit_address = speech_rom_bit_address & ((memory_region_length(CVS_REGION_SPEECH) * 8) - 1);
	bit = (ROM[speech_rom_bit_address >> 3] >> (speech_rom_bit_address & 0x07)) & 0x01;

	/* prepare for next bit */
	speech_rom_bit_address = speech_rom_bit_address + 1;

	return bit;
}


static const struct TMS5110interface tms5100_interface =
{
	//TMS5110_IS_5110A,
	TMS5110_IS_5100,
	-1,					 /* ROM region */
	speech_rom_read_bit, /* M0 callback function. Called whenever chip requests a single bit of data */
	NULL
};



/*************************************
 *
 *  Inter-CPU communications
 *
 *************************************/

static WRITE8_HANDLER( audio_command_w )
{
    /* cause interrupt on audio CPU if bit 7 set */
	if (data & 0x80)
	{
	   	soundlatch_w(0, data);
		cvs_audio_cpu_interrupt();

		LOG(("%04x : CVS: Audio command = %02x\n", activecpu_get_pc(), data));
	}

    /* speech command if bit 6 is not set */
    if ((data & 0x40) == 0)
 		speech_execute_command(data & 0x03f);
}



static READ8_HANDLER( cvs_393hz_clock_r )
{
  	if(cpu_scalebyfcount(6) & 1) return 0x80;
    else return 0;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

static MACHINE_START( cvs )
{
	/* allocate memory */
	cvs_color_ram = auto_malloc(0x400);
	cvs_palette_ram = auto_malloc(0x10);
	cvs_character_ram = auto_malloc(3 * 0x800);  /* only half is used, but
                                                    by allocating twice the amount,
                                                    we can use the same gfx_layout */
	/* register state save */
	state_save_register_global(character_banking_mode);
	state_save_register_global(character_ram_page_start);
	state_save_register_global(speech_rom_bit_address);
}



/*************************************
 *
 *  Main CPU memory/IO handlers
 *
 *************************************/

static ADDRESS_MAP_START( cvs_main_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
    AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_bullet_ram_or_palette_r, cvs_bullet_ram_or_palette_w) AM_BASE(&cvs_bullet_ram)
    AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_s2636_3_or_character_ram_r, cvs_s2636_3_or_character_ram_w) AM_BASE(&s2636_3_ram)
    AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_s2636_2_or_character_ram_r, cvs_s2636_2_or_character_ram_w) AM_BASE(&s2636_2_ram)
    AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_s2636_1_or_character_ram_r, cvs_s2636_1_or_character_ram_w) AM_BASE(&s2636_1_ram)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_READWRITE(cvs_video_or_color_ram_r, cvs_video_or_color_ram_w) AM_BASE(&cvs_video_ram)
    AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x4000, 0x53ff) AM_ROM
	AM_RANGE(0x6000, 0x73ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvs_main_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(cvs_input_r, cvs_scroll_w)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE(cvs_collision_clear, cvs_video_fx_w)
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE(cvs_collision_r, audio_command_w)
    AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_6_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU memory/IO handlers
 *
 *************************************/

static ADDRESS_MAP_START( cvs_audio_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(13) )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
    AM_RANGE(0x1000, 0x107f) AM_RAM
    AM_RANGE(0x1800, 0x1800) AM_READ(soundlatch_r)
    AM_RANGE(0x1840, 0x1840) AM_WRITE(DAC_0_data_w)
    AM_RANGE(0x1880, 0x1883) AM_WRITE(cvs_4_bit_dac_data_w) AM_BASE(&cvs_4_bit_dac_data)
    AM_RANGE(0x1884, 0x1887) AM_WRITE(MWA8_NOP)		/* not connected to anything */
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvs_audio_cpu_io_map, ADDRESS_SPACE_IO, 8 )
    AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(cvs_393hz_clock_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Standard CVS port definition
 *
 *************************************/

static INPUT_PORTS_START( cvs )

	PORT_START	/* Matrix 0 */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )				/* confirmed */
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) 		  	/* confirmed */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )				/* confirmed */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )				/* confirmed */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )			/* confirmed */
    PORT_BIT( 0xC0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* Dunno */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )			/* confirmed */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )		/* confirmed */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) 		/* confirmed */
    PORT_BIT( 0xcc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* Dunno */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )				/* duplicate? */
    PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* Dunno */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )		/* confirmed */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )		/* confirmed */
    PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* SW BANK 3 */
	PORT_DIPNAME( 0x01, 0x00, "Color" )
	PORT_DIPSETTING(    0x00, "Option 1" )
	PORT_DIPSETTING(    0x01, "Option 2" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
    PORT_DIPNAME( 0x0C, 0x00, DEF_STR( Bonus_Life ) )
    PORT_DIPSETTING(    0x00, "10k only" )
    PORT_DIPSETTING(    0x04, "20k only" )
    PORT_DIPSETTING(    0x08, "30k and every 40k" )
    PORT_DIPSETTING(    0x0c, "40k and every 80k" )
	PORT_DIPNAME( 0x10, 0x00, "Registration Length" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x20, 0x00, "Registration" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_START	/* SW BANK 2 */
	PORT_DIPNAME( 0x03, 0x00, "Coins for 1 Play" )			/* confirmed */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
    PORT_DIPSETTING(    0x02, "3" )
    PORT_DIPSETTING(    0x03, "4" )
    PORT_DIPNAME( 0x0C, 0x0c, "Plays for 1 Coin" )			/* confirmed */
    PORT_DIPSETTING(    0x0C, "2" )
    PORT_DIPSETTING(    0x08, "3" )
    PORT_DIPSETTING(    0x04, "4" )
    PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )			/* confirmed */
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, "Meter Pulses" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "5" )

	PORT_START	/* SENSE */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	3,		/* 3 bits per pixel */
	{ 0, 0x800*8, 0x1000*8 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( cvs )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,                  0, 256+3 )	/* chars */
  	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, s2636_gfx_layout, (256+3)*8, 8        )	/* s2636 #1  */
  	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, s2636_gfx_layout, (256+3)*8, 8        )	/* s2636 #2  */
  	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, s2636_gfx_layout, (256+3)*8, 8        )    /* s2636 #3  */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( cvs )

	/* basic machine hardware */
	MDRV_CPU_ADD(S2650, 894886.25)
	MDRV_CPU_PROGRAM_MAP(cvs_main_cpu_map,0)
	MDRV_CPU_IO_MAP(cvs_main_cpu_io_map,0)
	MDRV_CPU_VBLANK_INT(cvs_main_cpu_interrupt, 1)

	MDRV_CPU_ADD(S2650, 894886.25)
	MDRV_CPU_PROGRAM_MAP(cvs_audio_cpu_map,0)
	MDRV_CPU_IO_MAP(cvs_audio_cpu_io_map,0)

	MDRV_MACHINE_START(cvs)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(cvs)
	MDRV_VIDEO_UPDATE(cvs)

	MDRV_GFXDECODE(cvs)
	MDRV_PALETTE_LENGTH(16)
	MDRV_COLORTABLE_LENGTH((256+3)*8+(8*2))
	MDRV_PALETTE_INIT(cvs)

	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 30*8-1, 1*8, 32*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1000))

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(TMS5110, 640000)
	MDRV_SOUND_CONFIG(tms5100_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



#include "cvsdrvr.c"
