// license:BSD-3-Clause
// copyright-holders:Couriersud
/*********************************************************************

    debugint.c

    Internal debugger frontend using render interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "rendfont.h"
#include "uiinput.h"

#include "debug/debugvw.h"
#include "debug/dvdisasm.h"
#include "debug/dvmemory.h"
#include "debug/dvbpoints.h"
#include "debug/dvwpoints.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "config.h"
#include "debugger.h"
#include "modules/lib/osdobj_common.h"
#include "debug_module.h"
#include "modules/osdmodule.h"

class debug_internal : public osd_module, public debug_module
{
public:
	debug_internal()
	: osd_module(OSD_DEBUG_PROVIDER, "internal"), debug_module(),
		m_machine(nullptr)
	{
	}

	virtual ~debug_internal() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override;

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	running_machine* m_machine;
	const char*      font_name;
	float            font_size;
};



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define BORDER_YTHICKNESS 1
#define BORDER_XTHICKNESS 1
#define HSB_HEIGHT 20
#define VSB_WIDTH 20
#define TITLE_HEIGHT (debug_font_height + 3*BORDER_YTHICKNESS)

enum
{
	RECT_DVIEW,
	RECT_DVIEW_CLIENT,
	RECT_DVIEW_TITLE,
	RECT_DVIEW_HSB,
	RECT_DVIEW_VSB,
	RECT_DVIEW_SIZE
};

enum
{
	VIEW_STATE_BUTTON           = 0x01,
	VIEW_STATE_MOVING           = 0x02,
	VIEW_STATE_SIZING           = 0x04,
	VIEW_STATE_NEEDS_UPDATE     = 0x08,
	VIEW_STATE_FOLLOW_CPU       = 0x10,
	VIEW_STATE_VISIBLE          = 0x20
};

/***************************************************************************
    MACROS
***************************************************************************/

//#define NX(_dv, _x) ((float) (_x)/(float)(_dv)->bounds.width())
//#define NY(_dv, _y) ((float) (_y)/(float)(_dv)->bounds.height())
#define NX(_dv, _x) ((float) (_x)/(float) (dv)->rt_width)
#define NY(_dv, _y) ((float) (_y)/(float) (dv)->rt_height)


#define LIST_ADD_FRONT(_list, _elem, _type) \
	do { \
		(_elem)->next = _list; \
		_list = _elem; \
	} while (0)

#define LIST_GET_PREVIOUS(_list, _elem, _prev) \
	do { \
		_prev = nullptr; \
		if (_list != _elem) \
			for (_prev = _list; _prev != nullptr; _prev = _prev->next) \
				if ((_prev)->next == _elem) \
					break; \
	} while (0)

#define LIST_GET_LAST(_list, _last) \
	do { \
		for (_last = _list; _last != nullptr; _last = _last->next) \
			if ((_last)->next == nullptr) \
				break; \
	} while (0)

#define LIST_REMOVE(_list, _elem, _type) \
	do { \
		_type *_hlp; \
		LIST_GET_PREVIOUS(_list, _elem, _hlp); \
		if (_hlp != nullptr) \
			(_hlp)->next = (_elem)->next; \
		else \
			_list = (_elem)->next; \
	} while (0)

#define LIST_ADD_BACK(_list, _elem, _type) \
	do { \
		_type *_hlp; \
		LIST_GET_LAST(_list, _hlp); \
		if (_hlp != nullptr) \
			(_hlp)->next = _elem; \
		else \
			_list = _elem; \
	} while (0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct adjustment
{
	int     visible;
	int     lower;
	int     upper;
	int     value;
	int     step_increment;
	int     page_increment;
	int     page_size;
};

class DView;

class DView_edit
{
	DISABLE_COPYING(DView_edit);

public:
	DView_edit(DView* owner): active(0), container(nullptr), owner(owner) { }
	~DView_edit() { }
	int                 active;
	render_container *  container;
	std::string         str;
	DView*              owner;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void dview_update(debug_view &dw, void *osdprivate);
static int map_point(DView *dv, INT32 target_x, INT32 target_y, INT32 *mapped_x, INT32 *mapped_y);

class DView
{
	DISABLE_COPYING(DView);

public:
	DView(render_target *target, running_machine &machine, debug_view_type type, int flags)
		: next(nullptr),
			type(0),
			state(0),
			ofs_x(0),
			ofs_y(0),
			editor(this)
		{
		this->target = target;
		//dv->container = render_target_get_component_container(target, name, &pos);
		this->container = target->debug_alloc();
		this->view = machine.debug_view().alloc_view(type, dview_update, this);
		this->type = type;
		this->m_machine = &machine;
		this->state = flags | VIEW_STATE_NEEDS_UPDATE | VIEW_STATE_VISIBLE;
		// initial size
		this->bounds.set(0, 300, 0, 300);

		/* specials */
		switch (type)
		{
		case DVT_DISASSEMBLY:
			/* set up disasm view */
			downcast<debug_view_disasm *>(this->view)->set_expression("curpc");
			//debug_view_  property_UINT32(dv->view, DVP_DASM_TRACK_LIVE, 1);
			break;
		default:
			break;
		}
		}
	~DView()
	{
		//this->target->debug_free(*this->container);
		machine().debug_view().free_view(*this->view);
	}

	running_machine &machine() const { assert(m_machine != nullptr); return *m_machine; }

	DView *             next;

	int                 type;
	debug_view *        view;
	render_container *  container;
	render_target *     target;
	running_machine *   m_machine;
	int                 state;
	// drawing
	rectangle           bounds;
	int                 ofs_x;
	int                 ofs_y;
	std::string         title;
	int                 last_x;
	int                 last_y;
	// Scrollbars
	adjustment          hsb;
	adjustment          vsb;
	// render target tracking
	INT32               rt_width;
	INT32               rt_height;
	//optional
	DView_edit          editor;
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

static inline int dview_is_state(DView *dv, int state)
{
	return ((dv->state & state) ? TRUE : FALSE);
}

static inline int dview_is_state_all(DView *dv, int state)
{
	return ((dv->state & state) == state ? TRUE : FALSE);
}

static inline void dview_set_state(DView *dv, int state, int onoff)
{
	if (onoff)
		dv->state |= state;
	else
		dv->state &= ~state;
}

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static render_font *    debug_font = nullptr;
static int              debug_font_width;
static int              debug_font_height;
static float            debug_font_aspect;
static DView *          list = nullptr;
static DView *          focus_view;

static ui_menu *        menu;
static DView_edit *     cur_editor;
static int              win_count;
static void*            menu_sel;  // selected item in the menu

static void set_focus_view(DView *dv)
{
	if (focus_view != nullptr)
		dview_set_state(focus_view, VIEW_STATE_NEEDS_UPDATE, TRUE);

	if (dv != nullptr)
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);

	if (focus_view != dv)
	{
		focus_view = dv;
		LIST_REMOVE(list, dv, DView);
		LIST_ADD_FRONT(list, dv, DView);
		dv->target->debug_append(*dv->container);
	}
}

static DView *dview_alloc(render_target *target, running_machine &machine, debug_view_type type, int flags)
{
	DView *dv;

	dv = global_alloc(DView(target, machine, type, flags));

	/* add to list */

	LIST_ADD_BACK(list, dv, DView);

	return dv;
}

static void dview_free(DView *dv)
{
	dv->container->empty();
	LIST_REMOVE(list, dv, DView);
	global_free(dv);
}

static void dview_get_rect(DView *dv, int type, rectangle &rect)
{
	rect = dv->bounds;
	switch (type)
	{
	case RECT_DVIEW:
		break;
	case RECT_DVIEW_CLIENT:
		rect.min_x += BORDER_XTHICKNESS;
		rect.max_x -= (BORDER_XTHICKNESS + dv->vsb.visible * VSB_WIDTH);
		rect.min_y += 2 * BORDER_YTHICKNESS + TITLE_HEIGHT;
		rect.max_y -= (BORDER_YTHICKNESS + dv->hsb.visible * HSB_HEIGHT);
		break;
	case RECT_DVIEW_HSB:
		rect.min_x += 0;
		rect.max_x -= /* dv->vsb.visible * */ VSB_WIDTH;
		rect.min_y = dv->bounds.max_y - HSB_HEIGHT;
		rect.max_y -= 0;
		break;
	case RECT_DVIEW_VSB:
		rect.min_x = dv->bounds.max_x - VSB_WIDTH;
		rect.max_x -= 0;
		rect.min_y += TITLE_HEIGHT;
		rect.max_y -= /* dv->hsb.visible * */ HSB_HEIGHT;
		break;
	case RECT_DVIEW_SIZE:
		rect.min_x = dv->bounds.max_x - VSB_WIDTH;
		rect.max_x -= 0;
		rect.min_y = dv->bounds.max_y - HSB_HEIGHT;
		rect.max_y -= 0;
		break;
	case RECT_DVIEW_TITLE:
		rect.min_x += 0;
		rect.max_x -= 0;
		rect.min_y += 0;
		rect.max_y = rect.min_y + TITLE_HEIGHT - 1;
		break;
	default:
		assert_always(FALSE, "unknown rectangle type");
	}
}


static void dview_clear(DView *dv)
{
	dv->container->empty();
}

static void dview_draw_outlined_box(DView *dv, int rtype, int x, int y, int w, int h, rgb_t bg)
{
	rectangle r;

	dview_get_rect(dv, rtype, r);
	dv->container->manager().machine().ui().draw_outlined_box(dv->container, NX(dv, x + r.min_x), NY(dv, y + r.min_y),
			NX(dv, x + r.min_x + w), NY(dv, y + r.min_y + h), bg);
}

static void dview_draw_box(DView *dv, int rtype, int x, int y, int w, int h, rgb_t col)
{
	rectangle r;

	dview_get_rect(dv, rtype, r);
	dv->container->add_rect(NX(dv, x + r.min_x), NY(dv, y + r.min_y),
			NX(dv, x + r.min_x + w), NY(dv, y + r.min_y + h), col,
			PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

static void dview_draw_line(DView *dv, int rtype, int x1, int y1, int x2, int y2, rgb_t col)
{
	rectangle r;

	dview_get_rect(dv, rtype, r);
	dv->container->add_line(NX(dv, x1 + r.min_x), NY(dv, y1 + r.min_y),
			NX(dv, x2 + r.min_x), NY(dv, y2 + r.min_y), UI_LINE_WIDTH, col,
			PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

static void dview_draw_char(DView *dv, int rtype, int x, int y, int h, rgb_t col, UINT16 ch)
{
	rectangle r;

	dview_get_rect(dv, rtype, r);
	dv->container->add_char(
			NX(dv, x + r.min_x),
			NY(dv, y + r.min_y),
			NY(dv, h),
			debug_font_aspect,
			//(float) dv->bounds.height() / (float) dv->bounds->width(), //render_get_ui_aspect(),
			col,
			*debug_font,
			ch);
}

static int dview_xy_in_rect(DView *dv, int type, int x, int y)
{
	rectangle r;

	dview_get_rect(dv, type, r);
	if (r.contains(x, y))
		return TRUE;
	return FALSE;
}

static void dview_draw_hsb(DView *dv)
{
	int vt;
	int ts;
	//int sz = SLIDER_SIZE;
	int sz;
	rectangle r;
	adjustment *sb = &dv->hsb;

	dview_get_rect(dv, RECT_DVIEW_HSB, r);

	dview_draw_box(dv, RECT_DVIEW_HSB, 0, 0, r.width(), r.height(), rgb_t(0xff, 0x60, 0x60, 0x60));
	dview_draw_outlined_box(dv, RECT_DVIEW_HSB, 0, 0, VSB_WIDTH,HSB_HEIGHT, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
	dview_draw_outlined_box(dv, RECT_DVIEW_HSB, r.width() - VSB_WIDTH, 0, VSB_WIDTH, HSB_HEIGHT, rgb_t(0xff, 0xc0, 0xc0, 0xc0));

	// draw arrows
	dview_draw_line(dv, RECT_DVIEW_HSB, (VSB_WIDTH/3)*2, HSB_HEIGHT/4, VSB_WIDTH/3, HSB_HEIGHT/2, rgb_t(0xff, 0x00, 0x00, 0x00));
	dview_draw_line(dv, RECT_DVIEW_HSB, VSB_WIDTH/3, HSB_HEIGHT/2, (VSB_WIDTH/3)*2, (HSB_HEIGHT/4)*3, rgb_t(0xff, 0x00, 0x00, 0x00));
	dview_draw_line(dv, RECT_DVIEW_HSB, r.width() - (VSB_WIDTH/3)*2, HSB_HEIGHT/4, r.width() - VSB_WIDTH/3, HSB_HEIGHT/2, rgb_t(0xff, 0x00, 0x00, 0x00));
	dview_draw_line(dv, RECT_DVIEW_HSB, r.width() - VSB_WIDTH/3, HSB_HEIGHT/2, r.width() - (VSB_WIDTH/3)*2, (HSB_HEIGHT/4)*3, rgb_t(0xff, 0x00, 0x00, 0x00));

	ts = (r.width()) - 2 * VSB_WIDTH;

	sz = (ts * (sb->page_size)) / (sb->upper - sb->lower);
	ts = ts - sz;

	vt = (ts * (sb->value - sb->lower)) / (sb->upper - sb->lower - sb->page_size) + sz / 2 + r.min_x + VSB_WIDTH;

	dview_draw_outlined_box(dv, RECT_DVIEW_HSB, vt - sz / 2, 0, sz, HSB_HEIGHT, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
}

static void dview_draw_vsb(DView *dv)
{
	INT64 vt;
	INT64 ts;
	//int sz = SLIDER_SIZE;
	INT64 sz;
	rectangle r;
	adjustment *sb = &dv->vsb;

	dview_get_rect(dv, RECT_DVIEW_VSB, r);

	dview_draw_box(dv, RECT_DVIEW_VSB, 0, 0, r.width(), r.height(), rgb_t(0xff, 0x60, 0x60, 0x60));
	dview_draw_outlined_box(dv, RECT_DVIEW_VSB, 0, r.height() - HSB_HEIGHT, VSB_WIDTH, HSB_HEIGHT, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
	dview_draw_outlined_box(dv, RECT_DVIEW_VSB, 0, 0,                       VSB_WIDTH, HSB_HEIGHT, rgb_t(0xff, 0xc0, 0xc0, 0xc0));

	// draw arrows
	dview_draw_line(dv, RECT_DVIEW_VSB, VSB_WIDTH/4, (HSB_HEIGHT/3)*2, VSB_WIDTH/2, HSB_HEIGHT/3, rgb_t(0xff, 0x00, 0x00, 0x00));
	dview_draw_line(dv, RECT_DVIEW_VSB, VSB_WIDTH/2, HSB_HEIGHT/3, (VSB_WIDTH/4)*3, (HSB_HEIGHT/3)*2, rgb_t(0xff, 0x00, 0x00, 0x00));
	dview_draw_line(dv, RECT_DVIEW_VSB, VSB_WIDTH/4, r.height() - (HSB_HEIGHT/3)*2, VSB_WIDTH/2, r.height() - HSB_HEIGHT/3, rgb_t(0xff, 0x00, 0x00, 0x00));
	dview_draw_line(dv, RECT_DVIEW_VSB, VSB_WIDTH/2, r.height() - HSB_HEIGHT/3, (VSB_WIDTH/4)*3, r.height() - (HSB_HEIGHT/3)*2, rgb_t(0xff, 0x00, 0x00, 0x00));

	ts = r.height() - 2 * HSB_HEIGHT;

	sz = (ts * (sb->page_size)) / (sb->upper - sb->lower);
	ts = ts - sz;

	vt = (ts * (sb->value - sb->lower)) / (sb->upper - sb->lower - sb->page_size) + sz / 2 + HSB_HEIGHT;

	dview_draw_outlined_box(dv, RECT_DVIEW_VSB, 0, vt - sz / 2, VSB_WIDTH, sz, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
}

static void dview_draw_size(DView *dv)
{
	rectangle r;

	dview_get_rect(dv, RECT_DVIEW_SIZE, r);

	dview_draw_outlined_box(dv, RECT_DVIEW_SIZE, 0, 0,
			r.width(),r.height(), rgb_t(0xff, 0x80, 0x80, 0x80));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 11, r.height()-1, r.width()-1, 11, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 12, r.height()-1, r.width()-1, 12, rgb_t(0xff, 0x60, 0x60, 0x60));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 13, r.height()-1, r.width()-1, 13, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 14, r.height()-1, r.width()-1, 14, rgb_t(0xff, 0x60, 0x60, 0x60));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 15, r.height()-1, r.width()-1, 15, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 16, r.height()-1, r.width()-1, 16, rgb_t(0xff, 0x60, 0x60, 0x60));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 17, r.height()-1, r.width()-1, 17, rgb_t(0xff, 0xc0, 0xc0, 0xc0));
	dview_draw_line(dv, RECT_DVIEW_SIZE, 18, r.height()-1, r.width()-1, 18, rgb_t(0xff, 0x60, 0x60, 0x60));
}

static void dview_set_title(DView *dv, std::string title)
{
	if (dv->title.compare(title) != 0)
	{
		dv->title = title;
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
}

static void dview_draw_title(DView *dv)
{
	int i;
	rgb_t col = rgb_t(0xff,0x00,0x00,0xff);
	rectangle r;
	int str_x = 2;

	dview_get_rect(dv, RECT_DVIEW_TITLE, r);

	if (dv == focus_view)
		col = rgb_t(0xff,0x00,0x7f,0x00);

	dview_draw_outlined_box(dv, RECT_DVIEW_TITLE, 0, 0, dv->bounds.width(), TITLE_HEIGHT, col);

	if (dv->title.empty())
		return;

	for (i = 0; i<strlen(dv->title.c_str()); i++)
	{
		if(str_x < r.width() - debug_font_width)
		{
			dview_draw_char(dv, RECT_DVIEW_TITLE, str_x,
				BORDER_YTHICKNESS, debug_font_height, //r.max_y - 2 * BORDER_YTHICKNESS,
				rgb_t(0xff,0xff,0xff,0xff), (UINT16) dv->title[i] );
		}
		str_x += debug_font->char_width(debug_font_height, debug_font_aspect,(UINT16) dv->title[i]) + 2*BORDER_XTHICKNESS;
	}
}

static int dview_on_mouse(DView *dv, int mx, int my, bool button)
{
	int clicked = (button && !dview_is_state(dv, VIEW_STATE_BUTTON));
	int handled = TRUE;
	int x,y;

	if (button && dview_is_state_all(dv, VIEW_STATE_BUTTON | VIEW_STATE_MOVING))
	{
		int dx = mx - dv->last_x;
		int dy = my - dv->last_y;

		dv->ofs_x += dx;
		dv->ofs_y += dy;
		dv->last_x = mx;
		dv->last_y = my;
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		return TRUE;
	}
	else if (button && dview_is_state_all(dv, VIEW_STATE_BUTTON | VIEW_STATE_SIZING))
	{
		int dx = mx - dv->last_x;
		int dy = my - dv->last_y;

		dv->bounds.max_x += dx;
		dv->bounds.max_y += dy;
		dv->last_x = mx;
		dv->last_y = my;
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		return TRUE;
	}
	else
		dview_set_state(dv, VIEW_STATE_MOVING | VIEW_STATE_SIZING, FALSE);

	if (!map_point(dv, mx, my, &x, &y))
		return FALSE;

	if (dview_xy_in_rect(dv, RECT_DVIEW_TITLE, x, y))
	{
		/* on title, do nothing */
		if (clicked) {
			dv->last_x = mx;
			dv->last_y = my;
			set_focus_view(dv);
			dview_set_state(dv, VIEW_STATE_MOVING, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_HSB, x, y))
	{
		/* on horizontal scrollbar */
		debug_view_xy pos;
		adjustment *sb = &dv->hsb;

		if (clicked)
		{
			rectangle r;
			int xt;

			dview_get_rect(dv, RECT_DVIEW_HSB, r);
			x -= r.min_x;

			xt = (x - VSB_WIDTH) * (sb->upper - sb->lower) / (r.width() - 2 * dv->vsb.visible * VSB_WIDTH) + sb->lower;
			if (x < VSB_WIDTH)
				sb->value -= sb->step_increment;
			else if (x > r.width() - VSB_WIDTH)
				sb->value += sb->step_increment;
			else if (xt < sb->value)
				sb->value -= sb->page_increment;
			else if (xt > sb->value)
				sb->value += sb->page_increment;

			if (sb->value < sb->lower)
				sb->value = sb->lower;
			if (sb->value > sb->upper)
				sb->value = sb->upper;
		}

		pos = dv->view->visible_position();

		if (sb->value != pos.x)
		{
			pos.x = sb->value;
			dv->view->set_visible_position(pos);
			dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_VSB, x, y) )
	{
		/* on vertical scrollbar */
		debug_view_xy pos;
		adjustment *sb = &dv->vsb;

		if (clicked)
		{
			rectangle r;
			int yt;

			dview_get_rect(dv, RECT_DVIEW_VSB, r);
			y -= r.min_y;
			yt = (y - HSB_HEIGHT) * (sb->upper - sb->lower) / (r.height() - 2 * HSB_HEIGHT) + sb->lower;

			if (y < HSB_HEIGHT)
				sb->value -= sb->step_increment;
			else if (y > r.height() - HSB_HEIGHT)
				sb->value += sb->step_increment;
			else if (yt < sb->value)
				sb->value -= sb->page_increment;
			else if (yt > sb->value)
				sb->value += sb->page_increment;

			if (sb->value < sb->lower)
				sb->value = sb->lower;
			if (sb->value > sb->upper)
				sb->value = sb->upper;
		}

		pos = dv->view->visible_position();

		if (sb->value != pos.y)
		{
			pos.y = sb->value;
			dv->view->set_visible_position(pos);
			dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_SIZE, x, y))
	{
		/* on sizing area */
		if (clicked)
		{
			dv->last_x = mx;
			dv->last_y = my;
			set_focus_view(dv);
			dview_set_state(dv, VIEW_STATE_SIZING, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_CLIENT, x, y))
	{
		y -= TITLE_HEIGHT;
		if (dv->view->cursor_supported() && clicked && y >= 0)
		{
			debug_view_xy topleft = dv->view->visible_position();
			debug_view_xy newpos;
			newpos.x = topleft.x + x / debug_font_width;
			newpos.y = topleft.y + y / debug_font_height;
			dv->view->set_cursor_position(newpos);
			dv->view->set_cursor_visible(true);
		}
		if (clicked)
			set_focus_view(dv);
	}
	else
	{
		handled = FALSE;
	}
	dview_set_state(dv, VIEW_STATE_BUTTON, button);
	return handled;

}

static inline void map_attr_to_fg_bg(unsigned char attr, rgb_t *fg, rgb_t *bg)
{
	*bg = rgb_t(0xff,0xff,0xff,0xff);
	*fg = rgb_t(0xff,0x00,0x00,0x00);

	if(attr & DCA_ANCILLARY)
		*bg = rgb_t(0xff,0xe0,0xe0,0xe0);
	if(attr & DCA_SELECTED) {
		*bg = rgb_t(0xff,0xff,0x80,0x80);
	}
	if(attr & DCA_CURRENT) {
		*bg = rgb_t(0xff,0xff,0xff,0x00);
	}
	if(attr & DCA_CHANGED) {
		*fg = rgb_t(0xff,0xff,0x00,0x00);
	}
	if(attr & DCA_INVALID) {
		*fg = rgb_t(0xff,0x00,0x00,0xff);
	}
	if(attr & DCA_DISABLED) {
		*fg = rgb_t(fg->a(), (fg->r() + bg->r()) >> 1, (fg->g() + bg->g()) >> 1, (fg->b() + bg->b()) >> 1);
	}
	if(attr & DCA_COMMENT) {
		*fg = rgb_t(0xff,0x00,0x80,0x00);
	}
}

static void dview_draw(DView *dv)
{
	const debug_view_char *viewdata;
	debug_view_xy vsize;
	UINT32 i, j, xx, yy;
	rgb_t bg_base, bg, fg;
	rectangle r;

	vsize = dv->view->visible_size();

	bg_base = rgb_t(0xff,0xff,0xff,0xff);

	/* always start clean */
	dview_clear(dv);

	dview_draw_title(dv);

	dview_get_rect(dv, RECT_DVIEW_CLIENT, r);

	dview_draw_outlined_box(dv, RECT_DVIEW_CLIENT, 0, 0,
			r.width() /*- (dv->vs ? VSB_WIDTH : 0)*/,
			r.height() /*- (dv->hsb.visible ? HSB_HEIGHT : 0)*/, bg_base);

	/* draw background and text */
	viewdata = dv->view->viewdata();
	yy = BORDER_YTHICKNESS;
	for(j=0; j<vsize.y; j++)
	{
		xx = BORDER_XTHICKNESS;
		for(i=0; i<vsize.x; i++)
		{
			UINT16 s = ' ';
			unsigned char v = viewdata->byte;

			if(v < 128) {
				s = v;
			} else {
				s = 0xc0 | (v>>6);
				s |= (0x80 | (v & 0x3f));
			}
			map_attr_to_fg_bg(viewdata->attrib, &fg, &bg);
			if (bg != bg_base)
				dview_draw_box(dv, RECT_DVIEW_CLIENT, xx, yy,
						debug_font_width, debug_font_height, bg);
			if (v != ' ')
				dview_draw_char(dv, RECT_DVIEW_CLIENT, xx, yy, debug_font_height, fg, s);
			xx += debug_font_width;
			viewdata++;
		}
		yy += debug_font_height;
	}

	if(dv->hsb.visible)
		dview_draw_hsb(dv);
	if(dv->vsb.visible)
		dview_draw_vsb(dv);
	dview_draw_size(dv);
}


static void dview_size_allocate(DView *dv)
{
	debug_view_xy size, pos, col, vsize;
	render_container::user_settings rcus;
	rectangle r;
	std::string title;

	dv->container->get_user_settings(rcus);
	rcus.m_xoffset = (float) dv->ofs_x / (float) dv->rt_width;
	rcus.m_yoffset = (float) dv->ofs_y / (float) dv->rt_height;
	rcus.m_xscale = 1.0; //(float) dv->bounds.width() / (float) dv->rt_width;
	rcus.m_yscale = 1.0; //(float) dv->bounds.height() / (float) dv->rt_height;
	dv->container->set_user_settings(rcus);
	//printf("%d %d %d %d\n", wpos.min_x, wpos.max_x, wpos.min_y, wpos.max_y);

	pos = dv->view->visible_position();
	size = dv->view->total_size();

	dv->hsb.visible = 0;
	dv->vsb.visible = 0;
	dview_get_rect(dv, RECT_DVIEW_CLIENT, r);

	dv->hsb.visible = (size.x * debug_font_width > r.width() ? 1 : 0);
	dv->vsb.visible = ((INT64)size.y * (INT64)debug_font_height > r.height() ? 1 : 0);
	dview_get_rect(dv, RECT_DVIEW_CLIENT, r);

//  dv->hsb.visible = (size.x * debug_font_width > r.width() ? 1 : 0);
//  dv->vsb.visible = (size.y * debug_font_height > r.height() ? 1 : 0);
//  dview_get_rect(dv, RECT_DVIEW_CLIENT, r);

	col.y = (r.height() - 2 * BORDER_YTHICKNESS /*+ debug_font_height  - 1*/) / debug_font_height;
	col.x = (r.width() - 2 * BORDER_XTHICKNESS /*+ debug_font_width - 1*/) / debug_font_width;

	vsize.y = size.y - pos.y;
	vsize.x = size.x - pos.x;
	if(vsize.y > col.y)
		vsize.y = col.y;
	else if(vsize.y < col.y) {
		pos.y = size.y-col.y;
		if(pos.y < 0)
			pos.y = 0;
		vsize.y = size.y-pos.y;
	}
	if(vsize.x > col.x)
		vsize.x = col.x;
	else if(vsize.x < col.x) {
		pos.x = size.x-col.x;
		if(pos.x < 0)
			pos.x = 0;
		vsize.x = size.x-pos.x;
	}

	dv->view->set_visible_position(pos);
	dv->view->set_visible_size(vsize);

	if(dv->hsb.visible) {
		int span = (r.width() - 2 * BORDER_XTHICKNESS) / debug_font_width;

		if(pos.x + span > size.x)
			pos.x = size.x - span;
		if(pos.x < 0)
			pos.x = 0;
		dv->hsb.lower = 0;
		dv->hsb.upper = size.x;
		dv->hsb.value = pos.x;
		dv->hsb.step_increment = 1;
		dv->hsb.page_increment = span;
		dv->hsb.page_size = span;

		dv->view->set_visible_position(pos);
	}

	if(dv->vsb.visible) {
		int span = (r.height() - 2 * BORDER_YTHICKNESS) / debug_font_height;

		if(pos.y + span > size.y)
			pos.y = size.y - span;
		if(pos.y < 0)
			pos.y = 0;
		dv->vsb.lower = 0;
		dv->vsb.upper = size.y;
		dv->vsb.value = pos.y;
		dv->vsb.step_increment = 1;
		dv->vsb.page_increment = span;
		dv->vsb.page_size = span;

		dv->view->set_visible_position(pos);
	}
}

static void dview_update(debug_view &dw, void *osdprivate)
{
	DView *dv = (DView *) osdprivate;

	dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);

#if 0
	debug_view_xy size = dw.total_size();

	if((dv->tr != size.y) || (dv->tc != size.x))
		gtk_widget_queue_resize(GTK_WIDGET(dv));
	else
		gtk_widget_queue_draw(GTK_WIDGET(dv));
#endif
}

void debug_internal::exit()
{
	for (DView *ndv = list; ndv != nullptr; )
	{
		DView *temp = ndv;
		ndv = ndv->next;
		dview_free(temp);
	}
	if (debug_font != nullptr)
	{
		m_machine->render().font_free(debug_font);
		debug_font = nullptr;
	}
	if (menu)
		global_free(menu);
}

void debug_internal::init_debugger(running_machine &machine)
{
	unicode_char ch;
	int chw;

	m_machine = &machine;
	font_name = (downcast<osd_options &>(m_machine->options()).debugger_font());
	font_size = (downcast<osd_options &>(m_machine->options()).debugger_font_size());

	if(!strcmp(font_name, OSDOPTVAL_AUTO))
		debug_font = m_machine->render().font_alloc("Courier New");
	else
		debug_font = m_machine->render().font_alloc(font_name);

	debug_font_width = 0;
	if(font_size < 8)
		debug_font_height = 16;  // default
	else
		debug_font_height = font_size;
	menu = nullptr;
	cur_editor = nullptr;
	list = nullptr;
	focus_view = nullptr;

	debug_font_aspect = m_machine->render().ui_aspect();

	for (ch=0;ch<=127;ch++)
	{
		chw = debug_font->char_width(debug_font_height, debug_font_aspect, ch);
		if (chw>debug_font_width)
			debug_font_width = chw;
	}
	debug_font_width += 2*BORDER_XTHICKNESS;
}

#if 0
static void set_view_by_name(render_target *target, const char *name)
{
	int i = 0;
	const char *s;

	for (i = 0; ; i++ )
	{
		s = target->view_name(i);
		if (s == nullptr)
			return;
		//printf("%d %s\n", i, s);
		if (strcmp(name, s) == 0)
		{
			target->set_view(i);
			//printf("%d\n", target->view() );
			return;
		}
	}
}
#endif

/*-------------------------------------------------
    Menu Callbacks
  -------------------------------------------------*/

static void process_string(DView *dv, const char *str)
{
	switch (dv->type)
	{
	case DVT_DISASSEMBLY:
		downcast<debug_view_disasm *>(dv->view)->set_expression(str);
		break;
	case DVT_CONSOLE:
		if(!dv->editor.str[(int)0])
			debug_cpu_get_visible_cpu(dv->machine())->debug()->single_step();
		else
			debug_console_execute_command(dv->machine(), str, 1);
		break;
	case DVT_MEMORY:
		downcast<debug_view_memory *>(dv->view)->set_expression(str);
		break;
	}
}

static void debug_hide_all()
{
	for (DView *dv = list; dv != nullptr; dv = dv->next)
	{
		dv->container->empty();
		dview_set_state(dv,VIEW_STATE_VISIBLE,false);
	}
}

static void debug_show_all()
{
	for (DView *dv = list; dv != nullptr; dv = dv->next)
		dview_set_state(dv,VIEW_STATE_VISIBLE,true);
}

static void on_memory_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;
	const debug_view_source *source;

	target = &dv->machine().render().ui_target();

	ndv = dview_alloc(target, dv->machine(), DVT_MEMORY, 0);
	ndv->editor.active = TRUE;
	ndv->editor.container = &dv->machine().render().ui_container();
	source = ndv->view->source();
	dview_set_title(ndv, source->name());
	ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
	ndv->bounds.setx(0,500);
	win_count++;
	dview_set_state(ndv,VIEW_STATE_VISIBLE,true);
	set_focus_view(ndv);
}

static void on_disassembly_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;
	const debug_view_source *source;

	target = &dv->machine().render().ui_target();

	ndv = dview_alloc(target, dv->machine(), DVT_DISASSEMBLY, 0);
	ndv->editor.active = TRUE;
	ndv->editor.container = &dv->machine().render().ui_container();
	source = ndv->view->source();
	dview_set_title(ndv, source->name());
	ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
	win_count++;
	dview_set_state(ndv,VIEW_STATE_VISIBLE,true);
	set_focus_view(ndv);
}

static void on_disasm_cpu_activate(DView *dv, const ui_menu_event *event)
{
	const debug_view_source *current = dv->view->source();

	if (event->iptkey == IPT_UI_RIGHT)
	{
		current = current->next();
		if (current == nullptr)
			current = dv->view->first_source();
		dv->view->set_source(*current);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		dview_set_title(dv, current->name());
	}
}

static void on_log_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;

	target = &dv->machine().render().ui_target();
	ndv = dview_alloc(target, dv->machine(), DVT_LOG, 0);
	dview_set_title(ndv, "Log");
	ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
	ndv->bounds.setx(0,600);
	win_count++;
	dview_set_state(ndv,VIEW_STATE_VISIBLE,true);
	set_focus_view(ndv);
}

static void on_bp_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;

	target = &dv->machine().render().ui_target();
	ndv = dview_alloc(target, dv->machine(), DVT_BREAK_POINTS, 0);
	dview_set_title(ndv, "Breakpoints");
	ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
	ndv->bounds.setx(0,600);
	win_count++;
	dview_set_state(ndv,VIEW_STATE_VISIBLE,true);
	set_focus_view(ndv);
}

static void on_wp_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;

	target = &dv->machine().render().ui_target();
	ndv = dview_alloc(target, dv->machine(), DVT_WATCH_POINTS, 0);
	dview_set_title(ndv, "Watchpoints");
	ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
	ndv->bounds.setx(0,600);
	win_count++;
	dview_set_state(ndv,VIEW_STATE_VISIBLE,true);
	set_focus_view(ndv);
}

static void on_close_activate(DView *dv, const ui_menu_event *event)
{
	if (focus_view == dv)
		set_focus_view(dv->next);
	win_count--;
	dview_free(dv);
}

static void on_run_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->go();
}

static void on_run_h_activate(DView *dv, const ui_menu_event *event)
{
	debug_hide_all();
	debug_cpu_get_visible_cpu(dv->machine())->debug()->go();
}

static void on_run_cpu_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->go_next_device();
}

static void on_run_irq_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->go_interrupt();
}

static void on_run_vbl_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->go_vblank();
}

static void on_step_into_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->single_step();
}

static void on_step_over_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->single_step_over();
}

#ifdef UNUSED_CODE
static void on_step_out_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine())->debug()->single_step_out();
}
#endif

static void on_hard_reset_activate(DView *dv, const ui_menu_event *event)
{
	dv->machine().schedule_hard_reset();
}

static void on_soft_reset_activate(DView *dv, const ui_menu_event *event)
{
	dv->machine().schedule_soft_reset();
	debug_cpu_get_visible_cpu(dv->machine())->debug()->go();
}

static void on_exit_activate(DView *dv, const ui_menu_event *event)
{
	dv->machine().schedule_exit();
}

static void on_view_opcodes_activate(DView *dv, const ui_menu_event *event)
{
	debug_view_disasm *dasmview = downcast<debug_view_disasm *>(focus_view->view);
	disasm_right_column rc = dasmview->right_column();
	disasm_right_column new_rc = DASM_RIGHTCOL_NONE;

	if (event->iptkey == IPT_UI_RIGHT)
	{
		switch (rc)
		{
		case DASM_RIGHTCOL_RAW:         new_rc = DASM_RIGHTCOL_ENCRYPTED; break;
		case DASM_RIGHTCOL_ENCRYPTED:   new_rc = DASM_RIGHTCOL_COMMENTS; break;
		case DASM_RIGHTCOL_COMMENTS:    new_rc = DASM_RIGHTCOL_RAW; break;
		default:                        break;
		}
		dasmview->set_right_column(new_rc);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
}

static void on_run_to_cursor_activate(DView *dv, const ui_menu_event *event)
{
	char command[64];

	if (dv->view->cursor_visible() && debug_cpu_get_visible_cpu(dv->machine()) == dv->view->source()->device())
	{
		offs_t address = downcast<debug_view_disasm *>(dv->view)->selected_address();
		sprintf(command, "go %X", address);
		debug_console_execute_command(dv->machine(), command, 1);
	}
}

static void on_memory_address_type(DView *dv, const ui_menu_event *event)
{
	debug_view_memory *memview = downcast<debug_view_memory *>(focus_view->view);
	bool phys = memview->physical();

	if (event->iptkey == IPT_UI_RIGHT)
	{
		memview->set_physical(!phys);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
}

static void on_memory_data_format(DView *dv, const ui_menu_event *event)
{
	debug_view_memory *memview = downcast<debug_view_memory *>(focus_view->view);
	int format = memview->get_data_format();
	int idx = 0;
	int order[7] = { 1, 2, 4, 8, 9, 10, 11 };

	for(int x=0; x<7; x++)
	{
		if(order[x] == format)
			idx = x;
	}

	if (event->iptkey == IPT_UI_RIGHT)
	{
		idx++;
		if(idx >= 7)
			idx = 0;
		memview->set_data_format(order[idx]);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
	if (event->iptkey == IPT_UI_LEFT)
	{
		idx--;
		if(idx < 0)
			idx = 6;
		memview->set_data_format(order[idx]);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
}

static void on_memory_region(DView *dv, const ui_menu_event *event)
{
	debug_view_memory *memview = downcast<debug_view_memory *>(focus_view->view);
	const debug_view_source *source = memview->source();

	if (event->iptkey == IPT_UI_LEFT)
	{
		int idx = memview->source_list().indexof(*source);
		if(idx > 0)
			memview->set_source(*memview->source_list().find(idx-1));
		else
			memview->set_source(*memview->source_list().find(memview->source_list().count()-1));
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		dview_set_title(dv, memview->source()->name());
	}
	if (event->iptkey == IPT_UI_RIGHT)
	{
		if(source->next() != nullptr)
			memview->set_source(*source->next());
		else
			memview->set_source(*memview->first_source());
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		dview_set_title(dv, memview->source()->name());
	}
}

/*-------------------------------------------------
    editor
  -------------------------------------------------*/

static void render_editor(DView_edit *editor)
{
	DView* dv = editor->owner;
	rectangle r;
	const char* str = editor->str.c_str();
	int str_x = 2 * BORDER_XTHICKNESS;
	int start;
//  int editor_width;

	dview_get_rect(dv,RECT_DVIEW_HSB,r);

//  editor_width = debug_font->string_width(debug_font_height, debug_font_aspect, editor->str.c_str());
	// figure out which character to start drawing, so that you can always see the end of the string you're typing
	start = strlen(str) - (r.width() / (debug_font_width + 2*BORDER_XTHICKNESS));
	if(start < 0)
		start = 0;

	dview_draw_box(dv,RECT_DVIEW_HSB,0,0,r.width(),r.height(),rgb_t(0xff,0xff,0xff,0xff));
	dview_draw_line(dv,RECT_DVIEW_HSB,0,0,r.width(),0,rgb_t(0xff,0xc0,0xc0,0xc0));
	dview_draw_line(dv,RECT_DVIEW_HSB,r.width(),0,r.width(),r.height(),rgb_t(0xff,0x60,0x60,0x60));
	dview_draw_line(dv,RECT_DVIEW_HSB,r.width(),r.height(),0,r.height(),rgb_t(0xff,0x60,0x60,0x60));
	dview_draw_line(dv,RECT_DVIEW_HSB,0,r.height(),0,0,rgb_t(0xff,0xc0,0xc0,0xc0));

	for(int x=start;x<strlen(str);x++)
	{
		if(str_x < r.width() - debug_font_width)
			dview_draw_char(dv,RECT_DVIEW_HSB,str_x,BORDER_YTHICKNESS,r.height(),rgb_t(0xff,0x00,0x00,0x00),(UINT16)str[x]);
		str_x += debug_font->char_width(r.height(),debug_font_aspect,(UINT16)str[x]) + 2*BORDER_XTHICKNESS;
	}
}

/*-------------------------------------------------
    menu_main_populate - populate the main menu
  -------------------------------------------------*/

class ui_menu_debug : public ui_menu {
public:
	ui_menu_debug(running_machine &machine, render_container *container) : ui_menu(machine, container) {}
	virtual ~ui_menu_debug() {}
	virtual void populate() override {}
	virtual void handle() override {}
};

static void CreateMainMenu(running_machine &machine)
{
	const char *subtext = "";
	int rc;
	std::string title;

	if (menu)
	{
		menu_sel = menu->get_selection();
		global_free( menu);
	}
	menu = global_alloc_clear<ui_menu_debug>(machine, &machine.render().ui_container());

	switch (focus_view->type)
	{
	case DVT_DISASSEMBLY:
		title = "Disassembly:";
		break;
	case DVT_CONSOLE:
		title = "Console:";
		break;
	case DVT_LOG:
		title = "Log:";
		break;
	case DVT_MEMORY:
		title = "Memory:";
		break;
	case DVT_STATE:
		title = "State:";
		break;
	case DVT_BREAK_POINTS:
		title = "Breakpoints:";
		break;
	case DVT_WATCH_POINTS:
		title = "Watchpoints:";
		break;
	}

	menu->item_append(title.append(focus_view->title).c_str(), nullptr, MENU_FLAG_DISABLE, nullptr);
	menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	switch (focus_view->type)
	{
	case DVT_DISASSEMBLY:
	{
		rc = downcast<debug_view_disasm *>(focus_view->view)->right_column();
		switch(rc)
		{
		case DASM_RIGHTCOL_RAW:         subtext = "Raw Opcodes"; break;
		case DASM_RIGHTCOL_ENCRYPTED:   subtext = "Enc Opcodes"; break;
		case DASM_RIGHTCOL_COMMENTS:        subtext = "Comments"; break;
		}
		menu->item_append("View", subtext, MENU_FLAG_RIGHT_ARROW, (void *)on_view_opcodes_activate);
		menu->item_append("Run to cursor", nullptr, 0, (void *)on_run_to_cursor_activate);

		if (!dview_is_state(focus_view, VIEW_STATE_FOLLOW_CPU))
		{
			menu->item_append("CPU", focus_view->view->source()->name(), MENU_FLAG_RIGHT_ARROW, (void *)on_disasm_cpu_activate);
		}
		menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
		break;
	}
	case DVT_MEMORY:
	{
		bool phys = downcast<debug_view_memory *>(focus_view->view)->physical();
		int format = downcast<debug_view_memory *>(focus_view->view)->get_data_format();
		const debug_view_source* source = downcast<debug_view_memory *>(focus_view->view)->source();
		menu->item_append("Region", source->name(), MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)on_memory_region);
		if(phys)
			menu->item_append("Address type", "Physical", MENU_FLAG_RIGHT_ARROW, (void *)on_memory_address_type);
		else
			menu->item_append("Address type", "Logical", MENU_FLAG_RIGHT_ARROW, (void *)on_memory_address_type);
		switch(format)
		{
		case 1: subtext = "1 byte chunks"; break;
		case 2: subtext = "2 byte chunks"; break;
		case 4: subtext = "4 byte chunks"; break;
		case 8: subtext = "8 byte chunks"; break;
		case 9: subtext = "32-bit floating point"; break;
		case 10: subtext = "64-bit floating point"; break;
		case 11: subtext = "80-bit floating point"; break;
		}
		menu->item_append("Format", subtext, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)on_memory_data_format);
		menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
		break;
	}
	}

	/* add input menu items */

	menu->item_append("New Memory Window", "[Ctrl+M]", 0, (void *)on_memory_window_activate);
	menu->item_append("New Disassembly Window", "[Ctrl+D]", 0, (void *)on_disassembly_window_activate);
	menu->item_append("New Error Log Window", "[Ctrl+L]", 0, (void *)on_log_window_activate);
	menu->item_append("New Breakpoints Window", "[Ctrl+B]", 0, (void *)on_bp_window_activate);
	menu->item_append("New Watchpoints Window", "[Ctrl+W]", 0, (void *)on_wp_window_activate);
	menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	menu->item_append("Run", "[F5]", 0, (void *)on_run_activate);
	menu->item_append("Run and Hide Debugger", "[F12]", 0, (void *)on_run_h_activate);
	menu->item_append("Run to Next CPU", "[F6]", 0, (void *)on_run_cpu_activate);
	menu->item_append("Run until Next Interrupt on This CPU", "[F7]", 0, (void *)on_run_irq_activate);
	menu->item_append("Run until Next VBLANK", "[F8]", 0, (void *)on_run_vbl_activate);
	menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	menu->item_append("Step Into", "[F11]", 0, (void *)on_step_into_activate);
	menu->item_append("Step Over", "[F10]", 0, (void *)on_step_over_activate);
	menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	menu->item_append("Soft Reset", "[F3]", 0, (void *)on_soft_reset_activate);
	menu->item_append("Hard Reset", "[Shift+F3]", 0, (void *)on_hard_reset_activate);
	menu->item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	if (!dview_is_state(focus_view, VIEW_STATE_FOLLOW_CPU))
		menu->item_append("Close Window", "[Shift+F4]", 0, (void *)on_close_activate);
	menu->item_append("Exit", nullptr, 0, (void *)on_exit_activate);
	menu->set_selection(menu_sel);
}

static int map_point(DView *dv, INT32 target_x, INT32 target_y, INT32 *mapped_x, INT32 *mapped_y)
{
	rectangle pos;

	/* default to point not mapped */
	*mapped_x = -1;
	*mapped_y = -1;

	pos = dv->bounds;
	pos.min_x += dv->ofs_x;
	pos.max_x += dv->ofs_x;
	pos.min_y += dv->ofs_y;
	pos.max_y += dv->ofs_y;
	//render_target_get_component_container(target, name, &pos);

	if (target_x >= pos.min_x && target_x <= pos.max_x && target_y >= pos.min_y && target_y <= pos.max_y)
	{
		*mapped_x = target_x - pos.min_x;
		*mapped_y = target_y - pos.min_y;
		return TRUE;
	}
	return FALSE;
}

static void handle_mouse(running_machine &machine)
{
	render_target * mouse_target;
	INT32           x,y;
	bool            button;

	if (menu != nullptr)
		return;

	mouse_target = machine.ui_input().find_mouse(&x, &y, &button);

	if (mouse_target == nullptr)
		return;
	//printf("mouse %d %d %d\n", x, y, button);

	for (DView *dv = list; dv != nullptr; dv = dv->next)
	{
		if (mouse_target == dv->target)
		{
			if (dview_on_mouse(dv, x, y, button))
				break;
		}
	}
}

static void handle_keys(running_machine &machine)
{
	if (menu != nullptr)
		return;

	// global keys
	if(machine.input().code_pressed_once(KEYCODE_F3))
	{
		if(machine.input().code_pressed(KEYCODE_LSHIFT))
			machine.schedule_hard_reset();
		else
		{
			machine.schedule_soft_reset();
			debug_cpu_get_visible_cpu(machine)->debug()->go();
		}
	}
	
	if(machine.input().code_pressed_once(KEYCODE_F5))
		debug_cpu_get_visible_cpu(machine)->debug()->go();
	if(machine.input().code_pressed_once(KEYCODE_F6))
		debug_cpu_get_visible_cpu(machine)->debug()->go_next_device();
	if(machine.input().code_pressed_once(KEYCODE_F7))
		debug_cpu_get_visible_cpu(machine)->debug()->go_interrupt();
	if(machine.input().code_pressed_once(KEYCODE_F8))
		debug_cpu_get_visible_cpu(machine)->debug()->go_vblank();
	if(machine.input().code_pressed_once(KEYCODE_F10))
		debug_cpu_get_visible_cpu(machine)->debug()->single_step_over();
	if(machine.input().code_pressed_once(KEYCODE_F11))
		debug_cpu_get_visible_cpu(machine)->debug()->single_step();
	if(machine.input().code_pressed_once(KEYCODE_F12))
	{
		debug_hide_all();
		debug_cpu_get_visible_cpu(machine)->debug()->go();
	}

	// TODO: make common functions to be shared here and with the menu callbacks
	if(machine.input().code_pressed_once(KEYCODE_D))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
		{
			DView *ndv;
			render_target *target;
			const debug_view_source *source;

			target = &machine.render().ui_target();

			ndv = dview_alloc(target, machine, DVT_DISASSEMBLY, 0);
			ndv->editor.active = TRUE;
			ndv->editor.container = &machine.render().ui_container();
			source = ndv->view->source();
			dview_set_title(ndv, source->name());
			ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
			win_count++;
			set_focus_view(ndv);
		}
	}
	if(machine.input().code_pressed_once(KEYCODE_M))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
		{
			DView *ndv;
			render_target *target;
			const debug_view_source *source;

			target = &machine.render().ui_target();

			ndv = dview_alloc(target, machine, DVT_MEMORY, 0);
			ndv->editor.active = TRUE;
			ndv->editor.container = &machine.render().ui_container();
			source = ndv->view->source();
			dview_set_title(ndv, source->name());
			ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
			ndv->bounds.setx(0,500);
			win_count++;

			set_focus_view(ndv);
		}
	}	
	if(machine.input().code_pressed_once(KEYCODE_L))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
		{
			DView *ndv;
			render_target *target;

			target = &machine.render().ui_target();
			ndv = dview_alloc(target, machine, DVT_LOG, 0);
			dview_set_title(ndv, "Log");
			ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
			ndv->bounds.setx(0,600);
			win_count++;
			set_focus_view(ndv);
		}
	}
	if(machine.input().code_pressed_once(KEYCODE_B))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
		{
			DView *ndv;
			render_target *target;

			target = &machine.render().ui_target();
			ndv = dview_alloc(target, machine, DVT_BREAK_POINTS, 0);
			dview_set_title(ndv, "Breakpoints");
			ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
			ndv->bounds.setx(0,600);
			win_count++;
			set_focus_view(ndv);
		}
	}
	if(machine.input().code_pressed_once(KEYCODE_W))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
		{
			DView *ndv;
			render_target *target;

			target = &machine.render().ui_target();
			ndv = dview_alloc(target, machine, DVT_WATCH_POINTS, 0);
			dview_set_title(ndv, "Watchpoints");
			ndv->ofs_x = ndv->ofs_y = win_count * TITLE_HEIGHT;
			ndv->bounds.setx(0,600);
			win_count++;
			set_focus_view(ndv);
		}
	}
	if (!dview_is_state(focus_view, VIEW_STATE_FOLLOW_CPU))
	{
		if(machine.input().code_pressed_once(KEYCODE_F4))
		{
			if(machine.input().code_pressed(KEYCODE_LSHIFT))  // use shift+F4, as ctrl+F4 is used to toggle keepaspect.
			{
				DView* dv = focus_view;
				set_focus_view(focus_view->next);
				win_count--;
				dview_free(dv);
			}
		}
	}

	
	// pass keypresses to debug view with focus
	if(machine.input().code_pressed_once(KEYCODE_UP))
		focus_view->view->process_char(DCH_UP);
	if(machine.input().code_pressed_once(KEYCODE_DOWN))
		focus_view->view->process_char(DCH_DOWN);
	if(machine.input().code_pressed_once(KEYCODE_LEFT))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
			focus_view->view->process_char(DCH_CTRLLEFT);
		else
			focus_view->view->process_char(DCH_LEFT);
	}
	if(machine.input().code_pressed_once(KEYCODE_RIGHT))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
			focus_view->view->process_char(DCH_CTRLRIGHT);
		else
			focus_view->view->process_char(DCH_RIGHT);
	}
	if(machine.input().code_pressed_once(KEYCODE_PGUP))
		focus_view->view->process_char(DCH_PUP);
	if(machine.input().code_pressed_once(KEYCODE_PGDN))
		focus_view->view->process_char(DCH_PDOWN);
	if(machine.input().code_pressed_once(KEYCODE_HOME))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
			focus_view->view->process_char(DCH_CTRLHOME);
		else
			focus_view->view->process_char(DCH_HOME);
	}
	if(machine.input().code_pressed_once(KEYCODE_END))
	{
		if(machine.input().code_pressed(KEYCODE_LCONTROL))
			focus_view->view->process_char(DCH_CTRLEND);
		else
			focus_view->view->process_char(DCH_END);
	}
}


/*-------------------------------------------------
    handle_editor - handle the editor
-------------------------------------------------*/

static void handle_editor(running_machine &machine)
{
	if (focus_view->editor.active && dview_is_state(focus_view, VIEW_STATE_VISIBLE))
	{
		ui_event event;

		/* loop while we have interesting events */
		while (machine.ui_input().pop_event(&event))
		{
			switch (event.event_type)
			{
			case UI_EVENT_CHAR:
				/* if it's a backspace and we can handle it, do so */
				if ((event.ch == 8 || event.ch == 0x7f) && focus_view->editor.str.length() > 0)
				{
					/* autoschow */
					cur_editor = &focus_view->editor;
					cur_editor->str = cur_editor->str.substr(0, cur_editor->str.length()-1);
				}
				/* if it's any other key and we're not maxed out, update */
				else if (event.ch >= ' ' && event.ch < 0x7f)
				{
					char buf[10];
					int ret;
					/* autoschow */
					cur_editor = &focus_view->editor;
					ret = utf8_from_uchar(buf, 10, event.ch);
					buf[ret] = 0;
					cur_editor->str = cur_editor->str.append(buf);
				}
				break;
			default:
				break;
			}
		}
		if (cur_editor != nullptr)
		{
			render_editor(cur_editor);
			if (machine.ui_input().pressed(IPT_UI_SELECT))
			{
				process_string(focus_view, focus_view->editor.str.c_str());
				focus_view->editor.str = "";
				cur_editor = nullptr;
			}
			if (machine.ui_input().pressed(IPT_UI_CANCEL))
				cur_editor = nullptr;
		}
	}

}


/*-------------------------------------------------
    menu_main - handle the main menu
-------------------------------------------------*/

static void handle_menus(running_machine &machine)
{
	const ui_menu_event *event;

	machine.render().ui_container().empty();
	machine.ui_input().frame_update();
	if (menu != nullptr)
	{
		/* process the menu */
		event = menu->process(0);
		if (event != nullptr && (event->iptkey == IPT_UI_SELECT || (event->iptkey == IPT_UI_LEFT) || (event->iptkey == IPT_UI_RIGHT)))
		{
			//global_free(menu);
			//menu = nullptr;
			((void (*)(DView *, const ui_menu_event *)) event->itemref)(focus_view, event);
			//ui_menu_stack_push(ui_menu_alloc(machine, menu->container, (ui_menu_handler_func)event->itemref,nullptr));
			CreateMainMenu(machine);
		}
		else if (machine.ui_input().pressed(IPT_UI_CONFIGURE))
		{
			menu_sel = menu->get_selection();
			global_free(menu);
			menu = nullptr;
		}
	}
	else
	{
		/* turn on menus if requested */
		if (machine.ui_input().pressed(IPT_UI_CONFIGURE))
			CreateMainMenu(machine);
		/* turn on editor if requested */
		//if (ui_input_pressed(machine, IPT_UI_UP) && focus_view->editor.active)
		//  cur_editor = &focus_view->editor;
		handle_editor(machine);
	}
}

//============================================================
//  followers_set_cpu
//============================================================

static void followers_set_cpu(device_t *device)
{
	std::string title;

	for (DView *dv = list; dv != nullptr; dv = dv->next)
	{
		if (dview_is_state(dv, VIEW_STATE_FOLLOW_CPU))
		{
			const debug_view_source *source = dv->view->source_for_device(device);
			switch (dv->type)
			{
			case DVT_DISASSEMBLY:
			case DVT_STATE:
				dv->view->set_source(*source);
				strprintf(title, "%s", source->name());
				dview_set_title(dv, title);
				break;
			}
		}
	}
	// and recompute the children
	//console_recompute_children(main_console);
}


static void dview_update_view(DView *dv)
{
	INT32 old_rt_width = dv->rt_width;
	INT32 old_rt_height = dv->rt_height;

	dv->rt_width = dv->target->width();
	dv->rt_height = dv->target->height();
	if (dview_is_state(dv, VIEW_STATE_NEEDS_UPDATE) || dv->rt_width != old_rt_width || dv->rt_height != old_rt_height)
	{
		dview_size_allocate(dv);
		if(dview_is_state(dv, VIEW_STATE_VISIBLE))
			dview_draw(dv);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, FALSE);
	}
}


static void update_views(void)
{
	DView *dv;

	for(dv=list;dv!=nullptr;dv=dv->next)
		dview_update_view(dv);
}


void debug_internal::wait_for_debugger(device_t &device, bool firststop)
{
	if (firststop && list == nullptr)
	{
		render_target *target = &device.machine().render().ui_target();

		//set_view_by_name(target, "Debug");
		win_count = 0;

		DView *disassembly = dview_alloc(target, device.machine(), DVT_DISASSEMBLY, VIEW_STATE_FOLLOW_CPU);
		disassembly->editor.active = TRUE;
		disassembly->editor.container = &device.machine().render().ui_container();
		disassembly->ofs_x = 500;
		disassembly->bounds.setx(0,600);
		win_count++;

		DView *statewin = dview_alloc(target, device.machine(), DVT_STATE, VIEW_STATE_FOLLOW_CPU);
		statewin->ofs_x = 300;
		statewin->bounds.set(0,200,0,600);
		win_count++;

		DView *console = dview_alloc(target, device.machine(), DVT_CONSOLE, VIEW_STATE_FOLLOW_CPU);
		dview_set_title(console, "Console");
		console->editor.active = TRUE;
		console->editor.container = &device.machine().render().ui_container();
		console->bounds.setx(0,600);
		console->ofs_x = 500;
		console->ofs_y = 300;
		win_count++;
		set_focus_view(console);
	}

	followers_set_cpu(&device);
	if(firststop)
	{
		debug_show_all();
		device.machine().ui_input().reset();
	}
	//ui_update_and_render(device.machine(), device.machine().render().ui_container()());
	update_views();
	device.machine().osd().update(false);
	handle_menus(device.machine());
	handle_mouse(device.machine());
	handle_keys(device.machine());
	//osd_sleep(osd_ticks_per_second()/60);

}

void debug_internal::debugger_update()
{
	if ((m_machine != nullptr) && (!debug_cpu_is_stopped(*m_machine)) && (m_machine->phase() == MACHINE_PHASE_RUNNING))
	{
		update_views();
	}
}

MODULE_DEFINITION(DEBUG_INTERNAL, debug_internal)
