// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2XXX LCD MCUs

    This extended SoC family combines a 65C02 CPU core (including the
    Rockwell bit opcodes) with a wide variety of on-chip peripherals.
    Features common to all besides internal RAM and ROM are parallel
    ports, internal timers, a multi-level interrupt controller, LCD
    controllers (of varying degrees of sophistication), R/C/slow XTAL
    clock generators, power management and PSG channels for speaker
    output. Each MCU also has numerous pins dedicated to LCD segment
    drivers, an external bus addressing several MB of off-chip
    memory using multiple chip select signals, or both. Program ROM,
    whether internal or external, is banked to a greater or lesser
    extent except on the smallest single-chip ST20XX models.

**********************************************************************/

#include "emu.h"
#include "st2xxx.h"
#include "r65c02d.h"

st2xxx_device::st2xxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map, int data_bits, u16 ireq_mask)
	: r65c02_device(mconfig, type, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, data_bits, 0)
	, m_in_port_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_out_port_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_pdata{0}
	, m_pctrl{0}
	, m_psel{0}
	, m_pfun{0}
	, m_pmcr(0)
	, m_ireq(0)
	, m_iena(0)
	, m_ireq_mask(ireq_mask)
	, m_sys(0)
{
	program_config.m_internal_map = std::move(internal_map);
}

device_memory_interface::space_config_vector st2xxx_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void st2xxx_device::device_resolve_objects()
{
	for (auto &cb : m_in_port_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_out_port_cb)
		cb.resolve_safe();
}

void st2xxx_device::device_reset()
{
	m6502_device::device_reset();

	// reset port registers
	std::fill(std::begin(m_pdata), std::end(m_pdata), 0xff);
	std::fill(std::begin(m_pctrl), std::end(m_pctrl), 0);
	std::fill(std::begin(m_psel), std::end(m_psel), 0xff);
	std::fill(std::begin(m_pfun), std::end(m_pfun), 0);
	for (auto &cb : m_out_port_cb)
		cb(0xff);
	m_pmcr = 0x80;

	// reset interrupt registers
	m_ireq = 0;
	m_iena = 0;
	update_irq_state();

	// reset miscellaneous registers
	m_sys = 0;

	// reset LCDC registers
	m_lssa = 0;
	m_lvpw = 0;
	m_lxmax = 0;
	m_lymax = 0;
}

std::unique_ptr<util::disasm_interface> st2xxx_device::create_disassembler()
{
	return std::make_unique<r65c02_disassembler>();
}

u8 st2xxx_device::acknowledge_irq()
{
	// IREQH interrupts have priority over IREQL interrupts
	for (int level = 8; level < 16; level++)
	{
		if (BIT(m_ireq & m_iena, level))
		{
			m_ireq &= ~(1 << level);
			update_irq_state();
			return level;
		}
	}
	for (int level = 0; level < 8; level++)
	{
		if (BIT(m_ireq & m_iena, level))
		{
			m_ireq &= ~(1 << level);
			update_irq_state();
			return level;
		}
	}
	throw emu_fatalerror("ST2XXX: no IRQ to acknowledge!\n");
}

u8 st2xxx_device::pdata_r(offs_t offset)
{
	u8 pdata = m_pdata[offset];
	u8 pinmask = ~m_pctrl[offset] | (pdata & ~m_psel[offset]);
	if (pinmask != 0)
		pdata = (pdata & ~pinmask) | (m_in_port_cb[offset](0, pinmask) & pinmask);
	return pdata;
}

void st2xxx_device::pdata_w(offs_t offset, u8 data)
{
	if (data != m_pdata[offset])
	{
		m_pdata[offset] = data;
		m_out_port_cb[offset](0, data, m_pctrl[offset]);
	}
}

u8 st2xxx_device::pctrl_r(offs_t offset)
{
	return m_pctrl[offset];
}

void st2xxx_device::pctrl_w(offs_t offset, u8 data)
{
	if (data != m_pctrl[offset])
	{
		m_pctrl[offset] = data;
		m_out_port_cb[offset](0, m_pdata[offset], data);
	}
}

u8 st2xxx_device::pfc_r()
{
	return m_pfun[0];
}

void st2xxx_device::pfc_w(u8 data)
{
	m_pfun[0] = data;
}

u8 st2xxx_device::pfd_r()
{
	return m_pfun[1];
}

void st2xxx_device::pfd_w(u8 data)
{
	m_pfun[1] = data;
}

u8 st2xxx_device::pl_r()
{
	return pdata_r(6);
}

void st2xxx_device::pl_w(u8 data)
{
	pdata_w(6, data);
}

u8 st2xxx_device::psc_r()
{
	return m_psel[2];
}

void st2xxx_device::psc_w(u8 data)
{
	m_psel[2] = data;
}

u8 st2xxx_device::pse_r()
{
	return m_psel[4];
}

void st2xxx_device::pse_w(u8 data)
{
	m_psel[4] = data;
}

u8 st2xxx_device::pcl_r()
{
	return pctrl_r(6);
}

void st2xxx_device::pcl_w(u8 data)
{
	pctrl_w(6, data);
}

u8 st2xxx_device::ireql_r()
{
	return m_ireq & 0x00ff;
}

void st2xxx_device::ireql_w(u8 data)
{
	if ((m_ireq & ~data & 0x00ff) != 0)
	{
		m_ireq &= data | 0xff00;
		update_irq_state();
	}
}

u8 st2xxx_device::ireqh_r()
{
	return m_ireq >> 8;
}

void st2xxx_device::ireqh_w(u8 data)
{
	if ((m_ireq & ~(u16(data) << 8) & 0xff00) != 0)
	{
		m_ireq &= u16(data) << 8 | 0x00ff;
		update_irq_state();
	}
}

u8 st2xxx_device::ienal_r()
{
	return m_iena & 0x00ff;
}

void st2xxx_device::ienal_w(u8 data)
{
	m_iena = (m_iena & 0xff00) | (data & m_ireq_mask);
	update_irq_state();
}

u8 st2xxx_device::ienah_r()
{
	return m_iena >> 8;
}

void st2xxx_device::ienah_w(u8 data)
{
	m_iena = (m_iena & 0x00ff) | ((u16(data) << 8) & m_ireq_mask);
	update_irq_state();
}

void st2xxx_device::lssal_w(u8 data)
{
	m_lssa = (m_lssa & 0xff00) | data;
}

void st2xxx_device::lssah_w(u8 data)
{
	m_lssa = (m_lssa & 0x00ff) | (u16(data) << 8);
}

void st2xxx_device::lvpw_w(u8 data)
{
	m_lvpw = data;
}

u8 st2xxx_device::lxmax_r()
{
	return m_lxmax;
}

void st2xxx_device::lxmax_w(u8 data)
{
	m_lxmax = data;
}

u8 st2xxx_device::lymax_r()
{
	return m_lymax;
}

void st2xxx_device::lymax_w(u8 data)
{
	m_lymax = data;
}

#include "cpu/m6502/st2xxx.hxx"
