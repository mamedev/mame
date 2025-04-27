// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui.cpp

    Functions used to handle MAME's user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"

#include "infoxml.h"
#include "iptseqpoll.h"
#include "luaengine.h"
#include "mame.h"
#include "ui/filemngr.h"
#include "ui/info.h"
#include "ui/mainmenu.h"
#include "ui/menu.h"
#include "ui/quitmenu.h"
#include "ui/sliders.h"
#include "ui/state.h"
#include "ui/systemlist.h"
#include "ui/viewgfx.h"

#include "imagedev/cassette.h"
#include "machine/laserdsc.h"
#include "video/vector.h"

#include "config.h"
#include "emuopts.h"
#include "mameopts.h"
#include "drivenum.h"
#include "fileio.h"
#include "natkeyboard.h"
#include "render.h"
#include "cheat.h"
#include "rendfont.h"
#include "rendlay.h"
#include "romload.h"
#include "screen.h"
#include "speaker.h"
#include "uiinput.h"

// FIXME: allow OSD module headers to be included in a less ugly way
#include "../osd/modules/lib/osdlib.h"
#include "../osd/modules/lib/osdobj_common.h"

#include <functional>
#include <type_traits>


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

// list of natural keyboard keys that are not associated with UI_EVENT_CHARs
static input_item_id const non_char_keys[] =
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

// messagebox buffer
std::string mame_ui_manager::messagebox_text;
std::string mame_ui_manager::messagebox_poptext;

// slider info
std::vector<ui::menu_item> mame_ui_manager::slider_list;


/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

static uint32_t const mouse_bitmap[32*32] =
{
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


enum class mame_ui_manager::ui_callback_type : int
{
	NOINPUT,
	GENERAL,
	MODAL,
	MENU,
	CUSTOM
};


struct mame_ui_manager::active_pointer
{
	active_pointer(ui_event const &event)
		: target(event.target)
		, updated(std::chrono::steady_clock::time_point::min())
		, type(event.pointer_type)
		, ptrid(event.pointer_id)
		, x(-1.0F)
		, y(-1.0F)
	{
	}

	bool operator<(std::pair<render_target *, u16> const &val) const noexcept
	{
		return std::make_pair(target, ptrid) < val;
	}

	render_target *target;
	std::chrono::steady_clock::time_point updated;
	osd::ui_event_handler::pointer type;
	u16 ptrid;
	float x, y;
};


class mame_ui_manager::pointer_options
{
public:
	pointer_options()
		: m_initial_timeout(std::chrono::seconds(3))
		, m_timeout(std::chrono::seconds(3))
		, m_initial_hide_inactive(true)
		, m_hide_inactive(true)
		, m_timeout_set(false)
		, m_hide_inactive_set(false)
	{
	}

	std::chrono::steady_clock::duration timeout() const noexcept { return m_timeout; }
	bool hide_inactive() const noexcept { return m_hide_inactive; }
	bool timeout_set() const noexcept { return m_timeout_set; }
	bool hide_inactive_set() const noexcept { return m_hide_inactive_set; }
	bool options_set() const noexcept { return m_timeout_set || m_hide_inactive_set; }

	void set_initial_timeout(std::chrono::steady_clock::duration value) noexcept
	{
		m_initial_timeout = value;
		if (!m_timeout_set)
			m_timeout = value;
	}

	void set_initial_hide_inactive(bool value) noexcept
	{
		m_initial_hide_inactive = value;
		if (!m_hide_inactive_set)
			m_hide_inactive = value;
	}

	void set_timeout(std::chrono::steady_clock::duration value) noexcept
	{
		m_timeout = value;
		m_timeout_set = true;
	}

	void set_hide_inactive(bool value) noexcept
	{
		m_hide_inactive = value;
		m_hide_inactive_set = true;
	}

	void restore_initial() noexcept
	{
		m_timeout = m_initial_timeout;
		m_hide_inactive = m_initial_hide_inactive;
		m_timeout_set = false;
		m_hide_inactive_set = false;
	}

private:
	std::chrono::steady_clock::duration m_initial_timeout;
	std::chrono::steady_clock::duration m_timeout;
	bool m_initial_hide_inactive;
	bool m_hide_inactive;
	bool m_timeout_set;
	bool m_hide_inactive_set;
};


//-------------------------------------------------
//  ctor - set up the user interface
//-------------------------------------------------

mame_ui_manager::mame_ui_manager(running_machine &machine)
	: ui_manager(machine)
	, m_font()
	, m_handler_callback()
	, m_handler_callback_type(ui_callback_type::NOINPUT)
	, m_ui_active(true)
	, m_single_step(false)
	, m_showfps(false)
	, m_showfps_end(0)
	, m_show_profiler(false)
	, m_popup_text_end(0)
	, m_mouse_bitmap(32, 32)
	, m_mouse_arrow_texture(nullptr)
	, m_pointers_changed(false)
	, m_target_font_height(0)
	, m_has_warnings(false)
	, m_unthrottle_mute(false)
	, m_image_display_enabled(true)
	, m_machine_info()
	, m_unemulated_features()
	, m_imperfect_features()
	, m_last_launch_time(std::time_t(-1))
	, m_last_warning_time(std::time_t(-1))
{
}

mame_ui_manager::~mame_ui_manager()
{
}

void mame_ui_manager::init()
{
	load_ui_options();

	// start loading system names as early as possible
	ui::system_list::instance().cache_data(options());

	// initialize the other UI bits
	m_ui_colors.refresh(options());

	// update font row info from setting
	update_target_font_height();

	// more initialization
	set_handler(
			ui_callback_type::NOINPUT,
			handler_callback_func(
				[this] (render_container &container) -> uint32_t
				{
					draw_text_box(container, messagebox_text, ui::text_layout::text_justify::LEFT, 0.5f, 0.5f, colors().background_color());
					return 0;
				}));
	m_non_char_keys_down = std::make_unique<uint8_t[]>((std::size(non_char_keys) + 7) / 8);

	// request notification callbacks
	machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&mame_ui_manager::frame_update, this));
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&mame_ui_manager::exit, this));
	machine().configuration().config_register(
			"ui_warnings",
			configuration_manager::load_delegate(&mame_ui_manager::config_load_warnings, this),
			configuration_manager::save_delegate(&mame_ui_manager::config_save_warnings, this));
	machine().configuration().config_register(
			"pointer_input",
			configuration_manager::load_delegate(&mame_ui_manager::config_load_pointers, this),
			configuration_manager::save_delegate(&mame_ui_manager::config_save_pointers, this));

	// create mouse bitmap
	uint32_t *dst = &m_mouse_bitmap.pix(0);
	memcpy(dst,mouse_bitmap,32*32*sizeof(uint32_t));
	m_mouse_arrow_texture = machine().render().texture_alloc();
	m_mouse_arrow_texture->set_bitmap(m_mouse_bitmap, m_mouse_bitmap.cliprect(), TEXFORMAT_ARGB32);
}


//-------------------------------------------------
//  update_target_font_height
//-------------------------------------------------

void mame_ui_manager::update_target_font_height()
{
	m_target_font_height = 1.0f / options().font_rows();
}


//-------------------------------------------------
//  exit - called for each emulated frame
//-------------------------------------------------

void mame_ui_manager::frame_update()
{
	// this hackery is needed to ensure natural keyboard and clickable artwork input is in sync with I/O ports
	if (ui_callback_type::GENERAL == m_handler_callback_type)
	{
		process_ui_events();
		for (auto *target = machine().render().first_target(); target; target = target->next())
		{
			if (!target->hidden())
				target->update_pointer_fields();
		}
	}
}


//-------------------------------------------------
//  exit - clean up ourselves on exit
//-------------------------------------------------

void mame_ui_manager::exit()
{
	// free the mouse texture
	machine().render().texture_free(m_mouse_arrow_texture);
	m_mouse_arrow_texture = nullptr;

	// free the font
	m_font.reset();

	// free persistent data for other classes
	m_session_data.clear();
}


//-------------------------------------------------
//  config_load_warnings - load info on last time
//  emulation status warnings showed
//-------------------------------------------------

void mame_ui_manager::config_load_warnings(
		config_type cfg_type,
		config_level cfg_level,
		util::xml::data_node const *parentnode)
{
	// make sure it's relevant and there's data available
	if (config_type::SYSTEM == cfg_type)
	{
		m_unemulated_features.clear();
		m_imperfect_features.clear();
		if (!parentnode)
		{
			m_last_launch_time = std::time_t(-1);
			m_last_warning_time = std::time_t(-1);
		}
		else
		{
			m_last_launch_time = std::time_t(parentnode->get_attribute_int("launched", -1));
			m_last_warning_time = std::time_t(parentnode->get_attribute_int("warned", -1));
			for (util::xml::data_node const *node = parentnode->get_first_child(); node; node = node->get_next_sibling())
			{
				if (!std::strcmp(node->get_name(), "feature"))
				{
					char const *const device = node->get_attribute_string("device", nullptr);
					char const *const feature = node->get_attribute_string("type", nullptr);
					char const *const status = node->get_attribute_string("status", nullptr);
					if (device && *device && feature && *feature && status && *status)
					{
						if (!std::strcmp(status, "unemulated"))
							m_unemulated_features.emplace(device, feature);
						else if (!std::strcmp(status, "imperfect"))
							m_imperfect_features.emplace(device, feature);
					}
				}
			}
		}
	}
}


//-------------------------------------------------
//  config_save_warnings - save information on
//  last time emulation status warnings showed
//-------------------------------------------------

void mame_ui_manager::config_save_warnings(
		config_type cfg_type,
		util::xml::data_node *parentnode)
{
	// only save system-level configuration when times are valid
	if ((config_type::SYSTEM == cfg_type) && (std::time_t(-1) != m_last_launch_time) && (std::time_t(-1) != m_last_warning_time))
	{
		parentnode->set_attribute_int("launched", static_cast<long long>(m_last_launch_time));
		parentnode->set_attribute_int("warned", static_cast<long long>(m_last_warning_time));

		for (auto const &feature : m_unemulated_features)
		{
			util::xml::data_node *const feature_node = parentnode->add_child("feature", nullptr);
			feature_node->set_attribute("device", feature.first.c_str());
			feature_node->set_attribute("type", feature.second.c_str());
			feature_node->set_attribute("status", "unemulated");
		}

		for (auto const &feature : m_imperfect_features)
		{
			util::xml::data_node *const feature_node = parentnode->add_child("feature", nullptr);
			feature_node->set_attribute("device", feature.first.c_str());
			feature_node->set_attribute("type", feature.second.c_str());
			feature_node->set_attribute("status", "imperfect");
		}
	}
}


//-------------------------------------------------
//  config_load_pointers - load pointer input
//  settings
//-------------------------------------------------

void mame_ui_manager::config_load_pointers(
		config_type cfg_type,
		config_level cfg_level,
		util::xml::data_node const *parentnode)
{
	switch (cfg_type)
	{
	case config_type::INIT:
		{
			int last(-1);
			for (auto const &target : machine().render().targets())
			{
				assert(target.index() >= 0);
				if (!target.hidden())
					last = (std::max)(target.index(), last);
			}
			m_pointer_options.resize(last + 1);
		}
		break;

	case config_type::CONTROLLER:
	case config_type::SYSTEM:
		if (!parentnode)
			break;
		for (auto const *targetnode = parentnode->get_child("target"); targetnode; targetnode = targetnode->get_next_sibling("target"))
		{
			auto const index(targetnode->get_attribute_int("index", -1));
			if ((0 <= index) && (m_pointer_options.size() > index))
			{
				auto const timeout(targetnode->get_attribute_float("activity_timeout", -1.0F));
				auto const ms(std::lround(timeout * 1000.0F));
				if ((0 <= ms) && (10'000 >= ms))
				{
					if (config_type::SYSTEM == cfg_type)
						m_pointer_options[index].set_timeout(std::chrono::milliseconds(ms));
					else
						m_pointer_options[index].set_initial_timeout(std::chrono::milliseconds(ms));
				}

				auto const hide(targetnode->get_attribute_int("hide_inactive", -1));
				if (0 <= hide)
				{
					if (config_type::SYSTEM == cfg_type)
						m_pointer_options[index].set_hide_inactive(hide != 0);
					else
						m_pointer_options[index].set_initial_hide_inactive(hide != 0);
				}
			}
		}
		break;

	case config_type::DEFAULT:
	case config_type::FINAL:
		break;
	}
}


//-------------------------------------------------
//  config_save_pointers - save pointer input
//  settings
//-------------------------------------------------

void mame_ui_manager::config_save_pointers(
		config_type cfg_type,
		util::xml::data_node *parentnode)
{
	if (config_type::SYSTEM == cfg_type)
	{
		for (std::size_t i = 0; m_pointer_options.size() > i; ++i)
		{
			pointer_options const &options(m_pointer_options[i]);
			if (options.options_set())
			{
				util::xml::data_node *const targetnode = parentnode->add_child("target", nullptr);
				if (targetnode)
				{
					targetnode->set_attribute_int("index", i);
					if (options.timeout_set())
					{
						auto const ms(std::chrono::duration_cast<std::chrono::milliseconds>(options.timeout()));
						targetnode->set_attribute_float("activity_timeout", float(ms.count()) * 0.001F);
					}
					if (options.hide_inactive_set())
						targetnode->set_attribute_int("hide_inactive", options.hide_inactive());
				}
			}
		}
	}
}


//-------------------------------------------------
//  initialize - initialize ui lists
//-------------------------------------------------

void mame_ui_manager::initialize(running_machine &machine)
{
	m_machine_info = std::make_unique<ui::machine_info>(machine);
	set_ui_active(!machine_info().has_keyboard() || machine.options().ui_active());

	// initialize the on-screen display system
	slider_list = slider_init(machine);

	// if no test switch found, assign its input sequence to a service mode DIP
	if (!m_machine_info->has_test_switch() && m_machine_info->has_dips())
	{
		const char *const service_mode_dipname = ioport_configurer::string_from_token(DEF_STR(Service_Mode));
		for (auto &port : machine.ioport().ports())
			for (ioport_field &field : port.second->fields())
				if ((field.type() == IPT_DIPSWITCH) && (field.name() == service_mode_dipname)) // FIXME: probably breaks with localisation, also issues with multiple devices
					field.set_defseq(machine.ioport().type_seq(IPT_SERVICE));
	}

	// handle throttle-related options and initial muting state now that the sound manager has been brought up
	const bool starting_throttle = machine.options().throttle();
	machine.video().set_throttled(starting_throttle);
	m_unthrottle_mute = options().unthrottle_mute();
	if (!starting_throttle && m_unthrottle_mute)
		machine.sound().ui_mute(true);
}


//-------------------------------------------------
//  set_handler - set a callback/parameter
//  pair for the current UI handler
//-------------------------------------------------

void mame_ui_manager::set_handler(ui_callback_type callback_type, handler_callback_func &&callback)
{
	m_active_pointers.clear();
	if (!m_display_pointers.empty())
	{
		m_display_pointers.clear();
		m_pointers_changed = true;
	}
	m_handler_callback = std::move(callback);
	m_handler_callback_type = callback_type;
}


//-------------------------------------------------
//  output_joined_collection
//-------------------------------------------------

template<typename TColl, typename TEmitMemberFunc, typename TEmitDelimFunc>
static void output_joined_collection(const TColl &collection, TEmitMemberFunc emit_member, TEmitDelimFunc emit_delim)
{
	bool is_first = true;

	for (const auto &member : collection)
	{
		if (is_first)
			is_first = false;
		else
			emit_delim();
		emit_member(member);
	}
}


//-------------------------------------------------
//  display_startup_screens - display the
//  various startup screens
//-------------------------------------------------

void mame_ui_manager::display_startup_screens(bool first_time)
{
	const int maxstate = 3;
	int const str = machine().options().seconds_to_run();
	bool show_gameinfo = !machine().options().skip_gameinfo();
	bool show_warnings = true;
	bool video_none = strcmp(downcast<osd_options &>(machine().options()).video(), OSDOPTVAL_NONE) == 0;

	// disable everything if we are using -str for 300 or fewer seconds, or if we're the empty driver,
	// or if we are debugging, or if there's no mame window to send inputs to
	if (!first_time || (str > 0 && str < 60*5) || &machine().system() == &GAME_NAME(___empty) || (machine().debug_flags & DEBUG_FLAG_ENABLED) || video_none)
		show_gameinfo = show_warnings = false;

#if defined(__EMSCRIPTEN__)
	// also disable for the JavaScript port since the startup screens do not run asynchronously
	show_gameinfo = show_warnings = false;
#endif

	// set up event handlers
	switch_code_poller poller(machine().input());
	std::string warning_text;
	rgb_t warning_color;
	bool config_menu = false;
	auto handler_messagebox_anykey =
		[this, &poller, &warning_text, &warning_color, &config_menu] (render_container &container) -> uint32_t
		{
			// draw a standard message window
			draw_text_box(container, warning_text, ui::text_layout::text_justify::LEFT, 0.5f, 0.5f, warning_color);

			if (machine().ui_input().pressed(IPT_UI_CANCEL))
			{
				// if the user cancels, exit out completely
				machine().schedule_exit();
				return HANDLER_CANCEL;
			}
			else if (machine().ui_input().pressed(IPT_UI_MENU))
			{
				config_menu = true;
				return HANDLER_CANCEL;
			}
			else if (poller.poll() != INPUT_CODE_INVALID)
			{
				// if any key is pressed, just exit
				return HANDLER_CANCEL;
			}

			return 0;
		};
	set_handler(ui_callback_type::GENERAL, handler_callback_func(&mame_ui_manager::handler_ingame, this));

	// loop over states
	for (int state = 0; state < maxstate && !machine().scheduled_event_pending() && !ui::menu::stack_has_special_main_menu(*this); state++)
	{
		// default to standard colors
		warning_color = colors().background_color();
		warning_text.clear();

		// pick the next state
		switch (state)
		{
		case 0:
			if (show_gameinfo)
				warning_text = machine_info().game_info_string();
			if (!warning_text.empty())
			{
				warning_text.append(_("\n\nPress any key to continue"));
				set_handler(ui_callback_type::MODAL, handler_callback_func(handler_messagebox_anykey));
			}
			break;

		case 1:
			warning_text = machine_info().warnings_string();
			m_has_warnings = !warning_text.empty();
			if (show_warnings)
			{
				bool need_warning = m_has_warnings;
				if (machine_info().has_severe_warnings() || !m_has_warnings)
				{
					// critical warnings - no need to persist stuff
					m_unemulated_features.clear();
					m_imperfect_features.clear();
					m_last_launch_time = std::time_t(-1);
					m_last_warning_time = std::time_t(-1);
				}
				else
				{
					// non-critical warnings - map current unemulated/imperfect features
					device_feature_set unemulated_features, imperfect_features;
					for (device_t &device : device_enumerator(machine().root_device()))
					{
						device_t::feature_type unemulated = device.type().unemulated_features();
						for (std::underlying_type_t<device_t::feature_type> feature = 1U; unemulated; feature <<= 1)
						{
							if (unemulated & feature)
							{
								char const *const name = info_xml_creator::feature_name(device_t::feature_type(feature));
								if (name)
									unemulated_features.emplace(device.type().shortname(), name);
								unemulated &= device_t::feature_type(~feature);
							}
						}
						device_t::feature_type imperfect = device.type().imperfect_features();
						for (std::underlying_type_t<device_t::feature_type> feature = 1U; imperfect; feature <<= 1)
						{
							if (imperfect & feature)
							{
								char const *const name = info_xml_creator::feature_name(device_t::feature_type(feature));
								if (name)
									imperfect_features.emplace(device.type().shortname(), name);
								imperfect &= device_t::feature_type(~feature);
							}
						}
					}

					// machine flags can cause warnings, too
					if (machine_info().machine_flags() & machine_flags::NO_COCKTAIL)
						unemulated_features.emplace(machine().root_device().type().shortname(), "cocktail");

					// if the warnings match what was shown sufficiently recently, it's skippable
					if ((unemulated_features != m_unemulated_features) || (imperfect_features != m_imperfect_features))
					{
						m_last_launch_time = std::time_t(-1);
					}
					else if (!machine().rom_load().warnings() && (std::time_t(-1) != m_last_launch_time) && (std::time_t(-1) != m_last_warning_time) && options().skip_warnings())
					{
						auto const now = std::chrono::system_clock::now();
						if (((std::chrono::system_clock::from_time_t(m_last_launch_time) + std::chrono::hours(7 * 24)) >= now) && ((std::chrono::system_clock::from_time_t(m_last_warning_time) + std::chrono::hours(14 * 24)) >= now))
							need_warning = false;
					}

					// update the information to save out
					m_unemulated_features = std::move(unemulated_features);
					m_imperfect_features = std::move(imperfect_features);
					if (need_warning)
						m_last_warning_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				}
				if (need_warning)
				{
					warning_text.append(_("\n\nPress any key to continue"));
					set_handler(ui_callback_type::MODAL, handler_callback_func(handler_messagebox_anykey));
					warning_color = machine_info().warnings_color();
				}
			}
			break;

		case 2:
			std::vector<std::reference_wrapper<const std::string>> mandatory_images = mame_machine_manager::instance()->missing_mandatory_images();
			if (!mandatory_images.empty())
			{
				std::ostringstream warning;
				if ((str > 0) || (machine().debug_flags & DEBUG_FLAG_ENABLED) || video_none)
				{
					warning << "Images must be mounted for the following devices: ";
					output_joined_collection(mandatory_images,
							[&warning] (const std::reference_wrapper<const std::string> &img) { warning << img.get(); },
							[&warning] () { warning << ", "; });

					throw emu_fatalerror(std::move(warning).str());
				}

				warning << _("This system requires media images to be mounted for the following device(s): ");
				output_joined_collection(mandatory_images,
						[&warning] (const std::reference_wrapper<const std::string> &img) { warning << '"' << img.get() << '"'; },
						[&warning] () { warning << ", "; });

				ui::menu_file_manager::force_file_manager(*this, machine().render().ui_container(), std::move(warning).str());
			}
			break;
		}

		// clear the input memory and wait for all keys to be released
		poller.reset();
		while (poller.poll() != INPUT_CODE_INVALID) { }

		if (m_handler_callback_type == ui_callback_type::MODAL)
		{
			config_menu = false;

			// loop while we have a handler
			while (m_handler_callback_type == ui_callback_type::MODAL && !machine().scheduled_event_pending() && !ui::menu::stack_has_special_main_menu(*this))
				machine().video().frame_update();
		}

		// clear the handler and force an update
		set_handler(ui_callback_type::GENERAL, handler_callback_func(&mame_ui_manager::handler_ingame, this));
		machine().video().frame_update();
	}

	// update last launch time if this was a run that was eligible for emulation warnings
	if (m_has_warnings && show_warnings && !machine().scheduled_event_pending())
		m_last_launch_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	// if we're the empty driver, force the menus on
	if (ui::menu::stack_has_special_main_menu(*this))
	{
		show_menu();
	}
	else if (config_menu)
	{
		show_menu();

		// loop while we have a handler
		while (m_handler_callback_type != ui_callback_type::GENERAL && !machine().scheduled_event_pending())
			machine().video().frame_update();
	}
}


//-------------------------------------------------
//  set_startup_text - set the text to display
//  at startup
//-------------------------------------------------

void mame_ui_manager::set_startup_text(const char *text, bool force)
{
	static osd_ticks_t lastupdatetime = 0;
	osd_ticks_t curtime = osd_ticks();

	// copy in the new text
	messagebox_text.assign(text);

	// don't update more than 4 times/second
	if (force || (curtime - lastupdatetime) > osd_ticks_per_second() / 4)
	{
		lastupdatetime = curtime;
		machine().video().frame_update();
	}
}


//-------------------------------------------------
//  update_and_render - update the UI and
//  render it; called by video.c
//-------------------------------------------------

bool mame_ui_manager::update_and_render(render_container &container)
{
	// always start clean
	for (auto &target : machine().render().targets())
	{
		if (target.ui_container())
			target.ui_container()->empty();
	}

	// if we're paused, dim the whole screen
	if (machine().phase() >= machine_phase::RESET && (single_step() || machine().paused()))
	{
		int alpha = (1.0f - machine().options().pause_brightness()) * 255.0f;
		if (ui::menu::stack_has_special_main_menu(*this))
			alpha = 255;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			container.add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(alpha,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	// render any cheat stuff at the bottom
	if (machine().phase() >= machine_phase::RESET)
		mame_machine_manager::instance()->cheat().render_text(*this, container);

	// draw the FPS counter if it should be visible
	if (show_fps_counter())
		draw_fps_counter(container);

	// call the current UI handler
	machine().ui_input().check_ui_inputs();
	uint32_t const handler_result = m_handler_callback(container);

	// display any popup messages
	if (osd_ticks() < m_popup_text_end)
		draw_text_box(container, messagebox_poptext, ui::text_layout::text_justify::CENTER, 0.5f, 0.9f, colors().background_color());
	else
		m_popup_text_end = 0;

	// display the internal pointers
	bool const pointer_update = m_pointers_changed;
	m_pointers_changed = false;
	if (!is_menu_active() || machine().options().ui_mouse())
	{
		const float cursor_size = 0.6 * get_line_height();
		for (auto const &pointer : m_display_pointers)
		{
			render_container &container = *pointer.target.get().ui_container();
			container.add_quad(
					pointer.x,
					pointer.y,
					pointer.x + cursor_size * container.manager().ui_aspect(&container),
					pointer.y + cursor_size,
					rgb_t::white(),
					m_mouse_arrow_texture,
					PRIMFLAG_ANTIALIAS(1) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}

	// cancel takes us back to the in-game handler
	if (handler_result & HANDLER_CANCEL)
	{
		machine().ui_input().reset();
		set_handler(ui_callback_type::GENERAL, handler_callback_func(&mame_ui_manager::handler_ingame, this));
	}

	return pointer_update || (handler_result & HANDLER_UPDATE);
}


//-------------------------------------------------
//  get_font - return the UI font
//-------------------------------------------------

render_font *mame_ui_manager::get_font()
{
	// allocate the font and messagebox string
	if (!m_font)
		m_font = machine().render().font_alloc(machine().options().ui_font());
	return m_font.get();
}


//-------------------------------------------------
//  get_line_height - return the current height
//  of a line
//-------------------------------------------------

float mame_ui_manager::get_line_height(float scale)
{
	int32_t const raw_font_pixel_height = get_font()->pixel_height();
	float target_pixel_height = machine().render().ui_target().height();

	// compute the font pixel height at the nominal size
	float const one_to_one_line_height = float(raw_font_pixel_height) / target_pixel_height;

	// determine the scale factor
	float scale_factor = target_font_height() * scale / one_to_one_line_height;

	// if our font is small-ish, do integral scaling
	if (raw_font_pixel_height < 24)
	{
		// do we want to scale smaller? only do so if we exceed the threshold
		if (scale_factor <= 1.0f)
		{
			if (one_to_one_line_height < UI_MAX_FONT_HEIGHT || raw_font_pixel_height < 12)
				scale_factor = 1.0f;
		}
		else
		{
			// otherwise, just ensure an integral scale factor
			scale_factor = floorf(scale_factor);
		}
	}
	else
	{
		// otherwise, just make sure we hit an even number of pixels
		int32_t height = scale_factor * one_to_one_line_height * target_pixel_height;
		scale_factor = float(height) / (one_to_one_line_height * target_pixel_height);
	}

	return scale_factor * one_to_one_line_height;
}


//-------------------------------------------------
//  get_char_width - return the width of a
//  single character
//-------------------------------------------------

float mame_ui_manager::get_char_width(char32_t ch)
{
	return get_font()->char_width(get_line_height(), machine().render().ui_aspect(), ch);
}


//-------------------------------------------------
//  get_string_width - return the width of a
//  character string
//-------------------------------------------------

float mame_ui_manager::get_string_width(std::string_view s)
{
	return get_string_width(s, get_line_height());
}

float mame_ui_manager::get_string_width(std::string_view s, float text_size)
{
	return get_font()->utf8string_width(text_size, machine().render().ui_aspect(), s);
}


//-------------------------------------------------
//  draw_outlined_box - add primitives to draw
//  an outlined box with the given background
//  color
//-------------------------------------------------

void mame_ui_manager::draw_outlined_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t backcolor)
{
	draw_outlined_box(container, x0, y0, x1, y1, colors().border_color(), backcolor);
}


//-------------------------------------------------
//  draw_outlined_box - add primitives to draw
//  an outlined box with the given background
//  color
//-------------------------------------------------

void mame_ui_manager::draw_outlined_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t fgcolor, rgb_t bgcolor)
{
	container.add_rect(x0, y0, x1, y1, bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x0, y0, x1, y0, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x1, y0, x1, y1, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x1, y1, x0, y1, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x0, y1, x0, y0, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


//-------------------------------------------------
//  draw_text - simple text renderer
//-------------------------------------------------

void mame_ui_manager::draw_text(render_container &container, std::string_view buf, float x, float y)
{
	draw_text_full(
			container,
			buf,
			x, y, 1.0f - x,
			ui::text_layout::text_justify::LEFT, ui::text_layout::word_wrapping::WORD,
			mame_ui_manager::NORMAL, colors().text_color(), colors().text_bg_color(), nullptr, nullptr);
}


//-------------------------------------------------
//  draw_text_full - full featured text
//  renderer with word wrapping, justification,
//  and full size computation
//-------------------------------------------------

void mame_ui_manager::draw_text_full(
		render_container &container,
		std::string_view origs,
		float x, float y, float origwrapwidth,
		ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap,
		draw_mode draw, rgb_t fgcolor, rgb_t bgcolor,
		float *totalwidth, float *totalheight)
{
	draw_text_full(
			container,
			origs,
			x, y, origwrapwidth,
			justify, wrap,
			draw, fgcolor, bgcolor,
			totalwidth, totalheight,
			get_line_height());
}

void mame_ui_manager::draw_text_full(
		render_container &container,
		std::string_view origs,
		float x, float y, float origwrapwidth,
		ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap,
		draw_mode draw, rgb_t fgcolor, rgb_t bgcolor,
		float *totalwidth, float *totalheight,
		float text_size)
{
	// create the layout
	ui::text_layout layout(
			*get_font(), machine().render().ui_aspect(&container) * text_size, text_size,
			origwrapwidth, justify, wrap);

	// append text to it
	layout.add_text(
			origs,
			fgcolor,
			(draw == OPAQUE_) ? bgcolor : rgb_t::transparent());

	// and emit it (if we are asked to do so)
	if (draw != NONE)
		layout.emit(container, x, y);

	// return width/height
	if (totalwidth)
		*totalwidth = layout.actual_width();
	if (totalheight)
		*totalheight = layout.actual_height();
}


//-------------------------------------------------
//  draw_text_box - draw a multiline text
//  message with a box around it
//-------------------------------------------------

void mame_ui_manager::draw_text_box(render_container &container, std::string_view text, ui::text_layout::text_justify justify, float xpos, float ypos, rgb_t backcolor)
{
	// cap the maximum width
	float maximum_width = 1.0f - (box_lr_border() * machine().render().ui_aspect(&container) * 2.0f);

	// create a layout
	ui::text_layout layout = create_layout(container, maximum_width, justify);

	// add text to it
	layout.add_text(text);

	// and draw the result
	draw_text_box(container, layout, xpos, ypos, backcolor);
}


//-------------------------------------------------
//  draw_text_box - draw a multiline text
//  message with a box around it
//-------------------------------------------------

void mame_ui_manager::draw_text_box(render_container &container, ui::text_layout &layout, float xpos, float ypos, rgb_t backcolor)
{
	// xpos and ypos are where we want to "pin" the layout, but we need to adjust for the actual size of the payload
	auto const lrborder = box_lr_border() * machine().render().ui_aspect(&container);
	auto const actual_left = layout.actual_left();
	auto const actual_width = layout.actual_width();
	auto const actual_height = layout.actual_height();
	auto const x = std::clamp(xpos - actual_width / 2, lrborder, 1.0f - actual_width - lrborder);
	auto const y = std::clamp(ypos - actual_height / 2, box_tb_border(), 1.0f - actual_height - box_tb_border());

	// add a box around that
	draw_outlined_box(
			container,
			x - lrborder, y - box_tb_border(),
			x + actual_width + lrborder, y + actual_height + box_tb_border(),
			backcolor);

	// emit the text
	layout.emit(container, x - actual_left, y);
}


//-------------------------------------------------
//  draw_message_window - draw a multiline text
//  message with a box around it
//-------------------------------------------------

void mame_ui_manager::draw_message_window(render_container &container, std::string_view text)
{
	draw_text_box(container, text, ui::text_layout::text_justify::LEFT, 0.5f, 0.5f, colors().background_color());
}


//-------------------------------------------------
//  show_fps_temp - show the FPS counter for
//  a specific period of time
//-------------------------------------------------

void mame_ui_manager::show_fps_temp(double seconds)
{
	if (!m_showfps)
		m_showfps_end = std::max<osd_ticks_t>(osd_ticks() + seconds * osd_ticks_per_second(), m_showfps_end);
}


//-------------------------------------------------
//  set_show_fps - show/hide the FPS counter
//-------------------------------------------------

void mame_ui_manager::set_show_fps(bool show)
{
	m_showfps = show;
	if (!show)
		m_showfps_end = 0;
}


//-------------------------------------------------
//  show_fps - return the current FPS
//  counter visibility state
//-------------------------------------------------

bool mame_ui_manager::show_fps() const
{
	return m_showfps || (m_showfps_end != 0);
}


//-------------------------------------------------
//  show_fps_counter
//-------------------------------------------------

bool mame_ui_manager::show_fps_counter()
{
	bool const result = m_showfps || (osd_ticks() < m_showfps_end);
	if (!result)
		m_showfps_end = 0;
	return result;
}


//-------------------------------------------------
//  set_show_profiler - show/hide the profiler
//-------------------------------------------------

void mame_ui_manager::set_show_profiler(bool show)
{
	m_show_profiler = show;
	g_profiler.enable(show);
}


//-------------------------------------------------
//  show_profiler - return the current
//  profiler visibility state
//-------------------------------------------------

bool mame_ui_manager::show_profiler() const
{
	return m_show_profiler;
}


//-------------------------------------------------
//  show_menu - show the menus
//-------------------------------------------------

void mame_ui_manager::show_menu()
{
	for (auto *target = machine().render().first_target(); target; target = target->next())
	{
		if (!target->hidden())
			target->forget_pointers();
	}

	set_handler(ui_callback_type::MENU, ui::menu::get_ui_handler(*this));
}


//-------------------------------------------------
//  is_menu_active - return true if the menu
//  UI handler is active
//-------------------------------------------------

bool mame_ui_manager::is_menu_active()
{
	return m_handler_callback_type == ui_callback_type::MENU;
}



/***************************************************************************
    UI HANDLERS
***************************************************************************/

//-------------------------------------------------
//  process_ui_events - processes queued UI input
//  events
//-------------------------------------------------

void mame_ui_manager::process_ui_events()
{
	// process UI events
	bool const use_natkbd(machine().natkeyboard().in_use() && (machine().phase() == machine_phase::RUNNING));
	ui_event event;
	while (machine().ui_input().pop_event(&event))
	{
		switch (event.event_type)
		{
		case ui_event::type::NONE:
		case ui_event::type::WINDOW_FOCUS:
		case ui_event::type::WINDOW_DEFOCUS:
		case ui_event::type::MOUSE_WHEEL:
			break;

		case ui_event::type::POINTER_UPDATE:
			if (event.target)
			{
				if (osd::ui_event_handler::pointer::TOUCH != event.pointer_type)
				{
					auto pos(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), std::make_pair(event.target, event.pointer_id)));
					if ((m_active_pointers.end() == pos) || (pos->target != event.target) || (pos->ptrid != event.pointer_id))
						pos = m_active_pointers.emplace(pos, event);
					else
						assert(pos->type == event.pointer_type);
					pos->updated = std::chrono::steady_clock::now();
					event.target->map_point_container(event.pointer_x, event.pointer_y, *event.target->ui_container(), pos->x, pos->y);
				}

				event.target->pointer_updated(
						event.pointer_type, event.pointer_id, event.pointer_device,
						event.pointer_x, event.pointer_y,
						event.pointer_buttons, event.pointer_pressed, event.pointer_released,
						event.pointer_clicks);
			}
			break;

		case ui_event::type::POINTER_LEAVE:
			if (event.target)
			{
				auto const pos(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), std::make_pair(event.target, event.pointer_id)));
				if (m_active_pointers.end() != pos)
					m_active_pointers.erase(pos);

				event.target->pointer_left(
						event.pointer_type, event.pointer_id, event.pointer_device,
						event.pointer_x, event.pointer_y,
						event.pointer_released,
						event.pointer_clicks);
			}
			break;

		case ui_event::type::POINTER_ABORT:
			if (event.target)
			{
				auto const pos(std::lower_bound(m_active_pointers.begin(), m_active_pointers.end(), std::make_pair(event.target, event.pointer_id)));
				if (m_active_pointers.end() != pos)
					m_active_pointers.erase(pos);

				event.target->pointer_aborted(
						event.pointer_type, event.pointer_id, event.pointer_device,
						event.pointer_x, event.pointer_y,
						event.pointer_released,
						event.pointer_clicks);
			}
			break;

		case ui_event::type::IME_CHAR:
			if (use_natkbd)
				machine().natkeyboard().post_char(event.ch);
			break;
		}
	}

	// process natural keyboard keys that don't get IME text input events
	if (use_natkbd)
	{
		for (int i = 0; i < std::size(non_char_keys); i++)
		{
			// identify this keycode
			input_item_id itemid = non_char_keys[i];
			input_code code = machine().input().code_from_itemid(itemid);

			// ...and determine if it is pressed
			bool pressed = machine().input().code_pressed(code);

			// figure out whey we are in the key_down map
			uint8_t *key_down_ptr = &m_non_char_keys_down[i / 8];
			uint8_t key_down_mask = 1 << (i % 8);

			if (pressed && !(*key_down_ptr & key_down_mask))
			{
				// this key is now down
				*key_down_ptr |= key_down_mask;

				// post the key
				machine().natkeyboard().post_char(UCHAR_MAMEKEY_BEGIN + code.item_id());
			}
			else if (!pressed && (*key_down_ptr & key_down_mask))
			{
				// this key is now up
				*key_down_ptr &= ~key_down_mask;
			}
		}
	}
}


//-------------------------------------------------
//  increase_frameskip
//-------------------------------------------------

void mame_ui_manager::increase_frameskip()
{
	// get the current value and increment it
	int newframeskip = machine().video().frameskip() + 1;
	if (newframeskip > MAX_FRAMESKIP)
		newframeskip = -1;
	machine().video().set_frameskip(newframeskip);

	// display the FPS counter for 2 seconds
	show_fps_temp(2.0);
}


//-------------------------------------------------
//  decrease_frameskip
//-------------------------------------------------

void mame_ui_manager::decrease_frameskip()
{
	// get the current value and decrement it
	int newframeskip = machine().video().frameskip() - 1;
	if (newframeskip < -1)
		newframeskip = MAX_FRAMESKIP;
	machine().video().set_frameskip(newframeskip);

	// display the FPS counter for 2 seconds
	show_fps_temp(2.0);
}


//-------------------------------------------------
//  can_paste
//-------------------------------------------------

bool mame_ui_manager::can_paste()
{
	// check to see if the clipboard is not empty
	// FIXME: this is expensive - need a cheaper way to check if clipboard contains suitable content
	return !osd_get_clipboard_text().empty();
}


//-------------------------------------------------
//  draw_fps_counter
//-------------------------------------------------

void mame_ui_manager::draw_fps_counter(render_container &container)
{
	draw_text_full(
			container,
			machine().video().speed_text(),
			0.0f, 0.0f, 1.0f,
			ui::text_layout::text_justify::RIGHT, ui::text_layout::word_wrapping::WORD,
			OPAQUE_, rgb_t::white(), rgb_t::black(), nullptr, nullptr);
}


//-------------------------------------------------
//  draw_profiler
//-------------------------------------------------

void mame_ui_manager::draw_profiler(render_container &container)
{
	std::string_view text = g_profiler.text(machine());
	draw_text_full(
			container,
			text,
			0.0f, 0.0f, 1.0f,
			ui::text_layout::text_justify::LEFT, ui::text_layout::word_wrapping::WORD,
			OPAQUE_, rgb_t::white(), rgb_t::black(), nullptr, nullptr);
}


//-------------------------------------------------
//  image_handler_ingame - execute display
//  callback function for each image device
//-------------------------------------------------

void mame_ui_manager::image_handler_ingame()
{
	// run display routine for devices
	if (m_image_display_enabled && machine().phase() == machine_phase::RUNNING)
	{
		auto layout = create_layout(machine().render().ui_container());

		// loop through all devices, build their text into the layout
		for (device_image_interface &image : image_interface_enumerator(machine().root_device()))
		{
			std::string str = image.call_display();
			if (!str.empty())
			{
				layout.add_text(str);
				layout.add_text("\n");
			}
		}

		// did we actually create anything?
		if (!layout.empty())
		{
			float x = 0.2f;
			float y = 0.5f * get_line_height() + 2.0f * box_tb_border();
			draw_text_box(machine().render().ui_container(), layout, x, y, colors().background_color());
		}
	}
}

//-------------------------------------------------
//  handler_ingame - in-game handler takes care
//  of the standard keypresses
//-------------------------------------------------

uint32_t mame_ui_manager::handler_ingame(render_container &container)
{
	// let the OSD do its thing first
	machine().osd().check_osd_inputs();

	bool is_paused = machine().paused();

	// draw the profiler if visible
	if (show_profiler())
		draw_profiler(container);

	// if we're single-stepping, pause now
	if (single_step())
	{
		machine().pause();
		set_single_step(false);
	}

	// determine if we should disable the rest of the UI
	bool const has_keyboard = machine_info().has_keyboard();
	bool const ui_disabled = !ui_active();

	// is ScrLk UI toggling applicable here?
	if (has_keyboard)
	{
		// are we toggling the UI with ScrLk?
		if (machine().ui_input().pressed(IPT_UI_TOGGLE_UI))
		{
			// toggle the UI
			set_ui_active(!ui_active());

			// display a popup indicating the new status
			std::string const name = get_general_input_setting(IPT_UI_TOGGLE_UI);
			if (ui_active())
				popup_time(2, _("UI controls enabled\nUse %1$s to toggle"), name);
			else
				popup_time(2, _("UI controls disabled\nUse %1$s to toggle"), name);
		}
	}

	// process UI events and update pointers if necessary
	process_ui_events();
	display_pointer_vector pointers;
	pointers.reserve(m_active_pointers.size());
	auto const now(std::chrono::steady_clock::now());
	auto expiry(now);
	render_target *target(nullptr);
	layout_view const *view(nullptr);
	bool hide_inactive(true);
	for (auto const &pointer : m_active_pointers)
	{
		if (pointer.target != target)
		{
			target = pointer.target;
			view = &target->current_view();
			hide_inactive = m_pointer_options[target->index()].hide_inactive() && view->hide_inactive_pointers();
			expiry = now - m_pointer_options[target->index()].timeout();
		}
		if (view->show_pointers())
		{
			if (!hide_inactive || (osd::ui_event_handler::pointer::PEN == pointer.type) || (pointer.updated >= expiry))
				pointers.emplace_back(display_pointer{ *target, pointer.type, pointer.x, pointer.y });
		}
	}
	set_pointers(pointers.begin(), pointers.end());

	if (!ui_disabled)
	{
		// paste command
		if (machine().ui_input().pressed(IPT_UI_PASTE))
			machine().natkeyboard().paste();
	}

	image_handler_ingame();

	if (ui_disabled)
		return 0;

	if (machine().ui_input().pressed(IPT_UI_CANCEL))
	{
		request_quit();
		return 0;
	}

	// turn on menus if requested
	if (machine().ui_input().pressed(IPT_UI_MENU))
	{
		show_menu();
		return 0;
	}

	// if the on-screen display isn't up and the user has toggled it, turn it on
	if (!(machine().debug_flags & DEBUG_FLAG_ENABLED) && machine().ui_input().pressed(IPT_UI_ON_SCREEN_DISPLAY))
	{
		ui::menu::stack_push<ui::menu_sliders>(*this, machine().render().ui_container(), true);
		show_menu();
		return 0;
	}

	// handle a reset request
	if (machine().ui_input().pressed(IPT_UI_RESET_MACHINE))
		machine().schedule_hard_reset();
	if (machine().ui_input().pressed(IPT_UI_SOFT_RESET))
		machine().schedule_soft_reset();

	// handle a request to display graphics/palette
	if (machine().ui_input().pressed(IPT_UI_SHOW_GFX))
	{
		for (auto *target = machine().render().first_target(); target; target = target->next())
		{
			if (!target->hidden())
				target->forget_pointers();
		}

		if (!is_paused)
			machine().pause();
		using namespace std::placeholders;
		set_handler(
				ui_callback_type::MENU,
				handler_callback_func(
					[this, is_paused] (render_container &container) -> uint32_t
					{
						return ui_gfx_ui_handler(container, *this, is_paused);
					}));
		return 0;
	}

	// handle a tape control key
	if (machine().ui_input().pressed(IPT_UI_TAPE_START))
	{
		for (cassette_image_device &cass : cassette_device_enumerator(machine().root_device()))
		{
			cass.change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
			return 0;
		}
	}
	if (machine().ui_input().pressed(IPT_UI_TAPE_STOP))
	{
		for (cassette_image_device &cass : cassette_device_enumerator(machine().root_device()))
		{
			cass.change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
			return 0;
		}
	}

	// handle a save state request
	if (machine().ui_input().pressed(IPT_UI_SAVE_STATE))
	{
		ui::menu::stack_push<ui::menu_save_state>(*this, machine().render().ui_container(), true);
		show_menu();
		return 0;
	}

	// handle a load state request
	if (machine().ui_input().pressed(IPT_UI_LOAD_STATE))
	{
		ui::menu::stack_push<ui::menu_load_state>(*this, machine().render().ui_container(), true);
		show_menu();
		return 0;
	}

	// handle a quick save state request
	if (machine().ui_input().pressed(IPT_UI_SAVE_STATE_QUICK))
	{
		machine().schedule_save("quick");
		return 0;
	}

	// handle a quick load state request
	if (machine().ui_input().pressed(IPT_UI_LOAD_STATE_QUICK))
	{
		machine().schedule_load("quick");
		return 0;
	}

	// handle a save snapshot request
	if (machine().ui_input().pressed(IPT_UI_SNAPSHOT))
		machine().video().save_active_screen_snapshots();

	// toggle pause
	if (machine().ui_input().pressed(IPT_UI_PAUSE))
		machine().toggle_pause();

	// pause single step
	if (machine().ui_input().pressed(IPT_UI_PAUSE_SINGLE))
	{
		machine().rewind_capture();
		set_single_step(true);
		machine().resume();
	}

	// rewind single step
	if (machine().ui_input().pressed(IPT_UI_REWIND_SINGLE))
		machine().rewind_step();

	// handle a toggle cheats request
	if (machine().ui_input().pressed(IPT_UI_TOGGLE_CHEAT))
		mame_machine_manager::instance()->cheat().set_enable(!mame_machine_manager::instance()->cheat().enabled(), true);

	// toggle MNG recording
	if (machine().ui_input().pressed(IPT_UI_RECORD_MNG))
		machine().video().toggle_record_movie(movie_recording::format::MNG);

	// toggle AVI recording
	if (machine().ui_input().pressed(IPT_UI_RECORD_AVI))
		machine().video().toggle_record_movie(movie_recording::format::AVI);

	// toggle profiler display
	if (machine().ui_input().pressed(IPT_UI_SHOW_PROFILER))
		set_show_profiler(!show_profiler());

	// toggle FPS display
	if (machine().ui_input().pressed(IPT_UI_SHOW_FPS))
		set_show_fps(!show_fps());

	// increment frameskip?
	if (machine().ui_input().pressed(IPT_UI_FRAMESKIP_INC))
		increase_frameskip();

	// decrement frameskip?
	if (machine().ui_input().pressed(IPT_UI_FRAMESKIP_DEC))
		decrease_frameskip();

	// toggle throttle?
	if (machine().ui_input().pressed(IPT_UI_THROTTLE))
	{
		const bool new_throttle_state = !machine().video().throttled();
		machine().video().set_throttled(new_throttle_state);
		if (m_unthrottle_mute)
			machine().sound().ui_mute(!new_throttle_state);
	}

	// check for fast forward
	if (machine().ioport().type_pressed(IPT_UI_FAST_FORWARD))
	{
		machine().video().set_fastforward(true);
		show_fps_temp(0.5);
	}
	else
		machine().video().set_fastforward(false);

	return 0;
}


//-------------------------------------------------
//  request_quit
//-------------------------------------------------

void mame_ui_manager::request_quit()
{
	if (!machine().options().confirm_quit())
	{
		machine().schedule_exit();
	}
	else
	{
		ui::menu::stack_push<ui::menu_confirm_quit>(*this, machine().render().ui_container());
		show_menu();
	}
}


//-------------------------------------------------
//  set_pointer_activity_timeout - set per-target
//  pointer activity timeout
//-------------------------------------------------

void mame_ui_manager::set_pointer_activity_timeout(int target, std::chrono::steady_clock::duration timeout) noexcept
{
	assert((0 <= target) && (m_pointer_options.size() > target));
	if ((0 <= target) && (m_pointer_options.size() > target))
		m_pointer_options[target].set_timeout(timeout);
}


//-------------------------------------------------
//  set_hide_inactive_pointers - set per-target
//  hide inactive pointers setting
//-------------------------------------------------

void mame_ui_manager::set_hide_inactive_pointers(int target, bool hide) noexcept
{
	assert((0 <= target) && (m_pointer_options.size() > target));
	if ((0 <= target) && (m_pointer_options.size() > target))
		m_pointer_options[target].set_hide_inactive(hide);
}


//-------------------------------------------------
//  restore_initial_pointer_options - restore
//  initial per-target pointer settings
//-------------------------------------------------

void mame_ui_manager::restore_initial_pointer_options(int target) noexcept
{
	assert((0 <= target) && (m_pointer_options.size() > target));
	if ((0 <= target) && (m_pointer_options.size() > target))
		m_pointer_options[target].restore_initial();
}


//-------------------------------------------------
//  pointer_activity_timeout - get per-target
//  pointer activity timeout
//-------------------------------------------------

std::chrono::steady_clock::duration mame_ui_manager::pointer_activity_timeout(int target) const noexcept
{
	assert((0 <= target) && (m_pointer_options.size() > target));
	if ((0 <= target) && (m_pointer_options.size() > target))
		return m_pointer_options[target].timeout();
	else
		return pointer_options().timeout();
}


//-------------------------------------------------
//  hide_inactive_pointers - get per-target hide
//  inactive pointers setting
//-------------------------------------------------

bool mame_ui_manager::hide_inactive_pointers(int target) const noexcept
{
	assert((0 <= target) && (m_pointer_options.size() > target));
	if ((0 <= target) && (m_pointer_options.size() > target))
		return m_pointer_options[target].hide_inactive();
	else
		return pointer_options().hide_inactive();
}



/***************************************************************************
    SLIDER CONTROLS
***************************************************************************/

//-------------------------------------------------
//  ui_get_slider_list - get the list of sliders
//-------------------------------------------------

std::vector<ui::menu_item> &mame_ui_manager::get_slider_list()
{
	return slider_list;
}


//----------------------------------------------------------
//  mame_ui_manager::slider_init - initialize the list of slider
//  controls
//----------------------------------------------------------

std::vector<ui::menu_item> mame_ui_manager::slider_init(running_machine &machine)
{
	using namespace std::placeholders;

	m_sliders.clear();

	// add overall volume
	slider_alloc(_("Master Volume"), -48, osd::linear_to_db(0), 6, 1, std::bind(&mame_ui_manager::slider_volume, this, _1, _2));

	// add per-sound device and per-sound device channel volume
	for (device_sound_interface &snd : sound_interface_enumerator(machine.root_device()))
	{
		// Don't add microphones, speakers or devices without outputs
		if (dynamic_cast<sound_io_device *>(&snd) || !snd.outputs())
			continue;

		slider_alloc(util::string_format(_("%1$s volume"), snd.device().tag()), -48, osd::linear_to_db_int(snd.user_output_gain()), 6, 1, std::bind(&mame_ui_manager::slider_devvol, this, &snd, _1, _2));
		if (snd.outputs() != 1)
			for (int channel = 0; channel != snd.outputs(); channel ++)
				slider_alloc(util::string_format(_("%1$s channel %d volume"), snd.device().tag(), channel), -48, osd::linear_to_db_int(snd.user_output_gain(channel)), 6, 1, std::bind(&mame_ui_manager::slider_devvol_chan, this, &snd, channel, _1, _2));
	}

	// add analog adjusters
	for (auto &port : machine.ioport().ports())
	{
		for (ioport_field &field : port.second->fields())
		{
			if (field.type() == IPT_ADJUSTER)
			{
				slider_alloc(field.name(), field.minval(), field.defvalue(), field.maxval(), 1, std::bind(&mame_ui_manager::slider_adjuster, this, std::ref(field), _1, _2));
			}
		}
	}

	// add CPU overclocking (cheat only)
	if (machine.options().cheat())
	{
		for (device_execute_interface &exec : execute_interface_enumerator(machine.root_device()))
		{
			std::string str = string_format(_("Overclock CPU %1$s"), exec.device().tag());
			slider_alloc(std::move(str), 100, 1000, 4000, 10, std::bind(&mame_ui_manager::slider_overclock, this, std::ref(exec.device()), _1, _2));
		}
		for (device_sound_interface &snd : sound_interface_enumerator(machine.root_device()))
		{
			device_execute_interface *exec;
			if (!snd.device().interface(exec) && snd.device().unscaled_clock() != 0)
			{
				std::string str = string_format(_("Overclock %1$s sound"), snd.device().tag());
				slider_alloc(std::move(str), 100, 1000, 4000, 10, std::bind(&mame_ui_manager::slider_overclock, this, std::ref(snd.device()), _1, _2));
			}
		}
	}

	// add screen parameters
	screen_device_enumerator scriter(machine.root_device());
	for (screen_device &screen : scriter)
	{
		int defxscale = floorf(screen.xscale() * 1000.0f + 0.5f);
		int defyscale = floorf(screen.yscale() * 1000.0f + 0.5f);
		int defxoffset = floorf(screen.xoffset() * 1000.0f + 0.5f);
		int defyoffset = floorf(screen.yoffset() * 1000.0f + 0.5f);
		std::string screen_desc = machine_info().get_screen_desc(screen);

		// add refresh rate tweaker
		if (machine.options().cheat())
		{
			std::string str = string_format(_("%1$s Refresh Rate"), screen_desc);
			slider_alloc(std::move(str), -10000, 0, 10000, 1000, std::bind(&mame_ui_manager::slider_refresh, this, std::ref(screen), _1, _2));
		}

		// add standard brightness/contrast/gamma controls per-screen
		std::string str = string_format(_("%1$s Brightness"), screen_desc);
		slider_alloc(std::move(str), 100, 1000, 2000, 10, std::bind(&mame_ui_manager::slider_brightness, this, std::ref(screen), _1, _2));
		str = string_format(_("%1$s Contrast"), screen_desc);
		slider_alloc(std::move(str), 100, 1000, 2000, 50, std::bind(&mame_ui_manager::slider_contrast, this, std::ref(screen), _1, _2));
		str = string_format(_("%1$s Gamma"), screen_desc);
		slider_alloc(std::move(str), 100, 1000, 3000, 50, std::bind(&mame_ui_manager::slider_gamma, this, std::ref(screen), _1, _2));

		// add scale and offset controls per-screen
		str = string_format(_("%1$s Horiz Stretch"), screen_desc);
		slider_alloc(std::move(str), 500, defxscale, 1500, 2, std::bind(&mame_ui_manager::slider_xscale, this, std::ref(screen), _1, _2));
		str = string_format(_("%1$s Horiz Position"), screen_desc);
		slider_alloc(std::move(str), -500, defxoffset, 500, 2, std::bind(&mame_ui_manager::slider_xoffset, this, std::ref(screen), _1, _2));
		str = string_format(_("%1$s Vert Stretch"), screen_desc);
		slider_alloc(std::move(str), 500, defyscale, 1500, 2, std::bind(&mame_ui_manager::slider_yscale, this, std::ref(screen), _1, _2));
		str = string_format(_("%1$s Vert Position"), screen_desc);
		slider_alloc(std::move(str), -500, defyoffset, 500, 2, std::bind(&mame_ui_manager::slider_yoffset, this, std::ref(screen), _1, _2));
	}

	for (laserdisc_device &laserdisc : laserdisc_device_enumerator(machine.root_device()))
	{
		if (laserdisc.overlay_configured())
		{
			laserdisc_overlay_config config;
			laserdisc.get_overlay_config(config);
			int defxscale = floorf(config.m_overscalex * 1000.0f + 0.5f);
			int defyscale = floorf(config.m_overscaley * 1000.0f + 0.5f);
			int defxoffset = floorf(config.m_overposx * 1000.0f + 0.5f);
			int defyoffset = floorf(config.m_overposy * 1000.0f + 0.5f);

			// add scale and offset controls per-overlay
			std::string str = string_format(_("Laserdisc '%1$s' Horiz Stretch"), laserdisc.tag());
			slider_alloc(std::move(str), 500, (defxscale == 0) ? 1000 : defxscale, 1500, 2, std::bind(&mame_ui_manager::slider_overxscale, this, std::ref(laserdisc), _1, _2));
			str = string_format(_("Laserdisc '%1$s' Horiz Position"), laserdisc.tag());
			slider_alloc(std::move(str), -500, defxoffset, 500, 2, std::bind(&mame_ui_manager::slider_overxoffset, this, std::ref(laserdisc), _1, _2));
			str = string_format(_("Laserdisc '%1$s' Vert Stretch"), laserdisc.tag());
			slider_alloc(std::move(str), 500, (defyscale == 0) ? 1000 : defyscale, 1500, 2, std::bind(&mame_ui_manager::slider_overyscale, this, std::ref(laserdisc), _1, _2));
			str = string_format(_("Laserdisc '%1$s' Vert Position"), laserdisc.tag());
			slider_alloc(std::move(str), -500, defyoffset, 500, 2, std::bind(&mame_ui_manager::slider_overyoffset, this, std::ref(laserdisc), _1, _2));
		}
	}

	for (screen_device &screen : scriter)
	{
		if (screen.screen_type() == SCREEN_TYPE_VECTOR)
		{
			// add vector control (FIXME: these should all be per-screen rather than global)
			slider_alloc(_("Vector Flicker"), 0, 0, 1000, 10, std::bind(&mame_ui_manager::slider_flicker, this, std::ref(screen), _1, _2));
			slider_alloc(_("Beam Width Minimum"), 100, 100, 1000, 1, std::bind(&mame_ui_manager::slider_beam_width_min, this, std::ref(screen), _1, _2));
			slider_alloc(_("Beam Width Maximum"), 100, 100, 1000, 1, std::bind(&mame_ui_manager::slider_beam_width_max, this, std::ref(screen), _1, _2));
			slider_alloc(_("Beam Dot Size"), 100, 100, 1000, 1,  std::bind(&mame_ui_manager::slider_beam_dot_size, this, std::ref(screen), _1, _2));
			slider_alloc(_("Beam Intensity Weight"), -1000, 0, 1000, 10, std::bind(&mame_ui_manager::slider_beam_intensity_weight, this, std::ref(screen), _1, _2));
			break;
		}
	}

#ifdef MAME_DEBUG
	// add crosshair adjusters
	for (auto &port : machine.ioport().ports())
	{
		for (ioport_field &field : port.second->fields())
		{
			if (field.crosshair_axis() != CROSSHAIR_AXIS_NONE && field.player() == 0)
			{
				std::string str = string_format(_("Crosshair Scale %1$s"), (field.crosshair_axis() == CROSSHAIR_AXIS_X) ? _("X") : _("Y"));
				slider_alloc(std::move(str), -3000, 1000, 3000, 100, std::bind(&mame_ui_manager::slider_crossscale, this, std::ref(field), _1, _2));
				str = string_format(_("Crosshair Offset %1$s"), (field.crosshair_axis() == CROSSHAIR_AXIS_X) ? _("X") : _("Y"));
				slider_alloc(std::move(str), -3000, 0, 3000, 100, std::bind(&mame_ui_manager::slider_crossoffset, this, std::ref(field), _1, _2));
			}
		}
	}
#endif

	std::vector<ui::menu_item> items;
	for (auto &slider : m_sliders)
	{
		ui::menu_item item(ui::menu_item_type::SLIDER, slider.get());
		item.set_text(slider->description);
		items.emplace_back(std::move(item));
	}

	return items;
}


//-------------------------------------------------
//  slider_volume - global volume slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_volume(std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		machine().sound().set_master_gain(newval == -48 ? 0 : osd::db_to_linear(newval));

	int curval = machine().sound().master_gain() == 0 ? -48 : osd::linear_to_db_int(machine().sound().master_gain());

	if (str)
	{
		if (curval == -48)
			*str = _("Mute");
		else
			*str = string_format(_(u8"%1$3d\u00a0dB"), curval);
	}
	return curval;
}


//-------------------------------------------------
//  slider_devvol - device volume
//  slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_devvol(device_sound_interface *snd, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		snd->set_user_output_gain(newval == -48 ? 0 : osd::db_to_linear(newval));

	int curval = snd->user_output_gain() == 0 ? -48 : osd::linear_to_db_int(snd->user_output_gain());

	if (str)
	{
		if (curval == -48)
			*str = _("Mute");
		else
			*str = string_format(_(u8"%1$3d\u00a0dB"), curval);
	}
	return curval;
}


//-------------------------------------------------
//  slider_devvol_chan - device channel volume
//  slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_devvol_chan(device_sound_interface *snd, int channel, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		snd->set_user_output_gain(channel, newval == -48 ? 0 : osd::db_to_linear(newval));

	int curval = snd->user_output_gain(channel) == 0 ? -48 : osd::linear_to_db_int(snd->user_output_gain(channel));

	if (str)
	{
		if (curval == -48)
			*str = _("Mute");
		else
			*str = string_format(_(u8"%1$3d\u00a0dB"), curval);
	}
	return curval;
}



//-------------------------------------------------
//  slider_adjuster - analog adjuster slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_adjuster(ioport_field &field, std::string *str, int32_t newval)
{
	ioport_field::user_settings settings;

	field.get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.value = newval;
		field.set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$3d%%"), settings.value);
	return settings.value;
}


//-------------------------------------------------
//  slider_overclock - CPU overclocker slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_overclock(device_t &device, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		device.set_clock_scale(double(newval) * 0.001);

	int32_t curval = floor(device.clock_scale() * 1000.0 + 0.5);
	if (str)
	{
		if (curval % 10)
			*str = string_format(_("%1$.1f%%"), float(curval) * 0.1f);
		else
			*str = string_format(_("%1$3d%%"), curval / 10);
	}

	return curval;
}


//-------------------------------------------------
//  slider_refresh - refresh rate slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_refresh(screen_device &screen, std::string *str, int32_t newval)
{
	double defrefresh = ATTOSECONDS_TO_HZ(screen.refresh_attoseconds());
	double refresh;

	if (newval != SLIDER_NOCHANGE)
	{
		int width = screen.width();
		int height = screen.height();
		const rectangle &visarea = screen.visible_area();
		screen.configure(width, height, visarea, HZ_TO_ATTOSECONDS(defrefresh + double(newval) * 0.001));
	}

	if (str)
		*str = string_format(_(u8"%1$.3f\u00a0Hz"), screen.frame_period().as_hz());
	refresh = screen.frame_period().as_hz();
	return floor((refresh - defrefresh) * 1000.0 + 0.5);
}


//-------------------------------------------------
//  slider_brightness - screen brightness slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_brightness(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_brightness = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_brightness);
	return floorf(settings.m_brightness * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_contrast - screen contrast slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_contrast(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_contrast = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_contrast);
	return floorf(settings.m_contrast * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_gamma - screen gamma slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_gamma(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_gamma = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_gamma);
	return floorf(settings.m_gamma * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_xscale - screen horizontal scale slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_xscale(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_xscale = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_xscale);
	return floorf(settings.m_xscale * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_yscale - screen vertical scale slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_yscale(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_yscale = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_yscale);
	return floorf(settings.m_yscale * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_xoffset - screen horizontal position
//  slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_xoffset(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_xoffset = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_xoffset);
	return floorf(settings.m_xoffset * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_yoffset - screen vertical position
//  slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_yoffset(screen_device &screen, std::string *str, int32_t newval)
{
	render_container::user_settings settings = screen.container().get_user_settings();
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_yoffset = float(newval) * 0.001f;
		screen.container().set_user_settings(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_yoffset);
	return floorf(settings.m_yoffset * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_overxscale - screen horizontal scale slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_overxscale(laserdisc_device &laserdisc, std::string *str, int32_t newval)
{
	laserdisc_overlay_config settings;

	laserdisc.get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overscalex = float(newval) * 0.001f;
		laserdisc.set_overlay_config(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_overscalex);
	return floorf(settings.m_overscalex * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_overyscale - screen vertical scale slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_overyscale(laserdisc_device &laserdisc, std::string *str, int32_t newval)
{
	laserdisc_overlay_config settings;

	laserdisc.get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overscaley = float(newval) * 0.001f;
		laserdisc.set_overlay_config(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_overscaley);
	return floorf(settings.m_overscaley * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_overxoffset - screen horizontal position
//  slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_overxoffset(laserdisc_device &laserdisc, std::string *str, int32_t newval)
{
	laserdisc_overlay_config settings;

	laserdisc.get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overposx = float(newval) * 0.001f;
		laserdisc.set_overlay_config(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_overposx);
	return floorf(settings.m_overposx * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_overyoffset - screen vertical position
//  slider callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_overyoffset(laserdisc_device &laserdisc, std::string *str, int32_t newval)
{
	laserdisc_overlay_config settings;

	laserdisc.get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overposy = float(newval) * 0.001f;
		laserdisc.set_overlay_config(settings);
	}
	if (str)
		*str = string_format(_("%1$.3f"), settings.m_overposy);
	return floorf(settings.m_overposy * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_flicker - vector flicker slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_flicker([[maybe_unused]] screen_device &screen, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_options::s_flicker = float(newval) * 0.001f;
	if (str)
		*str = string_format(_("%1$1.2f"), vector_options::s_flicker);
	return floorf(vector_options::s_flicker * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_beam_width_min - minimum vector beam width slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_beam_width_min([[maybe_unused]] screen_device &screen, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_options::s_beam_width_min = std::min(float(newval) * 0.01f, vector_options::s_beam_width_max);
	if (str != nullptr)
		*str = string_format(_("%1$1.2f"), vector_options::s_beam_width_min);
	return floorf(vector_options::s_beam_width_min * 100.0f + 0.5f);
}


//-------------------------------------------------
//  slider_beam_width_max - maximum vector beam width slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_beam_width_max([[maybe_unused]] screen_device &screen, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_options::s_beam_width_max = std::max(float(newval) * 0.01f, vector_options::s_beam_width_min);
	if (str != nullptr)
		*str = string_format(_("%1$1.2f"), vector_options::s_beam_width_max);
	return floorf(vector_options::s_beam_width_max * 100.0f + 0.5f);
}


//-------------------------------------------------
//  slider_beam_dot_size - beam dot size slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_beam_dot_size([[maybe_unused]] screen_device &screen, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_options::s_beam_dot_size = std::max(float(newval) * 0.01f, 0.1f);
	if (str != nullptr)
		*str = string_format(_("%1$1.2f"), vector_options::s_beam_dot_size);
	return floorf(vector_options::s_beam_dot_size * 100.0f + 0.5f);
}


//-------------------------------------------------
//  slider_beam_intensity_weight - vector beam intensity weight slider
//  callback
//-------------------------------------------------

int32_t mame_ui_manager::slider_beam_intensity_weight([[maybe_unused]] screen_device &screen, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_options::s_beam_intensity_weight = float(newval) * 0.001f;
	if (str != nullptr)
		*str = string_format(_("%1$1.2f"), vector_options::s_beam_intensity_weight);
	return floorf(vector_options::s_beam_intensity_weight * 1000.0f + 0.5f);
}


//-------------------------------------------------
//  slider_crossscale - crosshair scale slider
//  callback
//-------------------------------------------------

#ifdef MAME_DEBUG
int32_t mame_ui_manager::slider_crossscale(ioport_field &field, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		field.set_crosshair_scale(float(newval) * 0.001);
	if (str)
		*str = string_format((field.crosshair_axis() == CROSSHAIR_AXIS_X) ? _("Crosshair Scale X %1$1.3f") :  _("Crosshair Scale Y %1$1.3f"), float(newval) * 0.001f);
	return floorf(field.crosshair_scale() * 1000.0f + 0.5f);
}
#endif


//-------------------------------------------------
//  slider_crossoffset - crosshair scale slider
//  callback
//-------------------------------------------------

#ifdef MAME_DEBUG
int32_t mame_ui_manager::slider_crossoffset(ioport_field &field, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
		field.set_crosshair_offset(float(newval) * 0.001f);
	if (str)
		*str = string_format((field.crosshair_axis() == CROSSHAIR_AXIS_X) ? _("Crosshair Offset X %1$1.3f") :  _("Crosshair Offset Y %1$1.3f"), float(newval) * 0.001f);
	return field.crosshair_offset();
}
#endif


//-------------------------------------------------
//  create_layout
//-------------------------------------------------

ui::text_layout mame_ui_manager::create_layout(render_container &container, float width, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap)
{
	// determine scale factors
	float const yscale = get_line_height();
	float const xscale = yscale * machine().render().ui_aspect(&container);

	// create the layout
	return ui::text_layout(*get_font(), xscale, yscale, width, justify, wrap);
}


//-------------------------------------------------
//  draw_textured_box - add primitives to
//  draw an outlined box with the given
//  textured background and line color
//-------------------------------------------------

void mame_ui_manager::draw_textured_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t backcolor, rgb_t linecolor, render_texture *texture, uint32_t flags)
{
	container.add_quad(x0, y0, x1, y1, backcolor, texture, flags);
	container.add_line(x0, y0, x1, y0, UI_LINE_WIDTH, linecolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x1, y0, x1, y1, UI_LINE_WIDTH, linecolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x1, y1, x0, y1, UI_LINE_WIDTH, linecolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container.add_line(x0, y1, x0, y0, UI_LINE_WIDTH, linecolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

void mame_ui_manager::popup_time_string(int seconds, std::string message)
{
	// extract the text
	messagebox_poptext = message;

	// set a timer
	m_popup_text_end = osd_ticks() + osd_ticks_per_second() * seconds;
}


/***************************************************************************
    LOADING AND SAVING OPTIONS
***************************************************************************/

//-------------------------------------------------
//  load ui options
//-------------------------------------------------

void mame_ui_manager::load_ui_options()
{
	// parse the file
	// attempt to open the output file
	emu_file file(machine().options().ini_path(), OPEN_FLAG_READ);
	if (!file.open("ui.ini"))
	{
		try
		{
			options().parse_ini_file((util::core_file &)file, OPTION_PRIORITY_MAME_INI, OPTION_PRIORITY_MAME_INI < OPTION_PRIORITY_DRIVER_INI, true);
		}
		catch (options_exception &)
		{
			osd_printf_error("**Error loading ui.ini**\n");
		}
	}
}

//-------------------------------------------------
//  save ui options
//-------------------------------------------------

void mame_ui_manager::save_ui_options()
{
	// attempt to open the output file
	emu_file file(machine().options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (!file.open("ui.ini"))
	{
		// generate the updated INI
		file.puts(options().output_ini());
		file.close();
	}
	else
		machine().popmessage(_("**Error saving ui.ini**"));
}

//-------------------------------------------------
//  save main option
//-------------------------------------------------

void mame_ui_manager::save_main_option()
{
	// parse the file
	std::string error;
	emu_options options(emu_options::option_support::GENERAL_ONLY);

	// This way we make sure that all OSD parts are in
	osd_setup_osd_specific_emu_options(options);

	options.copy_from(machine().options());

	// attempt to open the main ini file
	{
		emu_file file(machine().options().ini_path(), OPEN_FLAG_READ);
		if (!file.open(std::string(emulator_info::get_configname()) + ".ini"))
		{
			try
			{
				options.parse_ini_file((util::core_file&)file, OPTION_PRIORITY_MAME_INI, OPTION_PRIORITY_MAME_INI < OPTION_PRIORITY_DRIVER_INI, true);
			}
			catch (options_error_exception &)
			{
				osd_printf_error("**Error loading %s.ini**\n", emulator_info::get_configname());
				return;
			}
			catch (options_exception &)
			{
				// ignore other exceptions related to options
			}
		}
	}

	for (const auto &f_entry : machine().options().entries())
	{
		const char *value = f_entry->value();
		if (value && options.exists(f_entry->name()) && strcmp(value, options.value(f_entry->name())))
		{
			options.set_value(f_entry->name(), *f_entry->value(), OPTION_PRIORITY_CMDLINE);
		}
	}

	// attempt to open the output file
	{
		emu_file file(machine().options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (!file.open(std::string(emulator_info::get_configname()) + ".ini"))
		{
			// generate the updated INI
			file.puts(options.output_ini());
			file.close();
		}
		else
		{
			machine().popmessage(_("**Error saving %s.ini**"), emulator_info::get_configname());
			return;
		}
	}
	popup_time(3, "%s", _("\n    Settings saved    \n\n"));
}

void mame_ui_manager::menu_reset()
{
	ui::menu::stack_reset(*this);
}


bool mame_ui_manager::set_ui_event_handler(std::function<bool ()> &&handler)
{
	// only allow takeover if there's nothing else happening
	if (ui_callback_type::GENERAL != m_handler_callback_type)
		return false;

	for (auto *target = machine().render().first_target(); target; target = target->next())
	{
		if (!target->hidden())
			target->forget_pointers();
	}

	set_handler(
			ui_callback_type::CUSTOM,
			handler_callback_func(
				[cb = std::move(handler)] (render_container &container) -> uint32_t
				{
					return !cb() ? HANDLER_CANCEL : 0;
				}));
	return true;
}


//-------------------------------------------------
//  get_general_input_setting - get the current
//  default setting for an input type (useful for
//  prompting the user)
//-------------------------------------------------

std::string mame_ui_manager::get_general_input_setting(ioport_type type, int player, input_seq_type seqtype)
{
	input_seq seq(machine().ioport().type_seq(type, player, seqtype));
	input_code codes[16]; // TODO: remove magic number
	unsigned len(0U);
	for (unsigned i = 0U; std::size(codes) > i; ++i)
	{
		if (input_seq::not_code == seq[i])
			++i;
		else
			codes[len++] = seq[i];
		if (input_seq::end_code == seq[i])
			break;
	}
	seq.reset();
	for (unsigned i = 0U; len > i; ++i)
		seq += codes[i];
	return machine().input().seq_name(seq);
}


void ui_colors::refresh(const ui_options &options)
{
	m_border_color = options.border_color();
	m_background_color = options.background_color();
	m_gfxviewer_bg_color = options.gfxviewer_bg_color();
	m_unavailable_color = options.unavailable_color();
	m_text_color = options.text_color();
	m_text_bg_color = options.text_bg_color();
	m_subitem_color = options.subitem_color();
	m_clone_color = options.clone_color();
	m_selected_color = options.selected_color();
	m_selected_bg_color = options.selected_bg_color();
	m_mouseover_color = options.mouseover_color();
	m_mouseover_bg_color = options.mouseover_bg_color();
	m_mousedown_color = options.mousedown_color();
	m_mousedown_bg_color = options.mousedown_bg_color();
	m_dipsw_color = options.dipsw_color();
	m_slider_color = options.slider_color();
}
