// license:BSD-3-Clause
// copyright-holders:Carl,Olivier Galibert

#include "i8271.h"

const device_type I8271 = &device_creator<i8271_device>;

i8271_device::i8271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8271, "Intel 8271", tag, owner, clock, "i8271", __FILE__), ready_connected(false), mode(0), main_phase(0),
	intrq_cb(*this),
	drq_cb(*this),
	hdl_cb(*this),
	opt_cb(*this), irq(false), drq(false), scan_done(false), scan_match(false), command_pos(0), sectors_read(0), scan_len(0), dma_data(0), oport(0), rr(0), scan_sec(0), moder(0), 
	precomp(0), perpmode(0), srate(0), hset(0), icnt(0), hload(0), sector_size(0), cur_rate(0)
{
	select_connected = true;
	external_ready = false;
}

DEVICE_ADDRESS_MAP_START(map, 8, i8271_device)
	AM_RANGE(0x0, 0x0) AM_READWRITE(sr_r, cmd_w)
	AM_RANGE(0x1, 0x1) AM_READWRITE(rr_r, param_w)
	AM_RANGE(0x2, 0x2) AM_WRITE(reset_w)
ADDRESS_MAP_END

void i8271_device::set_ready_line_connected(bool _ready)
{
	ready_connected = _ready;
}

void i8271_device::set_select_lines_connected(bool _select)
{
	select_connected = _select;
}

void i8271_device::device_start()
{
	intrq_cb.resolve_safe();
	drq_cb.resolve_safe();
	hdl_cb.resolve_safe();
	opt_cb.resolve_safe();

	for(int i=0; i != 2; i++) {
		char name[2];
		flopi[i].tm = timer_alloc(i);
		flopi[i].id = i;
		if(select_connected) {
			name[0] = '0'+i;
			name[1] = 0;
			floppy_connector *con = subdevice<floppy_connector>(name);
			if(con) {
				flopi[i].dev = con->get_device();
				if (flopi[i].dev != NULL)
					flopi[i].dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(i8271_device::index_callback), this));
			} else
				flopi[i].dev = NULL;
		} else
			flopi[i].dev = NULL;

		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].dir = 0;
		flopi[i].counter = 0;
		flopi[i].pcn = 0;
		flopi[i].live = false;
		flopi[i].index = false;
		flopi[i].ready = true;
	}
	cur_rate = 250000;

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.fi = NULL;
}

void i8271_device::device_reset()
{
	soft_reset();
}

void i8271_device::soft_reset()
{
	main_phase = PHASE_IDLE;
	for(int i=0; i<2; i++) {
		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].live = false;
		flopi[i].ready = get_ready(i);
	}
	set_irq(false);
	set_drq(false);
	command_pos = 0;
	cur_live.fi = 0;
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.fi = NULL;
	rr = 0;
	scan_sec = 0;
	moder = 0xc0;
	oport = 0;
	scan_cnt[0] = scan_cnt[1] = 0;
}

void i8271_device::ready_w(bool _ready)
{
	external_ready = _ready;
}

bool i8271_device::get_ready(int fid)
{
	if(!flopi[fid].ready)
		return false;
	if(ready_connected)
		return flopi[fid].dev ? !flopi[fid].dev->ready_r() : false;
	return !external_ready;
}

void i8271_device::set_floppy(floppy_image_device *flop)
{
	for(int fid=0; fid<2; fid++) {
		if(flopi[fid].dev)
			flopi[fid].dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		flopi[fid].dev = flop;
	}
	if(flop)
		flop->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(i8271_device::index_callback), this));
}

READ8_MEMBER(i8271_device::sr_r)
{
	UINT32 ret = (irq ? SR_IRQ : 0);
	switch(main_phase) {
	case PHASE_CMD:
		ret |= SR_CF;
		break;
	case PHASE_EXEC:
		ret |= SR_BSY;
		if((moder & 1) && drq)
			ret |= SR_DRQ;
		break;
	case PHASE_RESULT:
		ret |= SR_RF;
		break;
	}
	return ret;
}

READ8_MEMBER(i8271_device::rr_r)
{
	if(main_phase == PHASE_RESULT)
		main_phase = PHASE_IDLE;
	set_irq(false);
	return rr;
}

void i8271_device::set_rate(int rate)
{
	cur_rate = rate;
}

READ8_MEMBER(i8271_device::data_r)
{
	set_drq(false);
	return dma_data;
}

WRITE8_MEMBER(i8271_device::data_w)
{
	if(drq) {
		set_drq(false);
		dma_data = data;
	}
}

WRITE8_MEMBER(i8271_device::cmd_w)
{
	if(main_phase == PHASE_IDLE) {
		command[0] = data;
		command_pos = 1;

		int cmd = check_command();
		if(cmd != C_INVALID) {
			main_phase = PHASE_CMD;
			if(cmd != C_INCOMPLETE)
				start_command(cmd);
		}
	}
}

WRITE8_MEMBER(i8271_device::param_w)
{
	if(main_phase == PHASE_CMD) {
		command[command_pos++] = data;
		int cmd = check_command();
		if(cmd != C_INCOMPLETE)
			start_command(cmd);
	}
}

bool i8271_device::set_output(UINT8 data)
{
	if(main_phase == PHASE_EXEC) {
		if(drq) {
			rr = ERR_DMA;
			return false;
		}
		dma_data = data;
		set_drq(true);
	}
	return true;
}

bool i8271_device::get_input(UINT8 *data)
{
	if(main_phase == PHASE_EXEC) {
		if(drq) {
			rr = ERR_DMA;
			return false;
		}
		*data = dma_data;
		set_drq(true);
	}
	return true;
}

void i8271_device::set_drq(bool state)
{
	if(state != drq) {
		drq = state;
		drq_cb(drq);
		if(moder & 1)
			intrq_cb(drq);
	}
}

void i8271_device::set_irq(bool state)
{
	if(state != irq) {
		irq = state;
		intrq_cb(irq);
	}
}

void i8271_device::live_start(floppy_info &fi, int state)
{
	cur_live.tm = machine().time();
	cur_live.state = state;
	cur_live.next_state = -1;
	cur_live.fi = &fi;
	cur_live.shift_reg = 0;
	cur_live.crc = 0xffff;
	cur_live.bit_counter = 0;
	cur_live.data_separator_phase = false;
	cur_live.data_reg = 0;
	cur_live.previous_type = live_info::PT_NONE;
	cur_live.data_bit_context = false;
	cur_live.byte_counter = 0;
	cur_live.pll.reset(cur_live.tm);
	cur_live.pll.set_clock(attotime::from_hz(cur_rate));
	checkpoint_live = cur_live;
	fi.live = true;

	live_run();
}

void i8271_device::checkpoint()
{
	if(cur_live.fi)
		cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
	checkpoint_live = cur_live;
}

void i8271_device::rollback()
{
	cur_live = checkpoint_live;
}

void i8271_device::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		cur_live.fi->tm->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void i8271_device::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
		} else {
			cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
				cur_live.tm = attotime::never;
				cur_live.fi->live = false;
				cur_live.fi = 0;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void i8271_device::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	if(cur_live.fi) {
		cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
		cur_live.fi->live = false;
		cur_live.fi = 0;
	}

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

void i8271_device::live_run(attotime limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	if(limit == attotime::never) {
		if(cur_live.fi->dev)
			limit = cur_live.fi->dev->time_next_index();
		if(limit == attotime::never) {
			// Happens when there's no disk or if the fdc is not
			// connected to a drive, hence no index pulse. Force a
			// sync from time to time in that case, so that the main
			// cpu timeout isn't too painful.  Avoids looping into
			// infinity looking for data too.

			limit = machine().time() + attotime::from_msec(1);
			cur_live.fi->tm->adjust(attotime::from_msec(1));
		}
	}

	for(;;) {
		switch(cur_live.state) {
		case SEARCH_ADDRESS_MARK_HEADER:
			if(read_one_bit(limit))
				return;
#if 0
			fprintf(stderr, "%s: shift = %04x data=%02x c=%d\n", tts(cur_live.tm).c_str(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter);
#endif

			if(cur_live.shift_reg == 0xf57e) {
				cur_live.crc = 0xef21;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_ID_BLOCK;
			}
			break;

		case READ_ID_BLOCK: {
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			int slot = (cur_live.bit_counter >> 4)-1;

			if(0)
				fprintf(stderr, "%s: slot=%d data=%02x crc=%04x\n", tts(cur_live.tm).c_str(), slot, cur_live.data_reg, cur_live.crc);
			cur_live.idbuf[slot] = cur_live.data_reg;
			if(cur_live.fi->main_state == READ_ID) {
				if(!set_output(cur_live.data_reg)) {
					live_delay(IDLE);
					return;
				}
			}
			if(slot == 5) {
				live_delay(IDLE);
				return;
			}
			break;
		}

		case SEARCH_ADDRESS_MARK_DATA:
			if(read_one_bit(limit))
				return;
#if 0
			fprintf(stderr, "%s: shift = %04x data=%02x c=%d.%x\n", tts(cur_live.tm).c_str(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter >> 4, cur_live.bit_counter & 15);
#endif

			if(cur_live.bit_counter > 23*16) {
				live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
				return;
			}

			if(cur_live.bit_counter >= 11*16 && (cur_live.shift_reg == 0xf56a || cur_live.shift_reg == 0xf56f)) {
				cur_live.crc = cur_live.shift_reg == 0xf56a ? 0x8fe7 : 0xbf84;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_SECTOR_DATA;
			}

			break;

		case SEARCH_ADDRESS_MARK_DATA_FAILED:
			cur_live.state = IDLE;
			return;

		case READ_SECTOR_DATA: {
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			int slot = (cur_live.bit_counter >> 4)-1;
			if(slot < sector_size) {
				// Sector data
				if(cur_live.fi->main_state == SCAN_DATA)
					live_delay(SCAN_SECTOR_DATA_BYTE);
				else if(cur_live.fi->main_state == VERIFY_DATA)
					live_delay(VERIFY_SECTOR_DATA_BYTE);
				else
					live_delay(READ_SECTOR_DATA_BYTE);
				return;
			} else if(slot < sector_size+2) {
				// CRC
				if(slot == sector_size+1) {
					live_delay(IDLE);
					return;
				}
			}
			break;
		}

		case READ_SECTOR_DATA_BYTE:
			if(!set_output(cur_live.data_reg)) {
				live_delay(IDLE);
				return;
			}
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		case SCAN_SECTOR_DATA_BYTE:
			if(!scan_done)
			{
				UINT8 data = 0;
				if(!get_input(&data)) {
					live_delay(IDLE);
					return;
				}
				if(cur_live.data_reg != 0xff)
				{
					switch(command[4] >> 6) {
					case 0:
						if(cur_live.data_reg != data)
							scan_match = false;
						break;
					case 1:
						if(cur_live.data_reg < data)
							scan_match = false;
						break;
					case 2:
						if(cur_live.data_reg > data)
							scan_match = false;
						break;
					default:
						scan_match = false;
						break;
					}
				}
				scan_len++;
				if(scan_len == command[5]) {
					if(scan_match) {
						int slot = calc_sector_size(command[3] >> 5) - ((cur_live.bit_counter >> 4)-1);
						scan_sec = cur_live.idbuf[2];
						scan_cnt[0] = slot & 0x7f;
						scan_cnt[1] = slot >> 9;
						scan_done = true;
						rr = command[4] & 0xc0 ? ERR_SMNE : ERR_SMEQ;
					}
					else
					{
						scan_match = true;
						scan_len = 0;
					}
				}
			}
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		case VERIFY_SECTOR_DATA_BYTE:
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		case WRITE_SECTOR_SKIP_GAP2:
			cur_live.bit_counter = 0;
			cur_live.byte_counter = 0;
			cur_live.state = WRITE_SECTOR_SKIP_GAP2_BYTE;
			checkpoint();
			break;

		case WRITE_SECTOR_SKIP_GAP2_BYTE:
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter != 11*16)
				break;
			cur_live.bit_counter = 0;
			cur_live.byte_counter = 0;
			live_delay(WRITE_SECTOR_DATA);
			return;

		case WRITE_SECTOR_DATA:
			if(cur_live.byte_counter < 6)
				live_write_fm(0x00);
			else if(cur_live.byte_counter < 7) {
				cur_live.crc = 0xffff;
				live_write_raw(BIT(command[0], 2) ? 0xf56a : 0xf56f);
			} else if(cur_live.byte_counter < 7+sector_size) {
				UINT8 data = 0;
				if(!get_input(&data)) {
					live_delay(IDLE);
					return;
				}
				live_write_fm(data);
			}
			else if(cur_live.byte_counter < 7+sector_size+2)
				live_write_fm(cur_live.crc >> 8);
			else if(cur_live.byte_counter < 7+sector_size+2+1)
				live_write_fm(0xff);
			else {
				cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
				cur_live.state = IDLE;
				return;
			}
			cur_live.state = WRITE_SECTOR_DATA_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_TRACK_PRE_SECTORS:
			if(command[4]) {
				if(cur_live.byte_counter < command[4])
					live_write_fm(0xff);
				else if(cur_live.byte_counter < (command[4] + 6))
					live_write_fm(0x00);
				else if(cur_live.byte_counter < (command[4] + 7))
					live_write_raw(0xf77a);
				else if(cur_live.byte_counter < (command[4] + command[5] + 7))
					live_write_fm(0xff);
				else {
					cur_live.state = WRITE_TRACK_SECTOR;
					cur_live.byte_counter = 0;
					break;
				}
			} else {
				if(cur_live.byte_counter < command[5])
					live_write_fm(0xff);
				else {
					cur_live.state = WRITE_TRACK_SECTOR;
					cur_live.byte_counter = 0;
					break;
				}
			}
			cur_live.state = WRITE_TRACK_PRE_SECTORS_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_TRACK_SECTOR:
			if(cur_live.byte_counter < 6)
				live_write_fm(0x00);
			else if(cur_live.byte_counter < 7) {
				cur_live.crc = 0xffff;
				live_write_raw(0xf57e);
			} else if(cur_live.byte_counter < 11) {
				UINT8 data = 0;
				if(!get_input(&data)) {
					live_delay(IDLE);
					return;
				}
				live_write_fm(data);
			} else if(cur_live.byte_counter < 13)
				live_write_fm(cur_live.crc >> 8);
			else if(cur_live.byte_counter < 24)
				live_write_fm(0xff);
			else if(cur_live.byte_counter < 30)
				live_write_fm(0x00);
			else if(cur_live.byte_counter < 31) {
				cur_live.crc = 0xffff;
				live_write_raw(0xf56f);
			} else if(cur_live.byte_counter < 31+sector_size)
				live_write_fm(0xe5);
			else if(cur_live.byte_counter < 33+sector_size)
				live_write_fm(cur_live.crc >> 8);
			else if(cur_live.byte_counter < 33+sector_size+command[2])
				live_write_fm(0xff);
			else {
				cur_live.byte_counter = 0;
				command[3] = (command[3] & 0xe0) | ((command[3] & 0x1f) - 1);
				cur_live.state = command[3] & 0x1f ? WRITE_TRACK_SECTOR : WRITE_TRACK_POST_SECTORS;
				break;
			}
			cur_live.state = WRITE_TRACK_SECTOR_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_TRACK_POST_SECTORS:
			live_write_fm(0xff);
			cur_live.state = WRITE_TRACK_POST_SECTORS_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_TRACK_PRE_SECTORS_BYTE:
		case WRITE_TRACK_SECTOR_BYTE:
		case WRITE_TRACK_POST_SECTORS_BYTE:
		case WRITE_SECTOR_DATA_BYTE:
			if(write_one_bit(limit))
				return;
			if(cur_live.bit_counter == 0) {
				cur_live.byte_counter++;
				live_delay(cur_live.state-1);
				return;
			}
			break;

		default:
			logerror("%s: Unknown live state %d\n", tts(cur_live.tm).c_str(), cur_live.state);
			return;
		}
	}
}

int i8271_device::check_command()
{
	switch(command[0] & 0x3f) {
	case 0x00:
	case 0x04:
		return command_pos == 6 ? C_SCAN               : C_INCOMPLETE;

	case 0x0a:
	case 0x0e:
		return command_pos == 3 ? C_WRITE_DATA_SINGLE   : C_INCOMPLETE;

	case 0x0b:
	case 0x0f:
		return command_pos == 4 ? C_WRITE_DATA_MULTI    : C_INCOMPLETE;

	case 0x12:
	case 0x16:
		return command_pos == 3 ? C_READ_DATA_SINGLE  : C_INCOMPLETE;

	case 0x13:
	case 0x17:
		return command_pos == 4 ? C_READ_DATA_MULTI   : C_INCOMPLETE;

	case 0x1e:
		return command_pos == 3 ? C_VERIFY_DATA_SINGLE : C_INCOMPLETE;

	case 0x1f:
		return command_pos == 4 ? C_VERIFY_DATA_MULTI  : C_INCOMPLETE;

	case 0x1b:
		return command_pos == 4 ? C_READ_ID            : C_INCOMPLETE;

	case 0x23:
		return command_pos == 6 ? C_FORMAT_TRACK       : C_INCOMPLETE;

	case 0x29:
		return command_pos == 2 ? C_SEEK               : C_INCOMPLETE;

	case 0x2c:
		return C_READ_DRIVE_STATUS;

	case 0x35:
		return command_pos == 5 ? C_SPECIFY            : C_INCOMPLETE;

	case 0x3a:
		return command_pos == 3 ? C_WRITE_SPECIAL_REGISTER : C_INCOMPLETE;

	case 0x3d:
		return command_pos == 2 ? C_READ_SPECIAL_REGISTER  : C_INCOMPLETE;

	default:
		return C_INVALID;
	}
}

void i8271_device::start_command(int cmd)
{
	command_pos = 0;
	main_phase = PHASE_EXEC;
	switch(cmd) {
	case C_READ_DATA_SINGLE:
		command[3] = 1;
	case C_READ_DATA_MULTI:
		read_data_start(flopi[BIT(command[0], 7)]);
		break;

	case C_VERIFY_DATA_SINGLE:
		command[3] = 1;
	case C_VERIFY_DATA_MULTI:
		verify_data_start(flopi[BIT(command[0], 7)]);
		break;

	case C_FORMAT_TRACK:
		format_track_start(flopi[BIT(command[0], 7)]);
		break;

	case C_READ_ID:
		read_id_start(flopi[BIT(command[0], 7)]);
		break;

	case C_SCAN:
		scan_start(flopi[BIT(command[0], 7)]);
		break;

	case C_SEEK:
		if(!command[1])
			recalibrate_start(flopi[BIT(command[0], 7)]);
		else
			seek_start(flopi[BIT(command[0], 7)]);
		break;

	case C_READ_DRIVE_STATUS:
	{
		floppy_info &fi = flopi[BIT(command[0], 7)];
		rr = (get_ready(1) ? 0x40 : 0) | (fi.dev->idx_r() ? 0x10 : 0) | (fi.dev->wpt_r() ? 0 : 8) |
				(get_ready(0) ? 4 : 0) | (fi.dev->trk00_r() ? 1 : 0);
		flopi[0].ready = true;
		flopi[1].ready = true;
		main_phase = PHASE_IDLE;
		break;
	}
	case C_SPECIFY:
		logerror("%s: command specify %02x %02x %02x %02x\n",
					tag(), command[1],
					command[2], command[3], command[4]);
		switch(command[1]) {
		case 0x0d:
			srate = command[2];
			hset = command[3];
			icnt = command[4] >> 4;
			hload = command[4] & 0xf;
			break;
		case 0x10:
		case 0x18:
		{
			floppy_info &fi = flopi[BIT(command[1], 3)];
			fi.badtrack[0] = command[2];
			fi.badtrack[1] = command[3];
			fi.pcn = command[4];
		}
		}
		main_phase = PHASE_IDLE;
		break;

	case C_WRITE_DATA_SINGLE:
		command[3] = 1;
	case C_WRITE_DATA_MULTI:
		write_data_start(flopi[BIT(command[0], 7)]);
		break;

	case C_READ_SPECIAL_REGISTER:
		switch(command[1] & 0x3f) {
		case 0x06:
			rr = scan_sec;
			break;
		case 0x10:
		case 0x11:
			rr = flopi[0].badtrack[BIT(command[1], 0)];
			break;
		case 0x12:
			rr = flopi[0].pcn;
			break;
		case 0x13:
			rr = scan_cnt[0];
			break;
		case 0x14:
			rr = scan_cnt[1];
			break;
		case 0x17:
			rr = moder;
			break;
		case 0x18:
		case 0x19:
			rr = flopi[1].badtrack[BIT(command[1], 0)];
			break;
		case 0x1a:
			rr = flopi[1].pcn;
			break;
		case 0x22: {
			floppy_info &fi = flopi[BIT(command[0], 7)];
			rr = (get_ready(1) ? 0x40 : 0) | (fi.dev->idx_r() ? 0x10 : 0) | (fi.dev->wpt_r() ? 0 : 8) |
					(get_ready(0) ? 4 : 0) | (fi.dev->trk00_r() ? 1 : 0);
			break;
		}
		case 0x23:
			rr = (command[0] & 0xc0) | oport;
			break;
		default:
			rr = 0;
			break;
		}
		main_phase = PHASE_IDLE;
	break;

	case C_WRITE_SPECIAL_REGISTER:
		switch(command[1]) {
		case 0x06:
			scan_sec = command[2];
			break;
		case 0x10:
		case 0x11:
			flopi[0].badtrack[BIT(command[1], 0)] = command[2];
			break;
		case 0x12:
			flopi[0].pcn = command[2];
			break;
		case 0x13:
			scan_cnt[0] = command[2];
			break;
		case 0x14:
			scan_cnt[1] = command[2];
			break;
		case 0x17:
			moder = command[2] | 0xc0;
			break;
		case 0x18:
		case 0x19:
			flopi[1].badtrack[BIT(command[1], 0)] = command[2];
			break;
		case 0x1a:
			flopi[1].pcn = command[2];
			break;
		case 0x22:
			break;
		case 0x23: {
			oport = command[2] & ~0xc0;
			floppy_info &fi = flopi[BIT(command[0], 7)];
			fi.dev->dir_w(BIT(command[2], 2));
			fi.dev->stp_w(BIT(command[2], 1));
			opt_cb(BIT(command[2], 5));
			hdl_cb(BIT(command[2], 3));
			break;
		}
		}
		main_phase = PHASE_IDLE;
		break;

	default:
		fprintf(stderr, "start command %d\n", cmd);
		exit(1);
	}
}

void i8271_device::command_end(floppy_info &fi, bool data_completion)
{
	logerror("%s: command done (%s) - %02x\n", tag(), data_completion ? "data" : "seek", rr);
	fi.main_state = fi.sub_state = IDLE;
	main_phase = PHASE_RESULT;
	set_irq(true);
}

void i8271_device::recalibrate_start(floppy_info &fi)
{
	logerror("%s: command recalibrate\n", tag());
	fi.main_state = RECALIBRATE;
	fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
	fi.dir = 1;
	fi.counter = 255;
	seek_continue(fi);
}

void i8271_device::seek_start(floppy_info &fi)
{
	logerror("%s: command seek %d\n", tag(), command[1]);
	fi.main_state = SEEK;
	fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
	fi.dir = fi.pcn > command[1] ? 1 : 0;
	seek_continue(fi);
}

void i8271_device::delay_cycles(emu_timer *tm, int cycles)
{
	tm->adjust(attotime::from_double(double(cycles)/cur_rate));
}

void i8271_device::seek_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case SEEK_MOVE:
			if(fi.dev) {
				fi.dev->dir_w(fi.dir);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500));
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			if(fi.dev)
				fi.dev->stp_w(1);

			do {
				if(fi.pcn > command[1])
					fi.pcn--;
				else
					fi.pcn++;
			} while((fi.pcn == fi.badtrack[0]) || (fi.pcn == fi.badtrack[1]));
			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi.tm, 500*srate);
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE: {
			bool done = false;
			switch(fi.main_state) {
			case RECALIBRATE:
				fi.counter--;
				done = fi.dev && !fi.dev->trk00_r();
				if(done)
					fi.pcn = 0;
				else if(!fi.counter) {
					rr = ERR_T0NF;
					command_end(fi, false);
					return;
				}
				break;
			case SEEK:
				done = fi.pcn == command[1];
				break;
			}
			if(done) {
				rr = ERR_NONE;
				command_end(fi, false);
				return;
			}
			fi.sub_state = SEEK_MOVE;
			break;
		}
		}
	}
}

void i8271_device::read_data_start(floppy_info &fi)
{
	fi.main_state = READ_DATA;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command read%s data%s cmd=%02x crn=(%d, %d, %d) len=%02x rate=%d\n",
				tag(),
				command[0] & 0x04 ? " deleted" : "",
				command[0] & 0x01 ? " multi" : "",
				command[0],
				command[1],
				command[2],
				calc_sector_size(command[3] >> 5),
				command[3] & 0x1f,
				cur_rate);

	rr = ERR_NONE;
	fi.ready = get_ready(BIT(command[0], 7));

	if(!fi.ready) {
		rr = ERR_NR;
		fi.sub_state = COMMAND_DONE;
		read_data_continue(fi);
		return;
	}

	read_data_continue(fi);
}

void i8271_device::scan_start(floppy_info &fi)
{
	fi.main_state = SCAN_DATA;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command scan%s data%s cmd=%02x crn=(%d, %d, %d) len=%02x rate=%d\n",
				tag(),
				command[0] & 0x04 ? " deleted" : "",
				command[0] & 0x01 ? " multi" : "",
				command[0],
				command[1],
				command[2],
				calc_sector_size(command[3] >> 5),
				command[3] & 0x1f,
				cur_rate);

	rr = ERR_NONE;
	scan_done = false;
	scan_len = 0;
	scan_match = true;
	fi.ready = get_ready(BIT(command[0], 7));

	if(!fi.ready) {
		rr = ERR_NR;
		fi.sub_state = COMMAND_DONE;
		read_data_continue(fi);
		return;
	}
	read_data_continue(fi);
}

void i8271_device::verify_data_start(floppy_info &fi)
{
	fi.main_state = VERIFY_DATA;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command verify%s data%s cmd=%02x crn=(%d, %d, %d) len=%02x rate=%d\n",
				tag(),
				command[0] & 0x04 ? " deleted" : "",
				command[0] & 0x01 ? " multi" : "",
				command[0],
				command[1],
				command[2],
				calc_sector_size(command[3] >> 5),
				command[3] & 0x1f,
				cur_rate);

	rr = ERR_NONE;
	fi.ready = get_ready(BIT(command[0], 7));

	if(!fi.ready) {
		rr = ERR_NR;
		fi.sub_state = COMMAND_DONE;
		read_data_continue(fi);
		return;
	}
	read_data_continue(fi);
}

void i8271_device::read_data_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD_DONE:
			if(fi.pcn == command[1]) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[1] ? 1 : 0);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500));
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			if(fi.dev)
				fi.dev->stp_w(1);

			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi.tm, 500*srate);
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			do {
				if(fi.pcn > command[1])
					fi.pcn--;
				else
					fi.pcn++;
			} while((fi.pcn == fi.badtrack[0]) || (fi.pcn == fi.badtrack[1]));
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(cur_live.crc) {
				rr = ERR_ICRC;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			if(!sector_matches()) {
				if(cur_live.idbuf[0] != command[1]) {
					rr = ERR_NF;
					fi.sub_state = COMMAND_DONE;
					break;
				}
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			logerror("%s: reading sector %02x %02x %02x %02x\n",
						tag(),
						cur_live.idbuf[0],
						cur_live.idbuf[1],
						cur_live.idbuf[2],
						cur_live.idbuf[3]);
			sector_size = calc_sector_size(cur_live.idbuf[3]);
			fi.sub_state = SECTOR_READ;
			live_start(fi, SEARCH_ADDRESS_MARK_DATA);
			return;

		case SCAN_ID_FAILED:
			rr = ERR_NF;
			fi.sub_state = COMMAND_DONE;
			break;

		case SECTOR_READ: {
			if(cur_live.crc) {
				rr = ERR_DCRC;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			bool done = rr || (!(command[0] & 1) && (fi.main_state != SCAN_DATA));
			if(!done)
			{
				command[3] = (command[3] & 0xe0) | ((command[3] & 0x1f) - 1);
				if(command[3] & 0x1f)
					command[2]++;
				else
					done = true;
			}
			if(!done) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			fi.sub_state = COMMAND_DONE;
			break;
		}

		case COMMAND_DONE:
			command_end(fi, true);
			return;

		default:
			logerror("%s: read sector unknown sub-state %d\n", ttsn().c_str(), fi.sub_state);
			return;
		}
	}
}

void i8271_device::write_data_start(floppy_info &fi)
{
	fi.main_state = WRITE_DATA;
	fi.sub_state = HEAD_LOAD_DONE;
	logerror("%s: command write%s data%s cmd=%02x crn=(%d, %d, %d) len=%02x rate=%d\n",
				tag(),
				command[0] & 0x04 ? " deleted" : "",
				command[0] & 0x01 ? " multi" : "",
				command[0],
				command[1],
				command[2],
				calc_sector_size(command[3] >> 5),
				command[3] & 0x1f,
				cur_rate);

	rr = ERR_NONE;
	fi.ready = get_ready(BIT(command[0], 7));

	if(!fi.ready) {
		rr = ERR_NR;
		fi.sub_state = COMMAND_DONE;
		write_data_continue(fi);
		return;
	}

	write_data_continue(fi);
}

void i8271_device::write_data_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD_DONE:
			if(fi.pcn == command[1]) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[1] ? 1 : 0);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500));
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			if(fi.dev)
				fi.dev->stp_w(1);

			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi.tm, 500*srate);
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			do {
				if(fi.pcn > command[1])
					fi.pcn--;
				else
					fi.pcn++;
			} while((fi.pcn == fi.badtrack[0]) || (fi.pcn == fi.badtrack[1]));
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(!sector_matches()) {
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				rr = ERR_ICRC;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			sector_size = calc_sector_size(cur_live.idbuf[3]);
			fi.sub_state = SECTOR_WRITTEN;
			set_drq(true);
			live_start(fi, WRITE_SECTOR_SKIP_GAP2);
			return;

		case SCAN_ID_FAILED:
			rr = ERR_NF;
			fi.sub_state = COMMAND_DONE;
			break;

		case SECTOR_WRITTEN: {
			bool done = rr || !(command[0] & 1);
			if(!done)
			{
				command[3] = (command[3] & 0xe0) | ((command[3] & 0x1f) - 1);
				if(command[3] & 0x1f)
					command[2]++;
				else
					done = true;
			}
			if(!done) {
				fi.sub_state = HEAD_LOAD_DONE;
				break;
			}
			fi.sub_state = COMMAND_DONE;
			break;
		}

		case COMMAND_DONE:
			command_end(fi, true);
			return;

		default:
			logerror("%s: write sector unknown sub-state %d\n", ttsn().c_str(), fi.sub_state);
			return;
		}
	}
}

int i8271_device::calc_sector_size(UINT8 size)
{
	return size > 7 ? 16384 : 128 << size;
}

void i8271_device::format_track_start(floppy_info &fi)
{
	fi.main_state = FORMAT_TRACK;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command format track c=%02x n=%02x sc=%02x gap3=%02x gap5=%02x gap1=%02x\n",
				tag(),
				command[1], command[3] >> 5, command[3] & 0x1f, command[2], command[4], command[5]);

	rr = ERR_NONE;

	fi.ready = get_ready(BIT(command[0], 7));

	if(!fi.ready) {
		rr = ERR_NR;
		fi.sub_state = TRACK_DONE;
		format_track_continue(fi);
		return;
	}
	sector_size = calc_sector_size(command[3] >> 5);

	format_track_continue(fi);
}

void i8271_device::format_track_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD_DONE:
			if(fi.pcn == command[1]) {
				fi.sub_state = WAIT_INDEX;
				break;
			}
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[1] ? 1 : 0);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500));
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			if(fi.dev)
				fi.dev->stp_w(1);

			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi.tm, 500*srate);
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			do {
				if(fi.pcn > command[1])
					fi.pcn--;
				else
					fi.pcn++;
			} while((fi.pcn == fi.badtrack[0]) || (fi.pcn == fi.badtrack[1]));
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			fi.sub_state = WAIT_INDEX;
			break;

		case WAIT_INDEX:
			return;

		case WAIT_INDEX_DONE:
			logerror("%s: index found, writing track\n", tag());
			fi.sub_state = TRACK_DONE;
			cur_live.pll.start_writing(machine().time());
			set_drq(true);
			live_start(fi, WRITE_TRACK_PRE_SECTORS);
			return;

		case TRACK_DONE:
			command_end(fi, true);
			return;

		default:
			logerror("%s: format track unknown sub-state %d\n", ttsn().c_str(), fi.sub_state);
			return;
		}
	}
}

void i8271_device::read_id_start(floppy_info &fi)
{
	fi.main_state = READ_ID;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command read id, rate=%d\n",
				tag(),
				cur_rate);

	rr = ERR_NONE;

	for(int i=0; i<4; i++)
		cur_live.idbuf[i] = 0x00;

	fi.ready = get_ready(command[1] & 3);

	if(!fi.ready) {
		rr = ERR_NR;
		fi.sub_state = COMMAND_DONE;
		read_id_continue(fi);
		return;
	}

	read_id_continue(fi);
}

void i8271_device::read_id_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD_DONE:
			if(fi.pcn == command[1]) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[1] ? 1 : 0);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500));
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			if(fi.dev)
				fi.dev->stp_w(1);

			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi.tm, 500*srate);
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			do {
				if(fi.pcn > command[1])
					fi.pcn--;
				else
					fi.pcn++;
			} while((fi.pcn == fi.badtrack[0]) || (fi.pcn == fi.badtrack[1]));
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(cur_live.crc) {
				rr = ERR_ICRC;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			if(!(command[3]--)) {
				fi.counter = 0;
				fi.sub_state = SCAN_ID;
			}
			else
				fi.sub_state = COMMAND_DONE;
			break;

		case SCAN_ID_FAILED:
			if(rr != ERR_DMA)
				rr = ERR_NF;
			fi.sub_state = COMMAND_DONE;
			break;

		case COMMAND_DONE:
			command_end(fi, true);
			return;

		default:
			logerror("%s: read id unknown sub-state %d\n", ttsn().c_str(), fi.sub_state);
			return;
		}
	}
}

std::string i8271_device::tts(attotime t)
{
	char buf[256];
	const char *sign = "";
	if(t.seconds() < 0) {
		t = attotime::zero-t;
		sign = "-";
	}
	int nsec = t.attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%s%04d.%03d,%03d,%03d", sign, int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

std::string i8271_device::ttsn()
{
	return tts(machine().time());
}

void i8271_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();

	floppy_info &fi = flopi[id];
	switch(fi.sub_state) {
	case SEEK_WAIT_STEP_SIGNAL_TIME:
		fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME_DONE;
		break;
	case SEEK_WAIT_STEP_TIME:
		fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
		break;
	}

	general_continue(fi);
}

void i8271_device::index_callback(floppy_image_device *floppy, int state)
{
	for(int fid=0; fid<2; fid++) {
		floppy_info &fi = flopi[fid];
		if(fi.dev != floppy)
			continue;

		if(fi.live)
			live_sync();
		fi.index = state;

		if(!state) {
			general_continue(fi);
			continue;
		}

		switch(fi.sub_state) {
		case IDLE:
		case SEEK_MOVE:
		case SEEK_WAIT_STEP_SIGNAL_TIME:
		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
		case SEEK_WAIT_STEP_TIME:
		case SEEK_WAIT_STEP_TIME_DONE:
		case HEAD_LOAD_DONE:
		case SCAN_ID_FAILED:
		case SECTOR_READ:
			break;

		case WAIT_INDEX:
			fi.sub_state = WAIT_INDEX_DONE;
			break;

		case SCAN_ID:
			fi.counter++;
			if(fi.counter == 2) {
				fi.sub_state = SCAN_ID_FAILED;
				live_abort();
			}
			break;

		case TRACK_DONE:
			live_abort();
			break;

		default:
			logerror("%s: Index pulse on unknown sub-state %d\n", ttsn().c_str(), fi.sub_state);
			break;
		}

		general_continue(fi);
	}
}


void i8271_device::general_continue(floppy_info &fi)
{
	if(fi.live && cur_live.state != IDLE) {
		live_run();
		if(cur_live.state != IDLE)
			return;
	}

	switch(fi.main_state) {
	case IDLE:
		break;

	case RECALIBRATE:
	case SEEK:
		seek_continue(fi);
		break;

	case READ_DATA:
	case SCAN_DATA:
	case VERIFY_DATA:
		read_data_continue(fi);
		break;

	case WRITE_DATA:
		write_data_continue(fi);
		break;

	case FORMAT_TRACK:
		format_track_continue(fi);
		break;

	case READ_ID:
		read_id_continue(fi);
		break;

	default:
		logerror("%s: general_continue on unknown main-state %d\n", ttsn().c_str(), fi.main_state);
		break;
	}
}

bool i8271_device::read_one_bit(const attotime &limit)
{
	int bit = cur_live.pll.get_next_bit(cur_live.tm, cur_live.fi->dev, limit);
	if(bit < 0)
		return true;
	cur_live.shift_reg = (cur_live.shift_reg << 1) | bit;
	cur_live.bit_counter++;
	if(cur_live.data_separator_phase) {
		cur_live.data_reg = (cur_live.data_reg << 1) | bit;
		if((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
		else
			cur_live.crc = cur_live.crc << 1;
	}
	cur_live.data_separator_phase = !cur_live.data_separator_phase;
	return false;
}

bool i8271_device::write_one_bit(const attotime &limit)
{
	bool bit = cur_live.shift_reg & 0x8000;
	if(cur_live.pll.write_next_bit(bit, cur_live.tm, cur_live.fi->dev, limit))
		return true;
	if(cur_live.bit_counter & 1) {
		if((cur_live.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			cur_live.crc = (cur_live.crc << 1) ^ 0x1021;
		else
			cur_live.crc = cur_live.crc << 1;
	}
	cur_live.shift_reg = cur_live.shift_reg << 1;
	cur_live.bit_counter--;
	return false;
}

void i8271_device::live_write_raw(UINT16 raw)
{
	//  logerror("write %04x %04x\n", raw, cur_live.crc);
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = raw & 1;
}

void i8271_device::live_write_fm(UINT8 fm)
{
	UINT16 raw = 0xaaaa;
	for(int i=0; i<8; i++)
		if(fm & (0x80 >> i))
			raw |= 0x4000 >> (2*i);
	cur_live.data_reg = fm;
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = fm & 1;
	//  logerror("write %02x   %04x %04x\n", fm, cur_live.crc, raw);
}

bool i8271_device::sector_matches() const
{
	if(0)
		logerror("%s: matching %02x %02x %02x - %02x %02x %02x\n", tag(),
					cur_live.idbuf[0], cur_live.idbuf[2], cur_live.idbuf[3],
					command[1], command[2], command[3] >> 5);
	return
		cur_live.idbuf[0] == command[1] &&
		cur_live.idbuf[2] == command[2] &&
		cur_live.idbuf[3] == (command[3] >> 5);
}
