// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugviewinfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "debugviewinfo.h"

#include "debugwininfo.h"
#include "uimetrics.h"
#include "debugger.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "util/xmlfile.h"

#include "strconv.h"

#include "winutil.h"

#include <mmsystem.h>


namespace osd::debugger::win {

// debugger view styles
#define DEBUG_VIEW_STYLE    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN
#define DEBUG_VIEW_STYLE_EX 0

// combo box styles
#define COMBO_BOX_STYLE     WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL
#define COMBO_BOX_STYLE_EX  0

// horizontal scroll bar styles
#define HSCROLL_STYLE       WS_CHILD | WS_VISIBLE | SBS_HORZ
#define HSCROLL_STYLE_EX    0

// vertical scroll bar styles
#define VSCROLL_STYLE       WS_CHILD | WS_VISIBLE | SBS_VERT
#define VSCROLL_STYLE_EX    0


bool debugview_info::s_window_class_registered = false;


debugview_info::debugview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent, debug_view_type type) :
	debugbase_info(debugger),
	m_owner(owner),
	m_view(nullptr),
	m_wnd(nullptr),
	m_hscroll(nullptr),
	m_vscroll(nullptr),
	m_contextmenu(nullptr)
{
	register_window_class();

	// create the child view
	m_wnd = CreateWindowEx(DEBUG_VIEW_STYLE_EX, TEXT("MAMEDebugView"), nullptr, DEBUG_VIEW_STYLE,
			0, 0, 100, 100, parent, nullptr, GetModuleHandleUni(), this);
	if (!m_wnd)
		goto cleanup;

	// create the scroll bars
	m_hscroll = CreateWindowEx(HSCROLL_STYLE_EX, TEXT("SCROLLBAR"), nullptr, HSCROLL_STYLE,
			0, 0, 100, CW_USEDEFAULT, m_wnd, nullptr, GetModuleHandleUni(), this);
	m_vscroll = CreateWindowEx(VSCROLL_STYLE_EX, TEXT("SCROLLBAR"), nullptr, VSCROLL_STYLE,
			0, 0, CW_USEDEFAULT, 100, m_wnd, nullptr, GetModuleHandleUni(), this);
	if (!m_hscroll || !m_vscroll)
		goto cleanup;

	// create the debug view
	m_view = machine().debug_view().alloc_view(type, &debugview_info::static_update, this);
	if (!m_view)
		goto cleanup;

	return;

cleanup:
	if (m_hscroll)
		DestroyWindow(m_hscroll);
	m_hscroll = nullptr;
	if (m_vscroll)
		DestroyWindow(m_vscroll);
	m_vscroll = nullptr;
	if (m_wnd)
		DestroyWindow(m_wnd);
	m_wnd = nullptr;
	if (m_view)
		machine().debug_view().free_view(*m_view);
	m_view = nullptr;
}


debugview_info::~debugview_info()
{
	if (m_contextmenu)
		DestroyMenu(m_contextmenu);
	if (m_wnd)
		DestroyWindow(m_wnd);
	if (m_view)
		machine().debug_view().free_view(*m_view);
}


bool debugview_info::is_valid() const
{
	return m_view && m_hscroll && m_vscroll && m_wnd;
}


uint32_t debugview_info::prefwidth() const
{
	return (m_view->total_size().x * metrics().debug_font_width()) + metrics().vscroll_width();
}


uint32_t debugview_info::maxwidth()
{
	uint32_t max = m_view->total_size().x;
	debug_view_source const *const cursource = m_view->source();
	for (auto &source : m_view->source_list())
	{
		m_view->set_source(*source);
		uint32_t const chars = m_view->total_size().x;
		if (max < chars)
			max = chars;
	}
	if (cursource)
		m_view->set_source(*cursource);
	return (max * metrics().debug_font_width()) + metrics().vscroll_width();
}


void debugview_info::get_bounds(RECT &bounds) const
{
	GetWindowRect(m_wnd, &bounds);
}


void debugview_info::set_bounds(RECT const &newbounds)
{
	// account for the edges and set the bounds
	if (m_wnd)
		smart_set_window_bounds(m_wnd, GetParent(m_wnd), newbounds);

	// update
	update();
}


void debugview_info::send_vscroll(int delta)
{
	if (m_vscroll)
	{
		int message_type = SB_LINEUP;
		if (delta < 0)
		{
			message_type = SB_LINEDOWN;
			delta = -delta;
		}
		while (delta > 0)
		{
			SendMessage(m_wnd, WM_VSCROLL, message_type, (LPARAM)m_vscroll);
			delta--;
		}
	}
}


void debugview_info::send_pageup()
{
	if (m_vscroll)
		SendMessage(m_wnd, WM_VSCROLL, SB_PAGELEFT, (LPARAM)m_vscroll);
}


void debugview_info::send_pagedown()
{
	if (m_vscroll)
		SendMessage(m_wnd, WM_VSCROLL, SB_PAGERIGHT, (LPARAM)m_vscroll);
}


int debugview_info::source_index() const
{
	if (m_view)
	{
		debug_view_source const *const source = m_view->source();
		if (source)
			return m_view->source_index(*source);
	}
	return -1;
}


char const *debugview_info::source_name() const
{
	if (m_view)
	{
		debug_view_source const *const source = m_view->source();
		if (source)
			return source->name();
	}
	return "";
}


device_t *debugview_info::source_device() const
{
	if (m_view)
	{
		debug_view_source const *const source = m_view->source();
		if (source)
			return source->device();
	}
	return nullptr;
}


bool debugview_info::source_is_visible_cpu() const
{
	if (m_view)
	{
		const debug_view_source *const source = m_view->source();
		return source && (machine().debugger().console().get_visible_cpu() == source->device());
	}
	return false;
}


bool debugview_info::set_source_index(int index)
{
	if (m_view)
	{
		const debug_view_source *const source = m_view->source(index);
		if (source)
		{
			m_view->set_source(*source);
			return true;
		}
	}
	return false;
}


bool debugview_info::set_source_for_device(device_t &device)
{
	if (m_view)
	{
		const debug_view_source *const source = m_view->source_for_device(&device);
		if (source)
		{
			m_view->set_source(*source);
			return true;
		}
	}
	return false;
}


bool debugview_info::set_source_for_visible_cpu()
{
	device_t *const curcpu = machine().debugger().console().get_visible_cpu();
	if (curcpu)
		return set_source_for_device(*curcpu);
	else
		return false;
}


HWND debugview_info::create_source_combobox(HWND parent, LONG_PTR userdata)
{
	// create a combo box
	HWND const result = CreateWindowEx(COMBO_BOX_STYLE_EX, TEXT("COMBOBOX"), nullptr, COMBO_BOX_STYLE,
			0, 0, 100, 1000, parent, nullptr, GetModuleHandleUni(), nullptr);
	SetWindowLongPtr(result, GWLP_USERDATA, userdata);
	SendMessage(result, WM_SETFONT, (WPARAM)metrics().debug_font(), (LPARAM)FALSE);

	// populate the combobox
	debug_view_source const *const cursource = m_view->source();
	int maxlength = 0;
	for (auto &source : m_view->source_list())
	{
		int const length = strlen(source->name());
		if (length > maxlength)
			maxlength = length;
		auto t_name = osd::text::to_tstring(source->name());
		SendMessage(result, CB_ADDSTRING, 0, (LPARAM)t_name.c_str());
	}
	if (cursource)
	{
		SendMessage(result, CB_SETCURSEL, m_view->source_index(*cursource), 0);
		SendMessage(result, CB_SETDROPPEDWIDTH, ((maxlength + 2) * metrics().debug_font_width()) + metrics().vscroll_width(), 0);
		m_view->set_source(*cursource);
	}
	return result;
}

void debugview_info::restore_configuration_from_node(util::xml::data_node const &node)
{
	if (m_view->cursor_supported())
	{
		util::xml::data_node const *const selection = node.get_child(NODE_WINDOW_SELECTION);
		if (selection)
		{
			debug_view_xy pos = m_view->cursor_position();
			m_view->set_cursor_visible(0 != selection->get_attribute_int(ATTR_SELECTION_CURSOR_VISIBLE, m_view->cursor_visible() ? 1 : 0));
			selection->get_attribute_int(ATTR_SELECTION_CURSOR_X, pos.x);
			selection->get_attribute_int(ATTR_SELECTION_CURSOR_Y, pos.y);
			m_view->set_cursor_position(pos);
		}
	}

	util::xml::data_node const *const scroll = node.get_child(NODE_WINDOW_SCROLL);
	if (scroll)
	{
		debug_view_xy origin = m_view->visible_position();
		origin.x = scroll->get_attribute_int(ATTR_SCROLL_ORIGIN_X, origin.x * metrics().debug_font_width()) / metrics().debug_font_width();
		origin.y = scroll->get_attribute_int(ATTR_SCROLL_ORIGIN_Y, origin.y * metrics().debug_font_height()) / metrics().debug_font_height();
		m_view->set_visible_position(origin);
	}
}


void debugview_info::save_configuration_to_node(util::xml::data_node &node)
{
	if (m_view->cursor_supported())
	{
		util::xml::data_node *const selection = node.add_child(NODE_WINDOW_SELECTION, nullptr);
		if (selection)
		{
			debug_view_xy const pos = m_view->cursor_position();
			selection->set_attribute_int(ATTR_SELECTION_CURSOR_VISIBLE, m_view->cursor_visible() ? 1 : 0);
			selection->set_attribute_int(ATTR_SELECTION_CURSOR_X, pos.x);
			selection->set_attribute_int(ATTR_SELECTION_CURSOR_Y, pos.y);
		}
	}

	util::xml::data_node *const scroll = node.add_child(NODE_WINDOW_SCROLL, nullptr);
	if (scroll)
	{
		debug_view_xy const origin = m_view->visible_position();
		scroll->set_attribute_int(ATTR_SCROLL_ORIGIN_X, origin.x * metrics().debug_font_width());
		scroll->set_attribute_int(ATTR_SCROLL_ORIGIN_Y, origin.y * metrics().debug_font_height());
	}
}


void debugview_info::add_items_to_context_menu(HMENU menu)
{
	AppendMenu(menu, MF_ENABLED, ID_CONTEXT_COPY_VISIBLE, TEXT("Copy Visible"));
	AppendMenu(menu, MF_ENABLED, ID_CONTEXT_PASTE, TEXT("Paste"));
}


void debugview_info::update_context_menu(HMENU menu)
{
	EnableMenuItem(menu, ID_CONTEXT_PASTE, MF_BYCOMMAND | (IsClipboardFormatAvailable(CF_UNICODETEXT) ? MF_ENABLED : MF_GRAYED));
}


void debugview_info::handle_context_menu(unsigned command)
{
	switch (command)
	{
	case ID_CONTEXT_COPY_VISIBLE:
		{
			// get visible text
			debug_view_xy const visarea = m_view->visible_size();
			debug_view_char const *viewdata = m_view->viewdata();
			if (!viewdata)
			{
				PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
				break;
			}

			// turn into a plain string, trimming trailing whitespace
			std::wstring text;
			for (uint32_t row = 0; row < visarea.y; row++, viewdata += visarea.x)
			{
				std::wstring::size_type const start = text.length();
				for (uint32_t col = 0; col < visarea.x; ++col)
					text += wchar_t(viewdata[col].byte);
				std::wstring::size_type const nonblank = text.find_last_not_of(L"\t\n\v\r ");
				if (nonblank != std::wstring::npos)
					text.resize((std::max)(start, nonblank + 1));
				text += L"\r\n";
			}

			// copy to the clipboard
			HGLOBAL const clip = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
			if (!clip)
			{
				PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
				break;
			}
			if (!OpenClipboard(m_wnd))
			{
				GlobalFree(clip);
				PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
				break;
			}
			EmptyClipboard();
			LPWSTR const lock = reinterpret_cast<LPWSTR>(GlobalLock(clip));
			std::copy_n(text.c_str(), text.length() + 1, lock);
			GlobalUnlock(clip);
			if (!SetClipboardData(CF_UNICODETEXT, clip))
			{
				GlobalFree(clip);
				PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
			}
			CloseClipboard();
			break;
		}

	case ID_CONTEXT_PASTE:
		if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(m_wnd))
		{
			PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
		}
		else
		{
			HGLOBAL const clip = GetClipboardData(CF_UNICODETEXT);
			LPCWSTR lock = clip ? reinterpret_cast<LPCWSTR>(GlobalLock(clip)) : nullptr;
			if (!clip || !lock)
			{
				PlaySound(TEXT("SystemAsterisk"), nullptr, SND_SYNC);
			}
			else
			{
				try
				{
					while (*lock)
					{
						if ((32 <= *lock) && (127 >= *lock))
							m_view->process_char(*lock);
						++lock;
					}
				}
				catch (...)
				{
					GlobalUnlock(clip);
					CloseClipboard();
					throw;
				}
				GlobalUnlock(clip);
			}
			CloseClipboard();
		}
		break;

	default:
		osd_printf_warning("debugview_info: unhandled context menu item %u\n", command);
	}
}


void debugview_info::draw_contents(HDC windc)
{
	debug_view_char const *viewdata = m_view->viewdata();
	debug_view_xy const visarea = m_view->visible_size();

	// get the client rect
	RECT client;
	GetClientRect(m_wnd, &client);
	bool const need_filldown = client.bottom > (metrics().debug_font_height() * visarea.y);

	// create a compatible DC and an offscreen bitmap
	HDC const dc = CreateCompatibleDC(windc);
	if (!dc)
		return;
	HBITMAP const bitmap = CreateCompatibleBitmap(windc, client.right, client.bottom);
	if (!bitmap)
	{
		DeleteDC(dc);
		return;
	}
	HGDIOBJ const oldbitmap = SelectObject(dc, bitmap);
	bool const show_hscroll = m_hscroll && IsWindowVisible(m_hscroll);
	if (show_hscroll)
		client.bottom -= metrics().hscroll_height();

	// set the font
	HGDIOBJ const oldfont = SelectObject(dc, metrics().debug_font());
	COLORREF const oldfgcolor = GetTextColor(dc);
	int const oldbkmode = GetBkMode(dc);
	SetBkMode(dc, TRANSPARENT);

	// iterate over rows and columns
	for (uint32_t row = 0; row < visarea.y; row++)
	{
		bool do_filldown = (row == (visarea.y - 1)) && need_filldown;

		// loop twice; once to fill the background and once to draw the text
		for (int iter = 0; iter < 2; iter++)
		{
			COLORREF fgcolor;
			COLORREF bgcolor = metrics().view_colors(DCA_NORMAL).second;
			HBRUSH bgbrush = nullptr;
			int last_attrib = -1;
			TCHAR buffer[256];
			int count = 0;
			RECT bounds;

			// initialize the text bounds
			bounds.left = bounds.right = 0;
			bounds.top = row * metrics().debug_font_height();
			bounds.bottom = bounds.top + metrics().debug_font_height();

			// start with a brush on iteration #0
			if (iter == 0)
				bgbrush = CreateSolidBrush(bgcolor);

			// iterate over columns
			for (uint32_t col = 0; col < visarea.x; col++)
			{
				// if the attribute changed, adjust the colors
				if (viewdata[col].attrib != last_attrib)
				{
					COLORREF oldbg = bgcolor;

					// pick new colors
					std::tie(fgcolor, bgcolor) = metrics().view_colors(viewdata[col].attrib);

					// flush any pending drawing
					if (count > 0)
					{
						bounds.right = bounds.left + (count * metrics().debug_font_width());
						if (iter == 0)
						{
							FillRect(dc, &bounds, bgbrush);
							if (do_filldown)
							{
								COLORREF const filldown = metrics().view_colors(last_attrib & DCA_ANCILLARY).second;
								if (oldbg != filldown)
								{
									DeleteObject(bgbrush);
									bgbrush = CreateSolidBrush(filldown);
									oldbg = filldown;
								}
								RECT padding = bounds;
								padding.top = padding.bottom;
								padding.bottom = client.bottom;
								FillRect(dc, &padding, bgbrush);
							}
						}
						else
						{
							ExtTextOut(dc, bounds.left, bounds.top, 0, nullptr, buffer, count, nullptr);
						}
						bounds.left = bounds.right;
						count = 0;
					}

					// set the new colors
					if (iter == 1)
					{
						SetTextColor(dc, fgcolor);
					}
					else if (oldbg != bgcolor)
					{
						DeleteObject(bgbrush);
						bgbrush = CreateSolidBrush(bgcolor);
					}
					last_attrib = viewdata[col].attrib;
				}

				// add this character to the buffer
				if (viewdata[col].byte == '\t')
				{
					for (int space_num = 0; space_num < 4; space_num++)
					{
						buffer[count++] = L' ';
					}
				}
				else
				{
					buffer[count++] = viewdata[col].byte;
				}
			}

			// flush any remaining stuff
			bounds.right = bounds.left + (count * metrics().debug_font_width());
			if (iter == 0)
			{
				// erase to the end of the line
				bounds.right = client.right;
				FillRect(dc, &bounds, bgbrush);
				if (do_filldown)
				{
					COLORREF const filldown = metrics().view_colors(last_attrib & DCA_ANCILLARY).second;
					if (bgcolor != filldown)
					{
						DeleteObject(bgbrush);
						bgbrush = CreateSolidBrush(filldown);
					}
					bounds.top = bounds.bottom;
					bounds.bottom = client.bottom;
					FillRect(dc, &bounds, bgbrush);
				}
				DeleteObject(bgbrush);
			}
			else if (count > 0)
			{
				ExtTextOut(dc, bounds.left, bounds.top, 0, nullptr, buffer, count, nullptr);
			}
		}

		// advance viewdata
		viewdata += visarea.x;
	}

	// prevent garbage from showing in the corner
	if (show_hscroll)
	{
		client.top = client.bottom;
		client.bottom = client.top + metrics().hscroll_height();
		FillRect(dc, &client, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	// reset the font
	SetBkMode(dc, oldbkmode);
	SetTextColor(dc, oldfgcolor);
	SelectObject(dc, oldfont);

	// blit the final results
	BitBlt(windc, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);

	// undo the offscreen stuff
	SelectObject(dc, oldbitmap);
	DeleteObject(bitmap);
	DeleteDC(dc);
}


void debugview_info::update()
{
	SCROLLINFO scrollinfo;

	// get the updated total rows/cols and left row/col
	debug_view_xy const totalsize = m_view->total_size();
	debug_view_xy topleft = m_view->visible_position();

	// get the view window bounds
	RECT bounds;
	GetClientRect(m_wnd, &bounds);
	debug_view_xy visiblesize;
	visiblesize.x = (bounds.right - bounds.left) / metrics().debug_font_width();
	visiblesize.y = (bounds.bottom - bounds.top) / metrics().debug_font_height();

	// determine if we need to show the scrollbars
	bool const fit_hscroll = (bounds.bottom - bounds.top) > metrics().hscroll_height();
	bool show_hscroll = fit_hscroll && (totalsize.x > visiblesize.x);
	if (show_hscroll)
	{
		bounds.bottom -= metrics().hscroll_height();
		visiblesize.y = (bounds.bottom - bounds.top) / metrics().debug_font_height();
	}
	bool const fit_vscroll = (bounds.right - bounds.left) > metrics().vscroll_width();
	bool const show_vscroll = fit_vscroll && (totalsize.y > visiblesize.y);
	if (show_vscroll)
	{
		bounds.right -= metrics().vscroll_width();
		visiblesize.x = (bounds.right - bounds.left) / metrics().debug_font_width();
		if (fit_hscroll && !show_hscroll && (totalsize.x > visiblesize.x))
		{
			bounds.bottom -= metrics().hscroll_height();
			visiblesize.y = (bounds.bottom - bounds.top) / metrics().debug_font_height();
			show_hscroll = true;
		}
	}

	// compute the bounds of the scrollbars
	RECT vscroll_bounds;
	GetClientRect(m_wnd, &vscroll_bounds);
	vscroll_bounds.left = vscroll_bounds.right - metrics().vscroll_width();
	if (show_hscroll)
		vscroll_bounds.bottom -= metrics().hscroll_height();
	RECT hscroll_bounds;
	GetClientRect(m_wnd, &hscroll_bounds);
	hscroll_bounds.top = hscroll_bounds.bottom - metrics().hscroll_height();
	if (show_vscroll)
		hscroll_bounds.right -= metrics().vscroll_width();

	// if we hid the scrollbars, make sure we reset the top/left corners
	if (topleft.y + visiblesize.y > totalsize.y)
		topleft.y = std::max(totalsize.y - visiblesize.y, 0);
	if (topleft.x + visiblesize.x > totalsize.x)
		topleft.x = std::max(totalsize.x - visiblesize.x, 0);

	// fill out the scroll info struct for the vertical scrollbar
	if (m_vscroll)
	{
		scrollinfo.cbSize = sizeof(scrollinfo);
		scrollinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrollinfo.nMin = 0;
		scrollinfo.nMax = totalsize.y - 1;
		scrollinfo.nPage = visiblesize.y;
		scrollinfo.nPos = topleft.y;
		SetScrollInfo(m_vscroll, SB_CTL, &scrollinfo, TRUE);
	}

	// fill out the scroll info struct for the horizontal scrollbar
	if (m_hscroll)
	{
		scrollinfo.cbSize = sizeof(scrollinfo);
		scrollinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrollinfo.nMin = 0;
		scrollinfo.nMax = totalsize.x - 1;
		scrollinfo.nPage = visiblesize.x;
		scrollinfo.nPos = topleft.x;
		SetScrollInfo(m_hscroll, SB_CTL, &scrollinfo, TRUE);
	}

	// update window info
	if (((bounds.right - bounds.left) > (visiblesize.x * metrics().debug_font_width())) && ((topleft.x + visiblesize.x) < totalsize.x))
		visiblesize.x++;
	if (((bounds.bottom - bounds.top) > (visiblesize.y * metrics().debug_font_height())) && ((topleft.y + visiblesize.y) < totalsize.y))
		visiblesize.y++;
	m_view->set_visible_size(visiblesize);
	m_view->set_visible_position(topleft);

	// invalidate the bounds
	InvalidateRect(m_wnd, nullptr, FALSE);

	// adjust the bounds of the scrollbars and show/hide them
	if (m_vscroll)
	{
		if (show_vscroll)
			smart_set_window_bounds(m_vscroll, m_wnd, vscroll_bounds);
		smart_show_window(m_vscroll, show_vscroll);
	}
	if (m_hscroll)
	{
		if (show_hscroll)
			smart_set_window_bounds(m_hscroll, m_wnd, hscroll_bounds);
		smart_show_window(m_hscroll, show_hscroll);
	}
}


uint32_t debugview_info::process_scroll(WORD type, HWND wnd)
{
	// get the current info
	SCROLLINFO scrollinfo;
	scrollinfo.cbSize = sizeof(scrollinfo);
	scrollinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(wnd, SB_CTL, &scrollinfo);

	// by default we stay put
	int32_t result = scrollinfo.nPos;

	// determine the maximum value
	int32_t const maxval = (scrollinfo.nMax > scrollinfo.nPage) ? (scrollinfo.nMax - scrollinfo.nPage + 1) : 0;

	// handle the message
	switch (type)
	{
	case SB_THUMBTRACK:
		result = scrollinfo.nTrackPos;
		break;

	case SB_LEFT:
		result = 0;
		break;

	case SB_RIGHT:
		result = maxval;
		break;

	case SB_LINELEFT:
		result -= 1;
		break;

	case SB_LINERIGHT:
		result += 1;
		break;

	case SB_PAGELEFT:
		result -= scrollinfo.nPage - 1;
		break;

	case SB_PAGERIGHT:
		result += scrollinfo.nPage - 1;
		break;
	}

	// generic rangecheck
	if (result < 0)
		result = 0;
	if (result > maxval)
		result = maxval;

	// set the new position
	scrollinfo.fMask = SIF_POS;
	scrollinfo.nPos = result;
	SetScrollInfo(wnd, SB_CTL, &scrollinfo, TRUE);

	return uint32_t(result);
}


bool debugview_info::process_context_menu(int x, int y)
{
	// don't show a menu if not in client rect
	RECT clientrect;
	GetClientRect(m_wnd, &clientrect);
	POINT loc{ x, y };
	ScreenToClient(m_wnd, &loc);
	if (!PtInRect(&clientrect, loc))
		return false;

	// create the context menu if we haven’t already
	if (!m_contextmenu)
	{
		m_contextmenu = CreatePopupMenu();
		if (!m_contextmenu)
			return false;
		add_items_to_context_menu(m_contextmenu);
	}

	// show the context menu
	update_context_menu(m_contextmenu);
	BOOL const command(TrackPopupMenu(
				m_contextmenu,
				(GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN) | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
				x, y,
				0,
				m_wnd,
				nullptr));
	if (command)
		handle_context_menu(command);
	return true;
}


LRESULT debugview_info::view_proc(UINT message, WPARAM wparam, LPARAM lparam)
{
	// handle a few messages
	switch (message)
	{
	// paint: redraw the last bitmap
	case WM_PAINT:
		{
			PAINTSTRUCT pstruct;
			HDC const dc = BeginPaint(m_wnd, &pstruct);
			draw_contents(dc);
			EndPaint(m_wnd, &pstruct);
			break;
		}

	// keydown: handle debugger keys
	case WM_SYSKEYDOWN:
		if (wparam != VK_F10)
			return DefWindowProc(m_wnd, message, wparam, lparam);
		[[fallthrough]];
	case WM_KEYDOWN:
		{
			if (m_owner.handle_key(wparam, lparam))
			{
				m_owner.set_ignore_char_lparam(lparam);
			}
			else
			{
				switch (wparam)
				{
				case VK_UP:
					m_view->process_char(DCH_UP);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_DOWN:
					m_view->process_char(DCH_DOWN);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_LEFT:
					if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
						m_view->process_char(DCH_CTRLLEFT);
					else
						m_view->process_char(DCH_LEFT);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_RIGHT:
					if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
						m_view->process_char(DCH_CTRLRIGHT);
					else
						m_view->process_char(DCH_RIGHT);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_PRIOR:
					m_view->process_char(DCH_PUP);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_NEXT:
					m_view->process_char(DCH_PDOWN);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_HOME:
					if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
						m_view->process_char(DCH_CTRLHOME);
					else
						m_view->process_char(DCH_HOME);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_END:
					if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
						m_view->process_char(DCH_CTRLEND);
					else
						m_view->process_char(DCH_END);
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_ESCAPE:
					m_owner.set_default_focus();
					m_owner.set_ignore_char_lparam(lparam);
					break;

				case VK_TAB:
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						m_owner.prev_view(this);
					else
						m_owner.next_view(this);
					m_owner.set_ignore_char_lparam(lparam);
					break;
				}
			}
			break;
		}

	// char: ignore chars associated with keys we've handled
	case WM_CHAR:
		if (m_owner.check_ignore_char_lparam(lparam))
		{
			if (waiting_for_debugger() || !seq_pressed())
			{
				if (wparam >= 32 && wparam < 127)
				{
					if (m_view->cursor_supported())
						m_view->set_cursor_visible(true);
					m_view->process_char(wparam);
				}
				else
				{
					return DefWindowProc(m_wnd, message, wparam, lparam);
				}
			}
		}
		break;

	// gaining focus
	case WM_SETFOCUS:
		if (m_view->cursor_supported())
			m_view->set_cursor_visible(true);
		break;

	// losing focus
	case WM_KILLFOCUS:
		if (m_view->cursor_supported())
			m_view->set_cursor_visible(false);
		break;

	// mouse click
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			debug_view_xy const topleft = m_view->visible_position();
			debug_view_xy const visiblesize = m_view->visible_size();
			debug_view_xy newpos;
			newpos.x = std::max(std::min<int>(topleft.x + GET_X_LPARAM(lparam) / metrics().debug_font_width(), topleft.x + visiblesize.x - 1), 0);
			newpos.y = std::max(std::min<int>(topleft.y + GET_Y_LPARAM(lparam) / metrics().debug_font_height(), topleft.y + visiblesize.y - 1), 0);
			m_view->process_click((message == WM_LBUTTONDOWN) ? DCK_LEFT_CLICK : DCK_MIDDLE_CLICK, newpos);
			SetFocus(m_wnd);
			break;
		}

	// right click
	case WM_RBUTTONDOWN:
		if (m_view->cursor_supported())
		{
			debug_view_xy const topleft = m_view->visible_position();
			debug_view_xy const visiblesize = m_view->visible_size();
			debug_view_xy newpos;
			newpos.x = std::max(std::min<int>(topleft.x + GET_X_LPARAM(lparam) / metrics().debug_font_width(), topleft.x + visiblesize.x - 1), 0);
			newpos.y = std::max(std::min<int>(topleft.y + GET_Y_LPARAM(lparam) / metrics().debug_font_height(), topleft.y + visiblesize.y - 1), 0);
			m_view->set_cursor_position(newpos);
			m_view->set_cursor_visible(true);
		}
		return DefWindowProc(m_wnd, message, wparam, lparam);

	// horizontal scroll
	case WM_HSCROLL:
		{
			debug_view_xy topleft = m_view->visible_position();
			topleft.x = process_scroll(LOWORD(wparam), (HWND)lparam);
			m_view->set_visible_position(topleft);
			machine().debug_view().flush_osd_updates();
			break;
		}

	// vertical scroll
	case WM_VSCROLL:
		{
			debug_view_xy topleft = m_view->visible_position();
			topleft.y = process_scroll(LOWORD(wparam), (HWND)lparam);
			m_view->set_visible_position(topleft);
			machine().debug_view().flush_osd_updates();
			break;
		}

	case WM_CONTEXTMENU:
		if (!process_context_menu(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)))
			return DefWindowProc(m_wnd, message, wparam, lparam);
		break;

	// everything else: defaults
	default:
		return DefWindowProc(m_wnd, message, wparam, lparam);
	}

	return 0;
}


void debugview_info::static_update(debug_view &view, void *osdprivate)
{
	auto *const info = (debugview_info *)osdprivate;
	assert(info->m_view == &view);
	info->update();
}


LRESULT CALLBACK debugview_info::static_view_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_CREATE)
	{
		// set the info pointer
		CREATESTRUCT const *const createinfo = (CREATESTRUCT *)lparam;
		SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)createinfo->lpCreateParams);
		return 0;
	}

	auto *const info = (debugview_info *)(uintptr_t)GetWindowLongPtr(wnd, GWLP_USERDATA);
	if (info == nullptr)
		return DefWindowProc(wnd, message, wparam, lparam);

	assert((info->m_wnd == wnd) || (info->m_wnd == nullptr));
	return info->view_proc(message, wparam, lparam);
}


void debugview_info::register_window_class()
{
	if (!s_window_class_registered)
	{
		WNDCLASS wc = { 0 };

		// initialize the description of the window class
		wc.lpszClassName    = TEXT("MAMEDebugView");
		wc.hInstance        = GetModuleHandleUni();
		wc.lpfnWndProc      = &debugview_info::static_view_proc;
		wc.hCursor          = LoadCursor(nullptr, IDC_ARROW);
		wc.hIcon            = LoadIcon(wc.hInstance, MAKEINTRESOURCE(2));
		wc.lpszMenuName     = nullptr;
		wc.hbrBackground    = nullptr;
		wc.style            = 0;
		wc.cbClsExtra       = 0;
		wc.cbWndExtra       = 0;

		UnregisterClass(wc.lpszClassName, wc.hInstance);

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			fatalerror("Unable to register debug view class\n");

		s_window_class_registered = true;
	}
}

} // namespace osd::debugger::win
