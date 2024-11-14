// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM2 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim2.h"

#define LOG_SETUP   (1U << 1)
#define LOG_MODE    (1U << 2)
#define VERBOSE     0
#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,    __VA_ARGS__)

DEFINE_DEVICE_TYPE(SWIM2, swim2_device, "swim2", "Apple SWIM2 (Sander/Wozniak Integrated Machine) version 2 floppy controller")

swim2_device::swim2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM2, tag, owner, clock),
	m_floppy(nullptr)
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
	save_item(NAME(m_sr));
	save_item(NAME(m_mfm_sync_counter));
	save_item(NAME(m_flux_write_start));
	save_item(NAME(m_flux_write));
	save_item(NAME(m_flux_write_count));
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
	m_sr = 0;
	m_mfm_sync_counter = 0;
	m_devsel_cb(0);
	m_sel35_cb(true);
	m_hdsel_cb(false);
	m_dat1byte_cb(CLEAR_LINE);
	m_flux_write_start = 0;
	m_flux_write_count = 0;
	std::fill(m_flux_write.begin(), m_flux_write.end(), 0);

	m_last_sync = machine().time().as_ticks(clock());
}

void swim2_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	sync();
	flush_write();

	m_floppy = floppy;
	update_phases();
	m_hdsel_cb((m_mode >> 5) & 1);
}

floppy_image_device *swim2_device::get_floppy() const
{
	return m_floppy;
}

void swim2_device::flush_write(u64 when)
{
	if(!m_flux_write_start)
		return;

	if(!when)
		when = m_last_sync;

	if(m_floppy && when > m_flux_write_start) {
		bool last_on_edge = m_flux_write_count && m_flux_write[m_flux_write_count-1] == when;
		if(last_on_edge)
			m_flux_write_count--;

		attotime start = cycles_to_time(m_flux_write_start);
		attotime end = cycles_to_time(when);
		std::vector<attotime> fluxes(m_flux_write_count);
		for(u32 i=0; i != m_flux_write_count; i++)
			fluxes[i] = cycles_to_time(m_flux_write[i]);
		m_floppy->write_flux(start, end, m_flux_write_count, m_flux_write_count ? &fluxes[0] : nullptr);

		m_flux_write_count = 0;
		if(last_on_edge)
			m_flux_write[m_flux_write_count++] = when;
		m_flux_write_start = when;
	} else
		m_flux_write_count = 0;
}

void swim2_device::show_mode() const
{
	LOGMODE("mode%s hdsel=%c %c%s %c%c%s\n",
			 m_mode & 0x80 ? " motoron" : "",
			 m_mode & 0x20 ? '1' : '0',
			 m_mode & 0x10 ? 'w' : 'r',
			 m_mode & 0x08 ? " action" : "",
			 m_mode & 0x04 ? 'b' : '-',
			 m_mode & 0x02 ? 'a' : '-',
			 m_mode & 0x01 ? " clear" : "");

}

u8 swim2_device::read(offs_t offset)
{
	sync();

	static const char *const names[] = {
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

	case 0x2: { // error
		u8 err = m_error;
		m_error = 0;
		return err;
	}

	case 0x3: { // param
		u8 r = m_param[m_param_idx];
		m_param_idx = (m_param_idx + 1) & 3;
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
			if(!(m_fifo[m_fifo_pos - 1] & M_CRC0))
				h |= 0x02;
		}
		// rddata on 4
		if(!m_floppy || m_floppy->wpt_r())
			h |= 0x08;
		if(m_error)
			h |= 0x20;
		if(m_mode & 0x10) {
			// write
			if(m_fifo_pos == 0)
				h |= 0xc0;
			else if(m_fifo_pos == 1)
				h |= 0x80;
		} else {
			// read
			if(m_fifo_pos == 2)
				h |= 0xc0;
			else if(m_fifo_pos == 1)
				h |= 0x80;
		}
		return h;
	}

	default:
		LOG("read %s\n", names[offset & 7]);
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
		LOG("param[%s] = %02x\n", pname[m_param_idx], data);
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
		m_sel35_cb((m_setup >> 1) & 1);
		LOGSETUP("setup write=%s %s test=%s %s %s 3.5=%s %s\n",
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
		update_dat1byte();
		break;

	case 7: // mode set
		m_mode |= data;
		show_mode();
		update_dat1byte();
		break;

	default:
		LOG("write %s, %02x\n", names[offset & 7], data);
		break;
	}

	if(m_mode & 0x01)
		fifo_clear();

	if((m_mode ^ prev_mode) & 0x86)
		m_devsel_cb(m_mode & 0x80 ? (m_mode >> 1) & 3 : 0);
	if((m_mode ^ prev_mode) & 0x20)
		m_hdsel_cb((m_mode >> 5) & 1);

	if((m_mode & 0x18) == 0x18 && ((prev_mode & 0x18) != 0x18)) {
		// Entering write mode
		m_current_bit = 0;
		LOG("%s write start %s %s floppy=%p\n", machine().time().to_string(), m_setup & 0x40 ? "gcr" : "mfm", m_setup & 0x08 ? "fclk/2" : "fclk", m_floppy);
		m_flux_write_start = m_last_sync;
		m_flux_write_count = 0;

	} else if((prev_mode & 0x18) == 0x18 && (m_mode & 0x18) != 0x18) {
		// Exiting write mode
		flush_write();
		m_flux_write_start = 0;
		m_current_bit = 0xff;
		m_half_cycles_before_change = 0;
		LOG("%s write end\n", machine().time().to_string());
	}

	if((m_mode & 0x18) == 0x08 && ((prev_mode & 0x18) != 0x08)) {
		// Entering read mode
		m_current_bit = 0;
		m_sr = 0;
		m_mfm_sync_counter = 0;
		LOG("%s read start %s %s floppy=%p\n", machine().time().to_string(), m_setup & 0x04 ? "gcr" : "mfm", m_setup & 0x08 ? "fclk/2" : "fclk", m_floppy);

		m_pll.reset(machine().time());
		static const int cycles_per_cell[4] = { 16, 31, 31, 63 };

		m_pll.set_clock(attotime::from_ticks(cycles_per_cell[(m_setup >> 2) & 3], clock()));
		LOG("PLL read clock %s\n", attotime::from_ticks(cycles_per_cell[(m_setup >> 2) & 3], clock()).to_string());

	} else if((prev_mode & 0x18) == 0x08 && (m_mode & 0x18) != 0x08) {
		// Exiting read mode
		flush_write();
		m_current_bit = 0xff;
		m_half_cycles_before_change = 0;
		LOG("%s read end\n", machine().time().to_string());
	}
}

void swim2_device::crc_clear()
{
	m_crc = 0xcdb4;
}

void swim2_device::crc_update(int bit)
{
	if((m_crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
		m_crc = (m_crc << 1) ^ 0x1021;
	else
		m_crc = m_crc << 1;

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
	update_dat1byte();
	crc_clear();
}

bool swim2_device::fifo_push(u16 data)
{
	if(m_fifo_pos == 2)
		return true;
	m_fifo[m_fifo_pos ++] = data;
	update_dat1byte();
	return false;
}

u16 swim2_device::fifo_pop()
{
	if(m_fifo_pos == 0)
		return 0xffff;
	u16 r = m_fifo[0];
	m_fifo[0] = m_fifo[1];
	m_fifo_pos --;
	update_dat1byte();
	return r;
}

void swim2_device::sync()
{
	u64 next_sync = time_to_cycles(machine().time());
	if(!(m_mode & 0x08)) {
		m_last_sync = next_sync;
		return;
	}

	if(m_mode & 0x10) {
		// We count in half-cycles but only toggle write on full cycles
		u32 cycles = (next_sync - m_last_sync) << 1;

		// Write mode
		while(cycles) {
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

			if(m_tss_output & 0xc) {
				bool bit;
				if(m_tss_output & 8) {
					bit = (m_tss_output >> 1) & 1;
					m_tss_output &= ~0xa;
				} else {
					bit = m_tss_output & 1;
					m_tss_output = 0;
				}
				if(bit) {
					if(m_flux_write_count == m_flux_write.size())
						flush_write(next_sync - (cycles >> 1));
					m_flux_write[m_flux_write_count ++] = next_sync - (cycles >> 1);
					m_half_cycles_before_change = 63;
				} else
					m_half_cycles_before_change = m_setup & 0x40 ? 63 : 31;
				if(m_setup & 8)
					m_half_cycles_before_change <<= 1;
				continue;
			}
			if(m_current_bit == 0xff)
				fatalerror("Sequence break on write\n");

			if(m_current_bit == 0) {
				if(m_sr & M_CRC)
					m_sr = m_crc >> 8;
				else {
					u16 r = fifo_pop();
					if(r == 0xffff && !m_error) {
						m_error |= 0x01;
						flush_write();
						m_current_bit = 0xff;
						m_half_cycles_before_change = 0;
						m_mode &= ~8;
						logerror("write end on underrun\n");
						break;
					}
					if(r & M_CRC)
						m_sr = M_CRC | (m_crc >> 8);
					else
						m_sr = r & (M_MARK | M_CRC | 0xff);
				}
				m_current_bit = 8;
				if(m_sr & M_MARK)
					crc_clear();
			}
			m_current_bit --;
			int bit = (m_sr >> m_current_bit) & 1;
			if(!(m_sr & M_MARK))
				crc_update(bit);
			m_tss_sr = (m_tss_sr << 1) | bit;
			if(m_setup & 0x40)
				m_tss_output = 4 | bit;
			else {
				static const u8 tss[4] = { 5, 0xd, 4, 5 };
				if((m_sr & M_MARK) && ((m_tss_sr & 0xf) == 8))
					m_tss_output = 0xc;
				else
					m_tss_output = tss[m_tss_sr & 3];
			}

			continue;
		}
	} else {
		attotime limit = machine().time();
		if(m_setup & 0x04) {
			// GCR mode
			for(;;) {
				attotime when;
				int bit = m_pll.get_next_bit(when, m_floppy, limit);
				if(bit == -1)
					break;
				m_sr = ((m_sr << 1) | bit) & 0xff;
				if(m_sr & 0x80) {
					if(fifo_push(m_sr) && !m_error)
						m_error |= 0x01;
					m_sr = 0;
				}
			}
		} else {
			// MFM mode
			for(;;) {
				attotime when;
				int bit = m_pll.get_next_bit(when, m_floppy, limit);
				if(bit == -1)
					break;
				if(m_mfm_sync_counter < 64) {
					if(bit != (m_mfm_sync_counter & 1))
						m_mfm_sync_counter ++;
					else
						m_mfm_sync_counter = 0;
				} else {
					if(m_mfm_sync_counter == 64 && bit)
						m_mfm_sync_counter --;
					else {
						if(m_mfm_sync_counter == 65 || m_mfm_sync_counter == 81) {
							m_tss_sr = 0xff;
							m_sr = 0;
						}
						if(m_mfm_sync_counter & 1) {
							m_sr |= bit << (((96 - m_mfm_sync_counter) >> 1) & 7);
							crc_update(bit);
						}
						m_tss_sr = (m_tss_sr << 1) | bit;
						if((m_tss_sr & 0xf) == 1 && !(m_mfm_sync_counter & 1))
							m_sr |= M_MARK;

						m_mfm_sync_counter ++;
						if(m_mfm_sync_counter == 80) {
							if(!(m_sr & M_MARK))
								m_mfm_sync_counter = 0;
							else {
								crc_clear();
								if(fifo_push(m_sr) && !m_error)
									m_error |= 0x01;
							}
						} else if(m_mfm_sync_counter == 96) {
							m_mfm_sync_counter -= 16;
							if(m_sr & M_MARK)
								crc_clear();
							else if(!m_crc)
								m_sr |= M_CRC0;
							if(fifo_push(m_sr) && !m_error)
								m_error |= 0x01;
						}
					}
				}
			}
		}
	}

	m_last_sync = next_sync;
}

void swim2_device::update_dat1byte()
{
	if (m_mode & 0x10)
	{
		// write: Does FIFO have room?
		m_dat1byte_cb((m_fifo_pos < 2) ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		// read: is FIFO not empty?
		m_dat1byte_cb((m_fifo_pos > 0) ? ASSERT_LINE : CLEAR_LINE);
	}
}
