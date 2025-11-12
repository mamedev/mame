// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK parallel slot with byte addressing mod

    Combines 16 input, 16 output bits (no readback) from register 177714
    and bitbanger tx/rx bits from register 177716 into a 64-pin connector.

    Bitbanger has software support only in bk0010 ROM (EMT 40..50), and
    is not implemented.

***************************************************************************/

#include "emu.h"
#include "parallel.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BK_PARALLEL_SLOT, bk_parallel_slot_device, "bk_parallel_slot", "BK Parallel Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_parallel_slot_device - constructor
//-------------------------------------------------

bk_parallel_slot_device::bk_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_PARALLEL_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bk_parallel_interface>(mconfig, *this)
	, m_cart(nullptr)
	, m_irq2_handler(*this)
	, m_irq3_handler(*this)
{
}

//-------------------------------------------------
//  bk_parallel_slot_device - destructor
//-------------------------------------------------

bk_parallel_slot_device::~bk_parallel_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bk_parallel_slot_device::device_start()
{
	m_cart = get_card_device();
}


//**************************************************************************
//  I/O PORTS
//**************************************************************************

/*
 * Internal bus on the BK is derived from Q-Bus (active low signalling.)
 *
 * Invert data for convenience; bus drivers for the slot are not inverting.
 */
uint16_t bk_parallel_slot_device::read()
{
	if (m_cart)
		return m_cart->io_r() ^ 0xffff;
	else
		return 0;
}

void bk_parallel_slot_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_cart)
		m_cart->io_w(data ^ 0xffff, mem_mask == 0xffff);
}

void bk_parallel_slot_device::init_w()
{
	if (m_cart)
		m_cart->init_w();
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bk_parallel_interface - constructor
//-------------------------------------------------

device_bk_parallel_interface::device_bk_parallel_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bkpar")
{
	m_slot = dynamic_cast<bk_parallel_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_bk_parallel_interface - destructor
//-------------------------------------------------

device_bk_parallel_interface::~device_bk_parallel_interface()
{
}
