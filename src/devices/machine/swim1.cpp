// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM1 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim1.h"

DEFINE_DEVICE_TYPE(SWIM1, swim1_device, "swim1", "Apple SWIM1 (Sander/Wozniak Integrated Machine) version 1 floppy controller")

swim1_device::swim1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM1, tag, owner, clock)
{
}

void swim1_device::device_start()
{
	applefdintf_device::device_start();

	save_item(NAME(m_last_sync));
	save_item(NAME(m_flux_write_start));
	save_item(NAME(m_flux_write));
	save_item(NAME(m_flux_write_count));

	save_item(NAME(m_ism_mode));
	save_item(NAME(m_ism_setup));
	save_item(NAME(m_ism_param_idx));
	save_item(NAME(m_ism_param));

	save_item(NAME(m_iwm_next_state_change));
	save_item(NAME(m_iwm_sync_update));
	save_item(NAME(m_iwm_async_update));
	save_item(NAME(m_iwm_active));
	save_item(NAME(m_iwm_rw));
	save_item(NAME(m_iwm_rw_state));
	save_item(NAME(m_iwm_data));
	save_item(NAME(m_iwm_whd));
	save_item(NAME(m_iwm_mode));
	save_item(NAME(m_iwm_status));
	save_item(NAME(m_iwm_control));
	save_item(NAME(m_iwm_rsh));
	save_item(NAME(m_iwm_wsh));
	save_item(NAME(m_iwm_rw_bit_count));
	save_item(NAME(m_iwm_to_ism_counter));
}

void swim1_device::device_reset()
{
	applefdintf_device::device_reset();
	m_ism_mode = 0x00;
	m_ism_setup = 0x00;
	m_ism_param_idx = 0;
	memset(m_ism_param, 0, sizeof(m_ism_param));
	m_floppy = nullptr;

	m_last_sync = machine().time().as_ticks(clock());
	m_flux_write_start = 0;
	m_flux_write_count = 0;

	m_iwm_next_state_change = 0;
	m_iwm_active = MODE_IDLE;
	m_iwm_rw = MODE_IDLE;
	m_iwm_rw_state = S_IDLE;
	m_iwm_data = 0x00;
	m_iwm_whd = 0xbf;
	m_iwm_mode = 0x00;
	m_iwm_status = 0x00;
	m_iwm_control = 0x00;
	m_iwm_wsh = 0x00;
	m_iwm_rsh = 0x00;
	m_iwm_rw_bit_count = 0;
	m_iwm_to_ism_counter = 0;

	m_devsel_cb(0);
	m_sel35_cb(true);
	m_hdsel_cb(false);
}

void swim1_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	sync();

	m_floppy = floppy;
	update_phases();
	if(m_ism_mode & 0x40)
		m_hdsel_cb((m_ism_mode >> 5) & 1);
}

floppy_image_device *swim1_device::get_floppy() const
{
	return m_floppy;
}

void swim1_device::ism_show_mode() const
{
	logerror("ism mode%s %s hdsel=%c %c%s %c%c%s\n",
			 m_ism_mode & 0x80 ? " motoron" : "",
			 m_ism_mode & 0x40 ? "ism" : "iwm",
			 m_ism_mode & 0x20 ? '1' : '0',
			 m_ism_mode & 0x10 ? 'w' : 'r',
			 m_ism_mode & 0x08 ? " action" : "",
			 m_ism_mode & 0x04 ? 'a' : '-',
			 m_ism_mode & 0x02 ? 'b' : '-',
			 m_ism_mode & 0x01 ? " clear" : "");

}

u8 swim1_device::read(offs_t offset)
{
	if(m_ism_mode & 0x40)
		return ism_read(offset);
	else
		return iwm_control(offset, 0x00);
}

void swim1_device::write(offs_t offset, u8 data)
{
	if(m_ism_mode & 0x40)
		ism_write(offset, data);
	else
		iwm_control(offset, data);
}

u8 swim1_device::ism_read(offs_t offset)
{
	static const char *const names[] = {
		"data", "mark", "crc", "param", "phases", "setup", "status", "handshake"
	};

	logerror("read ism %s\n", names[offset & 7]);
	switch(offset & 7) {
	case 0x3: {
		u8 r = m_ism_param[m_ism_param_idx];
		m_ism_param_idx = (m_ism_param_idx + 1) & 15;
		return r;
	}

	case 0x4: // phases
		return m_phases;

	case 0x5: // setup
		return m_ism_setup;

	case 0x6: // mode
		return m_ism_mode;

	default:
		//      logerror("read %s\n", names[offset & 7]);
		break;
	}
	return 0xff;
}

void swim1_device::ism_write(offs_t offset, u8 data)
{
	static const char *const names[] = {
		"data", "mark", "crc", "param", "phases", "setup", "mode0", "mode1",
	};
	switch(offset) {
	case 0x3: {
		static const char *const pname[16] = {
			"minct", "mult", "ssl", "sss", "sll", "sls", "rpt", "csls",
			"lsl", "lss", "lll", "lls", "late", "time0", "early", "time1"
		};
		logerror("param[%s] = %02x\n", pname[m_ism_param_idx], data);
		m_ism_param[m_ism_param_idx] = data;
		m_ism_param_idx = (m_ism_param_idx + 1) & 15;
		break;
	}
	case 0x4: {
		logerror("ism phases %02x\n", data);
		m_phases = data;
		update_phases();
		break;
	}

	case 0x5:
		m_ism_setup = data;
		logerror("setup timer=%s tsm=%s %s ecm=%s %s %s 3.5=%s %s\n",
				 m_ism_setup & 0x80 ? "on" : "off",
				 m_ism_setup & 0x40 ? "off" : "on",
				 m_ism_setup & 0x20 ? "ibm" : "apple",
				 m_ism_setup & 0x10 ? "on" : "off",
				 m_ism_setup & 0x08 ? "fclk/2" : "fclk",
				 m_ism_setup & 0x04 ? "gcr" : "mfm",
				 m_ism_setup & 0x02 ? "off" : "on",
				 m_ism_setup & 0x01 ? "hdsel" : "q3");
		break;

	case 0x6:
		m_ism_mode &= ~data;
		m_ism_param_idx = 0;
		ism_show_mode();
		if(!(m_ism_mode & 0x40))
			logerror("switch to iwm\n");
		break;

	case 0x7:
		m_ism_mode |= data;
		ism_show_mode();
		break;

	default:
		logerror("write %s, %02x\n", names[offset & 7], data);
		break;
	}
}

void swim1_device::device_timer(emu_timer &, device_timer_id, int, void *)
{
	if(m_iwm_active == MODE_DELAY) {
		m_iwm_active = MODE_IDLE;
		m_iwm_status &= ~0x20;
		m_devsel_cb(0);
		m_iwm_status &= ~0x20;
		m_iwm_whd &= ~0x40;
	}
}

void swim1_device::flush_write(u64 when)
{
	if(!m_flux_write_start)
		return;

	if(!when)
		when = m_last_sync;

	if(m_floppy && when > m_flux_write_start) {
		if(m_flux_write_count && m_flux_write[m_flux_write_count-1] == when)
			m_flux_write_count--;
		attotime start = cycles_to_time(m_flux_write_start);
		attotime end = cycles_to_time(when);
		std::vector<attotime> fluxes(m_flux_write_count);
		for(u32 i=0; i != m_flux_write_count; i++)
			fluxes[i] = cycles_to_time(m_flux_write[i]);
		m_floppy->write_flux(start, end, m_flux_write_count, m_flux_write_count ? &fluxes[0] : nullptr);
	}
	m_flux_write_count = 0;
	m_flux_write_start = when;
}

u8 swim1_device::iwm_control(int offset, u8 data)
{
	sync();
	u8 prev_iwm_to_ism_counter = m_iwm_to_ism_counter;

	if(0)
		logerror("iwm control trigger %x, %02x\n", offset, data);
	u8 changed = m_iwm_control | (m_phases & 0xf);
	if(offset < 8) {
		if(offset & 1)
			m_phases |= 1 << (offset >> 1);
		else
			m_phases &= ~(1 << (offset >> 1));
		update_phases();
	} else {
		if(offset & 1)
			m_iwm_control |= 1 << (offset >> 1);
		else
			m_iwm_control &= ~(1 << (offset >> 1));
	}

	changed ^= m_iwm_control | (m_phases & 0xf);

	if(changed & 0x30)
		m_devsel_cb(m_iwm_control & 0x10 ? m_iwm_control & 0x20 ? 2 : 1 : 0);

	if(changed & 0x10) {
		if(m_iwm_control & 0x10) {
			m_iwm_active = MODE_ACTIVE;
			m_iwm_status |= 0x20;
		} else {
			if(m_iwm_mode & 0x04) {
				m_iwm_active = MODE_IDLE;
				m_iwm_status &= ~0x20;
			} else {
				m_devsel_cb(m_iwm_control & 0x20 ? 2 : 1);
				m_iwm_active = MODE_DELAY;
				m_timer->adjust(cycles_to_time(8388608));
			}
		}
	}

	if(changed & 0xd0) {
		if((m_iwm_control & 0xc0) == 0x00 && m_iwm_active) {
			if(m_iwm_rw == MODE_WRITE)
				flush_write();
			m_iwm_rw = MODE_READ;
			m_iwm_rw_state = S_IDLE;
			m_iwm_status &= ~0x20;
			m_iwm_whd &= ~0x40;
			m_iwm_next_state_change = 0;
			m_iwm_sync_update = 0;
			m_iwm_async_update = 0;
			m_iwm_data = 0x00;

		} else if((m_iwm_control & 0xc0) == 0xc0 && m_iwm_active && m_iwm_rw != MODE_WRITE) {
			m_iwm_rw = MODE_WRITE;
			m_iwm_rw_state = S_IDLE;
			m_iwm_whd |= 0x40;
			m_iwm_next_state_change = 0;
			m_flux_write_start = m_last_sync;
			m_flux_write_count = 0;
			if(m_floppy)
				m_floppy->set_write_splice(cycles_to_time(m_flux_write_start));

		} else if(m_iwm_rw == MODE_WRITE) {
			if(!(m_iwm_control & 0x80)) {
				flush_write();
				m_iwm_rw = MODE_IDLE;
			}
		} else
			m_iwm_rw = MODE_IDLE;
	}

	if(changed && 0) {
		u8 s = m_iwm_control & 0xc0;
		const char *slot = "?";
		if(s == 0x00 && !m_iwm_active)
			slot = "idle / ff";
		if(s == 0x00 && m_iwm_active)
			slot = "read / read data";
		if(s == 0x40)
			slot = "wp sense / read status";
		if(s == 0x80)
			slot = "write / read whd";
		if(s == 0xc0 && !m_iwm_active)
			slot = "mode set / write mode";
		if(s == 0xc0 && m_iwm_active)
			slot = "write load / write data";

		logerror("control %c%c %c%c %c%c%c%c (%s) [%s, %s]\n",
				 m_iwm_control & 0x80 ? '1' : '0',
				 m_iwm_control & 0x40 ? '1' : '0',
				 m_iwm_control & 0x20 ? 'b' : 'a',
				 m_iwm_control & 0x10 ? '#' : '.',
				 m_phases & 0x08 ? '#' : '.',
				 m_phases & 0x04 ? '#' : '.',
				 m_phases & 0x02 ? '#' : '.',
				 m_phases & 0x01 ? '#' : '.',
				 slot,
				 m_iwm_active == MODE_IDLE ? "idle" : m_iwm_active == MODE_DELAY ? "delay" : "active",
				 m_iwm_rw == MODE_IDLE ? "idle" : m_iwm_rw == MODE_READ ? "read" : "write");
	}

	if(m_iwm_active && !(m_iwm_control & 0xc0) && !iwm_is_sync() && (m_iwm_data & 0x80))
		m_iwm_async_update = m_last_sync + 14;

	if(offset == 0xf) {
		switch(m_iwm_to_ism_counter) {
		case 0: case 2:
			if(data & 0x40)
				m_iwm_to_ism_counter++;
			break;
		case 1:
			if(!(data & 0x40))
				m_iwm_to_ism_counter++;
			break;
		case 3:
			if(data & 0x40) {
				m_ism_mode |= 0x40;
				logerror("switch to ism\n");
			}
			break;
		}
	}
	if(m_iwm_to_ism_counter != prev_iwm_to_ism_counter+1)
		m_iwm_to_ism_counter = 0;
	else
		logerror("iwm counter = %d\n", m_iwm_to_ism_counter);

	switch(m_iwm_control & 0xc0) {
	case 0x00: return m_iwm_active ? m_iwm_data : 0xff;
	case 0x40: return (m_iwm_status & 0x7f) | (!m_floppy || m_floppy->wpt_r() ? 0x80 : 0);
	case 0x80: return m_iwm_whd;
	case 0xc0: if(offset & 1) { if(m_iwm_active) iwm_data_w(data); else iwm_mode_w(data); } return 0xff;
	}

	abort();
}

void swim1_device::ism_crc_clear()
{
	m_ism_crc = 0xcdb4;
}

void swim1_device::ism_crc_update(int bit)
{
	if((m_ism_crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
		m_ism_crc = (m_ism_crc << 1) ^ 0x1021;
	else
		m_ism_crc = m_ism_crc << 1;

}

u64 swim1_device::time_to_cycles(const attotime &tm) const
{
	return tm.as_ticks(clock());
}

attotime swim1_device::cycles_to_time(u64 cycles) const
{
	return attotime::from_ticks(cycles, clock());
}

void swim1_device::ism_fifo_clear()
{
	m_ism_fifo_pos = 0;
	ism_crc_clear();
}

bool swim1_device::ism_fifo_push(u16 data)
{
	if(m_ism_fifo_pos == 2)
		return true;
	m_ism_fifo[m_ism_fifo_pos ++] = data;
	return false;
}

u16 swim1_device::ism_fifo_pop()
{
	if(m_ism_fifo_pos == 0)
		return 0xffff;
	u16 r = m_ism_fifo[0];
	m_ism_fifo[0] = m_ism_fifo[1];
	m_ism_fifo_pos --;
	return r;
}

void swim1_device::iwm_mode_w(u8 data)
{
	m_iwm_mode = data;
	m_iwm_status = (m_iwm_status & 0xe0) | (data & 0x1f);
	logerror("mode %02x%s%s%s%s%s%s%s\n", m_iwm_mode,
			 m_iwm_mode & 0x80 ? " b7" : "",
			 m_iwm_mode & 0x40 ? " mz-reset" : "",
			 m_iwm_mode & 0x20 ? " test" : " normal",
			 m_iwm_mode & 0x10 ? " 8MHz" : " 7MHz",
			 m_iwm_mode & 0x08 ? " fast" : " slow",
			 m_iwm_mode & 0x04 ? "" : " timer",
			 m_iwm_mode & 0x02 ? " async" : " sync",
			 m_iwm_mode & 0x01 ? " latched" : "");
}

void swim1_device::iwm_data_w(u8 data)
{
	m_iwm_data = data;
	if(iwm_is_sync() && m_iwm_rw == MODE_WRITE)
		m_iwm_wsh = data;
	if(m_iwm_mode & 0x01)
		m_iwm_whd &= 0x7f;
}

bool swim1_device::iwm_is_sync() const
{
	return m_iwm_mode & 0x02 ? false : true;
}

u64 swim1_device::iwm_half_window_size() const
{
	switch(m_iwm_mode & 0x18) {
	case 0x00: return 14;
	case 0x08: return  7;
	case 0x10: return 16;
	case 0x18: return  8;
	}
	abort();
}

u64 swim1_device::iwm_window_size() const
{
	switch(m_iwm_mode & 0x18) {
	case 0x00: return 28;
	case 0x08: return 14;
	case 0x10: return 36;
	case 0x18: return 16;
	}
	abort();
}

u64 swim1_device::iwm_read_register_update_delay() const
{
	return m_iwm_mode & 0x08 ? 4 : 8;
}

void swim1_device::iwm_sync()
{
	if(!m_iwm_active)
		return;

	u64 next_sync = machine().time().as_ticks(clock());
	switch(m_iwm_rw) {
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
			if(next_sync < m_iwm_next_state_change) {
				m_last_sync = next_sync;
				break;
			}
			if(m_last_sync < m_iwm_next_state_change)
				m_last_sync = m_iwm_next_state_change;
			switch(m_iwm_rw_state) {
			case S_IDLE:
				m_iwm_rsh = 0x00;
				m_iwm_rw_state = SR_WINDOW_EDGE_0;
				m_iwm_next_state_change = m_last_sync + iwm_window_size();
				m_iwm_sync_update = 0;
				m_iwm_async_update = 0;
				break;

			case SR_WINDOW_EDGE_0:
			case SR_WINDOW_EDGE_1: {
				u64 endw = m_iwm_next_state_change + (m_iwm_rw_state == SR_WINDOW_EDGE_0 ? iwm_window_size() : iwm_half_window_size());
				if(m_iwm_rw_state == SR_WINDOW_EDGE_0 && endw >= next_flux_change && next_sync >= next_flux_change) {
					m_last_sync = m_iwm_next_state_change = next_flux_change;
					m_iwm_rw_state = SR_WINDOW_EDGE_1;
					break;
				}
				if(next_sync < endw) {
					m_last_sync = next_sync;
					break;
				}
				m_iwm_rsh = (m_iwm_rsh << 1) | (m_iwm_rw_state == SR_WINDOW_EDGE_1 ? 1 : 0);
				m_iwm_next_state_change = m_last_sync = endw;
				m_iwm_rw_state = SR_WINDOW_EDGE_0;
				if(iwm_is_sync()) {
					if(m_iwm_rsh >= 0x80) {
						m_iwm_data = m_iwm_rsh;
						m_iwm_rsh = 0;
					} else if(m_iwm_rsh >= 0x04) {
						m_iwm_data = m_iwm_rsh;
						m_iwm_sync_update = 0;
					} else if(m_iwm_rsh >= 0x02)
						m_iwm_sync_update = m_last_sync + iwm_read_register_update_delay();

				} else if(m_iwm_rsh >= 0x80) {
					m_iwm_data = m_iwm_rsh;
					m_iwm_rsh = 0;
				}
				break;
			}
			}
		}
		if(m_iwm_sync_update && m_iwm_sync_update <= m_last_sync) {
			if(iwm_is_sync())
				m_iwm_data = m_iwm_rsh;
			m_iwm_sync_update = 0;
		}
		if(m_iwm_async_update && m_iwm_async_update <= m_last_sync) {
			if(!iwm_is_sync()) {
				m_iwm_data = 0;
			}
			m_iwm_async_update = 0;
		}
		break;
	}

	case MODE_WRITE: {
		while(next_sync > m_last_sync) {
			if(next_sync < m_iwm_next_state_change) {
				m_last_sync = next_sync;
				break;
			}
			if(m_last_sync < m_iwm_next_state_change)
				m_last_sync = m_iwm_next_state_change;
			switch(m_iwm_rw_state) {
			case S_IDLE:
				m_flux_write_count = 0;
				if(m_iwm_mode & 0x02) {
					m_iwm_rw_state = SW_WINDOW_LOAD;
					m_iwm_rw_bit_count = 8;
					m_iwm_next_state_change = m_last_sync + 7;
				} else {
					m_iwm_wsh = m_iwm_data;
					m_iwm_rw_state = SW_WINDOW_MIDDLE;
					m_iwm_next_state_change = m_last_sync + iwm_half_window_size();
				}
				break;

			case SW_WINDOW_LOAD:
				if(m_iwm_whd & 0x80) {
					logerror("Underrun\n");
					m_iwm_whd &= ~0x40;
					m_iwm_rw_state = S_IDLE;
					m_last_sync = next_sync;
				} else {
					m_iwm_wsh = m_iwm_data;
					m_iwm_rw_state = SW_WINDOW_MIDDLE;
					m_iwm_whd |= 0x80;
					m_iwm_next_state_change = m_last_sync + iwm_half_window_size() - 7;
				}
				break;

			case SW_WINDOW_MIDDLE:
				if(m_iwm_wsh & 0x80)
					m_flux_write[m_flux_write_count++] = m_last_sync;
				m_iwm_wsh <<= 1;
				m_iwm_rw_state = SW_WINDOW_END;
				m_iwm_next_state_change = m_last_sync + iwm_half_window_size();
				break;

			case SW_WINDOW_END:
				if(m_flux_write_count == m_flux_write.size())
					flush_write();
				if(m_iwm_mode & 0x02) {
					m_iwm_rw_bit_count --;
					if(m_iwm_rw_bit_count == 0) {
						m_iwm_rw_state = SW_WINDOW_LOAD;
						m_iwm_rw_bit_count = 8;
						m_iwm_next_state_change = m_last_sync + 7;
					} else {
						m_iwm_rw_state = SW_WINDOW_MIDDLE;
						m_iwm_next_state_change = m_last_sync + iwm_half_window_size();
					}
				} else {
					m_iwm_next_state_change = m_last_sync + iwm_half_window_size();
					m_iwm_rw_state = SW_WINDOW_MIDDLE;
				}
				break;
			}
		}
		break;
	}
}
}

void swim1_device::ism_sync()
{
}

void swim1_device::sync()
{
	if(m_ism_mode & 0x40)
		return ism_sync();
	else
		return iwm_sync();
}
