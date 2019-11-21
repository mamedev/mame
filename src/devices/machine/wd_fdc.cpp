// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "wd_fdc.h"

#include "imagedev/floppy.h"

#include "debugger.h"

//#define LOG_GENERAL   (1U << 0) //defined in logmacro.h already
#define LOG_SETUP   (1U << 1) // Shows register setup
#define LOG_SHIFT   (1U << 2) // Shows shift register contents
#define LOG_COMP    (1U << 3) // Shows operations on the CPU side
#define LOG_COMMAND (1U << 4) // Shows command invocation
#define LOG_SYNC    (1U << 5) // Shows sync actions
#define LOG_LINES   (1U << 6) // Show control lines
#define LOG_EVENT   (1U << 7) // Show events
#define LOG_MATCH   (1U << 8) // Show sector match operation
#define LOG_DESC    (1U << 9) // Show track description
#define LOG_WRITE   (1U << 10) // Show write operation on image
#define LOG_TRANSITION  (1U << 11) // Show transitions
#define LOG_STATE   (1U << 12) // Show state machine
#define LOG_LIVE    (1U << 13) // Live states
#define LOG_FUNC    (1U << 14) // Function calls

#define VERBOSE (LOG_GENERAL )
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,  __VA_ARGS__)
#define LOGSHIFT(...)   LOGMASKED(LOG_SHIFT, __VA_ARGS__)
#define LOGCOMP(...)    LOGMASKED(LOG_COMP, __VA_ARGS__)
#define LOGCOMMAND(...) LOGMASKED(LOG_COMMAND, __VA_ARGS__)
#define LOGSYNC(...)    LOGMASKED(LOG_SYNC, __VA_ARGS__)
#define LOGLINES(...)   LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGEVENT(...)   LOGMASKED(LOG_EVENT, __VA_ARGS__)
#define LOGMATCH(...)   LOGMASKED(LOG_MATCH, __VA_ARGS__)
#define LOGDESC(...)    LOGMASKED(LOG_DESC, __VA_ARGS__)
#define LOGWRITE(...)   LOGMASKED(LOG_WRITE, __VA_ARGS__)
#define LOGTRANSITION(...) LOGMASKED(LOG_TRANSITION, __VA_ARGS__)
#define LOGSTATE(...) LOGMASKED(LOG_STATE, __VA_ARGS__)
#define LOGLIVE(...) LOGMASKED(LOG_LIVE, __VA_ARGS__)
#define LOGFUNC(...) LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(FD1771,     fd1771_device,     "fd1771",     "FD1771 FDC")

DEFINE_DEVICE_TYPE(FD1781,     fd1781_device,     "fd1781",     "FD1781 FDC")

DEFINE_DEVICE_TYPE(FD1791,     fd1791_device,     "fd1791",     "FD1791 FDC")
DEFINE_DEVICE_TYPE(FD1792,     fd1792_device,     "fd1792",     "FD1792 FDC")
DEFINE_DEVICE_TYPE(FD1793,     fd1793_device,     "fd1793",     "FD1793 FDC")
DEFINE_DEVICE_TYPE(KR1818VG93, kr1818vg93_device, "kr1818vg93", "KR1818VG93 FDC")
DEFINE_DEVICE_TYPE(FD1794,     fd1794_device,     "fd1794",     "FD1794 FDC")
DEFINE_DEVICE_TYPE(FD1795,     fd1795_device,     "fd1795",     "FD1795 FDC")
DEFINE_DEVICE_TYPE(FD1797,     fd1797_device,     "fd1797",     "FD1797 FDC")

DEFINE_DEVICE_TYPE(MB8866,     mb8866_device,     "mb8866",     "Fujitsu MB8866 FDC")
DEFINE_DEVICE_TYPE(MB8876,     mb8876_device,     "mb8876",     "Fujitsu MB8876 FDC")
DEFINE_DEVICE_TYPE(MB8877,     mb8877_device,     "mb8877",     "Fujitsu MB8877 FDC")

DEFINE_DEVICE_TYPE(FD1761,     fd1761_device,     "fd1761",     "FD1761 FDC")
DEFINE_DEVICE_TYPE(FD1763,     fd1763_device,     "fd1763",     "FD1763 FDC")
DEFINE_DEVICE_TYPE(FD1765,     fd1765_device,     "fd1765",     "FD1765 FDC")
DEFINE_DEVICE_TYPE(FD1767,     fd1767_device,     "fd1767",     "FD1767 FDC")

DEFINE_DEVICE_TYPE(WD2791,     wd2791_device,     "wd2791",     "Western Digital WD2791 FDC")
DEFINE_DEVICE_TYPE(WD2793,     wd2793_device,     "wd2793",     "Western Digital WD2793 FDC")
DEFINE_DEVICE_TYPE(WD2795,     wd2795_device,     "wd2795",     "Western Digital WD2795 FDC")
DEFINE_DEVICE_TYPE(WD2797,     wd2797_device,     "wd2797",     "Western Digital WD2797 FDC")

DEFINE_DEVICE_TYPE(WD1770,     wd1770_device,     "wd1770",     "Western Digital WD1770 FDC")
DEFINE_DEVICE_TYPE(WD1772,     wd1772_device,     "wd1772",     "Western Digital WD1772 FDC")
DEFINE_DEVICE_TYPE(WD1773,     wd1773_device,     "wd1773",     "Western Digital WD1773 FDC")

wd_fdc_device_base::wd_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	intrq_cb(*this),
	drq_cb(*this),
	hld_cb(*this),
	enp_cb(*this),
	sso_cb(*this),
	ready_cb(*this), // actually output by the drive, not by the FDC
	enmf_cb(*this)
{
	force_ready = false;
	disable_motor_control = false;
	spinup_on_interrupt = false;
}

void wd_fdc_device_base::set_force_ready(bool _force_ready)
{
	force_ready = _force_ready;
}

void wd_fdc_device_base::set_disable_motor_control(bool _disable_motor_control)
{
	disable_motor_control = _disable_motor_control;
}

void wd_fdc_device_base::device_start()
{
	intrq_cb.resolve();
	drq_cb.resolve();
	hld_cb.resolve();
	enp_cb.resolve();
	sso_cb.resolve();
	ready_cb.resolve();
	enmf_cb.resolve();

	if (!has_enmf && !enmf_cb.isnull())
		logerror("Warning, this chip doesn't have an ENMF line.\n");

	t_gen = timer_alloc(TM_GEN);
	t_cmd = timer_alloc(TM_CMD);
	t_track = timer_alloc(TM_TRACK);
	t_sector = timer_alloc(TM_SECTOR);
	dden = disable_mfm;
	enmf = false;
	floppy = nullptr;
	status = 0x00;
	mr = true;

	save_item(NAME(status));
	save_item(NAME(command));
	save_item(NAME(main_state));
	save_item(NAME(sub_state));
	save_item(NAME(track));
	save_item(NAME(sector));
	save_item(NAME(intrq_cond));
	save_item(NAME(cmd_buffer));
	save_item(NAME(track_buffer));
	save_item(NAME(sector_buffer));
	save_item(NAME(counter));
	save_item(NAME(status_type_1));
	save_item(NAME(last_dir));
	if (!disable_mfm)
		save_item(NAME(dden));
	save_item(NAME(mr));
}

void wd_fdc_device_base::device_reset()
{
	soft_reset();
}

void wd_fdc_device_base::soft_reset()
{
	if(mr) {
		mr_w(0);
		mr_w(1);
	}
}

WRITE_LINE_MEMBER(wd_fdc_device_base::mr_w)
{
	if(mr && !state) {
		command = 0x00;
		main_state = IDLE;
		sub_state = IDLE;
		cur_live.state = IDLE;
		track = 0x00;
		sector = 0x01;
		status = 0x00;
		data = 0x00;
		cmd_buffer = track_buffer = sector_buffer = -1;
		counter = 0;
		status_type_1 = true;
		last_dir = 1;
		mr = false;

		// gnd == enmf enabled, otherwise disabled (default)
		if (!enmf_cb.isnull() && has_enmf)
			enmf = enmf_cb() ? false : true;

		intrq = false;
		if (!intrq_cb.isnull())
		{
			intrq_cb(intrq);
		}
		drq = false;
		if (!drq_cb.isnull())
		{
			drq_cb(drq);
		}
		hld = false;
		intrq_cond = 0;
		live_abort();
	} else if(state && !mr) {
		// trigger a restore after everything else is reset too, in particular the floppy device itself
		sub_state = INITIAL_RESTORE;
		t_gen->adjust(attotime::zero);
		mr = true;
	}
}

void wd_fdc_device_base::set_floppy(floppy_image_device *_floppy)
{
	if(floppy == _floppy)
		return;

	int prev_ready = floppy ? floppy->ready_r() : 1;

	if(floppy) {
		// Warning: deselecting a drive does *not* stop its motor if it was running
		floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		floppy->setup_ready_cb(floppy_image_device::ready_cb());
	}

	floppy = _floppy;

	int next_ready = floppy ? floppy->ready_r() : 1;

	if(floppy) {
		if(motor_control && !disable_motor_control)
			floppy->mon_w(status & S_MON ? 0 : 1);
		floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&wd_fdc_device_base::index_callback, this));
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&wd_fdc_device_base::ready_callback, this));
	}

	if(prev_ready != next_ready)
		ready_callback(floppy, next_ready);
}

WRITE_LINE_MEMBER(wd_fdc_device_base::dden_w)
{
	if(disable_mfm) {
		logerror("Error, this chip does not have a dden line\n");
		return;
	}

	if(dden != bool(state)) {
		dden = bool(state);
		LOGLINES("select %s\n", dden ? "fm" : "mfm");
	}
}

std::string wd_fdc_device_base::tts(const attotime &t)
{
	char buf[256];
	int nsec = t.attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%4d.%03d,%03d,%03d", int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

std::string wd_fdc_device_base::ttsn()
{
	return tts(machine().time());
}

void wd_fdc_device_base::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOGEVENT("Event fired for timer %s\n", (id==TM_GEN)? "TM_GEN" : (id==TM_CMD)? "TM_CMD" : (id==TM_TRACK)? "TM_TRACK" : "TM_SECTOR");
	live_sync();

	switch(id) {
	case TM_GEN: do_generic(); break;
	case TM_CMD: do_cmd_w(); break;
	case TM_TRACK: do_track_w(); break;
	case TM_SECTOR: do_sector_w(); break;
	}

	general_continue();
}

void wd_fdc_device_base::command_end()
{
	LOGFUNC("%s\n", FUNCNAME);
	main_state = sub_state = IDLE;
	motor_timeout = 0;

	if (!drq) {
		status &= ~S_BUSY;
		intrq = true;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}
}

void wd_fdc_device_base::seek_start(int state)
{
	LOGCOMMAND("cmd: seek %d %x (track=%d)\n", state, data, track);
	main_state = state;
	status &= ~(S_CRC|S_RNF|S_SPIN);
	if(head_control) {
		// TODO get value from HLT callback
		if(command & 8)
			status |= S_HLD;
		else
			status &= ~S_HLD;
	}
	sub_state = motor_control ? SPINUP : SPINUP_DONE;
	status_type_1 = true;
	seek_continue();
}

void wd_fdc_device_base::seek_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			LOGSTATE("SPINUP\n");
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			if(!(command & 0x08))
				status |= S_SPIN;
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			LOGSTATE("SPINUP_WAIT\n");
			return;

		case SPINUP_DONE:
			LOGSTATE("SPINUP_DONE\n");
			if(main_state == RESTORE && floppy && !floppy->trk00_r()) {
				sub_state = SEEK_WAIT_STEP_TIME;
				delay_cycles(t_gen, step_times[command & 3]);
			}

			if(main_state == SEEK && track == data) {
				sub_state = SEEK_WAIT_STABILIZATION_TIME;
				delay_cycles(t_gen, 30000);
			}

			if(sub_state == SPINUP_DONE) {
				counter = 0;
				sub_state = SEEK_MOVE;
			}
			break;

		case SEEK_MOVE:
			LOGSTATE("SEEK_MOVE\n");
			if(floppy) {
				floppy->dir_w(last_dir);
				floppy->stp_w(0);
				floppy->stp_w(1);
			}
			// When stepping with update, the track register is updated before seeking.
			// Important for the sam coupe format code.
			if(main_state == STEP && (command & 0x10))
				track += last_dir ? -1 : 1;
			counter++;
			sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(t_gen, step_times[command & 3]);
			return;

		case SEEK_WAIT_STEP_TIME:
			LOGSTATE("SEEK_WAIT_STEP_TIME\n");
			return;

		case SEEK_WAIT_STEP_TIME_DONE: {
			LOGSTATE("SEEK_WAIT_STEP_TIME_DONE\n");
			bool done = false;
			switch(main_state) {
			case RESTORE:
				done = floppy && !floppy->trk00_r();
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
					delay_cycles(t_gen, 30000);
					return;
				} else
					sub_state = SEEK_DONE;

			} else
				sub_state = SEEK_MOVE;

			break;
		}

		case SEEK_WAIT_STABILIZATION_TIME:
			LOGSTATE("SEEK_WAIT_STABILIZATION_TIME\n");
			return;

		case SEEK_WAIT_STABILIZATION_TIME_DONE:
			LOGSTATE("SEEK_WAIT_STABILIZATION_TIME_DONE\n");
			sub_state = SEEK_DONE;
			break;

		case SEEK_DONE:
			LOGSTATE("SEEK_DONE\n");
			status |= S_HLD;
			hld = true;
			if (!hld_cb.isnull())
				hld_cb(hld);
			if(command & 0x04) {
				if(!is_ready()) {
					status |= S_RNF;
					command_end();
					return;
				}
				sub_state = SCAN_ID;
				counter = 0;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			command_end();
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
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
			LOGSTATE("SCAN_ID_FAILED\n");
			status |= S_RNF;
			command_end();
			return;

		default:
			logerror("seek unknown sub-state %d\n", ttsn().c_str(), sub_state);
			return;
		}
	}
}

bool wd_fdc_device_base::sector_matches() const
{
	   LOGMATCH("matching read T=%02x H=%02x S=%02x L=%02x - searched T=%02x S=%02x\n",
					cur_live.idbuf[0], cur_live.idbuf[1], cur_live.idbuf[2], cur_live.idbuf[3],
					track, sector);

	if(cur_live.idbuf[0] != track || cur_live.idbuf[2] != sector)
		return false;
	if(!side_compare || ((command & 2)==0))
		return true;
	if(command & 8)
		return cur_live.idbuf[1] & 1;
	else
		return !(cur_live.idbuf[1] & 1);
}

bool wd_fdc_device_base::is_ready()
{
	return !ready_hooked || force_ready || (floppy && !floppy->ready_r());
}

void wd_fdc_device_base::read_sector_start()
{
	LOGCOMMAND("cmd: read sector%s (c=%02x) t=%d, s=%d\n", command & 0x10 ? " multiple" : "", command, track, sector);
	if(!is_ready()) {
		command_end();
		return;
	}

	main_state = READ_SECTOR;
	status &= ~(S_CRC|S_LOST|S_RNF|S_WP|S_DDM);
	drop_drq();
	if(side_control)
		update_sso();
	sub_state = motor_control ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	read_sector_continue();
}

void wd_fdc_device_base::read_sector_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			LOGSTATE("SPINUP\n");
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			LOGSTATE("SPINUP_WAIT\n");
			return;

		case SPINUP_DONE:
			LOGSTATE("SPINUP_DONE\n");
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			LOGSTATE("SETTLE_WAIT\n");
			return;

		case SETTLE_DONE:
			LOGSTATE("SETTLE_DONE\n");
			sub_state = SCAN_ID;
			counter = 0;
			live_start(SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			if(!sector_matches()) {
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				status |= S_CRC;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			sector_size = calc_sector_size(cur_live.idbuf[3], command);
			sub_state = SECTOR_READ;
			live_start(SEARCH_ADDRESS_MARK_DATA);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			status |= S_RNF;
			command_end();
			return;

		case SECTOR_READ:
			LOGSTATE("SECTOR_READ\n");
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
			logerror("read sector unknown sub-state %d\n", ttsn().c_str(), sub_state);
			return;
		}
	}
}

void wd_fdc_device_base::read_track_start()
{
	LOGCOMMAND("cmd: read track (c=%02x) t=%d\n", command, track);

	if(!is_ready()) {
		command_end();
		return;
	}

	main_state = READ_TRACK;
	status &= ~(S_LOST|S_RNF);
	drop_drq();
	if(side_control)
		update_sso();
	sub_state = motor_control ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	read_track_continue();
}

void wd_fdc_device_base::read_track_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			LOGSTATE("SPINUP\n");
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			LOGSTATE("SPINUP_WAIT\n");
			return;

		case SPINUP_DONE:
			LOGSTATE("SPINUP_DONE\n");
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;

			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			LOGSTATE("SETTLE_WAIT\n");
			return;

		case SETTLE_DONE:
			LOGSTATE("SETTLE_DONE\n");
			sub_state = WAIT_INDEX;
			return;

		case WAIT_INDEX:
			LOGSTATE("WAIT_INDEX\n");
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("WAIT_INDEX_DONE\n");
			sub_state = TRACK_DONE;
			live_start(READ_TRACK_DATA);
			return;

		case TRACK_DONE:
			LOGSTATE("TRACK_DONE\n");
			command_end();
			return;

		default:
			logerror("read track unknown sub-state %d\n", ttsn().c_str(), sub_state);
			return;
		}
	}
}

void wd_fdc_device_base::read_id_start()
{
	LOGCOMMAND("cmd: read id (c=%02x)\n", command);
	if(!is_ready()) {
		LOGCOMMAND("cmd: - not ready!");
		command_end();
		return;
	}

	main_state = READ_ID;
	status &= ~(S_WP|S_DDM|S_LOST|S_RNF);
	drop_drq();
	if(side_control)
		update_sso();
	sub_state = motor_control ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	read_id_continue();
}

void wd_fdc_device_base::read_id_continue()
{
	LOGFUNC("%s\n", FUNCNAME);
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			LOGSTATE("SPINUP\n");
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			LOGSTATE("SPINUP_WAIT\n");
			return;

		case SPINUP_DONE:
			LOGSTATE("SPINUP_DONE\n");
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			LOGSTATE("SETTLE_WAIT\n");
			return;

		case SETTLE_DONE:
			LOGSTATE("SETTLE_DONE\n");
			sub_state = SCAN_ID;
			counter = 0;
			live_start(SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			command_end();
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			status |= S_RNF;
			command_end();
			return;

		default:
			logerror("read id unknown sub-state %d\n", ttsn().c_str(), sub_state);
			return;
		}
	}
}

void wd_fdc_device_base::write_track_start()
{
	LOGCOMMAND("cmd: write track (c=%02x) t=%d\n", command, track);

	if(!is_ready()) {
		command_end();
		return;
	}

	main_state = WRITE_TRACK;
	status &= ~(S_WP|S_DDM|S_LOST|S_RNF);
	drop_drq();
	if(side_control)
		update_sso();
	sub_state = motor_control ? SPINUP : SPINUP_DONE;
	status_type_1 = false;

	format_last_byte = 0;
	format_last_byte_count = 0;
	format_description_string = "";

	write_track_continue();
}

void wd_fdc_device_base::write_track_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			LOGSTATE("SPINUP\n");
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			LOGSTATE("SPINUP_WAIT\n");
			return;

		case SPINUP_DONE:
			LOGSTATE("SPINUP_DONE\n");
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			LOGSTATE("SETTLE_WAIT\n");
			return;

		case SETTLE_DONE:
			LOGSTATE("SETTLE_DONE\n");
			set_drq();
			sub_state = DATA_LOAD_WAIT;
			delay_cycles(t_gen, 192);
			return;

		case DATA_LOAD_WAIT:
			LOGSTATE("DATA_LOAD_WAIT\n");
			return;

		case DATA_LOAD_WAIT_DONE:
			LOGSTATE("DATA_LOAD_WAIT_DONE\n");
			if(drq) {
				status |= S_LOST;
				drop_drq();
				command_end();
				return;
			}
			sub_state = WAIT_INDEX;
			break;

		case WAIT_INDEX:
			LOGSTATE("WAIT_INDEX\n");
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("WAIT_INDEX_DONE\n");
			sub_state = TRACK_DONE;
			live_start(WRITE_TRACK_DATA);
			pll_start_writing(machine().time());
			return;

		case TRACK_DONE:
			LOGSTATE("TRACK_DONE\n");
			if(format_last_byte_count) {
				char buf[32];
				if(format_last_byte_count > 1)
					sprintf(buf, "%dx%02x", format_last_byte_count, format_last_byte);
				else
					sprintf(buf, "%02x", format_last_byte);
				format_description_string += buf;
			}
			LOGDESC("track description %s\n", format_description_string.c_str());
			command_end();
			return;

		default:
			logerror("write track unknown sub-state %d\n", ttsn().c_str(), sub_state);
			return;
		}
	}
}


void wd_fdc_device_base::write_sector_start()
{
	LOGCOMMAND("cmd: write sector%s (c=%02x) t=%d, s=%d\n", command & 0x10 ? " multiple" : "", command, track, sector);

	if(!is_ready()) {
		command_end();
		return;
	}

	main_state = WRITE_SECTOR;
	status &= ~(S_CRC|S_LOST|S_RNF|S_WP|S_DDM);
	drop_drq();
	if(side_control)
		update_sso();
	sub_state = motor_control  ? SPINUP : SPINUP_DONE;
	status_type_1 = false;
	write_sector_continue();
}

void wd_fdc_device_base::write_sector_continue()
{
	for(;;) {
		switch(sub_state) {
		case SPINUP:
			LOGSTATE("SPINUP\n");
			if(!(status & S_MON)) {
				spinup();
				return;
			}
			sub_state = SPINUP_DONE;
			break;

		case SPINUP_WAIT:
			LOGSTATE("SPINUP_WAIT\n");
			return;

		case SPINUP_DONE:
			LOGSTATE("SPINUP_DONE\n");
			if(command & 4) {
				sub_state = SETTLE_WAIT;
				delay_cycles(t_gen, settle_time());
				return;
			} else {
				sub_state = SETTLE_DONE;
				break;
			}

		case SETTLE_WAIT:
			LOGSTATE("SETTLE_WAIT\n");
			return;

		case SETTLE_DONE:
			LOGSTATE("SETTLE_DONE\n");
			sub_state = SCAN_ID;
			counter = 0;
			live_start(SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			if(!sector_matches()) {
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				status |= S_CRC;
				live_start(SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			sector_size = calc_sector_size(cur_live.idbuf[3], command);
			sub_state = SECTOR_WRITE;
			live_start(WRITE_SECTOR_PRE);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			status |= S_RNF;
			command_end();
			return;

		case SECTOR_WRITE:
			LOGSTATE("SECTOR_WRITE\n");
			if(command & 0x10) {
				sector++;
				sub_state = SPINUP_DONE;
			} else {
				command_end();
				return;
			}
			break;

		default:
			logerror("write sector unknown sub-state %d\n", ttsn().c_str(), sub_state);
			return;
		}
	}
}

void wd_fdc_device_base::interrupt_start()
{
	// technically we should re-execute this (at chip-specific rate) all the time while interrupt command code is in command register

	LOGCOMMAND("cmd: forced interrupt (c=%02x)\n", command);

	if(status & S_BUSY) {
		main_state = sub_state = cur_live.state = IDLE;
		cur_live.tm = attotime::never;
		status &= ~S_BUSY;
		drop_drq();
		motor_timeout = 0;
	}
	else
	{
		// when a force interrupt command is issued and there is no
		// currently running command, return the status type 1 bits
		status_type_1 = true;
	}

	intrq_cond = command & 0x0f;

	if(!intrq && (command & I_IMM)) {
		intrq = true;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}

	if(command & 0x03) {
		logerror("%s: unhandled interrupt generation (%02x)\n", ttsn().c_str(), command);
	}
}

void wd_fdc_device_base::general_continue()
{
	LOGFUNC("%s\n", FUNCNAME);
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
		logerror("%s: general_continue on unknown main-state %d\n", ttsn().c_str(), main_state);
		break;
	}
}

void wd_fdc_device_base::do_generic()
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

	case INITIAL_RESTORE:
		last_dir = 1;
		seek_start(RESTORE);
		break;

	default:
		if(cur_live.tm.is_never())
			logerror("%s: do_generic on unknown sub-state %d\n", ttsn().c_str(), sub_state);
		break;
	}
}

void wd_fdc_device_base::do_cmd_w()
{
	// Only available command when busy is interrupt
	if(main_state != IDLE && (cmd_buffer & 0xf0) != 0xd0) {
		cmd_buffer = -1;
		return;
	}
	command = cmd_buffer;
	cmd_buffer = -1;

	LOGCOMMAND("%s %02x: %s\n", FUNCNAME, cmd_buffer, std::array<char const *, 16>
		   {{"RESTORE", "SEEK", "STEP", "STEP", "STEP", "STEP", "STEP", "STEP",
			 "READ sector start", "READ sector start", "WRITE sector start", "WRITE sector start",
			 "READ ID start",     "INTERRUPT start",   "READ track start",   "WRITE track start"}}[(command >> 4) & 0x0f]);
	switch(command & 0xf0) {
	case 0x00:
		last_dir = 1;
		seek_start(RESTORE);
		break;
	case 0x10:
		last_dir = data > track ? 0 : 1;
		seek_start(SEEK);
		break;
	case 0x20:
	case 0x30:
		seek_start(STEP);
		break;
	case 0x40:
	case 0x50:
		last_dir = 0;
		seek_start(STEP);
		break;
	case 0x60:
	case 0x70:
		last_dir = 1;
		seek_start(STEP);
		break;
	case 0x80:
	case 0x90:
		read_sector_start();
		break;
	case 0xa0:
	case 0xb0:
		write_sector_start();
		break;
	case 0xc0:
		read_id_start();
		break;
	case 0xd0:
		interrupt_start();
		break;
	case 0xe0:
		read_track_start();
		break;
	case 0xf0:
		write_track_start();
		break;
	}
}

void wd_fdc_device_base::cmd_w(uint8_t val)
{
	if (inverted_bus) val ^= 0xff;
	if (!mr) {
		logerror("Not initiating command %02x during master reset\n", val);
		return;
	}

	LOGCOMP("Initiating command %02x\n", val);

	if (intrq) {
		intrq = false;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}

	// No more than one write in flight, but interrupts take priority
	if(cmd_buffer != -1 && ((val & 0xf0) != 0xd0))
		return;

	cmd_buffer = val;

	if ((val & 0xf0) == 0xd0)
	{
		// checkme
		delay_cycles(t_cmd, dden ? delay_register_commit * 2 : delay_register_commit);
		if (spinup_on_interrupt)  // see note in WD1772 constructor
			spinup();
	}
	else
	{
		intrq_cond = 0;
		// set busy, then set a timer to process the command
		status |= S_BUSY;
		delay_cycles(t_cmd, dden ? delay_command_commit*2 : delay_command_commit);
	}
}

uint8_t wd_fdc_device_base::status_r()
{
	if(intrq && !(intrq_cond & I_IMM) && !machine().side_effects_disabled()) {
		intrq = false;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}

	if(status_type_1) {
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

	if(status_type_1) {
		status &= ~(S_TR00|S_WP);
		if(floppy) {
			if(floppy->wpt_r())
				status |= S_WP;
			if(!floppy->trk00_r())
				status |= S_TR00;
		}
	}

	if(ready_hooked) {
		if(!is_ready())
			status |= S_NRDY;
		else
			status &= ~S_NRDY;
	}

	uint8_t val = status;
	if (inverted_bus) val ^= 0xff;

	return val;
}

void wd_fdc_device_base::do_track_w()
{
	track = track_buffer;
	track_buffer = -1;
}

void wd_fdc_device_base::track_w(uint8_t val)
{
	if (inverted_bus) val ^= 0xff;

	// No more than one write in flight
	if(track_buffer != -1 || !mr)
		return;

	track_buffer = val;
	delay_cycles(t_track, dden ? delay_register_commit*2 : delay_register_commit);
}

uint8_t wd_fdc_device_base::track_r()
{
	uint8_t val = track;
	if (inverted_bus) val ^= 0xff;

	return val;
}

void wd_fdc_device_base::do_sector_w()
{
	sector = sector_buffer;
	sector_buffer = -1;
}

void wd_fdc_device_base::sector_w(uint8_t val)
{
	if (!mr) return;

	if (inverted_bus) val ^= 0xff;

	// No more than one write in flight
	// C1581 accesses this register with an INC opcode,
	// i.e. write old value, write new value, and the new value gets ignored by this
	//if(sector_buffer != -1)
	//  return;

	sector_buffer = val;

	// set a timer to write the new value to the register, but only if we aren't in
	// the middle of an already occurring update
	if (!t_sector->enabled())
		delay_cycles(t_sector, dden ? delay_register_commit*2 : delay_register_commit);
}

uint8_t wd_fdc_device_base::sector_r()
{
	uint8_t val = sector;
	if (inverted_bus) val ^= 0xff;

	return val;
}

void wd_fdc_device_base::data_w(uint8_t val)
{
	if (!mr) return;

	if (inverted_bus) val ^= 0xff;

	data = val;
	drop_drq();
}

uint8_t wd_fdc_device_base::data_r()
{
	if (!machine().side_effects_disabled())
		drop_drq();

	uint8_t val = data;
	if (inverted_bus) val ^= 0xff;

	return val;
}

void wd_fdc_device_base::write(offs_t reg, uint8_t val)
{
	LOGFUNC("%s %02x: %02x\n", FUNCNAME, reg, val);
	switch(reg) {
	case 0: cmd_w(val); break;
	case 1: track_w(val); break;
	case 2: sector_w(val); break;
	case 3: data_w(val); break;
	}
}

uint8_t wd_fdc_device_base::read(offs_t reg)
{
	switch(reg) {
	case 0: return status_r();
	case 1: return track_r();
	case 2: return sector_r();
	case 3: return data_r();
	}
	return 0xff;
}

void wd_fdc_device_base::delay_cycles(emu_timer *tm, int cycles)
{
	tm->adjust(clocks_to_attotime(cycles*clock_ratio));
}

void wd_fdc_device_base::spinup()
{
	if(command & 0x08)
		sub_state = SPINUP_DONE;
	else {
		sub_state = SPINUP_WAIT;
		counter = 0;
	}

	status |= S_MON|S_SPIN;
	if(floppy && !disable_motor_control)
		floppy->mon_w(0);
}

void wd_fdc_device_base::ready_callback(floppy_image_device *floppy, int state)
{
	if(!ready_cb.isnull())
		ready_cb(state);

	// why is this even possible?
	if (!floppy)
		return;

	live_sync();
	if(!ready_hooked)
		return;

	if(!intrq && (((intrq_cond & I_RDY) && !state) || ((intrq_cond & I_NRDY) && state))) {
		intrq = true;
		if(!intrq_cb.isnull())
			intrq_cb(intrq);
	}
}

void wd_fdc_device_base::index_callback(floppy_image_device *floppy, int state)
{
	live_sync();

	if(!state) {
		general_continue();
		return;
	}

	switch(sub_state) {
	case IDLE:
		if(motor_control || head_control) {
			motor_timeout ++;
			if(motor_control && motor_timeout >= 5) {
				status &= ~S_MON;
				if(floppy && !disable_motor_control)
					floppy->mon_w(1);
			}

			if (head_control && motor_timeout >= 3)
			{
				hld = false;

				// signal drive to unload head
				if (!hld_cb.isnull())
					hld_cb(hld);

				status &= ~S_HLD; // todo: should get this value from the drive
			}
		}

		if(!intrq && (intrq_cond & I_IDX)) {
			intrq = true;
			if(!intrq_cb.isnull())
				intrq_cb(intrq);
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
		logerror("%s: Index pulse on unknown sub-state %d\n", ttsn().c_str(), sub_state);
		break;
	}

	general_continue();
}

READ_LINE_MEMBER(wd_fdc_device_base::intrq_r)
{
	return intrq;
}

READ_LINE_MEMBER(wd_fdc_device_base::drq_r)
{
	return drq;
}

READ_LINE_MEMBER(wd_fdc_device_base::hld_r)
{
	return hld;
}

WRITE_LINE_MEMBER(wd_fdc_device_base::hlt_w)
{
	hlt = bool(state);
}

READ_LINE_MEMBER(wd_fdc_device_base::enp_r)
{
	return enp;
}

void wd_fdc_device_base::live_start(int state)
{
	LOGFUNC("%s\n", FUNCNAME);
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

	if (!enmf_cb.isnull() && has_enmf)
		enmf = enmf_cb() ? false : true;

	pll_reset(dden, enmf, cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();

	live_run();
}

void wd_fdc_device_base::checkpoint()
{
	LOGFUNC("%s\n", FUNCNAME);
	pll_commit(floppy, cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();
}

void wd_fdc_device_base::rollback()
{
	cur_live = checkpoint_live;
	pll_retrieve_checkpoint();
}

void wd_fdc_device_base::live_delay(int state)
{
	cur_live.next_state = state;
	t_gen->adjust(cur_live.tm - machine().time());
}

void wd_fdc_device_base::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			LOGSYNC("%s: Rolling back and replaying (%s)\n", ttsn().c_str(), tts(cur_live.tm).c_str());
			rollback();
			live_run(machine().time());
			pll_commit(floppy, cur_live.tm);
		} else {
			LOGSYNC("%s: Committing (%s)\n", ttsn().c_str(), tts(cur_live.tm).c_str());
			pll_commit(floppy, cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				pll_stop_writing(floppy, cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void wd_fdc_device_base::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	pll_stop_writing(floppy, cur_live.tm);
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

bool wd_fdc_device_base::read_one_bit(const attotime &limit)
{
	int bit = pll_get_next_bit(cur_live.tm, floppy, limit);
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

bool wd_fdc_device_base::write_one_bit(const attotime &limit)
{
	bool bit = cur_live.shift_reg & 0x8000;
	if(pll_write_next_bit(bit, cur_live.tm, floppy, limit))
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

void wd_fdc_device_base::live_write_raw(uint16_t raw)
{
	LOGWRITE("write raw %04x, CRC=%04x\n", raw, cur_live.crc);
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = raw & 1;
}

void wd_fdc_device_base::live_write_mfm(uint8_t mfm)
{
	bool context = cur_live.data_bit_context;
	uint16_t raw = 0;
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
	LOGWRITE("live_write_mfm byte=%02x, raw=%04x, CRC=%04x\n", mfm, raw, cur_live.crc);
}


void wd_fdc_device_base::live_write_fm(uint8_t fm)
{
	uint16_t raw = 0xaaaa;
	for(int i=0; i<8; i++)
		if(fm & (0x80 >> i))
			raw |= 0x4000 >> (2*i);
	cur_live.data_reg = fm;
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = fm & 1;
	LOGWRITE("live_write_fm byte=%02x, raw=%04x, CRC=%04x\n", fm, raw, cur_live.crc);
}

void wd_fdc_device_base::live_run(attotime limit)
{
  //    LOG("%s\n", FUNCNAME);
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

	//  fprintf(stderr, "%s: live_run(%s)\n", ttsn().c_str(), tts(limit).c_str());

	for(;;) {
		switch(cur_live.state) {
		case SEARCH_ADDRESS_MARK_HEADER:
			LOGLIVE("%s - SEARCH_ADDRESS_MARK_HEADER\n", FUNCNAME);
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x c=%d\n", tts(cur_live.tm).c_str(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter);

			if(!dden && cur_live.shift_reg == 0x4489) {
				cur_live.crc = 0x443b;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_HEADER_BLOCK_HEADER;
			}

			if(dden && cur_live.shift_reg == 0xf57e) {
				cur_live.crc = 0xef21;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				if(main_state == READ_ID)
					cur_live.state = READ_ID_BLOCK_TO_DMA;
				else
					cur_live.state = READ_ID_BLOCK_TO_LOCAL;
			}
			break;

		case READ_HEADER_BLOCK_HEADER: {
			LOGLIVE("%s - READ_HEADER_BLOCK_HEADER\n", FUNCNAME);
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x counter=%d\n", tts(cur_live.tm).c_str(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter);

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
			LOGLIVE("%s - READ_ID_BLOCK_TO_LOCAL\n", FUNCNAME);
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			int slot = (cur_live.bit_counter >> 4)-1;
			//          fprintf(stderr, "%s: slot[%d] = %02x  crc = %04x\n", tts(cur_live.tm).c_str(), slot, cur_live.data_reg, cur_live.crc);
			cur_live.idbuf[slot] = cur_live.data_reg;
			if(slot == 5) {
				live_delay(IDLE);
				return;
			}
			break;
		}

		case READ_ID_BLOCK_TO_DMA:
			LOGLIVE("%s - READ_ID_BLOCK_TO_DMA\n", FUNCNAME);
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			live_delay(READ_ID_BLOCK_TO_DMA_BYTE);
			return;

		case READ_ID_BLOCK_TO_DMA_BYTE:
			LOGLIVE("%s - READ_ID_BLOCK_TO_DMA_BYTE\n", FUNCNAME);
			data = cur_live.data_reg;
			if(cur_live.bit_counter == 16)
				sector = data;
			set_drq();

			if(cur_live.bit_counter == 16*6) {
				if(cur_live.crc) {
					status |= S_CRC;
				}

				// Already synchronous
				cur_live.state = IDLE;
				return;
			}

			cur_live.state = READ_ID_BLOCK_TO_DMA;
			checkpoint();
			break;

		case SEARCH_ADDRESS_MARK_DATA:
			LOGLIVE("%s - SEARCH_ADDRESS_MARK_DATA\n", FUNCNAME);
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x c=%d.%x\n", tts(cur_live.tm).c_str(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter >> 4, cur_live.bit_counter & 15);

			if(!dden) {
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
			} else {
				if(cur_live.bit_counter > 23*16) {
					live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
					return;
				}

				if(cur_live.bit_counter >= 11*16 && (cur_live.shift_reg == 0xf56a || cur_live.shift_reg == 0xf56b ||
														cur_live.shift_reg == 0xf56e || cur_live.shift_reg == 0xf56f)) {
					cur_live.crc =
						cur_live.shift_reg == 0xf56a ? 0x8fe7 :
						cur_live.shift_reg == 0xf56b ? 0x9fc6 :
						cur_live.shift_reg == 0xf56e ? 0xafa5 :
						0xbf84;

					if((cur_live.data_reg & 0xfe) == 0xf8)
						status |= S_DDM;

					cur_live.data_separator_phase = false;
					cur_live.bit_counter = 0;
					cur_live.state = READ_SECTOR_DATA;
				}
			}
			break;

		case READ_DATA_BLOCK_HEADER: {
			LOGLIVE("%s - READ_DATA_BLOCK_HEADER\n", FUNCNAME);
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x counter=%d\n", tts(cur_live.tm).c_str(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter);

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
			if((cur_live.data_reg & 0xfe) != 0xfa && (cur_live.data_reg & 0xfe) != 0xf8) {
				live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
				return;
			}

			cur_live.bit_counter = 0;
			if((cur_live.data_reg & 0xfe) == 0xf8)
				status |= S_DDM;
			live_delay(READ_SECTOR_DATA);
			return;
		}

		case SEARCH_ADDRESS_MARK_DATA_FAILED:
			LOGLIVE("%s - SEARCH_ADDRESS_MARK_DATA_FAILED\n", FUNCNAME);
			status |= S_RNF;
			cur_live.state = IDLE;
			return;

		case READ_SECTOR_DATA: {
			LOGLIVE("%s - READ_SECTOR_DATA\n", FUNCNAME);
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
			LOGLIVE("%s - READ_SECTOR_DATA_BYTE\n", FUNCNAME);
			data = cur_live.data_reg;
			set_drq();
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		case READ_TRACK_DATA: {
			LOGLIVE("%s - READ_TRACK_DATA\n", FUNCNAME);
			if(read_one_bit(limit))
				return;

			if(cur_live.bit_counter != 16
				// MFM resyncs
				&& !(!dden && (cur_live.shift_reg == 0x4489
							|| cur_live.shift_reg == 0x5224))
				// FM resyncs
				&& !(dden && (cur_live.shift_reg == 0xf57e      // FM IDAM
							|| cur_live.shift_reg == 0xf56f     // FM DAM
							|| cur_live.shift_reg == 0xf56a))   // FM DDAM
				)
				break;


			// Incorrect, hmmm
			// Probably >2 + not just after a sync if <16

			// Transitions 00..00 -> 4489.4489.4489 at varied syncs:
			//  0: 00.00.14.a1   1: ff.fe.c2.a1   2: 00.01.14.a1   3: ff.fc.c2.a1
			//  4: 00.02.14.a1   5: ff.f8.c2.a1   6: 00.05.14.a1   7: ff.f0.c2.a1
			//  8: 00.00.0a.a1   9: ff.ff.e1.a1  10: 00.00.14.a1  11: ff.ff.ce.a1
			// 12: 00.00.14.a1  13: ff.ff.c2.a1  14: 00.00.14.a1  15: ff.ff.c2.a1

			// MZ: TI99 "DISkASSEMBLER" copy protection requires a threshold of 8
			bool output_byte = cur_live.bit_counter > 8;

			cur_live.data_separator_phase = false;
			cur_live.bit_counter = 0;

			if(output_byte) {
				live_delay(READ_TRACK_DATA_BYTE);
				return;
			}
			break;
		}

		case READ_TRACK_DATA_BYTE:
			LOGLIVE("%s - READ_TRACK_DATA_BYTE\n", FUNCNAME);
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
			if(data != format_last_byte) {
				if(format_last_byte_count) {
					char buf[32];
					if(format_last_byte_count > 1)
						sprintf(buf, "%dx%02x ", format_last_byte_count, format_last_byte);
					else
						sprintf(buf, "%02x ", format_last_byte);
					format_description_string += buf;
				}
				format_last_byte = data;
				format_last_byte_count = 1;
			} else
				format_last_byte_count++;

			if(dden) {
				switch(data) {
				case 0xf7:
					if(cur_live.previous_type == live_info::PT_CRC_2) {
						cur_live.previous_type = live_info::PT_NONE;
						live_write_fm(0xf7);
					} else {
						cur_live.previous_type = live_info::PT_CRC_1;
						live_write_fm(cur_live.crc >> 8);
					}
					break;
				case 0xf8:
					live_write_raw(0xf56a);
					cur_live.crc = 0xffff;
					cur_live.previous_type = live_info::PT_NONE;
					break;
				case 0xf9:
					live_write_raw(0xf56b);
					cur_live.crc = 0xffff;
					cur_live.previous_type = live_info::PT_NONE;
					break;
				case 0xfa:
					live_write_raw(0xf56e);
					cur_live.crc = 0xffff;
					cur_live.previous_type = live_info::PT_NONE;
					break;
				case 0xfb:
					live_write_raw(0xf56f);
					cur_live.crc = 0xffff;
					cur_live.previous_type = live_info::PT_NONE;
					break;
				case 0xfc:
					live_write_raw(0xf77a);
					cur_live.previous_type = live_info::PT_NONE;
					break;
				case 0xfe:
					live_write_raw(0xf57e);
					cur_live.crc = 0xffff;
					cur_live.previous_type = live_info::PT_NONE;
					break;
				default:
					cur_live.previous_type = live_info::PT_NONE;
					live_write_fm(data);
					break;
				}

			} else {
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
					if(dden)
						live_write_fm(cur_live.crc >> 8);
					else
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

				if(dden) {
					if(cur_live.byte_counter < 6)
						live_write_fm(0x00);
					else if(cur_live.byte_counter < 7) {
						cur_live.crc = 0xffff;
						live_write_raw(command & 1 ? 0xf56a : 0xf56f);
					} else if(cur_live.byte_counter < sector_size + 7-1) {
						if(drq) {
							status |= S_LOST;
							data = 0;
						}
						live_write_fm(data);
						set_drq();
					} else if(cur_live.byte_counter < sector_size + 7) {
						if(drq) {
							status |= S_LOST;
							data = 0;
						}
						live_write_fm(data);
					} else if(cur_live.byte_counter < sector_size + 7+2)
						live_write_fm(cur_live.crc >> 8);
					else if(cur_live.byte_counter < sector_size + 7+3)
						live_write_fm(0xff);
					else {
						pll_stop_writing(floppy, cur_live.tm);
						cur_live.state = IDLE;
						return;
					}

				} else {
					if(cur_live.byte_counter < 12)
						live_write_mfm(0x00);
					else if(cur_live.byte_counter < 15)
						live_write_raw(0x4489);
					else if(cur_live.byte_counter < 16) {
						cur_live.crc = 0xcdb4;
						live_write_mfm(command & 1 ? 0xf8 : 0xfb);

					} else if(cur_live.byte_counter < sector_size + 16-1) {
						if(drq) {
							status |= S_LOST;
							data = 0;
						}
						live_write_mfm(data);
						set_drq();
					} else if(cur_live.byte_counter < sector_size + 16) {
						if(drq) {
							status |= S_LOST;
							data = 0;
						}
						live_write_mfm(data);
					} else if(cur_live.byte_counter < sector_size + 16+2)
						live_write_mfm(cur_live.crc >> 8);
					else if(cur_live.byte_counter < sector_size + 16+3)
						live_write_mfm(0xff);
					else {
						pll_stop_writing(floppy, cur_live.tm);
						cur_live.state = IDLE;
						return;
					}
				}


				checkpoint();
				break;

			default:
				logerror("%s: Unknown sub state %d in WRITE_BYTE_DONE\n", tts(cur_live.tm).c_str(), sub_state);
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

			// MZ: There is an inconsistency in the wd177x specs; compare
			// the flow chart and the text of the section "Write sector" (1-9) and
			// pages 1-17 and 1-18.
			//
			// I suppose the sum of the delays in the flow chart should be
			// 11 and 22, so we shorten the 9-byte delay to 8 bytes.

			// case 11:
			case 10:
				if(drq) {
					status |= S_LOST;
					cur_live.state = IDLE;
					return;
				}
				break;
			// case 12:
			case 11:
				if(dden) {
					cur_live.state = WRITE_BYTE;
					cur_live.bit_counter = 16;
					cur_live.byte_counter = 0;
					cur_live.data_bit_context = cur_live.data_reg & 1;
					pll_start_writing(cur_live.tm);
					live_write_fm(0x00);
				}
				break;

			case 22:
				cur_live.state = WRITE_BYTE;
				cur_live.bit_counter = 16;
				cur_live.byte_counter = 0;
				cur_live.data_bit_context = cur_live.data_reg & 1;
				pll_start_writing(cur_live.tm);
				live_write_mfm(0x00);
				break;
			}
			break;

		default:
			logerror("%s: Unknown live state %d\n", tts(cur_live.tm).c_str(), cur_live.state);
			return;
		}
	}
}

void wd_fdc_device_base::set_drq()
{
	if(drq) {
		status |= S_LOST;
		drq = false;
		if(!drq_cb.isnull())
			drq_cb(false);
	} else if(!(status & S_LOST)) {
		drq = true;
		if(!drq_cb.isnull())
			drq_cb(true);
	}
}

void wd_fdc_device_base::drop_drq()
{
	if(drq) {
		drq = false;
		if(!drq_cb.isnull())
			drq_cb(false);
		if (main_state == IDLE) {
			status &= ~S_BUSY;
			intrq = true;
			if(!intrq_cb.isnull())
				intrq_cb(intrq);
		}
	}
}

void wd_fdc_device_base::update_sso()
{
	// The 'side_control' flag is interpreted as meaning that the FDC has
	// a SSO output feature, not that it necessarily controls the floppy.
	if(!side_control)
		return;

	uint8_t side = (command & 0x02) ? 1 : 0;

	// If a SSO callback is defined then it is assumed that this callback
	// will update the floppy side if that is the connection. There are
	// some machines that use the SSO output for other purposes.
	if(!sso_cb.isnull()) {
		sso_cb(side);
		return;
	}

	// If a SSO callback is not defined then assume that the machine
	// intended the driver to update the floppy side which appears to be
	// the case in most cases.
	if(floppy) {
		floppy->ss_w((command & 0x02) ? 1 : 0);
	}
}

int wd_fdc_device_base::calc_sector_size(uint8_t size, uint8_t command) const
{
	return 128 << (size & 3);
}

int wd_fdc_device_base::settle_time() const
{
	return 60000;
}

wd_fdc_analog_device_base::wd_fdc_analog_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	wd_fdc_device_base(mconfig, type, tag, owner, clock)
{
	clock_ratio = 1;
}

void wd_fdc_analog_device_base::pll_reset(bool fm, bool enmf, const attotime &when)
{
	int clocks = 2;

	if (fm)   clocks *= 2;
	if (enmf) clocks *= 2;

	cur_pll.reset(when);
	cur_pll.set_clock(clocks_to_attotime(clocks));
}

void wd_fdc_analog_device_base::pll_start_writing(const attotime &tm)
{
	cur_pll.start_writing(tm);
}

void wd_fdc_analog_device_base::pll_commit(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.commit(floppy, tm);
}

void wd_fdc_analog_device_base::pll_stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.stop_writing(floppy, tm);
}

void wd_fdc_analog_device_base::pll_save_checkpoint()
{
	checkpoint_pll = cur_pll;
}

void wd_fdc_analog_device_base::pll_retrieve_checkpoint()
{
	cur_pll = checkpoint_pll;
}

int wd_fdc_analog_device_base::pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.get_next_bit(tm, floppy, limit);
}

bool wd_fdc_analog_device_base::pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.write_next_bit(bit, tm, floppy, limit);
}

wd_fdc_digital_device_base::wd_fdc_digital_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	wd_fdc_device_base(mconfig, type, tag, owner, clock)
{
	clock_ratio = 4;
}

constexpr int wd_fdc_digital_device_base::wd_digital_step_times[4];

void wd_fdc_digital_device_base::pll_reset(bool fm, bool enmf, const attotime &when)
{
	int clocks = 1;

	if (fm)   clocks *= 2;
	if (enmf) clocks *= 2;

	cur_pll.reset(when);
	cur_pll.set_clock(clocks_to_attotime(clocks));
}

void wd_fdc_digital_device_base::pll_start_writing(const attotime &tm)
{
	cur_pll.start_writing(tm);
}

void wd_fdc_digital_device_base::pll_commit(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.commit(floppy, tm);
}

void wd_fdc_digital_device_base::pll_stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.stop_writing(floppy, tm);
}

int wd_fdc_digital_device_base::pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.get_next_bit(tm, floppy, limit);
}

bool wd_fdc_digital_device_base::pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.write_next_bit(bit, tm, floppy, limit);
}

void wd_fdc_digital_device_base::pll_save_checkpoint()
{
	checkpoint_pll = cur_pll;
}

void wd_fdc_digital_device_base::pll_retrieve_checkpoint()
{
	cur_pll = checkpoint_pll;
}

void wd_fdc_digital_device_base::digital_pll_t::set_clock(const attotime &period)
{
	for(int i=0; i<42; i++)
		delays[i] = period*(i+1);
}

void wd_fdc_digital_device_base::digital_pll_t::reset(const attotime &when)
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

int wd_fdc_digital_device_base::digital_pll_t::get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	attotime when = floppy ? floppy->get_next_transition(ctime) : attotime::never;

	/*
	    if(!when.is_never())
	        LOGTRANSITION("transition_time=%s\n", tts(when).c_str());
	*/
	for(;;) {
		// LOGTRANSITION("slot=%2d, counter=%03x\n", slot, counter);
		attotime etime = ctime+delays[slot];
		// LOGTRANSITION("etime=%s\n", tts(etime).c_str());
		if(etime > limit)
			return -1;
		if(transition_time == 0xffff && !when.is_never() && etime >= when)
			transition_time = counter;
		if(slot < 8) {
			uint8_t mask = 1 << slot;
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
	//LOGTRANSITION("first transition, time=%03x, inc=%3d\n", transition_time, increment);
	int bit = transition_time != 0xffff;

	if(transition_time != 0xffff) {
		static const uint8_t pha[8] = { 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 };
		static const uint8_t phs[8] = { 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf };
		static const uint8_t freqa[4][8] = {
			{ 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0 }
		};
		static const uint8_t freqs[4][8] = {
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

void wd_fdc_digital_device_base::digital_pll_t::start_writing(const attotime &tm)
{
	write_start_time = tm;
	write_position = 0;
}

void wd_fdc_digital_device_base::digital_pll_t::stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	commit(floppy, tm);
	write_start_time = attotime::never;
}

bool wd_fdc_digital_device_base::digital_pll_t::write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	if(write_start_time.is_never()) {
		write_start_time = ctime;
		write_position = 0;
	}

	for(;;) {
		attotime etime = ctime+delays[slot];
		if(etime > limit)
			return true;
		uint16_t pre_counter = counter;
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

void wd_fdc_digital_device_base::digital_pll_t::commit(floppy_image_device *floppy, const attotime &tm)
{
	if(write_start_time.is_never() || tm == write_start_time)
		return;

	if(floppy)
		floppy->write_flux(write_start_time, tm, write_position, write_buffer);
	write_start_time = tm;
	write_position = 0;
}

fd1771_device::fd1771_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1771, tag, owner, clock)
{
	constexpr static int fd1771_step_times[4] = { 12000, 12000, 20000, 40000 };

	step_times = fd1771_step_times;
	delay_register_commit = 16;
	delay_command_commit = 20; // x2 due to fm
	disable_mfm = true;
	inverted_bus = true;
	side_control = false;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int fd1771_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return size ? size << 4 : 4096;
}

fd1781_device::fd1781_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1781, tag, owner, clock)
{
	constexpr static int fd1781_step_times[4] = { 6000, 12000, 20000, 40000 };

	step_times = fd1781_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	inverted_bus = true;
	side_control = false;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int fd1781_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return size ? size << 4 : 4096;
}

constexpr int wd_fdc_device_base::fd179x_step_times[4];
constexpr int wd_fdc_device_base::fd176x_step_times[4];

fd1791_device::fd1791_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1791, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1792_device::fd1792_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1792, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = true;
	has_enmf = false;
	inverted_bus = true;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1793_device::fd1793_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1793, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

kr1818vg93_device::kr1818vg93_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, KR1818VG93, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1794_device::fd1794_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1794, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = true;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1795_device::fd1795_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1795, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = true;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int fd1795_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return 128 << ((size + 1) & 3);
}

fd1797_device::fd1797_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1797, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = true;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int fd1797_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return 128 << ((size + 1) & 3);
}

mb8866_device::mb8866_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, MB8866, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

mb8876_device::mb8876_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, MB8876, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

mb8877_device::mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, MB8877, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 4;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1761_device::fd1761_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1761, tag, owner, clock)
{
	step_times = fd176x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1763_device::fd1763_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1763, tag, owner, clock)
{
	step_times = fd176x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

fd1765_device::fd1765_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1765, tag, owner, clock)
{
	step_times = fd176x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = true;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int fd1765_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return 128 << ((size + 1) & 3);
}

fd1767_device::fd1767_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, FD1767, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = true;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int fd1767_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return 128 << ((size + 1) & 3);
}

wd2791_device::wd2791_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, WD2791, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = true;
	inverted_bus = true;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

wd2793_device::wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, WD2793, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = true;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

wd2795_device::wd2795_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, WD2795, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = true;
	side_control = true;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int wd2795_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return 128 << ((size + 1) & 3);
}

wd2797_device::wd2797_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_analog_device_base(mconfig, WD2797, tag, owner, clock)
{
	step_times = fd179x_step_times;
	delay_register_commit = 16;
	delay_command_commit = 12;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = true;
	side_compare = false;
	head_control = true;
	motor_control = false;
	ready_hooked = true;
}

int wd2797_device::calc_sector_size(uint8_t size, uint8_t command) const
{
	if(command & 0x08)
		return 128 << (size & 3);
	else
		return 128 << ((size + 1) & 3);
}

wd1770_device::wd1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_digital_device_base(mconfig, WD1770, tag, owner, clock)
{
	step_times = wd_digital_step_times;
	delay_register_commit = 32;
	delay_command_commit = 36; // official 48 is too high for oric jasmin boot
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = false;
	head_control = false;
	motor_control = true;
	ready_hooked = false;
}

wd1772_device::wd1772_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_digital_device_base(mconfig, WD1772, tag, owner, clock)
{
	const static int wd1772_step_times[4] = { 12000, 24000, 4000, 6000 };

	step_times = wd1772_step_times;
	delay_register_commit = 32;
	delay_command_commit = 48;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = false;
	head_control = false;
	motor_control = true;
	ready_hooked = false;
	
	/* Sam Coupe/+D/Disciple expect a 0xd0 force interrupt command to cause a spin-up.
	   eg. +D issues 2x 0xd0, then waits for index pulses to start, bails out with no disk error if that doesn't happen.
	   Not sure if other chips should do this too? */
	spinup_on_interrupt = true;
}

int wd1772_device::settle_time() const
{
	return 30000;
}

wd1773_device::wd1773_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : wd_fdc_digital_device_base(mconfig, WD1773, tag, owner, clock)
{
	step_times = wd_digital_step_times;
	delay_register_commit = 32;
	delay_command_commit = 48;
	disable_mfm = false;
	has_enmf = false;
	inverted_bus = false;
	side_control = false;
	side_compare = true;
	head_control = false;
	motor_control = false;
	ready_hooked = true;
}
