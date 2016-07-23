// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC 1MHz Bus emulation

**********************************************************************/

#include "1mhzbus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BBC_1MHZBUS_SLOT = &device_creator<bbc_1mhzbus_slot_device>;



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

bbc_1mhzbus_slot_device::bbc_1mhzbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, BBC_1MHZBUS_SLOT, "BBC Micro 1MHz Bus port", tag, owner, clock, "bbc_1mhzbus_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_1mhzbus_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_1mhzbus_interface *>(get_card_device());
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


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_1mhzbus_devices )
//-------------------------------------------------


// slot devices
//#include "teletext.h"
//#include "ieee488.h"
//#include "music500.h"
//#include "music5000.h"
//#include "opus3.h"
//#include "torchg400.h"
//#include "torchg800.h"
//#include "beebsid.h"
//#include "prisma3.h"


SLOT_INTERFACE_START( bbc_1mhzbus_devices )
//	SLOT_INTERFACE("teletext",  BBC_TELETEXT)        /* Acorn ANE01 Teletext Adapter */
//	SLOT_INTERFACE("ieee488",   BBC_IEEE488)         /* Acorn ANK01 IEEE488 Interface */
//	SLOT_INTERFACE("music500",  BBC_MUSIC500)        /* Acorn ANV02 Music500 */
//	SLOT_INTERFACE("music2000", BBC_MUSIC2000)       /* Hybrid Music 2000 MIDI Interface */
//	SLOT_INTERFACE("music3000", BBC_MUSIC3000)       /* Hybrid Music 3000 Expander */
//	SLOT_INTERFACE("music5000", BBC_MUSIC5000)       /* Hybrid Music 5000 Synthesiser */
//	SLOT_INTERFACE("opus3",     BBC_OPUS3)           /* Opus Challenger 3 */
//	SLOT_INTERFACE("torchg400", BBC_TORCHG400)       /* Torch Graduate G400 */
//	SLOT_INTERFACE("torchg800", BBC_TORCHG800)       /* Torch Graduate G800 */
//	SLOT_INTERFACE("beebsid",   BBC_BEEBSID)         /* BeebSID */
//	SLOT_INTERFACE("prisma3",   BBC_PRISMA3)         /* Prisma 3 - Millipede 1989 */
SLOT_INTERFACE_END
