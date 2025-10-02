// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum ULA Contention helper device
**********************************************************************/

#include "emu.h"

#include "spectrum_ula.h"

DEFINE_DEVICE_TYPE(SPECTRUM_ULA_UNCONTENDED, spectrum_ula_device, "ula", "Spectrum ULA Contention :: Uncontended")

spectrum_ula_device::spectrum_ula_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}

spectrum_ula_device::spectrum_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spectrum_ula_device(mconfig, SPECTRUM_ULA_UNCONTENDED, tag, owner, clock)
{
}

void spectrum_ula_device::device_start()
{
	save_item(NAME(m_bank3_page));
}


spectrum_ula_contended_device::spectrum_ula_contended_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: spectrum_ula_device(mconfig, type, tag, owner, clock)
{
	m_is_timings_late = false;
}

INPUT_CHANGED_MEMBER(spectrum_ula_contended_device::on_contention_changed)
{
	m_is_timings_late = newval & 1;
}

void spectrum_ula_contended_device::nomem_rq(offs_t offset, u8 data)
{
	if (is_contended(offset))
		content_early();
}

void spectrum_ula_contended_device::m1(offs_t offset)
{
	m_is_m1_rd_contended = false;
	if (!machine().side_effects_disabled() && is_contended(offset))
		content_early();
}

void spectrum_ula_contended_device::io_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && is_contended(offset))
	{
		content_early();
		content_late();
	}
}

void spectrum_ula_contended_device::io_w(offs_t offset)
{
	if (is_contended(offset))
	{
		content_early();
		content_late();
	}
}

void spectrum_ula_contended_device::ula_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (is_contended(offset))
			content_early();
		content_early(1);
	}
}

void spectrum_ula_contended_device::ula_w(offs_t offset)
{
	if (is_contended(offset))
		content_early();
	content_early(1);
}

void spectrum_ula_contended_device::data_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && is_contended(offset))
		content_early();
}

void spectrum_ula_contended_device::data_w(offs_t offset)
{
	if (is_contended(offset))
		content_early();
}

void spectrum_ula_contended_device::content_early(s8 shift)
{
	const u64 vpos = m_screen->vpos();
	if (vpos < m_screen_area.top() || vpos > m_screen_area.bottom())
		return;

	const u64 now = m_maincpu->total_cycles() - m_int_at + shift;
	const u64 cf = vpos * m_video_line_clocks + get_raster_contention_offset();
	const u64 ct = cf + m_raster_line_clocks;

	if(cf <= now && now < ct)
	{
		m_is_m1_rd_contended = true; // make sure M1 sets it to false before
		const u64 clocks = now - cf;
		const u8 c = m_pattern[clocks % 8];
		m_maincpu->adjust_icount(-c);
	}
}

void spectrum_ula_contended_device::content_late()
{
	const u64 vpos = m_screen->vpos();
	if (vpos < m_screen_area.top() || vpos > m_screen_area.bottom())
		return;

	u64 now = m_maincpu->total_cycles() - m_int_at + 1;
	const u64 cf = vpos * m_video_line_clocks + get_raster_contention_offset();
	const u64 ct = cf + m_raster_line_clocks;
	for(auto i = 0x04; i; i >>= 1)
	{
		if(cf <= now && now < ct)
		{
			const u64 clocks = now - cf;
			const u8 c = m_pattern[clocks % 8];
			m_maincpu->adjust_icount(-c);
			now += c;
		}
		now++;
	}
}

void spectrum_ula_contended_device::on_irq()
{
	m_int_at = m_maincpu->total_cycles() - m_maincpu->attotime_to_cycles(m_maincpu->local_time() - machine().time());
}

bool spectrum_ula_contended_device::is_snow_possible(u16 addr)
{
	return is_contended(addr) && !m_is_m1_rd_contended && m_screen_area.contains(m_screen->hpos(), m_screen->vpos());
}

bool spectrum_ula_contended_device::is_in_contended_area()
{
	const u64 vpos = m_screen->vpos();
	return vpos >= m_screen_area.top() && vpos <= m_screen_area.bottom();
}

void spectrum_ula_contended_device::device_start()
{
	spectrum_ula_device::device_start();

	save_item(NAME(m_is_timings_late));
	save_item(NAME(m_int_at));
	save_item(NAME(m_is_m1_rd_contended));

	m_video_line_clocks = m_screen->width() * m_maincpu->clock() / m_screen->clock();
	m_raster_line_clocks = m_screen_area.width() * m_maincpu->clock() / m_screen->clock();
}

void spectrum_ula_contended_device::device_reset()
{
	m_is_m1_rd_contended = false;
}


DEFINE_DEVICE_TYPE(SPECTRUM_ULA_48K, spectrum_ula_48k_device, "ula48", "Spectrum ULA Contention :: 48K")

spectrum_ula_48k_device::spectrum_ula_48k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spectrum_ula_contended_device(mconfig, SPECTRUM_ULA_48K, tag, owner, clock)
{
	const u8 pattern[] = {6, 5, 4, 3, 2, 1, 0, 0};
	memcpy(m_pattern, pattern, 8);
	m_btime = 2;
	m_atime_left = 4;
	m_atime_right = 0;
	m_base_offset = -1;
}

bool spectrum_ula_48k_device::is_contended(offs_t offset)
{
	return offset >= 0x4000 && offset < 0x8000;
}


DEFINE_DEVICE_TYPE(SPECTRUM_ULA_128K, spectrum_ula_128k_device, "ula128", "Spectrum ULA Contention :: 128K/+2")

spectrum_ula_128k_device::spectrum_ula_128k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spectrum_ula_contended_device(mconfig, SPECTRUM_ULA_128K, tag, owner, clock)
{
	const u8 pattern[] = {6, 5, 4, 3, 2, 1, 0, 0};
	memcpy(m_pattern, pattern, 8);
	m_btime = 3;
	m_atime_left = 4;
	m_atime_right = 0;
	m_base_offset = -1; // leave it one for now, but according to Timings_Test it must be -3
}

bool spectrum_ula_128k_device::is_contended(offs_t offset)
{
	return (offset >= 0x4000 && offset < 0x8000)
		|| ((offset >= 0xc000 && offset <= 0xffff) && (m_bank3_page & 1)); // Memory pages 1,3,5 and 7 are contended
}


DEFINE_DEVICE_TYPE(SPECTRUM_ULA_PLUS2A, spectrum_ula_plus2a_device, "ulaplus2a", "Spectrum ULA Contention :: +2A/+3")

spectrum_ula_plus2a_device::spectrum_ula_plus2a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spectrum_ula_contended_device(mconfig, SPECTRUM_ULA_PLUS2A, tag, owner, clock)
{
	const u8 pattern[] = {1, 0, 7, 6, 5, 4, 3, 2};
	memcpy(m_pattern, pattern, 8);
	m_btime = 5;
	m_atime_left = 4;
	m_atime_right = 0;
	m_base_offset = 1;
}

INPUT_CHANGED_MEMBER(spectrum_ula_plus2a_device::on_contention_changed)
{
	m_is_timings_late = false;
}

bool spectrum_ula_plus2a_device::is_contended(offs_t offset)
{
	return (offset >= 0x4000 && offset < 0x8000)
		|| ((offset >= 0xc000 && offset <= 0xffff) && (m_bank3_page & 4)); // Memory banks 4, 5, 6 and 7 are contended
}
