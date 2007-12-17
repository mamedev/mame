/***************************************************************************

    uitext.h

    Functions used to retrieve text used by MAME, to aid in
    translation.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __uitext_H__
#define __uitext_H__

#include "mamecore.h"

/* Important: this must match the default_text list in uitext.c! */
enum
{
	UI_mame = 0,

	/* copyright stuff */
	UI_copyright1,
	UI_copyright2,
	UI_copyright3,

	/* misc menu stuff */
	UI_returntomain,
	UI_returntoprior,
	UI_anykey,
	UI_on,
	UI_off,
	UI_NA,
	UI_OK,
	UI_INVALID,
	UI_none,
	UI_cpu,
	UI_address,
	UI_value,
	UI_sound,
	UI_sound_lc, /* lower-case version */
	UI_stereo,
	UI_vectorgame,
	UI_screenres,
	uitext,
	UI_volume,
	UI_relative,
	UI_allchannels,
	UI_brightness,
	UI_contrast,
	UI_gamma,
	UI_vectorflicker,
	UI_overclock,
	UI_allcpus,
	UI_historymissing,

	/* special characters */
	UI_leftarrow,
	UI_rightarrow,
	UI_uparrow,
	UI_downarrow,
	UI_lefthilight,
	UI_righthilight,

	/* warnings */
	UI_knownproblems,
	UI_imperfectcolors,
	UI_wrongcolors,
	UI_imperfectgraphics,
	UI_imperfectsound,
	UI_nosound,
	UI_nococktail,
	UI_brokengame,
	UI_brokenprotection,
	UI_workingclones,
	UI_incorrectroms,
	UI_typeok,

	/* main menu */
	UI_inputgeneral,
	UI_dipswitches,
	UI_analogcontrols,
	UI_calibrate,
	UI_bookkeeping,
	UI_inputspecific,
	UI_gameinfo,
	UI_history,
	UI_resetgame,
	UI_selectgame,
	UI_returntogame,
	UI_exit,
	UI_cheat,
	UI_memorycard,

	/* input stuff */
	UI_keyjoyspeed,
	UI_centerspeed,
	UI_reverse,
	UI_sensitivity,

	/* input groups */
	UI_uigroup,
	UI_p1group,
	UI_p2group,
	UI_p3group,
	UI_p4group,
	UI_p5group,
	UI_p6group,
	UI_p7group,
	UI_p8group,
	UI_othergroup,
	UI_returntogroup,

	/* stats */
	UI_totaltime,
	UI_tickets,
	UI_coin,
	UI_locked,

	/* memory card */
	UI_selectcard,
	UI_loadcard,
	UI_ejectcard,
	UI_createcard,
	UI_loadfailed,
	UI_loadok,
	UI_cardejected,
	UI_cardcreated,
	UI_cardcreatedfailed,
	UI_cardcreatedfailed2,

	/* cheat stuff */
	UI_enablecheat,
	UI_addeditcheat,
	UI_startcheat,
	UI_continuesearch,
	UI_viewresults,
	UI_restoreresults,
	UI_memorywatch,
	UI_generalhelp,
	UI_options,
	UI_reloaddatabase,
	UI_watchpoint,
	UI_disabled,
	UI_cheats,
	UI_watchpoints,
	UI_moreinfo,
	UI_moreinfoheader,
	UI_cheatname,
	UI_cheatdescription,
	UI_cheatactivationkey,
	UI_code,
	UI_max,
	UI_set,
	UI_conflict_found,
	UI_no_help_available,

	/* watchpoint stuff */
	UI_watchlength,
	UI_watchdisplaytype,
	UI_watchlabeltype,
	UI_watchlabel,
	UI_watchx,
	UI_watchy,
	UI_watch,

	UI_hex,
	UI_decimal,
	UI_binary,

	/* search stuff */
	UI_search_lives,
	UI_search_timers,
	UI_search_energy,
	UI_search_status,
	UI_search_slow,
	UI_search_speed,
	UI_search_speed_fast,
	UI_search_speed_medium,
	UI_search_speed_slow,
	UI_search_speed_veryslow,
	UI_search_speed_allmemory,
	UI_search_select_memory_areas,
	UI_search_matches_found,
	UI_search_noinit,
	UI_search_nosave,
	UI_search_done,
	UI_search_OK,
	UI_search_select_value,
	UI_search_all_values_saved,
	UI_search_one_match_found_added,

	/* refresh rate */
	UI_refresh_rate,
	UI_decoding_gfx,

	UI_video,
	UI_screen,
	UI_rotate_clockwise,
	UI_rotate_counterclockwise,
	UI_flip_x,
	UI_flip_y,

	UI_configuration,

	UI_last_mame_entry
};

#ifdef MESS
#include "muitext.h"
#endif

struct _lang_struct
{
	int version;
	int multibyte;			/* UNUSED: 1 if this is a multibyte font/language */
	UINT8 *fontdata;		/* pointer to the raw font data to be decoded */
	UINT16 fontglyphs;		/* total number of glyps in the external font - 1 */
	char langname[255];
	char fontname[255];
	char author[255];
};
typedef struct _lang_struct lang_struct;

int uistring_init (mame_file *language_file);

const char * ui_getstring (int string_num);

#endif /* __uitext_H__ */

