// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Micro-Professor Expansion Slot

***************************************************************************/

#include "emu.h"
#include "slot.h"

// supported devices
#include "epb.h"
#include "iom.h"
#include "prt.h"
#include "sgb.h"
#include "ssb.h"
#include "tva.h"
#include "vid.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPF1_EXP, mpf1_exp_device, "mpf_exp", "Micro-Professor Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  mpf1_exp_device - constructor
//-------------------------------------------------

mpf1_exp_device::mpf1_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MPF1_EXP, tag, owner, clock)
	, device_single_card_slot_interface<device_mpf1_exp_interface>(mconfig, *this)
	, m_program(*this, finder_base::DUMMY_TAG, -1)
	, m_io(*this, finder_base::DUMMY_TAG, -1)
	, m_card(nullptr)
	, m_int_handler(*this)
	, m_nmi_handler(*this)
	, m_wait_handler(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mpf1_exp_device::device_config_complete()
{
	// for pass-through connectors, use the parent slot's spaces
	if (dynamic_cast<device_mpf1_exp_interface *>(owner()) != nullptr)
	{
		auto parent = dynamic_cast<mpf1_exp_device *>(owner()->owner());
		if (parent != nullptr)
		{
			if (m_program.finder_tag() == finder_base::DUMMY_TAG)
				m_program.set_tag(parent->m_program, parent->m_program.spacenum());
			if (m_io.finder_tag() == finder_base::DUMMY_TAG)
				m_io.set_tag(parent->m_io, parent->m_io.spacenum());
		}
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mpf1_exp_device::device_start()
{
	m_card = get_card_device();
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_mpf1_exp_interface - constructor
//-------------------------------------------------

device_mpf1_exp_interface::device_mpf1_exp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "mpf1exp")
	, m_slot(dynamic_cast<mpf1_exp_device *>(device.owner()))
{
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

void mpf1_exp_devices(device_slot_interface &device)
{
	device.option_add("epb", MPF_EPB);
	device.option_add("epb_ibp", MPF_EPB_IBP);
	device.option_add("prt", MPF_PRT);
	device.option_add("sgb", MPF_SGB);
	device.option_add("ssb", MPF_SSB);
	//device.option_add("vid", MPF_VID);
}

void mpf1p_exp_devices(device_slot_interface &device)
{
	device.option_add("epb_ibp", MPF_EPB_IBP);
	//device.option_add("epb_ip", MPF_EPB_IP);
	device.option_add("iom_ip", MPF_IOM_IP);
	device.option_add("prt_ip", MPF_PRT_IP);
	//device.option_add("sgb_ip", MPF_SGB_IP);
	//device.option_add("ssb_ip", MPF_SSB_IP);
	device.option_add("tva", MPF_TVA_IP);
	device.option_add("vid", MPF_VID);
}

void mpf1_88_exp_devices(device_slot_interface &device)
{
	//device.option_add("epb_i88", MPF_EPB_I88);
	//device.option_add("prt_i88", MPF_PRT_I88);
}
