// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM1 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim1.h"

DEFINE_DEVICE_TYPE(SWIM1, swim1_device, "swim1", "Apple SWIM1 (Sander/Wozniak Integrated Machine) version 1 floppy controller")

swim1_device::swim1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM1, tag, owner, clock),
	m_floppy(nullptr),
	m_timer(nullptr)
{
}

void swim1_device::device_start()
{
	applefdintf_device::device_start();

	m_timer = timer_alloc();
	save_item(NAME(m_last_sync));
	save_item(NAME(m_flux_write_start));
	save_item(NAME(m_flux_write));
	save_item(NAME(m_flux_write_count));

	save_item(NAME(m_ism_param));
	save_item(NAME(m_ism_mode));
	save_item(NAME(m_ism_setup));
	save_item(NAME(m_ism_error));
	save_item(NAME(m_ism_param_idx));
	save_item(NAME(m_ism_fifo_pos));
	save_item(NAME(m_ism_tss_sr));
	save_item(NAME(m_ism_tss_output));
	save_item(NAME(m_ism_current_bit));
	save_item(NAME(m_ism_fifo));
	save_item(NAME(m_ism_sr));
	save_item(NAME(m_ism_crc));
	save_item(NAME(m_ism_half_cycles_before_change));
	save_item(NAME(m_ism_correction_factor));
	save_item(NAME(m_ism_latest_edge));
	save_item(NAME(m_ism_prev_ls));
	save_item(NAME(m_ism_csm_state));
	save_item(NAME(m_ism_csm_error_counter));
	save_item(NAME(m_ism_csm_pair_side));
	save_item(NAME(m_ism_csm_min_count));
	save_item(NAME(m_ism_tsm_out));
	save_item(NAME(m_ism_tsm_bits));
	save_item(NAME(m_ism_tsm_mark));

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
	save_item(NAME(m_iwm_devsel));
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
	m_iwm_devsel = 0;

	m_devsel_cb(0);
	m_sel35_cb(true);
	m_hdsel_cb(false);
	m_dat1byte_cb(0);
}

void swim1_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	sync();
	flush_write();

	logerror("floppy %s\n", floppy ? floppy->tag() : "-");

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
	{
		if(!machine().side_effects_disabled())
			iwm_control(offset, 0x00);

		switch(m_iwm_control & 0xc0) {
		case 0x00: return m_iwm_active ? m_iwm_data : 0xff;
		case 0x40: return (m_iwm_status & 0x7f) | ((!m_floppy || m_floppy->wpt_r()) ? 0x80 : 0x00);
		case 0x80: return m_iwm_whd;
		case 0xc0: return 0xff;
		}

		abort();
	}
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
	ism_sync();

	//  static const char *const names[] = {
	//      "data", "mark", "crc", "param", "phases", "setup", "status", "handshake"
	//  };

	//  logerror("read ism %s\n", names[offset & 7]);
	switch(offset & 7) {
	case 0x0: { // data
		u16 r = ism_fifo_pop();
		if(!m_ism_error) {
			if(r == 0xffff)
				m_ism_error |= 4;
			else if(r & M_MARK)
				m_ism_error |= 2;
		}
		return r;
	}

	case 0x1: { // mark
		u16 r = ism_fifo_pop();
		if(!m_ism_error && r == 0xffff)
			m_ism_error |= 4;
		return r;
	}

	case 0x2: { // error
		u8 err = m_ism_error;
		m_ism_error = 0;
		return err;
	}

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

	case 0x7: { // handshake
		u8 h = 0;
		if(m_ism_fifo_pos > 0) {
			if(m_ism_fifo[m_ism_fifo_pos - 1] & M_MARK)
				h |= 0x01;
			if(!(m_ism_fifo[m_ism_fifo_pos - 1] & M_CRC0))
				h |= 0x02;
		}
		// rddata on 4
		if(!m_floppy || m_floppy->wpt_r())
			h |= 0x08;
		if(m_ism_error)
			h |= 0x20;
		if(m_ism_mode & 0x10) {
			// write
			if(m_ism_fifo_pos == 0)
				h |= 0xc0;
			else if(m_ism_fifo_pos == 1)
				h |= 0x80;
		} else {
			// read
			if(m_ism_fifo_pos == 2)
				h |= 0xc0;
			else if(m_ism_fifo_pos == 1)
				h |= 0x80;
		}
		return h;
	}

	default:
		//      logerror("read %s\n", names[offset & 7]);
		break;
	}
	return 0xff;
}

void swim1_device::ism_write(offs_t offset, u8 data)
{
	ism_sync();

	u8 prev_mode = m_ism_mode;

	static const char *const names[] = {
		"data", "mark", "crc", "param", "phases", "setup", "mode0", "mode1",
	};
	switch(offset) {
	case 0:
		if(ism_fifo_push(data) && !m_ism_error)
			m_ism_error |= 0x04;
		break;

	case 1:
		if(ism_fifo_push(M_MARK | data) && !m_ism_error)
			m_ism_error |= 0x04;
		break;

	case 2:
		if(ism_fifo_push(M_CRC) && !m_ism_error)
			m_ism_error |= 0x04;
		break;

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
		if(data & 0x10)
			m_dat1byte_cb((m_ism_fifo_pos != 0) ? 1 : 0);
		if(!(m_ism_mode & 0x40)) {
			logerror("switch to iwm\n");
			u8 ism_devsel = m_ism_mode & 0x80 ? (m_ism_mode >> 1) & 3 : 0;
			if(ism_devsel != m_iwm_devsel)
				m_devsel_cb(m_iwm_devsel);
		}
		break;

	case 0x7:
		m_ism_mode |= data;
		ism_show_mode();
		if(data & 0x10)
			m_dat1byte_cb((m_ism_fifo_pos != 2) ? 1 : 0);
		break;

	default:
		logerror("write %s, %02x\n", names[offset & 7], data);
		break;
	}

	if(m_ism_mode & 0x01)
		ism_fifo_clear();

	if((m_ism_mode ^ prev_mode) & 0x86)
		m_devsel_cb(m_ism_mode & 0x80 ? (m_ism_mode >> 1) & 3 : 0);
	if((m_ism_mode ^ prev_mode) & 0x20)
		m_hdsel_cb((m_ism_mode >> 5) & 1);

	if((m_ism_mode & 0x18) == 0x18 && ((prev_mode & 0x18) != 0x18)) {
		// Entering write mode
		m_ism_current_bit = 0;
		logerror("%s write start %s %s floppy=%p\n", machine().time().to_string(), m_ism_setup & 0x40 ? "gcr" : "mfm", m_ism_setup & 0x08 ? "fclk/2" : "fclk", m_floppy);
		m_flux_write_start = m_last_sync;
		m_flux_write_count = 0;

	} else if((prev_mode & 0x18) == 0x18 && (m_ism_mode & 0x18) != 0x18) {
		// Exiting write mode
		flush_write();
		m_flux_write_start = 0;
		m_ism_current_bit = 0xff;
		m_ism_half_cycles_before_change = 0;
		logerror("%s write end\n", machine().time().to_string());
	}

	if((m_ism_mode & 0x18) == 0x08 && ((prev_mode & 0x18) != 0x08)) {
		// Entering read mode
		m_ism_current_bit = 0;
		m_ism_sr = 0;
		m_ism_latest_edge = m_last_sync;
		m_ism_prev_ls = (1<<2) | 1;
		m_ism_csm_state = m_ism_setup & 0x04 ? CSM_SYNCHRONIZED : CSM_INIT;
		m_ism_csm_error_counter[0] = m_ism_csm_error_counter[1] = 0;
		m_ism_correction_factor[0] = m_ism_correction_factor[1] = 0;
		m_ism_csm_pair_side = 0;
		m_ism_csm_min_count = 0;

		logerror("%s read start %s %s floppy=%p\n", machine().time().to_string(), m_ism_setup & 0x04 ? "gcr" : "mfm", m_ism_setup & 0x08 ? "fclk/2" : "fclk", m_floppy);

	} else if((prev_mode & 0x18) == 0x08 && (m_ism_mode & 0x18) != 0x08) {
		// Exiting read mode
		flush_write();
		m_ism_current_bit = 0xff;
		m_ism_half_cycles_before_change = 0;
		logerror("%s read end\n", machine().time().to_string());
	}
}

void swim1_device::device_timer(emu_timer &, device_timer_id, int)
{
	if(m_iwm_active == MODE_DELAY) {
		flush_write();
		m_iwm_active = MODE_IDLE;
		m_iwm_rw = MODE_IDLE;
		m_iwm_rw_state = S_IDLE;
		if(!(m_ism_mode & 0x40))
			m_devsel_cb(0);
		m_iwm_devsel = 0;
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

void swim1_device::iwm_control(int offset, u8 data)
{
	iwm_sync();

	u8 prev_iwm_to_ism_counter = m_iwm_to_ism_counter;

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

	if(m_iwm_control & 0x10) {
		if(m_iwm_active != MODE_ACTIVE) {
			m_iwm_active = MODE_ACTIVE;
			m_iwm_status |= 0x20;
			if(m_floppy)
				m_floppy->mon_w(false);
		}

		if((m_iwm_control & 0x80) == 0x00) {
			if(m_iwm_rw != MODE_READ) {
				if(m_iwm_rw == MODE_WRITE) {
					flush_write();
					m_flux_write_start = 0;
				}
				m_iwm_rw = MODE_READ;
				m_iwm_rw_state = S_IDLE;
				m_iwm_next_state_change = 0;
				m_iwm_sync_update = 0;
				m_iwm_async_update = 0;
				m_iwm_data = 0x00;
			}

		} else {
			if(m_iwm_rw != MODE_WRITE) {
				m_iwm_rw = MODE_WRITE;
				m_iwm_rw_state = S_IDLE;
				m_iwm_whd |= 0x40;
				m_iwm_next_state_change = 0;
				m_flux_write_start = m_last_sync;
				m_flux_write_count = 0;
				if(m_floppy)
					m_floppy->set_write_splice(cycles_to_time(m_flux_write_start));
			}
		}
	} else {
		if(m_iwm_active == MODE_ACTIVE) {
			flush_write();
			if(m_iwm_mode & 0x04) {
				m_flux_write_start = 0;
				m_iwm_active = MODE_IDLE;
				m_iwm_rw = MODE_IDLE;
				m_iwm_rw_state = S_IDLE;
				m_iwm_status &= ~0x20;
				m_iwm_whd &= ~0x40;
				if(m_floppy)
					m_floppy->mon_w(true);
			} else {
				m_devsel_cb(m_iwm_control & 0x20 ? 2 : 1);
				m_iwm_active = MODE_DELAY;
				m_timer->adjust(cycles_to_time(8388608));
			}
		}
	}

	u8 devsel = m_iwm_active != MODE_IDLE ? m_iwm_control & 0x20 ? 2 : 1 : 0;
	if(devsel != m_iwm_devsel) {
		m_iwm_devsel = devsel;
		m_devsel_cb(devsel);
	}

	if(0) {
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

		logerror("%s control %c%c %c%c %c%c%c%c (%s) [%s, %s] whd=%02x data=%02x\n",
				 machine().time().to_string(),
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
				 m_iwm_rw == MODE_IDLE ? "idle" : m_iwm_rw == MODE_READ ? "read" : "write",
				 m_iwm_whd, m_iwm_data);
	}

	if(m_iwm_active && !(m_iwm_control & 0x80) && !iwm_is_sync() && (m_iwm_data & 0x80))
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
				u8 ism_devsel = m_ism_mode & 0x80 ? (m_ism_mode >> 1) & 3 : 0;
				if(ism_devsel != m_iwm_devsel)
					m_devsel_cb(ism_devsel);
			}
			break;
		}
	}
	if(m_iwm_to_ism_counter != prev_iwm_to_ism_counter+1)
		m_iwm_to_ism_counter = 0;
	else
		logerror("iwm counter = %d\n", m_iwm_to_ism_counter);

	if((m_iwm_control & 0xc0) == 0xc0 && (offset & 1))
	{
		if(m_iwm_active)
			iwm_data_w(data);
		else
			iwm_mode_w(data);
	}
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

// The ism part of the swim1 works in half-clocks
u64 swim1_device::time_to_cycles(const attotime &tm) const
{
	return tm.as_ticks(2*clock());
}

attotime swim1_device::cycles_to_time(u64 cycles) const
{
	return attotime::from_ticks(cycles, 2*clock());
}

void swim1_device::ism_fifo_clear()
{
	m_ism_fifo_pos = 0;
	m_dat1byte_cb((m_ism_mode & 0x10) ? 1 : 0);
	ism_crc_clear();
}

bool swim1_device::ism_fifo_push(u16 data)
{
	if(m_ism_fifo_pos == 2)
		return true;
	m_ism_fifo[m_ism_fifo_pos ++] = data;
	if(m_ism_mode & 0x10) {
		// write
		if(m_ism_fifo_pos == 2)
			m_dat1byte_cb(0);
	} else {
		// read
		if(m_ism_fifo_pos == 1)
			m_dat1byte_cb(1);
	}
	return false;
}

u16 swim1_device::ism_fifo_pop()
{
	if(m_ism_fifo_pos == 0)
		return 0xffff;
	u16 r = m_ism_fifo[0];
	m_ism_fifo[0] = m_ism_fifo[1];
	m_ism_fifo_pos --;
	if(m_ism_mode & 0x10) {
		// write
		if(m_ism_fifo_pos == 1)
			m_dat1byte_cb(1);
	} else {
		// read
		if(m_ism_fifo_pos == 0)
			m_dat1byte_cb(0);
	}
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
	case 0x00: return 2*14;
	case 0x08: return 2* 7;
	case 0x10: return 2*16;
	case 0x18: return 2* 8;
	}
	abort();
}

u64 swim1_device::iwm_window_size() const
{
	switch(m_iwm_mode & 0x18) {
	case 0x00: return 2*28;
	case 0x08: return 2*14;
	case 0x10: return 2*36;
	case 0x18: return 2*16;
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

	u64 next_sync = time_to_cycles(machine().time());
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
					m_iwm_async_update = 0;
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
			if(!iwm_is_sync())
				m_iwm_data = 0;
			m_iwm_async_update = 0;
		}
		break;
	}

	case MODE_WRITE: {
		while(next_sync > m_last_sync) {
			if(next_sync < m_iwm_next_state_change || !(m_iwm_whd & 0x40)) {
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
					logerror("underrun\n");
					flush_write();
					m_flux_write_start = 0;
					m_iwm_whd &= ~0x40;
					m_last_sync = next_sync;
					m_iwm_rw_state = SW_UNDERRUN;

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

			case SW_UNDERRUN:
				m_last_sync = next_sync;
				break;
			}
		}
		break;
	}
	}
}

void swim1_device::ism_sync()
{
	u64 next_sync = time_to_cycles(machine().time());
	if(!(m_ism_mode & 0x08)) {
		m_last_sync = next_sync;
		return;
	}

	if(m_ism_mode & 0x10) {
		u32 cycles = next_sync - m_last_sync;

		// Write mode
		while(cycles) {
			if(m_ism_half_cycles_before_change) {
				if(cycles >= m_ism_half_cycles_before_change) {
					cycles -= m_ism_half_cycles_before_change;
					m_ism_half_cycles_before_change = 0;
				} else {
					m_ism_half_cycles_before_change -= cycles;
					cycles = 0;
					break;
				}
			}

			if(m_ism_tss_output & 0xc) {
				bool bit;
				if(m_ism_tss_output & 8) {
					bit = (m_ism_tss_output >> 1) & 1;
					m_ism_tss_output &= ~0xa;
				} else {
					bit = m_ism_tss_output & 1;
					m_ism_tss_output = 0;
				}
				if(bit) {
					if(m_flux_write_count == m_flux_write.size())
						flush_write(next_sync - cycles);
					m_flux_write[m_flux_write_count ++] = next_sync - cycles;
					m_ism_half_cycles_before_change = m_ism_param[P_TIME1] + 2*2;
				} else
					m_ism_half_cycles_before_change = m_ism_param[P_TIME0] + 2*2;
				if(m_ism_setup & 8)
					m_ism_half_cycles_before_change <<= 1;
				continue;
			}
			if(m_ism_current_bit == 0xff)
				fatalerror("Sequence break on write\n");

			if(m_ism_current_bit == 0) {
				if(m_ism_sr & M_CRC)
					m_ism_sr = m_ism_crc >> 8;
				else {
					u16 r = ism_fifo_pop();
					if(r == 0xffff && !m_ism_error) {
						m_ism_error |= 0x01;
						flush_write();
						m_ism_current_bit = 0xff;
						m_ism_half_cycles_before_change = 0;
						m_ism_mode &= ~8;
						logerror("write end on underrun\n");
						break;
					}
					if(r & M_CRC)
						m_ism_sr = M_CRC | (m_ism_crc >> 8);
					else
						m_ism_sr = r & (M_MARK | M_CRC | 0xff);
				}
				m_ism_current_bit = 8;
				if(m_ism_sr & M_MARK)
					ism_crc_clear();
			}
			m_ism_current_bit --;
			int bit = (m_ism_sr >> m_ism_current_bit) & 1;
			if(!(m_ism_sr & M_MARK))
				ism_crc_update(bit);
			m_ism_tss_sr = (m_ism_tss_sr << 1) | bit;
			if(m_ism_setup & 0x40)
				m_ism_tss_output = 4 | bit;
			else {
				static const u8 tss[4] = { 5, 0xd, 4, 5 };
				if((m_ism_sr & M_MARK) && ((m_ism_tss_sr & 0xf) == 8))
					m_ism_tss_output = 0xc;
				else
					m_ism_tss_output = tss[m_ism_tss_sr & 3];
			}

			continue;
		}
	} else {
		while(m_last_sync < next_sync) {
			// Find when in the future the next edge happens
			u64 cycles_to_next;
			bool will_hit_edge;
			if(!m_floppy) {
				cycles_to_next = next_sync - m_ism_latest_edge;
				will_hit_edge = false;
			} else {
				auto when = m_floppy->get_next_transition(cycles_to_time(m_ism_latest_edge+2));
				if(when == attotime::never || when > cycles_to_time(next_sync)) {
					cycles_to_next = next_sync - m_ism_latest_edge;
					will_hit_edge = false;
				} else {
					cycles_to_next = time_to_cycles(when) - m_ism_latest_edge;
					will_hit_edge = true;
				}
			}

			// Pick up the current rescaling factor
			int scale = m_ism_correction_factor[m_ism_csm_pair_side];
			if(scale < 192)
				scale |= 256;

			// Count the number of cells in the L and the S hypothesis
			u32 sct, lct;
			if(m_ism_prev_ls == 0x5) {
				// Previous was a short
				if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2)) >> 8)
					sct = 0;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_SSS] + 2*2)) >> 8)
					sct = 1;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_SSS] + 2*2 + m_ism_param[P_SLS] + 2*2)) >> 8)
					sct = 2;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_SSS] + 2*2 + m_ism_param[P_SLS] + 2*2 + m_ism_param[P_RPT] + 2*2)) >> 8)
					sct = 3;
				else
					sct = 4;
				if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2)) >> 8)
					lct = 0;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_SSL] + 2*2)) >> 8)
					lct = 1;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_SSL] + 2*2 + m_ism_param[P_SLL] + 2*2)) >> 8)
					lct = 2;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_SSL] + 2*2 + m_ism_param[P_SLL] + 2*2 + m_ism_param[P_RPT] + 2*2)) >> 8)
					lct = 3;
				else
					lct = 4;

			} else if(m_ism_prev_ls == 0x6 || m_ism_prev_ls == 0x7 || m_ism_prev_ls == 0x9 || m_ism_prev_ls == 0xd) {
				// Previous was marginal
				if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2)) >> 8)
					sct = 0;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSS] + 2*2)) >> 8)
					sct = 1;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSS] + 2*2 + m_ism_param[P_CSLS] + 2*2)) >> 8)
					sct = 2;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSS] + 2*2 + m_ism_param[P_CSLS] + 2*2 + m_ism_param[P_RPT] + 2*2)) >> 8)
					sct = 3;
				else
					sct = 4;
				if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2)) >> 8)
					lct = 0;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSL] + 2*2)) >> 8)
					lct = 1;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSL] + 2*2 + m_ism_param[P_CSLS] + 2*2)) >> 8)
					lct = 2;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSL] + 2*2 + m_ism_param[P_CSLS] + 2*2 + m_ism_param[P_RPT] + 2*2)) >> 8)
					lct = 3;
				else
					lct = 4;

			} else {
				// Previous was long
				if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2)) >> 8)
					sct = 0;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSS] + 2*2)) >> 8)
					sct = 1;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSS] + 2*2 + m_ism_param[P_LLS] + 2*2)) >> 8)
					sct = 2;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSS] + 2*2 + m_ism_param[P_LLS] + 2*2 + m_ism_param[P_RPT] + 2*2)) >> 8)
					sct = 3;
				else
					sct = 4;
				if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2)) >> 8)
					lct = 0;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSL] + 2*2)) >> 8)
					lct = 1;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSL] + 2*2 + m_ism_param[P_LLL] + 2*2)) >> 8)
					lct = 2;
				else if(cycles_to_next <= (scale * (m_ism_param[P_MINCT] + 3*2 + m_ism_param[P_LSL] + 2*2 + m_ism_param[P_LLL] + 2*2 + m_ism_param[P_RPT] + 2*2)) >> 8)
					lct = 3;
				else
					lct = 4;
			}

			// Actually resolve the cell lengths according to the counts
			int resolved_ls_count = 0;
			int resolved_ls_type[2] = { 0, 0 };
			if((sct == 4 || lct == 4) && !m_ism_error)
				m_ism_error |= 0x20;

			if(will_hit_edge) {
				if(sct == 0) {
					// Don't do the short cell error, write splices often trigger it and the physical media
					// probably doesn't actually allow for it.
					sct = lct = 1;
				}
				if(sct == 4)
					sct = 3;
				if(lct == 4)
					lct = 3;

				bool previous_marginal = m_ism_prev_ls == 0x6 || m_ism_prev_ls == 0x7 || m_ism_prev_ls == 0x9 || m_ism_prev_ls == 0xd;
				bool current_marginal = (sct == 1 && lct > 1) || (lct == 1 && sct > 1);

				if(previous_marginal && current_marginal) {
					if(!m_ism_error)
						m_ism_error |= 0x40;
					resolved_ls_count = 2;
					resolved_ls_type[0] = (m_ism_prev_ls >> 2) & 3;
					resolved_ls_type[1] = lct;
				} else {
					if(previous_marginal) {
						if(sct == 1)
							resolved_ls_type[resolved_ls_count++] = m_ism_prev_ls & 3;
						else
							resolved_ls_type[resolved_ls_count++] = (m_ism_prev_ls >> 2) & 3;
					}
					if(!current_marginal) {
						if(sct == 1)
							resolved_ls_type[resolved_ls_count++] = sct;
						else
							resolved_ls_type[resolved_ls_count++] = lct;
					}
				}

				m_ism_prev_ls = (lct << 2) | sct;
			}

			// If we have cell lengths, run the Correction State Machine and the Trans-Space Machine appropriately
			for(int i=0; i != resolved_ls_count; i++) {
				int type = resolved_ls_type[i];
				bool drop_one_bit = false;
				switch(m_ism_csm_state) {
				case CSM_INIT:
					m_ism_csm_error_counter[0] = m_ism_csm_error_counter[1] = 0;
					m_ism_csm_pair_side = 0;
					m_ism_csm_min_count = 0;
					m_ism_csm_state = CSM_COUNT_MIN;
					break;

				case CSM_COUNT_MIN:
					if(type != 1) {
						m_ism_csm_state = CSM_INIT;
						break;
					}
					m_ism_csm_error_counter[m_ism_csm_pair_side] += m_ism_param[P_MULT] * (cycles_to_next >> 1);
					m_ism_csm_min_count ++;
					if(m_ism_csm_min_count == 64) {
						for(int i=0; i != 2; i++) {
							m_ism_correction_factor[i] = m_ism_csm_error_counter[i] >> 8;
							if(!m_ism_error && (m_ism_csm_error_counter[i] < 0xc000 || m_ism_csm_error_counter[i] >= 0x1c000))
								m_ism_error |= 0x08;
						}
						m_ism_csm_state = CSM_WAIT_NON_MIN;
					}
					break;

				case CSM_WAIT_NON_MIN:
					if(type == 1) {
						break;
					}
					m_ism_csm_state = CSM_CHECK_MARK;
					m_ism_tsm_out = 0;
					m_ism_tsm_mark = false;
					m_ism_tsm_bits = 0;
					ism_crc_clear();
					drop_one_bit = true;
					[[fallthrough]];

					// 2 1 3 1 3 2 2 2 1
					// 2 3 2 3 2

				case CSM_CHECK_MARK:
				case CSM_SYNCHRONIZED:
					if(m_ism_setup & 0x04) {
						for(int i=0; i != type; i++) {
							int bit = (i+1 == type) ? 1 : 0;
							m_ism_tsm_out = (m_ism_tsm_out << 1) | bit;
							if(m_ism_tsm_out & 0x80) {
								if(ism_fifo_push(m_ism_tsm_out) && !m_ism_error)
									m_ism_error |= 0x01;
								m_ism_tsm_out = 0;
							}
						}
					} else {
						static u32 nb[6] = { 1, 1, 2, 1, 2, 2 };
						static u32 bb[6] = { 1, 0, 1, 0, 1, 0 };
						int idx = (m_ism_tsm_out & 1 ? 0 : 3) + type - 1;
						int nbc = nb[idx];
						int bbc = bb[idx];
						if(drop_one_bit) {
							nbc--;
							drop_one_bit = false;
						}
						if(idx == 5)
							m_ism_tsm_mark = true;
						for(int i=0; i != nbc; i++) {
							int bit = (bbc >> (nbc-1-i)) & 1;
							m_ism_tsm_out = (m_ism_tsm_out << 1) | bit;
							m_ism_tsm_bits ++;
							ism_crc_update(bit);

							if(m_ism_tsm_bits == 8) {
								if(m_ism_csm_state == CSM_CHECK_MARK) {
									if(!m_ism_tsm_mark) {
										m_ism_csm_state = CSM_INIT;
										break;
									}
									m_ism_csm_state = CSM_SYNCHRONIZED;
								}
								u16 val = m_ism_tsm_out & 0xff;
								if(m_ism_tsm_mark) {
									m_ism_tsm_mark = false;
									val |= M_MARK;
									ism_crc_clear();
								}
								if(!m_ism_crc)
									val |= M_CRC0;
								if(ism_fifo_push(val) && !m_ism_error)
									m_ism_error |= 0x01;
								machine().debug_break();
								m_ism_tsm_bits = 0;
							}
						}
					}
					break;
				}

				m_ism_csm_pair_side = !m_ism_csm_pair_side;
			}


			// Go to the next sync point
			if(will_hit_edge) {
				m_ism_latest_edge += cycles_to_next;
				m_last_sync = m_ism_latest_edge;
			} else
				m_last_sync = next_sync;
		}
	}

	m_last_sync = next_sync;
}

void swim1_device::sync()
{
	if(m_ism_mode & 0x40)
		return ism_sync();
	else
		return iwm_sync();
}
