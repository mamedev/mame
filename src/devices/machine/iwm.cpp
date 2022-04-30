// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    iwm.cpp

    Implementation of the Apple IWM floppy disk controller

*********************************************************************/

#include "emu.h"
#include "iwm.h"

#define LOG_CONTROL (1 << 1U)
#define LOG_MODE    (1 << 2U)
#define VERBOSE     0
#include "logmacro.h"

#define LOGCNTRL(...) LOGMASKED(LOG_CONTROL, __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,    __VA_ARGS__)

DEFINE_DEVICE_TYPE(IWM, iwm_device, "iwm", "Apple IWM floppy controller")

iwm_device::iwm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t q3_clock) :
	applefdintf_device(mconfig, IWM, tag, owner, clock),
	m_floppy(nullptr),
	m_q3_clock(q3_clock)
{
	m_q3_fclk_ratio = q3_clock ? double(clock)/double(q3_clock) : 0; // ~0.25
	m_fclk_q3_ratio = q3_clock ? double(q3_clock)/double(clock) : 0; // ~4
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
	save_item(NAME(m_q3_clock_active));
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
	save_item(NAME(m_rw_bit_count));
	save_item(NAME(m_devsel));
}

void iwm_device::device_reset()
{
	applefdintf_device::device_reset();

	m_last_sync = machine().time().as_ticks(clock());
	m_next_state_change = 0;
	m_active = MODE_IDLE;
	m_rw = MODE_IDLE;
	m_rw_state = S_IDLE;
	m_data = 0x00;
	m_whd = 0xbf;
	m_mode = 0x00;
	m_status = 0x00;
	m_control = 0x00;
	m_wsh = 0x00;
	m_rsh = 0x00;
	m_flux_write_start = 0;
	m_flux_write_count = 0;
	m_rw_bit_count = 0;
	m_devsel = 0;
	m_devsel_cb(0);
	m_q3_clock_active = false;
}

void iwm_device::device_timer(emu_timer &, device_timer_id, int)
{
	if(m_active == MODE_DELAY) {
		flush_write();
		m_active = MODE_IDLE;
		m_rw = MODE_IDLE;
		m_rw_state = S_IDLE;
		if(m_floppy)
			m_floppy->mon_w(true);
		m_devsel_cb(0);
		m_devsel = 0;
		m_status &= ~0x20;
		m_whd &= ~0x40;
	}
}

void iwm_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	sync();
	flush_write();

	LOG("floppy %s\n", floppy ? floppy->tag() : "-");

	if(m_floppy && (m_control & 0x10))
		m_floppy->mon_w(true);
	m_floppy = floppy;
	if(m_floppy && (m_control & 0x10))
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

void iwm_device::flush_write(u64 when)
{
	if(!m_flux_write_start)
		return;

	if(!when)
		when = m_last_sync;

	if(when > m_flux_write_start) {
		bool last_on_edge = m_flux_write_count && m_flux_write[m_flux_write_count-1] == when;
		if(last_on_edge)
			m_flux_write_count--;

		attotime start = cycles_to_time(m_flux_write_start);
		attotime end = cycles_to_time(when);
		std::vector<attotime> fluxes(m_flux_write_count);
		for(u32 i=0; i != m_flux_write_count; i++)
			fluxes[i] = cycles_to_time(m_flux_write[i]);

		if(m_floppy)
			m_floppy->write_flux(start, end, m_flux_write_count, m_flux_write_count ? &fluxes[0] : nullptr);

		m_flux_write_count = 0;
		if(last_on_edge)
			m_flux_write[m_flux_write_count++] = when;
		m_flux_write_start = when;

	} else
		m_flux_write_count = 0;
}

u8 iwm_device::control(int offset, u8 data)
{
	sync();

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

	if(m_control & 0x10) {
		if(m_active != MODE_ACTIVE) {
			m_active = MODE_ACTIVE;
			m_status |= 0x20;
			if(m_floppy)
				m_floppy->mon_w(false);
		}

		if((m_control & 0x80) == 0x00) {
			if(m_rw != MODE_READ) {
				if(m_rw == MODE_WRITE) {
					flush_write();
					write_clock_stop();
				}
				m_rw = MODE_READ;
				m_rw_state = S_IDLE;
				m_next_state_change = 0;
				m_sync_update = 0;
				m_async_update = 0;
				m_data = 0x00;
			}

		} else {
			if(m_rw != MODE_WRITE) {
				m_rw = MODE_WRITE;
				m_rw_state = S_IDLE;
				m_whd |= 0x40;
				m_next_state_change = 0;
				write_clock_start();
				if(m_floppy)
					m_floppy->set_write_splice(cycles_to_time(m_flux_write_start));
			}
		}
	} else {
		if(m_active == MODE_ACTIVE) {
			flush_write();
			if(m_mode & 0x04) {
				write_clock_stop();
				m_active = MODE_IDLE;
				m_rw = MODE_IDLE;
				m_rw_state = S_IDLE;
				m_status &= ~0x20;
				m_whd &= ~0x40;
				if(m_floppy)
					m_floppy->mon_w(true);
			} else {
				m_devsel_cb(m_control & 0x20 ? 2 : 1);
				m_active = MODE_DELAY;
				m_timer->adjust(cycles_to_time(8388608));
			}
		}
	}

	u8 devsel = m_active != MODE_IDLE ? m_control & 0x20 ? 2 : 1 : 0;
	if(devsel != m_devsel) {
		m_devsel = devsel;
		m_devsel_cb(devsel);
	}

	if(0) {
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

		LOGCNTRL("%s control %c%c %c%c %c%c%c%c (%s) [%s, %s] whd=%02x data=%02x\n",
				 machine().time().to_string(),
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
				 m_rw == MODE_IDLE ? "idle" : m_rw == MODE_READ ? "read" : "write",
				 m_whd, m_data);
	}

	if(m_active && !(m_control & 0x80) && !is_sync() && (m_data & 0x80))
		m_async_update = m_last_sync + 14;

	switch(m_control & 0xc0) {
	case 0x00: return m_active ? m_data : 0xff;
	case 0x40: return (m_status & 0x7f) | ((!m_floppy || m_floppy->wpt_r()) ? 0x80 : 0x00);
	case 0x80: return m_whd;
	case 0xc0: if(offset & 1) { if(m_active) data_w(data); else mode_w(data); } return 0xff;
	}

	abort();
}

void iwm_device::mode_w(u8 data)
{
	m_mode = data;
	m_status = (m_status & 0xe0) | (data & 0x1f);
	LOGMODE("mode %02x%s%s%s%s%s%s%s\n", m_mode,
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
	if(m_mode & 0x01)
		m_whd &= 0x7f;
}

u64 iwm_device::time_to_cycles(const attotime &tm) const
{
	return tm.as_ticks(m_q3_clock_active ? m_q3_clock : clock());
}

attotime iwm_device::cycles_to_time(u64 cycles) const
{
	return attotime::from_ticks(cycles, m_q3_clock_active ? m_q3_clock : clock());
}

bool iwm_device::is_sync() const
{
	return m_mode & 0x02 ? false : true;
}

u64 iwm_device::half_window_size() const
{
	if(m_q3_clock_active)
		return m_mode & 0x08 ? 2 : 4;

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
	if(m_q3_clock_active)
		return m_mode & 0x08 ? 4 : 8;

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

void iwm_device::write_clock_start()
{
	if(is_sync() && m_q3_clock) {
		m_q3_clock_active = true;
		m_last_sync = machine().time().as_ticks(m_q3_clock);
	}
	m_flux_write_start = m_last_sync;
	m_flux_write_count = 0;
}

void iwm_device::write_clock_stop()
{
	if(m_q3_clock_active) {
		m_q3_clock_active = false;
		m_last_sync = machine().time().as_ticks(clock());
	}
	m_flux_write_start = 0;
}

void iwm_device::sync()
{
	if(!m_active)
		return;

	u64 next_sync = machine().time().as_ticks(m_q3_clock_active ? m_q3_clock : clock());
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
					m_async_update = 0;
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
			if(!is_sync())
				m_data = 0;
			m_async_update = 0;
		}
		break;
	}

	case MODE_WRITE: {
		while(next_sync > m_last_sync) {
			if(next_sync < m_next_state_change || !(m_whd & 0x40)) {
				m_last_sync = next_sync;
				break;
			}
			if(m_last_sync < m_next_state_change)
				m_last_sync = m_next_state_change;
			switch(m_rw_state) {
			case S_IDLE:
				m_flux_write_count = 0;
				if(m_mode & 0x02) {
					m_rw_state = SW_WINDOW_LOAD;
					m_rw_bit_count = 8;
					m_next_state_change = m_last_sync + 7;
				} else {
					m_wsh = m_data;
					m_rw_state = SW_WINDOW_MIDDLE;
					m_next_state_change = m_last_sync + half_window_size();
				}
				break;

			case SW_WINDOW_LOAD:
				if(m_whd & 0x80) {
					logerror("underrun\n");
					flush_write(next_sync);
					write_clock_stop();
					m_whd &= ~0x40;
					m_last_sync = next_sync;
					m_rw_state = SW_UNDERRUN;

				} else {
					m_wsh = m_data;
					m_rw_state = SW_WINDOW_MIDDLE;
					m_whd |= 0x80;
					m_next_state_change = m_last_sync + half_window_size() - 7;
				}
				break;

			case SW_WINDOW_MIDDLE:
				if(m_wsh & 0x80)
					m_flux_write[m_flux_write_count++] = m_last_sync;
				m_wsh <<= 1;
				m_rw_state = SW_WINDOW_END;
				m_next_state_change = m_last_sync + half_window_size();
				break;

			case SW_WINDOW_END:
				if(m_flux_write_count == m_flux_write.size())
					flush_write();
				if(m_mode & 0x02) {
					m_rw_bit_count --;
					if(m_rw_bit_count == 0) {
						m_rw_state = SW_WINDOW_LOAD;
						m_rw_bit_count = 8;
						m_next_state_change = m_last_sync + 7;
					} else {
						m_rw_state = SW_WINDOW_MIDDLE;
						m_next_state_change = m_last_sync + half_window_size();
					}
				} else {
					m_next_state_change = m_last_sync + half_window_size();
					m_rw_state = SW_WINDOW_MIDDLE;
				}
				break;

			case SW_UNDERRUN:
				m_last_sync = next_sync;
				break;
			}
		}
		break;
	}
	}
}
