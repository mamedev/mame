/*************************************************************************

    ldplayer.c

    Laserdisc player driver.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************/

#include "driver.h"
#include "uimenu.h"
#include "machine/laserdsc.h"
#include <ctype.h>


/*************************************
 *
 *  Constants
 *
 *************************************/

enum
{
	CMD_SCAN_REVERSE,
	CMD_SCAN_REVERSE_END,
	CMD_STEP_REVERSE,
	CMD_SCAN_FORWARD,
	CMD_SCAN_FORWARD_END,
	CMD_STEP_FORWARD,
	CMD_PLAY,
	CMD_PAUSE,
	CMD_DISPLAY_ON,
	CMD_DISPLAY_OFF,
	CMD_0,
	CMD_1,
	CMD_2,
	CMD_3,
	CMD_4,
	CMD_5,
	CMD_6,
	CMD_7,
	CMD_8,
	CMD_9,
	CMD_SEARCH
};
	


/*************************************
 *
 *  Globals
 *
 *************************************/

static astring *filename;

static input_port_value last_controls;
static UINT8 playing;
static UINT8 displaying;

static UINT8 pr8210_last_was_number;
static emu_timer *pr8210_bit_timer;
static UINT32 pr8210_command_buffer_in, pr8210_command_buffer_out;
static UINT8 pr8210_command_buffer[10];

static void (*execute_command)(const device_config *laserdisc, int command);



/*************************************
 *
 *  Timers and sync
 *
 *************************************/

static void process_commands(const device_config *laserdisc)
{
	input_port_value controls = input_port_read(laserdisc->machine, "controls");
 	int number;

	/* scan/step backwards */
	if (!(last_controls & 0x01) && (controls & 0x01))
	{
		if (playing)
			(*execute_command)(laserdisc, CMD_SCAN_REVERSE);
		else
			(*execute_command)(laserdisc, CMD_STEP_REVERSE);
	}
	else if ((last_controls & 0x01) && !(controls & 0x01))
	{
		if (playing)
			(*execute_command)(laserdisc, CMD_SCAN_REVERSE_END);
	}

	/* scan/step forwards */
	if (!(last_controls & 0x02) && (controls & 0x02))
	{
		if (playing)
			(*execute_command)(laserdisc, CMD_SCAN_FORWARD);
		else
			(*execute_command)(laserdisc, CMD_STEP_FORWARD);
	}
	else if ((last_controls & 0x02) && !(controls & 0x02))
	{
		if (playing)
			(*execute_command)(laserdisc, CMD_SCAN_FORWARD_END);
	}

	/* play/pause */
	if (!(last_controls & 0x10) && (controls & 0x10))
	{
		playing = !playing;
		(*execute_command)(laserdisc, playing ? CMD_PLAY : CMD_PAUSE);
	}

	/* toggle display */
	if (!(last_controls & 0x20) && (controls & 0x20))
	{
		displaying = !displaying;
		(*execute_command)(laserdisc, displaying ? CMD_DISPLAY_ON : CMD_DISPLAY_OFF);
	}

	/* numbers */
	for (number = 0; number < 10; number++)
		if (!(last_controls & (0x100 << number)) && (controls & (0x100 << number)))
			(*execute_command)(laserdisc, CMD_0 + number);

	/* enter */
	if (!(last_controls & 0x40000) && (controls & 0x40000))
		(*execute_command)(laserdisc, CMD_SEARCH);

	last_controls = controls;
}


static TIMER_CALLBACK( vsync_update )
{
	const device_config *laserdisc = device_list_first(machine->config->devicelist, LASERDISC);
	int vblank_scanline;
	attotime target;

	/* handle commands */
	if (!param)
		process_commands(laserdisc);

	/* update the laserdisc */
	laserdisc_vsync(laserdisc);

	/* set a timer to go off on the next VBLANK */
	vblank_scanline = video_screen_get_visible_area(machine->primary_screen)->max_y + 1;
	target = video_screen_get_time_until_pos(machine->primary_screen, vblank_scanline, 0);
	timer_set(target, NULL, 0, vsync_update);
}


static MACHINE_START( ldplayer )
{
	vsync_update(machine, NULL, 1);
}


static TIMER_CALLBACK( autoplay )
{
	const device_config *laserdisc = device_list_first(machine->config->devicelist, LASERDISC);

	/* start playing */
	(*execute_command)(laserdisc, CMD_PLAY);
	playing = TRUE;
	displaying = FALSE;
}


static MACHINE_RESET( ldplayer )
{
	/* set up a timer to start playing immediately */
	timer_set(attotime_zero, NULL, 0, autoplay);

	/* indicate the name of the file we opened */
	popmessage("Opened %s\n", astring_c(filename));
}



/*************************************
 *
 *  PR-8210 implementation
 *
 *************************************/

INLINE void pr8210_add_command(UINT8 command)
{
	pr8210_command_buffer[pr8210_command_buffer_in++ % ARRAY_LENGTH(pr8210_command_buffer)] = (command & 0x1f) | 0x20;
	pr8210_command_buffer[pr8210_command_buffer_in++ % ARRAY_LENGTH(pr8210_command_buffer)] = (command & 0x1f) | 0x20;
	pr8210_command_buffer[pr8210_command_buffer_in++ % ARRAY_LENGTH(pr8210_command_buffer)] = 0x00 | 0x20;
}


static TIMER_CALLBACK( pr8210_bit_off_callback )
{
	const device_config *laserdisc = ptr;
	
	/* deassert the control line */
	laserdisc_line_w(laserdisc, LASERDISC_LINE_CONTROL, CLEAR_LINE);
}


static TIMER_CALLBACK( pr8210_bit_callback )
{
	attotime duration = ATTOTIME_IN_MSEC(30);
	const device_config *laserdisc = ptr;
	UINT8 bitsleft = param >> 16;
	UINT8 data = param;
	
	/* if we have bits, process */
	if (bitsleft != 0)
	{
		/* assert the line and set a timer for deassertion */
	   	laserdisc_line_w(laserdisc, LASERDISC_LINE_CONTROL, ASSERT_LINE);
		timer_set(ATTOTIME_IN_USEC(250), ptr, 0, pr8210_bit_off_callback);

		/* space 0 bits apart by 1msec, and 1 bits by 2msec */
		duration = attotime_mul(ATTOTIME_IN_MSEC(1), (data & 0x80) ? 2 : 1);
		data <<= 1;
		bitsleft--;
	}

	/* if we're out of bits, queue up the next command */
	else if (bitsleft == 0 && pr8210_command_buffer_in != pr8210_command_buffer_out)
	{
		data = pr8210_command_buffer[pr8210_command_buffer_out++ % ARRAY_LENGTH(pr8210_command_buffer)];
		bitsleft = 12;
	}
	timer_adjust_oneshot(pr8210_bit_timer, duration, (bitsleft << 16) | data);
}


static MACHINE_START( pr8210 )
{
	const device_config *laserdisc = device_list_first(machine->config->devicelist, LASERDISC);
	MACHINE_START_CALL(ldplayer);
	pr8210_bit_timer = timer_alloc(pr8210_bit_callback, (void *)laserdisc);
}


static MACHINE_RESET( pr8210 )
{
	MACHINE_RESET_CALL(ldplayer);
	timer_adjust_oneshot(pr8210_bit_timer, attotime_zero, 0);
}


static void pr8210_execute(const device_config *laserdisc, int command)
{
	static const UINT8 digits[10] = { 0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d, 0x03, 0x13 };
	int prev_was_number = pr8210_last_was_number;
	
	pr8210_last_was_number = FALSE;
	switch (command)
	{
		case CMD_SCAN_REVERSE:
			pr8210_add_command(0x1c);
			playing = TRUE;
			break;
		
		case CMD_SCAN_REVERSE_END:
			pr8210_add_command(0x14);
			playing = TRUE;
			break;
		
		case CMD_STEP_REVERSE:
			pr8210_add_command(0x12);
			playing = FALSE;
			break;

		case CMD_SCAN_FORWARD:
			pr8210_add_command(0x08);
			playing = TRUE;
			break;

		case CMD_SCAN_FORWARD_END:
			pr8210_add_command(0x14);
			playing = TRUE;
			break;
		
		case CMD_STEP_FORWARD:
			pr8210_add_command(0x04);
			playing = FALSE;
			break;
		
		case CMD_PLAY:
			pr8210_add_command(0x14);
			playing = TRUE;
			break;
		
		case CMD_PAUSE:
			pr8210_add_command(0x0a);
			playing = FALSE;
			break;

		case CMD_DISPLAY_ON:
//			pr8210_add_command(digits[1]);
//			pr8210_add_command(0xf1);
			break;

		case CMD_DISPLAY_OFF:
//			pr8210_add_command(digits[0]);
//			pr8210_add_command(0xf1);
			break;
		
		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			if (!prev_was_number)
				pr8210_add_command(0x1a);
			pr8210_add_command(digits[command - CMD_0]);
			pr8210_last_was_number = TRUE;
			break;
		
		case CMD_SEARCH:
			pr8210_add_command(0x1a);
			playing = FALSE;
			break;
	}
}



/*************************************
 *
 *  LD-V1000 implementation
 *
 *************************************/

static void ldv1000_execute(const device_config *laserdisc, int command)
{
	static const UINT8 digits[10] = { 0x3f, 0x0f, 0x8f, 0x4f, 0x2f, 0xaf, 0x6f, 0x1f, 0x9f, 0x5f };
	switch (command)
	{
		case CMD_SCAN_REVERSE:
			laserdisc_data_w(laserdisc, 0xf8);
			playing = TRUE;
			break;
		
		case CMD_SCAN_REVERSE_END:
			laserdisc_data_w(laserdisc, 0xfd);
			playing = TRUE;
			break;
		
		case CMD_STEP_REVERSE:
			laserdisc_data_w(laserdisc, 0xfe);
			playing = FALSE;
			break;

		case CMD_SCAN_FORWARD:
			laserdisc_data_w(laserdisc, 0xf0);
			playing = TRUE;
			break;

		case CMD_SCAN_FORWARD_END:
			laserdisc_data_w(laserdisc, 0xfd);
			playing = TRUE;
			break;
		
		case CMD_STEP_FORWARD:
			laserdisc_data_w(laserdisc, 0xf6);
			playing = FALSE;
			break;
		
		case CMD_PLAY:
			laserdisc_data_w(laserdisc, 0xfd);
			playing = TRUE;
			break;
		
		case CMD_PAUSE:
			laserdisc_data_w(laserdisc, 0xa0);
			playing = FALSE;
			break;

		case CMD_DISPLAY_ON:
			laserdisc_data_w(laserdisc, digits[1]);
			laserdisc_data_w(laserdisc, 0xf1);
			break;

		case CMD_DISPLAY_OFF:
			laserdisc_data_w(laserdisc, digits[0]);
			laserdisc_data_w(laserdisc, 0xf1);
			break;
		
		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			laserdisc_data_w(laserdisc, digits[command - CMD_0]);
			break;
		
		case CMD_SEARCH:
			laserdisc_data_w(laserdisc, 0xf7);
			playing = FALSE;
			break;
	}
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( ldplayer )
	PORT_START("controls")
	PORT_BIT( 0x00001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x00002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x00004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x00008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x00010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x00020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x00100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x00200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x00400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x00800, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x01000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x02000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x04000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x08000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x10000, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20000, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x40000, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( ldplayer_core )

	MDRV_MACHINE_START(ldplayer)
	MDRV_MACHINE_RESET(ldplayer)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("laserdisc", CUSTOM, 0)
	MDRV_SOUND_CONFIG(laserdisc_custom_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ldplayer_ntsc )
	MDRV_IMPORT_FROM(ldplayer_core)
	MDRV_LASERDISC_SCREEN_ADD_NTSC("main", BITMAP_FORMAT_RGB32)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ldv1000 )
	MDRV_IMPORT_FROM(ldplayer_ntsc)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_LDV1000)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pr8210 )
	MDRV_IMPORT_FROM(ldplayer_ntsc)
	MDRV_MACHINE_START(pr8210)
	MDRV_MACHINE_RESET(pr8210)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_PR8210)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ldv1000 )
	DISK_REGION( "laserdisc" )
ROM_END


ROM_START( pr8210 )
	DISK_REGION( "laserdisc" )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static void free_string(running_machine *machine)
{
	astring_free(filename);
}


static DRIVER_INIT( ldplayer )
{
	mame_file *image_file = NULL;
	chd_file *image_chd = NULL;
	mame_path *path;

	/* open a path to the ROMs and find the first CHD file */
	path = mame_openpath(mame_options(), OPTION_ROMPATH);
	if (path != NULL)
	{
		const osd_directory_entry *dir;

		/* iterate while we get new objects */
		while ((dir = mame_readpath(path)) != NULL)
		{
			int length = strlen(dir->name);

			/* look for files ending in .chd */
			if (length > 4 &&
				tolower(dir->name[length - 4] == '.') &&
				tolower(dir->name[length - 3] == 'c') &&
				tolower(dir->name[length - 2] == 'h') &&
				tolower(dir->name[length - 1] == 'd'))
			{
				file_error filerr;
				chd_error chderr;

				/* open the file itself via our search path */
				filerr = mame_fopen(SEARCHPATH_IMAGE, dir->name, OPEN_FLAG_READ, &image_file);
				if (filerr == FILERR_NONE)
				{
					/* try to open the CHD */
					chderr = chd_open_file(mame_core_file(image_file), CHD_OPEN_READ, NULL, &image_chd);
					if (chderr == CHDERR_NONE)
					{
						set_disk_handle("laserdisc", image_file, image_chd);
						filename = astring_dupc(dir->name);
						add_exit_callback(machine, free_string);
						break;
					}

					/* close the file on failure */
					mame_fclose(image_file);
					image_file = NULL;
				}
			}
		}
		mame_closepath(path);
	}

	/* if we failed, pop a message and exit */
	if (image_file == NULL)
		fatalerror("No valid image file found!\n");
}



static DRIVER_INIT( ldv1000 ) { execute_command = ldv1000_execute; DRIVER_INIT_CALL(ldplayer); }
static DRIVER_INIT( pr8210 )  { execute_command = pr8210_execute; DRIVER_INIT_CALL(ldplayer); }



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2008, ldv1000, 0, ldv1000, ldplayer, ldv1000, ROT0, "MAME", "Pioneer LDV-1000 Simulator", 0 )
GAME( 2008, pr8210,  0, pr8210,  ldplayer, pr8210,  ROT0, "MAME", "Pioneer PR-8210 Simulator", 0 )
