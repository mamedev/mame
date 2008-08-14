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


static render_texture *video_texture;
static astring *filename;

static input_port_value last_controls;
static UINT8 playing;
static UINT8 displaying;



/*************************************
 *
 *  Video update
 *
 *************************************/

static VIDEO_START( ldplayer )
{
	/* allocate a video texture */
	video_texture = render_texture_alloc(NULL, NULL);
	if (video_texture == NULL)
		fatalerror("Out of memory allocating video texture");
}


static VIDEO_UPDATE( ldplayer )
{
	const device_config *laserdisc = device_list_first(screen->machine->config->devicelist, LASERDISC);
	bitmap_t *video_bitmap;

	/* now talk to the laserdisc */
	laserdisc_get_video(laserdisc, &video_bitmap);
	if (video_bitmap != NULL)
		render_texture_set_bitmap(video_texture, video_bitmap, NULL, 0, TEXFORMAT_YUY16);

	/* add a quad to the screen */
	render_container_empty(render_container_get_screen(screen));
	render_screen_add_quad(screen, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), video_texture, PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));

	return 0;
}



/*************************************
 *
 *  Timers and sync
 *
 *************************************/

static void process_commands(const device_config *laserdisc)
{
	static const UINT8 digits[10] = { 0x3f, 0x0f, 0x8f, 0x4f, 0x2f, 0xaf, 0x6f, 0x1f, 0x9f, 0x5f };
	input_port_value controls = input_port_read(laserdisc->machine, "controls");
 	int number;

	/* scan/step backwards */
	if (!(last_controls & 0x01) && (controls & 0x01))
	{
		if (playing)
			laserdisc_data_w(laserdisc, 0xf8);
		else
			laserdisc_data_w(laserdisc, 0xfe);
	}
	else if ((last_controls & 0x01) && !(controls & 0x01))
	{
		if (playing)
			laserdisc_data_w(laserdisc, 0xfd);
	}

	/* scan/step forwards */
	if (!(last_controls & 0x02) && (controls & 0x02))
	{
		if (playing)
			laserdisc_data_w(laserdisc, 0xf0);
		else
			laserdisc_data_w(laserdisc, 0xf6);
	}
	else if ((last_controls & 0x02) && !(controls & 0x02))
	{
		if (playing)
			laserdisc_data_w(laserdisc, 0xfd);
	}

	/* play/pause */
	if (!(last_controls & 0x10) && (controls & 0x10))
	{
		playing = !playing;
		laserdisc_data_w(laserdisc, playing ? 0xfd : 0xa0);
	}

	/* toggle display */
	if (!(last_controls & 0x20) && (controls & 0x20))
	{
		displaying = !displaying;
		laserdisc_data_w(laserdisc, digits[displaying]);
		laserdisc_data_w(laserdisc, 0xf1);
	}

	/* numbers */
	for (number = 0; number < 10; number++)
		if (!(last_controls & (0x100 << number)) && (controls & (0x100 << number)))
			laserdisc_data_w(laserdisc, digits[number]);

	/* enter */
	if (!(last_controls & 0x40000) && (controls & 0x40000))
	{
		playing = FALSE;
		laserdisc_data_w(laserdisc, 0xf7);
	}

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
	laserdisc_data_w(laserdisc, 0xfd);
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

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_SELF_RENDER)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)

	MDRV_VIDEO_START(ldplayer)
	MDRV_VIDEO_UPDATE(ldplayer)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("laserdisc", CUSTOM, 0)
	MDRV_SOUND_CONFIG(laserdisc_custom_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ldplayer_ntsc )
	MDRV_IMPORT_FROM(ldplayer_core)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_RAW_PARAMS(XTAL_14_31818MHz, 910, 0, 720, 525.0/2, 0, 480/2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ldv1000 )
	MDRV_IMPORT_FROM(ldplayer_ntsc)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_LDV1000)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ldv1000 )
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



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2008, ldv1000, 0, ldv1000, ldplayer, ldplayer, ROT0, "MAME", "LDV-1000 Simulator", 0 )
