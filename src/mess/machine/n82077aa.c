
#include "n82077aa.h"

const device_type N82077AA = &device_creator<n82077aa_device>;

DEVICE_ADDRESS_MAP_START(amap, 8, n82077aa_device)
	AM_RANGE(0x0, 0x0) AM_READ(sra_r)
	AM_RANGE(0x1, 0x1) AM_READ(srb_r)
	AM_RANGE(0x2, 0x2) AM_READWRITE(dor_r, dor_w)
	AM_RANGE(0x3, 0x3) AM_READWRITE(tdr_r, tdr_w)
	AM_RANGE(0x4, 0x4) AM_READWRITE(msr_r, dsr_w)
	AM_RANGE(0x5, 0x5) AM_READWRITE(fifo_r, fifo_w)
	AM_RANGE(0x7, 0x7) AM_READWRITE(dir_r, ccr_w)
ADDRESS_MAP_END

int n82077aa_device::rates[4] = { 500000, 300000, 250000, 1000000 };

n82077aa_device::n82077aa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : device_t(mconfig, N82077AA, "N82077AA", tag, owner, clock)
{
}

void n82077aa_device::set_mode(int _mode)
{
	mode = _mode;
}

void n82077aa_device::setup_intrq_cb(line_cb cb)
{
	intrq_cb = cb;
}

void n82077aa_device::setup_drq_cb(line_cb cb)
{
	drq_cb = cb;
}

void n82077aa_device::device_start()
{
	static const char *names[] = { "fd0", "fd1", "fd2", "fd3" };
	for(int i=0; i != 4; i++) {
		flopi[i].tm = timer_alloc(i);
		flopi[i].id = i;
		floppy_connector *con = machine().device<floppy_connector>(names[i]);
		if(con) {
			flopi[i].dev = con->get_device();
			flopi[i].dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(n82077aa_device::index_callback), this));
		} else
			flopi[i].dev = 0;
	}
}

void n82077aa_device::device_reset()
{
	main_phase = PHASE_CMD;
	for(int i=0; i<4; i++) {
		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].irq = false;
		flopi[i].live = false;
	}
	data_irq = false;
	dor = 0x00;
	fifo_pos = 0;
	command_pos = 0;
	result_pos = 0;
	fifocfg = 0x10;
	cur_live.fi = 0;
	cur_irq = false;
	drq = false;
	live_abort();
}

READ8_MEMBER(n82077aa_device::sra_r)
{
	UINT8 sra = 0;
	int fid = dor & 3;
	floppy_info &fi = flopi[fid];
	if(fi.dir)
		sra |= 0x01;
	if(fi.index)
		sra |= 0x04;
	if((dsr & 3) == 0 || (dsr & 3) == 3)
		sra |= 0x08;
	if(fi.dev && fi.dev->trk00_r())
		sra |= 0x10;
	if(fi.main_state == SEEK_WAIT_STEP_SIGNAL_TIME)
		sra |= 0x20;
	sra |= 0x40;
	if(cur_irq)
		sra |= 0x80;
	if(mode == MODE_M30)
		sra ^= 0x1f;
	return sra;
}

READ8_MEMBER(n82077aa_device::srb_r)
{
	return 0;
}

READ8_MEMBER(n82077aa_device::dor_r)
{
	return dor;
}

WRITE8_MEMBER(n82077aa_device::dor_w)
{
	UINT8 diff = dor ^ data;
	dor = data;
	if((diff & 4) && !(data & 4))
		device_reset();
	else
		for(int i=0; i<4; i++) {
			floppy_info &fi = flopi[i];
			if(fi.dev && (diff & (0x10 << i)))
				fi.dev->mon_w(!(dor & (0x10 << i)));
		}

}

READ8_MEMBER(n82077aa_device::tdr_r)
{
	return 0;
}

WRITE8_MEMBER(n82077aa_device::tdr_w)
{
}

READ8_MEMBER(n82077aa_device::msr_r)
{
	UINT32 msr = 0;
	switch(main_phase) {
	case PHASE_CMD:
		msr |= 0x80;
		if(command_pos)
			msr |= 0x10;
		break;
	case PHASE_EXEC:
		msr |= 0x10;
		break;

	case PHASE_RESULT:
		msr |= 0xd0;
		break;
	}
	for(int i=0; i<4; i++)
		if(flopi[i].main_state == RECALIBRATE || flopi[i].main_state == SEEK)
			msr |= 1<<i;

	if(data_irq) {
		data_irq = false;
		check_irq();
	}

	return msr;
}

WRITE8_MEMBER(n82077aa_device::dsr_w)
{
	dsr = data;
}

READ8_MEMBER(n82077aa_device::fifo_r)
{
	UINT8 r = 0;
	switch(main_phase) {
	case PHASE_RESULT:
		r = result[0];
		result_pos--;
		memmove(result, result+1, result_pos);
		if(!result_pos)
			main_phase = PHASE_CMD;
		break;
	default:
		logerror("%s: fifo_r in phase %d\n", tag(), main_phase);
		//      exit(1);
	}

	return r;
}

WRITE8_MEMBER(n82077aa_device::fifo_w)
{
	switch(main_phase) {
	case PHASE_CMD: {
		command[command_pos++] = data;
		int cmd = check_command();
		if(cmd == C_INCOMPLETE)
			break;
		if(cmd == C_INVALID) {
			logerror("%s: Invalid on %02x\n", tag(), command[0]);
			exit(1);
			command_pos = 0;
			return;
		}
		start_command(cmd);
		break;
	}
	default:
		logerror("%s: fifo_w in phase %d\n", tag(), main_phase);
		exit(1);
	}
}

READ8_MEMBER(n82077aa_device::dir_r)
{
	return 0x78;
}

WRITE8_MEMBER(n82077aa_device::ccr_w)
{
	dsr = (dsr & 0xfc) | (data & 3);
}

void n82077aa_device::set_drq(bool state)
{
	if(state != drq) {
		drq = state;
		if(!drq_cb.isnull())
			drq_cb(drq);
	}
}

bool n82077aa_device::get_drq() const
{
	return drq;
}

void n82077aa_device::fifo_push(UINT8 data)
{
	if(fifo_pos == 16) {
		logerror("%s: Overflow\n", tag());
		return;
	}
	fifo[fifo_pos++] = data;
	fifo_expected--;
	if(!drq) {
		int thr = (fifocfg & 15)+1;
		if(!fifo_expected || fifo_pos >= thr || (fifocfg & 0x20))
			set_drq(true);
	}
}

void n82077aa_device::fifo_expect(int size, bool write)
{
	fifo_expected = size;
	fifo_write = write;
}

UINT8 n82077aa_device::dma_r()
{
	if(!fifo_pos)
		return 0;
	UINT8 r = fifo[0];
	fifo_pos--;
	memmove(fifo, fifo+1, fifo_pos);
	if(!fifo_pos)
		set_drq(false);
	return r;
}

void n82077aa_device::live_start(floppy_info &fi, int state)
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
	cur_live.pll.set_clock(attotime::from_hz(rates[dsr & 3]*2));
	checkpoint_live = cur_live;
	fi.live = true;

	live_run();
}

void n82077aa_device::checkpoint()
{
	if(cur_live.fi)
		cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
	checkpoint_live = cur_live;
}

void n82077aa_device::rollback()
{
	cur_live = checkpoint_live;
}

void n82077aa_device::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		cur_live.fi->tm->adjust(cur_live.tm - machine().time());
}

void n82077aa_device::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
		}
		if(cur_live.tm == machine().time()) {
			cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				cur_live.tm = attotime::never;
				cur_live.fi->live = false;
				cur_live.fi = 0;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void n82077aa_device::live_abort()
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

void n82077aa_device::live_run(attotime limit)
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
			fprintf(stderr, "%s: shift = %04x data=%02x c=%d\n", tts(cur_live.tm).cstr(), cur_live.shift_reg,
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

			if(cur_live.shift_reg == 0x4489) {
				cur_live.crc = 0x443b;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_HEADER_BLOCK_HEADER;
			}
			break;

		case READ_HEADER_BLOCK_HEADER: {
			if(read_one_bit(limit))
				return;
#if 0
			fprintf(stderr, "%s: shift = %04x data=%02x counter=%d\n", tts(cur_live.tm).cstr(), cur_live.shift_reg,
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
			if(cur_live.bit_counter & 15)
				break;

			int slot = cur_live.bit_counter >> 4;

			if(slot < 3) {
				if(cur_live.shift_reg != 0x4489)
					cur_live.state = SEARCH_ADDRESS_MARK_HEADER;
				break;
			}
			if(cur_live.data_reg != 0xfe) {
				cur_live.state = SEARCH_ADDRESS_MARK_HEADER;
				break;
			}

			cur_live.bit_counter = 0;
			cur_live.state = READ_ID_BLOCK_TO_LOCAL;

			break;
		}

		case READ_ID_BLOCK_TO_LOCAL: {
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			int slot = (cur_live.bit_counter >> 4)-1;
			cur_live.idbuf[slot] = cur_live.data_reg;
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
			fprintf(stderr, "%s: shift = %04x data=%02x c=%d.%x\n", tts(cur_live.tm).cstr(), cur_live.shift_reg,
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
			if(cur_live.bit_counter > 43*16) {
				live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
				return;
			}

			if(cur_live.bit_counter >= 28*16 && cur_live.shift_reg == 0x4489) {
				cur_live.crc = 0x443b;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_DATA_BLOCK_HEADER;
			}
			break;

		case READ_DATA_BLOCK_HEADER: {
			if(read_one_bit(limit))
				return;
#if 0
			fprintf(stderr, "%s: shift = %04x data=%02x counter=%d\n", tts(cur_live.tm).cstr(), cur_live.shift_reg,
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
			if(cur_live.bit_counter & 15)
				break;

			int slot = cur_live.bit_counter >> 4;

			if(slot < 3) {
				if(cur_live.shift_reg != 0x4489) {
					live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
					return;
				}
				break;
			}
			if(cur_live.data_reg != 0xfb && cur_live.data_reg != 0xfd) {
				live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
				return;
			}

			cur_live.bit_counter = 0;
			cur_live.state = READ_SECTOR_DATA;
			break;
		}

		case SEARCH_ADDRESS_MARK_DATA_FAILED:
			//          status |= S_RNF;
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
			fifo_push(cur_live.data_reg);
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		default:
			logerror("%s: Unknown live state %d\n", tts(cur_live.tm).cstr(), cur_live.state);
			return;
		}
	}
}

int n82077aa_device::check_command()
{
	// 0.000010 read track
	// 00000011 specify
	// 00000100 sense drive status
	// ..000101 write data
	// ...00110 read data
	// 00000111 recalibrate
	// 00001000 sense interrupt status
	// ..001001 write deleted data
	// 0.001010 read id
	// ...01100 read deleted data
	// 0.001101 format track
	// 00001110 dumpreg
	// 00001111 seek
	// 00010000 version
	// ...10001 scan equal
	// 00010010 perpendicular mode
	// 00010011 configure
	// .0010100 lock
	// ...10110 verify
	// ...11001 scan low or equal
	// ...11101 scan high or equal
	// 1.001111 relative seek

	switch(command[0]) {
	case 0x03:
		return command_pos == 3 ? C_SPECIFY     : C_INCOMPLETE;

	case 0x06: case 0x26: case 0x46: case 0x66: case 0x86: case 0xa6: case 0xc6: case 0xe6:
		return command_pos == 9 ? C_READ_DATA   : C_INCOMPLETE;

	case 0x07:
		return command_pos == 2 ? C_RECALIBRATE : C_INCOMPLETE;

	case 0x08:
		return C_SENSE_INTERRUPT_STATUS;

	case 0x0a: case 0x4a:
		return command_pos == 2 ? C_READ_ID     : C_INCOMPLETE;

	case 0x0f:
		return command_pos == 3 ? C_SEEK        : C_INCOMPLETE;

	case 0x12:
		return command_pos == 2 ? C_PERPENDICULAR : C_INCOMPLETE;

	case 0x13:
		return command_pos == 4 ? C_CONFIGURE   : C_INCOMPLETE;

	default:
		return C_INVALID;
	}
}

void n82077aa_device::start_command(int cmd)
{
	command_pos = 0;
	result_pos = 0;
	main_phase = PHASE_EXEC;
	switch(cmd) {
	case C_CONFIGURE:
		logerror("%s: command configure %02x %02x %02x\n",
				 tag(),
				 command[1], command[2], command[3]);
		// byte 1 is ignored, byte 3 is precompensation-related
		fifocfg = command[2];
		main_phase = PHASE_CMD;
		break;

	case C_PERPENDICULAR:
		logerror("%s: command perpendicular\n", tag());
		main_phase = PHASE_CMD;
		break;

	case C_READ_DATA:
		read_data_start(flopi[command[1] & 3]);
		break;

	case C_READ_ID:
		read_id_start(flopi[command[1] & 3]);
		break;

	case C_RECALIBRATE:
		recalibrate_start(flopi[command[1] & 3]);
		main_phase = PHASE_CMD;
		break;

	case C_SEEK:
		seek_start(flopi[command[1] & 3]);
		main_phase = PHASE_CMD;
		break;

	case C_SENSE_INTERRUPT_STATUS: {
		logerror("%s: command sense interrupt status\n", tag());
		main_phase = PHASE_RESULT;

		int fid;
		for(fid=0; fid<4 && !flopi[fid].irq; fid++);
		if(fid == 4) {
			result[0] = 0x80;
			result_pos = 1;
			break;
		}
		floppy_info &fi = flopi[fid];
		fi.irq = false;
		result[0] = (fi.status << 6) | 0x20 | fid;
		result[1] = fi.pcn;
		result_pos = 2;
		check_irq();
		break;
	}

	case C_SPECIFY:
		logerror("%s command specify %02x %02x\n",
				 tag(),
				 command[1], command[2]);
		spec = (command[1] << 8) | command[2];
		main_phase = PHASE_CMD;
		break;

	default:
		fprintf(stderr, "start command %d\n", cmd);
		exit(1);
	}
}

void n82077aa_device::command_end(floppy_info &fi, bool data_completion, int status)
{
	fi.main_state = fi.sub_state = IDLE;
	if(data_completion)
		data_irq = true;
	else
		fi.irq = true;
	fi.status = status;
	check_irq();
}

void n82077aa_device::recalibrate_start(floppy_info &fi)
{
	logerror("%s: command recalibrate\n", tag());
	fi.main_state = RECALIBRATE;
	fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
	fi.dir = 1;
	seek_continue(fi);
}

void n82077aa_device::seek_start(floppy_info &fi)
{
	logerror("%s: command seek %d\n", tag(), command[2]);
	fi.main_state = SEEK;
	fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
	fi.dir = fi.pcn > command[2] ? 1 : 0;
	seek_continue(fi);
}

void n82077aa_device::delay_cycles(emu_timer *tm, int cycles)
{
	tm->adjust(attotime::from_double(double(cycles)/rates[dsr & 3]));
}

void n82077aa_device::seek_continue(floppy_info &fi)
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

			if(fi.main_state == SEEK) {
				if(fi.pcn > command[2])
					fi.pcn--;
				else
					fi.pcn++;
			}
			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi.tm, 500*(16-(spec >> 12)));
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE: {
			bool done = false;
			switch(fi.main_state) {
			case RECALIBRATE:
				done = !fi.dev || !fi.dev->trk00_r();
				if(done)
					fi.pcn = 0;
				break;
			case SEEK:
				done = fi.pcn == command[2];
				break;
			}
			if(done) {
				command_end(fi, false, 0);
				return;
			}
			fi.sub_state = SEEK_MOVE;
			break;
		}
		}
	}
}

void n82077aa_device::read_data_start(floppy_info &fi)
{
	fi.main_state = READ_DATA;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command read data%s%s%s cmd=%02x sel=%x chrn=(%d, %d, %d, %d) eot=%02x gpl=%02x dtl=%02x rate=%d\n",
			 tag(),
			 command[0] & 0x80 ? " mt" : "",
			 command[0] & 0x40 ? " mfm" : "",
			 command[0] & 0x20 ? " sk" : "",
			 command[0],
			 command[1],
			 command[2],
			 command[3],
			 command[4],
			 128 << (command[5] & 7),
			 command[6],
			 command[7],
			 command[8],
			 rates[dsr & 3]);

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);
	read_data_continue(fi);
}

void n82077aa_device::read_data_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD_DONE:
			if(fi.pcn == command[2] || !(fifocfg & 0x40)) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[2] ? 1 : 0);
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
			delay_cycles(fi.tm, 500*(16-(spec >> 12)));
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			if(fi.pcn > command[2])
				fi.pcn--;
			else
				fi.pcn++;
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			counter = 0;
			fi.sub_state = SCAN_ID;
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(!sector_matches()) {
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				fprintf(stderr, "Header CRC error\n");
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			sector_size = 128 << (cur_live.idbuf[3] & 3);
			fifo_expect(sector_size, false);
			fi.sub_state = SECTOR_READ;
			live_start(fi, SEARCH_ADDRESS_MARK_DATA);
			return;

		case SCAN_ID_FAILED:
			fprintf(stderr, "RNF\n");
			command_end(fi, true, 1);
			return;

		case SECTOR_READ:
			if(cur_live.crc) {
				fprintf(stderr, "CRC error\n");
			}
			if(command[4] < command[6]) {
				command[4]++;
				fi.sub_state = HEAD_LOAD_DONE;
				break;
			}

			main_phase = PHASE_RESULT;
			result[0] = 0;
			result[1] = 0;
			result[2] = 0;
			result[3] = command[2];
			result[4] = command[3];
			result[5] = command[4];
			result[6] = command[5];
			result_pos = 7;
			command_end(fi, true, 0);
			return;

		default:
			logerror("%s: read sector unknown sub-state %d\n", ttsn().cstr(), fi.sub_state);
			return;
		}
	}
}

void n82077aa_device::read_id_start(floppy_info &fi)
{
	fi.main_state = READ_DATA;
	fi.sub_state = HEAD_LOAD_DONE;

	logerror("%s: command read id%s, rate=%d\n",
			 tag(),
			 command[0] & 0x40 ? " mfm" : "",
			 rates[dsr & 3]);

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);
	read_id_continue(fi);
}

void n82077aa_device::read_id_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD_DONE:
			counter = 0;
			fi.sub_state = SCAN_ID;
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(cur_live.crc) {
				fprintf(stderr, "Header CRC error\n");
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}

			main_phase = PHASE_RESULT;
			result[0] = 0;
			result[1] = 0;
			result[2] = 0;
			result[3] = cur_live.idbuf[0];
			result[4] = cur_live.idbuf[1];
			result[5] = cur_live.idbuf[2];
			result[6] = cur_live.idbuf[3];
			result_pos = 7;
			command_end(fi, true, 0);
			return;

		default:
			logerror("%s: read id unknown sub-state %d\n", ttsn().cstr(), fi.sub_state);
			return;
		}
	}
}

void n82077aa_device::check_irq()
{
	bool old_irq = cur_irq;
	cur_irq = data_irq;
	for(int i=0; i<4; i++)
		cur_irq = cur_irq || flopi[i].irq;
	if(cur_irq != old_irq && !intrq_cb.isnull())
		intrq_cb(cur_irq);
}

astring n82077aa_device::tts(attotime t)
{
	char buf[256];
	int nsec = t.attoseconds / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%4d.%03d,%03d,%03d", int(t.seconds), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

astring n82077aa_device::ttsn()
{
	return tts(machine().time());
}

void n82077aa_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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

void n82077aa_device::index_callback(floppy_image_device *floppy, int state)
{
	int fid;
	for(fid=0; fid<4; fid++)
		if(flopi[fid].dev == floppy)
			break;

	assert(fid != 4);
	floppy_info &fi = flopi[fid];

	live_sync();
	fi.index = state;

	if(!state) {
		general_continue(fi);
		return;
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

	case SCAN_ID:
		counter++;
		if(counter == 2)
			fi.sub_state = SCAN_ID_FAILED;
		break;

	default:
		logerror("%s: Index pulse on unknown sub-state %d\n", ttsn().cstr(), fi.sub_state);
		break;
	}

	general_continue(fi);
}


void n82077aa_device::general_continue(floppy_info &fi)
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
		read_data_continue(fi);
		break;

	case READ_ID:
		read_id_continue(fi);
		break;

	default:
		logerror("%s: general_continue on unknown main-state %d\n", ttsn().cstr(), fi.main_state);
		break;
	}
}

bool n82077aa_device::read_one_bit(attotime limit)
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

bool n82077aa_device::write_one_bit(attotime limit)
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

bool n82077aa_device::sector_matches() const
{
	return
		cur_live.idbuf[0] == command[2] &&
		cur_live.idbuf[1] == command[3] &&
		cur_live.idbuf[2] == command[4] &&
		cur_live.idbuf[3] == command[5];
}

void n82077aa_device::pll_t::set_clock(attotime _period)
{
	period = _period;
	period_adjust_base = period * 0.05;
	min_period = period * 0.75;
	max_period = period * 1.25;
}

void n82077aa_device::pll_t::reset(attotime when)
{
	ctime = when;
	phase_adjust = attotime::zero;
	freq_hist = 0;
	write_position = 0;
	write_start_time = attotime::never;
}

void n82077aa_device::pll_t::start_writing(attotime tm)
{
	write_start_time = tm;
	write_position = 0;
}

void n82077aa_device::pll_t::stop_writing(floppy_image_device *floppy, attotime tm)
{
	commit(floppy, tm);
	write_start_time = attotime::never;
}

void n82077aa_device::pll_t::commit(floppy_image_device *floppy, attotime tm)
{
	if(write_start_time.is_never() || tm == write_start_time)
		return;

	if(floppy)
		floppy->write_flux(write_start_time, tm, write_position, write_buffer);
	write_start_time = tm;
	write_position = 0;
}

int n82077aa_device::pll_t::get_next_bit(attotime &tm, floppy_image_device *floppy, attotime limit)
{
	attotime edge = floppy ? floppy->get_next_transition(ctime) : attotime::never;

	attotime next = ctime + period + phase_adjust;

#if 0
	if(!edge.is_never())
		fprintf(stderr, "transition_time=%s, next=%s\n", tts(edge).cstr(), tts(next).cstr());
#endif

	if(next > limit)
		return -1;

	ctime = next;

	if(edge.is_never() || edge >= next) {
		// No transition in the window means 0 and pll in free run mode
		phase_adjust = attotime::zero;
		tm = next;
		return 0;
	}

	// Transition in the window means 1, and the pll is adjusted

	attotime delta = edge - (next - period/2);

	phase_adjust = 0.65*delta;

	if(delta < attotime::zero) {
		if(freq_hist < 0)
			freq_hist--;
		else
			freq_hist = -1;
	} else if(delta > attotime::zero) {
		if(freq_hist > 0)
			freq_hist++;
		else
			freq_hist = 1;
	} else
		freq_hist = 0;

	if(freq_hist) {
		int afh = freq_hist < 0 ? -freq_hist : freq_hist;
		if(afh > 1) {
			attotime aper = attotime::from_double(period_adjust_base.as_double()*delta.as_double()/period.as_double());
			period += aper;

			if(period < min_period)
				period = min_period;
			else if(period > max_period)
				period = max_period;
		}
	}

	return 1;
}

bool n82077aa_device::pll_t::write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, attotime limit)
{
	if(write_start_time.is_never()) {
		write_start_time = ctime;
		write_position = 0;
	}

	attotime etime = ctime + period;
	if(etime > limit)
		return true;

	if(write_position < ARRAY_LENGTH(write_buffer))
		write_buffer[write_position++] = ctime + period/2;

	tm = etime;
	ctime = etime;
	return false;
}
