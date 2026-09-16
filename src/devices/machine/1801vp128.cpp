// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1801VP1-128 gate array (MFM codec for floppy controllers)

    https://github.com/1801BM1/k1801/tree/master/128
    https://felixl.com/UKNC_FDD_1801vp1-128
    https://zx-pk.ru/threads/20406-emulyatsiya-1801vp1-128-v-plis.html

    To do:
    - DRQ status in read and write modes is tracked separately (TR bit)
    - missing MFM clock is added by MFM encoder for every 00 sequence
    - deep internals of CRC, GDR bits; read/write mode switching
    - optional external timer for PY device

**********************************************************************/

#include "emu.h"
#include "1801vp128.h"

#define LOG_WARN    (1U << 1)   // Show warnings
#define LOG_SHIFT   (1U << 2)   // Shows shift register contents
#define LOG_REGS    (1U << 6)   // Register I/O
#define LOG_STATE   (1U << 11)  // State machine
#define LOG_LIVE    (1U << 12)  // Live states

//#define VERBOSE (LOG_GENERAL | LOG_REGS | LOG_STATE)
#include "logmacro.h"

#define LOGWARN(...)     LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGSHIFT(...)    LOGMASKED(LOG_SHIFT, __VA_ARGS__)
#define LOGREGS(...)     LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGLIVE(...)     LOGMASKED(LOG_LIVE, __VA_ARGS__)
#define LOGSTATE(...)    LOGMASKED(LOG_STATE, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(K1801VP128, k1801vp128_device, "1801vp1_128", "1801VP1-128 FDC")



inline k1801vp128_device::floppy_info::floppy_info()
	: tm(nullptr)
	, dev(nullptr)
	, id(0)
	, main_state(0)
	, sub_state(0)
	, dir(0)
	, counter(0)
	, live(false)
	, index(false)
{
}


inline k1801vp128_device::live_info::live_info()
	: tm(attotime::never)
	, state(IDLE)
	, next_state(-1)
	, fi(nullptr)
	, shift_reg(0)
	, crc(0)
	, bit_counter(0)
	, data_separator_phase(false)
	, data_bit_context(false)
	, crc_init(false)
	, data_reg(0)
	, pll()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k1801vp128_device - constructor
//-------------------------------------------------

k1801vp128_device::k1801vp128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1801VP128, tag, owner, clock)
	, m_connectors(*this, "%u", 0U)
	, m_read_ds(*this, -1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1801vp128_device::device_start()
{
	for (int i = 0; i != 4; i++)
	{
		flopi[i].tm = timer_alloc(FUNC(k1801vp128_device::update_floppy), this);
		flopi[i].id = i;
		if (m_connectors[i])
		{
			flopi[i].dev = m_connectors[i]->get_device();
			if (flopi[i].dev != nullptr)
				flopi[i].dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&k1801vp128_device::index_callback, this));
		}
		else
			flopi[i].dev = nullptr;

		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].live = false;
	}

	m_wbuf = m_rbuf = 0;

	// register for state saving
	save_item(NAME(selected_drive));
	save_item(NAME(m_cr));
	save_item(NAME(m_sr));
	save_item(NAME(m_rbuf));
	save_item(NAME(m_wbuf));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1801vp128_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].live = false;
	}
	live_abort();
	m_cr = m_sr = 0;
	set_ds(-1);
}

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vp128_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 1)
	{
	case 0:
		data = m_sr;
		if (selected_drive != -1)
		{
			floppy_info &fi = flopi[selected_drive];
			data = (fi.dev->trk00_r() ^ 1) | (fi.dev->ready_r() << 1) | (fi.dev->wpt_r() << 2) | (fi.dev->idx_r() << 15) | m_sr;
		}
		break;

	case 1:
		data = m_rbuf;
		if (!machine().side_effects_disabled())
		{
			m_sr &= ~CSR_R_TR;
			if (selected_drive != -1)
			{
				floppy_info &fi = flopi[selected_drive];
				if (fi.main_state == WRITE_DATA)
				{
					// semi-read mode
					live_abort();
					fi.main_state = READ_DATA;
					fi.sub_state = SCAN_ID;
					read_data_continue(fi);
				}
			}
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vp128_device::write(offs_t offset, uint16_t data)
{
	LOGREGS("%s W %06o <- %06o\n", machine().describe_context(), 0177130 + (offset << 1), data);

	switch (offset & 1)
	{
	case 0:
		set_ds((int16_t)m_read_ds(data & (CSR_W_DS | CSR_W_REZ)));
		if (selected_drive != -1)
		{
			floppy_info &fi = flopi[selected_drive];
			fi.dev->mon_w(!BIT(data, 4));
			fi.dev->ss_w(BIT(data, 5));
			if (BIT(data, 7))
			{
				LOG("COMMAND STEP %d %s\n", fi.id, BIT(data, 6) ? "+1" : "-1");
				execute_command(CMD_SEEK);
			}
			if (BIT(m_cr ^ data, 8) && !BIT(data, 8))
			{
				LOG("COMMAND READ drive %d c:h %d:%d\n", selected_drive, fi.dev->get_cyl(), BIT(data, 5));
				m_sr &= ~CSR_R_TR;
				execute_command(CMD_READ);
			}
		}
		m_cr = data;
		break;

	case 1:
		m_wbuf = data;
		m_sr &= ~CSR_R_TR;
		if (selected_drive != -1)
		{
			floppy_info &fi = flopi[selected_drive];
			if (fi.main_state != WRITE_DATA)
			{
				LOG("COMMAND WRITE drive %d c:h %d:%d\n", selected_drive, fi.dev->get_cyl(), BIT(m_cr, 5));
				execute_command(CMD_WRITE);
			}
		}
		break;
	}
}

void k1801vp128_device::execute_command(int command)
{
	live_abort();

	switch (command)
	{
	case CMD_READ:
		read_data_start(flopi[selected_drive]);
		break;

	case CMD_WRITE:
		write_data_start(flopi[selected_drive]);
		break;

	case CMD_SEEK:
		seek_start(flopi[selected_drive]);
		break;
	}
}


//-------------------------------------------------
//  update_tick - pump the device life cycle
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(k1801vp128_device::update_floppy)
{
	live_sync();

	floppy_info &fi = flopi[param];
	switch (fi.sub_state)
	{
	case SEEK_WAIT_STEP_SIGNAL_TIME:
		fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME_DONE;
		break;
	case SEEK_WAIT_STEP_TIME:
		fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
		break;
	}

	general_continue(fi);
}

void k1801vp128_device::live_start(floppy_info &fi, int state)
{
	cur_live.tm = machine().time();
	cur_live.state = state;
	cur_live.next_state = -1;
	cur_live.fi = &fi;
	cur_live.shift_reg = 0;
	cur_live.crc = 0xffff;
	cur_live.crc_init = false;
	cur_live.bit_counter = 0;
	cur_live.data_separator_phase = false;
	cur_live.data_reg = 0;
	cur_live.data_bit_context = false;
	cur_live.pll.reset(cur_live.tm);
	cur_live.pll.set_clock(attotime::from_hz(500000));
	checkpoint_live = cur_live;
	fi.live = true;

	live_run();
}

void k1801vp128_device::checkpoint()
{
	if (cur_live.fi)
		cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
	checkpoint_live = cur_live;
}

void k1801vp128_device::rollback()
{
	cur_live = checkpoint_live;
}

void k1801vp128_device::live_delay(int state)
{
	cur_live.next_state = state;
	if (cur_live.tm != machine().time())
		cur_live.fi->tm->adjust(cur_live.tm - machine().time(), cur_live.fi->id);
	else
		live_sync();
}

void k1801vp128_device::live_sync()
{
	if (!cur_live.tm.is_never())
	{
		if (cur_live.tm > machine().time())
		{
			rollback();
			live_run(machine().time());
			cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
		}
		else
		{
			cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
			if (cur_live.next_state != -1)
			{
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if (cur_live.state == IDLE)
			{
				cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
				cur_live.tm = attotime::never;
				cur_live.fi->live = false;
				cur_live.fi = nullptr;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void k1801vp128_device::live_abort()
{
	if (!cur_live.tm.is_never() && cur_live.tm > machine().time())
	{
		rollback();
		live_run(machine().time());
	}

	if (cur_live.fi)
	{
		cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
		cur_live.fi->live = false;
		cur_live.fi = nullptr;
	}

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

void k1801vp128_device::live_run(attotime limit)
{
	if (cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	if (limit == attotime::never)
	{
		if (cur_live.fi->dev)
			limit = cur_live.fi->dev->time_next_index();
		if (limit == attotime::never)
		{
			// Happens when there's no disk or if the fdc is not
			// connected to a drive, hence no index pulse. Force a
			// sync from time to time in that case, so that the main
			// cpu timeout isn't too painful.  Avoids looping into
			// infinity looking for data too.

			limit = machine().time() + attotime::from_msec(1);
			cur_live.fi->tm->adjust(attotime::from_msec(1), cur_live.fi->id);
		}
	}

	for (;;)
	{
		switch (cur_live.state)
		{
		case SEARCH_ADDRESS_MARK_HEADER:
			if (read_one_bit(limit))
				return;

			if (!(cur_live.bit_counter & 255))
			{
				LOGSHIFT("%s (%s): shift = %04x data=%02x c=%d\n", cur_live.tm.to_string(), limit.to_string(), cur_live.shift_reg,
					bitswap<8>(cur_live.shift_reg, 14, 12, 10, 8, 6, 4, 2, 0), cur_live.bit_counter);
			}

			if (cur_live.shift_reg == 0x4489)
			{
				LOGLIVE("%s: Found A1\n", cur_live.tm.to_string());
				cur_live.crc = 0x443b;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_DATA_LOW;
				cur_live.data_reg = 0xa1;
				m_sr &= ~CSR_R_CRC;
				checkpoint();
			}
			break;

		case READ_DATA_HIGH:
			if (read_one_bit(limit))
				return;
			if (cur_live.bit_counter & 15)
				break;
			live_delay(READ_DATA_HIGH_BYTE);
			return;

		case READ_DATA_HIGH_BYTE:
			cur_live.state = READ_DATA_LOW;
			checkpoint();
			break;

		case READ_DATA_LOW:
			if (read_one_bit(limit))
				return;
			if (cur_live.bit_counter & 15)
				break;
			live_delay(READ_DATA_LOW_BYTE);
			return;

		case READ_DATA_LOW_BYTE:
			m_rbuf = cur_live.data_reg;
			if (cur_live.crc == 0)
			{
				m_sr |= CSR_R_CRC;
			}
			m_sr |= CSR_R_TR;
			LOGLIVE("%s: Read %04x (CRC %04x)\n", cur_live.tm.to_string(), cur_live.data_reg, cur_live.crc);
			cur_live.state = READ_DATA_HIGH;
			checkpoint();
			break;

		case WRITE_MFM_DATA_LOW:
			if ((m_wbuf & 255) == 0xa1 && cur_live.crc_init == false && !(m_sr & CSR_R_CRC))
			{
				cur_live.crc_init = true;
				cur_live.crc = 0xffff;
			}
			// if DRQ has not been serviced, write CRC
			// if DRQ has not been serviced AND CRC has been written, write zeros
			if (m_sr & CSR_R_TR)
			{
				if (cur_live.crc_init)
				{
					LOGLIVE("%s: Write CRC %04x\n", cur_live.tm.to_string(), cur_live.crc);
					m_sr |= CSR_R_CRC;
					m_sr &= ~CSR_R_TR;
					cur_live.crc_init = false;
				}
				else
				{
					LOGLIVE("%s: Write after CRC\n", cur_live.tm.to_string());
					m_wbuf = 0;
				}
			}
			live_write_mfm((m_sr & CSR_R_CRC) ? (cur_live.crc >> 8) : m_wbuf, BIT(m_cr, 9));
			cur_live.state++;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_MFM_DATA_HIGH:
			live_write_mfm((m_sr & CSR_R_CRC) ? cur_live.crc : (m_wbuf >> 8), BIT(m_cr, 9));
			LOGLIVE("%s: Write next %s\n", cur_live.tm.to_string(), (m_sr & CSR_R_TR)?"TR":"");
			if (!(m_sr & CSR_R_CRC))
				m_sr |= CSR_R_TR;
			cur_live.state++;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_MFM_DATA_LOW_BYTE:
			if (write_one_bit(limit))
				return;
			if (cur_live.bit_counter == 0)
			{
				live_delay(WRITE_MFM_DATA_HIGH);
				return;
			}
			break;

		case WRITE_MFM_DATA_HIGH_BYTE:
			if (write_one_bit(limit))
				return;
			if (cur_live.bit_counter == 0)
			{
				live_delay(WRITE_MFM_DATA_LOW);
				m_sr &= ~CSR_R_CRC;
				return;
			}
			break;

		default:
			LOGWARN("%s: Unknown live state %d\n", cur_live.tm.to_string(), cur_live.state);
			return;
		}
	}
}


void k1801vp128_device::seek_start(floppy_info &fi)
{
	fi.sub_state = SEEK_MOVE;
	fi.dir = !BIT(m_cr, 6);
	general_continue(fi);
}


void k1801vp128_device::read_data_start(floppy_info &fi)
{
	fi.main_state = READ_DATA;
	fi.sub_state = WAIT_INDEX_DONE;
	m_sr &= ~CSR_R_CRC;
	read_data_continue(fi);
}

void k1801vp128_device::read_data_continue(floppy_info &fi)
{
	for (;;)
	{
		switch (fi.sub_state)
		{
		case SEEK_MOVE:
			LOGSTATE("sub %d SEEK_MOVE\n", fi.id);
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_msec(2), fi.id);
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			LOGSTATE("sub %d SEEK_WAIT_STEP_SIGNAL_TIME\n", fi.id);
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			LOGSTATE("sub %d SEEK_WAIT_STEP_SIGNAL_TIME_DONE\n", fi.id);
			if (fi.dev)
			{
				fi.dev->dir_w(fi.dir);
				fi.dev->stp_w(0);
				fi.dev->stp_w(1);
				fi.sub_state = WAIT_INDEX_DONE;
			}
			else
				fi.main_state = fi.sub_state = IDLE;
			break;

		case WAIT_INDEX:
			LOGSTATE("sub %d WAIT_INDEX\n", fi.id);
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("sub %d WAIT_INDEX_DONE\n", fi.id);
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			LOGSTATE("live %d SEARCH_ADDRESS_MARK_HEADER\n", fi.id);
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("sub %d SCAN_ID\n", fi.id);
			fi.sub_state = TRACK_READ;
			LOGSTATE("live %d READ_DATA_HIGH\n", fi.id);
			live_start(fi, READ_DATA_HIGH);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("sub %d SCAN_ID_FAILED\n", fi.id);
			fi.sub_state = COMMAND_DONE;
			break;

		case TRACK_READ:
			LOGSTATE("sub %d TRACK_READ\n", fi.id);
			fi.sub_state = COMMAND_DONE;
			break;

		case COMMAND_DONE:
			LOGSTATE("sub %d COMMAND_DONE\n", fi.id);
			fi.main_state = fi.sub_state = IDLE;
			return;

		default:
			LOGWARN("%s: read sector unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

void k1801vp128_device::write_data_start(floppy_info &fi)
{
	fi.main_state = WRITE_DATA;
	fi.sub_state = WAIT_INDEX_DONE;
	m_sr &= ~CSR_R_CRC;
	write_data_continue(fi);
}

void k1801vp128_device::write_data_continue(floppy_info &fi)
{
	for (;;)
	{
		switch (fi.sub_state)
		{
		case WAIT_INDEX:
			LOGSTATE("sub %d WAIT_INDEX\n", fi.id);
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("sub %d WAIT_INDEX_DONE\n", fi.id);
			fi.sub_state = TRACK_WRITTEN;
			LOGSTATE("live %d WRITE_MFM_DATA_LOW\n", fi.id);
			live_start(fi, WRITE_MFM_DATA_LOW);
			return;

		case TRACK_WRITTEN:
			LOGSTATE("sub %d TRACK_WRITTEN\n", fi.id);
			fi.sub_state = COMMAND_DONE;
			break;

		case COMMAND_DONE:
			LOGSTATE("sub %d COMMAND_DONE\n", fi.id);
			fi.main_state = fi.sub_state = IDLE;
			return;

		default:
			LOGWARN("%s: write sector unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}


void k1801vp128_device::index_callback(floppy_image_device *floppy, int state)
{
	LOGLIVE("%s: Pulse %d\n", machine().time().to_string(), state);
	for (floppy_info &fi : flopi)
	{
		if (fi.dev != floppy)
			continue;

		if (!state)
		{
			general_continue(fi);
			continue;
		}

		switch (fi.sub_state)
		{
		case IDLE:
		case SEEK_MOVE:
		case SEEK_WAIT_STEP_SIGNAL_TIME:
		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
		case SEEK_WAIT_STEP_TIME:
		case SEEK_WAIT_STEP_TIME_DONE:
		case HEAD_LOAD:
		case HEAD_LOAD_DONE:
		case SCAN_ID:
		case SCAN_ID_FAILED:
			break;

		case TRACK_READ:
			fi.sub_state = IDLE;
			break;

		case WAIT_INDEX:
			fi.sub_state = WAIT_INDEX_DONE;
			live_abort();
			break;

		default:
			LOGWARN("%s: Index pulse on unknown sub-state %d\n", ttsn(), fi.sub_state);
			break;
		}

		general_continue(fi);
	}
}


void k1801vp128_device::general_continue(floppy_info &fi)
{
	if (fi.live && cur_live.state != IDLE)
	{
		live_run();
		if (cur_live.state != IDLE)
			return;
	}

	switch (fi.main_state)
	{
	case IDLE:
		break;

	case READ_DATA:
		read_data_continue(fi);
		break;

	case WRITE_DATA:
		write_data_continue(fi);
		break;

	default:
		LOGWARN("%s: general_continue on unknown main-state %d\n", ttsn(), fi.main_state);
		break;
	}
}


bool k1801vp128_device::read_one_bit(const attotime &limit)
{
	int bit = cur_live.pll.get_next_bit(cur_live.tm, cur_live.fi->dev, limit);
	if (bit < 0)
		return true;
	cur_live.shift_reg = (cur_live.shift_reg << 1) | bit;
	cur_live.bit_counter++;
	if (cur_live.data_separator_phase)
	{
		cur_live.data_reg = (cur_live.data_reg << 1) | bit;
		if ((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
		else
			cur_live.crc = cur_live.crc << 1;
	}
	cur_live.data_separator_phase = !cur_live.data_separator_phase;
	return false;
}

bool k1801vp128_device::write_one_bit(const attotime &limit)
{
	bool bit = cur_live.shift_reg & 0x8000;
	if (cur_live.pll.write_next_bit(bit, cur_live.tm, cur_live.fi->dev, limit))
		return true;
	if ((cur_live.bit_counter & 1) && cur_live.crc_init)
	{
		if ((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
		else
			cur_live.crc = cur_live.crc << 1;
	}
	cur_live.shift_reg = cur_live.shift_reg << 1;
	cur_live.bit_counter--;
	return false;
}

void k1801vp128_device::live_write_mfm(uint8_t mfm, bool marker)
{
	bool context = cur_live.data_bit_context;
	uint16_t raw = 0;
	for (int i = 0; i < 8; i++)
	{
		bool bit = mfm & (0x80 >> i);
		if (!(bit || context))
			raw |= 0x8000 >> (2*i);
		if (bit)
			raw |= 0x4000 >> (2*i);
		context = bit;
	}
	if (marker && (mfm & 0xc) == 0) raw &= 0xffdf; // A1 and C2 sync sequences
	cur_live.data_reg = mfm;
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = context;
	LOGLIVE("%s: write %02x   %04x %04x\n", cur_live.tm.to_string(), mfm, cur_live.crc, raw);
}


void k1801vp128_device::set_ds(int fid)
{
	if (selected_drive == fid)
		return;

	live_abort();

	// pass drive select to connected drives
	for (floppy_info &fi : flopi)
		if (fi.dev)
			fi.dev->ds_w(fid);

	// record selected drive
	selected_drive = fid;

	if (fid != -1)
	{
		LOG("COMMAND ~READ drive %d c:h %d:%d\n", selected_drive, flopi[fid].dev->get_cyl(), BIT(m_cr, 5));
		m_sr &= ~CSR_R_TR;
		execute_command(CMD_READ);
	}
}

std::string k1801vp128_device::ttsn() const
{
	return machine().time().to_string();
}
