// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    iwm.c

    Implementation of the Apple IWM floppy disk controller

*********************************************************************/

#include "emu.h"
#include "iwm.h"

DEFINE_DEVICE_TYPE(IWM, iwm_device, "iwm", "Apple IWM floppy controller")

iwm_device::iwm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t q3_clock) :
	applefdintf_device(mconfig, IWM, tag, owner, clock),
	m_q3_clock(q3_clock)
{
	m_q3_fclk_ratio = double(clock)/double(q3_clock); // ~0.25
	m_fclk_q3_ratio = double(q3_clock)/double(clock); // ~4
}

u64 iwm_device::q3_to_fclk(u64 cycles) const
{
	return u64(m_q3_fclk_ratio * double(cycles) + 0.5);
}

u64 iwm_device::fclk_to_q3(u64 cycles) const
{
	return u64(m_fclk_q3_ratio * double(cycles) + 0.5);
}

void iwm_device::device_start()
{
	applefdintf_device::device_start();

	m_timer = timer_alloc();
	save_item(NAME(m_last_sync));
	save_item(NAME(m_next_state_change));
	save_item(NAME(m_sync_update));
	save_item(NAME(m_async_update));
	save_item(NAME(m_flux_write_start));
	save_item(NAME(m_flux_write));
	save_item(NAME(m_flux_write_count));
	save_item(NAME(m_q3_clock));
	save_item(NAME(m_active));
	save_item(NAME(m_rw));
	save_item(NAME(m_rw_state));
	save_item(NAME(m_data));
	save_item(NAME(m_whd));
	save_item(NAME(m_mode));
	save_item(NAME(m_status));
	save_item(NAME(m_control));
	save_item(NAME(m_rsh));
	save_item(NAME(m_wsh));
}

void iwm_device::device_reset()
{
	applefdintf_device::device_reset();

	m_floppy = nullptr;
	m_last_sync = machine().time().as_ticks(clock());
	m_next_state_change = 0;
	m_active = MODE_IDLE;
	m_rw = MODE_IDLE;
	m_rw_state = S_IDLE;
	m_data = 0x00;
	m_whd = 0xff;
	m_mode = 0x00;
	m_status = 0x00;
	m_control = 0x00;
	m_wsh = 0x00;
	m_rsh = 0x00;
	m_flux_write_start = 0;
	m_flux_write_count = 0;
	m_devsel_cb(1);
}

void iwm_device::device_timer(emu_timer &, device_timer_id, int, void *)
{
	if(m_active == MODE_DELAY) {
		m_active = MODE_IDLE;
		if(m_floppy)
			m_floppy->mon_w(true);
		m_status &= ~0x20;
	}
}

void iwm_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	sync();

	if(m_floppy)
		m_floppy->mon_w(true);
	m_floppy = floppy;
	if(m_active)
		m_floppy->mon_w(false);
	update_phases();
}

floppy_image_device *iwm_device::get_floppy() const
{
	return m_floppy;
}

uint8_t iwm_device::read(offs_t offset)
{
	return control(offset, 0x00);
}

void iwm_device::write(offs_t offset, u8 data)
{
	control(offset, data);
}

void iwm_device::flush_write()
{
	if(m_floppy && m_last_sync > m_flux_write_start) {
		if(m_flux_write_count && m_flux_write[m_flux_write_count-1] == m_last_sync)
			m_flux_write_count--;
		attotime start = cycles_to_time(m_flux_write_start);
		attotime end = cycles_to_time(m_last_sync);
		std::vector<attotime> fluxes(m_flux_write_count);
		for(u32 i=0; i != m_flux_write_count; i++)
			fluxes[i] = cycles_to_time(m_flux_write[i]);
		m_floppy->write_flux(start, end, m_flux_write_count, m_flux_write_count ? &fluxes[0] : nullptr);
	}
	m_flux_write_count = 0;
	m_flux_write_start = m_last_sync;
}

u8 iwm_device::control(int offset, u8 data)
{
	sync();
	logerror("control trigger %x, %02x\n", offset, data);
	u8 changed = m_control | (m_phases & 0xf);
	if(offset < 8) {
		if(offset & 1)
			m_phases |= 1 << (offset >> 1);
		else
			m_phases &= ~(1 << (offset >> 1));
		update_phases();
	} else {
		if(offset & 1)
			m_control |= 1 << (offset >> 1);
		else
			m_control &= ~(1 << (offset >> 1));
	}

	changed ^= m_control | (m_phases & 0xf);

	if(changed & 0x20)
		m_devsel_cb(m_control & 0x20 ? 2 : 1);

	if(changed & 0x10) {
		if(m_control & 0x10) {
			m_active = MODE_ACTIVE;
			if(m_floppy)
				m_floppy->mon_w(false);
			m_status |= 0x20;
		} else {
			if(m_mode & 0x04) {
				m_active = MODE_IDLE;
				if(m_floppy) {
					m_floppy->mon_w(true);
					m_floppy->seek_phase_w(0);
				}
				m_status &= ~0x20;
			} else {
				m_active = MODE_DELAY;
				m_timer->adjust(cycles_to_time(8388608));
			}
		}
	}

	if(changed & 0xd0) {
		if((m_control & 0xc0) == 0x00 && m_active) {
			if(m_rw == MODE_WRITE)
				flush_write();
			m_rw = MODE_READ;
			m_rw_state = S_IDLE;
			m_next_state_change = 0;
			m_sync_update = 0;
			m_async_update = 0;
			m_data = 0x00;

		} else if((m_control & 0xc0) == 0xc0 && (changed & 0xc0) == 0x40 && m_active && m_rw != MODE_WRITE) {
			m_rw = MODE_WRITE;
			m_rw_state = S_IDLE;
			m_next_state_change = 0;
			m_flux_write_start = m_last_sync;
			m_flux_write_count = 0;
			if(m_floppy)
				m_floppy->set_write_splice(cycles_to_time(m_flux_write_start));

		} else if(m_rw == MODE_WRITE) {
			if(!(m_control & 0x80)) {
				flush_write();
				m_rw = MODE_IDLE;
			}
		} else
			m_rw = MODE_IDLE;
	}

	if(changed || 1) {
		u8 s = m_control & 0xc0;
		const char *slot = "?";
		if(s == 0x00 && !m_active)
			slot = "idle / ff";
		if(s == 0x00 && m_active)
			slot = "read / read data";
		if(s == 0x40)
			slot = "wp sense / read status";
		if(s == 0x80)
			slot = "write / read whd";
		if(s == 0xc0 && !m_active)
			slot = "mode set / write mode";
		if(s == 0xc0 && m_active)
			slot = "write load / write data";

		logerror("control %c%c %c%c %c%c%c%c (%s) [%s, %s]\n",
				 m_control & 0x80 ? '1' : '0',
				 m_control & 0x40 ? '1' : '0',
				 m_control & 0x20 ? 'b' : 'a',
				 m_control & 0x10 ? '#' : '.',
				 m_phases & 0x08 ? '#' : '.',
				 m_phases & 0x04 ? '#' : '.',
				 m_phases & 0x02 ? '#' : '.',
				 m_phases & 0x01 ? '#' : '.',
				 slot,
				 m_active == MODE_IDLE ? "idle" : m_active == MODE_DELAY ? "delay" : "active",
				 m_rw == MODE_IDLE ? "idle" : m_rw == MODE_READ ? "read" : "write");
	}

	if(m_active && !(m_control & 0xc0) && !is_sync() && (m_data & 0x80))
		m_async_update = m_last_sync + 14;

	switch(m_control & 0xc0) {
	case 0x00: return m_active ? m_data : 0xff;
	case 0x40: return m_status;
	case 0x80: return m_whd;
	case 0xc0: if(offset & 1) { if(m_active) data_w(data); else mode_w(data); } return 0xff;
	}

	abort();
}

void iwm_device::mode_w(u8 data)
{
	m_mode = data;
	m_status = (m_status & 0xe0) | (data & 0x1f);
	logerror("mode %02x%s%s%s%s%s%s%s\n", m_mode,
			 m_mode & 0x80 ? " b7" : "",
			 m_mode & 0x40 ? " mz-reset" : "",
			 m_mode & 0x20 ? " test" : " normal",
			 m_mode & 0x10 ? " 8MHz" : " 7MHz",
			 m_mode & 0x08 ? " fast" : " slow",
			 m_mode & 0x04 ? "" : " timer",
			 m_mode & 0x02 ? " async" : " sync",
			 m_mode & 0x01 ? " latched" : "");
}

void iwm_device::data_w(u8 data)
{
	m_data = data;
	if(is_sync() && m_rw == MODE_WRITE)
		m_wsh = data;
}

u64 iwm_device::time_to_cycles(const attotime &tm) const
{
	return tm.as_ticks(clock());
}

attotime iwm_device::cycles_to_time(u64 cycles) const
{
	return attotime::from_ticks(cycles, clock());
}

bool iwm_device::is_sync() const
{
	return m_mode & 0x02 ? false : true;
}

u64 iwm_device::half_window_size() const
{
	switch(m_mode & 0x18) {
	case 0x00: return 14;
	case 0x08: return  7;
	case 0x10: return 16;
	case 0x18: return  8;
	}
	abort();
}

u64 iwm_device::window_size() const
{
	switch(m_mode & 0x18) {
	case 0x00: return 28;
	case 0x08: return 14;
	case 0x10: return 36;
	case 0x18: return 16;
	}
	abort();
}

u64 iwm_device::read_register_update_delay() const
{
	return m_mode & 0x08 ? 4 : 8;
}

u64 iwm_device::write_sync_half_window_size() const
{
	return m_mode & 0x08 ? 2 : 4;
}

void iwm_device::sync()
{
	if(!m_active)
		return;

	u64 next_sync = machine().time().as_ticks(clock());
	switch(m_rw) {
	case MODE_IDLE:
		m_last_sync = next_sync;
		break;

	case MODE_READ: {
		u64 next_flux_change = 0;
		while(next_sync > m_last_sync) {
			if(next_flux_change <= m_last_sync) {
				attotime flux = m_floppy ? m_floppy->get_next_transition(cycles_to_time(m_last_sync+1)) : attotime::never;
				next_flux_change = flux.is_never() ? u64(-1) : time_to_cycles(flux);
				if(next_flux_change <= m_last_sync)
					next_flux_change = m_last_sync+1;
			}
			if(next_sync < m_next_state_change) {
				m_last_sync = next_sync;
				break;
			}
			if(m_last_sync < m_next_state_change)
				m_last_sync = m_next_state_change;
			switch(m_rw_state) {
			case S_IDLE:
				m_rsh = 0x00;
				m_rw_state = SR_WINDOW_EDGE_0;
				m_next_state_change = m_last_sync + window_size();
				m_sync_update = 0;
				m_async_update = 0;
				break;

			case SR_WINDOW_EDGE_0:
			case SR_WINDOW_EDGE_1: {
				u64 endw = m_next_state_change + (m_rw_state == SR_WINDOW_EDGE_0 ? window_size() : half_window_size());
				if(m_rw_state == SR_WINDOW_EDGE_0 && endw >= next_flux_change && next_sync >= next_flux_change) {
					m_last_sync = m_next_state_change = next_flux_change;
					m_rw_state = SR_WINDOW_EDGE_1;
					break;
				}
				if(next_sync < endw) {
					m_last_sync = next_sync;
					break;
				}
				m_rsh = (m_rsh << 1) | (m_rw_state == SR_WINDOW_EDGE_1 ? 1 : 0);
				m_next_state_change = m_last_sync = endw;				
				m_rw_state = SR_WINDOW_EDGE_0;
				if(is_sync()) {
					if(m_rsh >= 0x80) {
						m_data = m_rsh;
						m_rsh = 0;
					} else if(m_rsh >= 0x04) {
						m_data = m_rsh;
						m_sync_update = 0;
					} else if(m_rsh >= 0x02)
						m_sync_update = m_last_sync + read_register_update_delay();

				} else if(m_rsh >= 0x80) {
					m_data = m_rsh;
					m_rsh = 0;
				}
				break;
			}
			}
		}
		if(m_sync_update && m_sync_update <= m_last_sync) {
			if(is_sync())
				m_data = m_rsh;
			m_sync_update = 0;
		}
		if(m_async_update && m_async_update <= m_last_sync) {
			if(!is_sync()) {
				m_data = 0;
			}
			m_async_update = 0;
		}
		break;
	}

	case MODE_WRITE: {
		while(next_sync > m_last_sync) {
			if(next_sync < m_next_state_change) {
				m_last_sync = next_sync;
				break;
			}
			if(m_last_sync < m_next_state_change)
				m_last_sync = m_next_state_change;
			switch(m_rw_state) {
			case S_IDLE:
				m_wsh = m_data;
				m_rw_state = SW_WINDOW_MIDDLE;
				m_next_state_change = q3_to_fclk(fclk_to_q3(m_last_sync) + write_sync_half_window_size());
				m_flux_write_count = 0;
				break;

			case SW_WINDOW_MIDDLE:
				if(m_wsh & 0x80)
					m_flux_write[m_flux_write_count++] = m_last_sync;
				m_wsh <<= 1;
				m_next_state_change = q3_to_fclk(fclk_to_q3(m_last_sync) + write_sync_half_window_size());

				m_rw_state = SW_WINDOW_END;
				break;
			case SW_WINDOW_END:
				if(m_flux_write_count == m_flux_write.size())
					flush_write();
				m_next_state_change = q3_to_fclk(fclk_to_q3(m_last_sync) + write_sync_half_window_size());
				m_rw_state = SW_WINDOW_MIDDLE;
				break;
			}
		}
		break;
	}
}
}
