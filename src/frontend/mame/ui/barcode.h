// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    ui/barcode.h

    "Barcode Reader" control

***************************************************************************/

#ifndef MAME_FRONTEND_UI_BARCODE_H
#define MAME_FRONTEND_UI_BARCODE_H

#pragma once

#include "ui/devctrl.h"

#include "machine/bcreader.h"

#include <string>


namespace ui {

class menu_barcode_reader : public menu_device_control<barcode_reader_device> {
public:
	menu_barcode_reader(mame_ui_manager &mui, render_container &container, barcode_reader_device *device);
	virtual ~menu_barcode_reader() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	std::string m_barcode_buffer;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_BARCODE_H
