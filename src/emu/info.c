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

#ifdef MESS
#include "infomess.h"
#endif /* MESS */

/* MESS/MAME configuration */
#ifdef MESS
#define XML_ROOT "mess"
#define XML_TOP "machine"
#else
#define XML_ROOT "mame"
#define XML_TOP "game"
#endif


/*-------------------------------------------------
    print_game_switches - print the DIP switch
    settings for a game
-------------------------------------------------*/

static void print_game_switches(FILE *out, const game_driver *game, const input_port_config *portlist)
{
	const input_port_config *port;
	const input_field_config *field;

	/* iterate looking for DIP switches */
	for (port = portlist; port != NULL; port = port->next)
		for (field = port->fieldlist; field != NULL; field = field->next)
			if (field->type == IPT_DIPSWITCH)
			{
				const input_setting_config *setting;

				/* output the switch name information */
				fprintf(out, "\t\t<dipswitch name=\"%s\">\n", xml_normalize_string(input_field_name(field)));

				/* loop over settings */
				for (setting = field->settinglist; setting != NULL; setting = setting->next)
				{
					fprintf(out, "\t\t\t<dipvalue name=\"%s\"", xml_normalize_string(setting->name));
					if (setting->value == field->defvalue)
						fprintf(out, " default=\"yes\"");
					fprintf(out, "/>\n");
				}

				/* terminate the switch entry */
				fprintf(out, "\t\t</dipswitch>\n");
			}
}


/*-------------------------------------------------
    print_game_input - print a summary of a game's
    input
-------------------------------------------------*/

static void print_game_input(FILE *out, const game_driver *game, const input_port_config *portlist)
{
	/* fix me -- this needs to be cleaned up to match the core style */

enum {cjoy, cdoublejoy, cAD_stick, cdial, ctrackball, cpaddle, clightgun, cpedal, ENDCONTROLTYPES};
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
	const input_port_config *port;
	const input_field_config *field;

	for (i=0;i<ENDCONTROLTYPES;i++)
	{
		control[i].type = control_types[i];
		control[i].Xway = NULL;
		control[i].analog = 0;
		control[i].min = 0;
		control[i].max = 0;
		control[i].sensitivity = 0;
		control[i].keydelta = 0;
		control[i].reverse = 0;
	}

	for (port = portlist; port != NULL; port = port->next)
		for (field = port->fieldlist; field != NULL; field = field->next)
		{
			if (nplayer < field->player+1)
				nplayer = field->player+1;

			switch (field->type)
			{
				case IPT_JOYSTICK_LEFT:
				case IPT_JOYSTICK_RIGHT:

					/* if control not defined, start it off as horizontal 2-way */
					if (control[cjoy].Xway == NULL)
						control[cjoy].Xway = "joy2way";
					else if (strcmp(control[cjoy].Xway,"joy2way") == 0)
						;
					/* if already defined as vertical, make it 4 or 8 way */
					else if (strcmp(control[cjoy].Xway,"vjoy2way") == 0)
					{
						if (field->way == 4)
							control[cjoy].Xway = "joy4way";
						else
							control[cjoy].Xway = "joy8way";
					}
					controlsyes = 1;
					break;

				case IPT_JOYSTICK_UP:
				case IPT_JOYSTICK_DOWN:

					/* if control not defined, start it off as vertical 2-way */
					if (control[cjoy].Xway == NULL)
						control[cjoy].Xway = "vjoy2way";
					else if (strcmp(control[cjoy].Xway,"vjoy2way") == 0)
						;
					/* if already defined as horiz, make it 4 or 8way */
					else if (strcmp(control[cjoy].Xway,"joy2way") == 0)
					{
						if (field->way == 4)
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
					if (control[cdoublejoy].Xway == NULL)
						control[cdoublejoy].Xway = "vdoublejoy2way";
					else if (strcmp(control[cdoublejoy].Xway,"vdoublejoy2way") == 0)
						;
					/* if already defined as horiz, make it 4 or 8 way */
					else if (strcmp(control[cdoublejoy].Xway,"doublejoy2way") == 0)
					{
						if (field->way == 4)
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
					if (control[cdoublejoy].Xway == NULL)
						control[cdoublejoy].Xway = "doublejoy2way";
					else if (strcmp(control[cdoublejoy].Xway,"doublejoy2way") == 0)
						;
					/* if already defined as vertical, make it 4 or 8 way */
					else if (strcmp(control[cdoublejoy].Xway,"vdoublejoy2way") == 0)
					{
						if (field->way == 4)
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
				case IPT_BUTTON2:
				case IPT_BUTTON3:
				case IPT_BUTTON4:
				case IPT_BUTTON5:
				case IPT_BUTTON6:
				case IPT_BUTTON7:
				case IPT_BUTTON8:
				case IPT_BUTTON9:
				case IPT_BUTTON10:
				case IPT_BUTTON11:
				case IPT_BUTTON12:
				case IPT_BUTTON13:
				case IPT_BUTTON14:
				case IPT_BUTTON15:
				case IPT_BUTTON16:
					nbutton = MAX(nbutton, field->type - IPT_BUTTON1 + 1);
					break;

				case IPT_COIN1:
				case IPT_COIN2:
				case IPT_COIN3:
				case IPT_COIN4:
				case IPT_COIN5:
				case IPT_COIN6:
				case IPT_COIN7:
				case IPT_COIN8:
					ncoin = MAX(ncoin, field->type - IPT_COIN1 + 1);

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

				if (field->min)
					control[analogcontrol].min = field->min;
				if (field->max)
					control[analogcontrol].max = field->max;
				if (field->sensitivity)
					control[analogcontrol].sensitivity = field->sensitivity;
				if (field->delta)
					control[analogcontrol].keydelta = field->delta;
				if (field->flags & ANALOG_FLAG_REVERSE)
					control[analogcontrol].reverse = 1;

				analogcontrol = 0;
			}
		}

	fprintf(out, "\t\t<input");
	fprintf(out, " players=\"%d\"", nplayer);
	if (nbutton != 0)
		fprintf(out, " buttons=\"%d\"", nbutton);
	if (ncoin != 0)
		fprintf(out, " coins=\"%d\"", ncoin);
	if (service != NULL)
		fprintf(out, " service=\"%s\"", xml_normalize_string(service));
	if (tilt != NULL)
		fprintf(out, " tilt=\"%s\"", xml_normalize_string(tilt));
	fprintf(out, ">\n");

	for (i = 0; i < ENDCONTROLTYPES; i++)
	{
		if (control[i].Xway != NULL)
			fprintf(out, "\t\t\t<control type=\"%s\"/>\n", xml_normalize_string(control[i].Xway));
		if (control[i].analog)
		{
			fprintf(out, "\t\t\t<control type=\"%s\"", xml_normalize_string(control_types[i]));
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
}


/*-------------------------------------------------
    print_game_bios - print the BIOS set for a
    game
-------------------------------------------------*/

static void print_game_bios(FILE *out, const game_driver *game)
{
	const rom_entry *rom;

	/* skip if no ROMs */
	if (game->rom == NULL)
		return;

	/* iterate over ROM entries and look for BIOSes */
	for (rom = game->rom; !ROMENTRY_ISEND(rom); rom++)
		if (ROMENTRY_ISSYSTEM_BIOS(rom))
		{
			const char *name = ROM_GETHASHDATA(rom);
			const char *description = name + strlen(name) + 1;

			/* output extracted name and descriptions */
			fprintf(out, "\t\t<biosset");
			fprintf(out, " name=\"%s\"", xml_normalize_string(name));
			fprintf(out, " description=\"%s\"", xml_normalize_string(description));
			if (ROM_GETBIOSFLAGS(rom) == 1)
				fprintf(out, " default=\"yes\"");
			fprintf(out, "/>\n");
		}
}


/*-------------------------------------------------
    print_game_rom - print the roms section of
    the XML output
-------------------------------------------------*/

static void print_game_rom(FILE *out, const game_driver *game)
{
	const game_driver *clone_of = driver_get_clone(game);
	int rom_type;

	/* if no roms, just exit early */
	if (game->rom == NULL)
		return;

	/* iterate over 3 different ROM "types": BIOS, ROMs, DISKs */
	for (rom_type = 0; rom_type < 3; rom_type++)
	{
		const rom_entry *region;

		/* iterate first through regions */
		for (region = rom_first_region(game); region != NULL; region = rom_next_region(region))
		{
			int is_disk = ROMREGION_ISDISKDATA(region);
			const rom_entry *rom;

			/* disk regions only work for disks */
			if ((is_disk && rom_type != 2) || (!is_disk && rom_type == 2))
				continue;

			/* iterate through ROM entries */
			for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
			{
				int is_bios = ROM_GETBIOSFLAGS(rom);
				const char *name = ROM_GETNAME(rom);
				int offset = ROM_GETOFFSET(rom);
				const rom_entry *parent_rom = NULL;
				const rom_entry *chunk;
				char bios_name[100];
				int length;

				/* BIOS ROMs only apply to bioses */
				if ((is_bios && rom_type != 0) || (!is_bios && rom_type == 0))
					continue;

				/* compute the total length of all chunks */
				length = 0;
				for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
					length += ROM_GETLENGTH(chunk);

				/* if we have a valid ROM and we are a clone, see if we can find the parent ROM */
				if (!ROM_NOGOODDUMP(rom) && clone_of != NULL)
				{
					const rom_entry *pregion, *prom;

					/* scan the clone_of ROM for a matching ROM entry */
					for (pregion = rom_first_region(clone_of); pregion != NULL; pregion = rom_next_region(pregion))
						for (prom = rom_first_file(pregion); prom != NULL; prom = rom_next_file(prom))
							if (hash_data_is_equal(ROM_GETHASHDATA(rom), ROM_GETHASHDATA(prom), 0))
							{
								parent_rom = prom;
								break;
							}
				}

				/* scan for a BIOS name */
				bios_name[0] = 0;
				if (!is_disk && is_bios)
				{
					const rom_entry *brom;

					/* scan backwards through the ROM entries */
					for (brom = rom - 1; brom != game->rom; brom--)
						if (ROMENTRY_ISSYSTEM_BIOS(brom))
						{
							strcpy(bios_name, ROM_GETHASHDATA(brom));
							break;
						}
				}

				/* opening tag */
				if (!is_disk)
					fprintf(out, "\t\t<rom");
				else
					fprintf(out, "\t\t<disk");

				/* add name, merge, bios, and size tags */
				if (name != NULL && name[0] != 0)
					fprintf(out, " name=\"%s\"", xml_normalize_string(name));
				if (parent_rom != NULL)
					fprintf(out, " merge=\"%s\"", xml_normalize_string(ROM_GETNAME(parent_rom)));
				if (bios_name[0] != 0)
					fprintf(out, " bios=\"%s\"", xml_normalize_string(bios_name));
				if (!is_disk)
					fprintf(out, " size=\"%d\"", length);

				/* dump checksum information only if there is a known dump */
				if (!hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_NO_DUMP))
				{
					char checksum[HASH_BUF_SIZE];
					int hashtype;

					/* iterate over hash function types and print out their values */
					for (hashtype = 0; hashtype < HASH_NUM_FUNCTIONS; hashtype++)
						if (hash_data_extract_printable_checksum(ROM_GETHASHDATA(rom), 1 << hashtype, checksum))
							fprintf(out, " %s=\"%s\"", hash_function_name(1 << hashtype), checksum);
				}

				/* append a region name */
				switch (ROMREGION_GETTYPE(region))
				{
					case REGION_CPU1: 	fprintf(out, " region=\"cpu1\"");	break;
					case REGION_CPU2: 	fprintf(out, " region=\"cpu2\"");	break;
					case REGION_CPU3: 	fprintf(out, " region=\"cpu3\"");	break;
					case REGION_CPU4: 	fprintf(out, " region=\"cpu4\"");	break;
					case REGION_CPU5: 	fprintf(out, " region=\"cpu5\"");	break;
					case REGION_CPU6: 	fprintf(out, " region=\"cpu6\"");	break;
					case REGION_CPU7: 	fprintf(out, " region=\"cpu7\"");	break;
					case REGION_CPU8: 	fprintf(out, " region=\"cpu8\"");	break;
					case REGION_GFX1: 	fprintf(out, " region=\"gfx1\"");	break;
					case REGION_GFX2: 	fprintf(out, " region=\"gfx2\"");	break;
					case REGION_GFX3: 	fprintf(out, " region=\"gfx3\"");	break;
					case REGION_GFX4: 	fprintf(out, " region=\"gfx4\"");	break;
					case REGION_GFX5: 	fprintf(out, " region=\"gfx5\"");	break;
					case REGION_GFX6: 	fprintf(out, " region=\"gfx6\"");	break;
					case REGION_GFX7: 	fprintf(out, " region=\"gfx7\"");	break;
					case REGION_GFX8: 	fprintf(out, " region=\"gfx8\"");	break;
					case REGION_PROMS: 	fprintf(out, " region=\"proms\"");	break;
					case REGION_PLDS: 	fprintf(out, " region=\"plds\"");	break;
					case REGION_SOUND1: fprintf(out, " region=\"sound1\"");	break;
					case REGION_SOUND2: fprintf(out, " region=\"sound2\"");	break;
					case REGION_SOUND3: fprintf(out, " region=\"sound3\"");	break;
					case REGION_SOUND4: fprintf(out, " region=\"sound4\"");	break;
					case REGION_SOUND5: fprintf(out, " region=\"sound5\"");	break;
					case REGION_SOUND6: fprintf(out, " region=\"sound6\"");	break;
					case REGION_SOUND7: fprintf(out, " region=\"sound7\"");	break;
					case REGION_SOUND8: fprintf(out, " region=\"sound8\"");	break;
					case REGION_USER1: 	fprintf(out, " region=\"user1\"");	break;
					case REGION_USER2: 	fprintf(out, " region=\"user2\"");	break;
					case REGION_USER3: 	fprintf(out, " region=\"user3\"");	break;
					case REGION_USER4: 	fprintf(out, " region=\"user4\"");	break;
					case REGION_USER5: 	fprintf(out, " region=\"user5\"");	break;
					case REGION_USER6: 	fprintf(out, " region=\"user6\"");	break;
					case REGION_USER7: 	fprintf(out, " region=\"user7\"");	break;
					case REGION_USER8: 	fprintf(out, " region=\"user8\"");	break;
					case REGION_USER9: 	fprintf(out, " region=\"user9\"");	break;
					case REGION_USER10: fprintf(out, " region=\"user10\"");	break;
					case REGION_USER11: fprintf(out, " region=\"user11\"");	break;
					case REGION_USER12: fprintf(out, " region=\"user12\"");	break;
					case REGION_USER13: fprintf(out, " region=\"user13\"");	break;
					case REGION_USER14: fprintf(out, " region=\"user14\"");	break;
					case REGION_USER15: fprintf(out, " region=\"user15\"");	break;
					case REGION_USER16: fprintf(out, " region=\"user16\"");	break;
					case REGION_USER17: fprintf(out, " region=\"user17\"");	break;
					case REGION_USER18: fprintf(out, " region=\"user18\"");	break;
					case REGION_USER19: fprintf(out, " region=\"user19\"");	break;
					case REGION_USER20: fprintf(out, " region=\"user20\"");	break;
					case REGION_DISKS: 	fprintf(out, " region=\"disks\"");	break;
					default:	 		fprintf(out, " region=\"0x%x\"", (int)ROMREGION_GETTYPE(region)); break;
				}

				/* add nodump/baddump flags */
				if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_NO_DUMP))
					fprintf(out, " status=\"nodump\"");
				if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP))
					fprintf(out, " status=\"baddump\"");

				/* for non-disk entries, print dispose flag and offset */
				if (!is_disk)
				{
					if (ROMREGION_GETFLAGS(region) & ROMREGION_DISPOSE)
						fprintf(out, " dispose=\"yes\"");
					fprintf(out, " offset=\"%x\"", offset);
				}

				/* for disk entries, add the disk index */
				else
					fprintf(out, " index=\"%x\"", DISK_GETINDEX(rom));
				fprintf(out, "/>\n");
			}
		}
	}
}


/*-------------------------------------------------
    print_game_sampleof - print the 'sampleof'
    attribute, if appropriate
-------------------------------------------------*/

static void print_game_sampleof(FILE *out, const game_driver *game, const machine_config *config)
{
#if (HAS_SAMPLES)
	int sndnum;

	for (sndnum = 0; sndnum < ARRAY_LENGTH(config->sound) && config->sound[sndnum].type != SOUND_DUMMY; sndnum++)
		if (config->sound[sndnum].type == SOUND_SAMPLES)
		{
			const char *const *samplenames = ((const struct Samplesinterface *)config->sound[sndnum].config)->samplenames;
			if (samplenames != NULL)
			{
				int sampnum;

				/* iterate over sample names */
				for (sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
				{
					const char *cursampname = samplenames[sampnum];

					/* only output sampleof if different from the game name */
					if (cursampname[0] == '*' && strcmp(cursampname + 1, game->name) != 0)
						fprintf(out, " sampleof=\"%s\"", xml_normalize_string(cursampname + 1));
				}
			}
		}
#endif
}


/*-------------------------------------------------
    print_game_sample - print a list of all
    samples referenced by a game_driver
-------------------------------------------------*/

static void print_game_sample(FILE *out, const game_driver *game, const machine_config *config)
{
#if (HAS_SAMPLES)
	int sndnum;

	/* iterate over sound chips looking for samples */
	for (sndnum = 0; sndnum < ARRAY_LENGTH(config->sound) && config->sound[sndnum].type != SOUND_DUMMY; sndnum++)
		if (config->sound[sndnum].type == SOUND_SAMPLES)
		{
			const char *const *samplenames = ((const struct Samplesinterface *)config->sound[sndnum].config)->samplenames;
			if (samplenames != NULL)
			{
				int sampnum;

				/* iterate over sample names */
				for (sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
				{
					const char *cursampname = samplenames[sampnum];
					int dupnum;

					/* ignore the special '*' samplename */
					if (sampnum == 0 && cursampname[0] == '*')
						continue;

					/* filter out duplicates */
					for (dupnum = 0; dupnum < sampnum; dupnum++)
						if (strcmp(samplenames[dupnum], cursampname) == 0)
							break;
					if (dupnum < sampnum)
						continue;

					/* output the sample name */
					fprintf(out, "\t\t<sample name=\"%s\"/>\n", xml_normalize_string(cursampname));
				}
			}
		}
#endif
}


/*-------------------------------------------------
    print_game_chips - print a list of CPU and
    sound chips used by a game
-------------------------------------------------*/

static void print_game_chips(FILE *out, const game_driver *game, const machine_config *config)
{
	int chipnum;

	/* iterate over CPUs */
	for (chipnum = 0; chipnum < ARRAY_LENGTH(config->cpu); chipnum++)
		if (config->cpu[chipnum].type != CPU_DUMMY)
		{
			fprintf(out, "\t\t<chip");
			fprintf(out, " type=\"cpu\"");
			fprintf(out, " name=\"%s\"", xml_normalize_string(cputype_name(config->cpu[chipnum].type)));
			fprintf(out, " clock=\"%d\"", config->cpu[chipnum].clock);
			fprintf(out, "/>\n");
		}

	/* iterate over sound chips */
	for (chipnum = 0; chipnum < ARRAY_LENGTH(config->sound); chipnum++)
		if (config->sound[chipnum].type != SOUND_DUMMY)
		{
			fprintf(out, "\t\t<chip");
			fprintf(out, " type=\"audio\"");
			fprintf(out, " name=\"%s\"", xml_normalize_string(sndtype_name(config->sound[chipnum].type)));
			if (config->sound[chipnum].clock != 0)
				fprintf(out, " clock=\"%d\"", config->sound[chipnum].clock);
			fprintf(out, "/>\n");
		}
}


/*-------------------------------------------------
    print_game_display - print a list of all the
    displays
-------------------------------------------------*/

static void print_game_display(FILE *out, const game_driver *game, const machine_config *config)
{
	const device_config *screen;

	/* iterate over screens */
	for (screen = video_screen_first(config); screen != NULL; screen = video_screen_next(screen))
	{
		const screen_config *scrconfig = screen->inline_config;

		fprintf(out, "\t\t<display");

		switch (scrconfig->type)
		{
			case SCREEN_TYPE_RASTER:	fprintf(out, " type=\"raster\"");	break;
			case SCREEN_TYPE_VECTOR:	fprintf(out, " type=\"vector\"");	break;
			case SCREEN_TYPE_LCD:		fprintf(out, " type=\"lcd\"");		break;
			default:					fprintf(out, " type=\"unknown\"");	break;
		}

		/* output the orientation as a string */
		switch (game->flags & ORIENTATION_MASK)
		{
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
		if (scrconfig->type != SCREEN_TYPE_VECTOR)
		{
			int dx = scrconfig->visarea.max_x - scrconfig->visarea.min_x + 1;
			int dy = scrconfig->visarea.max_y - scrconfig->visarea.min_y + 1;

			fprintf(out, " width=\"%d\"", dx);
			fprintf(out, " height=\"%d\"", dy);
		}

		/* output refresh rate */
		fprintf(out, " refresh=\"%f\"", ATTOSECONDS_TO_HZ(scrconfig->refresh));

		/* output raw video parameters only for games that are not vector */
		/* and had raw parameters specified                               */
		if ((scrconfig->type != SCREEN_TYPE_VECTOR) && !scrconfig->oldstyle_vblank_supplied)
		{
			int pixclock = scrconfig->width * scrconfig->height * ATTOSECONDS_TO_HZ(scrconfig->refresh);

			fprintf(out, " pixclock=\"%d\"", pixclock);
			fprintf(out, " htotal=\"%d\"", scrconfig->width);
			fprintf(out, " hbend=\"%d\"", scrconfig->visarea.min_x);
			fprintf(out, " hbstart=\"%d\"", scrconfig->visarea.max_x+1);
			fprintf(out, " vtotal=\"%d\"", scrconfig->height);
			fprintf(out, " vbend=\"%d\"", scrconfig->visarea.min_y);
			fprintf(out, " vbstart=\"%d\"", scrconfig->visarea.max_y+1);
		}
		fprintf(out, " />\n");
	}
}


/*-------------------------------------------------
    print_game_sound - print a list of all the
    displays
-------------------------------------------------*/

static void print_game_sound(FILE *out, const game_driver *game, const machine_config *config)
{
	int speakers = speaker_output_count(config);
	int has_sound = FALSE;
	int sndnum;

	/* see if we have any sound chips to report */
	for (sndnum = 0; sndnum < ARRAY_LENGTH(config->sound); sndnum++)
		if (config->sound[sndnum].type != SOUND_DUMMY)
		{
			has_sound = TRUE;
			break;
		}

	/* if we have sound, count the number of speakers */
	if (!has_sound)
		speakers = 0;

	fprintf(out, "\t\t<sound channels=\"%d\"/>\n", speakers);
}


/*-------------------------------------------------
    print_game_driver - print driver status
-------------------------------------------------*/

static void print_game_driver(FILE *out, const game_driver *game, const machine_config *config)
{
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

	fprintf(out, " palettesize=\"%d\"", config->total_colors);

	fprintf(out, "/>\n");
}


/*-------------------------------------------------
    print_game_info - print the XML information
    for one particular game driver
-------------------------------------------------*/

static void print_game_info(FILE *out, const game_driver *game)
{
	const input_port_config *portconfig;
	const game_driver *clone_of;
	machine_config *config;
	const char *start;

	/* no action if not a game */
	if (game->flags & GAME_NO_STANDALONE)
		return;

	/* start tracking resources and allocate the machine and input configs */
	config = machine_config_alloc(game->machine_config);
#ifdef MESS
	/* temporary hook until MESS device transition is complete */
	mess_devices_setup(config, game);
#endif /* MESS */
	portconfig = input_port_config_alloc(game->ipt, NULL, 0);

	/* print the header and the game name */
	fprintf(out, "\t<" XML_TOP);
	fprintf(out, " name=\"%s\"", xml_normalize_string(game->name) );

	/* strip away any path information from the source_file and output it */
	start = strrchr(game->source_file, '/');
	if (start == NULL)
		start = strrchr(game->source_file, '\\');
	if (start == NULL)
		start = game->source_file - 1;
	fprintf(out, " sourcefile=\"%s\"", xml_normalize_string(start + 1));

	/* append bios and runnable flags */
	if (game->flags & GAME_IS_BIOS_ROOT)
		fprintf(out, " isbios=\"yes\"");
	if (game->flags & GAME_NO_STANDALONE)
		fprintf(out, " runnable=\"no\"");

	/* display clone information */
	clone_of = driver_get_clone(game);
	if (clone_of != NULL && !(clone_of->flags & GAME_IS_BIOS_ROOT))
		fprintf(out, " cloneof=\"%s\"", xml_normalize_string(clone_of->name));
	if (clone_of != NULL)
		fprintf(out, " romof=\"%s\"", xml_normalize_string(clone_of->name));

	/* display sample information and close the game tag */
	print_game_sampleof(out, game, config);
	fprintf(out, ">\n");

	/* output game description */
	if (game->description != NULL)
		fprintf(out, "\t\t<description>%s</description>\n", xml_normalize_string(game->description));

	/* print the year only if is a number */
	if (game->year != NULL && strspn(game->year, "0123456789") == strlen(game->year))
		fprintf(out, "\t\t<year>%s</year>\n", xml_normalize_string(game->year));

	/* print the manufacturer information */
	if (game->manufacturer != NULL)
		fprintf(out, "\t\t<manufacturer>%s</manufacturer>\n", xml_normalize_string(game->manufacturer));

	/* now print various additional information */
	print_game_bios(out, game);
	print_game_rom(out, game);
	print_game_sample(out, game, config);
	print_game_chips(out, game, config);
	print_game_display(out, game, config);
	print_game_sound(out, game, config);
	print_game_input(out, game, portconfig);
	print_game_switches(out, game, portconfig);
	print_game_driver(out, game, config);
#ifdef MESS
	print_game_device(out, game, config);
	print_game_ramoptions(out, game, config);
#endif /* MESS */

	/* close the topmost tag */
	fprintf(out, "\t</" XML_TOP ">\n");

	input_port_config_free(portconfig);
	machine_config_free(config);
}


/*-------------------------------------------------
    print_mame_xml - print the XML information
    for all known games
-------------------------------------------------*/

void print_mame_xml(FILE *out, const game_driver *const games[], const char *gamename)
{
	int drvnum;

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
		"\t\t\t<!ATTLIST display type (raster|vector|lcd|unknown) #REQUIRED>\n"
		"\t\t\t<!ATTLIST display rotate (0|90|180|270) #REQUIRED>\n"
		"\t\t\t<!ATTLIST display flipx (yes|no) \"no\">\n"
		"\t\t\t<!ATTLIST display width CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display height CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display refresh CDATA #REQUIRED>\n"
		"\t\t\t<!ATTLIST display pixclock CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display htotal CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display hbend CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display hbstart CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display vtotal CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display vbend CDATA #IMPLIED>\n"
		"\t\t\t<!ATTLIST display vbstart CDATA #IMPLIED>\n"
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

	for (drvnum = 0; games[drvnum] != NULL; drvnum++)
		if (mame_strwildcmp(gamename, games[drvnum]->name) == 0)
			print_game_info(out, games[drvnum]);

	fprintf(out, "</" XML_ROOT ">\n");
}
