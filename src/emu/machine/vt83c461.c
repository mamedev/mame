#include "vt83c461.h"

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE                     0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


#define VT83C461_CONFIG_UNK                1
#define VT83C461_CONFIG_REGISTER           2
#define VT83C461_CONFIG_DATA               3


const device_type VT83C461 = &device_creator<vt83c461_device>;

vt83c461_device::vt83c461_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ide_controller_device(mconfig, VT83C461, "VIA VT83C461", tag, owner, clock, "vt83c461", __FILE__),
	m_config_unknown(0),
	m_config_register_num(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vt83c461_device::device_start()
{
	ide_controller_device::device_start();

	/* register ide states */
	save_item(NAME(m_config_unknown));
	save_item(NAME(m_config_register));
	save_item(NAME(m_config_register_num));
}

READ32_MEMBER( vt83c461_device::read_config )
{
	UINT32 result = 0;

	/* logit */
	LOG(("%s:IDE via config read at %X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask));

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
		logerror("%s:unknown IDE via config read at %03X, mem_mask=%d\n", machine().describe_context(), offset, mem_mask);
		break;
	}

//  printf( "vt83c461 read config %04x %08x %04x\n", offset, result, mem_mask );
	return result;
}

WRITE32_MEMBER( vt83c461_device::write_config )
{
//  printf( "vt83c461 write config %04x %08x %04x\n", offset, data, mem_mask );

	/* logit */
	LOG(("%s:IDE via config write to %X = %08X, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask));

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
		logerror("%s:unknown IDE via config write at %03X = %08x, mem_mask=%d\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}
}

READ32_MEMBER(vt83c461_device::read_cs0)
{
	UINT32 data = 0;

	if (ACCESSING_BITS_0_15)
	{
		data = ide_controller_device::read_cs0(space, (offset * 2), mem_mask);

		if (offset == 0 && ACCESSING_BITS_16_31)
			data |= ide_controller_device::read_cs0(space, (offset * 2), mem_mask >> 16) << 16;
	}
	else if (ACCESSING_BITS_16_31)
	{
		data = ide_controller_device::read_cs0(space, (offset * 2) + 1, mem_mask >> 16) << 16;
	}

//	printf( "vt83c461 read cs0 %08x %08x %08x\n", offset, data, mem_mask );

	return data;
}

READ32_MEMBER(vt83c461_device::read_cs1)
{
	UINT32 data = 0;

	if (ACCESSING_BITS_0_15)
	{
		data = ide_controller_device::read_cs1(space, (offset * 2), mem_mask);
	}
	else if (ACCESSING_BITS_16_23)
	{
		data = ide_controller_device::read_cs1(space, (offset * 2) + 1, mem_mask >> 16) << 16;
	}

//	printf( "vt83c461 read cs1 %08x %08x %08x\n", offset, data, mem_mask );

	return data;
}

WRITE32_MEMBER(vt83c461_device::write_cs0)
{
//	printf( "vt83c461 write cs0 %08x %08x %08x\n", offset, data, mem_mask );

	if (ACCESSING_BITS_0_15)
	{
		ide_controller_device::write_cs0(space, (offset * 2), data, mem_mask);

		if (offset == 0 && ACCESSING_BITS_16_31)
			ata_interface_device::write_cs0(space, (offset * 2), data >> 16, mem_mask >> 16);
	}
	else if (ACCESSING_BITS_16_31)
	{
		ide_controller_device::write_cs0(space, (offset * 2) + 1, data >> 16, mem_mask >> 16);
	}
}

WRITE32_MEMBER(vt83c461_device::write_cs1)
{
//	printf( "vt83c461 write cs1 %08x %08x %08x\n", offset, data, mem_mask );

	if (ACCESSING_BITS_0_7)
	{
		ide_controller_device::write_cs1(space, (offset * 2), data, mem_mask);
	}
	else if (ACCESSING_BITS_16_23)
	{
		ide_controller_device::write_cs1(space, (offset * 2) + 1, data >> 16, mem_mask >> 16);
	}
}
