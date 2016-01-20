// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 */

#include "68340.h"




int m68340_calc_cs(m68340cpu_device *m68k, offs_t address)
{
	m68340_sim* sim = m68k->m68340SIM;

	if ( !(sim->m_ba[0] & 1) ) return 1;

	for (int i=0;i<4;i++)
	{
		if (sim->m_ba[i] & 1)
		{
			int mask = ((sim->m_am[i]&0xffffff00) | 0xff);
			int base = sim->m_ba[i] & 0xffffff00;
			int fcmask = (sim->m_am[i] & 0xf0);
			int fcbase = (sim->m_ba[i] & 0xf0) & ~(sim->m_am[i] & 0xf0);
			int fc = m68k->mmu_tmp_fc;

			if ((address & ~mask) == base && ((fc << 4) & ~fcmask ) == fcbase ) return i+1;
		}
	}

	return 0;
}



UINT16 m68340_get_cs(m68340cpu_device *device, offs_t address)
{
	device->m68340_currentcs = m68340_calc_cs(device, address);

	return device->m68340_currentcs;
}



/* 68340 specifics - MOVE */

READ32_MEMBER( m68340cpu_device::m68340_internal_base_r )
{
	m68340cpu_device *m68k = this;
	int pc = space.device().safe_pc();
	logerror("%08x m68340_internal_base_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	return m68k->m68340_base;
}

WRITE32_MEMBER( m68340cpu_device::m68340_internal_base_w )
{
	m68340cpu_device *m68k = this;

	int pc = space.device().safe_pc();
	logerror("%08x m68340_internal_base_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);

	// other conditions?
	if (m68k->dfc==0x7)
	{
		// unmap old modules
		if (m68k->m68340_base&1)
		{
			int base = m68k->m68340_base & 0xfffff000;

			m68k->internal->unmap_readwrite(base + 0x000, base + 0x05f);
			m68k->internal->unmap_readwrite(base + 0x600, base + 0x67f);
			m68k->internal->unmap_readwrite(base + 0x700, base + 0x723);
			m68k->internal->unmap_readwrite(base + 0x780, base + 0x7bf);

		}

		COMBINE_DATA(&m68k->m68340_base);
		logerror("%08x m68340_internal_base_w %08x, %08x (%08x) (m68340_base write)\n", pc, offset*4,data,mem_mask);

		// map new modules
		if (m68k->m68340_base&1)
		{
			int base = m68k->m68340_base & 0xfffff000;

			m68k->internal->install_readwrite_handler(base + 0x000, base + 0x03f, read16_delegate(FUNC(m68340cpu_device::m68340_internal_sim_r),this),     write16_delegate(FUNC(m68340cpu_device::m68340_internal_sim_w),this),0xffffffff);
			m68k->internal->install_readwrite_handler(base + 0x010, base + 0x01f, read8_delegate(FUNC(m68340cpu_device::m68340_internal_sim_ports_r),this),write8_delegate(FUNC(m68340cpu_device::m68340_internal_sim_ports_w),this),0xffffffff);
			m68k->internal->install_readwrite_handler(base + 0x040, base + 0x05f, read32_delegate(FUNC(m68340cpu_device::m68340_internal_sim_cs_r),this),  write32_delegate(FUNC(m68340cpu_device::m68340_internal_sim_cs_w),this));
			m68k->internal->install_readwrite_handler(base + 0x600, base + 0x67f, read32_delegate(FUNC(m68340cpu_device::m68340_internal_timer_r),this),   write32_delegate(FUNC(m68340cpu_device::m68340_internal_timer_w),this));
			m68k->internal->install_readwrite_handler(base + 0x700, base + 0x723, read32_delegate(FUNC(m68340cpu_device::m68340_internal_serial_r),this),  write32_delegate(FUNC(m68340cpu_device::m68340_internal_serial_w),this));
			m68k->internal->install_readwrite_handler(base + 0x780, base + 0x7bf, read32_delegate(FUNC(m68340cpu_device::m68340_internal_dma_r),this),     write32_delegate(FUNC(m68340cpu_device::m68340_internal_dma_w),this));

		}

	}
	else
	{
		logerror("%08x m68340_internal_base_w %08x, %04x (%04x) (should fall through?)\n", pc, offset*4,data,mem_mask);
	}



}


static ADDRESS_MAP_START( m68340_internal_map, AS_PROGRAM, 32, m68340cpu_device )
	AM_RANGE(0x0003ff00, 0x0003ff03) AM_READWRITE( m68340_internal_base_r, m68340_internal_base_w)
ADDRESS_MAP_END





m68340cpu_device::m68340cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: fscpu32_device(mconfig, "MC68340", tag, owner, clock, M68340, 32,32, ADDRESS_MAP_NAME(m68340_internal_map), "mc68340", __FILE__)
{
	m68340SIM = nullptr;
	m68340DMA = nullptr;
	m68340SERIAL = nullptr;
	m68340TIMER = nullptr;
	m68340_base = 0;
}






void m68340cpu_device::device_reset()
{
	fscpu32_device::device_reset();
}


void m68340cpu_device::device_start()
{
	fscpu32_device::device_start();

	m68340SIM    = new m68340_sim();
	m68340DMA    = new m68340_dma();
	m68340SERIAL = new m68340_serial();
	m68340TIMER  = new m68340_timer();

	m68340SIM->reset();
	m68340DMA->reset();
	m68340SERIAL->reset();
	m68340TIMER->reset();

	start_68340_sim();

	m68340_base = 0x00000000;

	internal = &this->space(AS_PROGRAM);
}
