// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    9h0-0008_iox.cpp

    Sega Toys 9H0-0008 I/O expansion slot

*******************************************************************************/

#include "emu.h"

#include "9h0-0008_iox.h"

#include "rendutil.h"
#include "softlist_dev.h"


device_sega_9h0_0008_iox_slot_interface::device_sega_9h0_0008_iox_slot_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "sega_9h0_0008_iox")
{
	m_port = dynamic_cast<sega_9h0_0008_iox_slot_device *>(device.owner());
}

device_sega_9h0_0008_iox_slot_interface::~device_sega_9h0_0008_iox_slot_interface()
{
}


DEFINE_DEVICE_TYPE(SEGA_9H0_0008_IOX_SLOT, sega_9h0_0008_iox_slot_device, "sega_9h0_0008_iox_slot", "Sega Toys 9H0-0008 I/O expansion slot")


sega_9h0_0008_iox_slot_device::sega_9h0_0008_iox_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega_9h0_0008_iox_slot_device(mconfig, SEGA_9H0_0008_IOX_SLOT, tag, owner, clock)
{
}

sega_9h0_0008_iox_slot_device::sega_9h0_0008_iox_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_sega_9h0_0008_iox_slot_interface>(mconfig, *this)
	, m_device(nullptr)
{
}

sega_9h0_0008_iox_slot_device::~sega_9h0_0008_iox_slot_device()
{
}

void sega_9h0_0008_iox_slot_device::device_start()
{
	m_device = get_card_device();
}
