// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HAL2 Audio Controller emulation

**********************************************************************/

#include "emu.h"
#include "machine/hal2.h"

DEFINE_DEVICE_TYPE(SGI_HAL2, hal2_device, "hal2", "SGI HAL2")

hal2_device::hal2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HAL2, tag, owner, clock)
	, m_iar(0)
	, m_idr{ 0, 0, 0, 0 }
{
}

#define VERBOSE_LEVEL ( 0 )

inline void ATTR_PRINTF(3,4) hal2_device::verboselog(int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%s: %s", machine().describe_context(), buf);
	}
}

void hal2_device::device_start()
{
}

void hal2_device::device_reset()
{
	m_iar = 0;
	memset(m_idr, 0, sizeof(uint32_t) * 4);
}

READ32_MEMBER(hal2_device::read)
{
	switch( offset )
	{
	case STATUS_REG:
		//verboselog((machine, 0, "HAL2 Status read: 0x0004\n" );
		return 0x0004;
	case REVISION_REG:
		//verboselog((machine, 0, "HAL2 Revision read: 0x4011\n" );
		return 0x4011;
	}
	//verboselog((machine, 0, "Unknown HAL2 read: 0x%08x (%08x)\n", 0x1fbd8000 + offset*4, mem_mask );
	return 0;
}

WRITE32_MEMBER(hal2_device::write)
{
	switch (offset)
	{
	case STATUS_REG:
		//verboselog((machine, 0, "HAL2 Status Write: 0x%08x (%08x)\n", data, mem_mask );
		if (data & ISR_GLOBAL_RESET)
		{
			//verboselog((machine, 0, "    HAL2 Global Reset\n" );
		}
		if (data & ISR_CODEC_RESET)
		{
			//verboselog((machine, 0, "    HAL2 Codec Reset\n" );
		}
		break;
	case INDIRECT_ADDRESS_REG:
		//verboselog((machine, 0, "HAL2 Indirect Address Register Write: 0x%08x (%08x)\n", data, mem_mask );
		m_iar = data;
		switch (data & IAR_TYPE)
		{
		case 0x1000:
			//verboselog((machine, 0, "    DMA Port\n" );
			switch (data & IAR_NUM)
			{
			case 0x0100:
				//verboselog((machine, 0, "        Synth In\n" );
				break;
			case 0x0200:
				//verboselog((machine, 0, "        AES In\n" );
				break;
			case 0x0300:
				//verboselog((machine, 0, "        AES Out\n" );
				break;
			case 0x0400:
				//verboselog((machine, 0, "        DAC Out\n" );
				break;
			case 0x0500:
				//verboselog((machine, 0, "        ADC Out\n" );
				break;
			case 0x0600:
				//verboselog((machine, 0, "        Synth Control\n" );
				break;
			}
			break;
		case 0x2000:
			//verboselog((machine, 0, "    Bresenham\n" );
			switch (data & IAR_NUM)
			{
			case 0x0100:
				//verboselog((machine, 0, "        Bresenham Clock Gen 1\n" );
				break;
			case 0x0200:
				//verboselog((machine, 0, "        Bresenham Clock Gen 2\n" );
				break;
			case 0x0300:
				//verboselog((machine, 0, "        Bresenham Clock Gen 3\n" );
				break;
			}
			break;

		case 0x3000:
			//verboselog((machine, 0, "    Unix Timer\n" );
			switch (data & IAR_NUM)
			{
			case 0x0100:
				//verboselog((machine, 0, "        Unix Timer\n" );
				break;
			}
			break;

		case 0x9000:
			//verboselog((machine, 0, "    Global DMA Control\n" );
			switch (data & IAR_NUM)
			{
			case 0x0100:
				//verboselog((machine, 0, "        DMA Control\n" );
				break;
			}
			break;
		}

		switch (data & IAR_ACCESS_SEL)
		{
		case 0x0000:
			//verboselog((machine, 0, "    Write\n" );
			break;
		case 0x0080:
			//verboselog((machine, 0, "    Read\n" );
			break;
		}
		//verboselog((machine, 0, "    Parameter: %01x\n", ( data & IAR_PARAM ) >> 2 );
		return;

	case INDIRECT_DATA0_REG:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 0 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_idr[0] = data;
		return;

	case INDIRECT_DATA1_REG:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 1 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_idr[1] = data;
		return;

	case INDIRECT_DATA2_REG:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 2 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_idr[2] = data;
		return;

	case INDIRECT_DATA3_REG:
		//verboselog((machine, 0, "HAL2 Indirect Data Register 3 Write: 0x%08x (%08x)\n", data, mem_mask );
		m_idr[3] = data;
		return;
	}
	//verboselog((machine, 0, "Unknown HAL2 write: 0x%08x: 0x%08x (%08x)\n", 0x1fbd8000 + offset*4, data, mem_mask );
}
