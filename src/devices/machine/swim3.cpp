// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    Implementation of the Apple SWIM3 floppy disk controller
    Emulation by Olivier Galibert, writing and GCR support by R. Belmont

    The copy protection read/write commands are not tested due to no
	known users of them.

*********************************************************************/

#include "emu.h"
#include "swim3.h"

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SWIM3, swim3_device, "swim3", "Apple SWIM3 (Sander/Wozniak Integrated Machine) version 3 floppy controller")

const u8 swim3_device::gcr6fw_tb[0x40] =
{
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

swim3_device::swim3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM3, tag, owner, clock),
	m_irq_cb(*this),
	m_drq_cb(*this)
{
}

void swim3_device::device_start()
{
	applefdintf_device::device_start();

	m_timer = timer_alloc(FUNC(swim3_device::update), this);

	// Build the Woz 6-and-2 decoder table
	std::fill(std::begin(m_gcr_bw), std::end(m_gcr_bw), 0);
	std::fill(std::begin(m_gcr_valid), std::end(m_gcr_valid), false);
	for(int i = 0; i != 0x40; i++) {
		m_gcr_bw[gcr6fw_tb[i]] = i;
		m_gcr_valid[gcr6fw_tb[i]] = true;
	}

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
	save_item(NAME(m_wr_esc_off));
	save_item(NAME(m_wr_crc_left));
	save_item(NAME(m_wr_mark_pending));
	save_item(NAME(m_wr_format));
	save_item(NAME(m_wr_gcr));

	save_item(NAME(m_cur_live.tm));
	save_item(NAME(m_cur_live.state));
	save_item(NAME(m_cur_live.next_state));
	save_item(NAME(m_cur_live.shift_reg));
	save_item(NAME(m_cur_live.crc));
	save_item(NAME(m_cur_live.bit_counter));
	save_item(NAME(m_cur_live.data_separator_phase));
	save_item(NAME(m_cur_live.data_bit_context));
	save_item(NAME(m_cur_live.no_crc));
	save_item(NAME(m_cur_live.data_reg));
	save_item(NAME(m_cur_live.idbuf));

	save_item(NAME(m_checkpoint_live.tm));
	save_item(NAME(m_checkpoint_live.state));
	save_item(NAME(m_checkpoint_live.next_state));
	save_item(NAME(m_checkpoint_live.shift_reg));
	save_item(NAME(m_checkpoint_live.crc));
	save_item(NAME(m_checkpoint_live.bit_counter));
	save_item(NAME(m_checkpoint_live.data_separator_phase));
	save_item(NAME(m_checkpoint_live.data_bit_context));
	save_item(NAME(m_checkpoint_live.no_crc));
	save_item(NAME(m_checkpoint_live.data_reg));
	save_item(NAME(m_checkpoint_live.idbuf));
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

	m_wr_esc_off = 0;
	m_wr_crc_left = 0;
	m_wr_mark_pending = false;
	m_wr_format = false;
	m_wr_gcr = false;

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

	LOG("floppy %s\n", floppy ? floppy->tag() : "-");

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
	LOG("mode%s%s hdsel=%c %c%s %c%c%s\n",
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
	case 0x0: // data
		return dma_r();

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
			h |= 0x0c;                       // rddata + sense
		if(m_irq & m_imask)
			h |= 0x02;                       // interrupt pending
		if(m_mode & 0x06)
			h |= 0x10;                       // a drive is enabled
		if(m_error)
			h |= 0x20;                       // error register is non-zero
		if(m_mode & 0x10) {
			if(!m_fifo_pos)
				h |= m_error ? 0x40 : 0xc0;
			else if(m_fifo_pos == 1 && !m_error)
				h |= 0x80;
		} else {
			if(m_fifo_pos == 2)
				h |= 0xc0;
			else if(m_fifo_pos == 1)
				h |= 0x80;
		}
		LOG("hand %02x\n", h);
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
		LOG("read %s\n", names[offset & 15]);
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
	case 0x0: // data
		dma_w(data);
		break;

	case 0x3: // param
		m_param = data;
		LOG("precompensation late=%x early=%x\n", m_param >> 4, m_param & 0xf);
		break;

	case 0x4: { // phases
		m_phases = data | 0xf0;
		update_phases();
		break;
	}

	case 0x5: // setup
		m_setup = data;
		m_sel35_cb((m_setup >> 1) & 1);
		LOG("setup write=%s %s nogcrconv=%s %s %s%s %s\n",
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
		LOG("load step %02x\n", data);
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
		LOG("imask%s%s%s%s%s\n",
				 m_imask & 0x10 ? " sense" : " -",
				 m_imask & 0x08 ? " sector" : " -",
				 m_imask & 0x04 ? " id" : " -",
				 m_imask & 0x02 ? " step" : " -",
				 m_imask & 0x01 ? " timer" : " -");
		update_irq();
		break;

	default:
		LOG("write %s, %02x\n", names[offset], data);
		break;
	}

	if((m_mode ^ prev_mode) & 0x86)
		m_devsel_cb((m_mode >> 1) & 3);
	if((m_mode ^ prev_mode) & 0x20)
		m_hdsel_cb((m_mode >> 5) & 1);

	if((m_mode & 0x80) && m_state == S_IDLE) {
		LOG("%s stepping start count=%02x floppy=%s\n", machine().time().to_string(), m_step, m_floppy ? m_floppy->tag() : "-");
		m_state = S_STEP;
		run(false, false);
	}

	if((m_mode & 0x48) == 0x48 && m_state == S_IDLE) {
		LOG("%s format start %s %s floppy=%s\n", machine().time().to_string(), m_setup & 0x40 ? "gcr" : "mfm", m_setup & 0x08 ? "fclk/2" : "fclk", m_floppy ? m_floppy->tag() : "-");
		m_state = S_FORMAT_WAIT_INDEX;
		wr_start(true);
		run(false, false);

	} else if((m_mode & 0x08) && m_state == S_IDLE) {
		LOG("%s %s start %s %s s1=%02x xfer=%d floppy=%s\n", machine().time().to_string(), m_mode & 0x10 ? "write" : "read", m_setup & 0x04 ? "gcr" : "mfm", m_setup & 0x08 ? "fclk/2" : "fclk", m_sect1, m_xfer, m_floppy ? m_floppy->tag() : "-");
		m_state = S_ID;
		if(m_mode & 0x10)
			wr_start(false);
		else {
			m_drq_write = false;
			m_fifo_pos = 0;
			update_drq();
		}
		run(false, false);
	}

	if(!(m_mode & 0x88) && m_state != S_IDLE) {
		LOG("%s transfer/step stopped\n", machine().time().to_string());
		m_drq_write = false;
		m_cur_sector &= 0x7f;
		live_abort();
		m_state = S_IDLE;
		m_fifo_pos = 0;
		update_drq();
	}
}

void swim3_device::update_irq()
{
	LOG("irq %02x m %02x -> %02x\n", m_irq, m_imask, m_irq & m_imask);
	m_irq_cb((m_irq & m_imask) != 0);
}

void swim3_device::update_drq()
{
	if(m_drq_write)
		m_drq_cb(m_fifo_pos != 2);
	else
		m_drq_cb(m_fifo_pos != 0);
}

void swim3_device::set_error(u8 error)
{
	// The ERS says only one error can be latched, so the first one wins
	if(!m_error)
		m_error = error;
}

void swim3_device::xfer_done()
{
	if(m_state == S_IDLE)
		return;
	m_irq |= m_error ? 0x28 : 0x08;
	m_mode &= ~0x08;
	m_state = S_IDLE;
	m_drq_write = false;
	m_cur_sector &= 0x7f;
	live_abort();

	// Since the transfer's done, flush the FIFO
	m_fifo_pos = 0;
	update_drq();
	update_irq();
}

bool swim3_device::sector_matches() const
{
	return !(m_sect1 & 0x80) && ((m_sect1 & 0x40) || (m_sect1 & 0x3f) == (m_cur_sector & 0x3f));
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

		case S_FORMAT_WAIT_INDEX:
			if(!index)
				return;
			LOG("%s format index seen\n", machine().time().to_string());
			m_state = S_FORMAT;
			live_start(L_WRITE_START);
			return;

		case S_FORMAT:
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
				LOG("%s: Rolling back and replaying (%s)\n", machine().time().to_string(), m_cur_live.tm.to_string());
			rollback();
			live_run(machine().time());
			m_cur_live.pll.commit(m_floppy, m_cur_live.tm);
		} else {
			if(0)
				LOG("%s: Committing (%s)\n", machine().time().to_string(), m_cur_live.tm.to_string());
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

bool swim3_device::read_one_bit_gcr(const attotime &limit, bool &byte_ready)
{
	int bit = m_cur_live.pll.get_next_bit(m_cur_live.tm, m_floppy, limit);
	if(bit < 0)
		return true;
	m_cur_live.shift_reg = ((m_cur_live.shift_reg << 1) | bit) & 0xff;
	byte_ready = m_cur_live.shift_reg & 0x80;
	if(byte_ready) {
		m_cur_live.data_reg = m_cur_live.shift_reg;
		m_cur_live.shift_reg = 0;
	}
	return false;
}

bool swim3_device::write_one_bit(const attotime &limit)
{
	bool bit = m_cur_live.shift_reg & 0x8000;
	if(m_cur_live.pll.write_next_bit(bit, m_cur_live.tm, m_floppy, limit))
		return true;
	if((m_cur_live.bit_counter & 1) && !m_cur_live.no_crc) {
		if((m_cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			m_cur_live.crc = (m_cur_live.crc << 1) ^ 0x1021;
		else
			m_cur_live.crc = m_cur_live.crc << 1;
	}
	m_cur_live.shift_reg = m_cur_live.shift_reg << 1;
	m_cur_live.bit_counter--;
	return false;
}

void swim3_device::live_write_raw(u16 raw)
{
	m_cur_live.shift_reg = raw;
	m_cur_live.data_bit_context = raw & 1;
}

void swim3_device::live_write_mfm(u8 mfm)
{
	bool context = m_cur_live.data_bit_context;
	u16 raw = 0;
	for(int i=0; i<8; i++) {
		bool bit = mfm & (0x80 >> i);
		if(!(bit || context))
			raw |= 0x8000 >> (2*i);
		if(bit)
			raw |= 0x4000 >> (2*i);
		context = bit;
	}
	m_cur_live.data_reg = mfm;
	m_cur_live.shift_reg = raw;
	m_cur_live.data_bit_context = context;
}

void swim3_device::live_write_gcr(u8 gcr)
{
	u8 raw = (m_setup & 0x10) ? gcr : gcr_encode(gcr);
	m_cur_live.data_reg = gcr;
	m_cur_live.shift_reg = raw << 8;
	m_cur_live.data_bit_context = raw & 1;
}

u8 swim3_device::gcr_encode(u8 data) const
{
	if(!(data & 0x80))
		return gcr6fw_tb[data & 0x3f];

	return m_gcr_valid[data] ? (data & 0x7f) : data;
}

u8 swim3_device::gcr_decode(u8 data) const
{
	if(m_setup & 0x10)
		return data;

	return m_gcr_valid[data] ? m_gcr_bw[data] : data;
}

attotime swim3_device::cell_period(int index) const
{
	static const int cycles_per_cell[4] = { 16, 31, 31, 63 };
	return attotime::from_ticks(cycles_per_cell[index & 3], clock());
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
	m_cur_live.data_bit_context = false;
	m_cur_live.no_crc = false;
	m_cur_live.data_reg = 0;

	const attotime period = cell_period(start_writing ? (((m_setup >> 6) & 1) | ((m_setup >> 2) & 2)) : ((m_setup >> 2) & 3));
	m_cur_live.pll.reset(m_cur_live.tm);
	m_cur_live.pll.set_clock(period);
	if(start_writing)
		m_cur_live.pll.start_writing(machine().time(), m_floppy);

	LOG("PLL %s clock %s\n", start_writing ? "write" : "read", period.to_string());

	m_checkpoint_live = m_cur_live;

	live_run();
}

bool swim3_device::fifo_push(u8 data, u8 error)
{
	if(m_fifo_pos == 2) {
		set_error(error);
		xfer_done();
		return false;
	}
	m_fifo[m_fifo_pos++] = data;
	update_drq();
	return true;
}

bool swim3_device::fifo_pop(u8 &data, u8 error)
{
	if(!m_fifo_pos) {
		set_error(error);
		xfer_done();
		return false;
	}
	data = m_fifo[0];
	m_fifo[0] = m_fifo[1];
	m_fifo_pos--;
	update_drq();
	return true;
}

u8 swim3_device::dma_r()
{
	u8 data = 0;
	fifo_pop(data, 0x04);
	return data;
}

void swim3_device::dma_w(u8 data)
{
	fifo_push(data, 0x04);
}

void swim3_device::wr_start(bool format)
{
	m_wr_format = format;
	m_wr_gcr = (m_setup & 0x40) != 0;
	m_wr_esc_off = 0;
	m_wr_crc_left = 0;
	m_wr_mark_pending = false;

	// Turn the fifo around and let it start filling
	m_fifo_pos = 0;
	m_drq_write = true;
	update_drq();
}

void swim3_device::wr_emit_data(u8 data)
{
	if(m_wr_mark_pending) {
		m_cur_live.crc = 0xcdb4;
		m_wr_mark_pending = false;
	}

	m_cur_live.no_crc = m_wr_gcr;
	if(m_wr_gcr)
		live_write_gcr(data);
	else
		live_write_mfm(data);
}

void swim3_device::wr_emit_mark(u16 raw)
{
	live_write_raw(raw);
	m_cur_live.no_crc = true;
	m_wr_mark_pending = true;
}

bool swim3_device::wr_next_byte()
{
	if(m_wr_crc_left) {
		// Second crc byte: pushing the high byte through the generator promotes
		// the low one into its place, so both are written as crc >> 8.
		m_wr_crc_left--;
		live_write_mfm(m_cur_live.crc >> 8);
		return true;
	}

	for(;;) {
		u8 data;
		if(!fifo_pop(data, 0x01)) {
			LOG("%s write underrun\n", machine().time().to_string());
			return false;
		}

		if(m_wr_esc_off) {
			m_wr_esc_off--;
			wr_emit_data(data);
			return true;
		}

		if(data != 0x99) {
			wr_emit_data(data);
			return true;
		}

		if(!fifo_pop(data, 0x01)) {
			LOG("%s write underrun in escape\n", machine().time().to_string());
			return false;
		}

		switch(data) {
		case 0x04: // write both crc bytes
			m_wr_crc_left = 1;
			m_cur_live.no_crc = false;
			live_write_mfm(m_cur_live.crc >> 8);
			return true;

		case 0x08: // end of data, terminate the transfer
			wr_finish();
			return false;

		case 0x0f: // turn escaping off for the next 512 bytes
			m_wr_esc_off = 512;
			break;

		case 0xa1: // $a1 mark byte, raw 0x4489
			wr_emit_mark(0x4489);
			return true;

		case 0xc2: // $c2 mark byte, raw 0x5224
			wr_emit_mark(0x5224);
			return true;

		default:
			wr_emit_data(data);
			return true;
		}
	}
}

void swim3_device::wr_finish()
{
	m_cur_live.pll.stop_writing(m_floppy, m_cur_live.tm);

	if(m_wr_format || m_error) {
		xfer_done();
		return;
	}

	if(m_xfer)
		m_xfer--;
	if(!m_xfer) {
		xfer_done();
		return;
	}

	m_cur_live.pll.set_clock(cell_period((m_setup >> 2) & 3));
	m_cur_live.pll.read_reset(m_cur_live.tm);
	m_cur_live.shift_reg = 0;
	m_cur_live.bit_counter = 0;
	m_cur_live.data_separator_phase = false;
	m_cur_live.crc = 0xffff;
	m_cur_live.no_crc = false;
	m_cur_live.state = m_setup & 0x04 ? L_GCR_SEARCH_ID : L_MFM_SEARCH_ID;
	checkpoint();
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

			//          LOG("pre counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
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

			//          LOG("post counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
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
					LOG("sector id found, track=%02d head=%d sector=%02d\n", m_cur_track & 0x7f, m_cur_track >> 7, m_cur_sector & 0x3f);
					m_cur_sector |= 0x80;
					m_irq |= 0x04;
					update_irq();
					if(sector_matches()) {
						if(m_mode & 0x10) {
							LOG("sector matches, writing data\n");
							m_cur_live.state = L_MFM_SKIP_GAP;
						} else {
							LOG("sector matches, reading data\n");
							m_cur_live.state = L_MFM_SEARCH_DATA;
						}
					}
				} else {
					checkpoint();
					set_error(0x40);
					xfer_done();
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

			//          LOG("pre counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
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

			//          LOG("post counter %02d sr %04x data %02x crc %04x\n", m_cur_live.bit_counter, m_cur_live.shift_reg, m_cur_live.data_reg, m_cur_live.crc);
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
				if(!fifo_push(m_cur_live.data_reg, 0x01))
					return;
			}

			if(m_cur_live.bit_counter == 514*16) {
				if(m_cur_live.crc == 0) {
					LOG("sector read done, crc ok\n");
					m_xfer --;
					if(m_xfer == 0) {
						xfer_done();
						return;
					}

					m_cur_live.state = L_MFM_SEARCH_DATA;

				} else {
					checkpoint();
					set_error(0x80);
					xfer_done();
					return;
				}
				checkpoint();
				break;
			}
			break;

		// The chip opens the write gate itself, part way into the gap that
		// follows the id field.  Per the Mac OS driver, "12 of 22 gap bytes
		// after address mark" before handing the remaining 10 to the byte
		// pump, so the hardware accounts for exactly 12 byte times.
		case L_MFM_SKIP_GAP:
			m_cur_live.bit_counter = 0;
			m_cur_live.state = L_MFM_SKIP_GAP_BYTE;
			checkpoint();
			break;

		case L_MFM_SKIP_GAP_BYTE:
			if(read_one_bit(limit))
				return;
			if(m_cur_live.bit_counter != 12*16)
				break;
			m_cur_live.bit_counter = 0;
			live_delay(L_WRITE_START);
			return;

		case L_GCR_SKIP_GAP: {
			bool byte_ready;
			if(read_one_bit_gcr(limit, byte_ready))
				return;
			if(!byte_ready || ++m_cur_live.bit_counter != 3)
				break;
			m_cur_live.bit_counter = 0;
			live_delay(L_WRITE_START);
			return;
		}

		case L_WRITE_START:
			LOG("%s write gate open\n", m_cur_live.tm.to_string());
			m_cur_sector &= 0x7f;
			m_cur_live.pll.set_clock(cell_period(((m_setup >> 6) & 1) | ((m_setup >> 2) & 2)));
			m_cur_live.pll.start_writing(m_cur_live.tm, m_floppy);
			if(m_floppy)
				m_floppy->set_write_splice(m_cur_live.tm);
			m_cur_live.data_bit_context = false;   // the preceding $4e gap byte ends in a 0
			m_cur_live.crc = 0xffff;
			m_cur_live.no_crc = false;
			m_cur_live.state = L_WRITE;
			break;

		// Entered only through live_delay(), so that the fifo pops and the
		// escape state changes below are never replayed by rollback().
		case L_WRITE:
			if(!wr_next_byte()) {
				if(m_cur_live.state == L_IDLE)
					return;
				break;
			}
			m_cur_live.state = L_WRITE_BYTE;
			m_cur_live.bit_counter = m_wr_gcr ? 8 : 16;
			checkpoint();
			break;

		case L_WRITE_BYTE:
			if(write_one_bit(limit))
				return;
			if(m_cur_live.bit_counter)
				break;
			live_delay(L_WRITE);
			return;

		case L_GCR_SEARCH_ID: {
			bool byte_ready;
			if(read_one_bit_gcr(limit, byte_ready))
				return;
			if(!byte_ready)
				break;
			if(m_setup & 0x02) {
				if(!m_gcr_valid[m_cur_live.data_reg]) {
					live_delay(L_GCR_COPY_PROTECT_BYTE);
					return;
				}
				break;
			}

			switch(m_cur_live.data_reg) {
			case 0xd5:
				m_cur_live.bit_counter = 1;
				break;
			case 0xaa:
				m_cur_live.bit_counter = m_cur_live.bit_counter == 1 ? 2 : 0;
				break;
			case 0x96:
				if(m_cur_live.bit_counter == 2) {
					m_cur_sector &= 0x7f;
					m_cur_live.state = L_GCR_READ_ID;
				}
				m_cur_live.bit_counter = 0;
				break;
			default:
				m_cur_live.bit_counter = 0;
				break;
			}
			break;
		}

		case L_GCR_COPY_PROTECT: {
			bool byte_ready;
			if(read_one_bit_gcr(limit, byte_ready))
				return;
			if(!byte_ready)
				break;
			live_delay(L_GCR_COPY_PROTECT_BYTE);
			return;
		}

		case L_GCR_COPY_PROTECT_BYTE:
			m_cur_live.state = L_GCR_COPY_PROTECT;
			if(!fifo_push(m_gcr_valid[m_cur_live.data_reg] ? 0x00 : 0x80, 0x01))
				return;
			if(!fifo_push(gcr_decode(m_cur_live.data_reg), 0x01))
				return;
			checkpoint();
			break;

		case L_GCR_READ_ID: {
			bool byte_ready;
			if(read_one_bit_gcr(limit, byte_ready))
				return;
			if(!byte_ready)
				break;
			live_delay(L_GCR_READ_ID_BYTE);
			return;
		}

		case L_GCR_READ_ID_BYTE: {
			m_cur_live.state = L_GCR_READ_ID;
			m_cur_live.idbuf[m_cur_live.bit_counter++] = gcr_decode(m_cur_live.data_reg);
			if(m_cur_live.bit_counter != 5) {
				checkpoint();
				break;
			}

			m_cur_live.bit_counter = 0;
			m_cur_live.state = L_GCR_SEARCH_ID;

			const u8 *const id = m_cur_live.idbuf;
			if(((id[0] ^ id[1] ^ id[2] ^ id[3]) & 0x3f) != (id[4] & 0x3f)) {
				checkpoint();
				set_error(0x40);
				xfer_done();
				return;
			}

			m_cur_track = (id[0] & 0x3f) | ((id[2] & 0x01) << 6) | ((id[2] & 0x20) ? 0x80 : 0);
			m_cur_sector = (id[1] & 0x3f) | 0x80;
			m_cur_format = id[3];
			LOG("gcr sector id found, track=%02d head=%d sector=%02d format=%02x\n", m_cur_track & 0x7f, m_cur_track >> 7, m_cur_sector & 0x3f, m_cur_format);
			m_irq |= 0x04;
			update_irq();
			if(sector_matches()) {
				if(m_mode & 0x10) {
					LOG("sector matches, writing data\n");
					m_cur_live.state = L_GCR_SKIP_GAP;
				} else {
					LOG("sector matches, reading data\n");
					m_cur_live.state = L_GCR_SEARCH_DATA;
				}
			}
			checkpoint();
			break;
		}

		case L_GCR_SEARCH_DATA: {
			bool byte_ready;
			if(read_one_bit_gcr(limit, byte_ready))
				return;
			if(!byte_ready)
				break;
			switch(m_cur_live.data_reg) {
			case 0xd5:
				m_cur_live.bit_counter = 1;
				break;
			case 0xaa:
				m_cur_live.bit_counter = m_cur_live.bit_counter == 1 ? 2 : 0;
				break;
			case 0xad:
				if(m_cur_live.bit_counter == 2) {
					m_cur_sector &= 0x7f;
					m_cur_live.state = L_GCR_READ_DATA;
				}
				m_cur_live.bit_counter = 0;
				break;
			default:
				m_cur_live.bit_counter = 0;
				break;
			}
			break;
		}

		case L_GCR_READ_DATA: {
			bool byte_ready;
			if(read_one_bit_gcr(limit, byte_ready))
				return;
			if(!byte_ready)
				break;
			live_delay(L_GCR_READ_DATA_BYTE);
			return;
		}

		case L_GCR_READ_DATA_BYTE:
			m_cur_live.state = L_GCR_READ_DATA;
			if(!fifo_push(gcr_decode(m_cur_live.data_reg), 0x01))
				return;

			if(++m_cur_live.bit_counter == 704) {
				LOG("gcr sector read done\n");
				m_xfer --;
				if(m_xfer == 0) {
					xfer_done();
					return;
				}
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_GCR_SEARCH_DATA;
			}
			checkpoint();
			break;
		}
	}
}
