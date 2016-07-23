// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Tube emulation

**********************************************************************/

#include "tube.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BBC_TUBE_SLOT = &device_creator<bbc_tube_slot_device>;



//**************************************************************************
//  DEVICE BBC_TUBE PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_tube_interface - constructor
//-------------------------------------------------

device_bbc_tube_interface::device_bbc_tube_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_tube_interface - destructor
//-------------------------------------------------

device_bbc_tube_interface::~device_bbc_tube_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_slot_device - constructor
//-------------------------------------------------

bbc_tube_slot_device::bbc_tube_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, BBC_TUBE_SLOT, "BBC Micro Tube port", tag, owner, clock, "bbc_tube_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_tube_interface *>(get_card_device());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_tube_ext_devices )
//-------------------------------------------------


// slot devices
//#include "6502copro.h"
//#include "z80copro.h"
//#include "32016copro.h"
//#include "cambcopro.h"
//#include "armcopro.h"
//#include "unicopro.h"


SLOT_INTERFACE_START( bbc_tube_ext_devices )
//	SLOT_INTERFACE("6502copro",  BBC_6502_COPRO)     /* Acorn ANC01 6502 2nd processor */
//	SLOT_INTERFACE("z80copro",   BBC_Z80_COPRO)      /* Acorn ANC04 Z80 2nd processor */
//	SLOT_INTERFACE("32016copro", BBC_32016_COPRO)    /* Acorn ANC05 32016 2nd processor */
//	SLOT_INTERFACE("cambcopro",  BBC_CAMB_COPRO)     /* Acorn ANC06 Cambridge Co-Processor */
//	SLOT_INTERFACE("armcopro",   BBC_ARM_COPRO)      /* Acorn ANC13 ARM Evaluation System */
//	SLOT_INTERFACE("unicopro",   BBC_UNIVERSAL)      /* Acorn ANC21 Universal 2nd Processor Unit */
//	SLOT_INTERFACE("a500copro",  BBC_A500_COPRO)     /* Acorn A500 2nd Processor */
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_tube_int_devices )
//-------------------------------------------------


// slot devices
//#include "65c102copro.h"
//#include "80186copro.h"
//#include "arm7copro.h"


SLOT_INTERFACE_START( bbc_tube_int_devices )
//	SLOT_INTERFACE("65c102copro", BBC_65C102_COPRO)  /* Acorn ADC06 6502 co-processor */
//	SLOT_INTERFACE("80186copro",  BBC_80186_COPRO)   /* Acorn ADC08 80186 co-processor */
//	SLOT_INTERFACE("80286copro",  BBC_80286_COPRO)   /* Acorn ADC08 80286 co-processor */
//	SLOT_INTERFACE("arm7copro",   BBC_ARM7_COPRO)    /* Sprow ARM7 co-processor */
SLOT_INTERFACE_END