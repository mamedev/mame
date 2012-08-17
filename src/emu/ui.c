/*********************************************************************

    ui.c

    Functions used to handle MAME's user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "video/vector.h"
#include "machine/laserdsc.h"
#include "render.h"
#include "cheat.h"
#include "rendfont.h"
#include "ui.h"
#include "uiinput.h"
#include "uimain.h"
#include "uigfx.h"
#include <ctype.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	LOADSAVE_NONE,
	LOADSAVE_LOAD,
	LOADSAVE_SAVE
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

/* list of natural keyboard keys that are not associated with UI_EVENT_CHARs */
static const input_item_id non_char_keys[] =
{
	ITEM_ID_ESC,
	ITEM_ID_F1,
	ITEM_ID_F2,
	ITEM_ID_F3,
	ITEM_ID_F4,
	ITEM_ID_F5,
	ITEM_ID_F6,
	ITEM_ID_F7,
	ITEM_ID_F8,
	ITEM_ID_F9,
	ITEM_ID_F10,
	ITEM_ID_F11,
	ITEM_ID_F12,
	ITEM_ID_NUMLOCK,
	ITEM_ID_0_PAD,
	ITEM_ID_1_PAD,
	ITEM_ID_2_PAD,
	ITEM_ID_3_PAD,
	ITEM_ID_4_PAD,
	ITEM_ID_5_PAD,
	ITEM_ID_6_PAD,
	ITEM_ID_7_PAD,
	ITEM_ID_8_PAD,
	ITEM_ID_9_PAD,
	ITEM_ID_DEL_PAD,
	ITEM_ID_PLUS_PAD,
	ITEM_ID_MINUS_PAD,
	ITEM_ID_INSERT,
	ITEM_ID_DEL,
	ITEM_ID_HOME,
	ITEM_ID_END,
	ITEM_ID_PGUP,
	ITEM_ID_PGDN,
	ITEM_ID_UP,
	ITEM_ID_DOWN,
	ITEM_ID_LEFT,
	ITEM_ID_RIGHT,
	ITEM_ID_PAUSE,
	ITEM_ID_CANCEL
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* font for rendering */
static render_font *ui_font;

/* current UI handler */
static UINT32 (*ui_handler_callback)(running_machine &, render_container *, UINT32);
static UINT32 ui_handler_param;

/* flag to track single stepping */
static int single_step;

/* FPS counter display */
static int showfps;
static osd_ticks_t showfps_end;

/* profiler display */
static int show_profiler;

/* popup text display */
static osd_ticks_t popup_text_end;

/* messagebox buffer */
static astring messagebox_text;
static rgb_t messagebox_backcolor;

/* slider info */
static slider_state *slider_list;
static slider_state *slider_current;

/* natural keyboard info */
static int ui_use_natural_keyboard;
static UINT8 non_char_keys_down[(ARRAY_LENGTH(non_char_keys) + 7) / 8];


static render_texture *ui_mouse_arrow_texture;
static bool ui_mouse_show;
/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_exit(running_machine &machine);

/* text generators */
static astring &disclaimer_string(running_machine &machine, astring &buffer);
static astring &warnings_string(running_machine &machine, astring &buffer);

/* UI handlers */
static UINT32 handler_messagebox(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_messagebox_ok(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_messagebox_anykey(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_ingame(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_load_save(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_confirm_quit(running_machine &machine, render_container *container, UINT32 state);

/* slider controls */
static slider_state *slider_alloc(running_machine &machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg);
static slider_state *slider_init(running_machine &machine);
static INT32 slider_volume(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_mixervol(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_adjuster(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overclock(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_refresh(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_brightness(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_contrast(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_gamma(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_xscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_yscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_xoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_yoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overxscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overyscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overxoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overyoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_flicker(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_beam(running_machine &machine, void *arg, astring *string, INT32 newval);
static char *slider_get_screen_desc(screen_device &screen);
#ifdef MAME_DEBUG
static INT32 slider_crossscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_crossoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
#endif


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ui_set_handler - set a callback/parameter
    pair for the current UI handler
-------------------------------------------------*/

INLINE UINT32 ui_set_handler(UINT32 (*callback)(running_machine &, render_container *, UINT32), UINT32 param)
{
	ui_handler_callback = callback;
	ui_handler_param = param;
	return param;
}


/*-------------------------------------------------
    is_breakable_char - is a given unicode
    character a possible line break?
-------------------------------------------------*/

INLINE int is_breakable_char(unicode_char ch)
{
	/* regular spaces and hyphens are breakable */
	if (ch == ' ' || ch == '-')
		return TRUE;

	/* In the following character sets, any character is breakable:
        Hiragana (3040-309F)
        Katakana (30A0-30FF)
        Bopomofo (3100-312F)
        Hangul Compatibility Jamo (3130-318F)
        Kanbun (3190-319F)
        Bopomofo Extended (31A0-31BF)
        CJK Strokes (31C0-31EF)
        Katakana Phonetic Extensions (31F0-31FF)
        Enclosed CJK Letters and Months (3200-32FF)
        CJK Compatibility (3300-33FF)
        CJK Unified Ideographs Extension A (3400-4DBF)
        Yijing Hexagram Symbols (4DC0-4DFF)
        CJK Unified Ideographs (4E00-9FFF) */
	if (ch >= 0x3040 && ch <= 0x9fff)
		return TRUE;

	/* Hangul Syllables (AC00-D7AF) are breakable */
	if (ch >= 0xac00 && ch <= 0xd7af)
		return TRUE;

	/* CJK Compatibility Ideographs (F900-FAFF) are breakable */
	if (ch >= 0xf900 && ch <= 0xfaff)
		return TRUE;

	return FALSE;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ui_init - set up the user interface
-------------------------------------------------*/
static const UINT32 mouse_bitmap[] = {
	0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x09a46f30,0x81ac7c43,0x24af8049,0x00ad7d45,0x00a8753a,0x00a46f30,0x009f6725,0x009b611c,0x00985b14,0x0095560d,0x00935308,0x00915004,0x00904e02,0x008f4e01,0x008f4d00,0x008f4d00,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x00a16a29,0xa2aa783d,0xffbb864a,0xc0b0824c,0x5aaf7f48,0x09ac7b42,0x00a9773c,0x00a67134,0x00a26b2b,0x009e6522,0x009a5e19,0x00965911,0x0094550b,0x00925207,0x00915004,0x008f4e01,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x009a5e18,0x39a06827,0xffb97c34,0xffe8993c,0xffc88940,0xedac7c43,0x93ad7c44,0x2dac7c43,0x00ab793f,0x00a87438,0x00a46f30,0x00a06827,0x009c611d,0x00985c15,0x0095570e,0x00935309,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x00935308,0x00965810,0xcc9a5e19,0xffe78a21,0xfffb9929,0xfff49931,0xffd88e39,0xffb9813f,0xc9ac7c43,0x66ad7c44,0x0cac7a41,0x00a9773c,0x00a67134,0x00a26b2b,0x009e6522,0x009a5e19,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4e01,0x00904e02,0x60925106,0xffba670a,0xfff88b11,0xfff98f19,0xfff99422,0xfff9982b,0xffe89434,0xffc9883c,0xf3ac7a41,0x9cad7c44,0x39ac7c43,0x00ab7a40,0x00a87539,0x00a56f31,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008e4d00,0x008e4d00,0x098e4d00,0xea8f4d00,0xffee7f03,0xfff68407,0xfff6870d,0xfff78b15,0xfff78f1d,0xfff79426,0xfff49730,0xffd98d38,0xffbc823f,0xd2ac7c43,0x6fad7c44,0x12ac7b42,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008e4d00,0x008e4d00,0x008e4c00,0x8a8e4c00,0xffc46800,0xfff37e00,0xfff37f02,0xfff38106,0xfff3830a,0xfff48711,0xfff48b19,0xfff58f21,0xfff5942b,0xffe79134,0xffcb863b,0xf9ac7a41,0xa5ac7c43,0x3fac7c43,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008e4d00,0x008e4d00,0x008e4c00,0x218d4c00,0xfc8e4c00,0xffee7a00,0xfff07c00,0xfff17c00,0xfff17d02,0xfff17e04,0xfff18008,0xfff2830d,0xfff28614,0xfff38a1c,0xfff38f25,0xfff2932e,0xffd98b37,0xffbc813e,0xdbac7c43,0x78ad7c44,0x15ac7b42,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008e4d00,0x008e4d00,0x008e4d00,0x008e4c00,0xb18d4c00,0xffcf6b00,0xffed7900,0xffed7900,0xffee7900,0xffee7a01,0xffee7a01,0xffee7b03,0xffee7c06,0xffef7e0a,0xffef8110,0xfff08618,0xfff08a20,0xfff18f2a,0xffe78f33,0xffcc863b,0xfcab7a40,0xaeac7c43,0x4bac7c43,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x488d4c00,0xffa85800,0xffe97500,0xffea7600,0xffea7600,0xffeb7600,0xffeb7600,0xffeb7600,0xffeb7701,0xffeb7702,0xffeb7804,0xffec7a07,0xffec7d0d,0xffec8013,0xffed851c,0xffee8a25,0xffee8f2e,0xffd98937,0xffbe813d,0xe4ab7a40,0x81ab7a40,0x1ba9763b,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x008d4c00,0xdb8d4c00,0xffd86c00,0xffe77300,0xffe77300,0xffe87300,0xffe87300,0xffe87300,0xffe87300,0xffe87300,0xffe87401,0xffe87401,0xffe87503,0xffe97606,0xffe9780a,0xffe97c10,0xffea7f16,0xffeb831d,0xffeb8623,0xffe48426,0xffc67725,0xffa5661f,0xb7985c15,0x54935309,0x038e4d00,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008e4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x6f8d4c00,0xffb25b00,0xffe36f00,0xffe47000,0xffe47000,0xffe57000,0xffe57000,0xffe57000,0xffe57000,0xffe57000,0xffe57000,0xffe57000,0xffe57000,0xffe57101,0xffe57000,0xffe47000,0xffe16e00,0xffde6c00,0xffd86900,0xffd06600,0xffc76200,0xffaa5500,0xff8a4800,0xea743f00,0x5a7a4200,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x0f8d4c00,0xf38d4c00,0xffdc6a00,0xffe16d00,0xffe16d00,0xffe26d00,0xffe26d00,0xffe26d00,0xffe26d00,0xffe26d00,0xffe16d00,0xffe06c00,0xffde6b00,0xffd96900,0xffd16500,0xffc76000,0xffb95900,0xffab5200,0xff9c4b00,0xff894300,0xff6b3600,0xf9512c00,0xa5542d00,0x3c5e3200,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x008d4c00,0x968d4c00,0xffbc5d00,0xffde6a00,0xffde6a00,0xffde6a00,0xffdf6a00,0xffdf6a00,0xffdf6a00,0xffde6a00,0xffdc6800,0xffd66600,0xffcc6100,0xffbf5b00,0xffaf5300,0xff9d4a00,0xff8a4200,0xff6d3500,0xff502900,0xe7402300,0x7b3f2200,0x15442500,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x2a8d4c00,0xff9b5000,0xffda6600,0xffdb6700,0xffdb6700,0xffdc6700,0xffdc6700,0xffdb6700,0xffd96500,0xffd16200,0xffc25b00,0xffad5100,0xff974700,0xff7f3c00,0xff602f00,0xff472500,0xbd3d2100,0x513d2100,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x008e4c00,0xc08d4c00,0xffc35c00,0xffd76300,0xffd76300,0xffd86300,0xffd86300,0xffd76300,0xffd06000,0xffc05800,0xffa54c00,0xff7f3b00,0xff582c00,0xf03f2200,0x903c2000,0x2a3e2100,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x548d4c00,0xffa55200,0xffd35f00,0xffd46000,0xffd46000,0xffd46000,0xffd25e00,0xffc65900,0xffac4e00,0xff833c00,0xe7472600,0x693c2000,0x0c3d2100,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x038d4c00,0xe48d4c00,0xffc95a00,0xffd15d00,0xffd15d00,0xffd15d00,0xffcb5a00,0xffb95200,0xff984300,0xff5f2e00,0x723f2200,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x7b8d4c00,0xffad5200,0xffce5a00,0xffce5a00,0xffcd5900,0xffc35500,0xffaa4a00,0xff853a00,0xf9472600,0x15432400,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x188d4c00,0xf98e4c00,0xffc95600,0xffcb5700,0xffc75500,0xffb94f00,0xff9b4200,0xff6c3100,0xab442500,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4d00,0x008e4c00,0xa58d4c00,0xffb35000,0xffc75300,0xffc05000,0xffac4800,0xff8b3a00,0xff542a00,0x45462500,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x398d4c00,0xff994d00,0xffc24f00,0xffb74b00,0xff9e4000,0xff763200,0xde472600,0x03492800,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x008e4c00,0xcf8d4c00,0xffb24b00,0xffab4500,0xff8d3900,0xff5e2b00,0x7e452500,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x638d4c00,0xff984800,0xffa03f00,0xff7e3200,0xfc492800,0x1b472600,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x098b4b00,0xed824600,0xff903800,0xff692c00,0xb4462600,0x004c2900,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008e4d00,0x008e4c00,0x008a4a00,0x8a7e4400,0xff793500,0xff572900,0x51472600,0x00542d00,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008d4c00,0x00884900,0x247a4200,0xfc633500,0xe74f2a00,0x034d2900,0x005e3300,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008e4d00,0x008d4c00,0x00884900,0x00794100,0xb4643600,0x87552e00,0x00593000,0x006b3900,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008f4d00,0x008d4c00,0x00884900,0x007c4300,0x486d3b00,0x24643600,0x00693800,0x00774000,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,
	0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff,0x00ffffff
};

int ui_init(running_machine &machine)
{
	/* make sure we clean up after ourselves */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_exit), &machine));

	/* initialize the other UI bits */
	ui_menu::init(machine);
	ui_gfx_init(machine);

	/* reset globals */
	single_step = FALSE;
	ui_set_handler(handler_messagebox, 0);
	/* retrieve options */
	ui_use_natural_keyboard = machine.options().natural_keyboard();
	bitmap_argb32 *ui_mouse_bitmap = auto_alloc(machine, bitmap_argb32(32, 32));
	UINT32 *dst = &ui_mouse_bitmap->pix32(0);
	memcpy(dst,mouse_bitmap,32*32*sizeof(UINT32));
	ui_mouse_arrow_texture = machine.render().texture_alloc();
	ui_mouse_arrow_texture->set_bitmap(*ui_mouse_bitmap, ui_mouse_bitmap->cliprect(), TEXFORMAT_ARGB32);
	return 0;
}


/*-------------------------------------------------
    ui_exit - clean up ourselves on exit
-------------------------------------------------*/

static void ui_exit(running_machine &machine)
{
	machine.render().texture_free(ui_mouse_arrow_texture);
	/* free the font */
	machine.render().font_free(ui_font);
	ui_font = NULL;
}


/*-------------------------------------------------
    ui_display_startup_screens - display the
    various startup screens
-------------------------------------------------*/

int ui_display_startup_screens(running_machine &machine, int first_time, int show_disclaimer)
{
	const int maxstate = 3;
	int str = machine.options().seconds_to_run();
	int show_gameinfo = !machine.options().skip_gameinfo();
	int show_warnings = TRUE;
	int state;

	/* disable everything if we are using -str for 300 or fewer seconds, or if we're the empty driver,
       or if we are debugging */
	if (!first_time || (str > 0 && str < 60*5) || &machine.system() == &GAME_NAME(___empty) || (machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		show_gameinfo = show_warnings = show_disclaimer = FALSE;

	/* initialize the on-screen display system */
	slider_list = slider_current = slider_init(machine);

	/* loop over states */
	ui_set_handler(handler_ingame, 0);
	for (state = 0; state < maxstate && !machine.scheduled_event_pending() && !ui_menu::stack_has_special_main_menu(); state++)
	{
		/* default to standard colors */
		messagebox_backcolor = UI_BACKGROUND_COLOR;

		/* pick the next state */
		switch (state)
		{
			case 0:
				if (show_disclaimer && disclaimer_string(machine, messagebox_text).len() > 0)
					ui_set_handler(handler_messagebox_ok, 0);
				break;

			case 1:
				if (show_warnings && warnings_string(machine, messagebox_text).len() > 0)
				{
					ui_set_handler(handler_messagebox_ok, 0);
					if (machine.system().flags & (GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS | GAME_REQUIRES_ARTWORK | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_KEYBOARD | GAME_NO_SOUND))
						messagebox_backcolor = UI_YELLOW_COLOR;
					if (machine.system().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_MECHANICAL))
						messagebox_backcolor = UI_RED_COLOR;
				}
				break;

			case 2:
				if (show_gameinfo && game_info_astring(machine, messagebox_text).len() > 0)
					ui_set_handler(handler_messagebox_anykey, 0);
				break;
		}

		/* clear the input memory */
		machine.input().reset_polling();
		while (machine.input().poll_switches() != INPUT_CODE_INVALID) ;

		/* loop while we have a handler */
		while (ui_handler_callback != handler_ingame && !machine.scheduled_event_pending() && !ui_menu::stack_has_special_main_menu())
			machine.video().frame_update();

		/* clear the handler and force an update */
		ui_set_handler(handler_ingame, 0);
		machine.video().frame_update();
	}

	/* if we're the empty driver, force the menus on */
	if (ui_menu::stack_has_special_main_menu())
		ui_set_handler(ui_menu::ui_handler, 0);

	return 0;
}


/*-------------------------------------------------
    ui_set_startup_text - set the text to display
    at startup
-------------------------------------------------*/

void ui_set_startup_text(running_machine &machine, const char *text, int force)
{
	static osd_ticks_t lastupdatetime = 0;
	osd_ticks_t curtime = osd_ticks();

	/* copy in the new text */
	messagebox_text.cpy(text);
	messagebox_backcolor = UI_BACKGROUND_COLOR;

	/* don't update more than 4 times/second */
	if (force || (curtime - lastupdatetime) > osd_ticks_per_second() / 4)
	{
		lastupdatetime = curtime;
		machine.video().frame_update();
	}
}


/*-------------------------------------------------
    ui_update_and_render - update the UI and
    render it; called by video.c
-------------------------------------------------*/

void ui_update_and_render(running_machine &machine, render_container *container)
{
	/* always start clean */
	container->empty();

	/* if we're paused, dim the whole screen */
	if (machine.phase() >= MACHINE_PHASE_RESET && (single_step || machine.paused()))
	{
		int alpha = (1.0f - machine.options().pause_brightness()) * 255.0f;
		if (ui_menu::stack_has_special_main_menu())
			alpha = 255;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			container->add_rect(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(alpha,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* render any cheat stuff at the bottom */
	if (machine.phase() >= MACHINE_PHASE_RESET)
		machine.cheat().render_text(*container);

	/* call the current UI handler */
	assert(ui_handler_callback != NULL);
	ui_handler_param = (*ui_handler_callback)(machine, container, ui_handler_param);

	/* display any popup messages */
	if (osd_ticks() < popup_text_end)
		ui_draw_text_box(container, messagebox_text, JUSTIFY_CENTER, 0.5f, 0.9f, messagebox_backcolor);
	else
		popup_text_end = 0;

	if (ui_mouse_show || ui_is_menu_active()) 
	{
		INT32 mouse_target_x, mouse_target_y;
		int mouse_button;
		render_target *mouse_target = ui_input_find_mouse(machine, &mouse_target_x, &mouse_target_y, &mouse_button);

		if (mouse_target != NULL)
		{
			float mouse_y=-1,mouse_x=-1;
			if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y)) {
				container->add_quad(mouse_x,mouse_y,mouse_x + 0.05*container->manager().ui_aspect(),mouse_y + 0.05,UI_TEXT_COLOR,ui_mouse_arrow_texture,PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
		}
	}

	/* cancel takes us back to the ingame handler */
	if (ui_handler_param == UI_HANDLER_CANCEL)
		ui_set_handler(handler_ingame, 0);
}


/*-------------------------------------------------
    ui_get_font - return the UI font
-------------------------------------------------*/

render_font *ui_get_font(running_machine &machine)
{
	/* allocate the font and messagebox string */
	if (ui_font == NULL)
		ui_font = machine.render().font_alloc(machine.options().ui_font());
	return ui_font;
}


/*-------------------------------------------------
    ui_get_line_height - return the current height
    of a line
-------------------------------------------------*/

float ui_get_line_height(running_machine &machine)
{
	INT32 raw_font_pixel_height = ui_get_font(machine)->pixel_height();
	render_target &ui_target = machine.render().ui_target();
	INT32 target_pixel_height = ui_target.height();
	float one_to_one_line_height;
	float scale_factor;

	/* compute the font pixel height at the nominal size */
	one_to_one_line_height = (float)raw_font_pixel_height / (float)target_pixel_height;

	/* determine the scale factor */
	scale_factor = UI_TARGET_FONT_HEIGHT / one_to_one_line_height;

	/* if our font is small-ish, do integral scaling */
	if (raw_font_pixel_height < 24)
	{
		/* do we want to scale smaller? only do so if we exceed the threshhold */
		if (scale_factor <= 1.0f)
		{
			if (one_to_one_line_height < UI_MAX_FONT_HEIGHT || raw_font_pixel_height < 12)
				scale_factor = 1.0f;
		}

		/* otherwise, just ensure an integral scale factor */
		else
			scale_factor = floor(scale_factor);
	}

	/* otherwise, just make sure we hit an even number of pixels */
	else
	{
		INT32 height = scale_factor * one_to_one_line_height * (float)target_pixel_height;
		scale_factor = (float)height / (one_to_one_line_height * (float)target_pixel_height);
	}

	return scale_factor * one_to_one_line_height;
}


/*-------------------------------------------------
    ui_get_char_width - return the width of a
    single character
-------------------------------------------------*/

float ui_get_char_width(running_machine &machine, unicode_char ch)
{
	return ui_get_font(machine)->char_width(ui_get_line_height(machine), machine.render().ui_aspect(), ch);
}


/*-------------------------------------------------
    ui_get_string_width - return the width of a
    character string
-------------------------------------------------*/

float ui_get_string_width(running_machine &machine, const char *s)
{
	return ui_get_font(machine)->utf8string_width(ui_get_line_height(machine), machine.render().ui_aspect(), s);
}


/*-------------------------------------------------
    ui_draw_outlined_box - add primitives to draw
    an outlined box with the given background
    color
-------------------------------------------------*/

void ui_draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor)
{
	container->add_rect(x0, y0, x1, y1, backcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x0, y0, x1, y0, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x1, y0, x1, y1, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x1, y1, x0, y1, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x0, y1, x0, y0, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


/*-------------------------------------------------
    ui_draw_text - simple text renderer
-------------------------------------------------*/

void ui_draw_text(render_container *container, const char *buf, float x, float y)
{
	ui_draw_text_full(container, buf, x, y, 1.0f - x, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


/*-------------------------------------------------
    ui_draw_text_full - full featured text
    renderer with word wrapping, justification,
    and full size computation
-------------------------------------------------*/

void ui_draw_text_full(render_container *container, const char *origs, float x, float y, float origwrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight)
{
	running_machine &machine = container->manager().machine();
	float lineheight = ui_get_line_height(machine);
	const char *ends = origs + strlen(origs);
	float wrapwidth = origwrapwidth;
	const char *s = origs;
	const char *linestart;
	float cury = y;
	float maxwidth = 0;
	float aspect = machine.render().ui_aspect();

	/* if we don't want wrapping, guarantee a huge wrapwidth */
	if (wrap == WRAP_NEVER)
		wrapwidth = 1000000.0f;
	if (wrapwidth <= 0)
		return;

	/* loop over lines */
	while (*s != 0)
	{
		const char *lastbreak = NULL;
		int line_justify = justify;
		unicode_char schar;
		int scharcount;
		float lastbreak_width = 0;
		float curwidth = 0;
		float curx = x;

		/* get the current character */
		scharcount = uchar_from_utf8(&schar, s, ends - s);
		if (scharcount == -1)
			break;

		/* if the line starts with a tab character, center it regardless */
		if (schar == '\t')
		{
			s += scharcount;
			line_justify = JUSTIFY_CENTER;
		}

		/* remember the starting position of the line */
		linestart = s;

		/* loop while we have characters and are less than the wrapwidth */
		while (*s != 0 && curwidth <= wrapwidth)
		{
			float chwidth;

			/* get the current chcaracter */
			scharcount = uchar_from_utf8(&schar, s, ends - s);
			if (scharcount == -1)
				break;

			/* if we hit a newline, stop immediately */
			if (schar == '\n')
				break;

			/* get the width of this character */
			chwidth = ui_get_font(machine)->char_width(lineheight, aspect, schar);

			/* if we hit a space, remember the location and width *without* the space */
			if (schar == ' ')
			{
				lastbreak = s;
				lastbreak_width = curwidth;
			}

			/* add the width of this character and advance */
			curwidth += chwidth;
			s += scharcount;

			/* if we hit any non-space breakable character, remember the location and width
               *with* the breakable character */
			if (schar != ' ' && is_breakable_char(schar) && curwidth <= wrapwidth)
			{
				lastbreak = s;
				lastbreak_width = curwidth;
			}
		}

		/* if we accumulated too much for the current width, we need to back off */
		if (curwidth > wrapwidth)
		{
			/* if we're word wrapping, back up to the last break if we can */
			if (wrap == WRAP_WORD)
			{
				/* if we hit a break, back up to there with the appropriate width */
				if (lastbreak != NULL)
				{
					s = lastbreak;
					curwidth = lastbreak_width;
				}

				/* if we didn't hit a break, back up one character */
				else if (s > linestart)
				{
					/* get the previous character */
					s = (const char *)utf8_previous_char(s);
					scharcount = uchar_from_utf8(&schar, s, ends - s);
					if (scharcount == -1)
						break;

					curwidth -= ui_get_font(machine)->char_width(lineheight, aspect, schar);
				}
			}

			/* if we're truncating, make sure we have enough space for the ... */
			else if (wrap == WRAP_TRUNCATE)
			{
				/* add in the width of the ... */
				curwidth += 3.0f * ui_get_font(machine)->char_width(lineheight, aspect, '.');

				/* while we are above the wrap width, back up one character */
				while (curwidth > wrapwidth && s > linestart)
				{
					/* get the previous character */
					s = (const char *)utf8_previous_char(s);
					scharcount = uchar_from_utf8(&schar, s, ends - s);
					if (scharcount == -1)
						break;

					curwidth -= ui_get_font(machine)->char_width(lineheight, aspect, schar);
				}
			}
		}

		/* align according to the justfication */
		if (line_justify == JUSTIFY_CENTER)
			curx += (origwrapwidth - curwidth) * 0.5f;
		else if (line_justify == JUSTIFY_RIGHT)
			curx += origwrapwidth - curwidth;

		/* track the maximum width of any given line */
		if (curwidth > maxwidth)
			maxwidth = curwidth;

		/* if opaque, add a black box */
		if (draw == DRAW_OPAQUE)
			container->add_rect(curx, cury, curx + curwidth, cury + lineheight, bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* loop from the line start and add the characters */
		while (linestart < s)
		{
			/* get the current character */
			unicode_char linechar;
			int linecharcount = uchar_from_utf8(&linechar, linestart, ends - linestart);
			if (linecharcount == -1)
				break;

			if (draw != DRAW_NONE)
			{
				container->add_char(curx, cury, lineheight, aspect, fgcolor, *ui_get_font(machine), linechar);
				curx += ui_get_font(machine)->char_width(lineheight, aspect, linechar);
			}
			linestart += linecharcount;
		}

		/* append ellipses if needed */
		if (wrap == WRAP_TRUNCATE && *s != 0 && draw != DRAW_NONE)
		{
			container->add_char(curx, cury, lineheight, aspect, fgcolor, *ui_get_font(machine), '.');
			curx += ui_get_font(machine)->char_width(lineheight, aspect, '.');
			container->add_char(curx, cury, lineheight, aspect, fgcolor, *ui_get_font(machine), '.');
			curx += ui_get_font(machine)->char_width(lineheight, aspect, '.');
			container->add_char(curx, cury, lineheight, aspect, fgcolor, *ui_get_font(machine), '.');
			curx += ui_get_font(machine)->char_width(lineheight, aspect, '.');
		}

		/* if we're not word-wrapping, we're done */
		if (wrap != WRAP_WORD)
			break;

		/* advance by a row */
		cury += lineheight;

		/* skip past any spaces at the beginning of the next line */
		scharcount = uchar_from_utf8(&schar, s, ends - s);
		if (scharcount == -1)
			break;

		if (schar == '\n')
			s += scharcount;
		else
			while (*s && isspace(schar))
			{
				s += scharcount;
				scharcount = uchar_from_utf8(&schar, s, ends - s);
				if (scharcount == -1)
					break;
			}
	}

	/* report the width and height of the resulting space */
	if (totalwidth)
		*totalwidth = maxwidth;
	if (totalheight)
		*totalheight = cury - y;
}


/*-------------------------------------------------
    ui_draw_text_box - draw a multiline text
    message with a box around it
-------------------------------------------------*/

void ui_draw_text_box(render_container *container, const char *text, int justify, float xpos, float ypos, rgb_t backcolor)
{
	float line_height = ui_get_line_height(container->manager().machine());
	float max_width = 2.0f * ((xpos <= 0.5f) ? xpos : 1.0f - xpos) - 2.0f * UI_BOX_LR_BORDER;
	float target_width = max_width;
	float target_height = line_height;
	float target_x = 0, target_y = 0;
	float last_target_height = 0;

	// limit this iteration to a finite number of passes
	for (int pass = 0; pass < 5; pass++)
	{
		/* determine the target location */
		target_x = xpos - 0.5f * target_width;
		target_y = ypos - 0.5f * target_height;

		/* make sure we stay on-screen */
		if (target_x < UI_BOX_LR_BORDER)
			target_x = UI_BOX_LR_BORDER;
		if (target_x + target_width + UI_BOX_LR_BORDER > 1.0f)
			target_x = 1.0f - UI_BOX_LR_BORDER - target_width;
		if (target_y < UI_BOX_TB_BORDER)
			target_y = UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > 1.0f)
			target_y = 1.0f - UI_BOX_TB_BORDER - target_height;

		/* compute the multi-line target width/height */
		ui_draw_text_full(container, text, target_x, target_y, target_width + 0.00001f,
					justify, WRAP_WORD, DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &target_width, &target_height);
		if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
			target_height = floor((1.0f - 2.0f * UI_BOX_TB_BORDER) / line_height) * line_height;

		/* if we match our last value, we're done */
		if (target_height == last_target_height)
			break;
		last_target_height = target_height;
	}

	/* add a box around that */
	ui_draw_outlined_box(container, target_x - UI_BOX_LR_BORDER,
					 target_y - UI_BOX_TB_BORDER,
					 target_x + target_width + UI_BOX_LR_BORDER,
					 target_y + target_height + UI_BOX_TB_BORDER, backcolor);
	ui_draw_text_full(container, text, target_x, target_y, target_width + 0.00001f,
				justify, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


/*-------------------------------------------------
    ui_popup_time - popup a message for a specific
    amount of time
-------------------------------------------------*/

void CLIB_DECL ui_popup_time(int seconds, const char *text, ...)
{
	va_list arg;

	/* extract the text */
	va_start(arg,text);
	messagebox_text.vprintf(text, arg);
	messagebox_backcolor = UI_BACKGROUND_COLOR;
	va_end(arg);

	/* set a timer */
	popup_text_end = osd_ticks() + osd_ticks_per_second() * seconds;
}


/*-------------------------------------------------
    ui_show_fps_temp - show the FPS counter for
    a specific period of time
-------------------------------------------------*/

void ui_show_fps_temp(double seconds)
{
	if (!showfps)
		showfps_end = osd_ticks() + seconds * osd_ticks_per_second();
}


/*-------------------------------------------------
    ui_set_show_fps - show/hide the FPS counter
-------------------------------------------------*/

void ui_set_show_fps(int show)
{
	showfps = show;
	if (!show)
	{
		showfps = 0;
		showfps_end = 0;
	}
}


/*-------------------------------------------------
    ui_get_show_fps - return the current FPS
    counter visibility state
-------------------------------------------------*/

int ui_get_show_fps(void)
{
	return showfps || (showfps_end != 0);
}


/*-------------------------------------------------
    ui_set_show_profiler - show/hide the profiler
-------------------------------------------------*/

void ui_set_show_profiler(int show)
{
	show_profiler = show;
	g_profiler.enable(show);
}


/*-------------------------------------------------
    ui_get_show_profiler - return the current
    profiler visibility state
-------------------------------------------------*/

int ui_get_show_profiler(void)
{
	return show_profiler;
}


/*-------------------------------------------------
    ui_show_menu - show the menus
-------------------------------------------------*/

void ui_show_menu(void)
{
	ui_set_handler(ui_menu::ui_handler, 0);
}


/*-------------------------------------------------
    ui_show_mouse - change mouse status 
-------------------------------------------------*/

void ui_show_mouse(bool status)
{
	ui_mouse_show = status;
}

/*-------------------------------------------------
    ui_is_menu_active - return TRUE if the menu
    UI handler is active
-------------------------------------------------*/

int ui_is_menu_active(void)
{
	return (ui_handler_callback == ui_menu::ui_handler);
}



/***************************************************************************
    TEXT GENERATORS
***************************************************************************/

/*-------------------------------------------------
    disclaimer_string - print the disclaimer
    text to the given buffer
-------------------------------------------------*/

static astring &disclaimer_string(running_machine &machine, astring &string)
{
	string.cpy("Usage of emulators in conjunction with ROMs you don't own is forbidden by copyright law.\n\n");
	string.catprintf("IF YOU ARE NOT LEGALLY ENTITLED TO PLAY \"%s\" ON THIS EMULATOR, PRESS ESC.\n\n", machine.system().description);
	string.cat("Otherwise, type OK or move the joystick left then right to continue");
	return string;
}


/*-------------------------------------------------
    warnings_string - print the warning flags
    text to the given buffer
-------------------------------------------------*/

static astring &warnings_string(running_machine &machine, astring &string)
{
#define WARNING_FLAGS (	GAME_NOT_WORKING | \
						GAME_UNEMULATED_PROTECTION | \
						GAME_MECHANICAL | \
						GAME_WRONG_COLORS | \
						GAME_IMPERFECT_COLORS | \
						GAME_REQUIRES_ARTWORK | \
						GAME_NO_SOUND |  \
						GAME_IMPERFECT_SOUND |  \
						GAME_IMPERFECT_GRAPHICS | \
						GAME_IMPERFECT_KEYBOARD | \
						GAME_NO_COCKTAIL)

	string.reset();

	/* if no warnings, nothing to return */
	if (rom_load_warnings(machine) == 0 && rom_load_knownbad(machine) == 0 && !(machine.system().flags & WARNING_FLAGS) && software_load_warnings_message(machine).len()==0)
		return string;

	/* add a warning if any ROMs were loaded with warnings */
	if (rom_load_warnings(machine) > 0)
	{
		string.cat("One or more ROMs/CHDs for this game are incorrect. The ");
		string.cat(emulator_info::get_gamenoun());
		string.cat(" may not run correctly.\n");
		if (machine.system().flags & WARNING_FLAGS)
			string.cat("\n");
	}

	if (software_load_warnings_message(machine).len()>0) {
		string.cat(software_load_warnings_message(machine));
		if (machine.system().flags & WARNING_FLAGS)
			string.cat("\n");
	}
	/* if we have at least one warning flag, print the general header */
	if ((machine.system().flags & WARNING_FLAGS) || rom_load_knownbad(machine) > 0)
	{
		string.cat("There are known problems with this ");
		string.cat(emulator_info::get_gamenoun());
		string.cat("\n\n");

		/* add a warning if any ROMs are flagged BAD_DUMP/NO_DUMP */
		if (rom_load_knownbad(machine) > 0) {
			string.cat("One or more ROMs/CHDs for this ");
			string.cat(emulator_info::get_gamenoun());
			string.cat(" have not been correctly dumped.\n");
		}
		/* add one line per warning flag */
		if (machine.system().flags & GAME_IMPERFECT_KEYBOARD)
			string.cat("The keyboard emulation may not be 100% accurate.\n");
		if (machine.system().flags & GAME_IMPERFECT_COLORS)
			string.cat("The colors aren't 100% accurate.\n");
		if (machine.system().flags & GAME_WRONG_COLORS)
			string.cat("The colors are completely wrong.\n");
		if (machine.system().flags & GAME_IMPERFECT_GRAPHICS)
			string.cat("The video emulation isn't 100% accurate.\n");
		if (machine.system().flags & GAME_IMPERFECT_SOUND)
			string.cat("The sound emulation isn't 100% accurate.\n");
		if (machine.system().flags & GAME_NO_SOUND)
			string.cat("The game lacks sound.\n");
		if (machine.system().flags & GAME_NO_COCKTAIL)
			string.cat("Screen flipping in cocktail mode is not supported.\n");

		/* check if external artwork is present before displaying this warning? */
		if (machine.system().flags & GAME_REQUIRES_ARTWORK)
			string.cat("The game requires external artwork files\n");

		/* if there's a NOT WORKING, UNEMULATED PROTECTION or GAME MECHANICAL warning, make it stronger */
		if (machine.system().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_MECHANICAL))
		{
			/* add the strings for these warnings */
			if (machine.system().flags & GAME_UNEMULATED_PROTECTION)
				string.cat("The game has protection which isn't fully emulated.\n");
			if (machine.system().flags & GAME_NOT_WORKING) {
				string.cat("\nTHIS ");
				string.cat(emulator_info::get_capgamenoun());
				string.cat(" DOESN'T WORK. The emulation for this game is not yet complete. "
					 "There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n");
			}
			if (machine.system().flags & GAME_MECHANICAL) {
				string.cat("\nCertain elements of this ");
				string.cat(emulator_info::get_gamenoun());
				string.cat(" cannot be emulated as it requires actual physical interaction or consists of mechanical devices. "
					 "It is not possible to fully play this ");
				string.cat(emulator_info::get_gamenoun());
				string.cat(".\n");
			}

			/* find the parent of this driver */
			driver_enumerator drivlist(machine.options());
			int maindrv = drivlist.find(machine.system());
			int clone_of = drivlist.non_bios_clone(maindrv);
			if (clone_of != -1)
				maindrv = clone_of;

			/* scan the driver list for any working clones and add them */
			bool foundworking = false;
			while (drivlist.next())
				if (drivlist.current() == maindrv || drivlist.clone() == maindrv)
					if ((drivlist.driver().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_MECHANICAL)) == 0)
					{
						/* this one works, add a header and display the name of the clone */
						if (!foundworking)
							string.cat("\n\nThere are working clones of this game: ");
						else
							string.cat(", ");
						string.cat(drivlist.driver().name);
						foundworking = true;
					}

			if (foundworking)
				string.cat("\n");
		}
	}

	/* add the 'press OK' string */
	string.cat("\n\nType OK or move the joystick left then right to continue");
	return string;
}


/*-------------------------------------------------
    game_info_astring - populate an allocated
    string with the game info text
-------------------------------------------------*/

astring &game_info_astring(running_machine &machine, astring &string)
{
	/* print description, manufacturer, and CPU: */
	astring tempstr;
	string.printf("%s\n%s %s\nDriver: %s\n\nCPU:\n", machine.system().description, machine.system().year, machine.system().manufacturer, core_filename_extract_base(tempstr, machine.system().source_file).cstr());

	/* loop over all CPUs */
	execute_interface_iterator execiter(machine.root_device());
	tagmap_t<UINT8> exectags;
	for (device_execute_interface *exec = execiter.first(); exec != NULL; exec = execiter.next())
	{
		if (exectags.add(exec->device().tag(), 1, FALSE) == TMERR_DUPLICATE)
			continue;
		/* get cpu specific clock that takes internal multiplier/dividers into account */
		int clock = exec->device().clock();

		/* count how many identical CPUs we have */
		int count = 1;
		const char *name = exec->device().name();
		execute_interface_iterator execinneriter(machine.root_device());
		for (device_execute_interface *scan = execinneriter.first(); scan != NULL; scan = execinneriter.next())
		{
			if (exec->device().type() == scan->device().type() && strcmp(name, scan->device().name()) == 0 && exec->device().clock() == scan->device().clock())
				if (exectags.add(scan->device().tag(), 1, FALSE) != TMERR_DUPLICATE)
					count++;
		}

		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			string.catprintf("%d" UTF8_MULTIPLY, count);
		string.cat(name);

		/* display clock in kHz or MHz */
		if (clock >= 1000000)
			string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
		else
			string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
	}

	/* loop over all sound chips */
	sound_interface_iterator snditer(machine.root_device());
	tagmap_t<UINT8> soundtags;
	bool found_sound = false;
	for (device_sound_interface *sound = snditer.first(); sound != NULL; sound = snditer.next())
	{
		if (soundtags.add(sound->device().tag(), 1, FALSE) == TMERR_DUPLICATE)
			continue;

		/* append the Sound: string */
		if (!found_sound)
			string.cat("\nSound:\n");
		found_sound = true;

		/* count how many identical sound chips we have */
		int count = 1;
		sound_interface_iterator sndinneriter(machine.root_device());
		for (device_sound_interface *scan = sndinneriter.first(); scan != NULL; scan = sndinneriter.next())
		{
			if (sound->device().type() == scan->device().type() && sound->device().clock() == scan->device().clock())
				if (soundtags.add(scan->device().tag(), 1, FALSE) != TMERR_DUPLICATE)
					count++;
		}
		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			string.catprintf("%d" UTF8_MULTIPLY, count);
		string.cat(sound->device().name());

		/* display clock in kHz or MHz */
		int clock = sound->device().clock();
		if (clock >= 1000000)
			string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
		else if (clock != 0)
			string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
		else
			string.cat("\n");
	}

	/* display screen information */
	string.cat("\nVideo:\n");
	screen_device_iterator scriter(machine.root_device());
	int scrcount = scriter.count();
	if (scrcount == 0)
		string.cat("None\n");
	else
	{
		for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
		{
			if (scrcount > 1)
			{
				string.cat(slider_get_screen_desc(*screen));
				string.cat(": ");
			}

			if (screen->screen_type() == SCREEN_TYPE_VECTOR)
				string.cat("Vector\n");
			else
			{
				const rectangle &visarea = screen->visible_area();

				string.catprintf("%d " UTF8_MULTIPLY " %d (%s) %f" UTF8_NBSP "Hz\n",
						visarea.width(), visarea.height(),
						(machine.system().flags & ORIENTATION_SWAP_XY) ? "V" : "H",
						ATTOSECONDS_TO_HZ(screen->frame_period().attoseconds));
			}
		}
	}

	return string;
}



/***************************************************************************
    UI HANDLERS
***************************************************************************/

/*-------------------------------------------------
    handler_messagebox - displays the current
    messagebox_text string but handles no input
-------------------------------------------------*/

static UINT32 handler_messagebox(running_machine &machine, render_container *container, UINT32 state)
{
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);
	return 0;
}


/*-------------------------------------------------
    handler_messagebox_ok - displays the current
    messagebox_text string and waits for an OK
-------------------------------------------------*/

static UINT32 handler_messagebox_ok(running_machine &machine, render_container *container, UINT32 state)
{
	/* draw a standard message window */
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	/* an 'O' or left joystick kicks us to the next state */
	if (state == 0 && (machine.input().code_pressed_once(KEYCODE_O) || ui_input_pressed(machine, IPT_UI_LEFT)))
		state++;

	/* a 'K' or right joystick exits the state */
	else if (state == 1 && (machine.input().code_pressed_once(KEYCODE_K) || ui_input_pressed(machine, IPT_UI_RIGHT)))
		state = UI_HANDLER_CANCEL;

	/* if the user cancels, exit out completely */
	else if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine.schedule_exit();
		state = UI_HANDLER_CANCEL;
	}

	return state;
}


/*-------------------------------------------------
    handler_messagebox_anykey - displays the
    current messagebox_text string and waits for
    any keypress
-------------------------------------------------*/

static UINT32 handler_messagebox_anykey(running_machine &machine, render_container *container, UINT32 state)
{
	/* draw a standard message window */
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	/* if the user cancels, exit out completely */
	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine.schedule_exit();
		state = UI_HANDLER_CANCEL;
	}

	/* if any key is pressed, just exit */
	else if (machine.input().poll_switches() != INPUT_CODE_INVALID)
		state = UI_HANDLER_CANCEL;

	return state;
}

/*-------------------------------------------------
    process_natural_keyboard - processes any
    natural keyboard input
-------------------------------------------------*/

static void process_natural_keyboard(running_machine &machine)
{
	ui_event event;
	int i, pressed;
	input_item_id itemid;
	input_code code;
	UINT8 *key_down_ptr;
	UINT8 key_down_mask;

	/* loop while we have interesting events */
	while (ui_input_pop_event(machine, &event))
	{
		/* if this was a UI_EVENT_CHAR event, post it */
		if (event.event_type == UI_EVENT_CHAR)
			machine.ioport().natkeyboard().post(event.ch);
	}

	/* process natural keyboard keys that don't get UI_EVENT_CHARs */
	for (i = 0; i < ARRAY_LENGTH(non_char_keys); i++)
	{
		/* identify this keycode */
		itemid = non_char_keys[i];
		code = machine.input().code_from_itemid(itemid);

		/* ...and determine if it is pressed */
		pressed = machine.input().code_pressed(code);

		/* figure out whey we are in the key_down map */
		key_down_ptr = &non_char_keys_down[i / 8];
		key_down_mask = 1 << (i % 8);

		if (pressed && !(*key_down_ptr & key_down_mask))
		{
			/* this key is now down */
			*key_down_ptr |= key_down_mask;

			/* post the key */
			machine.ioport().natkeyboard().post(UCHAR_MAMEKEY_BEGIN + code.item_id());
		}
		else if (!pressed && (*key_down_ptr & key_down_mask))
		{
			/* this key is now up */
			*key_down_ptr &= ~key_down_mask;
		}
	}
}

/*-------------------------------------------------
    ui_paste - does a paste from the keyboard
-------------------------------------------------*/

void ui_paste(running_machine &machine)
{
	/* retrieve the clipboard text */
	char *text = osd_get_clipboard_text();

	/* was a result returned? */
	if (text != NULL)
	{
		/* post the text */
		machine.ioport().natkeyboard().post_utf8(text);

		/* free the string */
		osd_free(text);
	}
}

/*-------------------------------------------------
    ui_image_handler_ingame - execute display
    callback function for each image device
-------------------------------------------------*/

void ui_image_handler_ingame(running_machine &machine)
{
	/* run display routine for devices */
	if (machine.phase() == MACHINE_PHASE_RUNNING)
	{
		image_interface_iterator iter(machine.root_device());
		for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
			image->call_display();
	}
}

/*-------------------------------------------------
    handler_ingame - in-game handler takes care
    of the standard keypresses
-------------------------------------------------*/

static UINT32 handler_ingame(running_machine &machine, render_container *container, UINT32 state)
{
	bool is_paused = machine.paused();

	/* first draw the FPS counter */
	if (showfps || osd_ticks() < showfps_end)
	{
		astring tempstring;
		ui_draw_text_full(container, machine.video().speed_text(tempstring), 0.0f, 0.0f, 1.0f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}
	else
		showfps_end = 0;

	/* draw the profiler if visible */
	if (show_profiler)
	{
		astring profilertext;
		g_profiler.text(machine, profilertext);
		ui_draw_text_full(container, profilertext, 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}

	/* if we're single-stepping, pause now */
	if (single_step)
	{
		machine.pause();
		single_step = FALSE;
	}

	/* determine if we should disable the rest of the UI */
	int ui_disabled = (machine.ioport().has_keyboard() && !machine.ui_active());

	/* is ScrLk UI toggling applicable here? */
	if (machine.ioport().has_keyboard())
	{
		/* are we toggling the UI with ScrLk? */
		if (ui_input_pressed(machine, IPT_UI_TOGGLE_UI))
		{
			/* toggle the UI */
			machine.set_ui_active(!machine.ui_active());

			/* display a popup indicating the new status */
			if (machine.ui_active())
			{
				ui_popup_time(2, "%s\n%s\n%s\n%s\n%s\n%s\n",
					"Keyboard Emulation Status",
					"-------------------------",
					"Mode: PARTIAL Emulation",
					"UI:   Enabled",
					"-------------------------",
					"**Use ScrLock to toggle**");
			}
			else
			{
				ui_popup_time(2, "%s\n%s\n%s\n%s\n%s\n%s\n",
					"Keyboard Emulation Status",
					"-------------------------",
					"Mode: FULL Emulation",
					"UI:   Disabled",
					"-------------------------",
					"**Use ScrLock to toggle**");
			}
		}
	}

	/* is the natural keyboard enabled? */
	if (ui_get_use_natural_keyboard(machine) && (machine.phase() == MACHINE_PHASE_RUNNING))
		process_natural_keyboard(machine);

	if (!ui_disabled)
	{
		/* paste command */
		if (ui_input_pressed(machine, IPT_UI_PASTE))
			ui_paste(machine);
	}

	ui_image_handler_ingame(machine);

	if (ui_disabled) return ui_disabled;

	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		if (!machine.options().confirm_quit())
			machine.schedule_exit();
		else
			return ui_set_handler(handler_confirm_quit, 0);
	}

	/* turn on menus if requested */
	if (ui_input_pressed(machine, IPT_UI_CONFIGURE))
		return ui_set_handler(ui_menu::ui_handler, 0);

	/* if the on-screen display isn't up and the user has toggled it, turn it on */
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) == 0 && ui_input_pressed(machine, IPT_UI_ON_SCREEN_DISPLAY))
		return ui_set_handler(ui_menu_sliders::ui_handler, 1);

	/* handle a reset request */
	if (ui_input_pressed(machine, IPT_UI_RESET_MACHINE))
		machine.schedule_hard_reset();
	if (ui_input_pressed(machine, IPT_UI_SOFT_RESET))
		machine.schedule_soft_reset();

	/* handle a request to display graphics/palette */
	if (ui_input_pressed(machine, IPT_UI_SHOW_GFX))
	{
		if (!is_paused)
			machine.pause();
		return ui_set_handler(ui_gfx_ui_handler, is_paused);
	}

	/* handle a save state request */
	if (ui_input_pressed(machine, IPT_UI_SAVE_STATE))
	{
		machine.pause();
		return ui_set_handler(handler_load_save, LOADSAVE_SAVE);
	}

	/* handle a load state request */
	if (ui_input_pressed(machine, IPT_UI_LOAD_STATE))
	{
		machine.pause();
		return ui_set_handler(handler_load_save, LOADSAVE_LOAD);
	}

	/* handle a save snapshot request */
	if (ui_input_pressed(machine, IPT_UI_SNAPSHOT))
		machine.video().save_active_screen_snapshots();

	/* toggle pause */
	if (ui_input_pressed(machine, IPT_UI_PAUSE))
	{
		/* with a shift key, it is single step */
		if (is_paused && (machine.input().code_pressed(KEYCODE_LSHIFT) || machine.input().code_pressed(KEYCODE_RSHIFT)))
		{
			single_step = TRUE;
			machine.resume();
		}
		else if (machine.paused())
			machine.resume();
		else
			machine.pause();
	}

	/* handle a toggle cheats request */
	if (ui_input_pressed(machine, IPT_UI_TOGGLE_CHEAT))
		machine.cheat().set_enable(!machine.cheat().enabled());

	/* toggle movie recording */
	if (ui_input_pressed(machine, IPT_UI_RECORD_MOVIE))
	{
		if (!machine.video().is_recording())
		{
			machine.video().begin_recording(NULL, video_manager::MF_MNG);
			popmessage("REC START");
		}
		else
		{
			machine.video().end_recording();
			popmessage("REC STOP");
		}
	}

	/* toggle profiler display */
	if (ui_input_pressed(machine, IPT_UI_SHOW_PROFILER))
		ui_set_show_profiler(!ui_get_show_profiler());

	/* toggle FPS display */
	if (ui_input_pressed(machine, IPT_UI_SHOW_FPS))
		ui_set_show_fps(!ui_get_show_fps());

	/* increment frameskip? */
	if (ui_input_pressed(machine, IPT_UI_FRAMESKIP_INC))
	{
		/* get the current value and increment it */
		int newframeskip = machine.video().frameskip() + 1;
		if (newframeskip > MAX_FRAMESKIP)
			newframeskip = -1;
		machine.video().set_frameskip(newframeskip);

		/* display the FPS counter for 2 seconds */
		ui_show_fps_temp(2.0);
	}

	/* decrement frameskip? */
	if (ui_input_pressed(machine, IPT_UI_FRAMESKIP_DEC))
	{
		/* get the current value and decrement it */
		int newframeskip = machine.video().frameskip() - 1;
		if (newframeskip < -1)
			newframeskip = MAX_FRAMESKIP;
		machine.video().set_frameskip(newframeskip);

		/* display the FPS counter for 2 seconds */
		ui_show_fps_temp(2.0);
	}

	/* toggle throttle? */
	if (ui_input_pressed(machine, IPT_UI_THROTTLE))
		machine.video().set_throttled(!machine.video().throttled());

	/* check for fast forward */
	if (machine.ioport().type_pressed(IPT_UI_FAST_FORWARD))
	{
		machine.video().set_fastforward(true);
		ui_show_fps_temp(0.5);
	}
	else
		machine.video().set_fastforward(false);

	return 0;
}


/*-------------------------------------------------
    handler_load_save - leads the user through
    specifying a game to save or load
-------------------------------------------------*/

static UINT32 handler_load_save(running_machine &machine, render_container *container, UINT32 state)
{
	char filename[20];
	input_code code;
	char file = 0;

	/* if we're not in the middle of anything, skip */
	if (state == LOADSAVE_NONE)
		return 0;

	/* okay, we're waiting for a key to select a slot; display a message */
	if (state == LOADSAVE_SAVE)
		ui_draw_message_window(container, "Select position to save to");
	else
		ui_draw_message_window(container, "Select position to load from");

	/* check for cancel key */
	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		/* display a popup indicating things were cancelled */
		if (state == LOADSAVE_SAVE)
			popmessage("Save cancelled");
		else
			popmessage("Load cancelled");

		/* reset the state */
		machine.resume();
		return UI_HANDLER_CANCEL;
	}

	/* check for A-Z or 0-9 */
	for (input_item_id id = ITEM_ID_A; id <= ITEM_ID_Z; id++)
		if (machine.input().code_pressed_once(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
			file = id - ITEM_ID_A + 'a';
	if (file == 0)
		for (input_item_id id = ITEM_ID_0; id <= ITEM_ID_9; id++)
			if (machine.input().code_pressed_once(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
				file = id - ITEM_ID_0 + '0';
	if (file == 0)
		for (input_item_id id = ITEM_ID_0_PAD; id <= ITEM_ID_9_PAD; id++)
			if (machine.input().code_pressed_once(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
				file = id - ITEM_ID_0_PAD + '0';
	if (file == 0)
		return state;

	/* display a popup indicating that the save will proceed */
	sprintf(filename, "%c", file);
	if (state == LOADSAVE_SAVE)
	{
		popmessage("Save to position %c", file);
		machine.schedule_save(filename);
	}
	else
	{
		popmessage("Load from position %c", file);
		machine.schedule_load(filename);
	}

	/* remove the pause and reset the state */
	machine.resume();
	return UI_HANDLER_CANCEL;
}


/*-------------------------------------------------
 handler_confirm_quit - leads the user through
 confirming quit emulation
 -------------------------------------------------*/

static UINT32 handler_confirm_quit(running_machine &machine, render_container *container, UINT32 state)
{
	astring quit_message("Are you sure you want to quit?\n\n");
	quit_message.cat("Press ''UI Select'' (default: Enter) to quit,\n");
	quit_message.cat("Press ''UI Cancel'' (default: Esc) to return to emulation.");

	ui_draw_text_box(container, quit_message, JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);
	machine.pause();

	/* if the user press ENTER, quit the game */
	if (ui_input_pressed(machine, IPT_UI_SELECT))
		machine.schedule_exit();

	/* if the user press ESC, just continue */
	else if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine.resume();
		state = UI_HANDLER_CANCEL;
	}

	return state;
}


/***************************************************************************
    SLIDER CONTROLS
***************************************************************************/

/*-------------------------------------------------
    ui_get_slider_list - get the list of sliders
-------------------------------------------------*/

const slider_state *ui_get_slider_list(void)
{
	return slider_list;
}


/*-------------------------------------------------
    slider_alloc - allocate a new slider entry
-------------------------------------------------*/

static slider_state *slider_alloc(running_machine &machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg)
{
	int size = sizeof(slider_state) + strlen(title);
	slider_state *state = (slider_state *)auto_alloc_array_clear(machine, UINT8, size);

	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
	strcpy(state->description, title);

	return state;
}


/*-------------------------------------------------
    slider_init - initialize the list of slider
    controls
-------------------------------------------------*/

static slider_state *slider_init(running_machine &machine)
{
	ioport_field *field;
	ioport_port *port;
	slider_state *listhead = NULL;
	slider_state **tailptr = &listhead;
	astring string;
	int item;

	/* add overall volume */
	*tailptr = slider_alloc(machine, "Master Volume", -32, 0, 0, 1, slider_volume, NULL);
	tailptr = &(*tailptr)->next;

	/* add per-channel volume */
	mixer_input info;
	for (item = 0; machine.sound().indexed_mixer_input(item, info); item++)
	{
		INT32 maxval = 2000;
		INT32 defval = 1000;

		info.stream->input_name(info.inputnum, string);
		string.cat(" Volume");
		*tailptr = slider_alloc(machine, string, 0, defval, maxval, 20, slider_mixervol, (void *)(FPTR)item);
		tailptr = &(*tailptr)->next;
	}

	/* add analog adjusters */
	for (port = machine.ioport().first_port(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
			if (field->type() == IPT_ADJUSTER)
			{
				void *param = (void *)field;
				*tailptr = slider_alloc(machine, field->name(), field->minval(), field->defvalue(), field->maxval(), 1, slider_adjuster, param);
				tailptr = &(*tailptr)->next;
			}

	/* add CPU overclocking (cheat only) */
	if (machine.options().cheat())
	{
		execute_interface_iterator iter(machine.root_device());
		for (device_execute_interface *exec = iter.first(); exec != NULL; exec = iter.next())
		{
			void *param = (void *)&exec->device();
			string.printf("Overclock CPU %s", exec->device().tag());
			*tailptr = slider_alloc(machine, string, 10, 1000, 2000, 1, slider_overclock, param);
			tailptr = &(*tailptr)->next;
		}
	}

	/* add screen parameters */
	screen_device_iterator scriter(machine.root_device());
	for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
	{
		int defxscale = floor(screen->xscale() * 1000.0f + 0.5f);
		int defyscale = floor(screen->yscale() * 1000.0f + 0.5f);
		int defxoffset = floor(screen->xoffset() * 1000.0f + 0.5f);
		int defyoffset = floor(screen->yoffset() * 1000.0f + 0.5f);
		void *param = (void *)screen;

		/* add refresh rate tweaker */
		if (machine.options().cheat())
		{
			string.printf("%s Refresh Rate", slider_get_screen_desc(*screen));
			*tailptr = slider_alloc(machine, string, -10000, 0, 10000, 1000, slider_refresh, param);
			tailptr = &(*tailptr)->next;
		}

		/* add standard brightness/contrast/gamma controls per-screen */
		string.printf("%s Brightness", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 2000, 10, slider_brightness, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Contrast", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 2000, 50, slider_contrast, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Gamma", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 3000, 50, slider_gamma, param);
		tailptr = &(*tailptr)->next;

		/* add scale and offset controls per-screen */
		string.printf("%s Horiz Stretch", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 500, defxscale, 1500, 2, slider_xscale, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Horiz Position", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, -500, defxoffset, 500, 2, slider_xoffset, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Vert Stretch", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 500, defyscale, 1500, 2, slider_yscale, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Vert Position", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, -500, defyoffset, 500, 2, slider_yoffset, param);
		tailptr = &(*tailptr)->next;
	}

	laserdisc_device_iterator lditer(machine.root_device());
	for (laserdisc_device *laserdisc = lditer.first(); laserdisc != NULL; laserdisc = lditer.next())
		if (laserdisc->overlay_configured())
		{
			laserdisc_overlay_config config;
			laserdisc->get_overlay_config(config);
			int defxscale = floor(config.m_overscalex * 1000.0f + 0.5f);
			int defyscale = floor(config.m_overscaley * 1000.0f + 0.5f);
			int defxoffset = floor(config.m_overposx * 1000.0f + 0.5f);
			int defyoffset = floor(config.m_overposy * 1000.0f + 0.5f);
			void *param = (void *)laserdisc;

			/* add scale and offset controls per-overlay */
			string.printf("Laserdisc '%s' Horiz Stretch", laserdisc->tag());
			*tailptr = slider_alloc(machine, string, 500, (defxscale == 0) ? 1000 : defxscale, 1500, 2, slider_overxscale, param);
			tailptr = &(*tailptr)->next;
			string.printf("Laserdisc '%s' Horiz Position", laserdisc->tag());
			*tailptr = slider_alloc(machine, string, -500, defxoffset, 500, 2, slider_overxoffset, param);
			tailptr = &(*tailptr)->next;
			string.printf("Laserdisc '%s' Vert Stretch", laserdisc->tag());
			*tailptr = slider_alloc(machine, string, 500, (defyscale == 0) ? 1000 : defyscale, 1500, 2, slider_overyscale, param);
			tailptr = &(*tailptr)->next;
			string.printf("Laserdisc '%s' Vert Position", laserdisc->tag());
			*tailptr = slider_alloc(machine, string, -500, defyoffset, 500, 2, slider_overyoffset, param);
			tailptr = &(*tailptr)->next;
		}

	for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
		if (screen->screen_type() == SCREEN_TYPE_VECTOR)
		{
			/* add flicker control */
			*tailptr = slider_alloc(machine, "Vector Flicker", 0, 0, 1000, 10, slider_flicker, NULL);
			tailptr = &(*tailptr)->next;
			*tailptr = slider_alloc(machine, "Beam Width", 10, 100, 1000, 10, slider_beam, NULL);
			tailptr = &(*tailptr)->next;
			break;
		}

#ifdef MAME_DEBUG
	/* add crosshair adjusters */
	for (port = machine.ioport().first_port(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
			if (field->crosshair_axis() != CROSSHAIR_AXIS_NONE && field->player() == 0)
			{
				void *param = (void *)field;
				string.printf("Crosshair Scale %s", (field->crosshair_axis() == CROSSHAIR_AXIS_X) ? "X" : "Y");
				*tailptr = slider_alloc(machine, string, -3000, 1000, 3000, 100, slider_crossscale, param);
				tailptr = &(*tailptr)->next;
				string.printf("Crosshair Offset %s", (field->crosshair_axis() == CROSSHAIR_AXIS_X) ? "X" : "Y");
				*tailptr = slider_alloc(machine, string, -3000, 0, 3000, 100, slider_crossoffset, param);
				tailptr = &(*tailptr)->next;
			}
#endif

	return listhead;
}


/*-------------------------------------------------
    slider_volume - global volume slider callback
-------------------------------------------------*/

static INT32 slider_volume(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		machine.sound().set_attenuation(newval);
	if (string != NULL)
		string->printf("%3ddB", machine.sound().attenuation());
	return machine.sound().attenuation();
}


/*-------------------------------------------------
    slider_mixervol - single channel volume
    slider callback
-------------------------------------------------*/

static INT32 slider_mixervol(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	mixer_input info;
	if (!machine.sound().indexed_mixer_input((FPTR)arg, info))
		return 0;
	if (newval != SLIDER_NOCHANGE)
	{
		INT32 curval = floor(info.stream->user_gain(info.inputnum) * 1000.0f + 0.5f);
		if (newval > curval && (newval - curval) <= 4) newval += 4; // round up on increment
		info.stream->set_user_gain(info.inputnum, (float)newval * 0.001f);
	}
	if (string != NULL)
		string->printf("%4.2f", info.stream->user_gain(info.inputnum));
	return floor(info.stream->user_gain(info.inputnum) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_adjuster - analog adjuster slider
    callback
-------------------------------------------------*/

static INT32 slider_adjuster(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	ioport_field *field = (ioport_field *)arg;
	ioport_field::user_settings settings;

	field->get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.value = newval;
		field->set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%d%%", settings.value);
	return settings.value;
}


/*-------------------------------------------------
    slider_overclock - CPU overclocker slider
    callback
-------------------------------------------------*/

static INT32 slider_overclock(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	device_t *cpu = (device_t *)arg;
	if (newval != SLIDER_NOCHANGE)
		cpu->set_clock_scale((float)newval * 0.001f);
	if (string != NULL)
		string->printf("%3.0f%%", floor(cpu->clock_scale() * 100.0f + 0.5f));
	return floor(cpu->clock_scale() * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_refresh - refresh rate slider callback
-------------------------------------------------*/

static INT32 slider_refresh(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	double defrefresh = ATTOSECONDS_TO_HZ(screen->refresh_attoseconds());
	double refresh;

	if (newval != SLIDER_NOCHANGE)
	{
		int width = screen->width();
		int height = screen->height();
		const rectangle &visarea = screen->visible_area();
		screen->configure(width, height, visarea, HZ_TO_ATTOSECONDS(defrefresh + (double)newval * 0.001));
	}
	if (string != NULL)
		string->printf("%.3ffps", ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds));
	refresh = ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds);
	return floor((refresh - defrefresh) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_brightness - screen brightness slider
    callback
-------------------------------------------------*/

static INT32 slider_brightness(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_brightness = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_brightness);
	return floor(settings.m_brightness * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_contrast - screen contrast slider
    callback
-------------------------------------------------*/

static INT32 slider_contrast(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_contrast = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_contrast);
	return floor(settings.m_contrast * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_gamma - screen gamma slider callback
-------------------------------------------------*/

static INT32 slider_gamma(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_gamma = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_gamma);
	return floor(settings.m_gamma * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_xscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_xscale = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_xscale);
	return floor(settings.m_xscale * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_yscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_yscale = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_yscale);
	return floor(settings.m_yscale * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_xoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_xoffset = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_xoffset);
	return floor(settings.m_xoffset * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_yoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_yoffset = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_yoffset);
	return floor(settings.m_yoffset * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overxscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_overxscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overscalex = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overscalex);
	return floor(settings.m_overscalex * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overyscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_overyscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overscaley = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overscaley);
	return floor(settings.m_overscaley * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overxoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_overxoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overposx = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overposx);
	return floor(settings.m_overposx * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overyoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_overyoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overposy = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overposy);
	return floor(settings.m_overposy * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_flicker - vector flicker slider
    callback
-------------------------------------------------*/

static INT32 slider_flicker(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_set_flicker((float)newval * 0.1f);
	if (string != NULL)
		string->printf("%1.2f", vector_get_flicker());
	return floor(vector_get_flicker() * 10.0f + 0.5f);
}


/*-------------------------------------------------
    slider_beam - vector beam width slider
    callback
-------------------------------------------------*/

static INT32 slider_beam(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_set_beam((float)newval * 0.01f);
	if (string != NULL)
		string->printf("%1.2f", vector_get_beam());
	return floor(vector_get_beam() * 100.0f + 0.5f);
}


/*-------------------------------------------------
    slider_get_screen_desc - returns the
    description for a given screen
-------------------------------------------------*/

static char *slider_get_screen_desc(screen_device &screen)
{
	screen_device_iterator iter(screen.machine().root_device());
	int scrcount = iter.count();
	static char descbuf[256];

	if (scrcount > 1)
		sprintf(descbuf, "Screen '%s'", screen.tag());
	else
		strcpy(descbuf, "Screen");

	return descbuf;
}

/*-------------------------------------------------
    slider_crossscale - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	ioport_field *field = (ioport_field *)arg;

	if (newval != SLIDER_NOCHANGE)
		field->set_crosshair_scale(float(newval) * 0.001);
	if (string != NULL)
		string->printf("%s %s %1.3f", "Crosshair Scale", (field->crosshair_axis() == CROSSHAIR_AXIS_X) ? "X" : "Y", float(newval) * 0.001f);
	return floor(field->crosshair_scale() * 1000.0f + 0.5f);
}
#endif


/*-------------------------------------------------
    slider_crossoffset - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	ioport_field *field = (ioport_field *)arg;

	if (newval != SLIDER_NOCHANGE)
		field->set_crosshair_offset(float(newval) * 0.001f);
	if (string != NULL)
		string->printf("%s %s %1.3f", "Crosshair Offset", (field->crosshair_axis() == CROSSHAIR_AXIS_X) ? "X" : "Y", float(newval) * 0.001f);
	return field->crosshair_offset();
}
#endif


/*-------------------------------------------------
    ui_get_use_natural_keyboard - returns
    whether the natural keyboard is active
-------------------------------------------------*/

int ui_get_use_natural_keyboard(running_machine &machine)
{
	return ui_use_natural_keyboard;
}



/*-------------------------------------------------
    ui_set_use_natural_keyboard - specifies
    whether the natural keyboard is active
-------------------------------------------------*/

void ui_set_use_natural_keyboard(running_machine &machine, int use_natural_keyboard)
{
	ui_use_natural_keyboard = use_natural_keyboard;
	astring error;
	machine.options().set_value(OPTION_NATURAL_KEYBOARD, use_natural_keyboard, OPTION_PRIORITY_CMDLINE, error);
	assert(!error);
}

