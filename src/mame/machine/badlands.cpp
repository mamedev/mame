// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************

    Badlands

    functions required by both badlands.cpp and
    badlandsbl.cpp drivers.

    Eventually this file will be nuked once both i/o and
    gfx_layout systems are properly state-ized.

***************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "screen.h"

/*************************************
 *
 *  GFX MO layout
 *
 *************************************/

// TODO: doesn't link?
#ifdef UNUSED_FUNCTION
const gfx_layout badlands_molayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 },
	{ 0*8, 8*8, 16*8, 24*8, 32*8, 40*8, 48*8, 56*8 },
	64*8
};
#endif

/*************************************
 *
 *  Port definitions
 *
 *************************************/

INPUT_PORTS_START( badlands )
	PORT_START("FE4000")    /* fe4000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // old steering wheels
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // old gas pedals
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // freeze-step
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Freeze") // freeze
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Start / Fire")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Start / Fire")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FE6000")    /* fe6000 */
	PORT_BIT( 0x00ff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FE6002")    /* fe6002 */
	PORT_BIT( 0x00ff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AUDIO")     /* audio port */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* self test */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")   /* response buffer full */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")    /* command buffer full */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* self test */

	PORT_START("PEDALS")    /* fake for pedals */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Pedal")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Pedal")
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

