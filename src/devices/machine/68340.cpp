// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340
 * TODO: - convert all modules to devices
 */

#include "emu.h"
#include "68340.h"

#include <algorithm>

#define LOG_BASE (1 << 1U)
#define LOG_IPL (1 << 2U)
#define VERBOSE (LOG_BASE)
#include "logmacro.h"

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



void m68340_cpu_device::update_ipl()
{
	uint8_t new_ipl = std::max({
		pit_irq_level(),
		m_serial->irq_level(),
		m_timer[0]->irq_level(),
		m_timer[1]->irq_level()
	});
	if (m_ipl != new_ipl)
	{
		if (m_ipl != 0)
			set_input_line(m_ipl, CLEAR_LINE);
		LOGMASKED(LOG_IPL, "Changing interrupt level from %d to %d\n", m_ipl, new_ipl);
		m_ipl = new_ipl;
		if (m_ipl != 0)
			set_input_line(m_ipl, ASSERT_LINE);
	}
}

void m68340_cpu_device::internal_vectors_r(address_map &map)
{
	map(0xfffffff0, 0xffffffff).r(FUNC(m68340_cpu_device::int_ack)).umask16(0x00ff);
}


uint8_t m68340_cpu_device::int_ack(offs_t offset)
{
	uint8_t pit_iarb = pit_arbitrate(offset);
	uint8_t scu_iarb = m_serial->arbitrate(offset);
	uint8_t t1_iarb = m_timer[0]->arbitrate(offset);
	uint8_t t2_iarb = m_timer[1]->arbitrate(offset);
	uint8_t iarb = std::max({pit_iarb, scu_iarb, t1_iarb, t2_iarb});
	LOGMASKED(LOG_IPL, "Level %d interrupt arbitration: PIT = %X, SCU = %X, T1 = %X, T2 = %X\n", offset, pit_iarb, scu_iarb, t1_iarb, t2_iarb);
	int response = 0;
	uint8_t vector = 0x18; // Spurious interrupt

	// Valid IARB levels are F (high) to 1 (low) and should be unique among modules using the same interrupt level
	if (iarb != 0)
	{
		if (iarb == scu_iarb)
		{
			vector = m_serial->irq_vector();
			LOGMASKED(LOG_IPL, "SCU acknowledged interrupt with vector %02X\n", vector);
			response++;
		}

		if (iarb == t1_iarb)
		{
			vector = m_timer[0]->irq_vector();
			LOGMASKED(LOG_IPL, "T1 acknowledged interrupt with vector %02X\n", vector);
			response++;
		}

		if (iarb == t2_iarb)
		{
			vector = m_timer[1]->irq_vector();
			LOGMASKED(LOG_IPL, "T2 acknowledged interrupt with vector %02X\n", vector);
			response++;
		}

		if (iarb == pit_iarb)
		{
			vector = pit_iack();
			LOGMASKED(LOG_IPL, "PIT acknowledged interrupt with vector %02X\n", vector);
			response++;
		}
	}

	if (response == 0)
		logerror("Spurious interrupt (level %d)\n", offset);
	else if (response > 1)
		logerror("%d modules responded to interrupt (level %d, IARB = %X)\n", response, offset, iarb);

	return vector;
}


/* 68340 specifics - MOVE */

READ32_MEMBER( m68340_cpu_device::m68340_internal_base_r )
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_BASE, "%08x m68340_internal_base_r %08x, (%08x)\n", m_ppc, offset*4,mem_mask);
	return m_m68340_base;
}

WRITE32_MEMBER( m68340_cpu_device::m68340_internal_base_w )
{
	LOGMASKED(LOG_BASE, "%08x m68340_internal_base_w %08x, %08x (%08x)\n", m_ppc, offset*4,data,mem_mask);

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
		LOGMASKED(LOG_BASE, "%08x m68340_internal_base_w %08x, %08x (%08x) (m_m68340_base write)\n", pc(), offset*4,data,mem_mask);

		// map new modules
		if (m_m68340_base & 1)
		{
			int base = m_m68340_base & 0xfffff000;

			m_internal->install_readwrite_handler(base + 0x000, base + 0x03f,
					read16_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_sim_r)),
					write16_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_sim_w)),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x010, base + 0x01f, // Intentionally punches a hole in previous address mapping
					read8_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_sim_ports_r)),
					write8_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_sim_ports_w)),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x040, base + 0x05f,
					read32_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_sim_cs_r)),
					write32_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_sim_cs_w)));
			m_internal->install_readwrite_handler(base + 0x600, base + 0x63f,
					read16_delegate(*m_timer[0], FUNC(mc68340_timer_module_device::read)),
					write16_delegate(*m_timer[0], FUNC(mc68340_timer_module_device::write)),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x640, base + 0x67f,
					read16_delegate(*m_timer[1], FUNC(mc68340_timer_module_device::read)),
					write16_delegate(*m_timer[1], FUNC(mc68340_timer_module_device::write)),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x700, base + 0x723,
					read8sm_delegate(*m_serial, FUNC(mc68340_serial_module_device::read)),
					write8sm_delegate(*m_serial, FUNC(mc68340_serial_module_device::write)),0xffffffff);
			m_internal->install_readwrite_handler(base + 0x780, base + 0x7bf,
					read32_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_dma_r)),
					write32_delegate(*this, FUNC(m68340_cpu_device::m68340_internal_dma_w)));
		}
	}
	else
	{
		LOGMASKED(LOG_BASE, "%08x m68340_internal_base_w %08x, %04x (%04x) (should fall through?)\n", pc(), offset*4,data,mem_mask);
	}
}

void m68340_cpu_device::m68340_internal_map(address_map &map)
{
	map(0x0003ff00, 0x0003ff03).rw(FUNC(m68340_cpu_device::m68340_internal_base_r), FUNC(m68340_cpu_device::m68340_internal_base_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void m68340_cpu_device::device_add_mconfig(machine_config &config)
{
	MC68340_SERIAL_MODULE(config, m_serial);
	m_serial->irq_cb().set(m_serial, FUNC(mc68340_serial_module_device::irq_w));
	MC68340_TIMER_MODULE(config, m_timer[0]);
	MC68340_TIMER_MODULE(config, m_timer[1]);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

m68340_cpu_device::m68340_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fscpu32_device(mconfig, tag, owner, clock, M68340, 32,32, address_map_constructor(FUNC(m68340_cpu_device::m68340_internal_map), this))
	, m_serial(*this, "serial")
	, m_timer(*this, "timer%u", 1U)
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
	m_ipl = 0;
	m_cpu_space_config.m_internal_map = address_map_constructor(FUNC(m68340_cpu_device::internal_vectors_r), this);
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

void m68340_cpu_device::m68k_reset_peripherals()
{
	m_m68340SIM->module_reset();
	m_m68340DMA->module_reset();
	m_serial->module_reset();
	m_timer[0]->module_reset();
	m_timer[1]->module_reset();
}
