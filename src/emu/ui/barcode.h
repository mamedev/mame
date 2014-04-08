/***************************************************************************

    ui/barcode.h

    MESS's "barcode reader" control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_BARCODE_H__
#define __UI_BARCODE_H__

#include "machine/bcreader.h"

class ui_menu_barcode_code : public ui_menu {
public:
	ui_menu_barcode_code(running_machine &machine, render_container *container, barcode_reader_device *reader);
	virtual ~ui_menu_barcode_code();
	virtual void populate();
	virtual void handle();

private:
	barcode_reader_device *m_reader;
	char  m_barcode_buffer[20];
};


class ui_menu_barcode_reader : public ui_menu {
public:
	ui_menu_barcode_reader(running_machine &machine, render_container *container);
	virtual ~ui_menu_barcode_reader();
	virtual void populate();
	virtual void handle();
};

#endif // __UI_BARCODE_H__
