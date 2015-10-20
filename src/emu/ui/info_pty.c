// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.c

    Information screen on pseudo terminals

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/info_pty.h"

ui_menu_pty_info::ui_menu_pty_info(running_machine &machine, render_container *container) :
    ui_menu(machine, container)
{
}

ui_menu_pty_info::~ui_menu_pty_info()
{
}

void ui_menu_pty_info::populate()
{
    item_append("Pseudo terminals", NULL, MENU_FLAG_DISABLE, NULL);
    item_append("", NULL, MENU_FLAG_DISABLE, NULL);

    pty_interface_iterator iter(machine().root_device());
    for (device_pty_interface *pty = iter.first(); pty != NULL; pty = iter.next()) {
        const char *port_name = pty->device().owner()->tag() + 1;
        if (pty->is_open()) {
            item_append(port_name , pty->slave_name() , MENU_FLAG_DISABLE , NULL);
        } else {
            item_append(port_name , "[failed]" , MENU_FLAG_DISABLE , NULL);
        }
        item_append("", NULL, MENU_FLAG_DISABLE, NULL);
    }
}

void ui_menu_pty_info::handle()
{
    process(0);
}
