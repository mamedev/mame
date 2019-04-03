// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Simple Sound Board VP595 emulation

**********************************************************************/

#include "emu.h"
#include "vp595.h"

#include "speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1863_TAG     "u1"
#define CDP1863_XTAL    440560



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP595, vp595_device, "vp595", "VP-595 Simple Sound")


//-------------------------------------------------
//  machine_config( vp595 )
//-------------------------------------------------

void vp595_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	CDP1863(config, m_pfg, 0);
	m_pfg->set_clock2(CDP1863_XTAL);
	m_pfg->add_route(ALL_OUTPUTS, "mono", 0.25);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp595_device - constructor
//-------------------------------------------------

vp595_device::vp595_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VP595, tag, owner, clock),
	device_vip_expansion_card_interface(mconfig, *this),
	m_pfg(*this, CDP1863_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp595_device::device_start()
{
}


//-------------------------------------------------
//  vip_io_w - I/O write
//-------------------------------------------------

void vp595_device::vip_io_w(offs_t offset, uint8_t data)
{
	if (offset == 0x03)
	{
		if (!data) data = 0x80;

		m_pfg->write_str(data);
	}
}


//-------------------------------------------------
//  vip_q_w - Q write
//-------------------------------------------------

void vp595_device::vip_q_w(int state)
{
	m_pfg->oe_w(state);
}
