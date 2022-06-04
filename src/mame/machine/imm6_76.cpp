// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "imm6_76.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(INTEL_IMM6_76, intel_imm6_76_device, "imm6_76", "Intel imm6-76 PROM programmer")


intel_imm6_76_device::intel_imm6_76_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, INTEL_IMM6_76, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_cycle_tmr(nullptr), m_cycle_a_tmr(nullptr), m_prg_tmr(nullptr)
	, m_di(0U), m_a(0U)
	, m_do_enable(true), m_di_pos(true), m_do_pos(true)
	, m_r_w(false), m_r_w_a(false)
	, m_prgm_pwr(false)
	, m_cycle(false), m_cycle_a(false), m_prg(false)
{
	std::fill(std::begin(m_data), std::end(m_data), 0U);
}


void intel_imm6_76_device::device_start()
{
	m_cycle_tmr     = timer_alloc(FUNC(intel_imm6_76_device::cycle_expired), this);
	m_cycle_a_tmr   = timer_alloc(FUNC(intel_imm6_76_device::cycle_a_expired), this);
	m_prg_tmr       = timer_alloc(FUNC(intel_imm6_76_device::prg_expired), this);

	save_item(NAME(m_data));
	save_item(NAME(m_di));
	save_item(NAME(m_a));
	save_item(NAME(m_do_enable));
	save_item(NAME(m_di_pos));
	save_item(NAME(m_do_pos));
	save_item(NAME(m_r_w));
	save_item(NAME(m_r_w_a));
	save_item(NAME(m_prgm_pwr));
	save_item(NAME(m_cycle));
	save_item(NAME(m_cycle_a));
	save_item(NAME(m_prg));
}


image_init_result intel_imm6_76_device::call_load()
{
	if (length() != std::size(m_data))
		return image_init_result::FAIL;
	else if (fread(m_data, std::size(m_data)) != std::size(m_data))
		return image_init_result::FAIL;
	else
		return image_init_result::PASS;
}

image_init_result intel_imm6_76_device::call_create(int format_type, util::option_resolution *format_options)
{
	std::fill(std::begin(m_data), std::end(m_data), 0U);
	if (fwrite(m_data, std::size(m_data)) != std::size(m_data))
		return image_init_result::FAIL;
	else
		return image_init_result::PASS;
}

void intel_imm6_76_device::call_unload()
{
	if (!is_readonly())
	{
		fseek(0, SEEK_SET);
		fwrite(m_data, std::size(m_data));
	}
	std::fill(std::begin(m_data), std::end(m_data), 0U);
}


void intel_imm6_76_device::di_w(u8 data)
{
	if (data != m_di)
	{
		if (m_prg)
			logerror("data inputs changed during write - PROM damage likely (%02X -> %02X)\n", m_di, data);
		m_di = data;
	}
}

void intel_imm6_76_device::a_w(u8 data)
{
	if (data != m_a)
	{
		if (m_prg)
			logerror("address changed during write - PROM damage likely (%02X -> %02X)\n", m_a, data);
		m_a = data;
	}
}

u8 intel_imm6_76_device::do_r() const
{
	if (!m_do_enable)
	{
		return 0xffU;
	}
	else
	{
		u8 const data(!m_prg ? m_data[m_a] : m_di_pos ? ~m_di : m_di);
		return m_do_pos ? ~data : data;
	}
}

DECLARE_WRITE_LINE_MEMBER(intel_imm6_76_device::data_out_enable)
{
	m_do_enable = bool(state);
}

DECLARE_WRITE_LINE_MEMBER(intel_imm6_76_device::data_in_positive)
{
	if (bool(state) != m_di_pos)
	{
		if (m_prg)
			logerror("input polarity changed during write - PROM damage likely (%c -> %c)\n", m_di_pos ? '+' : '-', bool(state) ? '+' : '-');
		m_di_pos = bool(state);
	}
}

DECLARE_WRITE_LINE_MEMBER(intel_imm6_76_device::data_out_positive)
{
	m_do_pos = !bool(state);
}

WRITE_LINE_MEMBER(intel_imm6_76_device::r_w)
{
	if (!m_r_w && !bool(state) && !m_cycle)
	{
		// half 9602 with Rx = 20kΩ, Cx = 22µF, 1N914 across capacitor
		// wired to prevent re-triggering
		m_cycle_tmr->adjust(attotime::from_msec(150));
		trigger_prg();
		m_cycle = true;
	}
	m_r_w = !bool(state);
}

WRITE_LINE_MEMBER(intel_imm6_76_device::r_w_a)
{
	if (!m_r_w_a && !bool(state) && !m_cycle_a)
	{
		// half 9602 with Rx = 20kΩ, Cx = 2.2µF, 1N914 across capacitor
		// wired to prevent re-triggering
		m_cycle_a_tmr->adjust(attotime::from_msec(15));
		trigger_prg();
		m_cycle_a = true;
	}
	m_r_w_a = !bool(state);
}

WRITE_LINE_MEMBER(intel_imm6_76_device::prgm_prom_pwr)
{
	if (!bool(state) != m_prgm_pwr)
	{
		if (m_prg)
			logerror("programming power %s during write - PROM damage likely\n", bool(state) ? "disabled" : "enabled");
		m_prgm_pwr = !bool(state);
	}
}


TIMER_CALLBACK_MEMBER(intel_imm6_76_device::cycle_expired)
{
	m_cycle = false;
}

TIMER_CALLBACK_MEMBER(intel_imm6_76_device::cycle_a_expired)
{
	m_cycle_a = false;
}

TIMER_CALLBACK_MEMBER(intel_imm6_76_device::prg_expired)
{
	m_prg = false;
}


void intel_imm6_76_device::trigger_prg()
{
	if (!m_cycle && !m_cycle_a)
	{
		// half 9602 with Rx = 4.7kΩ + 20kΩ trimpot, Cx = 1.0µF
		m_prg_tmr->adjust(attotime::from_usec(3250));
		if (!m_prg)
		{
			if (m_prgm_pwr && is_loaded())
				m_data[m_a] |= m_di_pos ? ~m_di : m_di; // PROM can't be erased electronically
			m_prg = true;
		}
		else
		{
			logerror("write re-triggered - PROM damage likely\n");
		}
	}
}
