// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

 NXP (Philips) LPC2103 series
 covering LPC2101, LPC2102, LPC2103*

 *currently only LPC2103

 these are based on an ARM7TDMI-S CPU
 internal flash and integrated peripherals

***************************************************************************/

#include "emu.h"
#include "lpc210x.h"

#include "arm7core.h"


DEFINE_DEVICE_TYPE(LPC2103, lpc210x_device, "lpc2103", "NXP LPC2103")

void lpc210x_device::lpc2103_map(address_map &map)
{
	map(0x00000000, 0x00007fff).rw(FUNC(lpc210x_device::flash_r), FUNC(lpc210x_device::flash_w)); // 32kb internal FLASH rom

	map(0x3FFFC000, 0x3FFFC01f).rw(FUNC(lpc210x_device::fio_r), FUNC(lpc210x_device::fio_w)); // GPIO


	map(0x40000000, 0x40001fff).ram(); // 8kb internal SROM (writes should actually latch - see docs)

	map(0xE0004000, 0xE000407f).rw(FUNC(lpc210x_device::timer0_r), FUNC(lpc210x_device::timer0_w));

	map(0xE0008000, 0xE000807f).rw(FUNC(lpc210x_device::timer1_r), FUNC(lpc210x_device::timer1_w));

	map(0xE002C000, 0xE002C007).rw(FUNC(lpc210x_device::pin_r), FUNC(lpc210x_device::pin_w));

	map(0xE01FC000, 0xE01FC007).rw(FUNC(lpc210x_device::mam_r), FUNC(lpc210x_device::mam_w));
	map(0xE01FC080, 0xE01FC08f).rw(FUNC(lpc210x_device::pll_r), FUNC(lpc210x_device::pll_w)); // phase locked loop
	map(0xE01FC100, 0xE01FC103).rw(FUNC(lpc210x_device::apbdiv_r), FUNC(lpc210x_device::apbdiv_w));
	map(0xE01FC1a0, 0xE01FC1a3).rw(FUNC(lpc210x_device::scs_r), FUNC(lpc210x_device::scs_w));

	map(0xfffff000, 0xffffffff).m(m_vic, FUNC(vic_pl190_device::map)); // interrupt controller
}


lpc210x_device::lpc210x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, LPC2103, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_LITTLE)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(lpc210x_device::lpc2103_map), this))
	, m_vic(*this, "vic")
{
}

uint32_t lpc210x_device::arm_E01FC088_r()
{
	return 0xffffffff;
}

uint32_t lpc210x_device::flash_r(offs_t offset)
{
	uint32_t ret = (m_flash[offset * 4 + 3] << 24) |
					(m_flash[offset * 4 + 2] << 16) |
					(m_flash[offset * 4 + 1] << 8) |
					(m_flash[offset * 4 + 0] << 0);
	return ret;
}

void lpc210x_device::flash_w(offs_t offset, uint32_t data)
{
	//
}


device_memory_interface::space_config_vector lpc210x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lpc210x_device::device_start()
{
	arm7_cpu_device::device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lpc210x_device::device_reset()
{
	arm7_cpu_device::device_reset();

	m_TxPR[0] = 0;
	m_TxPR[1] = 0;
}

/* PIN Select block */

uint32_t lpc210x_device::pin_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset*4)
	{
	default:
		logerror("%s unhandled read from PINSEL offset %08x mem_mask %08x\n",machine().describe_context(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


void lpc210x_device::pin_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset * 4)
	{
	default:
		logerror("%s unhandled write PINSEL offset %02x data %08x mem_mask %08x\n", machine().describe_context(), offset * 4, data, mem_mask);
	}
}

/* MAM block (memory conttroller) */

uint32_t lpc210x_device::mam_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset*4)
	{
	default:
		logerror("%s unhandled read from MAM offset %08x mem_mask %08x\n", machine().describe_context(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


void lpc210x_device::mam_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset * 4)
	{
	default:
		logerror("%s unhandled write MAM offset %02x data %08x mem_mask %08x\n", machine().describe_context(), offset * 4, data, mem_mask);
	}
}

/* FIO block */

uint32_t lpc210x_device::fio_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset*4)
	{
	default:
		logerror("%s unhandled read from FIO offset %08x mem_mask %08x\n", machine().describe_context(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


void lpc210x_device::fio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset * 4)
	{
	default:
		logerror("%s unhandled write FIO offset %02x data %08x mem_mask %08x\n", machine().describe_context(), offset * 4, data, mem_mask);
	}
}


/* APB Divider */

uint32_t lpc210x_device::apbdiv_r(offs_t offset, uint32_t mem_mask)
{
	logerror("%s unhandled read from APBDIV offset %08x mem_mask %08x\n", machine().describe_context(), offset * 4, mem_mask);
	return 0x00000000;
}


void lpc210x_device::apbdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s unhandled write APBDIV offset %02x data %08x mem_mask %08x\n", machine().describe_context(),offset * 4, data, mem_mask);
}

/* Syscon misc registers */

uint32_t lpc210x_device::scs_r(offs_t offset, uint32_t mem_mask)
{
	logerror("%s unhandled read from SCS offset %08x mem_mask %08x\n", machine().describe_context(),offset * 4, mem_mask);
	return 0x00000000;
}


void lpc210x_device::scs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s unhandled write SCS offset %02x data %08x mem_mask %08x\n", machine().describe_context(),offset * 4, data, mem_mask);
}

/* PLL Phase Locked Loop */

uint32_t lpc210x_device::pll_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset*4)
	{
	default:
		logerror("%s unhandled read from PLL offset %08x mem_mask %08x\n", machine().describe_context(),offset * 4, mem_mask);
	}

	return 0xffffffff;
}


void lpc210x_device::pll_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset * 4)
	{
	default:
		logerror("%s unhandled write PLL offset %02x data %08x mem_mask %08x\n", machine().describe_context(),offset * 4, data, mem_mask);
	}
}


/* Timers */

uint32_t lpc210x_device::read_timer(int timer, int offset, uint32_t mem_mask)
{
	switch (offset*4)
	{
	case 0x0c:
		return m_TxPR[timer];

	default:
		logerror("%s unhandled read from timer %d offset %02x mem_mask %08x\n", machine().describe_context(),timer, offset * 4, mem_mask);
	}

	return 0x00000000;
}


void lpc210x_device::write_timer(int timer, int offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset * 4)
	{
	case 0x0c:
		COMBINE_DATA(&m_TxPR[timer]);
		logerror("%s Timer %d Prescale Register set to %08x\n", machine().describe_context(),timer, m_TxPR[timer]);
		break;

	default:
		logerror("%s unhandled write timer %d offset %02x data %08x mem_mask %08x\n", machine().describe_context(),timer, offset * 4, data, mem_mask);
	}
}



void lpc210x_device::device_add_mconfig(machine_config &config)
{
	PL190_VIC(config, m_vic, 0);
	m_vic->out_irq_cb().set_inputline(*this, ARM7_IRQ_LINE);
	m_vic->out_fiq_cb().set_inputline(*this, ARM7_FIRQ_LINE);
}
