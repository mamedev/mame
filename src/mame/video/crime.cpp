// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI CRIME skeleton device

**********************************************************************/

#include "emu.h"
#include "video/crime.h"

#define LOG_UNKNOWN         (1 << 0U)
#define LOG_READS           (1 << 1U)
#define LOG_WRITES          (1 << 2U)
#define LOG_ALL             (LOG_UNKNOWN | LOG_READS | LOG_WRITES)

#define VERBOSE             (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_CRIME, crime_device, "sgicrime", "SGI CRIME")

crime_device::crime_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_CRIME, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
{
}

void crime_device::device_start()
{
	save_item(NAME(m_mem_bank_ctrl));
}

void crime_device::device_reset()
{
	memset(m_mem_bank_ctrl, 0, sizeof(uint64_t) * 8);
}

//**************************************************************************
//  DEVICE HARDWARE
//**************************************************************************

void crime_device::device_add_mconfig(machine_config &config)
{
}

void crime_device::map(address_map &map)
{
	map(0x00000000, 0x000003ff).rw(FUNC(crime_device::base_r), FUNC(crime_device::base_w));
}

//**************************************************************************
//  REGISTER ACCESS
//**************************************************************************

READ64_MEMBER(crime_device::base_r)
{
	uint64_t ret = 0ULL;
	switch (offset)
	{
	case 0x000000/8:
		ret = 1ULL;
		LOGMASKED(LOG_READS, "%s: CRIME: ID Read: %08x%08x & %08x%08x\n", machine().describe_context()
			, (uint32_t)(ret >> 32), (uint32_t)ret, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
		break;
	case 0x000208/8:
	case 0x000210/8:
	case 0x000218/8:
	case 0x000220/8:
	case 0x000228/8:
	case 0x000230/8:
	case 0x000238/8:
	case 0x000240/8:
	{
		const uint32_t index = offset - 0x000208/8;
		ret = m_mem_bank_ctrl[index];
		LOGMASKED(LOG_READS, "%s: CRIME: Memory Bank %d Control Read: %08x%08x & %08x%08x\n", machine().describe_context(), index
			, (uint32_t)(ret >> 32), (uint32_t)ret, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
		break;
	}
	default:
		LOGMASKED(LOG_UNKNOWN | LOG_READS, "%s: CRIME: Unknown Read: [%08x] & %08x%08x\n", machine().describe_context()
			, 0x14000000 + offset*8, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
		break;
	}
	return ret;
}

WRITE64_MEMBER(crime_device::base_w)
{
	switch (offset)
	{
	case 0x000208/8:
	case 0x000210/8:
	case 0x000218/8:
	case 0x000220/8:
	case 0x000228/8:
	case 0x000230/8:
	case 0x000238/8:
	case 0x000240/8:
	{
		const uint32_t index = offset - 0x000208/8;
		LOGMASKED(LOG_WRITES, "%s: CRIME: Memory Bank %d Control Write: %08x%08x & %08x%08x\n", machine().describe_context(), index
			, (uint32_t)(data >> 32), (uint32_t)data, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
		m_mem_bank_ctrl[index] = data;
		break;
	}
	default:
		LOGMASKED(LOG_UNKNOWN | LOG_WRITES, "%s: CRIME: Unknown Write: %08x = %08x%08x & %08x%08x\n", machine().describe_context(), 0x14000000 + offset*8
			, (uint32_t)(data >> 32), (uint32_t)data, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
		return;
	}
}
