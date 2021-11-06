// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/keyboard.cpp

    Keyboard mode menu.

***************************************************************************/

#include "emu.h"
#include "ui/keyboard.h"

#include "natkeyboard.h"


namespace ui {

namespace {

constexpr uintptr_t ITEM_KBMODE         = 0x00000100;
constexpr uintptr_t ITEM_KBDEV_FIRST    = 0x00000200;

} // anonymous namespace


menu_keyboard_mode::menu_keyboard_mode(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_keyboard_mode::populate(float &customtop, float &custombottom)
{
	natural_keyboard &natkbd(machine().natkeyboard());

	if (natkbd.can_post())
	{
		bool const natmode(natkbd.in_use());
		item_append(
				_("Keyboard Mode"),
				natmode ? _("Natural") : _("Emulated"),
				natmode ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW,
				reinterpret_cast<void *>(ITEM_KBMODE));
		item_append(menu_item_type::SEPARATOR);
	}

	uintptr_t ref(ITEM_KBDEV_FIRST);
	for (size_t i = 0; natkbd.keyboard_count() > i; ++i, ++ref)
	{
		device_t &kbddev(natkbd.keyboard_device(i));
		bool const enabled(natkbd.keyboard_enabled(i));
		item_append(
				util::string_format(
					kbddev.owner() ? _("%1$s [root%2$s]") : _("[root%2$s]"),
					kbddev.type().fullname(),
					kbddev.tag()),
				enabled ? _("Enabled") : _("Disabled"),
				enabled ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW,
				reinterpret_cast<void *>(ref));
	}
	item_append(menu_item_type::SEPARATOR);
}

menu_keyboard_mode::~menu_keyboard_mode()
{
}

void menu_keyboard_mode::handle(event const *ev)
{
	if (ev && uintptr_t(ev->itemref))
	{
		natural_keyboard &natkbd(machine().natkeyboard());
		uintptr_t const ref(uintptr_t(ev->itemref));
		bool const left(IPT_UI_LEFT == ev->iptkey);
		bool const right(IPT_UI_RIGHT == ev->iptkey);
		if (ITEM_KBMODE == ref)
		{
			if ((left || right) && (natkbd.in_use() != right))
			{
				natkbd.set_in_use(right);
				reset(reset_options::REMEMBER_REF);
			}
		}
		else if (ITEM_KBDEV_FIRST <= ref)
		{
			if ((left || right) && (natkbd.keyboard_enabled(ref - ITEM_KBDEV_FIRST) != right))
			{
				if (right)
					natkbd.enable_keyboard(ref - ITEM_KBDEV_FIRST);
				else
					natkbd.disable_keyboard(ref - ITEM_KBDEV_FIRST);
				reset(reset_options::REMEMBER_REF);
			}
		}
	}
}

} // namespace ui
