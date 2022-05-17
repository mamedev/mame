// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Not implemented for lack of test cases: FFR

#include "emu.h"
#include "mc6843.h"

DEFINE_DEVICE_TYPE(MC6843, mc6843_device, "mc6843", "Motorola MC6843 FDC")

mc6843_device::mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MC6843, tag, owner, clock),
	  m_irq(*this),
	  m_force_ready(false)
{
}

void mc6843_device::device_start()
{
	m_irq.resolve_safe();
	m_timer = timer_alloc(FUNC(mc6843_device::update_tick), this);
	m_floppy = nullptr;

	m_dir  = 0x00;
	m_dor  = 0x00;
	m_ctar = 0x00;
	m_sur  = 0x00;
	m_cmr  = 0x00;
	m_gcr  = 0x00;
	m_ccr  = 0x00;
	m_ltar = 0x00;

	save_item(NAME(m_dir));
	save_item(NAME(m_dor));
	save_item(NAME(m_ctar));
	save_item(NAME(m_cmr));
	save_item(NAME(m_isr));
	save_item(NAME(m_sur));
	save_item(NAME(m_stra));
	save_item(NAME(m_sar));
	save_item(NAME(m_strb));
	save_item(NAME(m_gcr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_ltar));
	save_item(NAME(m_state));
	save_item(NAME(m_step_count));
	save_item(NAME(m_head_loaded));
	save_item(NAME(m_dir_loaded));
	save_item(NAME(m_dor_loaded));
	save_item(NAME(m_dor_needed));
	save_item(NAME(m_idam_turns));
}

void mc6843_device::device_reset()
{
	m_state = S_IDLE;
	m_cur_live.state = L_IDLE;
	m_cur_live.tm = attotime::never;

	m_step_count = 0;
	m_head_loaded = false;

	m_dir_loaded = false;
	m_dor_loaded = false;
	m_dor_needed = false;

	m_idam_turns = 0;

	m_cmr &= 0xf0;
	m_isr  = 0x00;
	m_stra = 0x00;
	m_sar  = 0x00;
	m_strb = 0x00;
}

void mc6843_device::map(address_map &map)
{
	map(0, 0).rw(FUNC(mc6843_device::dir_r),  FUNC(mc6843_device::dor_w));
	map(1, 1).rw(FUNC(mc6843_device::ctar_r), FUNC(mc6843_device::ctar_w));
	map(2, 2).rw(FUNC(mc6843_device::isr_r),  FUNC(mc6843_device::cmr_w));
	map(3, 3).rw(FUNC(mc6843_device::stra_r), FUNC(mc6843_device::sur_w));
	map(4, 4).rw(FUNC(mc6843_device::strb_r), FUNC(mc6843_device::sar_w));
	map(5, 5).w (FUNC(mc6843_device::gcr_w));
	map(6, 6).w (FUNC(mc6843_device::ccr_w));
	map(7, 7).w (FUNC(mc6843_device::ltar_w));
}

void mc6843_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	int prev_ready = m_floppy ? m_floppy->ready_r() : 1;

	if(m_floppy) {
		m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		m_floppy->setup_ready_cb(floppy_image_device::ready_cb());
	}

	m_floppy = floppy;

	logerror("floppy %s\n", m_floppy ? m_floppy->tag() : "-");

	int next_ready = m_floppy ? m_floppy->ready_r() : 1;

	if(m_floppy) {
		m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&mc6843_device::index_callback, this));
		m_floppy->setup_ready_cb(floppy_image_device::ready_cb(&mc6843_device::ready_callback, this));
	}

	if(prev_ready != next_ready)
		ready_callback(m_floppy, next_ready);
}

u8 mc6843_device::dir_r()
{
	if(!machine().side_effects_disabled()) {
		m_dir_loaded = false;
		logerror("dir_r %02x\n", m_dir);
	}
	return m_dir;
}

void mc6843_device::dor_w(u8 data)
{
	m_dor = data;
	m_dor_loaded = true;
	logerror("dor_w %02x\n", m_dor);
}

u8 mc6843_device::ctar_r()
{
	if(!machine().side_effects_disabled())
		logerror("ctar_r %02x\n", m_ctar);
	return m_ctar;
}

void mc6843_device::ctar_w(u8 data)
{
	m_ctar = data;
	logerror("ctar_w %02x\n", m_ctar);
}

void mc6843_device::isr_raise(u8 flag)
{
	m_isr |= flag;
}

u8 mc6843_device::isr_r()
{
	u8 res = m_isr;
	if(!machine().side_effects_disabled()) {
		logerror("isr_r %02x\n", m_isr);
		m_isr &= 0x8;
	}
	return res;
}

void mc6843_device::cmr_w(u8 data)
{
	static const char *cmds[0x10] = {
		"nop", "?1",
		"stz", "sek", "ssr", "ssw", "rcr", "swd",
		"?8", "?9",
		"ffr", "ffw", "msr", "msw",
		"?e", "?f"
	};

	if(m_stra & SA_BUSY) {
		if((data & 0xf) == 0 && (m_cmr & 0xf) == C_FFW) {
			m_stra &= ~SA_BUSY;
			live_abort();
			m_state = S_IDLE;
		} else {
			logerror("cmr_w %02x - dropped busy\n", data);
			return;
		}
	}

	m_cmr = data;
	logerror("cmr_w %02x - isr=%s isr3=%s %s fwf=%d %s\n", m_cmr,
			 m_cmr & 0x80 ? "off" : "on",
			 m_cmr & 0x40 ? "off" : "on",
			 m_cmr & 0x20 ? "dma" : "pio",
			 m_cmr & 0x10 ? 1 : 0,
			 cmds[m_cmr & 0xf]);
	command_start();
}

u8 mc6843_device::stra_r()
{
	u8 res = m_stra;
	if(m_floppy) {
		if(m_floppy->idx_r())
			res |= SA_IDX;
		if(m_floppy->wpt_r())
			res |= SA_WPT;
		if(!m_floppy->trk00_r())
			res |= SA_TRK0;
	}
	if(is_ready())
		res |= SA_RDY;

	switch(m_cmr & 0xf) {
	case C_SSR: case C_MSR: case C_FFR:
		if(m_dir_loaded)
			res |= SA_DTR;
		break;
	case C_SSW: case C_SWD: case C_MSW: case C_FFW:
		if(!m_dor_loaded && m_dor_needed)
			res |= SA_DTR;
		break;
	}

	if(!machine().side_effects_disabled()) {
		static int prev = -1;
		if(prev != res) {
			logerror("stra_r %02x -%s%s%s%s%s%s%s%s\n", res,
					 res & SA_BUSY ? " busy" : "",
					 res & SA_IDX  ? " idx" : "",
					 res & SA_TNEQ ? " tneq" : "",
					 res & SA_WPT  ? " wpt" : "",
					 res & SA_TRK0 ? " trk0" : "",
					 res & SA_RDY  ? " rdy" : "",
					 res & SA_DDM  ? " ddm" : "",
					 res & SA_DTR  ? " dtr" : "");
			prev = res;
		}
	}
	return res;
}

void mc6843_device::sur_w(u8 data)
{
	m_sur = data;
	logerror("sur_w %02x\n", m_sur);
}

u8 mc6843_device::strb_r()
{
	if(!machine().side_effects_disabled()) {
		logerror("strb_r %02x -%s%s%s%s%s%s%s%s\n", m_strb,
				 m_strb & SB_HERR  ? " herr" : "",
				 m_strb & SB_WERR  ? " werr" : "",
				 m_strb & SB_FI    ? " fi" : "",
				 m_strb & SB_SERR  ? " serr" : "",
				 m_strb & SB_SAERR ? " saerr" : "",
				 m_strb & SB_DMERR ? " dmerr" : "",
				 m_strb & SB_CRC   ? " crc" : "",
				 m_strb & SB_DTERR ? " dterr" : "");
	}
	return m_strb;
}

void mc6843_device::sar_w(u8 data)
{
	m_sar = data & 0x1f;
	logerror("sar_w %02x\n", m_sar);
}

void mc6843_device::gcr_w(u8 data)
{
	m_gcr = data & 0x7f;
	logerror("gcr_w %02x\n", m_gcr);
}

void mc6843_device::ccr_w(u8 data)
{
	m_ccr = data & 3;
	logerror("ccr_w %02x\n", m_ccr);
}

void mc6843_device::ltar_w(u8 data)
{
	m_ltar = data & 0x7f;
	logerror("ltar_w %02x\n", m_ltar);
}

void mc6843_device::index_callback(floppy_image_device *floppy, int state)
{
	if(state) {
		live_sync();
		logerror("idam %d\n", m_idam_turns);
		if(m_idam_turns) {
			m_idam_turns --;
			if(!m_idam_turns) {
				live_abort();
				m_state = S_IDAM_NOT_FOUND;
			}
		}
		run(false, false, true);
	}
}

void mc6843_device::ready_callback(floppy_image_device *floppy, int state)
{
	if(state) {
		live_sync();
		run(false, true, false);
	}
}

TIMER_CALLBACK_MEMBER(mc6843_device::update_tick)
{
	live_sync();
	run(true, false, false);
}

void mc6843_device::command_start()
{
	assert(m_state == S_IDLE);
	switch(m_cmr & 0xf) {

	case C_STZ:
		logerror("Seek to track 0\n");
		if(m_floppy)
			m_floppy->dir_w(1);
		m_stra |= SA_BUSY;
		m_step_count = 82;
		m_state = S_STZ_STEP;
		break;

	case C_SEK:
		logerror("Seek from track %d to %d\n", m_ctar, m_gcr);
		if(m_gcr > m_ctar) {
			if(m_floppy)
				m_floppy->dir_w(0);
			m_step_count = m_gcr - m_ctar;
		} else {
			if(m_floppy)
				m_floppy->dir_w(1);
			m_step_count = m_ctar - m_gcr;
		}
		m_stra |= SA_BUSY;
		if(!m_step_count) {
			m_state = S_SEEK_HEAD_SETTLING;
			delay(4096 * (m_sur & 15));
		} else
			m_state = S_SEEK_STEP;
		break;

	case C_SSR: case C_RCR: case C_SSW: case C_SWD: case C_MSR: case C_MSW:
		m_stra |= SA_BUSY;
		m_stra &= ~(SA_DDM|SA_TNEQ);
		m_state = S_SRW_WAIT_READY;
		break;

	case C_FFW:
		m_stra |= SA_BUSY;
		m_state = S_FFW_WAIT_READY;
		m_dor_needed = true;
		break;

	case 0x0:
	case 0x1:
	case 0x8:
	case 0x9:
	case 0xe:
	case 0xf:
		break;

	default:
		fatalerror("Unsupported command\n");
	}

	run(false, false, false);
}

void mc6843_device::delay(int cycles)
{
	// A delay of zero freezes the fdc, it's expected
	if(cycles) {
		m_timer->adjust(attotime::from_ticks(cycles, clock()));
	}
}

bool mc6843_device::is_ready() const
{
	return m_force_ready || (m_floppy && !m_floppy->ready_r());
}


void mc6843_device::run(bool timeout, bool ready, bool index)
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

		case S_STZ_STEP:
			if(m_floppy && !m_floppy->trk00_r()) {
				m_floppy->stp_w(0);
				m_floppy->stp_w(1);
			}
			m_step_count --;
			m_state = S_STZ_STEP_WAIT;
			delay(1024 * (m_sur >> 4));
			return;

		case S_STZ_STEP_WAIT:
			if(!timeout)
				return;
			if(m_step_count)
				m_state = S_STZ_STEP;
			else {
				m_ctar = m_gcr = 0;
				m_state = S_STZ_HEAD_SETTLING;
				delay(4096 * (m_sur & 15));
				return;
			}
			break;

		case S_STZ_HEAD_SETTLING:
			if(!timeout)
				return;
			m_head_loaded = true;
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_SCE);
			return;

		case S_SEEK_STEP:
			if(m_floppy) {
				m_floppy->stp_w(0);
				m_floppy->stp_w(1);
			}
			m_step_count --;
			m_state = S_SEEK_STEP_WAIT;
			delay(1024 * (m_sur >> 4));
			return;

		case S_SEEK_STEP_WAIT:
			if(!timeout)
				return;
			if(m_step_count)
				m_state = S_SEEK_STEP;
			else {
				m_ctar = m_gcr;
				m_state = S_SEEK_HEAD_SETTLING;
				delay(4096 * (m_sur & 15));
				return;
			}
			break;

		case S_SEEK_HEAD_SETTLING:
			if(!timeout)
				return;
			m_head_loaded = true;
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_SCE);
			return;

		case S_SRW_WAIT_READY:
			if(!is_ready())
				return;
			if(!m_head_loaded) {
				m_state = S_SRW_HEAD_SETTLING;
				delay(4096 * (m_sur & 15));
				return;
			} else
				m_state = S_SRW_START;
			break;

		case S_SRW_HEAD_SETTLING:
			if(timeout) {
				m_head_loaded = true;
				m_state = S_SRW_START;
			}
			break;

		case S_SRW_START:
			m_idam_turns = 3;
			live_start(L_IDAM_SEARCH);
			return;

		case S_FFW_WAIT_READY:
			if(!is_ready())
				return;
			if(!m_head_loaded) {
				m_state = S_FFW_HEAD_SETTLING;
				delay(4096 * (m_sur & 15));
				return;
			} else
				m_state = S_FFW_START;
			break;

		case S_FFW_HEAD_SETTLING:
			if(timeout) {
				m_head_loaded = true;
				m_state = S_FFW_START;
			}
			break;

		case S_FFW_START:
			live_start(L_FFW_BYTE);
			return;

		case S_IDAM_BAD_TRACK:
			m_dir = m_cur_live.data_reg;
			m_stra |= SA_TNEQ;
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_RWCE);
			return;

		case S_IDAM_BAD_CRC:
			m_strb |= SB_CRC | SB_SAERR;
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_RWCE | I_STRB);
			return;

		case S_IDAM_FOUND:
			if(m_cmr & 0x20)
				isr_raise(I_SSR);
			if((m_cmr & 0xf) == C_SSR || (m_cmr & 0xf) == C_MSR || (m_cmr & 0xf) == C_RCR)
				live_start(L_DAM_SEARCH);

			else {
				m_dor_needed = true;
				live_start(L_DAM_WAIT);
			}
			return;

		case S_IDAM_NOT_FOUND:
			m_stra &= ~SA_BUSY;
			m_strb |= SB_SAERR;
			m_state = S_IDLE;
			isr_raise(I_RWCE | I_STRB);
			logerror("not found\n");
			return;

		case S_DAM_NOT_FOUND:
			m_strb |= SB_DMERR;
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_RWCE | I_STRB);
			return;

		case S_DAM_BAD_CRC:
			m_strb |= SB_CRC;
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_RWCE | I_STRB);
			return;

		case S_DAM_DONE:
			m_dor_needed = false;
			if((m_cmr & 0xf) == C_MSR || (m_cmr & 0xf) == C_MSW) {
				m_sar = (m_sar + 1) & 0x1f;
				m_gcr = (m_gcr - 1) & 0x7f;
				if(m_gcr != 0x7f) {
					m_state = S_SRW_START;
					break;
				}
			}
			m_stra &= ~SA_BUSY;
			m_state = S_IDLE;
			isr_raise(I_RWCE);
			return;
		}
	}
}

void mc6843_device::live_start(int state, bool start_writing)
{
	m_cur_live.tm = machine().time();
	m_cur_live.state = state;
	m_cur_live.next_state = -1;
	m_cur_live.shift_reg = 0;
	m_cur_live.crc = 0xffff;
	m_cur_live.bit_counter = 0;
	m_cur_live.data_separator_phase = false;
	m_cur_live.data_reg = 0;

	m_cur_live.pll.reset(m_cur_live.tm);
	m_cur_live.pll.set_clock(attotime::from_ticks(2, clock()));
	if(start_writing)
		m_cur_live.pll.start_writing(machine().time());

	m_checkpoint_live = m_cur_live;

	live_run();
}

void mc6843_device::checkpoint()
{
	m_cur_live.pll.commit(m_floppy, m_cur_live.tm);
	m_checkpoint_live = m_cur_live;
}

void mc6843_device::rollback()
{
	m_cur_live = m_checkpoint_live;
}

void mc6843_device::live_delay(int state)
{
	m_cur_live.next_state = state;
	m_timer->adjust(m_cur_live.tm - machine().time());
}

void mc6843_device::live_sync()
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

void mc6843_device::live_abort()
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

bool mc6843_device::read_one_bit(const attotime &limit)
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

bool mc6843_device::write_one_bit(const attotime &limit)
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

void mc6843_device::live_run(attotime limit)
{
	if(m_cur_live.state == L_IDLE || m_cur_live.next_state != -1)
		return;

	if(limit == attotime::never) {
		if(m_floppy)
			limit = m_floppy->time_next_index();
		if(limit == attotime::never) {
			// Happens when there's no disk or if the wd is not
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
		case L_IDAM_SEARCH:
			if(read_one_bit(limit))
				return;

			if(0)
			logerror("%s: shift = %04x data=%02x c=%d\n", m_cur_live.tm.to_string(), m_cur_live.shift_reg,
					(m_cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(m_cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(m_cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(m_cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(m_cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(m_cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(m_cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(m_cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					m_cur_live.bit_counter);

			if(m_cur_live.shift_reg == 0xf57e) {
				m_cur_live.crc = 0xef21;
				m_cur_live.data_separator_phase = false;
				m_cur_live.bit_counter = 0;
				m_cur_live.state = L_IDAM_CHECK_TRACK;
			}
			break;

		case L_IDAM_CHECK_TRACK:
			if(read_one_bit(limit))
				return;

			if(m_cur_live.bit_counter == 16) {
				logerror("%s IDAM track %d\n", m_cur_live.tm.to_string(), m_cur_live.data_reg);
				if(m_cur_live.data_reg != m_ltar) {
					m_state = S_IDAM_BAD_TRACK;
					live_delay(L_IDLE);
					return;

				} else
					m_cur_live.state = L_IDAM_CHECK_SECTOR;
			}
			break;

		case L_IDAM_CHECK_SECTOR:
			if(read_one_bit(limit))
				return;

			if(m_cur_live.bit_counter == 48) {
				logerror("%s IDAM sector %d\n", m_cur_live.tm.to_string(), m_cur_live.data_reg);
				if(m_cur_live.data_reg != m_sar)
					m_cur_live.state = L_IDAM_SEARCH;
				else
					m_cur_live.state = L_IDAM_CHECK_CRC;
			}
			break;

		case L_IDAM_CHECK_CRC:
			if(read_one_bit(limit))
				return;

			if(m_cur_live.bit_counter == 96) {
				logerror("IDAM crc remainder %04x\n", m_cur_live.crc);
				if(m_cur_live.crc)
					m_state = S_IDAM_BAD_CRC;
				else
					m_state = S_IDAM_FOUND;
				live_delay(L_IDLE);
				return;
			}
			break;

		case L_DAM_SEARCH:
			if(read_one_bit(limit))
				return;

			if(m_cur_live.shift_reg == 0xf56a || m_cur_live.shift_reg == 0xf56f) {
				m_cur_live.data_separator_phase = false;
				m_cur_live.bit_counter = 0;
				logerror("DAM mark %02x\n", m_cur_live.data_reg);
				if(m_cur_live.shift_reg == 0xf56f) {
					logerror("DAM found\n");
					m_cur_live.crc = 0xbf84;
					m_cur_live.state = L_DAM_READ;
				} else {
					logerror("DDAM found\n");
					m_cur_live.crc = 0x8fe7;
					live_delay(L_DAM_DELETED);
					return;
				}
			}
			break;

		case L_DAM_DELETED:
			m_stra |= SA_DDM;
			m_cur_live.state = L_DAM_READ;
			break;

		case L_DAM_READ:
			if(read_one_bit(limit))
				return;

			if(!(m_cur_live.bit_counter & 0xf)) {
				live_delay(L_DAM_READ_BYTE);
				return;
			}
			break;

		case L_DAM_READ_BYTE: {
			int byte = m_cur_live.bit_counter >> 4;
			logerror("byte %02x = %02x crc %04x\n", byte, m_cur_live.data_reg, m_cur_live.crc);
			if(byte <= 128) {
				if((m_cmr & 0xf) != C_RCR) {
					if(m_dir_loaded)
						m_strb |= SB_DTERR;
					m_dir_loaded = true;
				}
				m_dir = m_cur_live.data_reg;
			} else if(byte == 130) {
				if(m_cur_live.crc)
					m_state = S_DAM_BAD_CRC;
				else
					m_state = S_DAM_DONE;
				m_cur_live.state = L_IDLE;
				return;
			}
			m_cur_live.state = L_DAM_READ;
			break;
		}

		case L_DAM_WAIT:
			if(read_one_bit(limit))
				return;

			if(m_cur_live.bit_counter == 11*16) {
				m_cur_live.bit_counter = (6+1+128+2)*16;
				live_delay(L_DAM_WRITE_BYTE);
				return;
			}
			break;

		case L_DAM_WRITE:
			if(write_one_bit(limit))
				return;
			if(!(m_cur_live.bit_counter & 0xf)) {
				live_delay(L_DAM_WRITE_BYTE);
				return;
			}
			break;

		case L_DAM_WRITE_BYTE: {
			int byte = (6+1+128+2) - (m_cur_live.bit_counter >> 4);
			if(!byte) {
				m_cur_live.pll.start_writing(m_cur_live.tm);
				m_cur_live.shift_reg = 0xaaaa;

			} else if(byte <= 5) {
				m_cur_live.shift_reg = 0xaaaa;

			} else if(byte <= 6) {
				m_cur_live.crc = 0xffff;
				if((m_cmr & 0xf) == C_SWD)
					m_cur_live.shift_reg = 0xf56a;
				else
					m_cur_live.shift_reg = 0xf56f;

			} else if(byte <= 134) {
				if(!m_dor_loaded)
					m_strb |= SB_DTERR;
				m_dor_loaded = false;
				if(byte == 128)
					m_dor_needed = false;
				m_cur_live.shift_reg = 0xaaaa |
					(m_dor & 0x80 ? 1<<14 : 0) |
					(m_dor & 0x40 ? 1<<12 : 0) |
					(m_dor & 0x20 ? 1<<10 : 0) |
					(m_dor & 0x10 ? 1<< 8 : 0) |
					(m_dor & 0x08 ? 1<< 6 : 0) |
					(m_dor & 0x04 ? 1<< 4 : 0) |
					(m_dor & 0x02 ? 1<< 2 : 0) |
					(m_dor & 0x01 ? 1     : 0);

			} else if(byte <= 136) {
				m_cur_live.shift_reg = 0xaaaa |
					(m_cur_live.crc & 0x8000 ? 1<<14 : 0) |
					(m_cur_live.crc & 0x4000 ? 1<<12 : 0) |
					(m_cur_live.crc & 0x2000 ? 1<<10 : 0) |
					(m_cur_live.crc & 0x1000 ? 1<< 8 : 0) |
					(m_cur_live.crc & 0x0800 ? 1<< 6 : 0) |
					(m_cur_live.crc & 0x0400 ? 1<< 4 : 0) |
					(m_cur_live.crc & 0x0200 ? 1<< 2 : 0) |
					(m_cur_live.crc & 0x0100 ? 1     : 0);

			} else if(byte <= 137) {
				m_cur_live.shift_reg = 0xffff;

			} else {
				m_state = S_DAM_DONE;
				m_cur_live.state = L_IDLE;
				m_cur_live.pll.stop_writing(m_floppy, m_cur_live.tm);
				return;
			}
			m_cur_live.state = L_DAM_WRITE;
			break;
		}

		case L_FFW_BYTE:
			if(!m_dor_loaded)
				m_strb |= SB_DTERR;
			m_dor_loaded = false;
			logerror("write %02x\n", m_dor);
			if(m_cmr & 0x10) {
				m_cur_live.shift_reg = m_dor << 8;
				m_cur_live.bit_counter = 8;

			} else {
				m_cur_live.shift_reg = 0xaaaa |
					(m_dor & 0x80 ? 1<<14 : 0) |
					(m_dor & 0x40 ? 1<<12 : 0) |
					(m_dor & 0x20 ? 1<<10 : 0) |
					(m_dor & 0x10 ? 1<< 8 : 0) |
					(m_dor & 0x08 ? 1<< 6 : 0) |
					(m_dor & 0x04 ? 1<< 4 : 0) |
					(m_dor & 0x02 ? 1<< 2 : 0) |
					(m_dor & 0x01 ? 1     : 0);
				m_cur_live.bit_counter = 16;
			}
			m_cur_live.state = L_FFW_WRITE;
			break;

		case L_FFW_WRITE:
			if(write_one_bit(limit))
				return;
			if(m_cur_live.bit_counter == 0) {
				live_delay(L_FFW_BYTE);
				return;
			}
			break;
		}
	}
}
