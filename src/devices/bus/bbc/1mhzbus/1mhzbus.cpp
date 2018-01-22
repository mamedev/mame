// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC 1MHz Bus emulation

**********************************************************************/

#include "emu.h"
#include "1mhzbus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_1MHZBUS_SLOT, bbc_1mhzbus_slot_device, "bbc_1mhzbus_slot", "BBC Micro 1MHz Bus port")



//**************************************************************************
//  DEVICE BBC_1MHZBUS PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_1mhzbus_interface - constructor
//-------------------------------------------------

device_bbc_1mhzbus_interface::device_bbc_1mhzbus_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_1mhzbus_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_1mhzbus_interface - destructor
//-------------------------------------------------

device_bbc_1mhzbus_interface::~device_bbc_1mhzbus_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_1mhzbus_slot_device - constructor
//-------------------------------------------------

bbc_1mhzbus_slot_device::bbc_1mhzbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_1MHZBUS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this)
{
}


//-------------------------------------------------
//  bbc_1mhzbus_slot_device - destructor
//-------------------------------------------------

bbc_1mhzbus_slot_device::~bbc_1mhzbus_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_1mhzbus_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_1mhzbus_interface *>(get_card_device());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_1mhzbus_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}

WRITE_LINE_MEMBER(bbc_1mhzbus_slot_device::rst_w)
{
	if (m_card)
		m_card->rst_w(state);
}

//-------------------------------------------------
//  SLOT_INTERFACE( bbc_1mhzbus_devices )
//-------------------------------------------------


// slot devices
//#include "teletext.h"
//#include "ieee488.h"
//#include "m5000.h"
//#include "multiform.h"
#include "opus3.h"
//#include "ramdisc.h"
//#include "graduate.h"
#include "beebsid.h"
//#include "prisma3.h"
#include "cfa3000opt.h"


SLOT_INTERFACE_START(bbc_1mhzbus_devices)
//  SLOT_INTERFACE("teletext",   BBC_TELETEXT)        /* Acorn ANE01 Teletext Adapter */
//  SLOT_INTERFACE("ieee488",    BBC_IEEE488)         /* Acorn ANK01 IEEE488 Interface */
//  SLOT_INTERFACE("m500",       BBC_M500)            /* Acorn ANV02 Music 500 */
//  SLOT_INTERFACE("m2000",      BBC_M2000)           /* Hybrid Music 2000 MIDI Interface */
//  SLOT_INTERFACE("m3000",      BBC_M3000)           /* Hybrid Music 3000 Expander */
//  SLOT_INTERFACE("m5000",      BBC_M5000)           /* Hybrid Music 5000 Synthesiser */
//  SLOT_INTERFACE("multiform",  BBC_MULTIFORM)       /* Technomatic Multiform Z80 */
	SLOT_INTERFACE("opus3",      BBC_OPUS3)           /* Opus Challenger 3 */
//  SLOT_INTERFACE("ramdisc",    BBC_RAMDISC)         /* Morley Electronics RAM Disc */
//  SLOT_INTERFACE("graduate",   BBC_GRADUATE)        /* The Torch Graduate G400/G800 */
	SLOT_INTERFACE("beebsid",    BBC_BEEBSID)         /* BeebSID */
//  SLOT_INTERFACE("prisma3",    BBC_PRISMA3)         /* PRISMA-3 - Millipede 1989 */
	SLOT_INTERFACE("cfa3000opt", CFA3000_OPT)         /* Henson CFA 3000 Option Board */
SLOT_INTERFACE_END
