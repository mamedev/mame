// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-20 VME SBC drivers
 *
 * The only purpose of this driver is to be able to start the VME board as
 * a single board computer and not as a slot device. It allows the board to
 * have a layout and number of other goodies as board driver too but the chassi
 * emulated here is really imaginary representing any chassi that provides power
 * and used without any other VME board installed.
 *
 * This board occupies two mechanical slots but is only using one slot.
 *
 * If you want to combine multiple VME boards there are a few multi VME slot chassi
 * board drivers which will allow you to do that, such as the 'miniforce' driver.
 *
 * +=============================+
 * |  CPU-20   |  SYS68K/PWR-09A |
 * |           |                 |
 * | RST       |                 |
 * | ABT       |O +5v            |
 * |           |O +12v           |
 * |O RUN O RUN|O -12v           |
 * |O HLT      |O ON             |
 * |O BM       |                 |
 * |           |                 |
 * |O FLM O SL0|                 |
 * |O EPR O SL1|                 |
 * |O 2WS      |    +-------+    |
 * |O 4WS      |    |   o   |PWR |
 * |O 6WS      |    |       |    |
 * |O 8WS      |    +-------+    |
 * |O12WS      |                 |
 * |O14WS      |                 |
 * |           |                 |
 * | CSH       |                 |
 * | R/M       |                 |
 * |           |                 |
 * |  o        |                 |
 * |  o        |                 |
 * |  o        |                 |
 * |  o        |                 |
 * | RS232/422 |                 |
 * | P4    P3  |                 |
 * |           |                 |
 * |SLOT1| 1U  |                 |
 * +=============================+
 *
 * History of Force Computers
 *---------------------------
 * see fccpu30.cpp
 *
 * Misc links about Force Computes and this board:
 *------------------------------------------------
 * See the CPU-20 slot device code in vme_fccpu20.cpp
 *
 *---------------------------------------------------------------------------
 *  TODO:
 *  - Add front layout
 ****************************************************************************/

#include "emu.h"
#include "bus/vme/vme.h"
#include "bus/vme/vme_fccpu20.h"

class cpu20_state : public driver_device
{
public:
cpu20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag) { }
};

/* Input ports */
static INPUT_PORTS_START (cpu20)
INPUT_PORTS_END

static SLOT_INTERFACE_START(cpu20_vme_cards)
	SLOT_INTERFACE("fccpu20", VME_FCCPU20)
SLOT_INTERFACE_END

/* Machine configuration */
MACHINE_CONFIG_START (cpu20, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", "slot1", cpu20_vme_cards, "fccpu20")
MACHINE_CONFIG_END

/* ROM configuration */
ROM_START(fccpu20sbc)
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT         COMPANY                   FULLNAME                FLAGS */
COMP (1986, fccpu20sbc,   0,       0,      cpu20,          cpu20,    driver_device,      0,      "Force Computers Gmbh",   "SYS68K/CPU-20",        MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
