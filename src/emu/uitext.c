/*********************************************************************

    uitext.c

    Functions used to retrieve text used by MAME, to aid in
    translation.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "uitext.h"

#ifdef MESS
extern const char *mess_default_text[];
#endif /* MESS */


lang_struct lang;

/* All entries in this table must match the enum ordering in "uitext.h" */
static const char *mame_default_text[] =
{
	APPNAME,
	/* copyright stuff */
	"Usage of emulators in conjunction with ROMs you don't own is forbidden by copyright law.",
	"IF YOU ARE NOT LEGALLY ENTITLED TO PLAY \"%s\" ON THIS EMULATOR, PRESS ESC.",
	"Otherwise, type OK or move the joystick left then right to continue",

	/* misc stuff */
	"Return to Main Menu",
	"Return to Prior Menu",
	"Press Any Key",
	"On",
	"Off",
	"NA",
	"OK",
	"INVALID",
	"(none)",
	"CPU",
	"Address",
	"Value",
	"Sound",
	"sound",
	"stereo",
	"Vector Game",
	"Screen Resolution",
	"Text",
	"Volume",
	"Relative",
	"ALL CHANNELS",
	"Brightness",
	"Contrast",
	"Gamma",
	"Vector Flicker",
	"Overclock",
	"ALL CPUS",
	HISTORYNAME " not available",

	/* special characters */
	"\xc2\xab",
	"\xc2\xbb",
	"(more)",
	"(more)",
	"",
	"",

	/* known problems */
	"There are known problems with this " GAMENOUN,
	"The colors aren't 100% accurate.",
	"The colors are completely wrong.",
	"The video emulation isn't 100% accurate.",
	"The sound emulation isn't 100% accurate.",
	"The game lacks sound.",
	"Screen flipping in cocktail mode is not supported.",
	"THIS " CAPGAMENOUN " DOESN'T WORK. You won't be able to make it work correctly.  Don't bother.",
	"The game has protection which isn't fully emulated.",
	"There are working clones of this game. They are:",
	"One or more ROMs/CHDs for this game are incorrect. The " GAMENOUN " may not run correctly.",
	"Type OK or move the joystick left then right to continue",

	/* main menu */
	"Input (general)",
	"Dip Switches",
	"Analog Controls",
	"Calibrate Joysticks",
	"Bookkeeping Info",

	"Input (this " CAPSTARTGAMENOUN")",
	CAPSTARTGAMENOUN " Information",
	CAPSTARTGAMENOUN " History",
	"Reset " CAPSTARTGAMENOUN,
	"Select New " CAPSTARTGAMENOUN,
	"Return to " CAPSTARTGAMENOUN,
	"Exit",

	"Cheat",
	"Memory Card",

	/* input */
	"Digital Speed",
	"Autocenter Speed",
	"Reverse",
	"Sensitivity",

	/* input groups */
	"User Interface",
	"Player 1 Controls",
	"Player 2 Controls",
	"Player 3 Controls",
	"Player 4 Controls",
	"Player 5 Controls",
	"Player 6 Controls",
	"Player 7 Controls",
	"Player 8 Controls",
	"Other Controls",
	"Return to Groups",

	/* stats */
	"Uptime",
	"Tickets dispensed",
	"Coin",
	"(locked)",

	/* memory card */
	"Card Number:",
	"Load Selected Card",
	"Eject Current Card",
	"Create New Card",
	"Error loading memory card",
	"Memory card loaded",
	"Memory card ejected",
	"Memory card created",
	"Error creating memory card",
	"(Card may already exist)",

	/* cheats */
	"Enable/Disable a Cheat",
	"Add/Edit a Cheat",
	"Start a New Cheat Search",
	"Continue Search",
	"View Last Results",
	"Restore Previous Results",
	"Configure Watchpoints",
	"General Help",
	"Options",
	"Reload Database",
	"Watchpoint",
	"Disabled",
	"Cheats",
	"Watchpoints",
	"More Info",
	"More Info for",
	"Name",
	"Description",
	"Activation Key",
	"Code",
	"Max",
	"Set",
	"Cheat conflict found: disabling",
	"Help not available yet",

	/* watchpoints */
	"Number of bytes",
	"Display Type",
	"Label Type",
	"Label",
	"X Position",
	"Y Position",
	"Watch",

	"Hex",
	"Decimal",
	"Binary",

	/* searching */
	"Lives (or another value)",
	"Timers (+/- some value)",
	"Energy (greater or less)",
	"Status (bits or flags)",
	"Slow But Sure (changed or not)",
	"Default Search Speed",
	"Fast",
	"Medium",
	"Slow",
	"Very Slow",
	"All Memory",
	"Select Memory Areas",
	"Matches found",
	"Search not initialized",
	"No previous values saved",
	"Previous values already restored",
	"Restoration successful",
	"Select a value",
	"All values saved",
	"One match found - added to list",

	/* refresh rate */
	"Refresh rate",
	"Decoding Graphics",

	"Video Options",
	"Screen #",
	"Rotate Clockwise",
	"Rotate Counter-clockwise",
	"Flip X",
	"Flip Y",

	"Driver Configuration",

	NULL
};



static const char **default_text[] =
{
	mame_default_text,
#ifdef MESS
	mess_default_text,
#endif /* MESS */
	NULL
};



static const char **trans_text;


int uistring_init (mame_file *langfile)
{
	/*
        TODO: This routine needs to do several things:
            - load an external font if needed
            - determine the number of characters in the font
            - deal with multibyte languages

    */

	int i, j, str;
	char curline[255];
	char section[255] = "\0";
	char *ptr;
	int string_count;

	/* count the total amount of strings */
	string_count = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			string_count++;
	}

	/* allocate the translated text array, and set defaults */
	trans_text = auto_malloc(sizeof(const char *) * string_count);

	/* copy in references to all of the strings */
	str = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			trans_text[str++] = default_text[i][j];
	}

	memset(&lang, 0, sizeof(lang));

	/* if no language file, exit */
	if (!langfile)
		return 0;

	while (mame_fgets (curline, sizeof(curline) / sizeof(curline[0]), langfile) != NULL)
	{
		/* Ignore commented and blank lines */
		if (curline[0] == ';') continue;
		if (curline[0] == '\n') continue;
		if (curline[0] == '\r') continue;

		if (curline[0] == '[')
		{
			ptr = strtok (&curline[1], "]");
			/* Found a section, indicate as such */
			strcpy (section, ptr);

			/* Skip to the next line */
			continue;
		}

		/* Parse the LangInfo section */
		if (strcmp (section, "LangInfo") == 0)
		{
			ptr = strtok (curline, "=");
			if (strcmp (ptr, "Version") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				sscanf (ptr, "%d", &lang.version);
			}
			else if (strcmp (ptr, "Language") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				strcpy (lang.langname, ptr);
			}
			else if (strcmp (ptr, "Author") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				strcpy (lang.author, ptr);
			}
			else if (strcmp (ptr, "Font") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				strcpy (lang.fontname, ptr);
			}
		}

		/* Parse the Strings section */
		if (strcmp (section, "Strings") == 0)
		{
			/* Get all text up to the first line ending */
			ptr = strtok (curline, "\n\r");

			/* Find a matching default string */
			str = 0;
			for (i = 0; default_text[i]; i++)
			{
				for (j = 0; default_text[i][j]; j++)
				{
					if (strcmp (curline, default_text[i][j]) == 0)
				{
					char transline[255];

					/* Found a match, read next line as the translation */
					mame_fgets (transline, 255, langfile);

					/* Get all text up to the first line ending */
					ptr = strtok (transline, "\n\r");

					/* Allocate storage and copy the string */
						trans_text[str] = auto_strdup(transline);
					}
					str++;
				}
			}
		}
	}

	/* indicate success */
	return 0;
}



const char * ui_getstring (int string_num)
{
		return trans_text[string_num];
}
