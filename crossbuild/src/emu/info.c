/***************************************************************************

    info.c

    Dumps the MAME internal data as an XML file.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <ctype.h>

#include "driver.h"
#include "sound/samples.h"
#include "info.h"
#include "xmlfile.h"
#include "hash.h"

/* MESS/MAME configuration */
#ifdef MESS
#define XML_ROOT "mess"
#define XML_TOP "machine"
#else
#define XML_ROOT "mame"
#define XML_TOP "game"
#endif

#ifdef MESS
void print_game_device(FILE* out, const game_driver* game);
void print_game_ramoptions(FILE* out, const game_driver* game);
#endif /* MESS */

/* Print a free format string */

static void print_game_switch(FILE* out, const game_driver* game)
{
	const input_port_entry* input;

	begin_resource_tracking();

	input = input_port_allocate(game->ipt, NULL);

	while (input->type != IPT_END)
	{
		if (input->type==IPT_DIPSWITCH_NAME)
		{
			int def = input->default_value;

			fprintf(out, "\t\t<dipswitch");

			fprintf(out, " name=\"%s\"", xml_normalize_string(input->name));
			++input;

			fprintf(out, ">\n");

			while (input->type==IPT_DIPSWITCH_SETTING)
			{
				fprintf(out, "\t\t\t<dipvalue");
				fprintf(out, " name=\"%s\"", xml_normalize_string(input->name));
				if (def == input->default_value)
					fprintf(out, " default=\"yes\"");

				fprintf(out, "/>\n");

				++input;
			}

			fprintf(out, "\t\t</dipswitch>\n");
		}
		else
			++input;
	}

	end_resource_tracking();
}

static void print_game_input(FILE* out, const game_driver* game)
{
enum {cjoy, cdoublejoy, cAD_stick, cdial, ctrackball, cpaddle, clightgun, cpedal, ENDCONTROLTYPES};
	const input_port_entry* input;
	int nplayer = 0;
	int nbutton = 0;
	int ncoin = 0;
	int controlsyes = 0;
	int analogcontrol = 0;
	int i;
	const char* service = 0;
	const char* tilt = 0;
	const char* control_types[] = {"joy", "doublejoy", "stick", "dial", "trackball", "paddle", "lightgun", "pedal"};
	static struct _input_info
	{
		const char *	type;			/* general type of input */
		const char *	Xway;			/* 2, 4, or 8 way */
		int				analog;
		int				min;			/* analog minimum value */
		int				max;			/* analog maximum value  */
		int				sensitivity;	/* default analog sensitivity */
		int				keydelta;		/* default analog keydelta */
		int				reverse;		/* default analog reverse setting */
	} control[ENDCONTROLTYPES];

	for (i=0;i<ENDCONTROLTYPES;i++)
	{
		control[i].type = control_types[i];
		control[i].Xway = 0;
		control[i].analog = 0;
		control[i].min = 0;
		control[i].max = 0;
		control[i].sensitivity = 0;
		control[i].keydelta = 0;
		control[i].reverse = 0;
	}

	begin_resource_tracking();

	input = input_port_allocate(game->ipt, NULL);

	while (input->type != IPT_END)
	{
		if (nplayer < input->player+1)
			nplayer = input->player+1;

		switch (input->type)
		{
			case IPT_JOYSTICK_LEFT:
			case IPT_JOYSTICK_RIGHT:

				/* if control not defined, start it off as horizontal 2-way */
				if (!control[cjoy].Xway)
					control[cjoy].Xway = "joy2way";
				else if (strcmp(control[cjoy].Xway,"joy2way") == 0)
					;
				/* if already defined as vertical, make it 4 or 8 way */
				else if (strcmp(control[cjoy].Xway,"vjoy2way") == 0)
				{
					if (input->way == 4)
						control[cjoy].Xway = "joy4way";
					else
						control[cjoy].Xway = "joy8way";
				}
				controlsyes = 1;
				break;

			case IPT_JOYSTICK_UP:
			case IPT_JOYSTICK_DOWN:

				/* if control not defined, start it off as vertical 2-way */
				if (!control[cjoy].Xway)
					control[cjoy].Xway= "vjoy2way";
				else if (strcmp(control[cjoy].Xway,"vjoy2way") == 0)
					;
				/* if already defined as horiz, make it 4 or 8way */
				else if (strcmp(control[cjoy].Xway,"joy2way")==0)
				{
					if (input->way == 4)
						control[cjoy].Xway = "joy4way";
					else
						control[cjoy].Xway = "joy8way";
				}
				controlsyes = 1;
				break;

			case IPT_JOYSTICKRIGHT_UP:
			case IPT_JOYSTICKRIGHT_DOWN:
			case IPT_JOYSTICKLEFT_UP:
			case IPT_JOYSTICKLEFT_DOWN:

				/* if control not defined, start it off as vertical 2way */
				if (!control[cdoublejoy].Xway)
					control[cdoublejoy].Xway= "vdoublejoy2way";
				else if (strcmp(control[cdoublejoy].Xway,"vdoublejoy2way") == 0)
					;
				/* if already defined as horiz, make it 4 or 8 way */
				else if (strcmp(control[cdoublejoy].Xway,"doublejoy2way") == 0)
				{
					if (input->way == 4)
						control[cdoublejoy].Xway = "doublejoy4way";
					else
						control[cdoublejoy].Xway = "doublejoy8way";
				}
				controlsyes = 1;
				break;

			case IPT_JOYSTICKRIGHT_LEFT:
			case IPT_JOYSTICKRIGHT_RIGHT:
			case IPT_JOYSTICKLEFT_LEFT:
			case IPT_JOYSTICKLEFT_RIGHT:

				/* if control not defined, start it off as horiz 2-way */
				if (!control[cdoublejoy].Xway)
					control[cdoublejoy].Xway="doublejoy2way";
				else if (strcmp(control[cdoublejoy].Xway,"doublejoy2way") == 0)
					;
				/* if already defined as vertical, make it 4 or 8 way */
				else if (strcmp(control[cdoublejoy].Xway,"vdoublejoy2way") == 0)
				{
					if (input->way == 4)
						control[cdoublejoy].Xway = "doublejoy4way";
					else
						control[cdoublejoy].Xway = "doublejoy8way";
				}
				controlsyes = 1;
				break;

			/* mark as an analog input, and get analog stats after switch */
			case IPT_PADDLE:
				analogcontrol = cpaddle;
				break;
			case IPT_DIAL:
				analogcontrol = cdial;
				break;
			case IPT_TRACKBALL_X:
			case IPT_TRACKBALL_Y:
				analogcontrol = ctrackball;
				break;
			case IPT_AD_STICK_X:
			case IPT_AD_STICK_Y:
				analogcontrol = cAD_stick;
				break;
			case IPT_LIGHTGUN_X:
			case IPT_LIGHTGUN_Y:
				analogcontrol = clightgun;
				break;
			case IPT_PEDAL:
			case IPT_PEDAL2:
			case IPT_PEDAL3:
				analogcontrol = cpedal;
				break;

			case IPT_BUTTON1:
				if (nbutton<1) nbutton = 1;
				break;
			case IPT_BUTTON2:
				if (nbutton<2) nbutton = 2;
				break;
			case IPT_BUTTON3:
				if (nbutton<3) nbutton = 3;
				break;
			case IPT_BUTTON4:
				if (nbutton<4) nbutton = 4;
				break;
			case IPT_BUTTON5:
				if (nbutton<5) nbutton = 5;
				break;
			case IPT_BUTTON6:
				if (nbutton<6) nbutton = 6;
				break;
			case IPT_BUTTON7:
				if (nbutton<7) nbutton = 7;
				break;
			case IPT_BUTTON8:
				if (nbutton<8) nbutton = 8;
				break;
			case IPT_BUTTON9:
				if (nbutton<9) nbutton = 9;
				break;
			case IPT_BUTTON10:
				if (nbutton<10) nbutton = 10;
				break;

			case IPT_COIN1:
				if (ncoin < 1) ncoin = 1;
				break;
			case IPT_COIN2:
				if (ncoin < 2) ncoin = 2;
				break;
			case IPT_COIN3:
				if (ncoin < 3) ncoin = 3;
				break;
			case IPT_COIN4:
				if (ncoin < 4) ncoin = 4;
				break;
			case IPT_COIN5:
				if (ncoin < 5) ncoin = 5;
				break;
			case IPT_COIN6:
				if (ncoin < 6) ncoin = 6;
				break;
			case IPT_COIN7:
				if (ncoin < 7) ncoin = 7;
				break;
			case IPT_COIN8:
				if (ncoin < 8) ncoin = 8;
				break;
			case IPT_SERVICE :
				service = "yes";
				break;
			case IPT_TILT :
				tilt = "yes";
				break;
		}

		/* get the analog stats */
		if (analogcontrol)
		{
			controlsyes = 1;
			control[analogcontrol].analog = 1;

			if (input->analog.min)
				control[analogcontrol].min = input->analog.min;
			if (input->analog.max)
				control[analogcontrol].max = input->analog.max;
			if (input->analog.sensitivity)
				control[analogcontrol].sensitivity = input->analog.sensitivity;
			if (input->analog.delta)
				control[analogcontrol].keydelta = input->analog.delta;
			if (input->analog.reverse)
				control[analogcontrol].reverse = 1;

			analogcontrol = 0;
		}

		++input;
	}

	fprintf(out, "\t\t<input");
	fprintf(out, " players=\"%d\"", nplayer );
	if (nbutton)
		fprintf(out, " buttons=\"%d\"", nbutton );
	if (ncoin)
		fprintf(out, " coins=\"%d\"", ncoin );
	if (service)
		fprintf(out, " service=\"%s\"", xml_normalize_string(service) );
	if (tilt)
		fprintf(out, " tilt=\"%s\"", xml_normalize_string(tilt) );
	fprintf(out, ">\n");

	for (i=0;i<ENDCONTROLTYPES;i++)
	{
		if (control[i].Xway)
			fprintf(out, "\t\t\t<control type=\"%s\"/>\n", xml_normalize_string(control[i].Xway) );
		if (control[i].analog)
		{
			fprintf(out, "\t\t\t<control type=\"%s\"", xml_normalize_string(control_types[i]) );
			if (control[i].min || control[i].max)
			{
				fprintf(out, " minimum=\"%d\"", control[i].min);
				fprintf(out, " maximum=\"%d\"", control[i].max);
			}
			if (control[i].sensitivity)
				fprintf(out, " sensitivity=\"%d\"", control[i].sensitivity);
			if (control[i].keydelta)
				fprintf(out, " keydelta=\"%d\"", control[i].keydelta);
			if (control[i].reverse)
				fprintf(out, " reverse=\"yes\"");

			fprintf(out, "/>\n");
		}
	}
	fprintf(out, "\t\t</input>\n");

	end_resource_tracking();
}

static void print_game_bios(FILE* out, const game_driver* game)
{
	const rom_entry *rom;

	if (game->rom != NULL)
	{
		for (rom = game->rom; !ROMENTRY_ISEND(rom); rom++)
			if (ROMENTRY_ISSYSTEM_BIOS(rom))
			{
				const char *name = ROM_GETHASHDATA(rom);
				const char *description = name + strlen(name) + 1;

				fprintf(out, "\t\t<biosset");
				fprintf(out, " name=\"%s\"", xml_normalize_string(name));
				fprintf(out, " description=\"%s\"", xml_normalize_string(description));
				if (ROM_GETBIOSFLAGS(rom) == 1)
					fprintf(out, " default=\"yes\"");
				fprintf(out, "/>\n");
			}
	}
}

static void print_game_rom(FILE* out, const game_driver* game)
{
	const rom_entry *region, *rom, *chunk;
	const rom_entry *pregion, *prom, *fprom=NULL;
	const game_driver *clone_of;
	int rom_type;

	if (!game->rom)
		return;

	clone_of = driver_get_clone(game);
	for (rom_type = 0; rom_type < 3; rom_type++)
	{
		for (region = rom_first_region(game); region; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				int offset, length, in_parent, is_disk, is_bios, found_bios, i;
				char name[100], bios_name[100];

				strcpy(name,ROM_GETNAME(rom));
				offset = ROM_GETOFFSET(rom);
				is_disk = ROMREGION_ISDISKDATA(region);
				is_bios = ROM_GETBIOSFLAGS(rom);

				switch (rom_type)
				{
					case 0:		/* rom_type 0 = BIOS */
						if (is_disk || !is_bios)
							continue;
						break;
					case 1:		/* rom_type 1 = ROM */
						if (is_disk || is_bios)
							continue;
						break;
					case 2:		/* rom_type 1 = DISK */
						if (!is_disk || is_bios)
							continue;
						break;
				}

				in_parent = 0;
				length = 0;
				for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
					length += ROM_GETLENGTH(chunk);

				if (!ROM_NOGOODDUMP(rom) && clone_of)
				{
					fprom=NULL;
					for (pregion = rom_first_region(clone_of); pregion; pregion = rom_next_region(pregion))
						for (prom = rom_first_file(pregion); prom; prom = rom_next_file(prom))
							if (hash_data_is_equal(ROM_GETHASHDATA(rom), ROM_GETHASHDATA(prom), 0))
							{
								if (!fprom || !strcmp(ROM_GETNAME(prom), name))
									fprom=prom;
								in_parent = 1;
							}
				}

				found_bios = 0;
				if(!is_disk && is_bios)
				{
					/* Scan backwards for name */
					for (prom = rom-1; prom != game->rom; prom--)
						if (ROMENTRY_ISSYSTEM_BIOS(prom))
						{
							strcpy(bios_name, ROM_GETHASHDATA(prom));
							found_bios = 1;
							break;
						}
				}


				if (!is_disk)
					fprintf(out, "\t\t<rom");
				else
					fprintf(out, "\t\t<disk");

				if (*name)
					fprintf(out, " name=\"%s\"", xml_normalize_string(name));
				if (in_parent)
					fprintf(out, " merge=\"%s\"", xml_normalize_string(ROM_GETNAME(fprom)));
				if (!is_disk && found_bios)
					fprintf(out, " bios=\"%s\"", xml_normalize_string(bios_name));
				if (!is_disk)
					fprintf(out, " size=\"%d\"", length);

				/* dump checksum information only if there is a known dump */
				if (!hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_NO_DUMP))
				{
					for (i=0;i<HASH_NUM_FUNCTIONS;i++)
					{
						int func = 1<<i;
						const char* func_name = hash_function_name(func);
						char checksum[1000];

						if (hash_data_extract_printable_checksum(ROM_GETHASHDATA(rom), func, checksum))
						{
							fprintf(out, " %s=\"%s\"", func_name, checksum);
						}
					}
				}

				switch (ROMREGION_GETTYPE(region))
				{
					case REGION_CPU1: fprintf(out, " region=\"cpu1\""); break;
					case REGION_CPU2: fprintf(out, " region=\"cpu2\""); break;
					case REGION_CPU3: fprintf(out, " region=\"cpu3\""); break;
					case REGION_CPU4: fprintf(out, " region=\"cpu4\""); break;
					case REGION_CPU5: fprintf(out, " region=\"cpu5\""); break;
					case REGION_CPU6: fprintf(out, " region=\"cpu6\""); break;
					case REGION_CPU7: fprintf(out, " region=\"cpu7\""); break;
					case REGION_CPU8: fprintf(out, " region=\"cpu8\""); break;
					case REGION_GFX1: fprintf(out, " region=\"gfx1\""); break;
					case REGION_GFX2: fprintf(out, " region=\"gfx2\""); break;
					case REGION_GFX3: fprintf(out, " region=\"gfx3\""); break;
					case REGION_GFX4: fprintf(out, " region=\"gfx4\""); break;
					case REGION_GFX5: fprintf(out, " region=\"gfx5\""); break;
					case REGION_GFX6: fprintf(out, " region=\"gfx6\""); break;
					case REGION_GFX7: fprintf(out, " region=\"gfx7\""); break;
					case REGION_GFX8: fprintf(out, " region=\"gfx8\""); break;
					case REGION_PROMS: fprintf(out, " region=\"proms\""); break;
					case REGION_PLDS: fprintf(out, " region=\"plds\""); break;
					case REGION_SOUND1: fprintf(out, " region=\"sound1\""); break;
					case REGION_SOUND2: fprintf(out, " region=\"sound2\""); break;
					case REGION_SOUND3: fprintf(out, " region=\"sound3\""); break;
					case REGION_SOUND4: fprintf(out, " region=\"sound4\""); break;
					case REGION_SOUND5: fprintf(out, " region=\"sound5\""); break;
					case REGION_SOUND6: fprintf(out, " region=\"sound6\""); break;
					case REGION_SOUND7: fprintf(out, " region=\"sound7\""); break;
					case REGION_SOUND8: fprintf(out, " region=\"sound8\""); break;
					case REGION_USER1: fprintf(out, " region=\"user1\""); break;
					case REGION_USER2: fprintf(out, " region=\"user2\""); break;
					case REGION_USER3: fprintf(out, " region=\"user3\""); break;
					case REGION_USER4: fprintf(out, " region=\"user4\""); break;
					case REGION_USER5: fprintf(out, " region=\"user5\""); break;
					case REGION_USER6: fprintf(out, " region=\"user6\""); break;
					case REGION_USER7: fprintf(out, " region=\"user7\""); break;
					case REGION_USER8: fprintf(out, " region=\"user8\""); break;
					case REGION_USER9: fprintf(out, " region=\"user9\""); break;
					case REGION_USER10: fprintf(out, " region=\"user10\""); break;
					case REGION_USER11: fprintf(out, " region=\"user11\""); break;
					case REGION_USER12: fprintf(out, " region=\"user12\""); break;
					case REGION_USER13: fprintf(out, " region=\"user13\""); break;
					case REGION_USER14: fprintf(out, " region=\"user14\""); break;
					case REGION_USER15: fprintf(out, " region=\"user15\""); break;
					case REGION_USER16: fprintf(out, " region=\"user16\""); break;
					case REGION_USER17: fprintf(out, " region=\"user17\""); break;
					case REGION_USER18: fprintf(out, " region=\"user18\""); break;
					case REGION_USER19: fprintf(out, " region=\"user19\""); break;
					case REGION_USER20: fprintf(out, " region=\"user20\""); break;
					case REGION_DISKS: fprintf(out, " region=\"disks\""); break;
					default: fprintf(out, " region=\"0x%x\"", (int)ROMREGION_GETTYPE(region)); break;
			}

			if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_NO_DUMP))
				fprintf(out, " status=\"nodump\"");
			if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP))
				fprintf(out, " status=\"baddump\"");

			if (!is_disk)
			{
				if (ROMREGION_GETFLAGS(region) & ROMREGION_DISPOSE)
					fprintf(out, " dispose=\"yes\"");

				fprintf(out, " offset=\"%x\"", offset);
			}
			else
			{
				fprintf(out, " index=\"%x\"", DISK_GETINDEX(rom));
			}
			fprintf(out, "/>\n");
		}
	}
}

static void print_game_sampleof(FILE* out, const game_driver* game)
{
#if (HAS_SAMPLES)
	machine_config drv;
	int i;

	expand_machine_driver(game->drv, &drv);

	for( i = 0; i < MAX_SOUND && drv.sound[i].type != SOUND_DUMMY; i++ )
	{
		const char *const *samplenames = NULL;
		if( drv.sound[i].type == SOUND_SAMPLES )
			samplenames = ((const struct Samplesinterface *)drv.sound[i].config)->samplenames;
		if (samplenames != 0 && samplenames[0] != 0) {
			int k = 0;
			if (samplenames[k][0]=='*')
			{
				/* output sampleof only if different from game name */
				if (strcmp(samplenames[k] + 1, game->name)!=0)
					fprintf(out, " sampleof=\"%s\"", xml_normalize_string(samplenames[k] + 1));
				++k;
			}
		}
	}
#endif
}

static void print_game_sample(FILE* out, const game_driver* game)
{
#if (HAS_SAMPLES)
	machine_config drv;
	int i;

	expand_machine_driver(game->drv, &drv);

	for( i = 0; i < MAX_SOUND && drv.sound[i].type != SOUND_DUMMY; i++ )
	{
		const char *const *samplenames = NULL;
		if( drv.sound[i].type == SOUND_SAMPLES )
			samplenames = ((const struct Samplesinterface *)drv.sound[i].config)->samplenames;
		if (samplenames != 0 && samplenames[0] != 0) {
			int k = 0;
			if (samplenames[k][0]=='*')
			{
				++k;
			}
			while (samplenames[k] != 0) {
				/* check if is not empty */
				if (*samplenames[k]) {
					/* check if sample is duplicate */
					int l = 0;
					while (l<k && strcmp(samplenames[k],samplenames[l])!=0)
						++l;
					if (l==k)
						fprintf(out, "\t\t<sample name=\"%s\"/>\n", xml_normalize_string(samplenames[k]));
				}
				++k;
			}
		}
	}
#endif
}

static void print_game_micro(FILE* out, const game_driver* game)
{
	machine_config driver;
	const cpu_config* cpu;
	const sound_config* sound;
	int j;

	expand_machine_driver(game->drv, &driver);
	cpu = driver.cpu;
	sound = driver.sound;

	for(j=0;j<MAX_CPU;++j)
	{
		if (cpu[j].type != CPU_DUMMY)
		{
			fprintf(out, "\t\t<chip");
			fprintf(out, " type=\"cpu\"");

			fprintf(out, " name=\"%s\"", xml_normalize_string(cputype_name(cpu[j].type)));

			fprintf(out, " clock=\"%d\"", cpu[j].clock);
			fprintf(out, "/>\n");
		}
	}

	for(j=0;j<MAX_SOUND;++j)
	{
		if (sound[j].type != SOUND_DUMMY)
		{
			fprintf(out, "\t\t<chip");
			fprintf(out, " type=\"audio\"");
			fprintf(out, " name=\"%s\"", xml_normalize_string(sndtype_name(sound[j].type)));
			if (sound[j].clock)
				fprintf(out, " clock=\"%d\"", sound[j].clock);
			fprintf(out, "/>\n");
		}
	}
}

static void print_game_display(FILE* out, const game_driver* game)
{
	machine_config driver;

	int dx;
	int dy;
	int scrnum;

	expand_machine_driver(game->drv, &driver);

	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		if (driver.screen[scrnum].tag != NULL)
		{
			fprintf(out, "\t\t<display");

			fprintf(out, " type=\"%s\"", (driver.video_attributes & VIDEO_TYPE_VECTOR) ? "vector" : "raster" );

			switch (game->flags & ORIENTATION_MASK) {
				case ORIENTATION_FLIP_X:
					fprintf(out, " rotate=\"0\" flipx=\"yes\"");
					break;
				case ORIENTATION_FLIP_Y:
					fprintf(out, " rotate=\"180\" flipx=\"yes\"");
					break;
				case ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
					fprintf(out, " rotate=\"180\"");
					break;
				case ORIENTATION_SWAP_XY:
					fprintf(out, " rotate=\"90\" flipx=\"yes\"");
					break;
				case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X:
					fprintf(out, " rotate=\"90\"");
					break;
				case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_Y:
					fprintf(out, " rotate=\"270\"");
					break;
				case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
					fprintf(out, " rotate=\"270\" flipx=\"yes\"");
					break;
				default:
					fprintf(out, " rotate=\"0\"");
					break;
			}

			/* output width and height only for games that are not vector */
			if (! (driver.video_attributes & VIDEO_TYPE_VECTOR) )
			{
				dx = driver.screen[scrnum].defstate.visarea.max_x - driver.screen[scrnum].defstate.visarea.min_x + 1;
				dy = driver.screen[scrnum].defstate.visarea.max_y - driver.screen[scrnum].defstate.visarea.min_y + 1;
				fprintf(out, " width=\"%d\"", dx);
				fprintf(out, " height=\"%d\"", dy);
			}

			fprintf(out, " refresh=\"%f\"", ATTOSECONDS_TO_HZ(driver.screen[scrnum].defstate.refresh));

			fprintf(out, " />\n");
		}
}

static void print_game_sound(FILE* out, const game_driver* game)
{
	machine_config driver;
	const cpu_config* cpu;
	const sound_config* sound;

	/* check if the game have sound emulation */
	int has_sound = 0;
	int i;

	expand_machine_driver(game->drv, &driver);
	cpu = driver.cpu;
	sound = driver.sound;

	i = 0;
	while (i < MAX_SOUND && !has_sound)
	{
		if (sound[i].type != SOUND_DUMMY)
			has_sound = 1;
		++i;
	}

	fprintf(out, "\t\t<sound");

	/* sound channel */
	if (has_sound)
	{
		int speakers;
		for (speakers = 0; speakers < MAX_SPEAKER; speakers++)
			if (driver.speaker[speakers].tag == NULL)
				break;
		fprintf(out, " channels=\"%d\"", speakers);
	}
	else
		fprintf(out, " channels=\"0\"");

	fprintf(out, "/>\n");
}

static void print_game_driver(FILE* out, const game_driver* game)
{
	machine_config driver;

	expand_machine_driver(game->drv, &driver);

	fprintf(out, "\t\t<driver");

	/* The status entry is an hint for frontend authors */
	/* to select working and not working games without */
	/* the need to know all the other status entries. */
	/* Games marked as status=good are perfectly emulated, games */
	/* marked as status=imperfect are emulated with only */
	/* some minor issues, games marked as status=preliminary */
	/* don't work or have major emulation problems. */

	if (game->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_WRONG_COLORS))
		fprintf(out, " status=\"preliminary\"");
	else if (game->flags & (GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS))
		fprintf(out, " status=\"imperfect\"");
	else
		fprintf(out, " status=\"good\"");

	if (game->flags & GAME_NOT_WORKING)
		fprintf(out, " emulation=\"preliminary\"");
	else
		fprintf(out, " emulation=\"good\"");

	if (game->flags & GAME_WRONG_COLORS)
		fprintf(out, " color=\"preliminary\"");
	else if (game->flags & GAME_IMPERFECT_COLORS)
		fprintf(out, " color=\"imperfect\"");
	else
		fprintf(out, " color=\"good\"");

	if (game->flags & GAME_NO_SOUND)
		fprintf(out, " sound=\"preliminary\"");
	else if (game->flags & GAME_IMPERFECT_SOUND)
		fprintf(out, " sound=\"imperfect\"");
	else
		fprintf(out, " sound=\"good\"");

	if (game->flags & GAME_IMPERFECT_GRAPHICS)
		fprintf(out, " graphic=\"imperfect\"");
	else
		fprintf(out, " graphic=\"good\"");

	if (game->flags & GAME_NO_COCKTAIL)
		fprintf(out, " cocktail=\"preliminary\"");

	if (game->flags & GAME_UNEMULATED_PROTECTION)
		fprintf(out, " protection=\"preliminary\"");

	if (game->flags & GAME_SUPPORTS_SAVE)
		fprintf(out, " savestate=\"supported\"");
	else
		fprintf(out, " savestate=\"unsupported\"");

	fprintf(out, " palettesize=\"%d\"", driver.total_colors);

	fprintf(out, "/>\n");
}

/* Print the MAME info record for a game */
static void print_game_info(FILE* out, const game_driver* game)
{
	const char *start;
	const game_driver *clone_of;

	/* No action if not a game */
	if (game->flags & GAME_NO_STANDALONE)
		return;

	fprintf(out, "\t<" XML_TOP);

	fprintf(out, " name=\"%s\"", xml_normalize_string(game->name) );

	start = strrchr(game->source_file, '/');
	if (!start)
		start = strrchr(game->source_file, '\\');
	if (!start)
		start = game->source_file - 1;
	fprintf(out, " sourcefile=\"%s\"", xml_normalize_string(start + 1));

	if (game->flags & GAME_IS_BIOS_ROOT)
		fprintf(out, " isbios=\"yes\"");

	if (game->flags & GAME_NO_STANDALONE)
		fprintf(out, " runnable=\"no\"");

	clone_of = driver_get_clone(game);
	if (clone_of && !(clone_of->flags & GAME_IS_BIOS_ROOT))
		fprintf(out, " cloneof=\"%s\"", xml_normalize_string(clone_of->name));

	if (clone_of)
		fprintf(out, " romof=\"%s\"", xml_normalize_string(clone_of->name));

	print_game_sampleof(out, game);

	fprintf(out, ">\n");

	if (game->description)
		fprintf(out, "\t\t<description>%s</description>\n", xml_normalize_string(game->description));

	/* print the year only if is a number */
	if (game->year && strspn(game->year,"0123456789")==strlen(game->year))
		fprintf(out, "\t\t<year>%s</year>\n", xml_normalize_string(game->year) );

	if (game->manufacturer)
		fprintf(out, "\t\t<manufacturer>%s</manufacturer>\n", xml_normalize_string(game->manufacturer));

	print_game_bios(out, game);
	print_game_rom(out, game);
	print_game_sample(out, game);
	print_game_micro(out, game);
	print_game_display(out, game);
	print_game_sound(out, game);
	print_game_input(out, game);
	print_game_switch(out, game);
	print_game_driver(out, game);
#ifdef MESS
	print_game_device(out, game);
	print_game_ramoptions(out, game);
#endif

	fprintf(out, "\t</" XML_TOP ">\n");
}

static void print_mame_data(FILE* out, const game_driver* const games[], const char *gamename)
{
	int j;

	/* print games */
	for(j=0;games[j];++j)
		if (mame_strwildcmp(gamename, games[j]->name) == 0)
			print_game_info(out, games[j]);
}

/* Print the MAME database in XML format */
void print_mame_xml(FILE* out, const game_driver* const games[], const char *gamename)
{
	fprintf(out,
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE " XML_ROOT " [\n"
		"<!ELEMENT " XML_ROOT " (" XML_TOP "+)>\n"
		"\t<!ATTLIST " XML_ROOT " build CDATA #IMPLIED>\n"
		"\t<!ATTLIST " XML_ROOT " debug (yes|no) \"no\">\n"
#ifdef MESS
		"\t<!ELEMENT " XML_TOP " (description, year?, manufacturer, biosset*, rom*, disk*, sample*, chip*, display*, sound?, input?, dipswitch*, driver?, device*, ramoption*)>\n"
#else
		"\t<!ELEMENT " XML_TOP " (description, year?, manufacturer, biosset*, rom*, disk*, sample*, chip*, display*, sound?, input?, dipswitch*, driver?)>\n"
#endif
		"\t\t<!ATTLIST " XML_TOP " name CDATA #REQUIRED>\n"
		"\t\t<!ATTLIST " XML_TOP " sourcefile CDATA #IMPLIED>\n"
		"\t\t<!ATTLIST " XML_TOP " isbios (yes|no) \"no\">\n"
		"\t\t<!ATTLIST " XML_TOP " runnable (yes|no) \"yes\">\n"
		"\t\t<!ATTLIST " XML_TOP " cloneof CDATA #IMPLIED>\n"
		"\t\t<!ATTLIST " XML_TOP " romof CDATA #IMPLIED>\n"
		"\t\t<!ATTLIST " XML_TOP " sampleof CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT description (#PCDATA)>\n"
		"\t\t<!ELEMENT year (#PCDATA)>\n"
		"\t\t<!ELEMENT manufacturer (#PCDATA)>\n"
		"\t\t<!ELEMENT biosset EMPTY>\n"
		"\t\t\t<!ATTLIST biosset name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST biosset description CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST biosset default (yes|no) \"no\">\n"
		"\t\t<!ELEMENT rom EMPTY>\n"
		"\t\t\t<!ATTLIST rom name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST rom bios CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom size CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST rom crc CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom md5 CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom sha1 CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom merge CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom region CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom offset CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST rom status (baddump|nodump|good) \"good\">\n"
		"\t\t\t<!ATTLIST rom dispose (yes|no) \"no\">\n"
		"\t\t<!ELEMENT disk EMPTY>\n"
		"\t\t\t<!ATTLIST disk name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST disk md5 CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk sha1 CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk merge CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk region CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk index CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST disk status (baddump|nodump|good) \"good\">\n"
		"\t\t<!ELEMENT sample EMPTY>\n"
		"\t\t\t<!ATTLIST sample name CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT chip EMPTY>\n"
		"\t\t\t<!ATTLIST chip name CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST chip type (cpu|audio) #REQUIRED>\n"
		"\t\t\t<!ATTLIST chip clock CDATA #IMPLIED>\n"
		"\t\t<!ELEMENT display EMPTY>\n"
		"\t\t\t<!ATTLIST display type (raster|vector) #REQUIRED>\n"
		"\t\t\t<!ATTLIST display rotate (0|90|180|270) #REQUIRED>\n"
		"\t\t\t<!ATTLIST display flipx (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST display width CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display height CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display refresh CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT sound EMPTY>\n"
		"\t\t\t<!ATTLIST sound channels CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT input (control*)>\n"
		"\t\t\t<!ATTLIST input service (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST input tilt (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST input players CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST input buttons CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST input coins CDATA #IMPLIED>\n"
		"\t\t\t<!ELEMENT control EMPTY>\n"
		"\t\t\t\t<!ATTLIST control type CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST control minimum CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control maximum CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control sensitivity CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control keydelta CDATA #IMPLIED>\n"
		"\t\t\t\t<!ATTLIST control reverse (yes|no) \"no\">\n"
		"\t\t<!ELEMENT dipswitch (dipvalue*)>\n"
		"\t\t\t<!ATTLIST dipswitch name CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT dipvalue EMPTY>\n"
		"\t\t\t\t<!ATTLIST dipvalue name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST dipvalue default (yes|no) \"no\">\n"
		"\t\t<!ELEMENT driver EMPTY>\n"
		"\t\t\t<!ATTLIST driver status (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver emulation (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver color (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver sound (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver graphic (good|imperfect|preliminary) #REQUIRED>\n"
		"\t\t\t<!ATTLIST driver cocktail (good|imperfect|preliminary) #IMPLIED>\n"
		"\t\t\t<!ATTLIST driver protection (good|imperfect|preliminary) #IMPLIED>\n"
		"\t\t\t<!ATTLIST driver savestate (supported|unsupported) #REQUIRED>\n"
 		"\t\t\t<!ATTLIST driver palettesize CDATA #REQUIRED>\n"
#ifdef MESS
		"\t\t<!ELEMENT device (instance*, extension*)>\n"
		"\t\t\t<!ATTLIST device type CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST device tag CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST device mandatory CDATA #IMPLIED>\n"
		"\t\t\t<!ELEMENT instance EMPTY>\n"
		"\t\t\t\t<!ATTLIST instance name CDATA #REQUIRED>\n"
		"\t\t\t\t<!ATTLIST instance briefname CDATA #REQUIRED>\n"
		"\t\t\t<!ELEMENT extension EMPTY>\n"
		"\t\t\t\t<!ATTLIST extension name CDATA #REQUIRED>\n"
		"\t\t<!ELEMENT ramoption (#PCDATA)>\n"
		"\t\t\t<!ATTLIST ramoption default CDATA #IMPLIED>\n"
#endif
		"]>\n\n"
		"<" XML_ROOT " build=\"%s\" debug=\""
#ifdef MAME_DEBUG
		"yes"
#else
		"no"
#endif
		"\">\n",
		xml_normalize_string(build_version)
	);

	print_mame_data(out, games, gamename);

	fprintf(out, "</" XML_ROOT ">\n");
}
