// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

	DVK "MX" floppy controller (decimal ID 3.057.122)

	Supports four 5.25" drives (single- or double-sided, 40- or 80-track).

	Uses FM encoding and software-defined track format;
	usually 11 sectors of 256 bytes, with simple additive checksum.

	RT-11 driver name: MX.SYS

	References:
	https://emuverse.ru/downloads/computers/DVK/docs/KMD/kngmd_MX_2.djvu
	https://torlus.com/floppy/forum/viewtopic.php?t=1384
	http://hobot.pdp-11.ru/ukdwk_archive/dwkwebcomplekt/DWKFiles/mx/README.mx1

***************************************************************************/

#include "emu.h"
#include "dvk_mx.h"

#include "formats/dvk_mx_dsk.h"

#define LOG_WARN    (1U << 1)   // Show warnings
#define LOG_SHIFT   (1U << 2)   // Shows shift register contents
#define LOG_REGS    (1U << 3)   // Digital input/output register and data rate select
#define LOG_STATE   (1U << 4)   // State machine
#define LOG_LIVE    (1U << 5)   // Live states

#define VERBOSE (LOG_GENERAL | LOG_REGS | LOG_LIVE | LOG_STATE)
#include "logmacro.h"

#define LOGWARN(...)     LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGSHIFT(...)    LOGMASKED(LOG_SHIFT, __VA_ARGS__)
#define LOGREGS(...)     LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGLIVE(...)     LOGMASKED(LOG_LIVE, __VA_ARGS__)
#define LOGSTATE(...)    LOGMASKED(LOG_STATE, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MXCSR_GETDRIVE(x)  (((x) >> MXCSR_V_DRIVE) & MXCSR_M_DRIVE)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DVK_MX, dvk_mx_device, "dvk_mx", "DVK MX floppy controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dvk_mx_device - constructor
//-------------------------------------------------

dvk_mx_device::dvk_mx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DVK_MX, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_connectors(*this, "%u", 0U)
{
	memset(&cur_live, 0x00, sizeof(cur_live));
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dvk_mx_device::device_start()
{
	for (int i = 0; i != 4; i++)
	{
		flopi[i].tm = timer_alloc(FUNC(dvk_mx_device::update_floppy), this);
		flopi[i].id = i;
		if (m_connectors[i])
		{
			flopi[i].dev = m_connectors[i]->get_device();
			if (flopi[i].dev != nullptr)
				flopi[i].dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&dvk_mx_device::index_callback, this));
		}
		else
			flopi[i].dev = nullptr;

		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].live = false;
	}

	// save state
	save_item(NAME(m_mxcs));
	save_item(NAME(m_rbuf));
	save_item(NAME(m_wbuf));
	save_item(NAME(selected_drive));

	m_timer_2khz = timer_alloc(FUNC(dvk_mx_device::twokhz_tick), this);

	m_bus->install_device(0177130, 0177133, read16sm_delegate(*this, FUNC(dvk_mx_device::read)),
		write16sm_delegate(*this, FUNC(dvk_mx_device::write)));

	m_mxcs = 0;
	selected_drive = -1;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dvk_mx_device::device_reset()
{
	m_rbuf = m_wbuf = 0;

	m_mxcs &= ~MXCSR_ERR;

	m_timer_2khz->adjust(attotime::never, 0, attotime::never);

	live_abort();
	set_ds(-1);
}

void dvk_mx_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_DVK_MX_FORMAT);
}

static void mx_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("525dd", FLOPPY_525_DD);
}

void dvk_mx_device::device_add_mconfig(machine_config &config)
{
	FLOPPY_CONNECTOR(config, "0", mx_floppies, "525qd", dvk_mx_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "1", mx_floppies, "525qd", dvk_mx_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "2", mx_floppies, "525dd", dvk_mx_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "3", mx_floppies, "525dd", dvk_mx_device::floppy_formats);
}

uint16_t dvk_mx_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_mxcs & MXCSR_RD;
		if (selected_drive != -1)
		{
			floppy_info &fi = flopi[selected_drive];
			if (!fi.dev->trk00_r()) data |= MXCSR_TRK0;
			if (fi.dev->wpt_r()) data |= MXCSR_WP;
			if (fi.index) data |= MXCSR_INDEX;
		}
		if (!machine().side_effects_disabled())
			m_mxcs &= ~MXCSR_TIMER;
		break;

	case 1:
		data = m_rbuf;
		if (!machine().side_effects_disabled())
			m_mxcs &= ~MXCSR_TR;
		break;
	}

	return data;
}

void dvk_mx_device::write(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:
		if (!BIT(data, 1))
			set_ds(MXCSR_GETDRIVE(data));
		else
			set_ds(-1);
		if (selected_drive != -1)
		{
			floppy_info &fi = flopi[selected_drive];
			fi.dev->mon_w(!BIT(data, 6));
			fi.dev->ss_w(BIT(data, 12));
			if (BIT(data, 4))
			{
				LOG("COMMAND STEP %s\n", BIT(data, 5) ? "+1" : "-1");
				execute_command(2);
			}
			if (data & MXCSR_GO)
			{
				LOG("COMMAND %s drive %d c:h %d:%d\n", BIT(data, 13)?"WRITE":"READ", selected_drive, fi.dev->get_cyl(), BIT(data, 12));
				m_mxcs &= ~MXCSR_TR;
				execute_command(BIT(data, 13));
			}
		}

		if (BIT(data, 7))
		{
			m_timer_2khz->adjust(attotime::from_hz(2000), 0, attotime::from_hz(2000));
		}
		else
		{
			m_timer_2khz->adjust(attotime::never, 0, attotime::never);
		}
		UPDATE_16BIT(&m_mxcs, data, MXCSR_WR);
		break;

	case 1:
		m_wbuf = data;
		m_mxcs &= ~MXCSR_TR;
	}
}

void dvk_mx_device::execute_command(int command)
{
	m_mxcs &= ~MXCSR_ERR;
	live_abort();

	switch (command)
	{
	case 0:
		read_data_start(flopi[selected_drive]);
		break;

	case 1:
		write_data_start(flopi[selected_drive]);
		break;

	case 2:
		seek_start(flopi[selected_drive]);
		break;
	}
}

TIMER_CALLBACK_MEMBER(dvk_mx_device::twokhz_tick)
{
	m_mxcs |= MXCSR_TIMER;
}

std::string dvk_mx_device::ttsn() const
{
	return machine().time().to_string();
}

//-------------------------------------------------
//  update_tick - pump the device life cycle
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dvk_mx_device::update_floppy)
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

void dvk_mx_device::live_start(floppy_info &fi, int state)
{
	cur_live.tm = machine().time();
	cur_live.state = state;
	cur_live.next_state = -1;
	cur_live.fi = &fi;
	cur_live.shift_reg = 0;
	cur_live.bit_counter = 0;
	cur_live.data_separator_phase = false;
	cur_live.data_reg = 0;
	cur_live.pll.reset(cur_live.tm);
	cur_live.pll.set_clock(attotime::from_hz(250000));
	checkpoint_live = cur_live;
	fi.live = true;

	LOGLIVE("%s: Called live_start (%d)\n", cur_live.tm.to_string(), state);
	live_run();
}

void dvk_mx_device::checkpoint()
{
	if (cur_live.fi)
		cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
	checkpoint_live = cur_live;
}

void dvk_mx_device::rollback()
{
	cur_live = checkpoint_live;
}

void dvk_mx_device::live_delay(int state)
{
	cur_live.next_state = state;
	if (cur_live.tm != machine().time())
		cur_live.fi->tm->adjust(cur_live.tm - machine().time(), cur_live.fi->id);
	else
		live_sync();
}

void dvk_mx_device::live_sync()
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

void dvk_mx_device::live_abort()
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

void dvk_mx_device::live_run(attotime limit)
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
		case SEARCH_ID:
			if (read_one_bit(limit))
				return;

			if (!(cur_live.bit_counter & 255))
			{
				LOGSHIFT("%s (%s): shift = %04x data=%02x c=%d\n", cur_live.tm.to_string(), limit.to_string(), cur_live.shift_reg,
					bitswap<8>(cur_live.shift_reg, 14, 12, 10, 8, 6, 4, 2, 0), cur_live.bit_counter);
			}

			if (cur_live.shift_reg == 0xaaaaffaf)
			{
				LOGLIVE("%s: Found SYNC\n", cur_live.tm.to_string());
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_TRACK_DATA_BYTE;
				cur_live.data_reg = 0363; // 0xf3
				checkpoint();
			}
			break;

		case READ_TRACK_DATA:
			if (read_one_bit(limit))
				return;
			if (cur_live.bit_counter & 31)
				break;
			live_delay(READ_TRACK_DATA_BYTE);
			return;

		case READ_TRACK_DATA_BYTE:
			m_rbuf = cur_live.data_reg;
			m_mxcs |= MXCSR_TR;
			cur_live.state = READ_TRACK_DATA;
			checkpoint();
			break;

		case WRITE_TRACK_DATA:
			live_write_fm(m_wbuf);
			cur_live.state = WRITE_TRACK_DATA_BYTE;
			cur_live.bit_counter = 32;
			checkpoint();
			break;

		case WRITE_TRACK_DATA_BYTE:
			if (write_one_bit(limit))
				return;
			if (cur_live.bit_counter == 0)
			{
				LOGLIVE("%s: Write next %s\n", cur_live.tm.to_string(), (m_mxcs & MXCSR_TR)?"TR":"");
				if (m_mxcs & MXCSR_TR)
				{
					m_mxcs |= MXCSR_ERR;
					cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
					cur_live.state = IDLE;
				}
				else
				{
					m_mxcs |= MXCSR_TR;
					live_delay(WRITE_TRACK_DATA);
				}
				return;
			}
			break;

		default:
			LOGWARN("%s: Unknown live state %d\n", cur_live.tm.to_string(), cur_live.state);
			return;
		}
	}
}

void dvk_mx_device::seek_start(floppy_info &fi)
{
	fi.main_state = SEEK;
	fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
	fi.dir = !BIT(m_mxcs, 5);
	seek_continue(fi);
}

void dvk_mx_device::seek_continue(floppy_info &fi)
{
	for (;;)
	{
		switch (fi.sub_state)
		{
		case SEEK_MOVE:
			LOGSTATE("sub SEEK_MOVE\n");
			if (fi.dev)
			{
				fi.dev->dir_w(fi.dir);
				fi.dev->stp_w(0);
				fi.dev->stp_w(1);
			}
			fi.main_state = fi.sub_state = IDLE;
			m_mxcs &= ~MXCSR_STEP;
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			LOGSTATE("sub SEEK_WAIT_STEP_SIGNAL_TIME\n");
			fi.tm->adjust(attotime::from_msec(2), fi.id); // FIXME
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			LOGSTATE("sub SEEK_WAIT_STEP_SIGNAL_TIME_DONE\n");
			fi.sub_state = SEEK_MOVE;
			break;
		}
	}
}


void dvk_mx_device::read_data_start(floppy_info &fi)
{
	fi.main_state = READ_DATA;
	fi.sub_state = WAIT_INDEX;
	read_data_continue(fi);
}

void dvk_mx_device::read_data_continue(floppy_info &fi)
{
	for (;;)
	{
		switch (fi.sub_state)
		{
		case WAIT_INDEX:
			LOGSTATE("sub WAIT_INDEX\n");
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("sub WAIT_INDEX_DONE\n");
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			LOGSTATE("live SEARCH_ID\n");
			live_start(fi, SEARCH_ID);
			return;

		case SCAN_ID:
			LOGSTATE("sub SCAN_ID\n");
			fi.sub_state = TRACK_READ;
			LOGSTATE("live READ_TRACK_DATA_BYTE\n");
			live_start(fi, READ_TRACK_DATA_BYTE);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("sub SCAN_ID_FAILED\n");
			fi.sub_state = COMMAND_DONE;
			break;

		case TRACK_READ:
			LOGSTATE("sub TRACK_READ\n");
			fi.sub_state = COMMAND_DONE;
			break;

		case COMMAND_DONE:
			LOGSTATE("sub COMMAND_DONE\n");
			fi.main_state = fi.sub_state = IDLE;
			return;

		default:
			LOGWARN("%s: read sector unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

void dvk_mx_device::write_data_start(floppy_info &fi)
{
	fi.main_state = WRITE_DATA;
	fi.sub_state = WAIT_INDEX;
	write_data_continue(fi);
}

void dvk_mx_device::write_data_continue(floppy_info &fi)
{
	for (;;)
	{
		switch (fi.sub_state)
		{
		case WAIT_INDEX:
			LOGSTATE("sub WAIT_INDEX\n");
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("sub WAIT_INDEX_DONE\n");
			fi.sub_state = TRACK_WRITTEN;
			LOGSTATE("live WRITE_TRACK_DATA\n");
			live_start(fi, WRITE_TRACK_DATA);
			return;

		case TRACK_WRITTEN:
			LOGSTATE("sub TRACK_WRITTEN\n");
			fi.sub_state = COMMAND_DONE;
			break;

		case COMMAND_DONE:
			LOGSTATE("sub COMMAND_DONE\n");
			fi.main_state = fi.sub_state = IDLE;
			return;

		default:
			LOGWARN("%s: write sector unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}


void dvk_mx_device::index_callback(floppy_image_device *floppy, int state)
{
	for (floppy_info &fi : flopi)
	{
		if (fi.dev != floppy)
			continue;

		fi.index = state;

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
		case SCAN_ID_FAILED:
			break;

		case TRACK_READ:
			fi.sub_state = IDLE;
			break;

		case WAIT_INDEX:
			fi.sub_state = WAIT_INDEX_DONE;
			live_abort();
			break;

		case SCAN_ID:
			fi.counter++;
			if (fi.counter == 2)
			{
				fi.sub_state = SCAN_ID_FAILED;
				live_abort();
			}
			break;

		default:
			LOGWARN("%s: Index pulse on unknown sub-state %d\n", ttsn(), fi.sub_state);
			break;
		}

		general_continue(fi);
	}
}

void dvk_mx_device::general_continue(floppy_info &fi)
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

	case SEEK:
		seek_continue(fi);
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


bool dvk_mx_device::read_one_bit(const attotime &limit)
{
	int bit = cur_live.pll.get_next_bit(cur_live.tm, cur_live.fi->dev, limit);
	if (bit < 0)
		return true;
	cur_live.shift_reg = (cur_live.shift_reg << 1) | bit;
	cur_live.bit_counter++;
	if (cur_live.data_separator_phase)
	{
		cur_live.data_reg = (cur_live.data_reg << 1) | bit;
	}
	cur_live.data_separator_phase = !cur_live.data_separator_phase;
	return false;
}

bool dvk_mx_device::write_one_bit(const attotime &limit)
{
	bool bit = cur_live.shift_reg & 0x80000000;
	if (cur_live.pll.write_next_bit(bit, cur_live.tm, cur_live.fi->dev, limit))
		return true;
	cur_live.shift_reg = cur_live.shift_reg << 1;
	cur_live.bit_counter--;
	return false;
}

void dvk_mx_device::live_write_fm(uint16_t fm)
{
	uint32_t raw = 0xaaaaaaaa;
	for (int i = 0; i < 16; i++)
		if (fm & (0x8000 >> i))
			raw |= 0x40000000 >> (2*i);
	cur_live.data_reg = fm;
	cur_live.shift_reg = raw;
	LOGLIVE("%s: Write %04x (%06o) FM %08x\n", cur_live.tm.to_string(), fm, fm, raw);
}


//-------------------------------------------------
//  set_floppy -
//-------------------------------------------------

void dvk_mx_device::set_ds(int fid)
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
}
