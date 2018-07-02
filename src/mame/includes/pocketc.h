// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pocketc.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_POCKETC_H
#define MAME_INCLUDES_POCKETC_H

#include "emupal.h"

typedef const char *POCKETC_FIGURE[];

class pocketc_state : public driver_device
{
public:
	pocketc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void pocketc(machine_config &config);

protected:
	void pocketc_draw_special(bitmap_ind16 &bitmap,int x, int y, const POCKETC_FIGURE fig, int color);

	static const unsigned short pocketc_colortable[8][2];

private:
	DECLARE_PALETTE_INIT(pocketc);

};

#endif // MAME_INCLUDES_POCKETC_H
