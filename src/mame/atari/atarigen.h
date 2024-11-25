// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarigen.h

    General functions for Atari games.

***************************************************************************/

#ifndef MAME_ATARI_ATARIGEN_H
#define MAME_ATARI_ATARIGEN_H

#include "screen.h"


/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

class atarigen_state : public driver_device
{
public:
	// construction/destruction
	atarigen_state(const machine_config &mconfig, device_type type, const char *tag);

protected:
	// users must call through to these
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// video helpers
	void halt_until_hblank_0(device_t &device, screen_device &screen);

	// misc helpers
	void blend_gfx(int gfx0, int gfx1, int mask0, int mask1);
	TIMER_CALLBACK_MEMBER(unhalt_cpu);

	required_device<cpu_device> m_maincpu;

	optional_device<gfxdecode_device> m_gfxdecode;
	optional_device<screen_device> m_screen;

	std::unique_ptr<u8[]> m_blended_data;

	emu_timer *m_unhalt_cpu_timer;
};



/***************************************************************************
    GENERAL ATARI NOTES
**************************************************************************##

    Atari 68000 list:

    Driver        Pr? Up? VC? PF? P2? MO? AL? BM? PH?
    ----------    --- --- --- --- --- --- --- --- ---
    arcadecl.cpp       *               *       *
    atarig1.cpp        *       *      rle  *
    atarig42.cpp       *       *      rle  *
    atarigt.cpp                *      rle  *
    atarigx2.cpp               *      rle  *
    atarisy1.cpp   *   *       *       *   *              270->260
    atarisy2.cpp   *   *       *       *   *              150->120
    badlands.cpp       *       *       *                  250->260
    batman.cpp     *   *   *   *   *   *   *       *      200->160 ?
    blstroid.cpp       *       *       *                  240->230
    cyberbal.cpp       *       *       *   *              125->105 ?
    eprom.cpp          *       *       *   *              170->170
    gauntlet.cpp   *   *       *       *   *       *      220->250
    klax.cpp       *   *       *       *                  480->440 ?
    offtwall.cpp       *   *   *       *                  260->260
    rampart.cpp        *               *       *          280->280
    relief.cpp     *   *   *   *   *   *                  240->240
    shuuz.cpp          *   *   *       *                  410->290 fix!
    skullxbo.cpp       *       *       *   *              150->145
    thunderj.cpp       *   *   *   *   *   *       *      180->180
    toobin.cpp         *       *       *   *              140->115 fix!
    vindictr.cpp   *   *       *       *   *       *      200->210
    xybots.cpp     *   *       *       *   *              235->238
    ----------  --- --- --- --- --- --- --- --- ---

    Pr? - do we have verifiable proof on priorities?
    Up? - have we updated to use new MO's & tilemaps?
    VC? - does it use the video controller?
    PF? - does it have a playfield?
    P2? - does it have a dual playfield?
    MO? - does it have MO's?
    AL? - does it have an alpha layer?
    BM? - does it have a bitmap layer?
    PH? - does it use the palette hack?

***************************************************************************/


#endif // MAME_ATARI_ATARIGEN_H
