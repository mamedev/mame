// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    MCA card template

***************************************************************************/

#include "emu.h"
#include "template.h"


#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MCA_CARD_ID 0xffff

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_TEMPLATE, mca16_template_device, "mca_template", "MCA Card Template")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_template_device::device_add_mconfig(machine_config &config)
{

}

//-------------------------------------------------
//  mca16_template_device - constructor
//-------------------------------------------------

mca16_template_device::mca16_template_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_template_device(mconfig, MCA16_TEMPLATE, tag, owner, clock)
{
}

mca16_template_device::mca16_template_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, MCA_CARD_ID)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_template_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_template_device::device_reset()
{
}

void mca16_template_device::unmap()
{
	
}

void mca16_template_device::remap()
{

}