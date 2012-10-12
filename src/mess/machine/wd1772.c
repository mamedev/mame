/***************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "wd1772.h"

#include "debugger.h"

const device_type WD1770x = &device_creator<wd1770_t>;
const device_type WD1772x = &device_creator<wd1772_t>;
const device_type WD1773x = &device_creator<wd1773_t>;

wd177x_t::wd177x_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}

void wd177x_t::device_start()
{
	t_gen = timer_alloc(TM_GEN);
	t_cmd = timer_alloc(TM_CMD);
	t_track = timer_alloc(TM_TRACK);
	t_sector = timer_alloc(TM_SECTOR);
	dden = false;
	floppy = 0;

	save_item(NAME(status));
	save_item(NAME(command));
	save_item(NAME(main_state));
	save_item(NAME(sub_state));
	save_item(NAME(track));
	save_item(NAME(sector));
	save_item(NAME(cmd_buffer));
	save_item(NAME(track_buffer));
	save_item(NAME(sector_buffer));
	save_item(NAME(counter));
	save_item(NAME(status_type_1));
	save_item(NAME(last_dir));
}

void wd177x_t::device_reset()
{
	command = 0x00;
	main_state = IDLE;
	sub_state = IDLE;
	cur_live.state = IDLE;
	track = 0x00;
	sector = 0x00;
	status = 0x00;
	data = 0x00;
	cmd_buffer = track_buffer = sector_buffer = -1;
	counter = 0;
	status_type_1 = true;
	last_dir = 1;
	intrq = false;
	drq = false;
	live_abort();
}

void wd177x_t::set_floppy(floppy_image_device *_floppy)
{
	if(floppy == _floppy)
		return;

	if(floppy) {
		floppy->mon_w(1);
		floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
	}

	floppy = _floppy;

	if(floppy) {
		if(has_motor())
			floppy->mon_w(status & S_MON ? 0 : 1);
		floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(wd177x_t::index_callback), this));
	}
}

void wd177x_t::setup_intrq_cb(line_cb cb)
{
	intrq_cb = cb;
}

void wd177x_t::setup_drq_cb(line_cb cb)
{
	drq_cb = cb;
}

void wd177x_t::dden_w(bool _dden)
{
	dden = _dden;
}

astring wd177x_t::tts(attotime t)
{
	char buf[256];
	int nsec = t.attoseconds / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%4d.%03d,%03d,%03d", int(t.seconds), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

astring wd177x_t::ttsn()
{
	return tts(machine().time());
}

void wd177x_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();

	switch(id) {
	case TM_GEN: do_generic(); break;
	case TM_CMD: do_cmd_w(); break;
	case TM_TRACK: do_track_w(); break;
	case TM_SECTOR: do_sector_w(); break;
	}

	general_continue();
}

void wd177x_t::command_end()
{
	main_state = sub_state = IDLE;
	status &= ~S_BUSY;
	intrq = true;
	motor_timeout = 0;
	if(!intrq_cb.isnull())
		intrq_cb(intrq);
}

void wd177x_t::seek_start(int state)
{
	main_state = state;
	status = (status & ~(S_CRC|S_RNF|S_SPIN)) | S_BUSY;
	sub_state = has_motor() ? SPINUP : SPINUP_DONE;
	status_type_1 = true;
	seek_continue();
}

void wd177x_t::seek_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			if(!(command & 0x08))
				status |= S_SPIN;
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			return;

		case SPINUP_DONE:
			if(main_state == RESTORE && floppy && !floppy->trk00_r())
				sub_state = SEEK_DONE;

			if(main_state == SEEK && track == data)
				sub_state = SEEK_DONE;

			if(sub_state == SPINUP_DONE) {
				counter = 0;
				sub_state = SEEK_MOVE;
			}
			break;

		case SEEK_MOVE:
			if(floppy) {
				floppy->dir_w(last_dir);
				floppy->stp_w(0);
				floppy->stp_w(1);

				// When stepping with update, the track register is updated before seeking.
				// Important for the sam coupe format code.
				if(main_state == STEP && (command & 0x10))
					track += last_dir ? -1 : 1;
			}
			counter++;
			sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(t_gen, step_time(command & 3));
			return;

		case SEEK_WAIT_STEP_TIME:
			return;

		case SEEK_WAIT_STEP_TIME_DONE: {
			bool done = false;
			switch(main_state) {
			case RESTORE:
				done = !floppy || !floppy->trk00_r();
				break;
			case SEEK:
				track += last_dir ? -1 : 1;
				done = track == data;
				break;
			case STEP:
				done = true;
				break;
			}

			if(done || counter == 255) {
				if(main_state == RESTORE)
					track = 0;

				if(command & 0x04) {
					sub_state = SEEK_WAIT_STABILIZATION_TIME;
					delay_cycles(t_gen, 120000);
					return;
				} else
					sub_state = SEEK_DONE;

			} else
				sub_state = SEEK_MOVE;

			break;
		}

		case SEEK_WAIT_STABILIZATION_TIME:
			return;

		case SEEK_WAIT_STABILIZATION_TIME_DONE:
			sub_state = SEEK_DONE;
			break;

		case SEEK_DONE:
			if(command & 0x04) {
				sub_state = SCAN_ID;
				counter = 0;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			command_end();
			return;

		case SCAN_ID:
			if(cur_live.idbuf[0] != track) {
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				status |= S_CRC;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			command_end();
			return;

		case SCAN_ID_FAILED:
			status |= S_RNF;
			command_end();
			return;

		default:
			logerror("%s: seek unknown sub-state %d\n", ttsn().cstr(), sub_state);
			return;
		}
	}
}

bool wd177x_t::sector_matches() const
{
	if(cur_live.idbuf[0] != track || cur_live.idbuf[2] != sector)
		return false;
	if(!has_side_check())
		return true;
	if(command & 8)
		return cur_live.idbuf[1] & 1;
	else
		return !(cur_live.idbuf[1] & 1);
}

void wd177x_t::read_sector_start()
{
	main_state = READ_SECTOR;
	status = (status & ~(S_CRC|S_LOST|S_RNF|S_WP|S_DDM)) | S_BUSY;
	drop_drq();
	sub_state = has_motor() ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	read_sector_continue();
}

void wd177x_t::read_sector_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			return;

		case SPINUP_DONE:
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			return;

		case SETTLE_DONE:
			sub_state = SCAN_ID;
			counter = 0;
			live_start(SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(!sector_matches()) {
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				status |= S_CRC;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			sector_size = 128 << (cur_live.idbuf[3] & 3);
			sub_state = SECTOR_READ;
			live_start(SEARCH_ADDRESS_MARK_DATA);
			return;

		case SCAN_ID_FAILED:
			status |= S_RNF;
			command_end();
			return;

		case SECTOR_READ:
			if(cur_live.crc)
				status |= S_CRC;

			if(command & 0x10 && !(status & S_RNF)) {
				sector++;
				sub_state = SETTLE_DONE;
			} else {
				command_end();
				return;
			}
			break;

		default:
			logerror("%s: read sector unknown sub-state %d\n", ttsn().cstr(), sub_state);
			return;
		}
	}
}

void wd177x_t::read_track_start()
{
	main_state = READ_TRACK;
	status = (status & ~(S_LOST|S_RNF)) | S_BUSY;
	drop_drq();
	sub_state = has_motor() ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	read_track_continue();
}

void wd177x_t::read_track_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			return;

		case SPINUP_DONE:
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;

			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			return;

		case SETTLE_DONE:
			sub_state = WAIT_INDEX;
			return;

		case WAIT_INDEX:
			return;

		case WAIT_INDEX_DONE:
			sub_state = TRACK_DONE;
			live_start(READ_TRACK_DATA);
			return;

		case TRACK_DONE:
			command_end();
			return;

		default:
			logerror("%s: read track unknown sub-state %d\n", ttsn().cstr(), sub_state);
			return;
		}
	}
}

void wd177x_t::read_id_start()
{
	main_state = READ_ID;
	status = (status & ~(S_WP|S_DDM|S_LOST|S_RNF)) | S_BUSY;
	drop_drq();
	sub_state = has_motor() ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	read_id_continue();
}

void wd177x_t::read_id_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			return;

		case SPINUP_DONE:
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			return;

		case SETTLE_DONE:
			sub_state = SCAN_ID;
			counter = 0;
			live_start(SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			command_end();
			return;

		case SCAN_ID_FAILED:
			status |= S_RNF;
			command_end();
			return;

		default:
			logerror("%s: read id unknown sub-state %d\n", ttsn().cstr(), sub_state);
			return;
		}
	}
}

void wd177x_t::write_track_start()
{
	main_state = WRITE_TRACK;
	status = (status & ~(S_WP|S_DDM|S_LOST|S_RNF)) | S_BUSY;
	drop_drq();
	sub_state = has_motor() ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	write_track_continue();
}

void wd177x_t::write_track_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			return;

		case SPINUP_DONE:
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			return;

		case SETTLE_DONE:
			set_drq();
			sub_state = DATA_LOAD_WAIT;
			delay_cycles(t_gen, 768);
			return;

		case DATA_LOAD_WAIT:
			return;

		case DATA_LOAD_WAIT_DONE:
			if(drq) {
				status |= S_LOST;
				command_end();
				return;
			}
			sub_state = WAIT_INDEX;
			break;

		case WAIT_INDEX:
			return;

		case WAIT_INDEX_DONE:
			sub_state = TRACK_DONE;
			live_start(WRITE_TRACK_DATA);
			cur_live.pll.start_writing(machine().time());
			return;

		case TRACK_DONE:
			command_end();
			return;

		default:
			logerror("%s: write track unknown sub-state %d\n", ttsn().cstr(), sub_state);
			return;
		}
	}
}


void wd177x_t::write_sector_start()
{
	main_state = WRITE_SECTOR;
	status = (status & ~(S_CRC|S_LOST|S_RNF|S_WP|S_DDM)) | S_BUSY;
	drop_drq();
	sub_state = has_motor() ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	write_sector_continue();
}

void wd177x_t::write_sector_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			return;

		case SPINUP_DONE:
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			return;

		case SETTLE_DONE:
			sub_state = SCAN_ID;
			counter = 0;
			live_start(SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			if(!sector_matches()) {
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				status |= S_CRC;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			sector_size = 128 << (cur_live.idbuf[3] & 3);
			sub_state = SECTOR_WRITE;
			live_start(WRITE_SECTOR_PRE);
			return;

		case SCAN_ID_FAILED:
			status |= S_RNF;
			command_end();
			return;

		case SECTOR_WRITE:
			if(command & 0x10) {
				sector++;
				sub_state = SPINUP_DONE;
			} else {
				command_end();
				return;
			}
			break;

		default:
			logerror("%s: write sector unknown sub-state %d\n", ttsn().cstr(), sub_state);
			return;
		}
	}
}

void wd177x_t::interrupt_start()
{
	if(status & S_BUSY) {
		main_state = sub_state = cur_live.state = IDLE;
		cur_live.tm = attotime::never;
		status &= ~S_BUSY;
		drop_drq();
		motor_timeout = 0;
	}
	if(command & 0x0f) {
		logerror("%s: unhandled interrupt generation (%02x)\n", ttsn().cstr(), command);
	}
}

void wd177x_t::general_continue()
{
	if(cur_live.state != IDLE) {
		live_run();
		if(cur_live.state != IDLE)
			return;
	}

	switch(main_state) {
	case IDLE:
		break;
	case RESTORE: case SEEK: case STEP:
		seek_continue();
		break;
	case READ_SECTOR:
		read_sector_continue();
		break;
	case READ_TRACK:
		read_track_continue();
		break;
	case READ_ID:
		read_id_continue();
		break;
	case WRITE_TRACK:
		write_track_continue();
		break;
	case WRITE_SECTOR:
		write_sector_continue();
		break;
	default:
		logerror("%s: general_continue on unknown main-state %d\n", ttsn().cstr(), main_state);
		break;
	}
}

void wd177x_t::do_generic()
{
	switch(sub_state) {
	case IDLE:
	case SCAN_ID:
	case SECTOR_READ:
		break;

	case SETTLE_WAIT:
		sub_state = SETTLE_DONE;
		break;

	case SEEK_WAIT_STEP_TIME:
		sub_state = SEEK_WAIT_STEP_TIME_DONE;
		break;

	case SEEK_WAIT_STABILIZATION_TIME:
		sub_state = SEEK_WAIT_STABILIZATION_TIME_DONE;
		break;

	case DATA_LOAD_WAIT:
		sub_state = DATA_LOAD_WAIT_DONE;
		break;

	default:
		if(cur_live.tm.is_never())
			logerror("%s: do_generic on unknown sub-state %d\n", ttsn().cstr(), sub_state);
		break;
	}
}

void wd177x_t::do_cmd_w()
{
	//  fprintf(stderr, "%s: command %02x\n", ttsn().cstr(), cmd_buffer);

	// Only available command when busy is interrupt
	if(main_state != IDLE && (cmd_buffer & 0xf0) != 0xd0) {
		cmd_buffer = -1;
		return;
	}
	command = cmd_buffer;
	cmd_buffer = -1;

	switch(command & 0xf0) {
	case 0x00: logerror("wd1772: restore\n"); last_dir = 1; seek_start(RESTORE); break;
	case 0x10: logerror("wd1772: seek %d\n", data); last_dir = data > track ? 0 : 1; seek_start(SEEK); break;
	case 0x20: case 0x30: logerror("wd1772: step\n"); seek_start(STEP); break;
	case 0x40: case 0x50: logerror("wd1772: step +\n"); last_dir = 0; seek_start(STEP); break;
	case 0x60: case 0x70: logerror("wd1772: step -\n"); last_dir = 1; seek_start(STEP); break;
	case 0x80: case 0x90: logerror("wd1772: read sector%s %d, %d\n", command & 0x10 ? " multiple" : "", track, sector); read_sector_start(); break;
	case 0xa0: case 0xb0: logerror("wd1772: write sector%s %d, %d\n", command & 0x10 ? " multiple" : "", track, sector); write_sector_start(); break;
	case 0xc0: logerror("wd1772: read id\n"); read_id_start(); break;
	case 0xd0: logerror("wd1772: interrupt\n"); interrupt_start(); break;
	case 0xe0: logerror("wd1772: read track %d\n", track); read_track_start(); break;
	case 0xf0: logerror("wd1772: write track %d\n", track); write_track_start(); break;
	}
}

void wd177x_t::cmd_w(UINT8 val)
{
	if(intrq && ((command & 0xf8) != 0xd8)) {
		intrq = false;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}

	// No more than one write in flight
	if(cmd_buffer != -1)
		return;

	cmd_buffer = val;

	delay_cycles(t_cmd, dden ? 384 : 184);
}

UINT8 wd177x_t::status_r()
{
	if(intrq && ((command & 0xf8) != 0xd8)) {
		intrq = false;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}

	if(status_type_1) {
		status &= ~(S_TR00|S_WP);
		if(floppy) {
			if(floppy->wpt_r())
				status |= S_WP;
			if(!floppy->trk00_r())
				status |= S_TR00;
		}
	}

	if(main_state == IDLE || status_type_1) {
		if(floppy && floppy->idx_r())
			status |= S_IP;
		else
			status &= ~S_IP;
	} else {
		if(drq)
			status |= S_DRQ;
		else
			status &= ~S_DRQ;
	}

	return status;
}

void wd177x_t::do_track_w()
{
	track = track_buffer;
	track_buffer = -1;
}

void wd177x_t::track_w(UINT8 val)
{
	// No more than one write in flight
	if(track_buffer != -1)
		return;

	track_buffer = val;
	delay_cycles(t_track, dden ? 256 : 128);
}

UINT8 wd177x_t::track_r()
{
	return track;
}

void wd177x_t::do_sector_w()
{
	sector = sector_buffer;
	sector_buffer = -1;
}

void wd177x_t::sector_w(UINT8 val)
{
	// No more than one write in flight
	if(sector_buffer != -1)
		return;

	sector_buffer = val;
	delay_cycles(t_sector, dden ? 256 : 128);
}

UINT8 wd177x_t::sector_r()
{
	return sector;
}

void wd177x_t::data_w(UINT8 val)
{
	data = val;
	drop_drq();
}

UINT8 wd177x_t::data_r()
{
	drop_drq();
	return data;
}

void wd177x_t::gen_w(int reg, UINT8 val)
{
	switch(reg) {
	case 0: cmd_w(val); break;
	case 1: track_w(val); break;
	case 2: sector_w(val); break;
	case 3: data_w(val); break;
	}
}

UINT8 wd177x_t::gen_r(int reg)
{
	switch(reg) {
	case 0: return status_r(); break;
	case 1: return track_r(); break;
	case 2: return sector_r(); break;
	case 3: return data_r(); break;
	}
	return 0xff;
}

void wd177x_t::delay_cycles(emu_timer *tm, int cycles)
{
	tm->adjust(clocks_to_attotime(cycles));
}

void wd177x_t::spinup()
{
	if(command & 0x08)
		sub_state = SPINUP_DONE;
	else {
		sub_state = SPINUP_WAIT;
		counter = 0;
	}

	status |= S_MON;
	if(floppy)
		floppy->mon_w(0);

}

void wd177x_t::index_callback(floppy_image_device *floppy, int state)
{
	live_sync();

	if(!state) {
		general_continue();
		return;
	}

	switch(sub_state) {
	case IDLE:
		if(has_motor()) {
			motor_timeout ++;
			if(motor_timeout >= 5) {
				status &= ~S_MON;
				if(floppy)
					floppy->mon_w(1);
			}
		}
		break;

	case SPINUP:
		break;

	case SPINUP_WAIT:
		counter++;
		if(counter == 6) {
			sub_state = SPINUP_DONE;
			if(status_type_1)
				status |= S_SPIN;
		}
		break;

	case SPINUP_DONE:
	case SETTLE_WAIT:
	case SETTLE_DONE:
	case DATA_LOAD_WAIT:
	case DATA_LOAD_WAIT_DONE:
	case SEEK_MOVE:
	case SEEK_WAIT_STEP_TIME:
	case SEEK_WAIT_STEP_TIME_DONE:
	case SEEK_WAIT_STABILIZATION_TIME:
	case SEEK_WAIT_STABILIZATION_TIME_DONE:
	case SEEK_DONE:
	case WAIT_INDEX_DONE:
	case SCAN_ID_FAILED:
	case SECTOR_READ:
	case SECTOR_WRITE:
		break;

	case SCAN_ID:
		counter++;
		if(counter == 5) {
			sub_state = SCAN_ID_FAILED;
			live_abort();
		}
		break;

	case WAIT_INDEX:
		sub_state = WAIT_INDEX_DONE;
		break;

	case TRACK_DONE:
		live_abort();
		break;

	default:
		logerror("%s: Index pulse on unknown sub-state %d\n", ttsn().cstr(), sub_state);
		break;
	}

	general_continue();
}

bool wd177x_t::intrq_r()
{
	return intrq;
}

bool wd177x_t::drq_r()
{
	return drq;
}

void wd177x_t::live_start(int state)
{
	cur_live.tm = machine().time();
	cur_live.state = state;
	cur_live.next_state = -1;
	cur_live.shift_reg = 0;
	cur_live.crc = 0xffff;
	cur_live.bit_counter = 0;
	cur_live.data_separator_phase = false;
	cur_live.data_reg = 0;
	cur_live.previous_type = live_info::PT_NONE;
	cur_live.data_bit_context = false;
	cur_live.byte_counter = 0;
	cur_live.pll.reset(cur_live.tm);
	cur_live.pll.set_clock(clocks_to_attotime(1));
	checkpoint_live = cur_live;

	live_run();
}

void wd177x_t::checkpoint()
{
	cur_live.pll.commit(floppy, cur_live.tm);
	checkpoint_live = cur_live;
}

void wd177x_t::rollback()
{
	cur_live = checkpoint_live;
}

void wd177x_t::live_delay(int state)
{
	cur_live.next_state = state;
	t_gen->adjust(cur_live.tm - machine().time());
}

void wd177x_t::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			//          fprintf(stderr, "%s: Rolling back and replaying (%s)\n", ttsn().cstr(), tts(cur_live.tm).cstr());
			rollback();
			live_run(machine().time());
			cur_live.pll.commit(floppy, cur_live.tm);
		} else {
			//          fprintf(stderr, "%s: Committing (%s)\n", ttsn().cstr(), tts(cur_live.tm).cstr());
			cur_live.pll.commit(floppy, cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				cur_live.pll.stop_writing(floppy, cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void wd177x_t::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	cur_live.pll.stop_writing(floppy, cur_live.tm);
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

bool wd177x_t::read_one_bit(attotime limit)
{
	int bit = cur_live.pll.get_next_bit(cur_live.tm, floppy, limit);
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

bool wd177x_t::write_one_bit(attotime limit)
{
	bool bit = cur_live.shift_reg & 0x8000;
	if(cur_live.pll.write_next_bit(bit, cur_live.tm, floppy, limit))
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

void wd177x_t::live_write_raw(UINT16 raw)
{
	//  logerror("write %04x %04x\n", raw, cur_live.crc);
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = raw & 1;
}

void wd177x_t::live_write_mfm(UINT8 mfm)
{
	bool context = cur_live.data_bit_context;
	UINT16 raw = 0;
	for(int i=0; i<8; i++) {
		bool bit = mfm & (0x80 >> i);
		if(!(bit || context))
			raw |= 0x8000 >> (2*i);
		if(bit)
			raw |= 0x4000 >> (2*i);
		context = bit;
	}
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = context;
	//  logerror("write %02x   %04x %04x\n", mfm, cur_live.crc, raw);
}

void wd177x_t::live_run(attotime limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	if(limit == attotime::never) {
		if(floppy)
			limit = floppy->time_next_index();
		if(limit == attotime::never) {
			// Happens when there's no disk or if the wd is not
			// connected to a drive, hence no index pulse. Force a
			// sync from time to time in that case, so that the main
			// cpu timeout isn't too painful.  Avoids looping into
			// infinity looking for data too.

			limit = machine().time() + attotime::from_msec(1);
			t_gen->adjust(attotime::from_msec(1));
		}
	}

	//  fprintf(stderr, "%s: live_run(%s)\n", ttsn().cstr(), tts(limit).cstr());

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
			if(cur_live.data_reg != 0xfe && cur_live.data_reg != 0xff) {
				cur_live.state = SEARCH_ADDRESS_MARK_HEADER;
				break;
			}

			cur_live.bit_counter = 0;

			if(main_state == READ_ID)
				cur_live.state = READ_ID_BLOCK_TO_DMA;
			else
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

		case READ_ID_BLOCK_TO_DMA: {
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			live_delay(READ_ID_BLOCK_TO_DMA_BYTE);
			return;
		}

		case READ_ID_BLOCK_TO_DMA_BYTE:
			data = cur_live.data_reg;
			if(cur_live.bit_counter == 16)
				sector = data;
			set_drq();

			if(cur_live.bit_counter == 16*6) {
				// Already synchronous
				cur_live.state = IDLE;
				return;
			}

			cur_live.state = READ_ID_BLOCK_TO_DMA;
			checkpoint();
			break;

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
			if((cur_live.data_reg & 0xfe) != 0xfa && (cur_live.data_reg & 0xfe) != 0xfc) {
				live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
				return;
			}

			cur_live.bit_counter = 0;
			cur_live.state = READ_SECTOR_DATA;
			break;
		}

		case SEARCH_ADDRESS_MARK_DATA_FAILED:
			status |= S_RNF;
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
			data = cur_live.data_reg;
			set_drq();
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		case READ_TRACK_DATA: {
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter != 16
			   && cur_live.shift_reg != 0x4489
			   && cur_live.shift_reg != 0x5224)
				break;

			// Incorrect, hmmm
			// Probably >2 + not just after a sync if <16

			// Transitions 00..00 -> 4489.4489.4489 at varied syncs:
			//  0: 00.00.14.a1   1: ff.fe.c2.a1   2: 00.01.14.a1   3: ff.fc.c2.a1
			//  4: 00.02.14.a1   5: ff.f8.c2.a1   6: 00.05.14.a1   7: ff.f0.c2.a1
			//  8: 00.00.0a.a1   9: ff.ff.e1.a1  10: 00.00.14.a1  11: ff.ff.ce.a1
			// 12: 00.00.14.a1  13: ff.ff.c2.a1  14: 00.00.14.a1  15: ff.ff.c2.a1

			bool output_byte = cur_live.bit_counter > 5;

			cur_live.data_separator_phase = false;
			cur_live.bit_counter = 0;

			if(output_byte) {
				live_delay(READ_TRACK_DATA_BYTE);
				return;
			}

			break;
		}

		case READ_TRACK_DATA_BYTE:
			data = cur_live.data_reg;
			set_drq();
			cur_live.state = READ_TRACK_DATA;
			checkpoint();
			break;

		case WRITE_TRACK_DATA:
			if(drq) {
				status |= S_LOST;
				data = 0;
			}

			switch(data) {
			case 0xf5:
				live_write_raw(0x4489);
				cur_live.crc = 0x968b; // Ensures that the crc is cdb4 after writing the byte
				cur_live.previous_type = live_info::PT_NONE;
				break;
			case 0xf6:
				cur_live.previous_type = live_info::PT_NONE;
				live_write_raw(0x5224);
				break;
			case 0xf7:
				if(cur_live.previous_type == live_info::PT_CRC_2) {
					cur_live.previous_type = live_info::PT_NONE;
					live_write_mfm(0xf7);
				} else {
					cur_live.previous_type = live_info::PT_CRC_1;
					live_write_mfm(cur_live.crc >> 8);
				}
				break;
			default:
				cur_live.previous_type = live_info::PT_NONE;
				live_write_mfm(data);
				break;
			}
			set_drq();
			cur_live.state = WRITE_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_BYTE:
			if(write_one_bit(limit))
				return;
			if(cur_live.bit_counter == 0) {
				live_delay(WRITE_BYTE_DONE);
				return;
			}
			break;

		case WRITE_BYTE_DONE:
			switch(sub_state) {
			case TRACK_DONE:
				if(cur_live.previous_type == live_info::PT_CRC_1) {
					cur_live.previous_type = live_info::PT_CRC_2;
					live_write_mfm(cur_live.crc >> 8);
					cur_live.state = WRITE_BYTE;
					cur_live.bit_counter = 16;
					checkpoint();
				} else
					cur_live.state = WRITE_TRACK_DATA;
				break;

			case SECTOR_WRITE:
				cur_live.state = WRITE_BYTE;
				cur_live.bit_counter = 16;
				cur_live.byte_counter++;
				if(cur_live.byte_counter <= 11)
					live_write_mfm(0x00);
				else if(cur_live.byte_counter == 12) {
					cur_live.crc = 0xffff;
					live_write_raw(0x4489);
				} else if(cur_live.byte_counter <= 14)
					live_write_raw(0x4489);
				else if(cur_live.byte_counter == 15)
					live_write_mfm(command & 1 ? 0xf8 : 0xfb);
				else if(cur_live.byte_counter <= sector_size + 16-2) {
					if(drq) {
						status |= S_LOST;
						data = 0;
					}
					live_write_mfm(data);
					set_drq();
				} else if(cur_live.byte_counter == sector_size + 16-1) {
					if(drq) {
						status |= S_LOST;
						data = 0;
					}
					live_write_mfm(data);
				} else if(cur_live.byte_counter <= sector_size + 16+1)
					live_write_mfm(cur_live.crc >> 8);
				else if(cur_live.byte_counter == sector_size + 16+2)
					// Is that correct?  It seems required (try ST formatting)
					live_write_mfm(0xff);
				else {
					cur_live.pll.stop_writing(floppy, cur_live.tm);
					cur_live.state = IDLE;
					return;
				}

				checkpoint();
				break;

			default:
				logerror("%s: Unknown sub state %d in WRITE_BYTE_DONE\n", tts(cur_live.tm).cstr(), sub_state);
				live_abort();
				return;
			}
			break;

		case WRITE_SECTOR_PRE:
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter != 16)
				break;
			live_delay(WRITE_SECTOR_PRE_BYTE);
			return;

		case WRITE_SECTOR_PRE_BYTE:
			cur_live.state = WRITE_SECTOR_PRE;
			cur_live.byte_counter++;
			cur_live.bit_counter = 0;
			switch(cur_live.byte_counter) {
			case 2:
				set_drq();
				checkpoint();
				break;
			case 11:
				if(drq) {
					status |= S_LOST;
					cur_live.state = IDLE;
					return;
				}
				break;
			case 22:
				cur_live.state = WRITE_BYTE;
				cur_live.bit_counter = 16;
				cur_live.byte_counter = 0;
				cur_live.data_bit_context = cur_live.data_reg & 1;
				cur_live.pll.start_writing(cur_live.tm);
				live_write_mfm(0x00);
				break;
			}
			break;

		default:
			logerror("%s: Unknown live state %d\n", tts(cur_live.tm).cstr(), cur_live.state);
			return;
		}
	}
}

void wd177x_t::set_drq()
{
	if(drq)
		status |= S_LOST;
	else {
		drq = true;
		if(!drq_cb.isnull())
			drq_cb(true);
	}
}

void wd177x_t::drop_drq()
{
	if(drq) {
		drq = false;
		if(!drq_cb.isnull())
			drq_cb(false);
	}
}

void wd177x_t::pll_t::set_clock(attotime period)
{
	for(int i=0; i<42; i++)
		delays[i] = period*(i+1);
}

void wd177x_t::pll_t::reset(attotime when)
{
	counter = 0;
	increment = 128;
	transition_time = 0xffff;
	history = 0x80;
	slot = 0;
	ctime = when;
	phase_add = 0x00;
	phase_sub = 0x00;
	freq_add  = 0x00;
	freq_sub  = 0x00;
	write_position = 0;
	write_start_time = attotime::never;
}

int wd177x_t::pll_t::get_next_bit(attotime &tm, floppy_image_device *floppy, attotime limit)
{
	attotime when = floppy ? floppy->get_next_transition(ctime) : attotime::never;
#if 0
	if(!when.is_never())
		fprintf(stderr, "transition_time=%s\n", tts(when).cstr());
#endif

	for(;;) {
		//      fprintf(stderr, "slot=%2d, counter=%03x\n", slot, counter);
		attotime etime = ctime+delays[slot];
		//      fprintf(stderr, "etime=%s\n", tts(etime).cstr());
		if(etime > limit)
			return -1;
		if(transition_time == 0xffff && !when.is_never() && etime >= when)
			transition_time = counter;
		if(slot < 8) {
			UINT8 mask = 1 << slot;
			if(phase_add & mask)
				counter += 226;
			else if(phase_sub & mask)
				counter += 30;
			else
				counter += increment;

			if((freq_add & mask) && increment < 140)
				increment++;
			else if((freq_sub & mask) && increment > 117)
				increment--;
		} else
			counter += increment;

		slot++;
		tm = etime;
		if(counter & 0x800)
			break;
	}
	//  fprintf(stderr, "first transition, time=%03x, inc=%3d\n", transition_time, increment);
	int bit = transition_time != 0xffff;

	if(transition_time != 0xffff) {
		static const UINT8 pha[8] = { 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 };
		static const UINT8 phs[8] = { 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf };
		static const UINT8 freqa[4][8] = {
			{ 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0 }
		};
		static const UINT8 freqs[4][8] = {
			{ 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0x1, 0x3, 0x7 },
			{ 0, 0, 0, 0, 0, 0x1, 0x3, 0x7 },
			{ 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf },
		};

		int cslot = transition_time >> 8;
		phase_add = pha[cslot];
		phase_sub = phs[cslot];
		int way = transition_time & 0x400 ? 1 : 0;
		if(history & 0x80)
			history = way ? 0x80 : 0x83;
		else if(history & 0x40)
			history = way ? history & 2 : (history & 2) | 1;
		freq_add = freqa[history & 3][cslot];
		freq_sub = freqs[history & 3][cslot];
		history = way ? (history >> 1) | 2 : history >> 1;

	} else
		phase_add = phase_sub = freq_add = freq_sub = 0;

	counter &= 0x7ff;

	ctime = tm;
	transition_time = 0xffff;
	slot = 0;

	return bit;
}

void wd177x_t::pll_t::start_writing(attotime tm)
{
	write_start_time = tm;
	write_position = 0;
}

void wd177x_t::pll_t::stop_writing(floppy_image_device *floppy, attotime tm)
{
	commit(floppy, tm);
	write_start_time = attotime::never;
}

bool wd177x_t::pll_t::write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, attotime limit)
{
	if(write_start_time.is_never()) {
		write_start_time = ctime;
		write_position = 0;
	}

	for(;;) {
		attotime etime = ctime+delays[slot];
		if(etime > limit)
			return true;
		UINT16 pre_counter = counter;
		counter += increment;
		if(bit && !(pre_counter & 0x400) && (counter & 0x400))
			if(write_position < ARRAY_LENGTH(write_buffer))
				write_buffer[write_position++] = etime;
		slot++;
		tm = etime;
		if(counter & 0x800)
			break;
	}

	counter &= 0x7ff;

	ctime = tm;
	slot = 0;

	return false;
}

void wd177x_t::pll_t::commit(floppy_image_device *floppy, attotime tm)
{
	if(write_start_time.is_never() || tm == write_start_time)
		return;

	if(floppy)
		floppy->write_flux(write_start_time, tm, write_position, write_buffer);
	write_start_time = tm;
	write_position = 0;
}

int wd177x_t::step_time(int mode) const
{
	const static int step_times[4] = { 48000, 96000, 160000, 240000 };
	return step_times[mode];
}

int wd177x_t::settle_time() const
{
	return 240000;
}

bool wd177x_t::has_side_check() const
{
	return false;
}

wd1770_t::wd1770_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : wd177x_t(mconfig, WD1770x, "WD1770", tag, owner, clock)
{
}

bool wd1770_t::has_motor() const
{
	return true;
}

wd1772_t::wd1772_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : wd177x_t(mconfig, WD1772x, "WD1772", tag, owner, clock)
{
}

bool wd1772_t::has_motor() const
{
	return true;
}

int wd1772_t::step_time(int mode) const
{
	const static int step_times[4] = { 48000, 96000, 16000, 24000 };
	return step_times[mode];
}

int wd1772_t::settle_time() const
{
	return 120000;
}

wd1773_t::wd1773_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : wd177x_t(mconfig, WD1773x, "WD1773", tag, owner, clock)
{
}

bool wd1773_t::has_motor() const
{
	return false;
}

bool wd1773_t::has_side_check() const
{
	return false;
}
