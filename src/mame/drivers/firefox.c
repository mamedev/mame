/***************************************************************************

    Atari Fire Fox hardware
    
    driver by smf, Aaron Giles, Chris Hardy & Scott Waye

short term:
	split driver/vidhrdw/sndhrdw/machine
	add option to centre joystick to enter test menu

it uses a quad pokey package 137323-1221-406???
the laser disc is a philips lvp 22vp931
( but maybe this works too... Special Drive: Laser Disc Player - Philips VP-832A )


AV# 60626
Atari "Firefox" V

Laser Disc - 30 minutes - Color - 1983

An interactive CAV laserdisc designed for use in the Atari video arcade game machine.
Contains over 100 visual and sound segments that include all of the branching possibilities of this game.
Each segment is two to five seconds long. This disc will play on any player,
but requires a special level III player for proper control. Video: CAV. Audio: Analog. 

*/

#include "mame.h"
#include "deprecat.h"
#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "sound/5220intf.h"
#include "machine/laserdsc.h"
#include "machine/6532riot.h"
#include "machine/x2212.h"

/*
fff6=firq e4a2 when dav goes active/low
fff8=irq e38f  This is through a flip-flop so goes off (high as active low) only when reset_irq is active - low.
fffa=??? e38d
fffc=??? e38d
fffe=reset e7cc
*/

/* 0x50-52 Used as a copy of the status 
   0x59 = 6-length of laser disc return code 
   0x53 = pointer to laser disc return
   ( LaserDiscBits & 0x80 ) != 0 when return code available
   DSKREAD = acknowledge
   ReadDiscData = return code
*/

/* FXXXXX for first field 
   AXXXXX for second field */

static int m_n_disc_lock;
static int m_n_disc_left_audio;
static int m_n_disc_right_audio;
static int m_n_disc_data;
static int command_offset;
static int command_data[ 3 ];
static int manchester_data[ 6 ];
static int manchester_offset;
static int disc_reset;
static int disk_opr = 0;
static int dav = 0x80;
static int dak_just_low = 0;  /* simulate the 15 uS time for the player to read the data */
static int dak = 0x40; /* DAK or DSKFULL active low indicates player has data, 
						  reset when player has read data */
static int disk_data; /* after a command is sent the first bit indicates an error, except of the data is 0x00 which indicates an invalid manchester data read (whatever that means) */

int laser_disc_speed = 0;
int laser_disc_field = 0;
void laser_seek_frame( int frame )
{
}


/* 20 = DISKOPR - Active low
   40 = DISKFULL - Active low
   80 = DISKDAV - Active low data available
   */
READ8_HANDLER( firefox_disc_status_r )
{
	int n_data;
	n_data = dav | dak | disk_opr; /* always operational */
	logerror( "%08x: disc status r %02x\n", activecpu_get_pc(), n_data & ( 0x80 | 0x40 | 0x20 ) );
	/*
	fprintf(stderr,  "%08x: reading disc status r %02x\n", activecpu_get_pc(), n_data & ( 0x80 | 0x40 | 0x20 ) );
	*/
	if(dak_just_low)
	{
		/* assume that the next status read will be after 15uS */
		dak = 0x40;
		dak_just_low = 0;
	}
	return n_data;
}

/* 4105 - DREAD */
/* this reset RDDSK (&DSKRD) */
READ8_HANDLER( firefox_disc_data_r )
{
	return disk_data;
}

/* DISK READ ENABLE */
/* 4218 - DSKREAD, set RDDSK */
WRITE8_HANDLER( firefox_disc_read_w )
{
	dav=0x80;
	if( manchester_offset < 6 )
	{
		disk_data = manchester_data[ manchester_offset++ ];
		if(manchester_offset < 6)
	    {
			dav = 0; /* more data */
			cpunum_set_input_line( machine, 0, M6809_FIRQ_LINE, ASSERT_LINE );
		}

	}
}

WRITE8_HANDLER( firefox_disc_lock_w )
{
	m_n_disc_lock = data & 0x80;
}

WRITE8_HANDLER( firefox_disc_right_audio_enable_w )
{
	m_n_disc_right_audio = data & 0x80;
}

WRITE8_HANDLER( firefox_disc_left_audio_enable_w )
{
	m_n_disc_left_audio = data & 0x80;
}

WRITE8_HANDLER( firefox_disc_reset_w )
{
	disc_reset = (data & 0x80);
	if(!disc_reset)
	{
		laser_disc_speed = 0;
		manchester_offset = 6; /* no data available */
		command_offset = 0;
		dak = 0x40;
		dav = 0x80;
	}
}

/* active low on dbb7 */
WRITE8_HANDLER( firefox_disc_write_w )
{
	if( ( data & 0x80 ) == 0 )
	{
		dak = 0; /* should go high after 15 uS */
		dak_just_low = 1;
		command_data[ command_offset++ ] = m_n_disc_data;
		if( command_offset == 3 )
		{
			command_offset = 0;
			switch( command_data[ 0 ] & 0xf0 )
			{
			case 0xf0:
				logerror( "CMD: goto Frame #%01x%02x%02x & play forward\n", command_data[ 0 ] & 0x0f, command_data[ 1 ], command_data[ 2 ] );
				laser_disc_field =
					( ( command_data[ 0 ] & 0x0f ) * 20000 ) +
					( (( command_data[ 1 ] & 0xf0 ) >> 4) * 2000 ) +
					( ( command_data[ 1 ] & 0x0f ) * 200 ) +
					( (( command_data[ 2 ] & 0xf0 ) >> 4) * 20 ) +
					( ( command_data[ 2 ] & 0x0f ) * 2 );
				laser_seek_frame(laser_disc_field >> 1);
				/*
				fprintf(stderr, "CMD: goto frame #%01x%02x%02x & play forward disc_field %d\n", command_data[ 0 ] & 0x0f, command_data[ 1 ], command_data[ 2 ] , laser_disc_field);
				*/
				laser_disc_speed = 1;
				return;
			case 0xd0:
				/*
				fprintf(stderr,  "CMD: goto Frame #%01x%02x%02x & halt (first field)\n", command_data[ 0 ] & 0x0f, command_data[ 1 ], command_data[ 2 ] );
				*/
				laser_disc_field =
					( ( command_data[ 0 ] & 0x0f ) * 20000 ) +
					( (( command_data[ 1 ] & 0xf0 ) >> 4) * 2000 ) +
					( ( command_data[ 1 ] & 0x0f ) * 200 ) +
					( (( command_data[ 2 ] & 0xf0 ) >> 4) * 20 ) +
					( ( command_data[ 2 ] & 0x0f ) * 2 );
				laser_seek_frame(laser_disc_field >> 1);
				laser_disc_speed = 0;
				return;
			case 0x00:
				switch( command_data[ 0 ] & 0x0f )
				{
				case 0x00:
					switch( command_data[ 1 ] & 0xf0 )
					{
					case 0x00:
						laser_disc_speed = 1;
						return;
					case 0x10:
						laser_disc_speed = -1;
						return;
					case 0x20:
						laser_disc_speed = 0;
						return;
					case 0x40:
						return;
					case 0x50:
						return;
					case 0xa0:
						laser_disc_speed = 75;
						return;
					case 0xb0:
						laser_disc_speed = -75;
						return;
					case 0xe0:
						return;
					case 0xf0:
						return;
					}
					break;
				case 0x02:
					switch( command_data[ 1 ] )
					{
					case 0xb0:
						logerror( "CMD: Video ON/OFF %02x\n", command_data[ 2 ] );
						return;
					case 0xb1:
						logerror( "CMD: Audio-I ON/OFF %02x\n", command_data[ 2 ] );
						return;
					case 0xb2:
						logerror( "CMD: Audio-II ON/OFF %02x\n", command_data[ 2 ] );
						return;
					case 0xb3:
						logerror( "CMD: CX ON/OFF %02x\n", command_data[ 2 ] );
						return;
					}
					break;
				}
				break;
			}
			logerror( "CMD: invalid %02x%02x%02x\n", command_data[ 0 ], command_data[ 1 ], command_data[ 2 ] );
		}
	}
}

/* latch the data */
WRITE8_HANDLER( firefox_disc_data_w )
{
	m_n_disc_data = data;
}

static TIMER_DEVICE_CALLBACK( laserdisk_timer_callback )
{
	if(param == 0 && laser_disc_speed != 0)
	{
		manchester_data[ 0 ] = ( ( laser_disc_field & 0x01 )?0xA0:0xF0 ) | ( ( laser_disc_field / 20000 ) % 10 );
		manchester_data[ 1 ] = ( ( ( laser_disc_field / 2000 ) % 10 ) << 4 ) | ( ( laser_disc_field / 200 ) % 10 );
		manchester_data[ 2 ] = ( ( ( laser_disc_field / 20 ) % 10 ) << 4 ) | ( ( laser_disc_field / 2 ) % 10 );
		manchester_data[ 3 ] = 0xff;
		manchester_data[ 4 ] = 0xff;
		manchester_data[ 5 ] = 0xff;
		manchester_offset = 0;
		cpunum_set_input_line( Machine, 0, M6809_FIRQ_LINE, ASSERT_LINE );
		dav = 0;  /* buffer contains info */

		laser_disc_field += laser_disc_speed;
	}
}



static unsigned char *tileram;
static size_t tileram_size;
static unsigned char *tile_palette;
static unsigned char *sprite_palette;
static const device_config *nvram_1c;
static const device_config *nvram_1d;

static int control_num;
static UINT8 sound_to_main_flag;
static UINT8 main_to_sound_flag;
static int sprite_bank;

/*************************************
 *
 *  Video
 *
 *************************************/

VIDEO_UPDATE( firefox )
{
	int x;
	int y;
	int sprite;
	const rectangle *visarea = video_screen_get_visible_area( Machine->primary_screen );

	for( y = 0; y < 64; y++ )
	{
		for( x = 0; x < 64; x++ )
		{
			drawgfx( bitmap, Machine->gfx[ 0 ], tileram[ x + ( y * 64 ) ], 0, 0, 0, x * 8, y * 8, visarea, TRANSPARENCY_NONE, 0 );
		}
	}

	for( sprite = 0; sprite < 32; sprite++ )
	{
		UINT8 *sprite_data = spriteram + ( 0x200 * sprite_bank ) + ( sprite * 16 );
		int flags = sprite_data[ 0 ];
		y = sprite_data[ 1 ] + ( 256 * ( ( flags >> 0 ) & 1 ) );
		x = sprite_data[ 2 ] + ( 256 * ( ( flags >> 1 ) & 1 ) );

		if( x != 0 )
		{
			int row;

			for( row = 0; row < 8; row++ )
			{
				int code = sprite_data[ 15 - row ] + ( 256 * ( ( flags >> 6 ) & 3 ) );
				int color = 2 * ( ( flags >> 2 ) & 0x03 );
				int flipx = flags & 0x20;
				int flipy = flags & 0x10;

				drawgfx( bitmap, Machine->gfx[ 1 ], code, color, flipx, flipy, x + 16, 500 - y - ( row * 16 ), visarea, TRANSPARENCY_PEN, 0 );
			}
		}
	}

	return 0;
}

static TIMER_DEVICE_CALLBACK( video_timer_callback )
{
	video_screen_update_now(timer->machine->primary_screen);

	cpunum_set_input_line( Machine, 0, M6809_IRQ_LINE, ASSERT_LINE );
}

static void set_rgba( int start, int index, unsigned char *palette_ram )
{
	int r = palette_ram[ index ];
	int g = palette_ram[ index + 256 ];
	int b = palette_ram[ index + 512 ];
	int a = ( b & 3 ) * 0x55;

	palette_set_color( Machine, start + index, MAKE_RGB( r, g, b ) );
	render_container_set_palette_alpha(render_container_get_screen(Machine->primary_screen), start + index, a);
}

static WRITE8_HANDLER( tile_palette_w )
{
	tile_palette[ offset ] = data;
	set_rgba( 0, offset & 0xff, tile_palette );
}

static WRITE8_HANDLER( sprite_palette_w )
{
	sprite_palette[ offset ] = data;
	set_rgba( 256, offset & 0xff, sprite_palette );
}

WRITE8_HANDLER( firefox_objram_bank_w )
{
	sprite_bank = data & 0x03;
}



/*************************************
 *
 *	Main <-> sound communication
 *
 *************************************/

static CUSTOM_INPUT( mainflag_r )
{
	return main_to_sound_flag;
}
	
static CUSTOM_INPUT( soundflag_r )
{
	return sound_to_main_flag;
}
	
static READ8_HANDLER( sound_to_main_r )
{
	sound_to_main_flag = 0;
	return soundlatch2_r(machine, 0);
}

static WRITE8_HANDLER( main_to_sound_w )
{
	main_to_sound_flag = 1;
	soundlatch_w(Machine, 0, data);
	cputag_set_input_line(Machine, "audio", INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( sound_reset_w )
{
	cputag_set_input_line(machine, "audio", INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
	if ((data & 0x80) != 0)
		sound_to_main_flag = main_to_sound_flag = 0;
}

static READ8_HANDLER( main_to_sound_r )
{
	main_to_sound_flag = 0;
	return soundlatch_r(Machine, 0);
}

static WRITE8_HANDLER( sound_to_main_w )
{
	sound_to_main_flag = 1;
	soundlatch2_w(Machine, 0, data);
}



/*************************************
 *
 *	6532 RIOT handlers
 *
 *************************************/

static UINT8 riot_porta_r(const device_config *device, UINT8 olddata)
{
	/* bit 7 = MAINFLAG */
	/* bit 6 = SOUNDFLAG */
	/* bit 5 = PA5 */
	/* bit 4 = TEST */
	/* bit 3 = n/c */
	/* bit 2 = TMS /ready */
	/* bit 1 = TMS /read */
	/* bit 0 = TMS /write */

	return (main_to_sound_flag << 7) | (sound_to_main_flag << 6) | 0x10 | (!tms5220_ready_r() << 2);
}


static void riot_porta_w(const device_config *device, UINT8 newdata, UINT8 olddata)
{
	/* handle 5220 read */
	if ((olddata & 2) != 0 && (newdata & 2) == 0)
		riot6532_portb_in_set(device, tms5220_status_r(device->machine, 0), 0xff);

	/* handle 5220 write */
	if ((olddata & 1) != 0 && (newdata & 1) == 0)
		tms5220_data_w(device->machine, 0, riot6532_portb_out_get(device));
}


static void riot_irq(const device_config *device, int state)
{
	cputag_set_input_line(device->machine, "audio", M6502_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  ADC input and control
 *
 *************************************/

static READ8_HANDLER( adc_r )
{
	if( control_num == 0 )
	{
		return input_port_read( Machine, "PITCH" );
	}

	return input_port_read( Machine, "YAW" );
}

static WRITE8_HANDLER( adc_select_w )
{
	control_num = offset;
}



/*************************************
 *
 *  Non-Volatile RAM (NOVRAM)
 *
 *************************************/

static WRITE8_HANDLER( nvram_w )
{
	x2212_write( nvram_1c, offset, data >> 4 );
	x2212_write( nvram_1d, offset, data & 0xf );
}

static READ8_HANDLER( nvram_r )
{
	return ( x2212_read( nvram_1c, offset ) << 4 ) | x2212_read( nvram_1d, offset );
}

static WRITE8_HANDLER( novram_recall_w )
{
	x2212_array_recall( nvram_1c, ( data >> 7 ) & 1 );
	x2212_array_recall( nvram_1d, ( data >> 7 ) & 1 );
}

static WRITE8_HANDLER( novram_store_w )
{
	x2212_store( nvram_1c, ( data >> 7 ) & 1 );
	x2212_store( nvram_1d, ( data >> 7 ) & 1 );
}



/*************************************
 *
 *  Main cpu
 *
 *************************************/

WRITE8_HANDLER( rom_bank_w )
{
	memory_set_bank(1, data & 0x1f);
}

static WRITE8_HANDLER( main_irq_clear_w )
{
    cpunum_set_input_line( machine, 0, M6809_IRQ_LINE, CLEAR_LINE );
}

static WRITE8_HANDLER( main_firq_clear_w )
{
    cpunum_set_input_line( machine, 0, M6809_FIRQ_LINE, CLEAR_LINE );
}

WRITE8_HANDLER( self_reset_w )
{
	cpunum_set_input_line( Machine, 0, INPUT_LINE_RESET, PULSE_LINE );
}



/*************************************
 *
 *  I/O
 *
 *************************************/

WRITE8_HANDLER( led_w )
{
	if( ( data & 0x80 ) != 0 )
	{
	    set_led_status( offset, 0 );
	}
	else
	{
	    set_led_status( offset, 1 );
	}
}

WRITE8_HANDLER( firefox_coin_counter_w )
{
	coin_counter_w( offset, data & 0x80 );
}



MACHINE_START( firefox )
{
	memory_configure_bank(1, 0, 32, memory_region(machine, "main") + 0x10000, 0x1000);
	nvram_1c = device_list_find_by_tag(machine->config->devicelist, X2212, "nvram_1c");
	nvram_1d = device_list_find_by_tag(machine->config->devicelist, X2212, "nvram_1d");

	control_num = 0;
	sprite_bank = 0;

	command_data[ 0 ] = 0;
	command_data[ 1 ] = 0;
	command_data[ 2 ] = 0;
	command_offset = 0;

	laser_disc_field = 0;
	laser_disc_speed = 0;
	manchester_offset = 6;
}


/*************************************
 *
 *	Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_RAM AM_BASE(&tileram) AM_SIZE(&tileram_size)
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x2800, 0x2aff) AM_READWRITE(SMH_RAM, sprite_palette_w) AM_BASE(&sprite_palette)
	AM_RANGE(0x2b00, 0x2b00) AM_WRITE(firefox_objram_bank_w)
	AM_RANGE(0x2b01, 0x2bff) AM_RAM
	AM_RANGE(0x2c00, 0x2eff) AM_READWRITE(SMH_RAM, tile_palette_w) AM_BASE(&tile_palette)
	AM_RANGE(0x2f00, 0x2fff) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK(1)
	AM_RANGE(0x4000, 0x40ff) AM_READWRITE(nvram_r, nvram_w)
	AM_RANGE(0x4100, 0x4100) AM_READ_PORT("IN0")
	AM_RANGE(0x4101, 0x4101) AM_READ_PORT("IN1")
	AM_RANGE(0x4102, 0x4102) AM_READ(firefox_disc_status_r)
	AM_RANGE(0x4103, 0x4103) AM_READ_PORT("IN3")
	AM_RANGE(0x4104, 0x4104) AM_READ_PORT("IN4")
	AM_RANGE(0x4105, 0x4105) AM_READ(firefox_disc_data_r)
	AM_RANGE(0x4106, 0x4106) AM_READ(sound_to_main_r)
	AM_RANGE(0x4107, 0x4107) AM_READ(adc_r)
	AM_RANGE(0x4200, 0x4200) AM_WRITE(main_irq_clear_w)
	AM_RANGE(0x4208, 0x4208) AM_WRITE(main_firq_clear_w)
	AM_RANGE(0x4210, 0x4210) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x4218, 0x4218) AM_WRITE(firefox_disc_read_w)
	AM_RANGE(0x4220, 0x4221) AM_WRITE(adc_select_w)
	AM_RANGE(0x4230, 0x4230) AM_WRITE(self_reset_w)
	AM_RANGE(0x4280, 0x4280) AM_WRITE(novram_recall_w)
	AM_RANGE(0x4281, 0x4281) AM_WRITE(sound_reset_w)
	AM_RANGE(0x4282, 0x4282) AM_WRITE(novram_store_w)
	AM_RANGE(0x4283, 0x4283) AM_WRITE(firefox_disc_lock_w)
	AM_RANGE(0x4284, 0x4284) AM_WRITE(firefox_disc_right_audio_enable_w)
	AM_RANGE(0x4285, 0x4285) AM_WRITE(firefox_disc_left_audio_enable_w)
	AM_RANGE(0x4286, 0x4286) AM_WRITE(firefox_disc_reset_w)
	AM_RANGE(0x4287, 0x4287) AM_WRITE(firefox_disc_write_w)
	AM_RANGE(0x4288, 0x4289) AM_WRITE(firefox_coin_counter_w)
	AM_RANGE(0x428c, 0x428f) AM_WRITE(led_w)
	AM_RANGE(0x4290, 0x4290) AM_WRITE(rom_bank_w)
	AM_RANGE(0x4298, 0x4298) AM_WRITE(main_to_sound_w)
	AM_RANGE(0x42a0, 0x42a7) AM_WRITE(firefox_disc_data_w)
	AM_RANGE(0x4400, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *	Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START(audio_map, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0700) AM_RAM /* RIOT ram */
	AM_RANGE(0x0880, 0x089f) AM_MIRROR(0x07e0) AM_DEVREADWRITE(RIOT6532,"riot",riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1000) AM_READ(main_to_sound_r)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_to_main_w)
	AM_RANGE(0x2000, 0x200f) AM_READWRITE(pokey1_r, pokey1_w)
	AM_RANGE(0x2800, 0x280f) AM_READWRITE(pokey2_r, pokey2_w)
	AM_RANGE(0x3000, 0x300f) AM_READWRITE(pokey3_r, pokey3_w)
	AM_RANGE(0x3800, 0x380f) AM_READWRITE(pokey4_r, pokey4_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *	Port definitions
 *
 *************************************/

INPUT_PORTS_START( firefox )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mainflag_r, NULL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(soundflag_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN3")
	PORT_DIPNAME(    0x03, 0x00, "Coins Per Credit" )
	PORT_DIPSETTING( 0x00, "1 Coin 1 Credit" )
	PORT_DIPSETTING( 0x01, "2 Coins 1 Credit" )
	PORT_DIPSETTING( 0x02, "3 Coins 1 Credit" )
	PORT_DIPSETTING( 0x03, "4 Coins 1 Credit" )
	PORT_DIPNAME(    0x0c, 0x00, "Right Coin" )
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x04, "1 Coin for 4 Coin Units" )
	PORT_DIPSETTING( 0x08, "1 Coin for 5 Coin Units" )
	PORT_DIPSETTING( 0x0c, "1 Coin for 6 Coin Units" )
	PORT_DIPNAME(    0x10, 0x00, "Left Coin" )
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x10, "1 Coin for 2 Coin Units" )
	PORT_DIPNAME(    0xe0, 0x00, "Bonus Adder" )
	PORT_DIPSETTING( 0x00, DEF_STR( None ) )
	PORT_DIPSETTING( 0x20, "1 Credit for 2 Coin Units" )
	PORT_DIPSETTING( 0xa0, "1 Credit for 3 Coin Units" )
	PORT_DIPSETTING( 0x40, "1 Credit for 4 Coin Units" )
	PORT_DIPSETTING( 0x80, "1 Credit for 5 Coin Units" )
	PORT_DIPSETTING( 0x60, "2 Credits for 4 Coin Units" )
	PORT_DIPSETTING( 0xe0, DEF_STR( Free_Play ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x00, "Missions" )
	PORT_DIPSETTING(    0x00, "All .50" )
	PORT_DIPSETTING(    0x01, ".50 .75" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, "Moderate" )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x00, "Gas Usage" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, "Moderate" )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x00, "Bonus Gas" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, "Pro Limit" )
	PORT_DIPSETTING(    0x00, "Moderate" )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )

	PORT_START("PITCH")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("YAW")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END



/*************************************
 *
 *	Graphics definitions
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( firefox )
	GFXDECODE_ENTRY("tiles",   0, tilelayout, 0, 16)
	GFXDECODE_ENTRY("sprites", 0, spritelayout, 256, 32)
GFXDECODE_END



/*************************************
 *
 *	Machine driver
 *
 *************************************/

static const riot6532_interface riot_intf =
{
	riot_porta_r,
	NULL,
	riot_porta_w,
	NULL,
	riot_irq
};


static MACHINE_DRIVER_START( firefox )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M6809, 4000000)
	MDRV_CPU_PROGRAM_MAP(main_map, 0)
	MDRV_TIMER_ADD_SCANLINE("32v", video_timer_callback, "main", 96, 128)

	MDRV_CPU_ADD("audio", M6502, 2000000)
	MDRV_CPU_PROGRAM_MAP(audio_map, 0)

	MDRV_MACHINE_START(firefox)

	/* video hardware */
	MDRV_LASERDISC_SCREEN_ADD_NTSC("main", BITMAP_FORMAT_INDEXED16)

	MDRV_GFXDECODE(firefox)
	MDRV_PALETTE_LENGTH(512)

	MDRV_LASERDISC_ADD("laserdisc", PIONEER_PR8210, "main", "ldsound")
	MDRV_LASERDISC_OVERLAY(firefox, 64*8, 64*8, BITMAP_FORMAT_INDEXED16)
	MDRV_LASERDISC_OVERLAY_CLIP(7*8, 53*8-1, 1*8, 62*8-1)
	MDRV_TIMER_ADD_SCANLINE("laserdisk", laserdisk_timer_callback, "main", 0, 0)

	MDRV_DEVICE_ADD("nvram_1c",X2212)
	MDRV_DEVICE_ADD("nvram_1d",X2212)
	MDRV_RIOT6532_ADD("riot", 1500000, riot_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("pokey1", POKEY, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.20)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.20)

	MDRV_SOUND_ADD("pokey2", POKEY, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.20)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.20)

	MDRV_SOUND_ADD("pokey3", POKEY, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.20)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.20)

	MDRV_SOUND_ADD("pokey4", POKEY, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.20)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.20)

	MDRV_SOUND_ADD("tms", TMS5220, 640000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD("ldsound", CUSTOM, 0)
	MDRV_SOUND_CONFIG(laserdisc_custom_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

MACHINE_DRIVER_END



/*************************************
 *
 *	ROM definitions
 *
 *************************************/

ROM_START( firefox )
	ROM_REGION( 0x30000, "main", 0 ) /* 64k for code + data & 128k for banked roms */
	ROM_LOAD( "136026.209",     0x04000, 0x4000, CRC(9f559f1b) SHA1(142d14cb5158ea77f6fc6d9bf0ce723842f345e2) ) /* 8b/c */
	ROM_LOAD( "136026.210",     0x08000, 0x4000, CRC(d769b40d) SHA1(2d354649a381f3399cb0161267bd1c36a8f2bb4b) ) /* 7b/c */
	ROM_LOAD( "136026.211",     0x0c000, 0x4000, CRC(7293ab03) SHA1(73d0d173da295ad59e431bab0a9814a71146cbc2) ) /* 6b/c */
	ROM_LOAD( "136026.201",     0x10000, 0x4000, CRC(c118547a) SHA1(4d3502cbde3116588ed944bf1750bab50e4c813c) ) /* 8a */
	/* empty 7a */
	/* empty 6a */
	/* empty 5a */
	ROM_LOAD( "136026.205",     0x20000, 0x4000, CRC(dc21677f) SHA1(576a96c1e07e1362a0a367e76dc369ee8a950144) ) /* 4a */
	ROM_LOAD( "136026.127",     0x24000, 0x2000, CRC(c0c765ab) SHA1(79f6c8c1d00684d7143b2d33a5669bdf5cd01e96) ) /* 3a */
	ROM_RELOAD( 0x26000, 0x2000 )
	/* empty 2a */
	/* empty 1a */

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for code */
	/* empty 4k/l */
	ROM_LOAD( "136026.128",     0x08000, 0x2000, CRC(5358d870) SHA1(e8f2983a7e612e1a050a3c0b9f19b1077de4c146) ) /* 4m */
	ROM_RELOAD( 0x0a000, 0x2000 )
	ROM_LOAD( "136026.214",     0x0c000, 0x4000, CRC(92378b78) SHA1(62c7a1fee675fa3f9125f8e208b8207f0ce28bbe) ) /* 4n */

	ROM_REGION( 0x2000, "tiles", ROMREGION_DISPOSE )
	ROM_LOAD( "136026.125",     0x0000,  0x2000, CRC(8a32f9f1) SHA1(f899174f55cd4a24a3be4a0f4bb44d3e8e938586) ) /* 6p */

	ROM_REGION( 0x28000, "sprites", ROMREGION_DISPOSE )
	/* empty 6c */
	/* empty 6a */
	ROM_LOAD( "136026.124",     0x00000,  0x4000, CRC(5efe0f6c) SHA1(df35fd9267d966ab379c2f78ed418f4606741b28)) /* 5c */
	ROM_LOAD( "136026.123",     0x04000,  0x4000, CRC(dffe48b3) SHA1(559907651bb425e26a834b467959b15092d23d27)) /* 5a */
	ROM_LOAD( "136026.118",     0x08000,  0x4000, CRC(0ed4df15) SHA1(7aa599f428112fff4bfedf63fafc22f19fa66546)) /* 4c */
	ROM_LOAD( "136026.122",     0x0c000,  0x4000, CRC(8e2c6616) SHA1(59cbd585028bb634034a9dfd552275bd41f01989)) /* 4a */
	ROM_LOAD( "136026.117",     0x10000,  0x4000, CRC(79129084) SHA1(4219ff7cd444ad11e4cb9f1c30ac15fe0cfc5a17)) /* 3c */
	ROM_LOAD( "136026.121",     0x14000,  0x4000, CRC(494972d4) SHA1(fa0e24e911b233e9644d7794ba03f76bfd39aa8c)) /* 3a */
	ROM_LOAD( "136026.116",     0x18000,  0x4000, CRC(d5282d4e) SHA1(de5fdf82a615625aa77b39e035b4206216faaf9c)) /* 2c */
	ROM_LOAD( "136026.120",     0x1c000,  0x4000, CRC(e1b95923) SHA1(b6d0c0af0a8f55e728cd0f4c3222745eefd57f50)) /* 2a */
	ROM_LOAD( "136026.115",     0x20000,  0x4000, CRC(861abc82) SHA1(1845888d07162ae915364a2a91294731f1c5b3bd)) /* 1c */
	ROM_LOAD( "136026.119",     0x24000,  0x4000, CRC(959471b1) SHA1(a032209a209f51d34360d5c7ad32ec62150158d2)) /* 1a */
ROM_END

ROM_START( firefoxa )
	ROM_REGION( 0x30000, "main", 0 ) /* 64k for code + data & 128k for banked roms */
	ROM_LOAD( "136026.109",     0x04000, 0x4000, CRC(7639270c) SHA1(1b8f53c516d26aecb4478ac99783a37e5b1a107f)) /* 8b/c */
	ROM_LOAD( "136026.110",     0x08000, 0x4000, CRC(f3102944) SHA1(460f18180b19b6360c99c7e70f86d745f69ba95d)) /* 7b/c */
	ROM_LOAD( "136026.111",     0x0c000, 0x4000, CRC(8a230bb5) SHA1(0cfa1e981e4a8ccaf5903b4e761a2085b5a56181)) /* 6b/c */
	ROM_LOAD( "136026.101",     0x10000, 0x4000, CRC(91bba45a) SHA1(d584a8f60bbbdbe250978b7aeb3f5e7698f94d60)) /* 8a */
	ROM_LOAD( "136026.102",     0x14000, 0x4000, CRC(5f1e423d) SHA1(c55c27600877272c1ca94eab75c1eb25ff84d36f)) /* 7a */
	/* empty 6a */
	/* empty 5a */
	ROM_LOAD( "136026.105",     0x20000, 0x4000, CRC(83f1d4ed) SHA1(ed4b22b3473f16cbcca1415f6d81be558ab10ff3)) /* 4a */
	ROM_LOAD( "136026.106",     0x24000, 0x4000, CRC(c5d8d417) SHA1(6a29595b2c091bbcf413c7213c6577eaf9c507d1)) /* 3a */
	/* empty 2a */
	/* empty 1a */

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for code */
	/* empty 4k/l */
	ROM_LOAD( "136026.113",     0x08000, 0x4000, CRC(90988b3b) SHA1(7571cf6b7e9e3e22f930d9ba991b730e734edfb7)) /* 4m */
	ROM_LOAD( "136026.114",     0x0c000, 0x4000, CRC(1437ce14) SHA1(eef14172b3935a4afb3470852f93d30926b139e4)) /* 4n */

	ROM_REGION( 0x2000, "tiles", ROMREGION_DISPOSE )
	ROM_LOAD( "136026.125",     0x0000,  0x2000, CRC(8a32f9f1) SHA1(f899174f55cd4a24a3be4a0f4bb44d3e8e938586) ) /* 6p */

	ROM_REGION( 0x28000, "sprites", ROMREGION_DISPOSE )
	/* empty 6c */
	/* empty 6a */
	ROM_LOAD( "136026.124",     0x00000,  0x4000, CRC(5efe0f6c) SHA1(df35fd9267d966ab379c2f78ed418f4606741b28)) /* 5c */
	ROM_LOAD( "136026.123",     0x04000,  0x4000, CRC(dffe48b3) SHA1(559907651bb425e26a834b467959b15092d23d27)) /* 5a */
	ROM_LOAD( "136026.118",     0x08000,  0x4000, CRC(0ed4df15) SHA1(7aa599f428112fff4bfedf63fafc22f19fa66546)) /* 4c */
	ROM_LOAD( "136026.122",     0x0c000,  0x4000, CRC(8e2c6616) SHA1(59cbd585028bb634034a9dfd552275bd41f01989)) /* 4a */
	ROM_LOAD( "136026.117",     0x10000,  0x4000, CRC(79129084) SHA1(4219ff7cd444ad11e4cb9f1c30ac15fe0cfc5a17)) /* 3c */
	ROM_LOAD( "136026.121",     0x14000,  0x4000, CRC(494972d4) SHA1(fa0e24e911b233e9644d7794ba03f76bfd39aa8c)) /* 3a */
	ROM_LOAD( "136026.116",     0x18000,  0x4000, CRC(d5282d4e) SHA1(de5fdf82a615625aa77b39e035b4206216faaf9c)) /* 2c */
	ROM_LOAD( "136026.120",     0x1c000,  0x4000, CRC(e1b95923) SHA1(b6d0c0af0a8f55e728cd0f4c3222745eefd57f50)) /* 2a */
	ROM_LOAD( "136026.115",     0x20000,  0x4000, CRC(861abc82) SHA1(1845888d07162ae915364a2a91294731f1c5b3bd)) /* 1c */
	ROM_LOAD( "136026.119",     0x24000,  0x4000, CRC(959471b1) SHA1(a032209a209f51d34360d5c7ad32ec62150158d2)) /* 1a */
ROM_END



/*************************************
 *
 *	Game drivers
 *
 *************************************/

GAME( 1984, firefox,  0,       firefox, firefox, 0, ROT0, "Atari", "Fire Fox (set 1)", 0 )
GAME( 1984, firefoxa, firefox, firefox, firefox, 0, ROT0, "Atari", "Fire Fox (set 2)", 0 )
