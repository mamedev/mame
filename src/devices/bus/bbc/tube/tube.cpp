// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Tube emulation

**********************************************************************/

#include "emu.h"
#include "tube.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_SLOT, bbc_tube_slot_device, "bbc_tube_slot", "BBC Micro Tube port")



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

bbc_tube_slot_device::bbc_tube_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_TUBE_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this)
{
}


//-------------------------------------------------
//  bbc_tube_slot_device - destructor
//-------------------------------------------------

bbc_tube_slot_device::~bbc_tube_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_tube_interface *>(get_card_device());

	// resolve callbacks
	m_irq_handler.resolve_safe();
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
//  host_r
//-------------------------------------------------

READ8_MEMBER(bbc_tube_slot_device::host_r)
{
	if (m_card)
		return m_card->host_r(space, offset);
	else
		return 0xfe;
}

//-------------------------------------------------
//  host_w
//-------------------------------------------------

WRITE8_MEMBER(bbc_tube_slot_device::host_w)
{
	if (m_card)
		m_card->host_w(space, offset, data);
}


// slot devices
#include "tube_6502.h"
#include "tube_65c102.h"
#include "tube_80186.h"
//#include "tube_80286.h"
//#include "tube_arm.h"
#include "tube_casper.h"
//#include "tube_x25.h"
#include "tube_z80.h"
//#include "tube_zep100.h"


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_extube_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( bbc_extube_devices )
	SLOT_INTERFACE("6502",   BBC_TUBE_6502)    /* Acorn ANC01 6502 2nd processor */
	SLOT_INTERFACE("z80",    BBC_TUBE_Z80)     /* Acorn ANC04 Z80 2nd processor */
//  SLOT_INTERFACE("32016", BBC_TUBE_32016)    /* Acorn ANC05 32016 2nd processor */
//  SLOT_INTERFACE("camb",  BBC_TUBE_CAMB)     /* Acorn ANC06 Cambridge Co-Processor */
//  SLOT_INTERFACE("arm",    BBC_TUBE_ARM)     /* Acorn ANC13 ARM Evaluation System */
//  SLOT_INTERFACE("80286",  BBC_TUBE_80286)   /* Acorn 80286 2nd Processor */
//  SLOT_INTERFACE("a500",  BBC_TUBE_A500)     /* Acorn A500 2nd Processor */
	SLOT_INTERFACE("casper", BBC_TUBE_CASPER)  /* Casper 68000 2nd Processor */
//  SLOT_INTERFACE("zep100", BBC_TUBE_ZEP100)  /* Torch Z80 Communicator (ZEP100) */
	/* Acorn ANC21 Universal 2nd Processor Unit */
	SLOT_INTERFACE("65c102", BBC_TUBE_65C102)  /* Acorn ADC06 65C102 co-processor */
	SLOT_INTERFACE("80186",  BBC_TUBE_80186)   /* Acorn ADC08 80186 co-processor */
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_intube_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( bbc_intube_devices )
	SLOT_INTERFACE("65c102", BBC_TUBE_65C102)  /* Acorn ADC06 65C102 co-processor */
	SLOT_INTERFACE("80186",  BBC_TUBE_80186)   /* Acorn ADC08 80186 co-processor */
//  SLOT_INTERFACE("arm7",   BBC_TUBE_ARM7)    /* Sprow ARM7 co-processor */
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_x25tube_devices )
//-------------------------------------------------

//SLOT_INTERFACE_START( bbc_x25tube_devices )
//  SLOT_INTERFACE("x25", BBC_TUBE_X25)        /* Econet X25 Gateway */
//SLOT_INTERFACE_END
