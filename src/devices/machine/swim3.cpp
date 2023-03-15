// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM3 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim3.h"

DEFINE_DEVICE_TYPE(SWIM3, swim3_device, "swim3", "Apple SWIM3 (Sander/Wozniak Integrated Machine) version 3 floppy controller")

swim3_device::swim3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM3, tag, owner, clock),
	m_irq_cb(*this),
	m_drq_cb(*this)
{
}

void swim3_device::device_start()
{
	applefdintf_device::device_start();

	m_irq_cb.resolve_safe();
	m_drq_cb.resolve_safe();
	m_timer = timer_alloc(FUNC(swim3_device::update), this);

	save_item(NAME(m_mode));
	save_item(NAME(m_setup));
	save_item(NAME(m_param));
	save_item(NAME(m_irq));
	save_item(NAME(m_imask));
	save_item(NAME(m_state));
	save_item(NAME(m_step));
	save_item(NAME(m_error));
	save_item(NAME(m_cur_track));
	save_item(NAME(m_cur_sector));
	save_item(NAME(m_cur_format));
	save_item(NAME(m_sect1));
	save_item(NAME(m_xfer));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_pos));
	save_item(NAME(m_drq_write));
}

void swim3_device::device_reset()
{
	applefdintf_device::device_reset();
	m_floppy = nullptr;

	m_cur_live.state = L_IDLE;
	m_cur_live.tm = attotime::never;

	m_mode = 0x00;
	m_setup = 0x00;
	m_param = 0x77;
	m_irq = 0;
	m_imask = 0;
	m_state = S_IDLE;
	m_step = 0;
	m_error = 0;
	m_cur_track = 0;
	m_cur_sector = 0;
	m_cur_format = 0;
	m_sect1 = 0xff;
	m_xfer = 0;

	m_fifo[0] = m_fifo[1] = 0;
	m_fifo_pos = 0;
	m_drq_write = false;

	m_devsel_cb(0);
	m_sel35_cb(true);
	m_hdsel_cb(false);
}

void swim3_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	if(m_floppy)
		m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());

	logerror("floppy %s\n", floppy ? floppy->tag() : "-");

	m_floppy = floppy;
	update_phases();
	m_hdsel_cb((m_mode >> 5) & 1);

	if(m_floppy)
		m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&swim3_device::index_callback, this));
}

floppy_image_device *swim3_device::get_floppy() const
{
	return m_floppy;
}

void swim3_device::show_mode() const
{
	logerror("mode%s%s hdsel=%c %c%s %c%c%s\n",
			 m_mode & 0x80 ? " step" : "",
			 m_mode & 0x40 ? " format" : "",
			 m_mode & 0x20 ? '1' : '0',
			 m_mode & 0x10 ? 'w' : 'r',
			 m_mode & 0x08 ? " go" : "",
			 m_mode & 0x04 ? 'b' : '-',
			 m_mode & 0x02 ? 'a' : '-',
			 m_mode & 0x01 ? " irq" : "");

}

u8 swim3_device::read(offs_t offset)
{
	static const char *const names[] = {
		"data", "timer", "error", "param", "phases", "setup", "?6", "handshake",
		"interrupt", "step", "track", "sector", "format", "sect1", "xfer", "imask"
	};

	switch(offset) {
	case 0x2: { // error
		u8 r = m_error;
		m_error = 0;
		return r;
	}

	case 0x3: // param
		return m_param;

	case 0x4: // phases
		return m_phases & 0xf;

	case 0x5: // setup
		return m_setup;

	case 0x6: // mode
		return m_mode;

	case 0x7: { // handshake
		u8 h = 0;
		if(!m_floppy || m_floppy->wpt_r())
			h |= 0x0c;
		logerror("hand %02x\n", h);
		return h;
	};

	case 0x8: {
		u8 res = m_irq;
		m_irq = 0;
		update_irq();
		return res;
	}

	case 0xa:
		return m_cur_track;

	case 0xb:
		return m_cur_sector;

	case 0xc:
		return m_cur_format;

	case 0xd:
		return m_sect1;

	case 0xe:
		return m_xfer;

	case 0xf:
		return m_imask;

	default:
		logerror("read %s\n", names[offset & 15]);
		break;
	}
	return 0xff;
}

void swim3_device::write(offs_t offset, u8 data)
{
	u8 prev_mode = m_mode;

	static const char *const names[] = {
		"data", "timer", "error", "param", "phases", "setup", "mode0", "mode1",
		"?8", "step", "track", "sector", "gap", "sect1", "xfer", "imask"
	};
	switch(offset) {
	case 0x3: // param
		m_param = data;
		logerror("precompensation late=%x early=%x\n", m_param >> 4, m_param & 0xf);
		break;

	case 0x4: { // phases
		m_phases = data | 0xf0;
		update_phases();
		break;
	}

	case 0x5: // setup
		m_setup = data;
		m_sel35_cb((m_setup >> 1) & 1);
		logerror("setup write=%s %s nogcrconv=%s %s %s%s %s\n",
				 m_setup & 0x40 ? "gcr" : "mfm",
				 m_setup & 0x20 ? "ibm" : "apple",
				 m_setup & 0x10 ? "on" : "off",
				 m_setup & 0x08 ? "fclk/2" : "fclk",
				 m_setup & 0x04 ? "gcr" : "mfm",
				 m_setup & 0x02 ? " copy" : "",
				 m_setup & 0x01 ? "wrinvert" : "wrdirect");
		break;

	case 0x6: // mode clear
		m_mode &= ~data;
		show_mode();
		break;

	case 0x7: // mode set
		m_mode |= data;
		show_mode();
		break;

	case 0x9: // step
		logerror("load step %02x\n", data);
		m_step = data;
		break;

	case 0xc:
		m_gap = data;
		break;

	case 0xd:
		m_sect1 = data;
		break;

	case 0xe:
		m_xfer = data;
		break;

	case 0xf:
		m_imask = data;
		logerror("imask%s%s%s%s%s\n",
				 m_imask & 0x10 ? " sense" : " -",
				 m_imask & 0x08 ? " sector" : " -",
				 m_imask & 0x04 ? " id" : " -",
				 m_imask & 0x02 ? " step" : " -",
				 m_imask & 0x01 ? " timer" : " -");
		update_irq();
		break;

	default:
		logerror("write %s, %02x\n", names[offset], data);
		break;
	}

	if((m_mode ^ prev_mode) & 0x86)
		m_devsel_cb((m_mode >> 1) & 3);
	if((m_mode ^ prev_mode) & 0x20)
		m_hdsel_cb((m_mode >> 5) & 1);

	if((m_mode & 0x80) && m_state == S_IDLE) {
		logerror("%s stepping start count=%02x floppy=%s\n", machine().time().to_string(), m_step, m_floppy ? m_floppy->tag() : "-");
		m_state = S_STEP;
		run(false, false);
	}

	if((m_mode & 0x08) && m_state == S_IDLE) {
		logerror("%s read start %s %s s1=%02x xfer=%d floppy=%s\n", machine().time().to_string(), m_setup & 0x04 ? "gcr" : "mfm", m_setup & 0x08 ? "fclk/2" : "fclk", m_sect1, m_xfer, m_floppy ? m_floppy->tag() : "-");
		m_state = S_ID;
		run(false, false);
	}

	if(!(m_mode & 0x88) && m_state != S_IDLE) {
		logerror("%s read/step stopped\n", machine().time().to_string());
		m_drq_write = false;
		live_abort();
		m_state = S_IDLE;
	}
}

void swim3_device::update_irq()
{
	logerror("irq %02x m %02x -> %02x\n", m_irq, m_imask, m_irq & m_imask);
	m_irq_cb((m_irq & m_imask) != 0);
}

void swim3_device::update_drq()
{
	if(m_drq_write)
		m_drq_cb(m_fifo_pos != 2);
	else
		m_drq_cb(m_fifo_pos != 0);
}

void swim3_device::index_callback(floppy_image_device *floppy, int state)
{
	if(state) {
		sync();
		run(false, true);
	}
}

TIMER_CALLBACK_MEMBER(swim3_device::update)
{
	sync();
	run(true, false);
}

void swim3_device::delay(int cycles)
{
	m_timer->adjust(attotime::from_ticks(cycles, clock()));
}

void swim3_device::run(bool timeout, bool index)
{
	for(;;) {
		if(m_cur_live.state != L_IDLE) {
			live_run();
			if(m_cur_live.state != L_IDLE)
				return;
		}

		switch(m_state) {
		case S_IDLE:
			return;

		case S_STEP:
			if(!m_step) {
				m_state = S_STEP_DONE;
				break;
			}

			m_phases ^= 8;
			update_phases();
			m_state = S_STEP_1;
			delay(4);
			break;

		case S_STEP_1:
			if(!timeout)
				return;

			m_phases ^= 8;
			update_phases();
			m_state = S_STEP_0;
			delay(2502);
			break;

		case S_STEP_0:
			if(!timeout)
				return;

			m_step --;
			m_state = S_STEP;
			break;

		case S_STEP_DONE:
			m_state = S_IDLE;
			m_mode &= 0x7f;
			m_irq |= 0x02;
			update_irq();
			break;

		case S_ID:
			live_start(m_setup & 0x04 ? L_GCR_SEARCH_ID : L_MFM_SEARCH_ID);
			return;
		}
	}
}

void swim3_device::checkpoint()
{
	m_cur_live.pll.commit(m_floppy, m_cur_live.tm);
	m_checkpoint_live = m_cur_live;
}

void swim3_device::rollback()
{
	m_cur_live = m_checkpoint_live;
}

void swim3_device::live_delay(int state)
{
	m_cur_live.next_state = state;
	m_timer->adjust(m_cur_live.tm - machine().time());
}

void swim3_device::sync()
{
	if(m_cur_live.state != L_IDLE && !m_cur_live.tm.is_never()) {
		if(m_cur_live.tm > machine().time()) {
			if(0)
				logerror("%s: Rolling back and replaying (%s)\n", machine().time().to_string(), m_cur_live.tm.to_string());
			rollback();
			live_run(machine().time());
			m_cur_live.pll.commit(m_floppy, m_cur_live.tm);
		} else {
			if(0)
				logerror("%s: Committing (%s)\n", machine().time().to_string(), m_cur_live.tm.to_string());
			m_cur_live.pll.commit(m_floppy, m_cur_live.tm);
			if(m_cur_live.next_state != -1) {
				m_cur_live.state = m_cur_live.next_state;
				m_cur_live.next_state = -1;
			}
			if(m_cur_live.state == L_IDLE) {
				m_cur_live.pll.stop_writing(m_floppy, m_cur_live.tm);
				m_cur_live.tm = attotime::never;
			}
		}
		m_cur_live.next_state = -1;
		checkpoint();
	}
}


void swim3_device::live_abort()
{
	if(!m_cur_live.tm.is_never() && m_cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	m_cur_live.pll.stop_writing(m_floppy, m_cur_live.tm);
	m_cur_live.tm = attotime::never;
	m_cur_live.state = L_IDLE;
	m_cur_live.next_state = -1;
}

bool swim3_device::read_one_bit(const attotime &limit)
{
	int bit = m_cur_live.pll.get_next_bit(m_cur_live.tm, m_floppy, limit);
	if(bit < 0)
		return true;
	m_cur_live.shift_reg = (m_cur_live.shift_reg << 1) | bit;
	m_cur_live.bit_counter++;
	if(m_cur_live.data_separator_phase) {
		m_cur_live.data_reg = (m_cur_live.data_reg << 1) | bit;
		if((m_cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			m_cur_live.crc = (m_cur_live.crc << 1) ^ 0x1021;
		else
			m_cur_live.crc = m_cur_live.crc << 1;
	}
	m_cur_live.data_separator_phase = !m_cur_live.data_separator_phase;
	return false;
}

bool swim3_device::write_one_bit(const attotime &limit)
{
	bool bit = m_cur_live.shift_reg & 0x8000;
	if(m_cur_live.pll.write_next_bit(bit, m_cur_live.tm, m_floppy, limit))
		return true;
	if(m_cur_live.bit_counter & 1) {
		if((m_cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			m_cur_live.crc = (m_cur_live.crc << 1) ^ 0x1021;
		else
			m_cur_live.crc = m_cur_live.crc << 1;
	}
	m_cur_live.shift_reg = m_cur_live.shift_reg << 1;
	m_cur_live.bit_counter--;
	return false;
}


void swim3_device::live_start(int state, bool start_writing)
{
	m_cur_live.tm = machine().time();
	m_cur_live.state = state;
	m_cur_live.next_state = -1;
	m_cur_live.shift_reg = 0;
	m_cur_live.crc = 0xffff;
	m_cur_live.bit_counter = 0;
	m_cur_live.data_separator_phase = false;
	m_cur_live.data_reg = 0;

	static const int cycles_per_cell[4] = { 16, 31, 31, 63 };
	m_cur_live.pll.reset(m_cur_live.tm);
	m_cur_live.pll.set_clock(attotime::from_ticks(cycles_per_cell[(m_setup >> 2) & 3], clock()));
	if(start_writing)
		m_cur_live.pll.start_writing(machine().time());

	logerror("PLL %s clock %s\n", start_writing ? "write" : "read", attotime::from_ticks(cycles_per_cell[(m_setup >> 2) & 3], clock()).to_string());

	m_checkpoint_live = m_cur_live;

	live_run();
}

u8 swim3_device::dma_r()
{
	if(m_fifo_pos == 0) {
		m_error |= 0x04;
		live_abort();
		m_state = S_IDLE;
		return 0;
	}
	u8 r = m_fifo[0];
	m_fifo[0] = m_fifo[1];
	m_fifo_pos--;
	update_drq();
	return r;
}

void swim3_device::fifo_push(u8 data)
{
	if(m_fifo_pos == 2) {
		m_error |= 0x01;
		live_abort();
		m_state = S_IDLE;
	} else {
		m_fifo[m_fifo_pos++] = data;
		update_drq();
	}
}

void swim3_device::live_run(attotime limit)
{
	if(m_cur_live.state == L_IDLE || m_cur_live.next_state != -1)
		return;

	if(limit == attotime::never) {
		if(m_floppy)
			limit = m_floppy->time_next_index();
		if(limit == attotime::never) {
			// Happens when there's no disk or if the swim3 is not
			// connected to a drive, hence no index pulse. Force a
			// sync from time to time in that case, so that the main
			// cpu timeout isn't too painful.  Avoids looping into
			// infinity looking for data too.

			limit = machine().time() + attotime::from_msec(1);
			m_timer->adjust(attotime::from_msec(1));
		}
	}

	for(;;) {
		switch(m_cur_live.state) {
		case L_MFM_SEARCH_ID:
			if(read_one_bit(limit))
				return;

			if((m_cur_live.bit_counter & 1) != (m_cur_live.shift_reg & 1)) {
				if(m_cur_live.bit_counter < 64) {
					m_cur_live.bit_counter = 0;
					m_cur_live.data_separator_phase = false;
				} else {
					m_cur_live.bit_counter = 1;
					m_cur_live.state = L_MFM_SCAN_ID;
					break;
				}
			}
			if(!(m_cur_live.bit_counter & 1))
				m_cur_live.crc = 0xffff;

			//          logerror("pre counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
			break;

		case L_MFM_SCAN_ID:
			if(read_one_bit(limit))
				return;
			if((m_cur_live.bit_counter == 16 || m_cur_live.bit_counter == 32 || m_cur_live.bit_counter == 48) && m_cur_live.shift_reg != 0x4489) {
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_MFM_SEARCH_ID;
				break;
			}
			if(m_cur_live.bit_counter == 64 && m_cur_live.shift_reg != 0x5554) { // fe
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_MFM_SEARCH_ID;
				break;
			}

			if(m_cur_live.bit_counter == 64) {
				m_cur_live.bit_counter = 0;
				live_delay(L_MFM_READ_ID);
				return;
			}

			//          logerror("post counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
			break;

		case L_MFM_READ_ID:
			if(!m_cur_live.bit_counter)
				m_cur_sector &= 0x7f;

			if(read_one_bit(limit))
				return;

			if(!(m_cur_live.bit_counter & 0xf)) {
				live_delay(L_MFM_READ_ID_BYTE);
				return;
			}
			break;

		case L_MFM_READ_ID_BYTE:
			m_cur_live.state = L_MFM_READ_ID;
			switch(m_cur_live.bit_counter >> 4) {
			case 1:
				m_cur_track = m_cur_live.data_reg & 0x7f;
				break;
			case 2:
				m_cur_track |= (m_cur_live.data_reg & 1) << 7;
				break;
			case 3:
				m_cur_sector = m_cur_live.data_reg & 0x3f;
				break;
			case 4:
				m_cur_format = m_cur_live.data_reg;
				break;
			case 6:
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_MFM_SEARCH_ID;
				if(m_cur_live.crc == 0) {
					logerror("sector id found, track=%02d head=%d sector=%02d\n", m_cur_track & 0x7f, m_cur_track >> 7, m_cur_sector & 0x3f);
					m_cur_sector |= 0x80;
					m_irq |= 0x04;
					update_irq();
					if(!(m_sect1 & 0x80) && ((m_sect1 & 0x40) || (m_sect1 & 0x3f) == (m_cur_sector & 0x3f))) {
						logerror("sector matches, reading data\n");
						m_cur_live.state = L_MFM_SEARCH_DATA;
					}
				} else {
					checkpoint();
					m_error |= 0x40;
					live_abort();
					m_state = S_IDLE;
					return;
				}
				checkpoint();
				break;
			}
			break;

		case L_MFM_SEARCH_DATA:
			if(read_one_bit(limit))
				return;

			if((m_cur_live.bit_counter & 1) != (m_cur_live.shift_reg & 1)) {
				if(m_cur_live.bit_counter < 64) {
					m_cur_live.bit_counter = 0;
					m_cur_live.data_separator_phase = false;
				} else {
					m_cur_live.bit_counter = 1;
					m_cur_live.state = L_MFM_SCAN_DATA;
					break;
				}
			}
			if(!(m_cur_live.bit_counter & 1))
				m_cur_live.crc = 0xffff;

			//          logerror("pre counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
			break;

		case L_MFM_SCAN_DATA:
			if(read_one_bit(limit))
				return;
			if((m_cur_live.bit_counter == 16 || m_cur_live.bit_counter == 32 || m_cur_live.bit_counter == 48) && m_cur_live.shift_reg != 0x4489) {
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_MFM_SEARCH_DATA;
				break;
			}

			if(m_cur_live.bit_counter == 64 && m_cur_live.shift_reg != 0x5545) { // fb
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_MFM_SEARCH_DATA;
				break;
			}

			if(m_cur_live.bit_counter == 64) {
				m_cur_live.bit_counter = 0;
				live_delay(L_MFM_READ_DATA);
				return;
			}

			//          logerror("post counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
			break;

		case L_MFM_READ_DATA:
			if(!m_cur_live.bit_counter)
				m_cur_sector &= 0x7f;

			if(read_one_bit(limit))
				return;

			if(!(m_cur_live.bit_counter & 0xf)) {
				live_delay(L_MFM_READ_DATA_BYTE);
				return;
			}
			break;

		case L_MFM_READ_DATA_BYTE:
			m_cur_live.state = L_MFM_READ_DATA;

			if(m_cur_live.bit_counter <= 512*16) {
				fifo_push(m_cur_live.data_reg);
				if(m_error)
					return;
			}

			if(m_cur_live.bit_counter == 514*16) {
				if(m_cur_live.crc == 0) {
					logerror("sector read done, crc ok\n");
					m_xfer --;
					if(m_xfer == 0) {
						m_irq |= 0x08;
						m_mode &= ~8;
						live_abort();
						m_state = S_IDLE;
						update_irq();
						return;
					}

					m_cur_live.state = L_MFM_SEARCH_DATA;

				} else {
					checkpoint();
					m_error |= 0x80;
					m_mode &= ~8;
					live_abort();
					m_state = S_IDLE;
					return;
				}
				checkpoint();
				break;
			}
			break;

		case L_GCR_SEARCH_ID:
			logerror("gcr search id\n");
			exit(0);
		}
	}
}
