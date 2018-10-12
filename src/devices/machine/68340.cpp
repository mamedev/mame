// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340
 * TODO: - convert all modules to devices
 */

#include "emu.h"
#include "68340.h"

DEFINE_DEVICE_TYPE(M68340, m68340_cpu_device, "mc68340", "MC68340")

int m68340_cpu_device::calc_cs(offs_t address) const
{
	if ( !(m_m68340SIM->m_ba[0] & 1) ) return 1;

	for (int i=0;i<4;i++)
	{
		if (m_m68340SIM->m_ba[i] & 1)
		{
			int mask = ((m_m68340SIM->m_am[i]&0xffffff00) | 0xff);
			int base = m_m68340SIM->m_ba[i] & 0xffffff00;
			int fcmask = (m_m68340SIM->m_am[i] & 0xf0);
			int fcbase = (m_m68340SIM->m_ba[i] & 0xf0) & ~(m_m68340SIM->m_am[i] & 0xf0);
			int fc = m_mmu_tmp_fc;

			if ((address & ~mask) == base && ((fc << 4) & ~fcmask ) == fcbase ) return i+1;
		}
	}

	return 0;
}



uint16_t m68340_cpu_device::get_cs(offs_t address)
{
	m_currentcs = calc_cs(address);

	return m_currentcs;
}



/* 68340 specifics - MOVE */

READ32_MEMBER( m68340_cpu_device::m68340_internal_base_r )
{
	logerror("%08x m68340_internal_base_r %08x, (%08x)\n", m_ppc, offset*4,mem_mask);
	return m_m68340_base;
}

WRITE32_MEMBER( m68340_cpu_device::m68340_internal_base_w )
{
	logerror("%08x m68340_internal_base_w %08x, %08x (%08x)\n", m_ppc, offset*4,data,mem_mask);

	// other conditions?
	if (m_dfc==0x7)
	{
		// unmap old modules
		if (m_m68340_base&1)
		{
			int base = m_m68340_base & 0xfffff000;

			m_internal->unmap_readwrite(base + 0x000, base + 0x05f);
			m_internal->unmap_readwrite(base + 0x600, base + 0x67f);
			m_internal->unmap_readwrite(base + 0x700, base + 0x723);
			m_internal->unmap_readwrite(base + 0x780, base + 0x7bf);

		}

		COMBINE_DATA(&m_m68340_base);
		logerror("%08x m68340_internal_base_w %08x, %08x (%08x) (m_m68340_base write)\n", pc(), offset*4,data,mem_mask);

		// map new modules
		if (m_m68340_base&1)
		{
			int base = m_m68340_base & 0xfffff000;

			m_internal->install_readwrite_handler(base + 0x000, base + 0x03f,
								read16_delegate(FUNC(m68340_cpu_device::m68340_internal_sim_r),this),
								write16_delegate(FUNC(m68340_cpu_device::m68340_internal_sim_w),this),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x010, base + 0x01f, // Intentionally punches a hole in previous address mapping
								read8_delegate(FUNC(m68340_cpu_device::m68340_internal_sim_ports_r),this),
								write8_delegate(FUNC(m68340_cpu_device::m68340_internal_sim_ports_w),this),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x040, base + 0x05f,
								read32_delegate(FUNC(m68340_cpu_device::m68340_internal_sim_cs_r),this),
								write32_delegate(FUNC(m68340_cpu_device::m68340_internal_sim_cs_w),this));
			m_internal->install_readwrite_handler(base + 0x600, base + 0x63f,
								READ16_DEVICE_DELEGATE(m_timer1, mc68340_timer_module_device, read),
								WRITE16_DEVICE_DELEGATE(m_timer1, mc68340_timer_module_device, write),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x640, base + 0x67f,
								READ16_DEVICE_DELEGATE(m_timer2, mc68340_timer_module_device, read),
								WRITE16_DEVICE_DELEGATE(m_timer2, mc68340_timer_module_device, write),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x700, base + 0x723,
								READ8_DEVICE_DELEGATE(m_serial, mc68340_serial_module_device, read),
								WRITE8_DEVICE_DELEGATE(m_serial, mc68340_serial_module_device, write),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x780, base + 0x7bf,
								read32_delegate(FUNC(m68340_cpu_device::m68340_internal_dma_r),this),
								write32_delegate(FUNC(m68340_cpu_device::m68340_internal_dma_w),this));

		}

	}
	else
	{
		logerror("%08x m68340_internal_base_w %08x, %04x (%04x) (should fall through?)\n", pc(), offset*4,data,mem_mask);
	}



}

void m68340_cpu_device::m68340_internal_map(address_map &map)
{
	map(0x0003ff00, 0x0003ff03).rw(FUNC(m68340_cpu_device::m68340_internal_base_r), FUNC(m68340_cpu_device::m68340_internal_base_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(m68340_cpu_device::device_add_mconfig)
	MCFG_DEVICE_ADD("serial", MC68340_SERIAL_MODULE, 0)
	MCFG_MC68340SER_IRQ_CALLBACK(WRITELINE("serial", mc68340_serial_module_device, irq_w))
	MCFG_DEVICE_ADD("timer1", MC68340_TIMER_MODULE, 0)
	MCFG_DEVICE_ADD("timer2", MC68340_TIMER_MODULE, 0)
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

m68340_cpu_device::m68340_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fscpu32_device(mconfig, tag, owner, clock, M68340, 32,32, address_map_constructor(FUNC(m68340_cpu_device::m68340_internal_map), this))
	, m_serial(*this, "serial")
	, m_timer1(*this, "timer1")
	, m_timer2(*this, "timer2")
	, m_clock_mode(0)
	, m_crystal(0)
	, m_extal(0)
	, m_pa_out_cb(*this)
	, m_pa_in_cb(*this)
	, m_pb_out_cb(*this)
	, m_pb_in_cb(*this)
{
	m_m68340SIM = nullptr;
	m_m68340DMA = nullptr;
	m_m68340_base = 0;
}

void m68340_cpu_device::device_reset()
{
	fscpu32_device::device_reset();
}

// Some hardwares pulls this low when resetting peripherals, most just ties this line to GND or VCC
// TODO: Support Limp mode and external clock with no PLL
WRITE_LINE_MEMBER( m68340_cpu_device::set_modck )
{
	m_modck = state;
	m_clock_mode &= ~(m68340_sim::CLOCK_MODCK | m68340_sim::CLOCK_PLL);
	m_clock_mode |= ((m_modck == ASSERT_LINE) ? (m68340_sim::CLOCK_MODCK | m68340_sim::CLOCK_PLL) : 0);
}

void m68340_cpu_device::device_start()
{
	fscpu32_device::device_start();

	m_m68340SIM    = new m68340_sim();
	m_m68340DMA    = new m68340_dma();

	m_m68340SIM->reset();
	m_m68340DMA->reset();

	start_68340_sim();

	m_m68340_base = 0x00000000;

	m_internal = &space(AS_PROGRAM);
}
