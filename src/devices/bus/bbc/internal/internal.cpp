// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro internal expansion boards

    These boards usually add shadow RAM and/or additional ROM sockets.
    They don't have a fixed interface, some use the 6502 socket and move
    the CPU to the board, others will move other IC's to the board and
    flying leads will be connected to various points to pick up control
    lines and writes to latches.

**********************************************************************/

#include "emu.h"
#include "internal.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_INTERNAL_SLOT, bbc_internal_slot_device, "bbc_internal_slot", "BBC Micro internal boards")



//**************************************************************************
//  DEVICE BBC_INTERNAL PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_internal_interface - constructor
//-------------------------------------------------

device_bbc_internal_interface::device_bbc_internal_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcinternal")
	, m_maincpu(*this, ":maincpu")
	, m_mb_ram(*this, ":ram")
	, m_mb_rom(*this, ":romslot%u", 0U)
	, m_region_swr(*this, ":swr")
	, m_region_mos(*this, ":mos")
{
	m_slot = dynamic_cast<bbc_internal_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_internal_slot_device - constructor
//-------------------------------------------------

bbc_internal_slot_device::bbc_internal_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_INTERNAL_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_internal_interface>(mconfig, *this)
	, m_irq_handler(*this)
	, m_nmi_handler(*this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_internal_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_internal_slot_device::ram_r(offs_t offset)
{
	if (m_card)
		return m_card->ram_r(offset);
	else
		return 0xff;
}

uint8_t bbc_internal_slot_device::romsel_r(offs_t offset)
{
	if (m_card)
		return m_card->romsel_r(offset);
	else
		return 0xfe;
}

uint8_t bbc_internal_slot_device::paged_r(offs_t offset)
{
	if (m_card)
		return m_card->paged_r(offset);
	else
		return 0xff;
}

uint8_t bbc_internal_slot_device::mos_r(offs_t offset)
{
	if (m_card)
		return m_card->mos_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_internal_slot_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->ram_w(offset, data);
}

void bbc_internal_slot_device::romsel_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->romsel_w(offset, data);
}

void bbc_internal_slot_device::paged_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->paged_w(offset, data);
}

void bbc_internal_slot_device::mos_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->mos_w(offset, data);
}

void bbc_internal_slot_device::latch_fe60_w(uint8_t data)
{
	if (m_card)
		m_card->latch_fe60_w(data);
}

WRITE_LINE_MEMBER(bbc_internal_slot_device::irq6502_w)
{
	if (m_card)
		m_card->irq6502_w(state);
}

//-------------------------------------------------
//  SLOT_INTERFACE( bbc_internal_devices )
//-------------------------------------------------


// slot devices
#include "aries.h"
#include "atpl.h"
#include "cumana68k.h"
#include "integrab.h"
#include "memex.h"
#include "morleyaa.h"
#include "overlay.h"
#include "peartree.h"
#include "ramamp.h"
#include "raven20.h"
//#include "replaym.h"
#include "romex.h"
#include "stlswr.h"
#include "stl2m128.h"
#include "stl4m32.h"
//#include "stl4m256.h"
#include "werom.h"
#include "weromram.h"
#include "we32kram.h"


void bbcb_internal_devices(device_slot_interface &device)
{
	device.option_add("ariesb12", BBC_ARIESB12);        /* Aries-B12 ROM expansion */
	device.option_add("ariesb20", BBC_ARIESB20);        /* Aries-B20 RAM expansion */
	device.option_add("ariesb32", BBC_ARIESB32);        /* Aries-B32 RAM expansion */
	device.option_add("atplsw", BBC_ATPLSW);            /* ATPL Sidewise ROM/RAM expansion */
	device.option_add("cumana68k", BBC_CUMANA68K);      /* Cumana 68008 Upgrade Board */
	device.option_add("integrab", BBC_INTEGRAB);        /* Computech Integra-B */
	device.option_add("memexb20", BBC_MEMEXB20);        /* Memex-B20 RAM expansion */
	device.option_add("mr3000", BBC_MR3000);            /* Peartree MR3000 ROM board */
	device.option_add("mr4200", BBC_MR4200);            /* Peartree MR4200 RAM board */
	device.option_add("mr4300", BBC_MR4300);            /* Peartree MR4300 ROM/RAM board */
	device.option_add("mr4800", BBC_MR4800);            /* Peartree MR4800 RAM board */
	device.option_add("ramamp", BBC_RAMAMP);            /* Ramamp Sideways RAM/ROM board */
	device.option_add("raven20", BBC_RAVEN20);          /* Raven-20 RAM expansion */
	device.option_add("romex13", BBC_ROMEX13);          /* GCC Romex13 ROM expansion */
	device.option_add("swr16", BBC_STLSWR16);           /* Solidisk SWR16 */
	device.option_add("swr32", BBC_STLSWR32);           /* Solidisk SWR32 */
	device.option_add("swr64", BBC_STLSWR64);           /* Solidisk SWR64 */
	device.option_add("swr128", BBC_STLSWR128);         /* Solidisk SWR128 */
	device.option_add("2m128", BBC_STL2M128);           /* Solidisk Twomeg 128 */
	device.option_add("4m32", BBC_STL4M32);             /* Solidisk Fourmeg 32 */
	//device.option_add("4m256", BBC_STL4M256);           /* Solidisk Fourmeg 256 */
	device.option_add("we12rom", BBC_WE12ROM);          /* Watford Electronics 12 ROM board */
	device.option_add("we13rom", BBC_WE13ROM);          /* Watford Electronics 13 ROM board */
	device.option_add("weromram", BBC_WEROMRAM);        /* Watford Electronics RAM/ROM board */
	device.option_add("we32kram", BBC_WE32KRAM);        /* Watford Electronics 32K Shadow RAM */
}

void bbcbp_internal_devices(device_slot_interface &device)
{
	device.option_add("atplswp", BBC_ATPLSWP);          /* ATPL Sidewise+ ROM expansion */
	device.option_add("cumana68k", BBC_CUMANA68K);      /* Cumana 68008 Upgrade Board */
}

void bbcm_internal_devices(device_slot_interface &device)
{
	device.option_add("morleyaa", BBC_MORLEYAA);        /* Morley Master Board 'AA' */
	device.option_add("overlay", BBC_OVERLAY);          /* Vine Micros OS Overlay Board */
	//device.option_add("replaym", BBC_REPLAYM);          /* Vine Micros Master Replay */
}
