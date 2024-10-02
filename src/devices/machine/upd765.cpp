// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "upd765.h"
#include "imagedev/floppy.h"
#include "debugger.h"

#define LOG_WARN    (1U << 1)   // Show warnings
#define LOG_SHIFT   (1U << 2)   // Shows shift register contents
#define LOG_HEADER  (1U << 3)   // Shows ID fields
#define LOG_FORMAT  (1U << 4)   // Sectors being formatted
#define LOG_TCIRQ   (1U << 5)   // Termination code line / interrupt
#define LOG_REGS    (1U << 6)   // Digital input/output register and data rate select
#define LOG_FIFO    (1U << 7)   // FIFO operations
#define LOG_COMMAND (1U << 8)   // Commands
#define LOG_RW      (1U << 9)   // Read/write sector or track
#define LOG_MATCH   (1U << 10)  // Sector matching
#define LOG_STATE   (1U << 11)  // State machine
#define LOG_LIVE    (1U << 12)  // Live states
#define LOG_DONE    (1U << 13)  // Command done

#define VERBOSE (LOG_GENERAL | LOG_WARN)

#include "logmacro.h"

#include <iomanip>
#include <sstream>

#define LOGWARN(...)     LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGSHIFT(...)    LOGMASKED(LOG_SHIFT, __VA_ARGS__)
#define LOGHEADER(...)   LOGMASKED(LOG_HEADER, __VA_ARGS__)
#define LOGFORMAT(...)   LOGMASKED(LOG_FORMAT, __VA_ARGS__)
#define LOGTCIRQ(...)    LOGMASKED(LOG_TCIRQ, __VA_ARGS__)
#define LOGREGS(...)     LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGFIFO(...)     LOGMASKED(LOG_FIFO, __VA_ARGS__)
#define LOGCOMMAND(...)  LOGMASKED(LOG_COMMAND, __VA_ARGS__)
#define LOGRW(...)       LOGMASKED(LOG_RW, __VA_ARGS__)
#define LOGMATCH(...)    LOGMASKED(LOG_MATCH, __VA_ARGS__)
#define LOGSTATE(...)    LOGMASKED(LOG_STATE, __VA_ARGS__)
#define LOGLIVE(...)     LOGMASKED(LOG_LIVE, __VA_ARGS__)
#define LOGDONE(...)     LOGMASKED(LOG_DONE, __VA_ARGS__)

DEFINE_DEVICE_TYPE(UPD765A,        upd765a_device,        "upd765a",        "NEC uPD765A FDC")
DEFINE_DEVICE_TYPE(UPD765B,        upd765b_device,        "upd765b",        "NEC uPD765B FDC")
DEFINE_DEVICE_TYPE(I8272A,         i8272a_device,         "i8272a",         "Intel 8272A FDC")
DEFINE_DEVICE_TYPE(UPD72065,       upd72065_device,       "upd72065",       "NEC uPD72065 FDC")
DEFINE_DEVICE_TYPE(UPD72067,       upd72067_device,       "upd72067",       "NEC uPD72067 FDC")
DEFINE_DEVICE_TYPE(UPD72069,       upd72069_device,       "upd72069",       "NEC uPD72069 FDC")
DEFINE_DEVICE_TYPE(I82072,         i82072_device,         "i82072",         "Intel 82072 FDC")
DEFINE_DEVICE_TYPE(SMC37C78,       smc37c78_device,       "smc37c78",       "SMC FDC37C78 FDC")
DEFINE_DEVICE_TYPE(N82077AA,       n82077aa_device,       "n82077aa",       "Intel N82077AA FDC")
DEFINE_DEVICE_TYPE(PC_FDC_SUPERIO, pc_fdc_superio_device, "pc_fdc_superio", "Winbond PC FDC Super I/O")
DEFINE_DEVICE_TYPE(DP8473,         dp8473_device,         "dp8473",         "National Semiconductor DP8473 FDC")
DEFINE_DEVICE_TYPE(PC8477A,        pc8477a_device,        "pc8477a",        "National Semiconductor PC8477A FDC")
DEFINE_DEVICE_TYPE(PC8477B,        pc8477b_device,        "pc8477b",        "National Semiconductor PC8477B FDC")
DEFINE_DEVICE_TYPE(WD37C65C,       wd37c65c_device,       "wd37c65c",       "Western Digital WD37C65C FDC")
DEFINE_DEVICE_TYPE(MCS3201,        mcs3201_device,        "mcs3201",        "Motorola MCS3201 FDC")
DEFINE_DEVICE_TYPE(TC8566AF,       tc8566af_device,       "tc8566af",       "Toshiba TC8566AF FDC")
DEFINE_DEVICE_TYPE(HD63266F,       hd63266f_device,       "hd63266f",       "Hitachi HD63266F FDC")

void upd765a_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(upd765a_device::msr_r));
	map(0x1, 0x1).rw(FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
}

void upd765b_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(upd765b_device::msr_r));
	map(0x1, 0x1).rw(FUNC(upd765b_device::fifo_r), FUNC(upd765b_device::fifo_w));
}

void i8272a_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(i8272a_device::msr_r));
	map(0x1, 0x1).rw(FUNC(i8272a_device::fifo_r), FUNC(i8272a_device::fifo_w));
}

void upd72065_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(upd72065_device::msr_r), FUNC(upd72065_device::auxcmd_w));
	map(0x1, 0x1).rw(FUNC(upd72065_device::fifo_r), FUNC(upd72065_device::fifo_w));
}

void i82072_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(i82072_device::msr_r), FUNC(i82072_device::dsr_w));
	map(0x1, 0x1).rw(FUNC(i82072_device::fifo_r), FUNC(i82072_device::fifo_w));
}

void smc37c78_device::map(address_map &map)
{
	map(0x2, 0x2).rw(FUNC(smc37c78_device::dor_r), FUNC(smc37c78_device::dor_w));
	map(0x3, 0x3).rw(FUNC(smc37c78_device::tdr_r), FUNC(smc37c78_device::tdr_w));
	map(0x4, 0x4).rw(FUNC(smc37c78_device::msr_r), FUNC(smc37c78_device::dsr_w));
	map(0x5, 0x5).rw(FUNC(smc37c78_device::fifo_r), FUNC(smc37c78_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(smc37c78_device::dir_r), FUNC(smc37c78_device::ccr_w));
}

void n82077aa_device::map(address_map &map)
{
	if(mode != mode_t::AT) {
		map(0x0, 0x0).r(FUNC(n82077aa_device::sra_r));
		map(0x1, 0x1).r(FUNC(n82077aa_device::srb_r));
	}
	map(0x2, 0x2).rw(FUNC(n82077aa_device::dor_r), FUNC(n82077aa_device::dor_w));
	map(0x3, 0x3).rw(FUNC(n82077aa_device::tdr_r), FUNC(n82077aa_device::tdr_w));
	map(0x4, 0x4).rw(FUNC(n82077aa_device::msr_r), FUNC(n82077aa_device::dsr_w));
	map(0x5, 0x5).rw(FUNC(n82077aa_device::fifo_r), FUNC(n82077aa_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(n82077aa_device::dir_r), FUNC(n82077aa_device::ccr_w));
}

void pc_fdc_superio_device::map(address_map &map)
{
	map(0x2, 0x2).w(FUNC(pc_fdc_superio_device::dor_w));
	map(0x3, 0x3).w(FUNC(pc_fdc_superio_device::tdr_w));
	map(0x4, 0x4).r(FUNC(pc_fdc_superio_device::msr_r));
	map(0x5, 0x5).rw(FUNC(pc_fdc_superio_device::fifo_r), FUNC(pc_fdc_superio_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(pc_fdc_superio_device::dir_r), FUNC(pc_fdc_superio_device::ccr_w));
}

void dp8473_device::map(address_map &map)
{
	map(0x2, 0x2).w(FUNC(dp8473_device::dor_w));
	map(0x4, 0x4).r(FUNC(dp8473_device::msr_r));
	map(0x5, 0x5).rw(FUNC(dp8473_device::fifo_r), FUNC(dp8473_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(dp8473_device::dir_r), FUNC(dp8473_device::ccr_w));
}

void pc8477a_device::map(address_map &map)
{
	if(mode != mode_t::AT) {
		map(0x0, 0x0).r(FUNC(pc8477a_device::sra_r));
		map(0x1, 0x1).r(FUNC(pc8477a_device::srb_r));
	}
	map(0x2, 0x2).rw(FUNC(pc8477a_device::dor_r), FUNC(pc8477a_device::dor_w));
	map(0x3, 0x3).rw(FUNC(pc8477a_device::tdr_r), FUNC(pc8477a_device::tdr_w));
	map(0x4, 0x4).rw(FUNC(pc8477a_device::msr_r), FUNC(pc8477a_device::dsr_w));
	map(0x5, 0x5).rw(FUNC(pc8477a_device::fifo_r), FUNC(pc8477a_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(pc8477a_device::dir_r), FUNC(pc8477a_device::ccr_w));
}

void pc8477b_device::map(address_map &map)
{
	if(mode != mode_t::AT) {
		map(0x0, 0x0).r(FUNC(pc8477b_device::sra_r));
		map(0x1, 0x1).r(FUNC(pc8477b_device::srb_r));
	}
	map(0x2, 0x2).rw(FUNC(pc8477b_device::dor_r), FUNC(pc8477b_device::dor_w));
	map(0x3, 0x3).rw(FUNC(pc8477b_device::tdr_r), FUNC(pc8477b_device::tdr_w));
	map(0x4, 0x4).rw(FUNC(pc8477b_device::msr_r), FUNC(pc8477b_device::dsr_w));
	map(0x5, 0x5).rw(FUNC(pc8477b_device::fifo_r), FUNC(pc8477b_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(pc8477b_device::dir_r), FUNC(pc8477b_device::ccr_w));
}

void wd37c65c_device::map(address_map &map)
{
	// NOTE: this map only covers registers defined by CS.
	// LDOR and LDCR must be mapped separately, since their addresses are
	// defined only by external decoding circuits. LDIR (optional) is also separate.
	map(0x0, 0x0).r(FUNC(wd37c65c_device::msr_r));
	map(0x1, 0x1).rw(FUNC(wd37c65c_device::fifo_r), FUNC(wd37c65c_device::fifo_w));
}

void mcs3201_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(mcs3201_device::input_r));
	map(0x2, 0x2).w(FUNC(mcs3201_device::dor_w));
	map(0x4, 0x4).r(FUNC(mcs3201_device::msr_r));
	map(0x5, 0x5).rw(FUNC(mcs3201_device::fifo_r), FUNC(mcs3201_device::fifo_w));
	map(0x7, 0x7).rw(FUNC(mcs3201_device::dir_r), FUNC(mcs3201_device::ccr_w));
}

void tc8566af_device::map(address_map &map)
{
	map(0x2, 0x2).w(FUNC(tc8566af_device::dor_w));
	map(0x3, 0x3).w(FUNC(tc8566af_device::cr1_w));
	map(0x4, 0x4).r(FUNC(tc8566af_device::msr_r));
	map(0x5, 0x5).rw(FUNC(tc8566af_device::fifo_r), FUNC(tc8566af_device::fifo_w));
}


constexpr int upd765_family_device::rates[4];

upd765_family_device::upd765_family_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	ready_connected(true),
	ready_polled(true),
	select_connected(true),
	select_multiplexed(true),
	has_dor(true),
	external_ready(false),
	recalibrate_steps(77),
	mode(mode_t::AT),
	intrq_cb(*this),
	drq_cb(*this),
	hdl_cb(*this),
	idx_cb(*this),
	us_cb(*this)
{
}

void upd765_family_device::set_ready_line_connected(bool _ready)
{
	ready_connected = _ready;
}

void upd765_family_device::set_select_lines_connected(bool _select)
{
	select_connected = _select;
}

void ps2_fdc_device::set_mode(mode_t _mode)
{
	mode = _mode;
}

void upd765_family_device::device_start()
{
	save_item(NAME(selected_drive));
	save_item(NAME(drive_busy));

	for(int i=0; i != 4; i++) {
		char name[2];
		flopi[i].tm = timer_alloc(FUNC(upd765_family_device::update_floppy), this);
		flopi[i].id = i;
		if(select_connected) {
			name[0] = '0'+i;
			name[1] = 0;
			floppy_connector *con = subdevice<floppy_connector>(name);
			if(con) {
				flopi[i].dev = con->get_device();
				if(flopi[i].dev != nullptr)
					flopi[i].dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&upd765_family_device::index_callback, this));
			} else
				flopi[i].dev = nullptr;
		} else
			flopi[i].dev = nullptr;

		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].dir = 0;
		flopi[i].counter = 0;
		flopi[i].pcn = 0;
		flopi[i].st0 = 0;
		flopi[i].st0_filled = false;
		flopi[i].live = false;
		flopi[i].index = false;
		flopi[i].ready = false;
	}
	cur_rate = 250000;
	tc = false;
	selected_drive = -1;
	dor = 0x0c;

	// reset at upper levels may cause a write to tc ending up with
	// live_sync, which will crash if the live structure isn't
	// initialized enough

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.fi = nullptr;

	if(ready_polled) {
		poll_timer = timer_alloc(FUNC(upd765_family_device::run_drive_ready_polling), this);
		poll_timer->adjust(attotime::from_usec(100), 0, attotime::from_usec(1024));
	} else
		poll_timer = nullptr;

	cur_irq = false;
	locked = false;
}

void upd765_family_device::device_reset()
{
	if(has_dor)
		dor = 0x00;
	locked = false;
	soft_reset();
}

void upd765_family_device::soft_reset()
{
	main_phase = PHASE_CMD;
	for(int i=0; i<4; i++) {
		flopi[i].main_state = IDLE;
		flopi[i].sub_state = IDLE;
		flopi[i].live = false;
		flopi[i].ready = !ready_polled;
		flopi[i].st0 = i;
		flopi[i].st0_filled = false;
	}
	clr_drive_busy();
	irq = false;
	internal_drq = false;
	fifo_pos = 0;
	command_pos = 0;
	result_pos = 0;
	if(!locked)
		fifocfg = FIF_DIS;
	cur_live.fi = nullptr;
	drq = false;
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.fi = nullptr;
	tc_done = false;
	st1 = st2 = st3 = 0x00;

	set_ds(select_multiplexed ? 0 : -1);

	check_irq();
	if(BIT(dor, 2))
		end_reset();
	else if(ready_polled)
		poll_timer->adjust(attotime::never);
}

void upd765_family_device::end_reset()
{
	if(ready_polled)
		poll_timer->adjust(attotime::from_usec(100), 0, attotime::from_usec(1024));
}

void upd765_family_device::tc_w(bool _tc)
{
	LOGTCIRQ("tc=%d\n", _tc);
	if(tc != _tc && _tc) {
		live_sync();
		tc_done = true;
		tc = _tc;
		if(cur_live.fi)
			general_continue(*cur_live.fi);
	} else
		tc = _tc;
}

void upd765_family_device::ready_w(bool _ready)
{
	external_ready = _ready;
}

bool upd765_family_device::get_ready(int fid)
{
	if(ready_connected)
		return flopi[fid].dev ? !flopi[fid].dev->ready_r() : false;
	return !external_ready;
}

void upd765_family_device::reset_w(int state)
{
	// This implementation is not valid for devices with DOR and possibly other extra registers.
	// The working assumption is that no need to manipulate the RESET line directly when software can use DOR instead.
	assert(!has_dor);
	if(bool(state) == !BIT(dor, 2))
		return;

	LOGREGS("reset = %d\n", state);
	if(state) {
		dor &= 0xfb;
		soft_reset();
	}
	else {
		dor |= 0x04;
		end_reset();
	}
}

void upd765_family_device::set_ds(int fid)
{
	if(selected_drive == fid)
		return;

	// pass drive select to connected drives
	for(floppy_info &fi : flopi)
		if(fi.dev)
			fi.dev->ds_w(fid);
	us_cb(fid);

	// record selected drive
	selected_drive = fid;
}

void upd765_family_device::set_floppy(floppy_image_device *flop)
{
	for(floppy_info & elem : flopi) {
		if(elem.dev) {
			elem.dev->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
			if(elem.dev != flop)
				elem.dev->ds_w(-1);
		}
		elem.dev = flop;
	}
	if(flop)
		flop->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&upd765_family_device::index_callback, this));
	else
		idx_cb(0);
}

uint8_t ps2_fdc_device::sra_r()
{
	uint8_t sra = 0;
	floppy_info &fi = flopi[dor & 3];
	if(fi.dir)
		sra |= 0x01;
	if(fi.index)
		sra |= 0x04;
	if(cur_rate >= 500000)
		sra |= 0x08;
	if(fi.dev && fi.dev->trk00_r())
		sra |= 0x10;
	if(fi.main_state == SEEK_WAIT_STEP_SIGNAL_TIME)
		sra |= 0x20;
	if(cur_irq)
		sra |= 0x80;
	if(mode == mode_t::M30)
	{
		sra ^= 0x1f;
		if(drq)
			sra |= 0x40;
	}
	else
	{
		if(!flopi[1].dev)
			sra |= 0x40;
	}
	return sra;
}

uint8_t ps2_fdc_device::srb_r()
{
	uint8_t srb = 0;
	// TODO: rddata, wrdata, write enable bits
	if(mode == mode_t::M30)
	{
		const uint8_t ds[4] = { 0x43, 0x23, 0x62, 0x61 };
		srb = ds[dor & 3];
		if(!flopi[1].dev)
			srb |= 0x80;
	}
	else
	{
		srb = 0xc0 | ((dor & 1) << 6) | ((dor & 0x30) >> 4);
	}
	return srb;
}

uint8_t upd765_family_device::dor_r()
{
	return dor;
}

void upd765_family_device::dor_w(uint8_t data)
{
	assert(has_dor);
	LOGREGS("dor = %02x\n", data);
	uint8_t diff = dor ^ data;
	dor = data;
	if(BIT(diff, 2)) {
		if(BIT(data, 2))
			end_reset();
		else
			soft_reset();
	}

	for(int i=0; i<4; i++) {
		floppy_info &fi = flopi[i];
		if(fi.dev)
			fi.dev->mon_w(!(dor & (0x10 << i)));
	}
	check_irq();
}

uint8_t upd765_family_device::tdr_r()
{
	return 0;
}

void upd765_family_device::tdr_w(uint8_t data)
{
}

uint8_t upd765_family_device::msr_r()
{
	uint8_t msr = 0;
	switch(main_phase) {
	case PHASE_CMD:
		msr |= MSR_RQM;
		if(command_pos)
			msr |= MSR_CB;
		break;
	case PHASE_EXEC:
		msr |= MSR_CB;
		if(spec & SPEC_ND)
			msr |= MSR_EXM;
		if(internal_drq) {
			msr |= MSR_RQM;
			if(!fifo_write)
				msr |= MSR_DIO;
		}
		break;

	case PHASE_RESULT:
		msr |= MSR_RQM|MSR_DIO|MSR_CB;
		break;
	}
	for(int i=0; i<4; i++) {
		if(flopi[i].main_state == RECALIBRATE || flopi[i].main_state == SEEK) {
			msr |= 1<<i;
			//msr |= MSR_CB;
		}
	}
	msr |= get_drive_busy();

	return msr;
}

void upd765_family_device::dsr_w(uint8_t data)
{
	LOGREGS("dsr_w %02x (%s)\n", data, machine().describe_context());
	if(data & 0x80)
		soft_reset();
	dsr = data & 0x7f;
	cur_rate = rates[dsr & 3];
}

void upd765_family_device::set_rate(int rate)
{
	cur_rate = rate;
}

uint8_t upd765_family_device::fifo_r()
{
	uint8_t r = 0xff;
	if(!machine().side_effects_disabled())
	{
		irq = false;
		check_irq();
	}
	switch(main_phase) {
	case PHASE_CMD:
		if(machine().side_effects_disabled())
			return 0xff;
		if(command_pos)
			fifo_w(0xff);
		LOGFIFO("fifo_r in command phase\n");
		break;
	case PHASE_EXEC:
		if(machine().side_effects_disabled())
			return fifo[0];
		if(internal_drq)
			return fifo_pop(false);
		LOGFIFO("fifo_r in execution phase\n");
		break;

	case PHASE_RESULT:
		r = result[0];
		if(!machine().side_effects_disabled()) {
			result_pos--;
			memmove(result, result+1, result_pos);
			if(!result_pos)
				main_phase = PHASE_CMD;
			else if(result_pos == 1) {
				// clear drive busy bit after the first sense interrupt status result byte is read
				for(floppy_info &fi : flopi)
					if((fi.main_state == RECALIBRATE || fi.main_state == SEEK) && fi.sub_state == IDLE && fi.st0_filled == false)
						fi.main_state = IDLE;
				clr_drive_busy();
			}
		}
		break;
	default:
		LOGFIFO("fifo_r in phase %d\n", main_phase);
		break;
	}

	return r;
}

void upd765_family_device::fifo_w(uint8_t data)
{
	if(!BIT(dor, 2))
		LOGWARN("%s: fifo_w(%02x) in reset\n", machine().describe_context(), data);

	irq = false;
	check_irq();

	switch(main_phase) {
	case PHASE_CMD: {
		command[command_pos++] = data;
		int cmd = check_command();
		if(cmd == C_INCOMPLETE)
			break;
		if(cmd == C_INVALID) {
			LOGFIFO("Invalid on %02x\n", command[0]);
			main_phase = PHASE_RESULT;
			result[0] = ST0_UNK;
			result_pos = 1;
			command_pos = 0;
			return;
		}
		start_command(cmd);
		break;
	}
	case PHASE_EXEC:
		if(internal_drq) {
			fifo_push(data, false);
			return;
		}
		LOGFIFO("fifo_w in execution phase\n");
		break;

	default:
		LOGFIFO("fifo_w in phase %d\n", main_phase);
		break;
	}
}

uint8_t upd765_family_device::do_dir_r()
{
	floppy_info &fi = flopi[dor & 3];
	if(fi.dev)
		return fi.dev->dskchg_r() ? 0x00 : 0x80;
	return 0x00;
}

void upd765_family_device::ccr_w(uint8_t data)
{
	dsr = (dsr & 0xfc) | (data & 3);
	cur_rate = rates[data & 3];
}

void upd765_family_device::set_drq(bool state)
{
	if(state != drq) {
		drq = state;
		drq_cb(drq);
	}
}

bool upd765_family_device::get_drq() const
{
	return drq;
}

void upd765_family_device::enable_transfer()
{
	if(spec & SPEC_ND) {
		// PIO
		if(!internal_drq) {
			internal_drq = true;
			check_irq();
		}

	}
	// DMA
	if(!drq)
		set_drq(true);
}

void upd765_family_device::disable_transfer()
{
	if(spec & SPEC_ND) {
		internal_drq = false;
		check_irq();
	}
	set_drq(false);
}

void upd765_family_device::fifo_push(uint8_t data, bool internal)
{
	// MZ: A bit speculative. These lines help to avoid some FIFO mess-up
	// with the HX5102 that happens when WRITE DATA fails to find the sector
	// but the host already starts pushing the sector data. Should not hurt.
	if(fifo_expected == 0) {
		LOGFIFO("Fifo not expecting data, discarding\n");
		return;
	}

	if(fifo_pos == 16) {
		if(internal) {
			if(!(st1 & ST1_OR))
				LOGFIFO("Fifo overrun\n");
			st1 |= ST1_OR;
		}
		return;
	}
	fifo[fifo_pos++] = data;
	fifo_expected--;

	int thr = (fifocfg & FIF_THR)+1;
	if(!fifo_write && (!fifo_expected || fifo_pos >= thr || (fifocfg & FIF_DIS)))
		enable_transfer();
	if(fifo_write && (fifo_pos == 16 || !fifo_expected))
		disable_transfer();
}


uint8_t upd765_family_device::fifo_pop(bool internal)
{
	if(!fifo_pos) {
		if(internal) {
			if(!(st1 & ST1_OR))
				LOGFIFO("Fifo underrun\n");
			st1 |= ST1_OR;
		}
		return 0;
	}
	uint8_t r = fifo[0];
	fifo_pos--;
	memmove(fifo, fifo+1, fifo_pos);
	if(!fifo_write && !fifo_pos)
		disable_transfer();
	int thr = fifocfg & FIF_THR;
	if(fifo_write && fifo_expected && (fifo_pos <= thr || (fifocfg & FIF_DIS)))
		enable_transfer();
	return r;
}

void upd765_family_device::fifo_expect(int size, bool write)
{
	fifo_expected = size;
	fifo_write = write;
	if(fifo_write)
		enable_transfer();
}

uint8_t upd765_family_device::dma_r()
{
	if(machine().side_effects_disabled())
		return fifo[0];
	return fifo_pop(false);
}

void upd765_family_device::dma_w(uint8_t data)
{
	fifo_push(data, false);
}

void upd765_family_device::live_start(floppy_info &fi, int state)
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
	cur_live.pll.set_clock(attotime::from_hz(mfm ? 2*cur_rate : cur_rate));
	checkpoint_live = cur_live;
	fi.live = true;

	live_run();
}

void upd765_family_device::checkpoint()
{
	if(cur_live.fi)
		cur_live.pll.commit(cur_live.fi->dev, cur_live.tm);
	checkpoint_live = cur_live;
}

void upd765_family_device::rollback()
{
	cur_live = checkpoint_live;
}

void upd765_family_device::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		cur_live.fi->tm->adjust(cur_live.tm - machine().time(), cur_live.fi->id);
	else
		live_sync();
}

void upd765_family_device::live_sync()
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
				cur_live.fi = nullptr;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void upd765_family_device::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	if(cur_live.fi) {
		cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
		cur_live.fi->live = false;
		cur_live.fi = nullptr;
	}

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

void upd765_family_device::live_run(attotime limit)
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
			cur_live.fi->tm->adjust(attotime::from_msec(1), cur_live.fi->id);
		}
	}

	for(;;) {
		switch(cur_live.state) {
		case SEARCH_ADDRESS_MARK_HEADER:
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x c=%d\n", cur_live.tm.to_string(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter);

			if(mfm && cur_live.shift_reg == 0x4489) {
				cur_live.crc = 0x443b;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_HEADER_BLOCK_HEADER;
				LOGLIVE("%s: Found A1\n", cur_live.tm.to_string());
			}

			if(!mfm && cur_live.shift_reg == 0xf57e) {
				cur_live.crc = 0xef21;
				cur_live.data_separator_phase = false;
				cur_live.bit_counter = 0;
				cur_live.state = READ_ID_BLOCK;
				LOGLIVE("%s: Found IDAM\n", cur_live.tm.to_string());
			}
			break;

		case READ_HEADER_BLOCK_HEADER: {
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x counter=%d\n", cur_live.tm.to_string(), cur_live.shift_reg,
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
				else
					LOGLIVE("%s: Found A1\n", cur_live.tm.to_string());
				break;
			}
			if(cur_live.data_reg != 0xfe) {
				LOGLIVE("%s: No ident byte found after triple-A1, continue search\n", cur_live.tm.to_string());
				cur_live.state = SEARCH_ADDRESS_MARK_HEADER;
				break;
			}

			cur_live.bit_counter = 0;
			cur_live.state = READ_ID_BLOCK;

			break;
		}

		case READ_ID_BLOCK: {
			if(read_one_bit(limit))
				return;
			if(cur_live.bit_counter & 15)
				break;
			int slot = (cur_live.bit_counter >> 4)-1;

			LOGLIVE("%s: slot=%d data=%02x crc=%04x\n", cur_live.tm.to_string(), slot, cur_live.data_reg, cur_live.crc);
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

			LOGSHIFT("%s: shift = %04x data=%02x c=%d.%x\n", cur_live.tm.to_string(), cur_live.shift_reg,
					(cur_live.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(cur_live.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(cur_live.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(cur_live.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(cur_live.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(cur_live.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(cur_live.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(cur_live.shift_reg & 0x0001 ? 0x01 : 0x00),
					cur_live.bit_counter >> 4, cur_live.bit_counter & 15);

			if(mfm) {
				// Large tolerance due to perpendicular recording at extended density
				if(cur_live.bit_counter > 62*16) {
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

				if(cur_live.bit_counter >= 11*16 && (cur_live.shift_reg == 0xf56a || cur_live.shift_reg == 0xf56f)) {
					cur_live.crc = cur_live.shift_reg == 0xf56a ? 0x8fe7 : 0xbf84;
					cur_live.data_separator_phase = false;
					cur_live.bit_counter = 0;
					cur_live.state = READ_SECTOR_DATA;
				}
			}

			break;

		case READ_DATA_BLOCK_HEADER: {
			if(read_one_bit(limit))
				return;

			LOGSHIFT("%s: shift = %04x data=%02x counter=%d\n", cur_live.tm.to_string(), cur_live.shift_reg,
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
			if(cur_live.data_reg != 0xfb && cur_live.data_reg != 0xf8) {
				live_delay(SEARCH_ADDRESS_MARK_DATA_FAILED);
				return;
			}

			if (
				((command[0] & 0x08) == 0 && cur_live.data_reg == 0xf8) // Encountered deleted sector during read data
				|| ((command[0] & 0x08) != 0 && cur_live.data_reg == 0xfb) // Encountered normal sector during read deleted data
			) {
				st2 |= ST2_CM;
			}

			cur_live.bit_counter = 0;
			cur_live.state = READ_SECTOR_DATA;
			break;
		}

		case SEARCH_ADDRESS_MARK_DATA_FAILED:
			st1 |= ST1_MA;
			st2 |= ST2_MD;
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
			if(!tc_done)
				fifo_push(cur_live.data_reg, true);
			cur_live.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		case SCAN_SECTOR_DATA_BYTE:
			if(!scan_done) { // TODO: handle stp, x68000 sets it to 0xff (as it would dtl)?
				int slot = (cur_live.bit_counter >> 4)-1;
				uint8_t data = fifo_pop(true);
				if(!slot)
					st2 = (st2 & ~(ST2_SN)) | ST2_SH;

				if(data != cur_live.data_reg) {
					st2 = (st2 & ~(ST2_SH)) | ST2_SN;
					if((data < cur_live.data_reg) && ((command[0] & 0x1f) == 0x19)) // low
						st2 &= ~ST2_SN;

					if((data > cur_live.data_reg) && ((command[0] & 0x1f) == 0x1d)) // high
						st2 &= ~ST2_SN;
				}
				if((slot == sector_size) && !(st2 & ST2_SN)) {
					scan_done = true;
					tc_done = true;
				}
			} else {
				if(fifo_pos)
					fifo_pop(true);
			}
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
			if(mfm && cur_live.bit_counter != 22*16)
				break;
			if(!mfm && cur_live.bit_counter != 11*16)
				break;
			cur_live.bit_counter = 0;
			cur_live.byte_counter = 0;
			live_delay(WRITE_SECTOR_DATA);
			return;

		case WRITE_SECTOR_DATA:
			if(mfm) {
				if(cur_live.byte_counter < 12)
					live_write_mfm(0x00);
				else if(cur_live.byte_counter < 15)
					live_write_raw(0x4489);
				else if(cur_live.byte_counter < 16) {
					cur_live.crc = 0xcdb4;
					live_write_mfm(command[0] & 0x08 ? 0xf8 : 0xfb);
				} else if(cur_live.byte_counter < 16+sector_size)
					live_write_mfm(tc_done && !fifo_pos? 0x00 : fifo_pop(true));
				else if(cur_live.byte_counter < 16+sector_size+2)
					live_write_mfm(cur_live.crc >> 8);
				else if(cur_live.byte_counter < 16+sector_size+2+command[7])
					live_write_mfm(0x4e);
				else {
					cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
					cur_live.state = IDLE;
					return;
				}

			} else {
				if(cur_live.byte_counter < 6)
					live_write_fm(0x00);
				else if(cur_live.byte_counter < 7) {
					cur_live.crc = 0xffff;
					live_write_raw(command[0] & 0x08 ? 0xf56a : 0xf56f);
				} else if(cur_live.byte_counter < 7+sector_size)
					live_write_fm(tc_done && !fifo_pos? 0x00 : fifo_pop(true));
				else if(cur_live.byte_counter < 7+sector_size+2)
					live_write_fm(cur_live.crc >> 8);
				else if(cur_live.byte_counter < 7+sector_size+2+command[7])
					live_write_fm(0xff);
				else {
					cur_live.pll.stop_writing(cur_live.fi->dev, cur_live.tm);
					cur_live.state = IDLE;
					return;
				}
			}
			cur_live.state = WRITE_SECTOR_DATA_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_TRACK_PRE_SECTORS:
			if(!cur_live.byte_counter && command[3])
				fifo_expect(4, true);
			if(mfm) {
				if(cur_live.byte_counter < 80)
					live_write_mfm(0x4e);
				else if(cur_live.byte_counter < 92)
					live_write_mfm(0x00);
				else if(cur_live.byte_counter < 95)
					live_write_raw(0x5224);
				else if(cur_live.byte_counter < 96)
					live_write_mfm(0xfc);
				else if(cur_live.byte_counter < 146)
					live_write_mfm(0x4e);
				else {
					cur_live.state = WRITE_TRACK_SECTOR;
					cur_live.byte_counter = 0;
					break;
				}
			} else {
				if(cur_live.byte_counter < 40)
					live_write_fm(0xff);
				else if(cur_live.byte_counter < 46)
					live_write_fm(0x00);
				else if(cur_live.byte_counter < 47)
					live_write_raw(0xf77a);
				else if(cur_live.byte_counter < 73)
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
			if(!cur_live.byte_counter) {
				command[3]--;
				if(command[3])
					fifo_expect(4, true);
			}
			if(mfm) {
				if(cur_live.byte_counter < 12)
					live_write_mfm(0x00);
				else if(cur_live.byte_counter < 15)
					live_write_raw(0x4489);
				else if(cur_live.byte_counter < 16) {
					cur_live.crc = 0xcdb4;
					live_write_mfm(0xfe);
				} else if(cur_live.byte_counter < 20) {
					uint8_t byte = fifo_pop(true);
					command[12+cur_live.byte_counter-16] = byte;
					live_write_mfm(byte);
					if(cur_live.byte_counter == 19)
						LOGFORMAT("formatting sector %02x %02x %02x %02x\n",
									command[12], command[13], command[14], command[15]);
				} else if(cur_live.byte_counter < 22)
					live_write_mfm(cur_live.crc >> 8);
				else if(cur_live.byte_counter < 44)
					live_write_mfm(0x4e);
				else if(cur_live.byte_counter < 56)
					live_write_mfm(0x00);
				else if(cur_live.byte_counter < 59)
					live_write_raw(0x4489);
				else if(cur_live.byte_counter < 60) {
					cur_live.crc = 0xcdb4;
					live_write_mfm(0xfb);
				} else if(cur_live.byte_counter < 60+sector_size)
					live_write_mfm(command[5]);
				else if(cur_live.byte_counter < 62+sector_size)
					live_write_mfm(cur_live.crc >> 8);
				else if(cur_live.byte_counter < 62+sector_size+command[4])
					live_write_mfm(0x4e);
				else {
					cur_live.byte_counter = 0;
					cur_live.state = command[3] ? WRITE_TRACK_SECTOR : WRITE_TRACK_POST_SECTORS;
					break;
				}

			} else {
				if(cur_live.byte_counter < 6)
					live_write_fm(0x00);
				else if(cur_live.byte_counter < 7) {
					cur_live.crc = 0xffff;
					live_write_raw(0xf57e);
				} else if(cur_live.byte_counter < 11) {
					uint8_t byte = fifo_pop(true);
					command[12+cur_live.byte_counter-7] = byte;
					live_write_fm(byte);
					if(cur_live.byte_counter == 10)
						LOGFORMAT("formatting sector %02x %02x %02x %02x\n",
									command[12], command[13], command[14], command[15]);
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
					live_write_fm(command[5]);
				else if(cur_live.byte_counter < 33+sector_size)
					live_write_fm(cur_live.crc >> 8);
				else if(cur_live.byte_counter < 33+sector_size+command[4])
					live_write_fm(0xff);
				else {
					cur_live.byte_counter = 0;
					cur_live.state = command[3] ? WRITE_TRACK_SECTOR : WRITE_TRACK_POST_SECTORS;
					break;
				}
			}
			cur_live.state = WRITE_TRACK_SECTOR_BYTE;
			cur_live.bit_counter = 16;
			checkpoint();
			break;

		case WRITE_TRACK_POST_SECTORS:
			if(mfm)
				live_write_mfm(0x4e);
			else
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
			LOGWARN("%s: Unknown live state %d\n", cur_live.tm.to_string(), cur_live.state);
			return;
		}
	}
}

int upd765_family_device::check_command()
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
	// 00101110 save
	// 01001110 restore
	// 10001110 drive specification command
	// 00001111 seek
	// 1.001111 relative seek
	// 00010000 version
	// ...10001 scan equal
	// 00010010 perpendicular mode
	// 00010011 configure
	// 00110011 option
	// .0010100 lock
	// ...10110 verify
	// 00010111 powerdown mode
	// 00011000 part id
	// ...11001 scan low or equal
	// ...11101 scan high or equal

	// MSDOS 6.22 format uses 0xcd to format a track, which makes one
	// think only the bottom 5 bits are decoded.
	// The real 765 completely decodes the commands that don't have
	// variable upper bits.  Some later superio chips don't.
	// The MCS Powerview depends on this.

	switch(command[0]) {
	case 0x03:
		return command_pos == 3 ? C_SPECIFY            : C_INCOMPLETE;

	case 0x04:
		return command_pos == 2 ? C_SENSE_DRIVE_STATUS : C_INCOMPLETE;

	case 0x07:
		return command_pos == 2 ? C_RECALIBRATE        : C_INCOMPLETE;

	case 0x08:
		return C_SENSE_INTERRUPT_STATUS;

	case 0x0f:
		return command_pos == 3 ? C_SEEK               : C_INCOMPLETE;
	}

	switch(command[0] & 0x1f) {
	case 0x02:
		return command_pos == 9 ? C_READ_TRACK         : C_INCOMPLETE;

	case 0x05:
	case 0x09:
		return command_pos == 9 ? C_WRITE_DATA         : C_INCOMPLETE;

	case 0x06:
	case 0x0c:
		return command_pos == 9 ? C_READ_DATA          : C_INCOMPLETE;

	case 0x0a:
		return command_pos == 2 ? C_READ_ID            : C_INCOMPLETE;

	case 0x0d:
		return command_pos == 6 ? C_FORMAT_TRACK       : C_INCOMPLETE;

	case 0x11:
		return command_pos == 9 ? C_SCAN_EQUAL         : C_INCOMPLETE;

	case 0x19:
		return command_pos == 9 ? C_SCAN_LOW           : C_INCOMPLETE;

	case 0x1d:
		return command_pos == 9 ? C_SCAN_HIGH          : C_INCOMPLETE;

	default:
		return C_INVALID;
	}
}

int ps2_fdc_device::check_command()
{
	switch(command[0] & 0x1f) {
	case 0x0e:
		return C_DUMP_REG;

	case 0x10:
		return C_VERSION;

	case 0x12:
		return command_pos == 2 ? C_PERPENDICULAR      : C_INCOMPLETE;

	case 0x13:
		return command_pos == 4 ? C_CONFIGURE          : C_INCOMPLETE;

	case 0x14:
		return C_LOCK;

	default:
		return upd765_family_device::check_command();
	}
}

void upd765_family_device::start_command(int cmd)
{
	command_pos = 0;
	result_pos = 0;
	main_phase = PHASE_EXEC;
	tc_done = false;

	execute_command(cmd);
}

void upd765_family_device::execute_command(int cmd)
{
	switch(cmd) {
	case C_FORMAT_TRACK:
		format_track_start(flopi[command[1] & 3]);
		break;

	case C_READ_DATA:
		read_data_start(flopi[command[1] & 3]);
		break;

	case C_READ_ID:
		read_id_start(flopi[command[1] & 3]);
		break;

	case C_READ_TRACK:
		read_track_start(flopi[command[1] & 3]);
		break;

	case C_SCAN_EQUAL:
	case C_SCAN_LOW:
	case C_SCAN_HIGH:
		scan_start(flopi[command[1] & 3]);
		break;

	case C_RECALIBRATE:
		recalibrate_start(flopi[command[1] & 3]);
		main_phase = PHASE_CMD;
		break;

	case C_SEEK:
		seek_start(flopi[command[1] & 3]);
		main_phase = PHASE_CMD;
		break;

	case C_SENSE_DRIVE_STATUS: {
		floppy_info &fi = flopi[command[1] & 3];
		main_phase = PHASE_RESULT;
		result[0] = get_st3(fi);
		LOGCOMMAND("command sense drive status %d (%02x)\n", fi.id, result[0]);
		result_pos = 1;
		break;
	}

	case C_SENSE_INTERRUPT_STATUS: {
		// Documentation is somewhat contradictory w.r.t polling
		// and irq.  PC bios, especially 5150, requires that only
		// one irq happens.  That's also what the ns82077a doc
		// says it does.  OTOH, a number of docs says you need to
		// call SIS 4 times, once per drive...
		//
		// There's also the interaction with the seek irq.  The
		// somewhat borderline tf20 code seems to think that
		// essentially ignoring the polling irq should work.
		//
		// And the pc98 expects to be able to accumulate irq reasons
		// for different drives and things to work.
		//
		// Current hypothesis:
		// - each drive has its own st0 and irq trigger
		// - SIS drops the irq always, but also returns the first full st0 it finds

		main_phase = PHASE_RESULT;

		int fid;
		for(fid=0; fid<4 && !flopi[fid].st0_filled; fid++) {};
		if(fid == 4) {
			result[0] = ST0_UNK;
			result_pos = 1;
			LOGCOMMAND("command sense interrupt status (%02x) (%s)\n", result[0], machine().describe_context());
			break;
		}

		floppy_info &fi = flopi[fid];
		fi.st0_filled = false;

		result[0] = fi.st0;
		result[1] = fi.pcn;

		LOGCOMMAND("command sense interrupt status (fid=%d %02x %02x) (%s)\n", fid, result[0], result[1], machine().describe_context());
		result_pos = 2;

		break;
	}

	case C_SPECIFY:
		spec = (command[1] << 8) | command[2];
		LOGCOMMAND("command specify %02x %02x: step_rate=%d ms, head_unload=%d ms, head_load=%d ms, non_dma=%s\n",
			command[1], command[2], 16-(command[1]>>4), (command[1]&0x0f)<<4, command[2]&0xfe, ((command[2]&1)==1)? "true":"false");
		main_phase = PHASE_CMD;
		break;

	case C_WRITE_DATA:
		write_data_start(flopi[command[1] & 3]);
		break;

	default:
		LOGWARN("Unknown command %02x\n", cmd);
		// exit(1);
	}
}

void ps2_fdc_device::execute_command(int cmd)
{
	switch(cmd) {
	case C_CONFIGURE:
		LOGCOMMAND("command configure %02x %02x %02x\n",
					command[1], command[2], command[3]);
		// byte 1 is ignored, byte 3 is precompensation-related
		fifocfg = command[2];
		precomp = command[3];
		main_phase = PHASE_CMD;
		break;

	case C_DUMP_REG:
		LOGCOMMAND("command dump regs\n");
		main_phase = PHASE_RESULT;
		result[0] = flopi[0].pcn;
		result[1] = flopi[1].pcn;
		result[2] = flopi[2].pcn;
		result[3] = flopi[3].pcn;
		result[4] = (spec & 0xff00) >> 8;
		result[5] = (spec & 0x00ff);
		result[6] = sector_size;
		result[7] = locked ? 0x80 : 0x00;
		result[7] |= (perpmode & 0x3f);
		result[8] = fifocfg;
		result[9] = precomp;
		result_pos = 10;
		break;

	case C_VERSION:
		LOGCOMMAND("command version\n");
		main_phase = PHASE_RESULT;
		result[0] = 0x90;
		result_pos = 1;
		break;

	case C_LOCK:
		locked = command[0] & 0x80;
		main_phase = PHASE_RESULT;
		result[0] = locked ? 0x10 : 0x00;
		result_pos = 1;
		LOGCOMMAND("command lock (%s)\n", locked ? "on" : "off");
		break;

	case C_PERPENDICULAR:
		LOGCOMMAND("command perpendicular\n");
		if(BIT(command[1], 7))
			perpmode = command[1] & 0x3f;
		else
			perpmode = (perpmode & 0x3c) | (command[1] & 3);
		main_phase = PHASE_CMD;
		break;

	default:
		upd765_family_device::execute_command(cmd);
		break;
	}
}

void upd765_family_device::command_end(floppy_info &fi, bool data_completion)
{
	LOGDONE("command done (%s) - %s\n", data_completion ? "data" : "seek", results());
	fi.main_state = fi.sub_state = IDLE;
	irq = true;
	if(!data_completion) {
		fi.st0_filled = true;
		drive_busy |= (1 << fi.id);
	}
	check_irq();
}

uint8_t upd765_family_device::get_st3(floppy_info &fi)
{
	uint8_t st3 = command[1] & 7;
	if(fi.ready)
		st3 |= ST3_RY;
	if(fi.dev)
		st3 |=
			(fi.dev->wpt_r() ? ST3_WP : 0x00) |
			(fi.dev->trk00_r() ? 0x00 : ST3_T0) |
			(fi.dev->twosid_r() ? 0x00 : ST3_TS);
	return st3;
}

void upd765_family_device::recalibrate_start(floppy_info &fi)
{
	LOGCOMMAND("command recalibrate %d\n", command[1] & 3);
	fi.main_state = RECALIBRATE;
	fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
	fi.dir = 1;
	fi.counter = recalibrate_steps;
	fi.ready = get_ready(command[1] & 3);
	fi.st0 = (fi.ready ? 0 : ST0_NR);
	seek_continue(fi);
}

void upd765_family_device::seek_start(floppy_info &fi)
{
	LOGCOMMAND("command %sseek %d\n", command[0] & 0x80 ? "relative " : "", command[2]);
	fi.main_state = SEEK;
	fi.sub_state = SEEK_WAIT_STEP_TIME_DONE;
	fi.dir = fi.pcn > command[2] ? 1 : 0;
	fi.ready = get_ready(command[1] & 3);
	fi.st0 = (fi.ready ? 0 : ST0_NR);
	seek_continue(fi);
}

void upd765_family_device::delay_cycles(floppy_info &fi, int cycles)
{
	fi.tm->adjust(attotime::from_double(double(cycles)/cur_rate), fi.id);
}

void upd765_family_device::seek_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case SEEK_MOVE:
			LOGSTATE("SEEK_MOVE\n");
			if(fi.dev) {
				fi.dev->dir_w(fi.dir);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500), fi.id);
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			LOGSTATE("SEEK_WAIT_STEP_SIGNAL_TIME\n");
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			LOGSTATE("SEEK_WAIT_STEP_SIGNAL_TIME_DONE\n");
			if(fi.dev)
				fi.dev->stp_w(1);

			if(fi.main_state == SEEK) {
				if(fi.pcn > command[2])
					fi.pcn--;
				else
					fi.pcn++;
			}
			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi, 500*(16-(spec >> 12)));
			return;

		case SEEK_WAIT_STEP_TIME:
			LOGSTATE("SEEK_WAIT_STEP_TIME\n");
			return;

		case SEEK_WAIT_STEP_TIME_DONE: {
			LOGSTATE("SEEK_WAIT_STEP_TIME_DONE\n");
			bool done = false;
			switch(fi.main_state) {
			case RECALIBRATE:
				LOGSTATE("RECALIBRATE\n");
				fi.counter--;
				done = fi.dev && !fi.dev->trk00_r();
				if(done)
					fi.pcn = 0;
				else if(!fi.counter) {
					fi.st0 |= ST0_FAIL|ST0_SE|ST0_EC | fi.id;
					command_end(fi, false);
					return;
				}
				break;
			case SEEK:
				LOGSTATE("SEEK\n");
				done = fi.pcn == command[2];
				break;
			}
			if(done) {
				fi.sub_state = SEEK_WAIT_DONE;
				// recalibrate and seek takes some time, even if we don't move
				fi.tm->adjust(attotime::from_nsec((fi.main_state == RECALIBRATE) ? 20000 : 10000), fi.id);
				return;
			}
			fi.sub_state = SEEK_MOVE;
			break;
		}

		case SEEK_WAIT_DONE:
			LOGSTATE("SEEK_WAIT_DONE\n");
			fi.st0 |= ST0_SE | fi.id;
			command_end(fi, false);
			return;

		}
	}
}

void upd765_family_device::read_data_start(floppy_info &fi)
{
	fi.main_state = READ_DATA;
	fi.sub_state = HEAD_LOAD;
	mfm = command[0] & 0x40;

	LOGCOMMAND("command read%s data%s%s%s%s cmd=%02x sel=%x chrn=(%d, %d, %d, %d) eot=%02x gpl=%02x dtl=%02x rate=%d\n",
				command[0] & 0x08 ? " deleted" : "",
				command[0] & 0x80 ? " mt" : "",
				command[0] & 0x40 ? " mfm" : "",
				command[0] & 0x20 ? " sk" : "",
				fifocfg & 0x40 ? " seek" : "",
				command[0],
				command[1],
				command[2],
				command[3],
				command[4],
				128 << (command[5] & 7),
				command[6],
				command[7],
				command[8],
				cur_rate);

	fi.st0 = command[1] & 7;
	st1 = ST1_MA;
	st2 = 0x00;
	hdl_cb(1);
	set_ds(command[1] & 3);
	fi.ready = get_ready(command[1] & 3);

	if(!fi.ready) {
		fi.st0 |= ST0_NR | ST0_FAIL;
		fi.sub_state = COMMAND_DONE;
		st1 = 0;
		st2 = 0;
		read_data_continue(fi);
		return;
	}

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);
	read_data_continue(fi);
}

void upd765_family_device::scan_start(floppy_info &fi)
{
	fi.main_state = SCAN_DATA;
	fi.sub_state = HEAD_LOAD;
	mfm = command[0] & 0x40;

	LOGCOMMAND("command scan%s data%s%s%s%s cmd=%02x sel=%x chrn=(%d, %d, %d, %d) eot=%02x gpl=%02x stp=%02x rate=%d\n",
				command[0] & 0x08 ? " deleted" : "",
				command[0] & 0x80 ? " mt" : "",
				command[0] & 0x40 ? " mfm" : "",
				command[0] & 0x20 ? " sk" : "",
				fifocfg & 0x40 ? " seek" : "",
				command[0],
				command[1],
				command[2],
				command[3],
				command[4],
				128 << (command[5] & 7),
				command[6],
				command[7],
				command[8],
				cur_rate);

	fi.st0 = command[1] & 7;
	st1 = ST1_MA;
	st2 = 0x00;
	scan_done = false;
	hdl_cb(1);
	set_ds(command[1] & 3);
	fi.ready = get_ready(command[1] & 3);

	if(!fi.ready) {
		fi.st0 |= ST0_NR | ST0_FAIL;
		fi.sub_state = COMMAND_DONE;
		st1 = 0;
		st2 = 0;
		read_data_continue(fi);
		return;
	}

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);
	read_data_continue(fi);
}

void upd765_family_device::read_data_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD:
			LOGSTATE("HEAD_LOAD\n");
			delay_cycles(fi, 500*(spec & 0x00fe));
			fi.sub_state = HEAD_LOAD_DONE;
			break;
		case HEAD_LOAD_DONE:
			LOGSTATE("HEAD_LOAD_DONE\n");
			if(fi.pcn == command[2] || !(fifocfg & FIF_EIS)) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			fi.st0 |= ST0_SE;
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[2] ? 1 : 0);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500), fi.id);
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			LOGSTATE("SEEK_WAIT_STEP_SIGNAL_TIME\n");
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			LOGSTATE("SEEK_WAIT_STEP_SIGNAL_TIME_DONE\n");
			if(fi.dev)
				fi.dev->stp_w(1);

			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi, 500*(16-(spec >> 12)));
			return;

		case SEEK_WAIT_STEP_TIME:
			LOGSTATE("SEEK_WAIT_STEP_TIME\n");
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			LOGSTATE("SEEK_WAIT_STEP_TIME_DONE\n");
			if(fi.pcn > command[2])
				fi.pcn--;
			else
				fi.pcn++;
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			LOGSTATE("SEEK_DONE\n");
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			LOGSTATE("SEARCH_ADDRESS_MARK_HEADER\n");
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			// MZ: This st1 handling ensures that both HX5102 floppy and the
			// Speedlock protection scheme are properly working.
			// a) HX5102 requires that the ND flag not be set when no address
			// marks could be found on the track at all (particularly due to
			// wrong density)
			// b) Speedlock requires the ND flag be set when there are valid
			// sectors on the track, but the desired sector is missing, also
			// when it has no valid address marks
			st1 &= ~ST1_MA;
			st1 |= ST1_ND;
			if(!sector_matches()) {
				if(cur_live.idbuf[0] != command[2]) {
					if(cur_live.idbuf[0] == 0xff)
						st2 |= ST2_WC|ST2_BC;
					else
						st2 |= ST2_WC;
				}
				LOGSTATE("SEARCH_ADDRESS_MARK_HEADER\n");
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				fi.st0 |= ST0_FAIL;
				st1 |= ST1_DE;
				st1 &= ~ST1_ND;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			st1 &= ~ST1_ND;
			st2 &= ~ST2_WC;
			LOGRW("reading sector %02x %02x %02x %02x\n",
						cur_live.idbuf[0],
						cur_live.idbuf[1],
						cur_live.idbuf[2],
						cur_live.idbuf[3]);
			sector_size = calc_sector_size(cur_live.idbuf[3]);
			if(fi.main_state == SCAN_DATA)
				fifo_expect(sector_size, true);
			else
				fifo_expect(sector_size, false);
			fi.sub_state = SECTOR_READ;
			LOGSTATE("SEARCH_ADDRESS_MARK_DATA\n");
			live_start(fi, SEARCH_ADDRESS_MARK_DATA);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			fi.st0 |= ST0_FAIL;
			fi.sub_state = COMMAND_DONE;
			break;

		case SECTOR_READ: {
			LOGSTATE("SECTOR_READ\n");
			if(st2 & ST2_MD) {
				fi.st0 |= ST0_FAIL;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			if(cur_live.crc) {
				fi.st0 |= ST0_FAIL;
				st1 |= ST1_DE;
				st2 |= ST2_DD;
				fi.sub_state = COMMAND_DONE;
				break;
			}

			if ((st2 & ST2_CM) && !(command[0] & 0x20)) {
				// Encountered terminating sector while in non-skip mode.
				// This will stop reading when a normal data sector is encountered during read deleted data,
				// or when a deleted sector is encountered during a read data command.
				fi.sub_state = COMMAND_DONE;
				break;
			}

			bool done = tc_done;
			if(command[4] == command[6]) {
				if(command[0] & 0x80) {
					command[3] = command[3] ^ 1;
					command[4] = 1;
					if(fi.dev)
						fi.dev->ss_w(command[3] & 1);
				}
				if(!(command[0] & 0x80) || !(command[3] & 1)) {
					if(!tc_done) {
						fi.st0 |= ST0_FAIL;
						st1 |= ST1_EN;
					} else {
						command[2]++;
						command[4] = 1;
					}
					done = true;
				}
			} else
				command[4]++;
			if(!done) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			fi.sub_state = COMMAND_DONE;
			break;
		}

		case COMMAND_DONE:
			LOGSTATE("COMMAND_DONE\n");
			main_phase = PHASE_RESULT;
			result[0] = fi.st0;
			result[1] = st1;
			result[2] = st2;
			result[3] = command[2];
			result[4] = command[3];
			result[5] = command[4];
			result[6] = command[5];
			result_pos = 7;
			command_end(fi, true);
			return;

		default:
			LOGWARN("%s: read sector unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

void upd765_family_device::write_data_start(floppy_info &fi)
{
	fi.main_state = WRITE_DATA;
	fi.sub_state = HEAD_LOAD;
	mfm = command[0] & 0x40;
	LOGCOMMAND("command write%s data%s%s cmd=%02x sel=%x chrn=(%d, %d, %d, %d) eot=%02x gpl=%02x dtl=%02x rate=%d\n",
				command[0] & 0x08 ? " deleted" : "",
				command[0] & 0x80 ? " mt" : "",
				command[0] & 0x40 ? " mfm" : "",
				command[0],
				command[1],
				command[2],
				command[3],
				command[4],
				128 << (command[5] & 7),
				command[6],
				command[7],
				command[8],
				cur_rate);

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);

	fi.st0 = command[1] & 7;
	st1 = ST1_MA;
	st2 = 0x00;
	hdl_cb(1);
	set_ds(command[1] & 3);
	fi.ready = get_ready(command[1] & 3);

	if(!fi.ready) {
		fi.st0 |= ST0_NR | ST0_FAIL;
		fi.sub_state = COMMAND_DONE;
		st1 = 0;
		st2 = 0;
		write_data_continue(fi);
		return;
	}
	else if(fi.dev && fi.dev->wpt_r()) {
		fi.st0 |= ST0_FAIL;
		fi.sub_state = COMMAND_DONE;
		st1 = ST1_NW;
		st2 = 0;
		write_data_continue(fi);
		return;
	}

	write_data_continue(fi);
}

void upd765_family_device::write_data_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD:
			LOGSTATE("HEAD_LOAD\n");
			delay_cycles(fi, 500*(spec & 0x00fe));
			fi.sub_state = HEAD_LOAD_DONE;
			break;
		case HEAD_LOAD_DONE:
			LOGSTATE("HEAD_LOAD_DONE\n");
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			LOGSTATE("SEARCH_ADDRESS_MARK_HEADER\n");
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			if(!sector_matches()) {
				LOGSTATE("SEARCH_ADDRESS_MARK_HEADER\n");
				live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
				return;
			}
			if(cur_live.crc) {
				fi.st0 |= ST0_FAIL;
				st1 |= ST1_DE|ST1_ND;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			st1 &= ~ST1_MA;
			LOGRW("writing sector %02x %02x %02x %02x\n",
						cur_live.idbuf[0],
						cur_live.idbuf[1],
						cur_live.idbuf[2],
						cur_live.idbuf[3]);
			sector_size = calc_sector_size(cur_live.idbuf[3]);
			fifo_expect(sector_size, true);
			fi.sub_state = SECTOR_WRITTEN;
			LOGSTATE("WRITE_SECTOR_SKIP_GAP2\n");
			live_start(fi, WRITE_SECTOR_SKIP_GAP2);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			fi.st0 |= ST0_FAIL;
			// st1 |= ST1_ND;
			fi.sub_state = COMMAND_DONE;
			break;

		case SECTOR_WRITTEN: {
			LOGSTATE("SECTOR_WRITTEN\n");
			bool done = tc_done;
			if(command[4] == command[6]) {
				if(command[0] & 0x80) {
					command[3] = command[3] ^ 1;
					command[4] = 1;
					if(fi.dev)
						fi.dev->ss_w(command[3] & 1);
				}
				if(!(command[0] & 0x80) || !(command[3] & 1)) {
					if(!tc_done) {
						fi.st0 |= ST0_FAIL;
						st1 |= ST1_EN;
					} else {
						command[2]++;
						command[4] = 1;
					}
					done = true;
				}
			} else
				command[4]++;
			if(!done) {
				fi.sub_state = HEAD_LOAD_DONE;
				break;
			}
			fi.sub_state = COMMAND_DONE;
			break;
		}

		case COMMAND_DONE:
			LOGSTATE("COMMAND_DONE\n");
			main_phase = PHASE_RESULT;
			result[0] = fi.st0;
			result[1] = st1;
			result[2] = st2;
			result[3] = command[2];
			result[4] = command[3];
			result[5] = command[4];
			result[6] = command[5];
			result_pos = 7;
			command_end(fi, true);
			return;

		default:
			LOGWARN("%s: write sector unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

void upd765_family_device::read_track_start(floppy_info &fi)
{
	fi.main_state = READ_TRACK;
	fi.sub_state = HEAD_LOAD;
	mfm = command[0] & 0x40;
	sectors_read = 0;

	LOGCOMMAND("command read track%s cmd=%02x sel=%x chrn=(%d, %d, %d, %d) eot=%02x gpl=%02x dtl=%02x rate=%d\n",
				command[0] & 0x40 ? " mfm" : "",
				command[0],
				command[1],
				command[2],
				command[3],
				command[4],
				128 << (command[5] & 7),
				command[6],
				command[7],
				command[8],
				cur_rate);
	fi.st0 = command[1] & 7;
	st1 = ST1_MA;
	st2 = 0x00;
	hdl_cb(1);
	set_ds(command[1] & 3);
	fi.ready = get_ready(command[1] & 3);

	if(!fi.ready) {
		fi.st0 |= ST0_NR | ST0_FAIL;
		fi.sub_state = COMMAND_DONE;
		st1 = 0;
		st2 = 0;
		read_track_continue(fi);
		return;
	}

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);
	read_track_continue(fi);
}

void upd765_family_device::read_track_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD:
			LOGSTATE("HEAD_LOAD\n");
			delay_cycles(fi, 500*(spec & 0x00fe));
			fi.sub_state = HEAD_LOAD_DONE;
			break;
		case HEAD_LOAD_DONE:
			LOGSTATE("HEAD_LOAD_DONE\n");
			if(fi.pcn == command[2] || !(fifocfg & FIF_EIS)) {
				fi.sub_state = SEEK_DONE;
				break;
			}
			fi.st0 |= ST0_SE;
			if(fi.dev) {
				fi.dev->dir_w(fi.pcn > command[2] ? 1 : 0);
				fi.dev->stp_w(0);
			}
			fi.sub_state = SEEK_WAIT_STEP_SIGNAL_TIME;
			fi.tm->adjust(attotime::from_nsec(2500), fi.id);
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME:
			LOGSTATE("SEEK_WAIT_STEP_SIGNAL_TIME\n");
			return;

		case SEEK_WAIT_STEP_SIGNAL_TIME_DONE:
			LOGSTATE("SEEK_WAIT_STEP_SIGNAL_TIME_DONE\n");
			if(fi.dev)
				fi.dev->stp_w(1);

			fi.sub_state = SEEK_WAIT_STEP_TIME;
			delay_cycles(fi, 500*(16-(spec >> 12)));
			return;

		case SEEK_WAIT_STEP_TIME:
			LOGSTATE("SEEK_WAIT_STEP_TIME\n");
			return;

		case SEEK_WAIT_STEP_TIME_DONE:
			LOGSTATE("SEEK_WAIT_STEP_TIME_DONE\n");
			if(fi.pcn > command[2])
				fi.pcn--;
			else
				fi.pcn++;
			fi.sub_state = HEAD_LOAD_DONE;
			break;

		case SEEK_DONE:
			LOGSTATE("SEEK_DONE\n");
			fi.counter = 0;
			fi.sub_state = WAIT_INDEX;
			return;

		case WAIT_INDEX:
			LOGSTATE("WAIT_INDEX\n");
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("WAIT_INDEX_DONE\n");
			fi.sub_state = SCAN_ID;
			LOGSTATE("SEARCH_ADDRESS_MARK_HEADER\n");
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			if(cur_live.crc) {
				st1 |= ST1_DE;
			}
			st1 &= ~ST1_MA;
			LOGRW("reading sector %02x %02x %02x %02x\n",
						cur_live.idbuf[0],
						cur_live.idbuf[1],
						cur_live.idbuf[2],
						cur_live.idbuf[3]);
			if(!sector_matches())
				st1 |= ST1_ND;
			else
				st1 &= ~ST1_ND;

			sector_size = calc_sector_size(command[5]);
			fifo_expect(sector_size, false);
			fi.sub_state = SECTOR_READ;
			LOGSTATE("SEARCH_ADDRESS_MARK_DATA\n");
			live_start(fi, SEARCH_ADDRESS_MARK_DATA);
			return;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			fi.st0 |= ST0_FAIL;
			// st1 |= ST1_ND;
			fi.sub_state = COMMAND_DONE;
			break;

		case SECTOR_READ: {
			LOGSTATE("SECTOR_READ\n");
			if(st2 & ST2_MD) {
				fi.st0 |= ST0_FAIL;
				fi.sub_state = COMMAND_DONE;
				break;
			}
			if(cur_live.crc) {
				st1 |= ST1_DE;
				st2 |= ST2_DD;
			}
			bool done = tc_done;
			sectors_read++;
			if(sectors_read == command[6]) {
				if(!tc_done) {
					fi.st0 |= ST0_FAIL;
					st1 |= ST1_EN;
				}
				done = true;
			}
			if(!done) {
				fi.sub_state = WAIT_INDEX_DONE;
				break;
			}
			fi.sub_state = COMMAND_DONE;
			break;
		}

		case COMMAND_DONE:
			LOGSTATE("COMMAND_DONE\n");
			main_phase = PHASE_RESULT;
			result[0] = fi.st0;
			result[1] = st1;
			result[2] = st2;
			result[3] = command[2];
			result[4] = command[3];
			result[5] = command[4];
			result[6] = command[5];
			result_pos = 7;
			command_end(fi, true);
			return;

		default:
			LOGWARN("%s: read track unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

int upd765_family_device::calc_sector_size(uint8_t size)
{
	return size > 7 ? 16384 : 128 << size;
}

void upd765_family_device::format_track_start(floppy_info &fi)
{
	fi.main_state = FORMAT_TRACK;
	fi.sub_state = HEAD_LOAD;
	mfm = command[0] & 0x40;

	LOGCOMMAND("command format track %s h=%02x n=%02x sc=%02x gpl=%02x d=%02x\n",
				command[0] & 0x40 ? "mfm" : "fm",
				command[1], command[2], command[3], command[4], command[5]);

	hdl_cb(1);
	set_ds(command[1] & 3);
	fi.ready = get_ready(command[1] & 3);

	st1 = 0;
	st2 = 0;
	if(!fi.ready) {
		fi.st0 = (command[1] & 7) | ST0_NR | ST0_FAIL;
		fi.sub_state = TRACK_DONE;
		format_track_continue(fi);
		return;
	}
	else if(fi.dev && fi.dev->wpt_r()) {
		fi.st0 = (command[1] & 7) | ST0_FAIL;
		fi.sub_state = TRACK_DONE;
		st1 = ST1_NW;
		format_track_continue(fi);
		return;
	}
	fi.st0 = command[1] & 7;

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);
	sector_size = calc_sector_size(command[2]);

	format_track_continue(fi);
}

void upd765_family_device::format_track_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD:
			LOGSTATE("HEAD_LOAD\n");
			delay_cycles(fi, 500*(spec & 0x00fe));
			fi.sub_state = HEAD_LOAD_DONE;
			break;
		case HEAD_LOAD_DONE:
			LOGSTATE("HEAD_LOAD_DONE\n");
			fi.sub_state = WAIT_INDEX;
			break;

		case WAIT_INDEX:
			LOGSTATE("WAIT_INDEX\n");
			return;

		case WAIT_INDEX_DONE:
			LOGSTATE("WAIT_INDEX_DONE\n");
			fi.sub_state = TRACK_DONE;
			cur_live.pll.start_writing(machine().time());
			LOGSTATE("WRITE_TRACK_PRE_SECTORS\n");
			live_start(fi, WRITE_TRACK_PRE_SECTORS);
			return;

		case TRACK_DONE:
			LOGSTATE("TRACK_DONE\n");
			main_phase = PHASE_RESULT;
			result[0] = fi.st0;
			result[1] = st1;
			result[2] = st2;
			result[3] = 0;
			result[4] = 0;
			result[5] = 0;
			result[6] = command[2];
			result_pos = 7;
			command_end(fi, true);
			return;

		default:
			LOGWARN("%s: format track unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

void upd765_family_device::read_id_start(floppy_info &fi)
{
	fi.main_state = READ_ID;
	fi.sub_state = HEAD_LOAD;
	mfm = command[0] & 0x40;

	LOGCOMMAND("command read id%s %d, rate=%d\n",
				command[0] & 0x40 ? " mfm" : "",
				command[1] & 3,
				cur_rate);

	if(fi.dev)
		fi.dev->ss_w(command[1] & 4 ? 1 : 0);

	fi.st0 = command[1] & 7;
	st1 = 0x00;
	st2 = 0x00;

	for(int i=0; i<4; i++)
		cur_live.idbuf[i] = 0x00;

	hdl_cb(1);
	set_ds(command[1] & 3);
	fi.ready = get_ready(command[1] & 3);

	if(!fi.ready) {
		fi.st0 |= ST0_NR | ST0_FAIL;
		fi.sub_state = COMMAND_DONE;
		read_id_continue(fi);
		return;
	}

	for(int i=0; i<4; i++)
		cur_live.idbuf[i] = command[i+2];

	read_id_continue(fi);
}

void upd765_family_device::read_id_continue(floppy_info &fi)
{
	for(;;) {
		switch(fi.sub_state) {
		case HEAD_LOAD:
			LOGSTATE("HEAD_LOAD\n");
			delay_cycles(fi, 500*(spec & 0x00fe));
			fi.sub_state = HEAD_LOAD_DONE;
			break;
		case HEAD_LOAD_DONE:
			LOGSTATE("HEAD_LOAD_DONE\n");
			fi.counter = 0;
			fi.sub_state = SCAN_ID;
			LOGSTATE("SEARCH_ADDRESS_MARK_HEADER\n");
			live_start(fi, SEARCH_ADDRESS_MARK_HEADER);
			return;

		case SCAN_ID:
			LOGSTATE("SCAN_ID\n");
			if(cur_live.crc) {
				fi.st0 |= ST0_FAIL;
				st1 |= ST1_MA|ST1_DE|ST1_ND;
			}
			fi.sub_state = COMMAND_DONE;
			break;

		case SCAN_ID_FAILED:
			LOGSTATE("SCAN_ID_FAILED\n");
			fi.st0 |= ST0_FAIL;
			st1 |= ST1_ND|ST1_MA;
			fi.sub_state = COMMAND_DONE;
			break;

		case COMMAND_DONE:
			LOGSTATE("COMMAND_DONE\n");
			main_phase = PHASE_RESULT;
			result[0] = fi.st0;
			result[1] = st1;
			result[2] = st2;
			result[3] = cur_live.idbuf[0];
			result[4] = cur_live.idbuf[1];
			result[5] = cur_live.idbuf[2];
			result[6] = cur_live.idbuf[3];
			result_pos = 7;
			command_end(fi, true);
			return;

		default:
			LOGWARN("%s: read id unknown sub-state %d\n", ttsn(), fi.sub_state);
			return;
		}
	}
}

void upd765_family_device::check_irq()
{
	bool old_irq = cur_irq;
	cur_irq = irq || internal_drq;
	cur_irq = cur_irq && (dor & 4) && (mode != mode_t::AT || (dor & 8));
	if(cur_irq != old_irq) {
		LOGTCIRQ("irq = %d\n", cur_irq);
		intrq_cb(cur_irq);
	}
}

bool upd765_family_device::get_irq() const
{
	return cur_irq;
}

std::string upd765_family_device::results() const
{
	std::ostringstream stream;
	stream << "results=(";
	if(!result_pos)
		stream << "none";
	else {
		stream << std::hex << std::setfill('0') << std::setw(2) << unsigned(result[0]);
		for (int i=1; i < result_pos; i++)
			stream << ',' << std::setw(2) << unsigned(result[i]);
	}
	stream << ')';
	return stream.str();
}

std::string upd765_family_device::ttsn() const
{
	return machine().time().to_string();
}

TIMER_CALLBACK_MEMBER(upd765_family_device::update_floppy)
{
	live_sync();

	floppy_info &fi = flopi[param];
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

TIMER_CALLBACK_MEMBER(upd765_family_device::run_drive_ready_polling)
{
	if(main_phase != PHASE_CMD || (fifocfg & FIF_POLL) || command_pos)
		return;

	for(int fid=0; fid<4; fid++) {
		bool ready = get_ready(fid);
		if(ready != flopi[fid].ready) {
			LOGCOMMAND("polled %d : %d -> %d\n", fid, flopi[fid].ready, ready);
			flopi[fid].ready = ready;
			if(!flopi[fid].st0_filled) {
				flopi[fid].st0 = ST0_ABRT | fid;
				flopi[fid].st0_filled = true;
				irq = true;
			}
		}
	}

	check_irq();
}

void upd765_family_device::index_callback(floppy_image_device *floppy, int state)
{
	LOGSTATE("Index pulse %d\n", state);
	LOGLIVE("%s: Pulse %d\n", ttsn(), state);
	for(floppy_info & fi : flopi) {
		if(fi.dev != floppy)
			continue;

		if(fi.live)
			live_sync();
		fi.index = state;
		idx_cb(state);

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
		case HEAD_LOAD:
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
			LOGWARN("%s: Index pulse on unknown sub-state %d\n", ttsn(), fi.sub_state);
			break;
		}

		general_continue(fi);
	}
}


void upd765_family_device::general_continue(floppy_info &fi)
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
		read_data_continue(fi);
		break;

	case WRITE_DATA:
		write_data_continue(fi);
		break;

	case READ_TRACK:
		read_track_continue(fi);
		break;

	case FORMAT_TRACK:
		format_track_continue(fi);
		break;

	case READ_ID:
		read_id_continue(fi);
		break;

	default:
		LOGWARN("%s: general_continue on unknown main-state %d\n", ttsn(), fi.main_state);
		break;
	}
}

bool upd765_family_device::read_one_bit(const attotime &limit)
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

bool upd765_family_device::write_one_bit(const attotime &limit)
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

void upd765_family_device::live_write_raw(uint16_t raw)
{
	LOGLIVE("%s: write %04x %04x\n", cur_live.tm.to_string(), raw, cur_live.crc);
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = raw & 1;
}

void upd765_family_device::live_write_mfm(uint8_t mfm)
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
	cur_live.data_reg = mfm;
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = context;
	LOGLIVE("%s: write %02x   %04x %04x\n", cur_live.tm.to_string(), mfm, cur_live.crc, raw);
}

void upd765_family_device::live_write_fm(uint8_t fm)
{
	uint16_t raw = 0xaaaa;
	for(int i=0; i<8; i++)
		if(fm & (0x80 >> i))
			raw |= 0x4000 >> (2*i);
	cur_live.data_reg = fm;
	cur_live.shift_reg = raw;
	cur_live.data_bit_context = fm & 1;
	LOGLIVE("%s: write %02x   %04x %04x\n", cur_live.tm.to_string(), fm, cur_live.crc, raw);
}

bool upd765_family_device::sector_matches() const
{
	LOGMATCH("matching %02x %02x %02x %02x - %02x %02x %02x %02x\n",
				cur_live.idbuf[0], cur_live.idbuf[1], cur_live.idbuf[2], cur_live.idbuf[3],
				command[2], command[3], command[4], command[5]);
	return
		cur_live.idbuf[0] == command[2] &&
		cur_live.idbuf[1] == command[3] &&
		cur_live.idbuf[2] == command[4] &&
		cur_live.idbuf[3] == command[5];
}

upd765a_device::upd765a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, UPD765A, tag, owner, clock)
{
	has_dor = false;
}

upd765b_device::upd765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, UPD765B, tag, owner, clock)
{
	has_dor = false;
}

i8272a_device::i8272a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, I8272A, tag, owner, clock)
{
	has_dor = false;
}

upd72065_device::upd72065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd72065_device(mconfig, UPD72065, tag, owner, clock)
{
}

upd72065_device::upd72065_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, type, tag, owner, clock)
{
	has_dor = false;
	recalibrate_steps = 255;
}

upd72067_device::upd72067_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd72065_device(mconfig, UPD72067, tag, owner, clock)
{
	ready_polled = true;
	ready_connected = true;
	select_connected = true;
	select_multiplexed = false;
}

upd72069_device::upd72069_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd72065_device(mconfig, UPD72069, tag, owner, clock)
{
	ready_polled = true;
	ready_connected = true;
	select_connected = true;
	select_multiplexed = false;
}

i82072_device::i82072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, I82072, tag, owner, clock)
{
	has_dor = false;
	recalibrate_steps = 255;
}

void i82072_device::device_start()
{
	upd765_family_device::device_start();

	save_item(NAME(motorcfg));
	save_item(NAME(motor_off_counter));
	save_item(NAME(motor_on_counter));
	save_item(NAME(delayed_command));
}

void i82072_device::soft_reset()
{
	motorcfg = 0x60;

	upd765_family_device::soft_reset();
}

int i82072_device::check_command()
{
	// ...00110 read data
	// ...01100 read deleted data
	// ..000101 write data
	// ..001001 write deleted data
	// 0.000010 read track
	// 0.001010 read id
	// 0.001101 format track
	// 00000111 recalibrate
	// 00001000 sense interrupt status
	// 00000011 specify
	// 00000100 sense drive status
	// 00001111 seek
	// 00010011 configure
	// ...01011 motor on/off
	// 1.001111 relative seek
	// 00001110 dumpreg

	switch(command[0] & 0x1f) {
	case 0x02:
		return command_pos == 9 ? C_READ_TRACK : C_INCOMPLETE;

	case 0x03:
		return command_pos == 3 ? C_SPECIFY : C_INCOMPLETE;

	case 0x04:
		return command_pos == 2 ? C_SENSE_DRIVE_STATUS : C_INCOMPLETE;

	case 0x05:
	case 0x09:
		return command_pos == 9 ? C_WRITE_DATA : C_INCOMPLETE;

	case 0x06:
	case 0x0c:
		return command_pos == 9 ? C_READ_DATA : C_INCOMPLETE;

	case 0x07:
		return command_pos == 2 ? C_RECALIBRATE : C_INCOMPLETE;

	case 0x08:
		return C_SENSE_INTERRUPT_STATUS;

	case 0x0a:
		return command_pos == 2 ? C_READ_ID : C_INCOMPLETE;

	case 0x0b:
		return C_MOTOR_ONOFF;

	case 0x0d:
		return command_pos == 6 ? C_FORMAT_TRACK : C_INCOMPLETE;

	case 0x0e:
		return C_DUMP_REG;

	case 0x0f:
		return command_pos == 3 ? C_SEEK : C_INCOMPLETE;

	case 0x13:
		return command_pos == 4 ? C_CONFIGURE : C_INCOMPLETE;

	default:
		return C_INVALID;
	}
}

void i82072_device::start_command(int cmd)
{
	// check if the command specifies a target drive
	switch(cmd) {
	case C_READ_TRACK:
	case C_SENSE_DRIVE_STATUS:
	case C_WRITE_DATA:
	case C_READ_DATA:
	case C_RECALIBRATE:
	//case C_WRITE_DELETED_DATA:
	case C_READ_ID:
	//case C_READ_DELETED_DATA:
	case C_FORMAT_TRACK:
	case C_SEEK:
		// start the motor
		motor_control(command[1] & 0x3, true);
		break;
	}

	// execute the command immediately if there's no motor on delay
	if(motor_on_counter == 0) {
		upd765_family_device::start_command(cmd);

		// set motor off counter if command execution has completed
		if(main_phase != PHASE_EXEC && motorcfg)
			motor_off_counter = (2 + ((motorcfg & MOFF) >> 4)) << (motorcfg & HSDA ? 1 : 0);
	} else
		delayed_command = cmd;
}

void i82072_device::execute_command(int cmd)
{
	switch(cmd) {
	case C_CONFIGURE:
		LOGCOMMAND("command configure %02x %02x %02x\n",
					command[1], command[2], command[3]);
		motorcfg = command[1];
		fifocfg = command[2];
		precomp = command[3];
		main_phase = PHASE_CMD;
		break;

	case C_DUMP_REG:
		LOGCOMMAND("command dump regs\n");
		main_phase = PHASE_RESULT;
		result[0] = flopi[0].pcn;
		result[1] = flopi[1].pcn;
		result[2] = flopi[2].pcn;
		result[3] = flopi[3].pcn;
		result[4] = (spec & 0xff00) >> 8;
		result[5] = (spec & 0x00ff);
		result[6] = sector_size;
		// i82072 dumps motor configuration at offset 7
		result[7] = motorcfg;
		result[8] = fifocfg;
		result[9] = precomp;
		result_pos = 10;
		break;

	case C_MOTOR_ONOFF: {
		bool motor_on = command[0] & 0x80;
		floppy_info &fi = flopi[(command[0] >> 5) & 0x3];

		LOGCOMMAND("command motor %s drive %d\n", motor_on ? "on" : "off", fi.id);

		// if we are selecting a different drive, stop the motor on the previously selected drive
		if(selected_drive != fi.id && flopi[selected_drive].dev && flopi[selected_drive].dev->mon_r() == 0)
			flopi[selected_drive].dev->mon_w(1);

		// select the drive
		if(motor_on)
			set_ds(fi.id);

		// start the motor
		if(fi.dev)
			fi.dev->mon_w(motor_on ? 0 : 1);

		main_phase = PHASE_CMD;
		break;
	}

	default:
		upd765_family_device::execute_command(cmd);
		break;
	}
}

void i82072_device::command_end(floppy_info &fi, bool data_completion)
{
	// set motor off counter
	if(motorcfg)
		motor_off_counter = (2 + ((motorcfg & MOFF) >> 4)) << (motorcfg & HSDA ? 1 : 0);

	// clear existing interrupt sense data
	for(floppy_info &fi : flopi) {
		fi.st0_filled = false;
	}

	upd765_family_device::command_end(fi, data_completion);
}

void i82072_device::motor_control(int fid, bool start_motor)
{
	// check if motor control is enabled
	if(motorcfg == 0)
		return;

	floppy_info &fi = flopi[fid];

	if(start_motor) {
		// if we are selecting a different drive, stop the motor on the previously selected drive
		if(selected_drive != fid && flopi[selected_drive].dev && flopi[selected_drive].dev->mon_r() == 0)
			flopi[selected_drive].dev->mon_w(1);

		// start the motor on the selected drive
		if(fi.dev && fi.dev->mon_r() == 1) {
			LOGCOMMAND("motor_control: switching on motor for drive %d\n", fid);

			// select the drive and enable the motor
			set_ds(fid);
			fi.dev->mon_w(0);

			// set motor on counter
			motor_on_counter = (motorcfg & MON) << (motorcfg & HSDA ? 1 : 0);
		}
	} else {
		// motor off timer only applies to the selected drive
		if(selected_drive != fid)
			return;

		// decrement motor on counter
		if(motor_on_counter)
			motor_on_counter--;

		// execute the command if the motor on counter has expired
		if(motor_on_counter == 0 && main_phase == PHASE_CMD && delayed_command) {
			upd765_family_device::start_command(delayed_command);

			// set motor off counter if command execution has completed
			if(main_phase != PHASE_EXEC && motorcfg)
				motor_off_counter = (2 + ((motorcfg & MOFF) >> 4)) << (motorcfg & HSDA ? 1 : 0);

			delayed_command = 0;

			return;
		}

		// ignore motor off timer while drive is busy
		if(fi.main_state == SEEK || fi.main_state == RECALIBRATE)
			return;

		// check if the motor is already off
		if(motor_off_counter == 0 || (fi.dev && fi.dev->mon_r() == 1))
			return;

		// decrement the counter
		motor_off_counter--;

		// if the motor off timer has expired, stop the motor
		if(motor_off_counter == 0 && fi.dev) {
			LOGCOMMAND("motor_control: switching off motor for drive %d\n", fid);
			fi.dev->mon_w(1);
		}
	}
}

void i82072_device::index_callback(floppy_image_device *floppy, int state)
{
	if(state)
		for(floppy_info &fi : flopi) {
			if(fi.dev != floppy)
				continue;

			// update motor on/off counters and stop motor if necessary
			motor_control(fi.id, false);
		}

	upd765_family_device::index_callback(floppy, state);
}

ps2_fdc_device::ps2_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, type, tag, owner, clock)
{
}

void ps2_fdc_device::device_start()
{
	upd765_family_device::device_start();

	save_item(NAME(perpmode));
}

void ps2_fdc_device::device_reset()
{
	upd765_family_device::device_reset();

	perpmode = 0;
}

void ps2_fdc_device::soft_reset()
{
	upd765_family_device::soft_reset();

	perpmode &= 0x3c;
}

smc37c78_device::smc37c78_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : ps2_fdc_device(mconfig, SMC37C78, tag, owner, clock)
{
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
	recalibrate_steps = 80;
}

n82077aa_device::n82077aa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : ps2_fdc_device(mconfig, N82077AA, tag, owner, clock)
{
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
}

pc_fdc_superio_device::pc_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, PC_FDC_SUPERIO, tag, owner, clock)
{
	ready_polled = false;
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
}

dp8473_device::dp8473_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : upd765_family_device(mconfig, DP8473, tag, owner, clock)
{
	ready_polled = false;
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
	recalibrate_steps = 77; // TODO: 3917 in extended track range mode
}

void dp8473_device::soft_reset()
{
	upd765_family_device::soft_reset();

	// "interrupt is generated when ... Internal Ready signal changes state immediately after a hardware or software reset"
	irq = true;
}

pc8477a_device::pc8477a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : ps2_fdc_device(mconfig, PC8477A, tag, owner, clock)
{
	ready_polled = true;
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
	recalibrate_steps = 85; // TODO: may also be programmed as 255, 3925 or 4095 by (unemulated) mode command
}

pc8477b_device::pc8477b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : ps2_fdc_device(mconfig, PC8477B, tag, owner, clock)
{
	ready_polled = true;
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
	recalibrate_steps = 85; // TODO: may also be programmed as 255, 3925 or 4095 by (unemulated) mode command
}

wd37c65c_device::wd37c65c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	upd765_family_device(mconfig, WD37C65C, tag, owner, clock),
	m_clock2(0)
{
	ready_polled = true;
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;

	(void)m_clock2; // TODO
}

uint8_t wd37c65c_device::get_st3(floppy_info &fi)
{
	uint8_t st3 = command[1] & 7;
	st3 |= 0x20;
	if(fi.dev)
		st3 |=
			(fi.dev->wpt_r() ? 0x48 : 0x00) |
			(fi.dev->trk00_r() ? 0x00 : 0x10);
	return st3;
}

mcs3201_device::mcs3201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	upd765_family_device(mconfig, MCS3201, tag, owner, clock),
	m_input_handler(*this, 0)
{
	has_dor = true;
	ready_polled = false;
	ready_connected = false;
	select_connected = true;
	select_multiplexed = false;
}

uint8_t mcs3201_device::input_r()
{
	return m_input_handler();
}

tc8566af_device::tc8566af_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd765_family_device(mconfig, TC8566AF, tag, owner, clock)
	, m_cr1(0)
{
	ready_polled = true;
	ready_connected = true;
	select_connected = true;
	select_multiplexed = false;
	recalibrate_steps = 255;
}

void tc8566af_device::device_start()
{
	upd765_family_device::device_start();
	save_item(NAME(m_cr1));
}

void tc8566af_device::cr1_w(uint8_t data)
{
	m_cr1 = data;

	if(m_cr1 & 0x02) {
		// Not sure if this inverted or not
		tc_w((m_cr1 & 0x01) ? true : false);
	}
}

void upd72065_device::auxcmd_w(uint8_t data)
{
	switch(data) {
	case 0x36: // reset
		soft_reset();
		break;
	case 0x35: // set standby
		break;
	case 0x34: // reset standby
		break;
	}
}

void upd72067_device::auxcmd_w(uint8_t data)
{
	/*
	 * Auxiliary commands:
	 *
	 *   ???? ????  start clock
	 *   0011 0011  enable external mode
	 *   0x00 1011  control internal mode
	 *   xxxx 1110  enable motors
	 *   010x 1111  select IBM or ECMA/ISO format
	 *
	 * The bare minimum needed to satisfy the news_r3k diagnostic has been
	 * implemented here.
	 */
	switch(data & 0x0f) {
	case 0x0e:
		// motor on/off - no idea about timeout
		for(unsigned i = 0; i < 4; i++)
			if(flopi[i].dev)
				flopi[i].dev->mon_w(!BIT(data, i + 4));
		[[fallthrough]];
	case 0x03: // enable external mode
	case 0x0b: // control internal mode
	case 0x0f: // select format
		// report a successful status
		main_phase = PHASE_RESULT;
		result[0] = ST0_UNK;
		result_pos = 1;
		break;
	default:
		upd72065_device::auxcmd_w(data);
		break;
	}
}

void upd72069_device::auxcmd_w(uint8_t data)
{
	switch(data) {
	case 0x36: // reset
		soft_reset();
		break;
	case 0x1e: // motor on, probably
		for(unsigned i = 0; i < 4; i++)
			if(flopi[i].dev)
				flopi[i].dev->mon_w(!BIT(data, i + 4));
		main_phase = PHASE_RESULT;
		result[0] = ST0_UNK;
		result_pos = 1;
		break;
	}
}


hd63266f_device::hd63266f_device(const machine_config& mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd765_family_device(mconfig, HD63266F, tag, owner, clock)
	, inp_cb(*this, 0)
{
	has_dor = false;
}

void hd63266f_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(hd63266f_device::msr_r), FUNC(hd63266f_device::abort_w));
	map(0x1, 0x1).rw(FUNC(hd63266f_device::fifo_r), FUNC(hd63266f_device::fifo_w));
	map(0x2, 0x2).r(FUNC(hd63266f_device::extstat_r));
}

void hd63266f_device::soft_reset()
{
	upd765_family_device::soft_reset();
	delayed_command = 0;
	motor_state = 0;
	for(int i = 0; i < 4; i++)
		if(flopi[i].dev) flopi[i].dev->mon_w(1);
}

void hd63266f_device::abort_w(u8 data)
{
	if(data == 0xff) {
		soft_reset();
		LOGCOMMAND("abort\n");
	}
}

int hd63266f_device::check_command()
{
	switch(command[0]) {
	case 0x0e:
		return C_SLEEP;
	case 0x0b:
	case 0x2b:
		return command_pos == 4 ? C_SPECIFY : C_INCOMPLETE;
	case 0x4b:
	case 0x6b:
		return command_pos == 7 ? C_SPECIFY2 : C_INCOMPLETE;
	}
	return upd765_family_device::check_command();
}

void hd63266f_device::execute_command(int cmd)
{
	switch(cmd)
	{
	case C_SLEEP:
		for(int i = 0; i < 4; i++) {
			if(flopi[i].dev) flopi[i].dev->mon_w(1);
		}
		main_phase = PHASE_CMD;
		motor_state = 0;
		LOGCOMMAND("sleep\n");
		break;
	case C_SPECIFY2:
		spec = (command[1] << 8) | command[2];
		LOGCOMMAND("command specify2 %02x %02x: step_rate=%d ms, head_unload=%d ms, head_load=%d ms, non_dma=%s\n",
				command[1], command[2], 16-(command[1]>>4), (command[1]&0x0f)<<4, command[2]&0xfe, ((command[2]&1)==1)? "true":"false");
		main_phase = PHASE_CMD;
		break;
	case C_SENSE_DRIVE_STATUS:
		upd765_family_device::execute_command(cmd);
		if(!inp_cb.isunset())
			result[0] = (result[0] & ~ST3_TS) | (inp_cb() ? 0 : 8);
		break;
	default:
		upd765_family_device::execute_command(cmd);
		break;
	}
}

u8 hd63266f_device::extstat_r()
{
	return (irq << 6) | motor_state;
}

// no documentation for motor control so borrow some of 82072
void hd63266f_device::start_command(int cmd)
{
	// check if the command specifies a target drive
	switch(cmd) {
	case C_READ_TRACK:
	case C_WRITE_DATA:
	case C_READ_DATA:
	case C_RECALIBRATE:
	//case C_WRITE_DELETED_DATA:
	case C_READ_ID:
	//case C_READ_DELETED_DATA:
	case C_FORMAT_TRACK:
	case C_SEEK:
		// start the motor
		motor_control(command[1] & 0x3, true);
		break;
	default:
		motor_on_counter = 0;
		break;
	}

	// execute the command immediately if there's no motor on delay
	if(motor_on_counter == 0) {
		upd765_family_device::start_command(cmd);
	} else
		delayed_command = cmd;
}

void hd63266f_device::motor_control(int fid, bool start_motor)
{
	floppy_info &fi = flopi[fid];

	if(start_motor) {
		// if we are selecting a different drive, stop the motor on the previously selected drive
		if(selected_drive != fid && flopi[selected_drive].dev && flopi[selected_drive].dev->mon_r() == 0)
			flopi[selected_drive].dev->mon_w(1);

		// start the motor on the selected drive
		if(fi.dev && fi.dev->mon_r() == 1) {
			LOGCOMMAND("motor_control: switching on motor for drive %d\n", fid);

			// select the drive and enable the motor
			set_ds(fid);
			fi.dev->mon_w(0);
			motor_on_counter = 3;
			motor_state |= 1 << fid;
		}
	} else {
		// motor off timer only applies to the selected drive
		if(selected_drive != fid)
			return;

		// decrement motor on counter
		if(motor_on_counter)
			motor_on_counter--;

		// execute the command if the motor on counter has expired
		if(motor_on_counter == 0 && main_phase == PHASE_CMD && delayed_command) {
			upd765_family_device::start_command(delayed_command);

			delayed_command = 0;

			return;
		}
	}
}

void hd63266f_device::index_callback(floppy_image_device *floppy, int state)
{
	if(state) {
		for(floppy_info &fi : flopi) {
			if(fi.dev != floppy)
				continue;

			// update motor on/off counters and stop motor if necessary
			motor_control(fi.id, false);
		}
	}
	upd765_family_device::index_callback(floppy, state);
}

