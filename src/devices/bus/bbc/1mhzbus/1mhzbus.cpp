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

device_bbc_1mhzbus_interface::device_bbc_1mhzbus_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "bbc1mhzbus")
{
	m_slot = dynamic_cast<bbc_1mhzbus_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_1mhzbus_slot_device - constructor
//-------------------------------------------------

bbc_1mhzbus_slot_device::bbc_1mhzbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_1MHZBUS_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_bbc_1mhzbus_interface>(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_1mhzbus_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_1mhzbus_slot_device::fred_r(offs_t offset)
{
	if (m_card)
		return m_card->fred_r(offset);
	else
		return 0xff;
}

uint8_t bbc_1mhzbus_slot_device::jim_r(offs_t offset)
{
	if (m_card)
		return m_card->jim_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_1mhzbus_slot_device::fred_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->fred_w(offset, data);
}

void bbc_1mhzbus_slot_device::jim_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->jim_w(offset, data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( bbc_1mhzbus_devices )
//-------------------------------------------------


// slot devices
#include "autoprom.h"
//#include "beebscan.h"
//#include "teletext.h"
//#include "digitiser.h"
#include "emrmidi.h"
#include "ieee488.h"
#include "m2000.h"
//#include "m5000.h"
//#include "scsi.h"
//#include "multiform.h"
#include "opus3.h"
//#include "ramdisc.h"
//#include "graduate.h"
#include "beebsid.h"
//#include "prisma3.h"
#include "sprite.h"
#include "cfa3000opt.h"


void bbc_1mhzbus_devices(device_slot_interface &device)
{
	//device.option_add("teletext",   BBC_TELETEXT);        /* Acorn ANE01 Teletext Adapter */
	device.option_add("ieee488",    BBC_IEEE488);         /* Acorn ANK01 IEEE488 Interface */
	//device.option_add("m500",       BBC_M500);            /* Acorn ANV02 Music 500 */
	//device.option_add("awdd",       BBC_AWDD);            /* Acorn Winchester 110/130 */
	device.option_add("autoprom",   BBC_AUTOPROM);        /* ATPL AutoPrommer */
	//device.option_add("beebscan",   BBC_BEEBSCAN);        /* Beeb HandScan */
	device.option_add("b488",       BBC_B488);            /* Aries B488 */
	//device.option_add("videodig",   BBC_VIDEODIG);        /* Video Digitiser (RH Electronics) */
	device.option_add("emrmidi",    BBC_EMRMIDI);         /* EMR Midi Interface */
	//device.option_add("procyon",    BBC_PROCYON);         /* CST Procyon IEEE Interface */
	//device.option_add("twdd",       BBC_TWDD);            /* Technomatic Winchester (Akhter Host Adaptor + ABD4070 */
	//device.option_add("multiform",  BBC_MULTIFORM);       /* Technomatic Multiform Z80 */
	device.option_add("opus3",      BBC_OPUS3);           /* Opus Challenger 3 */
	//device.option_add("ramdisc",    BBC_RAMDISC);         /* Morley Electronics RAM Disc */
	//device.option_add("graduate",   BBC_GRADUATE);        /* The Torch Graduate G400/G800 */
	device.option_add("beebsid",    BBC_BEEBSID);         /* BeebSID */
	//device.option_add("prisma3",    BBC_PRISMA3);         /* PRISMA-3 - Millipede 1989 */
	device.option_add("sprite",     BBC_SPRITE);          /* Logotron Sprite Board */
}

void bbcm_1mhzbus_devices(device_slot_interface &device)
{
	//device.option_add("teletext",   BBC_TELETEXT);        /* Acorn ANE01 Teletext Adapter */
	device.option_add("ieee488",    BBC_IEEE488);         /* Acorn ANK01 IEEE488 Interface */
	//device.option_add("m500",       BBC_M500);            /* Acorn ANV02 Music 500 */
	//device.option_add("awdd",       BBC_AWDD);            /* Acorn Winchester 110/130 */
	device.option_add("b488",       BBC_B488);            /* Aries B488 */
	//device.option_add("videodig",   BBC_VIDEODIG);        /*  Video Digitiser (RH Electronics) */
	device.option_add("emrmidi",    BBC_EMRMIDI);         /* EMR Midi Interface */
	//device.option_add("procyon",    BBC_PROCYON);         /* CST Procyon IEEE Interface */
	//device.option_add("twdd",       BBC_TWDD);            /* Technomatic Winchester (Akhter Host Adaptor + ABD4070 */
	device.option_add("m2000",      BBC_M2000);           /* Hybrid Music 2000 Interface */
	//device.option_add("m3000",      BBC_M3000);           /* Hybrid Music 3000 Expander */
	//device.option_add("m5000",      BBC_M5000);           /* Hybrid Music 5000 Synthesiser */
	//device.option_add("m87",        BBC_M87);             /* Peartree Music 87 Synthesiser */
	//device.option_add("multiform",  BBC_MULTIFORM);       /* Technomatic Multiform Z80 */
	device.option_add("opusa",      BBC_OPUSA);           /* Opus Challenger ADFS */
	//device.option_add("ramdisc",    BBC_RAMDISC);         /* Morley Electronics RAM Disc */
	//device.option_add("graduate",   BBC_GRADUATE);        /* The Torch Graduate G400/G800 */
	device.option_add("beebsid",    BBC_BEEBSID);         /* BeebSID */
	//device.option_add("prisma3",    BBC_PRISMA3);         /* PRISMA-3 - Millipede 1989 */
	device.option_add("sprite",     BBC_SPRITE);          /* Logotron Sprite Board */
	device.option_add_internal("cfa3000opt", CFA3000_OPT);/* Henson CFA 3000 Option Board */
}
