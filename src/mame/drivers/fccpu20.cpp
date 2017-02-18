// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-20 and CPU-21 VME SBC drivers
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

//**************************************************************************
//  CONFIGURABLE LOGGING
//**************************************************************************

#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

class cpu20_state : public driver_device
{
public:
cpu20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
	{
	}
	virtual void machine_start () override { LOGSETUP("%s\n", FUNCNAME); }
//  virtual void machine_reset () override;

	DECLARE_DRIVER_INIT(cpu20)      { LOGSETUP("%s\n", FUNCNAME); }
	DECLARE_DRIVER_INIT(cpu21s)     { LOGSETUP("%s\n", FUNCNAME); }
	DECLARE_DRIVER_INIT(cpu21)      { LOGSETUP("%s\n", FUNCNAME); }
	DECLARE_DRIVER_INIT(cpu21a)     { LOGSETUP("%s\n", FUNCNAME); }
	DECLARE_DRIVER_INIT(cpu21ya)    { LOGSETUP("%s\n", FUNCNAME); }
	DECLARE_DRIVER_INIT(cpu21b)     { LOGSETUP("%s\n", FUNCNAME); }
	DECLARE_DRIVER_INIT(cpu21yb)    { LOGSETUP("%s\n", FUNCNAME); }
};

/* Input ports */
static INPUT_PORTS_START (cpu20)
INPUT_PORTS_END

/* Slot interfaces */
static SLOT_INTERFACE_START(cpu20_vme_cards)
	SLOT_INTERFACE("fccpu20", VME_FCCPU20)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(cpu21s_vme_cards)
	SLOT_INTERFACE("fccpu21s", VME_FCCPU21S)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(cpu21_vme_cards)
	SLOT_INTERFACE("fccpu21", VME_FCCPU21)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(cpu21a_vme_cards)
	SLOT_INTERFACE("fccpu21a", VME_FCCPU21A)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(cpu21ya_vme_cards)
	SLOT_INTERFACE("fccpu21ya", VME_FCCPU21YA)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(cpu21b_vme_cards)
	SLOT_INTERFACE("fccpu21b", VME_FCCPU21B)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(cpu21yb_vme_cards)
	SLOT_INTERFACE("fccpu21yb", VME_FCCPU21YB)
SLOT_INTERFACE_END

/* Machine configurations */
MACHINE_CONFIG_START (cpu20, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu20_vme_cards, "fccpu20")
MACHINE_CONFIG_END

MACHINE_CONFIG_START (cpu21s, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu21s_vme_cards, "fccpu21s")
MACHINE_CONFIG_END

MACHINE_CONFIG_START (cpu21, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu21_vme_cards, "fccpu21")
MACHINE_CONFIG_END

MACHINE_CONFIG_START (cpu21a, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu21a_vme_cards, "fccpu21a")
MACHINE_CONFIG_END

MACHINE_CONFIG_START (cpu21ya, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu21ya_vme_cards, "fccpu21ya")
MACHINE_CONFIG_END

MACHINE_CONFIG_START (cpu21b, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu21b_vme_cards, "fccpu21b")
MACHINE_CONFIG_END

MACHINE_CONFIG_START (cpu21yb, cpu20_state)
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, cpu21yb_vme_cards, "fccpu21yb")
MACHINE_CONFIG_END

/* ROM configurations */
ROM_START(fccpu20sbc)   ROM_END

/* Boards supported by same rom set, need to do like this to avoid need for multi named rom sets */
#define rom_fccpu21ssbc     rom_fccpu20sbc
#define rom_fccpu21sbc      rom_fccpu20sbc
#define rom_fccpu21asbc     rom_fccpu20sbc
#define rom_fccpu21yasbc    rom_fccpu20sbc
#define rom_fccpu21bsbc     rom_fccpu20sbc
#define rom_fccpu21ybsbc    rom_fccpu20sbc

/* Driver */
/*    YEAR  NAME          PARENT    COMPAT MACHINE         INPUT     CLASS          INIT         COMPANY                   FULLNAME                FLAGS */
COMP (1986, fccpu20sbc,    0,          0,  cpu20,          cpu20,    driver_device, 0,           "Force Computers Gmbh",   "SYS68K/CPU-20",        MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
COMP (1986, fccpu21ssbc,   fccpu20sbc, 0,  cpu21s,         cpu20,    cpu20_state,   cpu21s,      "Force Computers Gmbh",   "SYS68K/CPU-21S",       MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
COMP (1986, fccpu21sbc,    fccpu20sbc, 0,  cpu21,          cpu20,    cpu20_state,   cpu21,       "Force Computers Gmbh",   "SYS68K/CPU-21",        MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
COMP (1986, fccpu21asbc,   fccpu20sbc, 0,  cpu21a,         cpu20,    cpu20_state,   cpu21a,      "Force Computers Gmbh",   "SYS68K/CPU-21A",       MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
COMP (1986, fccpu21yasbc,  fccpu20sbc, 0,  cpu21ya,        cpu20,    cpu20_state,   cpu21ya,     "Force Computers Gmbh",   "SYS68K/CPU-21YA",      MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
COMP (1986, fccpu21bsbc,   fccpu20sbc, 0,  cpu21b,         cpu20,    cpu20_state,   cpu21b,      "Force Computers Gmbh",   "SYS68K/CPU-21B",       MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
COMP (1986, fccpu21ybsbc,  fccpu20sbc, 0,  cpu21yb,        cpu20,    cpu20_state,   cpu21yb,     "Force Computers Gmbh",   "SYS68K/CPU-21YB",      MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
