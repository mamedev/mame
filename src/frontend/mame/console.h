// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    console.c

    Console interface frontend for MAME.

***************************************************************************/
#ifndef MAME_FRONTEND_CONSOLE_H
#define MAME_FRONTEND_CONSOLE_H

#pragma once

#include "emu.h"

class console_frontend
{
public:
	// construction/destruction
	console_frontend(emu_options &options, osd_interface &osd);
	~console_frontend();

	void start_console();
	
private:
	// internal state
	emu_options &       m_options;
	osd_interface &     m_osd;
};

#endif  /* MAME_FRONTEND_CONSOLE_H */
