/*************************************************************************

    ldplayer.c

    Laserdisc player driver.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "uimenu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/laserdsc.h"
#include <ctype.h>

#include "pr8210.lh"



/*************************************
 *
 *  Constants
 *
 *************************************/

enum
{
	CMD_SCAN_REVERSE,
	CMD_STEP_REVERSE,
	CMD_SLOW_REVERSE,
	CMD_FAST_REVERSE,
	CMD_SCAN_FORWARD,
	CMD_STEP_FORWARD,
	CMD_SLOW_FORWARD,
	CMD_FAST_FORWARD,
	CMD_PLAY,
	CMD_PAUSE,
	CMD_FRAME_TOGGLE,
	CMD_CHAPTER_TOGGLE,
	CMD_CH1_TOGGLE,
	CMD_CH2_TOGGLE,
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

static astring filename;

static input_port_value last_controls;
static UINT8 playing;

static emu_timer *pr8210_bit_timer;
static UINT32 pr8210_command_buffer_in, pr8210_command_buffer_out;
static UINT8 pr8210_command_buffer[10];

static void (*execute_command)(running_device *laserdisc, int command);



/*************************************
 *
 *  Disc location
 *
 *************************************/

static void free_string(running_machine &machine)
{
}


static chd_file *get_disc(running_device *device)
{
	mame_file *image_file = NULL;
	chd_file *image_chd = NULL;
	mame_path *path;

	/* open a path to the ROMs and find the first CHD file */
	path = mame_openpath(device->machine->options(), OPTION_ROMPATH);
	if (path != NULL)
	{
		const osd_directory_entry *dir;

		/* iterate while we get new objects */
		while ((dir = mame_readpath(path)) != NULL)
		{
			int length = strlen(dir->name);

			/* look for files ending in .chd */
			if (length > 4 &&
				dir->name[length - 4] == '.' &&
				tolower(dir->name[length - 3]) == 'c' &&
				tolower(dir->name[length - 2]) == 'h' &&
				tolower(dir->name[length - 1]) == 'd')
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
						set_disk_handle(device->machine, "laserdisc", image_file, image_chd);
						filename.cpy(dir->name);
						device->machine->add_notifier(MACHINE_NOTIFY_EXIT, free_string);
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

	return get_disk_handle(device->machine, "laserdisc");
}



/*************************************
 *
 *  Timers and sync
 *
 *************************************/

static void process_commands(running_device *laserdisc)
{
	input_port_value controls = input_port_read(laserdisc->machine, "controls");
	int number;

	/* step backwards */
	if (!(last_controls & 0x01) && (controls & 0x01))
		(*execute_command)(laserdisc, CMD_STEP_REVERSE);

	/* step forwards */
	if (!(last_controls & 0x02) && (controls & 0x02))
		(*execute_command)(laserdisc, CMD_STEP_FORWARD);

	/* scan backwards */
	if (controls & 0x04)
		(*execute_command)(laserdisc, CMD_SCAN_REVERSE);

	/* scan forwards */
	if (controls & 0x08)
		(*execute_command)(laserdisc, CMD_SCAN_FORWARD);

	/* slow backwards */
	if (!(last_controls & 0x10) && (controls & 0x10))
		(*execute_command)(laserdisc, CMD_SLOW_REVERSE);

	/* slow forwards */
	if (!(last_controls & 0x20) && (controls & 0x20))
		(*execute_command)(laserdisc, CMD_SLOW_FORWARD);

	/* fast backwards */
	if (controls & 0x40)
		(*execute_command)(laserdisc, CMD_FAST_REVERSE);

	/* fast forwards */
	if (controls & 0x80)
		(*execute_command)(laserdisc, CMD_FAST_FORWARD);

	/* play/pause */
	if (!(last_controls & 0x100) && (controls & 0x100))
	{
		playing = !playing;
		(*execute_command)(laserdisc, playing ? CMD_PLAY : CMD_PAUSE);
	}

	/* toggle frame display */
	if (!(last_controls & 0x200) && (controls & 0x200))
		(*execute_command)(laserdisc, CMD_FRAME_TOGGLE);

	/* toggle chapter display */
	if (!(last_controls & 0x400) && (controls & 0x400))
		(*execute_command)(laserdisc, CMD_CHAPTER_TOGGLE);

	/* toggle left channel */
	if (!(last_controls & 0x800) && (controls & 0x800))
		(*execute_command)(laserdisc, CMD_CH1_TOGGLE);

	/* toggle right channel */
	if (!(last_controls & 0x1000) && (controls & 0x1000))
		(*execute_command)(laserdisc, CMD_CH2_TOGGLE);

	/* numbers */
	for (number = 0; number < 10; number++)
		if (!(last_controls & (0x10000 << number)) && (controls & (0x10000 << number)))
			(*execute_command)(laserdisc, CMD_0 + number);

	/* enter */
	if (!(last_controls & 0x4000000) && (controls & 0x4000000))
		(*execute_command)(laserdisc, CMD_SEARCH);

	last_controls = controls;
}


static TIMER_CALLBACK( vsync_update )
{
	running_device *laserdisc = machine->m_devicelist.first(LASERDISC);
	int vblank_scanline;
	attotime target;

	/* handle commands */
	if (!param)
		process_commands(laserdisc);

	/* set a timer to go off on the next VBLANK */
	vblank_scanline = machine->primary_screen->visible_area().max_y + 1;
	target = machine->primary_screen->time_until_pos(vblank_scanline);
	timer_set(machine, target, NULL, 0, vsync_update);
}


static MACHINE_START( ldplayer )
{
	vsync_update(machine, NULL, 1);
}


static TIMER_CALLBACK( autoplay )
{
	running_device *laserdisc = machine->m_devicelist.first(LASERDISC);

	/* start playing */
	(*execute_command)(laserdisc, CMD_PLAY);
	playing = TRUE;
}


static MACHINE_RESET( ldplayer )
{
	/* set up a timer to start playing immediately */
	timer_set(machine, attotime_zero, NULL, 0, autoplay);

	/* indicate the name of the file we opened */
	popmessage("Opened %s\n", filename.cstr());
}



/*************************************
 *
 *  PR-8210 implementation
 *
 *************************************/

INLINE void pr8210_add_command(UINT8 command)
{
	pr8210_command_buffer[pr8210_command_buffer_in++ % ARRAY_LENGTH(pr8210_command_buffer)] = (command & 0x1f) | 0x20;
	pr8210_command_buffer[pr8210_command_buffer_in++ % ARRAY_LENGTH(pr8210_command_buffer)] = 0x00 | 0x20;
}


static TIMER_CALLBACK( pr8210_bit_off_callback )
{
	running_device *laserdisc = (running_device *)ptr;

	/* deassert the control line */
	laserdisc_line_w(laserdisc, LASERDISC_LINE_CONTROL, CLEAR_LINE);
}


static TIMER_CALLBACK( pr8210_bit_callback )
{
	attotime duration = ATTOTIME_IN_MSEC(30);
	running_device *laserdisc = (running_device *)ptr;
	UINT8 bitsleft = param >> 16;
	UINT8 data = param;

	/* if we have bits, process */
	if (bitsleft != 0)
	{
		/* assert the line and set a timer for deassertion */
		laserdisc_line_w(laserdisc, LASERDISC_LINE_CONTROL, ASSERT_LINE);
		timer_set(machine, ATTOTIME_IN_USEC(250), ptr, 0, pr8210_bit_off_callback);

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
	running_device *laserdisc = machine->m_devicelist.first(LASERDISC);
	MACHINE_START_CALL(ldplayer);
	pr8210_bit_timer = timer_alloc(machine, pr8210_bit_callback, (void *)laserdisc);
}


static MACHINE_RESET( pr8210 )
{
	MACHINE_RESET_CALL(ldplayer);
	timer_adjust_oneshot(pr8210_bit_timer, attotime_zero, 0);
}


static void pr8210_execute(running_device *laserdisc, int command)
{
	static const UINT8 digits[10] = { 0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d, 0x03, 0x13 };

	switch (command)
	{
		case CMD_SCAN_REVERSE:
			if (pr8210_command_buffer_in == pr8210_command_buffer_out ||
				pr8210_command_buffer_in == (pr8210_command_buffer_out + 1) % ARRAY_LENGTH(pr8210_command_buffer))
			{
				pr8210_add_command(0x1c);
				playing = TRUE;
			}
			break;

		case CMD_STEP_REVERSE:
			pr8210_add_command(0x12);
			playing = FALSE;
			break;

		case CMD_SLOW_REVERSE:
			pr8210_add_command(0x02);
			playing = TRUE;
			break;

		case CMD_FAST_REVERSE:
			if (pr8210_command_buffer_in == pr8210_command_buffer_out ||
				pr8210_command_buffer_in == (pr8210_command_buffer_out + 1) % ARRAY_LENGTH(pr8210_command_buffer))
			{
				pr8210_add_command(0x0c);
				playing = TRUE;
			}
			break;

		case CMD_SCAN_FORWARD:
			if (pr8210_command_buffer_in == pr8210_command_buffer_out ||
				pr8210_command_buffer_in == (pr8210_command_buffer_out + 1) % ARRAY_LENGTH(pr8210_command_buffer))
			{
				pr8210_add_command(0x08);
				playing = TRUE;
			}
			break;

		case CMD_STEP_FORWARD:
			pr8210_add_command(0x04);
			playing = FALSE;
			break;

		case CMD_SLOW_FORWARD:
			pr8210_add_command(0x18);
			playing = TRUE;
			break;

		case CMD_FAST_FORWARD:
			if (pr8210_command_buffer_in == pr8210_command_buffer_out ||
				pr8210_command_buffer_in == (pr8210_command_buffer_out + 1) % ARRAY_LENGTH(pr8210_command_buffer))
			{
				pr8210_add_command(0x10);
				playing = TRUE;
			}
			break;

		case CMD_PLAY:
			pr8210_add_command(0x14);
			playing = TRUE;
			break;

		case CMD_PAUSE:
			pr8210_add_command(0x0a);
			playing = FALSE;
			break;

		case CMD_FRAME_TOGGLE:
			pr8210_add_command(0x0b);
			break;

		case CMD_CHAPTER_TOGGLE:
			pr8210_add_command(0x06);
			break;

		case CMD_CH1_TOGGLE:
			pr8210_add_command(0x0e);
			break;

		case CMD_CH2_TOGGLE:
			pr8210_add_command(0x16);
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
			pr8210_add_command(digits[command - CMD_0]);
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

static void ldv1000_execute(running_device *laserdisc, int command)
{
	static const UINT8 digits[10] = { 0x3f, 0x0f, 0x8f, 0x4f, 0x2f, 0xaf, 0x6f, 0x1f, 0x9f, 0x5f };
	switch (command)
	{
		case CMD_SCAN_REVERSE:
			laserdisc_data_w(laserdisc, 0xf8);
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

		case CMD_FRAME_TOGGLE:
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
	PORT_BIT( 0x0000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Step reverse") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x0000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Step forward") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x0000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Scan reverse") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x0000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Scan forward") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x0000010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Slow reverse") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x0000020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Slow forward") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x0000040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Fast reverse") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x0000080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Fast forward") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x0000100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Play/Pause") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x0000200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Toggle frame display") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0000400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle chapter display") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0000800, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle left channel") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x0001000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle right channel") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0010000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("0") PORT_PLAYER(2) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x0020000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("1") PORT_PLAYER(2) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x0040000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("2") PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x0080000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("3") PORT_PLAYER(2) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x0100000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("4") PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x0200000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("5") PORT_PLAYER(2) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x0400000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("6") PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x0800000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("7") PORT_PLAYER(2) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x1000000, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("8") PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x2000000, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("9") PORT_PLAYER(2) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x4000000, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Enter") PORT_PLAYER(2) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_ENTER)
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
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ldplayer_ntsc )
	MDRV_IMPORT_FROM(ldplayer_core)
	MDRV_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_RGB32)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ldv1000 )
	MDRV_IMPORT_FROM(ldplayer_ntsc)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_LDV1000, "screen", "ldsound")
	MDRV_LASERDISC_GET_DISC(get_disc)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pr8210 )
	MDRV_IMPORT_FROM(ldplayer_ntsc)
	MDRV_MACHINE_START(pr8210)
	MDRV_MACHINE_RESET(pr8210)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_PR8210, "screen", "ldsound")
	MDRV_LASERDISC_GET_DISC(get_disc)
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

static DRIVER_INIT( ldv1000 ) { execute_command = ldv1000_execute; }
static DRIVER_INIT( pr8210 )  { execute_command = pr8210_execute; }



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2008, ldv1000, 0, ldv1000, ldplayer, ldv1000, ROT0, "MAME", "Pioneer LDV-1000 Simulator", 0 )
GAMEL(2008, pr8210,  0, pr8210,  ldplayer, pr8210,  ROT0, "MAME", "Pioneer PR-8210 Simulator", 0, layout_pr8210 )
