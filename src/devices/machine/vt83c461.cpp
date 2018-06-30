// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "vt83c461.h"

//#define VERBOSE 1
#include "logmacro.h"


#define VT83C461_CONFIG_UNK                1
#define VT83C461_CONFIG_REGISTER           2
#define VT83C461_CONFIG_DATA               3


DEFINE_DEVICE_TYPE(VT83C461, vt83c461_device, "vt83c461", "VIA VT83C461 IDE Controller")

vt83c461_device::vt83c461_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ide_controller_32_device(mconfig, VT83C461, tag, owner, clock),
	m_config_unknown(0),
	m_config_register_num(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vt83c461_device::device_start()
{
	ide_controller_32_device::device_start();

	/* register ide states */
	save_item(NAME(m_config_unknown));
	save_item(NAME(m_config_register));
	save_item(NAME(m_config_register_num));
}

uint32_t vt83c461_device::read_config(offs_t offset)
{
	uint32_t result = 0;

	/* logit */
	LOG("%s:IDE via config read at %X\n", machine().describe_context(), offset);

	switch(offset)
	{
	/* unknown config register */
	case VT83C461_CONFIG_UNK:
		result = m_config_unknown;
		break;

	/* active config register */
	case VT83C461_CONFIG_REGISTER:
		result = m_config_register_num;
		break;

	/* data from active config register */
	case VT83C461_CONFIG_DATA:
		if (m_config_register_num < IDE_CONFIG_REGISTERS)
			result = m_config_register[m_config_register_num];
		break;

	default:
		logerror("%s:unknown IDE via config read at %03X\n", machine().describe_context(), offset);
		break;
	}

	return result;
}

void vt83c461_device::write_config(offs_t offset, uint32_t data)
{
	/* logit */
	LOG("%s:IDE via config write to %X = %08X\n", machine().describe_context(), offset, data);

	switch (offset)
	{
	/* unknown config register */
	case VT83C461_CONFIG_UNK:
		m_config_unknown = data;
		break;

	/* active config register */
	case VT83C461_CONFIG_REGISTER:
		m_config_register_num = data;
		break;

	/* data from active config register */
	case VT83C461_CONFIG_DATA:
		if (m_config_register_num < IDE_CONFIG_REGISTERS)
			m_config_register[m_config_register_num] = data;
		break;

	default:
		logerror("%s:unknown IDE via config write at %03X = %08x\n", machine().describe_context(), offset, data);
		break;
	}
}
