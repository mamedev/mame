// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    ui/barcode.h

    "Barcode Reader" control

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_BARCODE_H
#define MAME_FRONTEND_UI_BARCODE_H

#include "machine/bcreader.h"
#include "ui/devctrl.h"

namespace ui {
class menu_barcode_reader : public menu_device_control<barcode_reader_device> {
public:
	menu_barcode_reader(mame_ui_manager &mui, render_container *container, barcode_reader_device *device);
	virtual ~menu_barcode_reader() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	char  m_barcode_buffer[20];
};

} // namespace ui

#endif // MAME_FRONTEND_UI_BARCODE_H
