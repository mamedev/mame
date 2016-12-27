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
#include "includes/mzr8300.h"
#include "machine/z80sio.h"
#include "machine/clock.h"

#define VERBOSE 0

#define LOGPRINT(...) do { if (VERBOSE) logerror(__VA_ARGS__); } while (0)
#define LOG(...)      LOGPRINT(__VA_ARGS__)

#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define LLFORMAT "%I64%"
#define FUNCNAME __func__
#else
#define LLFORMAT "%lld"
#define FUNCNAME __PRETTY_FUNCTION__
#endif

MACHINE_CONFIG_FRAGMENT( mzr8300 )
	MCFG_Z80SIO_ADD("sio0", XTAL_4MHz, 0, 0, 0, 0 )
	MCFG_Z80SIO_ADD("sio1", XTAL_4MHz, 0, 0, 0, 0 )
MACHINE_CONFIG_END

const device_type VME_P1_MZR8300 = &device_creator<vme_p1_mzr8300_device>;

machine_config_constructor vme_p1_mzr8300_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mzr8300 );
}

vme_p1_mzr8300_device::vme_p1_mzr8300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, VME_P1_MZR8300, "Mizar 8300 quad channel SIO board", tag, owner, clock, "vme_mzr8300", __FILE__)
		,device_vme_p1_card_interface(mconfig, *this)
{
}

vme_p1_mzr8300_device::vme_p1_mzr8300_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source)
		,device_vme_p1_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vme_p1_mzr8300_device::device_start()
{
	//	uint32_t slotspace;

	// set_nubus_device makes m_slot valid
	//set_vme_p1_device();

	//	slotspace = get_slotspace();

	/* Setup r/w handlers for first SIO */
	//	uint32_t base = 0xFF0000;
	//	m_vme_p1->install_device(base,     base + 3, 
	//	read8_delegate(FUNC(vme_p1_mzr8300_device::mzr8300_r), this), 
	//	write8_delegate(FUNC(vme_p1_mzr8300_device::mzr8300_w), this), 0xffffffff);
//		read8_delegate(FUNC(z80sio_device::ba_cd_r),  subdevice<z80sio_device>("sio0")), 
//		write8_delegate(FUNC(z80sio_device::ba_cd_w), subdevice<z80sio_device>("sio0")), 0xffffffff);
//	m_vme_p1->install_device(base + 3, base + 7, 
//		read8_delegate(FUNC(z80sio_device::ba_cd_r),  subdevice<z80sio_device>("sio1")), 
//		write8_delegate(FUNC(z80sio_device::ba_cd_w), subdevice<z80sio_device>("sio1")), 0xffffffff);
}

READ8_MEMBER(vme_p1_mzr8300_device::mzr8300_r)
{
	return 0;
}

WRITE8_MEMBER(vme_p1_mzr8300_device::mzr8300_w)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vme_p1_mzr8300_device::device_reset()
{
}
