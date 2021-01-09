// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM2 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim2.h"

DEFINE_DEVICE_TYPE(SWIM2, swim2_device, "swim2", "Apple SWIM2 (Sander/Wozniak Integrated Machine) version 2 floppy controller")

swim2_device::swim2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM2, tag, owner, clock)
{
}

void swim2_device::device_start()
{
	applefdintf_device::device_start();

	save_item(NAME(m_mode));
	save_item(NAME(m_setup));
	save_item(NAME(m_param_idx));
	save_item(NAME(m_param));
	save_item(NAME(m_last_sync));
	save_item(NAME(m_crc));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_pos));
	save_item(NAME(m_error));
	save_item(NAME(m_half_cycles_before_change));
	save_item(NAME(m_current_bit));
	save_item(NAME(m_tss_sr));
	save_item(NAME(m_tss_output));
}

void swim2_device::device_reset()
{
	applefdintf_device::device_reset();

	m_mode = 0x40;
	m_setup = 0x00;
	m_param_idx = 0;
	memset(m_param, 0, sizeof(m_param));
	m_floppy = nullptr;
	m_crc = 0xffff;
	m_fifo[0] = m_fifo[1] = 0;
	m_fifo_pos = 0;
	m_error = 0;
	m_half_cycles_before_change = 0;
	m_current_bit = 0;
	m_tss_sr = 0;
	m_tss_output = 0;
	m_devsel_cb(0);
	m_sel35_cb(true);
	m_hdsel_cb(false);

	m_last_sync = machine().time().as_ticks(clock());
}

void swim2_device::device_timer(emu_timer &, device_timer_id, int, void *)
{
}

void swim2_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	if(m_floppy)
		m_floppy->mon_w(true);
	m_floppy = floppy;
	if(m_mode & 0x80)
		m_floppy->mon_w(false);
	update_phases();
}

floppy_image_device *swim2_device::get_floppy() const
{
	return m_floppy;
}

void swim2_device::flush_write(u64 when)
{
	if(!when)
		when = m_last_sync;

	if(m_floppy && when > m_flux_write_start) {
		if(m_flux_write_count && m_flux_write[m_flux_write_count-1] == when)
			m_flux_write_count--;
		attotime start = cycles_to_time(m_flux_write_start);
		attotime end = cycles_to_time(when);
		std::vector<attotime> fluxes(m_flux_write_count);
		u32 i = 0;
		if(m_flux_write_count && m_flux_write[0] == m_flux_write_start)
			i++;
		while(i != m_flux_write_count) {
			fluxes[i] = cycles_to_time(m_flux_write[i]);
			i++;
		}
		m_floppy->write_flux(start, end, m_flux_write_count, m_flux_write_count ? &fluxes[0] : nullptr);
	}
	m_flux_write_count = 0;
	m_flux_write_start = when;
}

void swim2_device::show_mode() const
{
	logerror("mode%s %s hdsel=%c %c%s %c%c%s\n",
			 m_mode & 0x80 ? " motoron" : "",
			 m_mode & 0x40 ? "ism" : "iwm",
			 m_mode & 0x20 ? '1' : '0',
			 m_mode & 0x10 ? 'w' : 'r',
			 m_mode & 0x08 ? " action" : "",
			 m_mode & 0x04 ? 'a' : '-',
			 m_mode & 0x02 ? 'b' : '-',
			 m_mode & 0x01 ? " clear" : "");

}

u8 swim2_device::read(offs_t offset)
{
	sync();

	static const char *const names[] = {
		"?0", "?1", "?2", "?3", "?4", "?5", "?6", "?7",
		"data", "mark", "crc", "param", "phases", "setup", "status", "handshake"
	};
	switch(offset & 7) {
	case 0x0: { // data
		u16 r = fifo_pop();
		if(!m_error) {
			if(r == 0xffff)
				m_error |= 4;
			else if(r & M_MARK)
				m_error |= 2;
		}
		return r;
	}

	case 0x1: { // mark
		u16 r = fifo_pop();
		if(!m_error && r == 0xffff)
			m_error |= 4;
		return r;
	}

	case 0x2: { // errpr
		u8 err = m_error;
		m_error = 0;
		return err;
	}

	case 0x3: { // param
		u8 r = m_param[m_param_idx];
		m_param_idx = (m_param_idx + 1) & 15;
		return r;
	}

	case 0x4: // phases
		return m_phases;

	case 0x5: // setup
		return m_setup;

	case 0x6: // mode
		return m_mode;

	case 0x7: { // handshake
		u8 h = 0;
		if(m_fifo_pos > 0) {
			if(m_fifo[m_fifo_pos - 1] & M_MARK)
				h |= 0x01;
			if(m_fifo[m_fifo_pos - 1] & M_CRC0)
				h |= 0x02;
		}
		// addata on 4
		if(m_floppy && !m_floppy->wpt_r())
			h |= 0x08;
		if(m_error)
			h |= 0x20;
		if(m_mode & 0x10) {
			// write
			if(m_fifo_pos == 0)
				h |= 0x40;
		} else {
			// read
			if(m_fifo_pos == 2)
				h |= 0x40;
		}

		if(!m_error && m_fifo_pos == 1)
			h |= 0x80;
		logerror("handshake %02x\n", h);
		return h;
	}

	default:
		logerror("read %s\n", names[offset & 15]);
		break;
	}
	return 0xff;
}

void swim2_device::write(offs_t offset, u8 data)
{
	sync();

	u8 prev_mode = m_mode;

	static const char *const names[] = {
		"data", "mark", "crc", "param", "phases", "setup", "mode0", "mode1",
		"?8", "?9", "?a", "?b", "?c", "?d", "?e", "?f"
	};
	switch(offset & 7) {
	case 0:
		if(fifo_push(data) && !m_error)
			m_error |= 0x04;
		break;

	case 1:
		if(fifo_push(M_MARK | data) && !m_error)
			m_error |= 0x04;
		break;

	case 2:
		if(fifo_push(M_CRC) && !m_error)
			m_error |= 0x04;
		break;
		
	case 3: { // param
		static const char *const pname[4] = {
			"late", "time0", "early", "time1"
		};
		logerror("param[%s] = %02x\n", pname[m_param_idx], data);
		m_param[m_param_idx] = data;
		m_param_idx = (m_param_idx + 1) & 3;
		break;
	}
	case 4: { // phases
		m_phases = data;
		update_phases();
		break;
	}

	case 5: // setup
		m_setup = data;
		m_sel35_cb(m_setup & 0x02);
		logerror("setup write=%s %s test=%s %s %s 3.5=%s %s\n",
				 m_setup & 0x40 ? "gcr" : "mfm",
				 m_setup & 0x20 ? "ibm" : "apple",
				 m_setup & 0x10 ? "on" : "off",
				 m_setup & 0x08 ? "fclk/2" : "fclk",
				 m_setup & 0x04 ? "gcr" : "mfm",
				 m_setup & 0x02 ? "off" : "on",
				 m_setup & 0x01 ? "wrinvert" : "wrdirect");
		break;

	case 6: // mode clear
		m_mode &= ~data;
		m_mode |= 0x40;
		m_param_idx = 0;
		show_mode();
		break;

	case 7: // mode set
		m_mode |= data;
		show_mode();
		break;

	default:
		logerror("write %s, %02x\n", names[offset], data);
		break;
	}

	if(m_mode & 0x01)
		fifo_clear();

	if((m_mode & 0x18) == 0x18 && ((prev_mode & 0x18) != 0x18)) {
		// Entering write mode
		m_current_bit = 0;
		logerror("%s write start\n", machine().time().to_string());
		machine().debug_break();
	} else if((prev_mode & 0x18) == 0x18 && (m_mode & 0x18) != 0x18) {
		// Exiting write mode
		flush_write();
		m_current_bit = 0xff;
		m_half_cycles_before_change = 0;
		logerror("%s write end\n", machine().time().to_string());
	}
}

u64 swim2_device::time_to_cycles(const attotime &tm) const
{
	return tm.as_ticks(clock());
}

attotime swim2_device::cycles_to_time(u64 cycles) const
{
	return attotime::from_ticks(cycles, clock());
}

void swim2_device::fifo_clear()
{
	m_fifo_pos = 0;
	// must clear the crc to the appropriate r/w-dependant value.  No idea yet which it is, but I have my suspicions.
	m_crc = 0x1234;
}

bool swim2_device::fifo_push(u16 data)
{
	logerror("fifo push %03x\n", data);
	if(m_fifo_pos == 2)
		return true;
	m_fifo[m_fifo_pos ++] = data;
	return false;
}

u16 swim2_device::fifo_pop()
{
	if(m_fifo_pos == 0)
		return 0xffff;
	u16 r = m_fifo[0];
	m_fifo[0] = m_fifo[1];
	m_fifo_pos --;
	return r;
}

// cell times
// 1 us:           31.32
// 1.5us:          47.32
// 2us:    31.16   63.32
// 3us:    47.16   94.32
// 4us:    63.16  125.32
// 6us:    94.16  188.32

// time1 = 31, time0 = 16 for clock /1 mfm
// time1 = 

// 32 mfm, t1=63, t0=31
// 16 mfm, t1=31.5, t0=15.5
// 16 gcr, t1=31.5, t0=31.5

void swim2_device::sync()
{
	u64 next_sync = time_to_cycles(machine().time());
	if(!(m_mode & 0x08)) {
		m_last_sync = next_sync;
		return;
	}

	// We count in half-cycles but only toggle write on full cycles

	u32 cycles = (next_sync - m_last_sync) << 1;
	//	logerror("ACTIVE %s %d-%d (%d)\n", m_mode & 0x10 ? "write" : "read", m_last_sync, next_sync, cycles);

	while(cycles) {
		//		logerror("half cycles avail %d needed %d\n", cycles, m_half_cycles_before_change);
		if(m_half_cycles_before_change) {
			if(cycles >= m_half_cycles_before_change) {
				cycles -= m_half_cycles_before_change;
				m_half_cycles_before_change = 0;
			} else {
				m_half_cycles_before_change -= cycles;
				cycles = 0;
				break;
			}
		}

		if(m_flux_write_count > 32)
			flush_write(next_sync - (cycles >> 1));

		if(m_mode & 0x10) {
			// Write mode

			if(m_tss_output & 0xc) {
				logerror("SR %03x.%d TSS %c%c\n", m_sr, m_current_bit, m_tss_output & 8 ? m_tss_output & 2 ? '1' : '0' : '.', m_tss_output & 4 ? m_tss_output & 1 ? '1' : '0' : '.');
				bool bit;
				if(m_tss_output & 8) {
					bit = (m_tss_output >> 1) & 1;
					m_tss_output &= ~0xa;
				} else {
					bit = m_tss_output & 1;
					m_tss_output = 0;
				}
				if(bit) {
					m_flux_write[m_flux_write_count ++] = next_sync - (cycles >> 1);
					m_half_cycles_before_change = 63;
				} else
					m_half_cycles_before_change = m_setup & 0x40 ? 63 : 31;
				if(m_setup & 8)
					m_half_cycles_before_change <<= 1;
				logerror("T%d, %d half cycles\n", bit, m_half_cycles_before_change);
				continue;
			}
			if(m_current_bit == 0xff)
				fatalerror("Sequence break on write\n");

			if(m_current_bit == 0) {
				u16 r = fifo_pop();
				logerror("fifo pop %03x\n", r);
				if(r == 0xffff && !m_error) {
					m_error |= 0x01;
					flush_write();
					m_current_bit = 0xff;
					m_half_cycles_before_change = 0;
					m_mode &= ~8;
					logerror("write end on underrun\n");
					break;
				}
				if(r & M_CRC) {
					fatalerror("crc alpha\n");
				}
				m_sr = r & (M_MARK | M_CRC | 0xff);
				m_current_bit = 8;
			}
			m_current_bit --;
			bool bit = (m_sr >> m_current_bit) & 1;
			m_tss_sr = (m_tss_sr << 1) | bit;
			if(m_setup & 0x40)
				m_tss_output = 4 | bit;
			else {
				static const u8 tss[4] = { 5, 0xd, 4, 5 };
				if((m_sr & M_MARK) && (m_tss_sr == 8))
					m_tss_output = 0xc;
				else
					m_tss_output = tss[m_tss_sr & 3];
			}
			continue;
		} else {
			cycles = 0;
		}
	}
	m_last_sync = next_sync;
}
