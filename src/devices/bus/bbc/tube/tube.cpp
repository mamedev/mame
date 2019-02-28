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
//  device_validity_check -
//-------------------------------------------------

void bbc_tube_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const carddev = get_card_device();
	if (carddev && !dynamic_cast<device_bbc_tube_interface *>(carddev))
		osd_printf_error("Card device %s (%s) does not implement device_bbc_tube_interface\n", carddev->tag(), carddev->name());
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_slot_device::device_start()
{
	device_t *const carddev = get_card_device();
	m_card = dynamic_cast<device_bbc_tube_interface *>(carddev);
	if (carddev && !m_card)
		fatalerror("Card device %s (%s) does not implement device_bbc_tube_interface\n", carddev->tag(), carddev->name());

	// resolve callbacks
	m_irq_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_slot_device::device_reset()
{
}


//-------------------------------------------------
//  host_r
//-------------------------------------------------

uint8_t bbc_tube_slot_device::host_r(offs_t offset)
{
	if (m_card)
		return m_card->host_r(offset);
	else
		return 0xfe;
}

//-------------------------------------------------
//  host_w
//-------------------------------------------------

void bbc_tube_slot_device::host_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->host_w(offset, data);
}


// slot devices
#include "tube_6502.h"
#include "tube_80186.h"
#include "tube_80286.h"
//#include "tube_a500.h"
#include "tube_arm.h"
#include "tube_casper.h"
//#include "tube_pmsb2p.h"
#include "tube_rc6502.h"
//#include "tube_x25.h"
#include "tube_z80.h"
#include "tube_zep100.h"


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_tube_devices )
//-------------------------------------------------

void bbc_tube_devices(device_slot_interface &device)
{
	device.option_add("6502",   BBC_TUBE_6502);    /* Acorn ANC01 6502 2nd processor */
	device.option_add("z80",    BBC_TUBE_Z80);     /* Acorn ANC04 Z80 2nd processor */
	//device.option_add("32016", BBC_TUBE_32016);    /* Acorn ANC05 32016 2nd processor */
	device.option_add("arm",    BBC_TUBE_ARM);     /* Acorn ANC13 ARM Evaluation System */
	device.option_add("80286",  BBC_TUBE_80286);   /* Acorn 80286 2nd Processor */
	//device.option_add("a500",   BBC_TUBE_A500);    /* Acorn A500 2nd Processor */
	device.option_add("casper", BBC_TUBE_CASPER);  /* Casper 68000 2nd Processor */
	//device.option_add("pmsb2p", BBC_TUBE_PMSB2P);  /* PMS B2P-6502 */
	//device.option_add("hdp68k", BBC_TUBE_HDP68K);  /* Torch Unicorn (HDP68K) */
	//device.option_add("x25",    BBC_TUBE_X25);     /* Econet X25 Gateway */
	device.option_add("zep100", BBC_TUBE_ZEP100);  /* Torch Z80 Communicator (ZEP100) (Torch) */
	//device.option_add("zep100l", BBC_TUBE_ZEP100L); /* Torch Z80 Communicator (ZEP100) (Acorn 8271) */
	//device.option_add("zep100w", BBC_TUBE_ZEP100W); /* Torch Z80 Communicator (ZEP100) (Acorn 1770) */
	/* Acorn ANC21 Universal 2nd Processor Unit */
	device.option_add("65c102", BBC_TUBE_65C102);  /* Acorn ADC06 65C102 co-processor */
	device.option_add("80186",  BBC_TUBE_80186);   /* Acorn ADC08 80186 co-processor */
	device.option_add("rc6502", BBC_TUBE_RC6502);   /* ReCo6502 (6502) */
	device.option_add("rc65816", BBC_TUBE_RC65816); /* ReCo6502 (65816) */
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_extube_devices )
//-------------------------------------------------

void bbc_extube_devices(device_slot_interface &device)
{
	device.option_add("6502",   BBC_TUBE_6502);     /* Acorn ANC01 6502 2nd processor */
	device.option_add("z80",    BBC_TUBE_Z80);      /* Acorn ANC04 Z80 2nd processor */
	//device.option_add("32016", BBC_TUBE_32016);   /* Acorn ANC05 32016 2nd processor */
	device.option_add("arm",    BBC_TUBE_ARM);      /* Acorn ANC13 ARM Evaluation System */
	device.option_add("80286",  BBC_TUBE_80286);    /* Acorn 80286 2nd Processor */
	//device.option_add("a500",   BBC_TUBE_A500);   /* Acorn A500 2nd Processor */
	//device.option_add("pmsb2p", BBC_TUBE_PMSB2P);   /* PMS B2P-6502 */
	/* Acorn ANC21 Universal 2nd Processor Unit */
	device.option_add("65c102", BBC_TUBE_65C102);   /* Acorn ADC06 65C102 co-processor */
	device.option_add("80186",  BBC_TUBE_80186);    /* Acorn ADC08 80186 co-processor */
	device.option_add("rc6502", BBC_TUBE_RC6502);   /* ReCo6502 (6502) */
	device.option_add("rc65816", BBC_TUBE_RC65816); /* ReCo6502 (65816) */
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_intube_devices )
//-------------------------------------------------

void bbc_intube_devices(device_slot_interface &device)
{
	device.option_add("65c102", BBC_TUBE_65C102);  /* Acorn ADC06 65C102 co-processor */
	device.option_add("80186",  BBC_TUBE_80186);   /* Acorn ADC08 80186 co-processor */
	//device.option_add("zep100m", BBC_TUBE_ZEP100M); /* Torch Z80 Communicator (ZEP100) (Master) */
	//device.option_add("arm7",    BBC_TUBE_ARM7);    /* Sprow ARM7 co-processor */
	device.option_add("rc6502",  BBC_TUBE_RC6502);  /* ReCo6502 (6502) */
	device.option_add("rc65816", BBC_TUBE_RC65816); /* ReCo6502 (65816) */
}


//-------------------------------------------------
//  SLOT_INTERFACE( electron_tube_devices )
//-------------------------------------------------

void electron_tube_devices(device_slot_interface &device)
{
	device.option_add("6502",   BBC_TUBE_6502);    /* Acorn ANC01 6502 2nd processor */
	device.option_add("z80",    BBC_TUBE_Z80);     /* Acorn ANC04 Z80 2nd processor */
	device.option_add("arm",    BBC_TUBE_ARM);     /* Acorn ANC13 ARM Evaluation System */
	device.option_add("65c102", BBC_TUBE_65C102);  /* Acorn ADC06 65C102 co-processor */
	device.option_add("80186",  BBC_TUBE_80186);   /* Acorn ADC08 80186 co-processor */
	device.option_add("rc6502", BBC_TUBE_RC6502);  /* ReCo6502 (6502) */
	device.option_add("rc65816", BBC_TUBE_RC65816); /* ReCo6502 (65816) */
}
