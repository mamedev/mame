// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM031-B Extended Memory Board emulation

**********************************************************************/

#include "emb.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

#define OPTION_ID       0x3f

#define RAM_SIZE        0x40000

#define A19_A18_A17     ((offset >> 16) & 0x07)
#define BASE(bank)      ((m_option >> (bank * 4)) & 0x07)
#define ENABLE(bank)    BIT(m_option, (bank * 4) + 3)
#define RAM_BANK(bank)  m_ram[(bank * 0x10000) | (offset & 0xffff)]



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WANGPC_EMB = &device_creator<wangpc_emb_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_emb_device - constructor
//-------------------------------------------------

wangpc_emb_device::wangpc_emb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_EMB, "Wang PC-PM031-B", tag, owner, clock, "wangpc_emb", __FILE__),
	device_wangpcbus_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_emb_device::device_start()
{
	// allocate memory
	m_ram.allocate(RAM_SIZE);

	// state saving
	save_item(NAME(m_option));
	save_item(NAME(m_parity_error));
	save_item(NAME(m_parity_odd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_emb_device::device_reset()
{
	m_option = 0;
	m_parity_error = 0;
	m_parity_odd = 1;
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

UINT16 wangpc_emb_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	for (int bank = 0; bank < 4; bank++)
	{
		if (ENABLE(bank) && (A19_A18_A17 == BASE(bank)))
		{
			data &= RAM_BANK(bank);
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_emb_device::wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	for (int bank = 0; bank < 4; bank++)
	{
		if (ENABLE(bank) && (A19_A18_A17 == BASE(bank)))
		{
			RAM_BANK(bank) = data;
		}
	}
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

UINT16 wangpc_emb_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0xc0/2:
			data = m_option;
			break;

		case 0xfe/2:
			data = 0xfc00 | (m_parity_odd << 9) | (m_parity_error << 8) | OPTION_ID;
			break;
		}

		logerror("emb read %06x:%02x\n", offset*2, data);
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_emb_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (sad(offset))
	{
		logerror("emb write %06x:%02x\n", offset*2, data);

		switch (offset & 0x7f)
		{
		case 0xc0/2:
			m_option = data;
			break;

		case 0xce/2:
			m_parity_error = 0;
			break;

		case 0xfc/2:
			device_reset();
			break;

		case 0xfe/2:
			m_parity_odd = BIT(data, 9);
			break;
		}
	}
}
