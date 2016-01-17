// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pocketc.h
 *
 ****************************************************************************/

#ifndef POCKETC_H_
#define POCKETC_H_

typedef const char *POCKETC_FIGURE[];

class pocketc_state : public driver_device
{
public:
	pocketc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_PALETTE_INIT(pocketc);

	static const unsigned short pocketc_colortable[8][2];
	void pocketc_draw_special(bitmap_ind16 &bitmap,int x, int y, const POCKETC_FIGURE fig, int color);
};

#endif /* POCKETC_H_ */
