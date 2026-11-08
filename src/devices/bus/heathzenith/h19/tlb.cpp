// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB)

    The board used in the H19 smart terminal designed and manufactured
    by Heath Company. (and the identical Z19 sold by Zenith Data Systems)
    This board is also used in Heath's H89 / H88, and ZDS's Z-89 and Z-90.

    The keyboard consists of a 9x10 matrix connected to a MM5740AAC/N
    mask-programmed keyboard controller. The output of this passes
    through a rom.

    Input can also come from the serial port.

****************************************************************************/

#include "emu.h"
#include "tlb.h"


DEFINE_DEVICE_TYPE(HEATH_TLB_CONNECTOR, heath_tlb_connector, "heath_tlb_connector", "Heath Terminal Logic board connector abstraction")

device_heath_tlb_card_interface::device_heath_tlb_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "heathtlbdevice")
	, m_slot(dynamic_cast<heath_tlb_connector *>(device.owner()))
{
}

/**
 * Terminal Logic Board Connector
 */
heath_tlb_connector::heath_tlb_connector(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HEATH_TLB_CONNECTOR, tag, owner, clock)
	, device_single_card_slot_interface(mconfig, *this)
	, m_write_sd(*this)
	, m_dtr_cb(*this)
	, m_rts_cb(*this)
	, m_reset(*this)
	, m_tlb(nullptr)
{
}

heath_tlb_connector::~heath_tlb_connector()
{
}

void heath_tlb_connector::device_start()
{
	m_tlb = get_card_device();
}
