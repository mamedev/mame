// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Mizar VME8300 rev G 3U VME slave slot device
 *
 *  23/09/2015
 *
 * This device was drycoded based on OS9 boot strap code on a Mizar mz8105 board
 * which expects to find a SIO on the VME bus + photos of a Mizar 8300 board on Ebay
 * I have found no formal documents for this board so far, so needs verification.
 *
 *         ||
 *         ||
 *         ||
 *         ||
 *         ||____________________________________________________________   ___
 * \+++====|| U2|AM26LS32|   |  NEC               | |74LS04N||74LS645   ||_|   |
 *  \=/-  o||   +--------+   |  D7201C            | ++-----+++----------+| |   |
 *   |  |  ||      +-------+ +--------------------+  |     | |SN74LS374N|| |   |
 *   |  |  ||    U1| xxx   |  ____________________   |     | +----------+| |   |
 *   |  |  ||      +-------+ |  NEC               |  |AMD  | |SN74LS374N|| |   |
 *   |  |  ||                |  7201C             |  |AM9513++----------+| |   |
 *   |  |  ||       K10      +--------------------+  | APC ||PAL14L8    || |VME|
 *   |  |==||            +-------+  K5       _______ |     |+-----------+| |   |
 *   |  |==||            |MC1488P|   K4     |SN74S38|| STC ||PAL20L8    || |P1 |
 *   |  |  ||           ++-------+--------+-+------++|     |+-----------+| |   |
 *   |  |  ||       K2  |AM26LS32|AM26LS32| 74S74  | |_____| |SN74LS244N|| |   |
 *   |  |  ||           +--------+--------+--------+_______  +----------+| |   |
 *   |  |  ||                             | 74S74  | 74F85 |             | |   |
 *   |  |  ||           +-------+--------++-------++-------+ +----------+| |   |
 *  /=\-  o||J1    K1 U4| xxx   |  xxx   | 74LS164|   K6     |AM25LS2521|| |   |
 * /+++====||  J2       +-------+--------+--------+--------+ +----------+|_|   |
 *         ||Rev G    U3| MC1488| MC1488 | 74LS161|  74F85 |      K8     | |___|
 *         ||-----------+-------+-----------------------------------------
 *         ||
 *         ||
 *
 *
 * Misc links about this board:
 * http://www.ebay.com/itm/MIZAR-INC-8300-0-01-REV-J-INTERFACE-CONTROL-BOARD-W-RIBBON-AND-PLATE-/231508658429?hash=item35e6fdc8fd
 *
 * Description
 * ------------
 * The Mizar mz8300 is a Quad Serial board.
 *
 *      -  Single High (3U) VME Slave board
 *      -  Two upd7201 SIO Serial Input/Ouput
 *      -  One AM9513 STC System Timing Controller
 *
 * Address Map (just guesses based on driver software behaviours)
 * --------------------------------------------------------------------------
 * Local          VME                  Decscription
 * -------------------------------------------------------------------------
 *  n/a           0xff0000 0xff0003   mzr8105.c Bootstrap expects to find a
 *                                    UPD7201 serial device here - configurable!
 * --------------------------------------------------------------------------
 *
 * Interrupt sources MVME
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 *
 * Off board Sources (other VME boards)
 *
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Setup a working address map (STARTED)
 *  - Get documentation for the board
 *  - Add VME bus interface
 *  - Hook up a CPU board that supports this board (mzr8105.c)
 *  - Get terminal working through this device over the VME interface
 *
 ****************************************************************************/

#include "emu.h"
#include "vme_mzr8300.h"

#include "machine/am9513.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_MZR8300, vme_mzr8300_card_device, "mzr8300", "Mizar 8300 SIO serial board")

/* These values are borrowed just to get the terminal going and should be replaced
 * once a proper serial board hardware (ie MZ 8300) is found and emulated. */


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vme_mzr8300_card_device::device_add_mconfig(machine_config &config)
{
	upd7201_new_device& sio0(UPD7201_NEW(config, "sio0", XTAL(4'000'000)));
	sio0.out_txdb_callback().set("rs232p1", FUNC(rs232_port_device::write_txd));
	sio0.out_dtrb_callback().set("rs232p1", FUNC(rs232_port_device::write_dtr));
	sio0.out_rtsb_callback().set("rs232p1", FUNC(rs232_port_device::write_rts));

	UPD7201_NEW(config, "sio1", XTAL(4'000'000));

	rs232_port_device &rs232p1(RS232_PORT(config, "rs232p1", default_rs232_devices, "terminal"));
	rs232p1.rxd_handler().set("sio0", FUNC(upd7201_new_device::rxb_w));
	rs232p1.cts_handler().set("sio0", FUNC(upd7201_new_device::ctsb_w));

	am9513_device &stc(AM9513(config, "stc", 4_MHz_XTAL));
	stc.out1_cb().set("sio0", FUNC(upd7201_new_device::rxca_w));
	stc.out1_cb().append("sio0", FUNC(upd7201_new_device::txca_w));
	stc.out2_cb().set("sio0", FUNC(upd7201_new_device::rxcb_w));
	stc.out2_cb().append("sio0", FUNC(upd7201_new_device::txcb_w));
	stc.out3_cb().set("sio1", FUNC(upd7201_new_device::rxca_w));
	stc.out3_cb().append("sio1", FUNC(upd7201_new_device::txca_w));
	stc.out4_cb().set("sio1", FUNC(upd7201_new_device::rxcb_w));
	stc.out4_cb().append("sio1", FUNC(upd7201_new_device::txcb_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

vme_mzr8300_card_device::vme_mzr8300_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vme_card_interface(mconfig, *this)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_mzr8300_card_device::vme_mzr8300_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vme_mzr8300_card_device(mconfig, VME_MZR8300, tag, owner, clock)
{
}

void vme_mzr8300_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);
	set_vme_device();

	/* Setup r/w handlers for first SIO in A16 */
	uint32_t base = 0xFF0000;
	//  m_vme->static_set_custom_spaces(*this);

	m_vme->install_device(vme_device::A16_SC, base + 0, base + 1, // Channel B - Data
						  read8_delegate(FUNC(z80sio_device::db_r),  subdevice<z80sio_device>("sio0")),
						  write8_delegate(FUNC(z80sio_device::db_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme->install_device(vme_device::A16_SC, base + 2, base + 3, // Channel B - Control
						  read8_delegate(FUNC(z80sio_device::cb_r),  subdevice<z80sio_device>("sio0")),
						  write8_delegate(FUNC(z80sio_device::cb_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme->install_device(vme_device::A16_SC, base + 4, base + 5, // Channel A - Data
						  read8_delegate(FUNC(z80sio_device::da_r),  subdevice<z80sio_device>("sio0")),
						  write8_delegate(FUNC(z80sio_device::da_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme->install_device(vme_device::A16_SC, base + 6, base + 7, // Channel A - Control
						  read8_delegate(FUNC(z80sio_device::ca_r),  subdevice<z80sio_device>("sio0")),
						  write8_delegate(FUNC(z80sio_device::ca_w), subdevice<z80sio_device>("sio0")), 0x00ff);
	m_vme->install_device(vme_device::A16_SC, base + 0x10, base + 0x13, // Am9513
						  read8_delegate(FUNC(am9513_device::read8),   subdevice<am9513_device>("stc")),
						  write8_delegate(FUNC(am9513_device::write8), subdevice<am9513_device>("stc")), 0x00ff);
}

void vme_mzr8300_card_device::device_reset()
{
	LOG("%s %s\n", tag(), FUNCNAME);
}

#if 0
READ8_MEMBER (vme_mzr8300_card_device::read8){
	LOG("%s()\n", FUNCNAME);
	return (uint8_t) 0;
}

WRITE8_MEMBER (vme_mzr8300_card_device::write8){
	LOG("%s()\n", FUNCNAME);
}
#endif
