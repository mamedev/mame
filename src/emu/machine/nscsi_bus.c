// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "nscsi_bus.h"

const device_type NSCSI_BUS = &device_creator<nscsi_bus_device>;
const device_type NSCSI_CONNECTOR = &device_creator<nscsi_connector>;

nscsi_bus_device::nscsi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NSCSI_BUS, "NSCSI Bus", tag, owner, clock, "nscsi_bus", __FILE__)
{
	devcnt = 0;
	memset(dev, 0, sizeof(dev));
}

void nscsi_bus_device::device_start()
{
	data = 0;
	ctrl = 0;
	save_item(NAME(data));
	save_item(NAME(ctrl));
	for(int i=0; i<devcnt; i++) {
		save_item(NAME(dev[i].data), i);
		save_item(NAME(dev[i].ctrl), i);
		save_item(NAME(dev[i].wait_ctrl), i);
	}
}

void nscsi_bus_device::device_reset()
{
}

void nscsi_bus_device::regen_data()
{
	data = 0;
	for(int i=0; i<devcnt; i++)
		data |= dev[i].data;
}

void nscsi_bus_device::regen_ctrl(int refid)
{
	static const char *phase[8] = {
		"dout", "din ", "cmd ", "stat", "4   ", "5   ", "mout", "min "
	};

	UINT32 octrl = ctrl;
	ctrl = 0;
	for(int i=0; i<devcnt; i++)
		ctrl |= dev[i].ctrl;

	if(0) {
		logerror("%s: ctrl %c%c%c%c%c%c%c%c%c %s %04x -",
					tag(),
					ctrl & nscsi_device::S_RST ? 'R' : '.',
					ctrl & nscsi_device::S_ATN ? 'A' : '.',
					ctrl & nscsi_device::S_ACK ? 'K' : '.',
					ctrl & nscsi_device::S_REQ ? 'Q' : '.',
					ctrl & nscsi_device::S_SEL ? 'S' : '.',
					ctrl & nscsi_device::S_BSY ? 'B' : '.',
					ctrl & nscsi_device::S_MSG ? 'M' : '.',
					ctrl & nscsi_device::S_CTL ? 'C' : '.',
					ctrl & nscsi_device::S_INP ? 'I' : '.',
					phase[ctrl & 7],
					data);
		for(int i=0; i<devcnt; i++)
			if(dev[i].ctrl) {
				logerror(" %d=", i);
				logerror("%s%s%s%s%s%s%s%s%s",
							dev[i].ctrl & nscsi_device::S_RST ? "R" : "",
							dev[i].ctrl & nscsi_device::S_ATN ? "A" : "",
							dev[i].ctrl & nscsi_device::S_ACK ? "K" : "",
							dev[i].ctrl & nscsi_device::S_REQ ? "Q" : "",
							dev[i].ctrl & nscsi_device::S_MSG ? "M" : "",
							dev[i].ctrl & nscsi_device::S_INP ? "I" : "",
							dev[i].ctrl & nscsi_device::S_CTL ? "C" : "",
							dev[i].ctrl & nscsi_device::S_SEL ? "S" : "",
							dev[i].ctrl & nscsi_device::S_BSY ? "B" : "");
			}
		logerror("\n");
	}

	octrl = octrl ^ ctrl;
	if(octrl)
		for(int i=0; i<devcnt; i++)
			if(i != refid && (dev[i].wait_ctrl & octrl))
				dev[i].dev->scsi_ctrl_changed();
}

UINT32 nscsi_bus_device::data_r() const
{
	return data;
}

UINT32 nscsi_bus_device::ctrl_r() const
{
	return ctrl;
}

void nscsi_bus_device::ctrl_w(int refid, UINT32 lines, UINT32 mask)
{
	UINT32 c = dev[refid].ctrl;
	dev[refid].ctrl = (c & ~mask) | (lines & mask);
	regen_ctrl(refid);
}

void nscsi_bus_device::data_w(int refid, UINT32 lines)
{
	dev[refid].data = lines;
	regen_data();
}

void nscsi_bus_device::ctrl_wait(int refid, UINT32 lines, UINT32 mask)
{
	UINT32 w = dev[refid].wait_ctrl;
	dev[refid].wait_ctrl = (w & ~mask) | (lines & mask);
}

void nscsi_bus_device::device_config_complete()
{
	char id[3];
	for(int i=0; i<16; i++) {
		sprintf(id, "%d", i);
		nscsi_connector *conn = downcast<nscsi_connector *>(subdevice(id));
		if(conn) {
			nscsi_device *sdev = conn->get_device();
			if(sdev) {
				int rid = devcnt++;
				dev[rid].dev = sdev;
				sdev->connect_to_bus(this, rid, i);
			}
		}
	}
}


nscsi_connector::nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NSCSI_CONNECTOR, "NSCSI Connector Abstraction", tag, owner, clock, "nscsi_connector", __FILE__),
	device_slot_interface(mconfig, *this)
{
}

nscsi_connector::~nscsi_connector()
{
}

void nscsi_connector::device_start()
{
}

nscsi_device *nscsi_connector::get_device()
{
	return dynamic_cast<nscsi_device *>(get_card_device());
}

nscsi_device::nscsi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_slot_card_interface(mconfig, *this)
{
	scsi_id = scsi_refid = -1;
	scsi_bus = 0;
}

void nscsi_device::connect_to_bus(nscsi_bus_device *bus, int refid, int default_scsi_id)
{
	scsi_bus = bus;
	scsi_refid = refid;
	scsi_id = default_scsi_id;
}

void nscsi_device::scsi_ctrl_changed()
{
}

void nscsi_device::device_start()
{
	save_item(NAME(scsi_id));
}

nscsi_full_device::nscsi_full_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	nscsi_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


const char *const nscsi_full_device::command_names[256] = {
	/* 00 */ "TEST_UNIT_READY", "REZERO", "?", "REQUEST_SENSE", "FORMAT_UNIT", "?", "?", "REASSIGN_BLOCKS",
	/* 08 */ "READ_6/RECIEVE", "?", "WRITE_6/SEND", "SEEK", "?", "?", "?", "?",
	/* 10 */ "?", "?", "INQUIRY", "?", "?", "MODE_SELECT_6", "RESERVE_6", "RELEASE_6",
	/* 18 */ "?", "?", "MODE_SENSE_6", "START_STOP_UNIT", "RECIEVE_DIAG_RES", "SEND_DIAGNOSTICS", "PREVENT_ALLOW_MEDIUM_REMOVAL", "?",
	/* 20 */ "?", "?", "?", "READ_FORMAT_CAPACITIES", "?", "READ_CAPACITY", "?", "?",

	/* 28 */ "READ_10", "READ_GENERATION", "WRITE_10", "SEEK_10", "ERASE_10", "READ_UPDATED_BLOCK_10", "WRITE_VERIFY", "VERIFY",
	/* 30 */ "SEARCH_DATA_HIGH_10", "SEARCH_DATA_EQUAL_10", "SEARCH_DATA_LOW_10", "SET_LIMITS_10", "PREFETCH", "SYNC_CACHE", "LOCK_UNLOCK_CACHE", "READ_DEFECT_DATA",
	/* 38 */ "MEDIUM_SCAN", "COMPARE", "COPY_AND_VERIFY", "WRITE_BUFFER", "READ_DATA_BUFFER", "UPDATE_BLOCK", "READ_LONG", "WRITE_LONG",
	/* 40 */ "CHANGE_DEFINITION", "WRITE_SAME", "READ_SUB_CHANNEL", "READ_TOC_PMA_ATIP", "READ_HEADER", "PLAY_AUDIO_10", "GET_CONFIGURATION", "PLAY_AUDIO_MSF",
	/* 48 */ "PLAY_AUDIO_TRACK_INDEX", "PLAY_RELATIVE_10", "GET_EVENT_STATUS_NOTIFICATION", "PAUSE_RESUME", "LOG_SELECT", "LOG_SENSE", "STOP_PLAY_SCAN", "?",
	/* 50 */ "XDWRITE", "READ_DISC_INFORMATION/XPWRITE", "READ_TRACK_INFORMATION/XDREAD", "RESERVE_TRACK", "SEND_OPC_INFORMATION", "MODE_SELECT_10", "RESERVE_10", "RELEASE_10",
	/* 58 */ "REPAIR_TRACK", "READ_MASTER_CUE", "MODE_SENSE_10", "CLOSE_TRACK_SESSION", "READ_BUFFER_CAPACITY", "SEND_CUE_SHEET", "PERSISTENT_RESERVE_IN", "PERSISTENT_RESERVE_OUT",
	/* 80 */ "XDWRITE_EXTENDED", "REBUILD", "REGENERATE", "EXTENDED_COPY", "RECEIVE_COPY_RESULTS", "?", "?", "?",
	/* 88 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* 90 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* 98 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* a0 */ "REPORT_LUNS", "BLANK", "SEND_EVENT", "REPORT_DEVICE_IDENTIFIER/SEND_KEY", "SET_DEVICE_IDENTIFIER/REPORT_KEY", "PLAY_AUDIO_12", "LOAD_UNLOAD_MEDIUM", "MOVE_MEDIUM_ATTACHED/SET_READ_AHEAD",
	/* a8 */ "READ_12", "PLAY_RELATIVE_12", "WRITE_12", "?", "ERASE_12/GET_PERFORMANCE", "READ_DVD_STRUCTURE", "WRITE_AND_VERIFY_12", "VERIFY_12",
	/* b0 */ "SEARCH_DATA_HIGH_12", "SEARCH_DATA_EQUAL_12", "SEARCH_DATA_LOW_12", "SET_LIMITS_12", "READ_ELEMENT_STATUS_ATTACHED", "?", "SET_STREAMING", "READ_DEFECT_DATA_12",
	/* b8 */ "?", "READ_CD_MSF", "SCAN_MMC", "SET_CD_SPEED", "PLAY_CD", "MECHANISM_STATUS", "READ_CD", "SEND_DVD_STRUCTURE",
	/* c0 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* c8 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* d0 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* d8 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* e0 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* e8 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* f0 */ "?", "?", "?", "?", "?", "?", "?", "?",
	/* f8 */ "?", "?", "?", "?", "?", "?", "?", "?",
};

void nscsi_full_device::device_start()
{
	nscsi_device::device_start();
	scsi_timer = timer_alloc(SCSI_TIMER);
	save_item(NAME(scsi_cmdbuf));
	save_item(NAME(scsi_sense_buffer));
	save_item(NAME(scsi_cmdsize));
	save_item(NAME(scsi_identify));
	save_item(NAME(scsi_state));
	save_item(NAME(scsi_substate));
	save_item(NAME(scsi_initiator_id));
	save_item(NAME(data_buffer_id));
	save_item(NAME(data_buffer_size));
	save_item(NAME(data_buffer_pos));
	save_item(NAME(buf_control_rpos));
	save_item(NAME(buf_control_wpos));
	for(int i=0; i<32; i++) {
		save_item(NAME(buf_control[i].action), i);
		save_item(NAME(buf_control[i].param1), i);
		save_item(NAME(buf_control[i].param2), i);
	}
}

void nscsi_full_device::device_reset()
{
	scsi_state = scsi_substate = IDLE;
	buf_control_rpos = buf_control_wpos = 0;
	scsi_identify = 0;
	scsi_bus->data_w(scsi_refid, 0);
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	scsi_bus->ctrl_wait(scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
}

void nscsi_full_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id != SCSI_TIMER)
		return;

	step(true);

}

void nscsi_full_device::scsi_ctrl_changed()
{
	step(false);
}

void nscsi_full_device::step(bool timeout)
{
	UINT32 ctrl = scsi_bus->ctrl_r();
	UINT32 data = scsi_bus->data_r();
	if(ctrl & S_RST) {
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
		scsi_state = IDLE;
		logerror("%s: scsi bus reset\n", tag());
		return;
	}

	if(0)
		logerror("%s: state=%d.%d %s\n",
					tag(), scsi_state & STATE_MASK, (scsi_state & SUB_MASK) >> SUB_SHIFT,
					timeout ? "timeout" : "change");

	switch(scsi_state & SUB_MASK ? scsi_state & SUB_MASK : scsi_state & STATE_MASK) {
	case IDLE:
		if(((ctrl & (S_SEL|S_BSY)) == S_SEL) && (scsi_id != -1) && ((data & (1 << scsi_id)) != 0)) {
			for(scsi_initiator_id = 0; scsi_initiator_id != 16 && (scsi_initiator_id == scsi_id || (data & (1 << scsi_initiator_id))); scsi_initiator_id++);
			if(scsi_initiator_id == 16)
				scsi_initiator_id = -1;
			scsi_state = TARGET_SELECT_WAIT_BUS_SETTLE;
			scsi_timer->adjust(scsi_bus_settle_delay());
		}
		break;

	case TARGET_SELECT_WAIT_BUS_SETTLE:
		if((ctrl & (S_SEL|S_BSY)) == S_SEL) {
			scsi_state = TARGET_SELECT_WAIT_SEL_0;
			scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
		} else
			scsi_state = IDLE;
		break;

	case TARGET_SELECT_WAIT_SEL_0:
		if(ctrl & S_SEL)
			break;
		buf_control_push()->action = BC_MSG_OR_COMMAND;
		scsi_state = TARGET_NEXT_CONTROL;
		step(false);
		break;

	case RECV_BYTE_T_WAIT_ACK_1 << SUB_SHIFT:
		if(ctrl & S_ACK) {
			scsi_put_data(data_buffer_id, data_buffer_pos++, scsi_bus->data_r());
			scsi_state = (scsi_state & STATE_MASK) | (RECV_BYTE_T_WAIT_ACK_0 << SUB_SHIFT);
			scsi_bus->ctrl_w(scsi_refid, 0, S_REQ);
		}
		break;

	case RECV_BYTE_T_WAIT_ACK_0 << SUB_SHIFT:
		if(!(ctrl & S_ACK)) {
			scsi_state &= STATE_MASK;
			scsi_bus->ctrl_wait(scsi_refid, 0, S_ACK);
			step(false);
		}
		break;

	case SEND_BYTE_T_WAIT_ACK_1 << SUB_SHIFT:
		if(ctrl & S_ACK) {
			scsi_state = (scsi_state & STATE_MASK) | (SEND_BYTE_T_WAIT_ACK_0 << SUB_SHIFT);
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_REQ);
		}
		break;

	case SEND_BYTE_T_WAIT_ACK_0 << SUB_SHIFT:
		if(!(ctrl & S_ACK)) {
			scsi_state &= STATE_MASK;
			scsi_bus->ctrl_wait(scsi_refid, 0, S_ACK);
			step(false);
		}
		break;

	case TARGET_NEXT_CONTROL: {
		control *ctl = buf_control_pop();
		switch(ctl->action) {
		case BC_MSG_OR_COMMAND:
			data_buffer_id = SBUF_MAIN;
			data_buffer_pos = 0;
			if(ctrl & S_ATN) {
				scsi_state = TARGET_WAIT_MSG_BYTE;
				scsi_bus->ctrl_w(scsi_refid, S_PHASE_MSG_OUT, S_PHASE_MASK);
			} else {
				scsi_state = TARGET_WAIT_CMD_BYTE;
				scsi_bus->ctrl_w(scsi_refid, S_PHASE_COMMAND, S_PHASE_MASK);
			}
			target_recv_byte();
			break;

		case BC_STATUS:
			scsi_bus->ctrl_w(scsi_refid, S_PHASE_STATUS, S_PHASE_MASK);
			target_send_byte(ctl->param1);
			break;

		case BC_DATA_IN:
			scsi_bus->ctrl_w(scsi_refid, S_PHASE_DATA_IN, S_PHASE_MASK);
			data_buffer_id = ctl->param1;
			data_buffer_size = ctl->param2;
			data_buffer_pos = 0;
			scsi_state = TARGET_WAIT_DATA_IN_BYTE;
			target_send_buffer_byte();
			break;

		case BC_DATA_OUT:
			scsi_bus->ctrl_w(scsi_refid, S_PHASE_DATA_OUT, S_PHASE_MASK);
			data_buffer_id = ctl->param1;
			data_buffer_size = ctl->param2;
			data_buffer_pos = 0;
			scsi_state = TARGET_WAIT_DATA_OUT_BYTE;
			target_recv_byte();
			break;

		case BC_MESSAGE_1:
			scsi_bus->ctrl_w(scsi_refid, S_PHASE_MSG_IN, S_PHASE_MASK);
			target_send_byte(ctl->param1);
			break;

		case BC_BUS_FREE:
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_wait(scsi_refid, S_BSY|S_SEL|S_RST, S_ALL);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
			scsi_state = IDLE;
			break;
		};
		break;
	}

	case TARGET_WAIT_DATA_IN_BYTE:
		if(data_buffer_pos == data_buffer_size-1)
			scsi_state = TARGET_NEXT_CONTROL;
		target_send_buffer_byte();
		break;

	case TARGET_WAIT_DATA_OUT_BYTE:
		if(data_buffer_pos == data_buffer_size-1)
			scsi_state = TARGET_NEXT_CONTROL;
		target_recv_byte();
		break;

	case TARGET_WAIT_MSG_BYTE:
		if(ctrl & S_SEL)
			return;
		if(!(ctrl & S_ATN)) {
			scsi_cmdsize = data_buffer_pos;
			scsi_message();
			data_buffer_id = SBUF_MAIN;
			data_buffer_pos = 0;
			scsi_state = TARGET_WAIT_CMD_BYTE;
			scsi_bus->ctrl_w(scsi_refid, S_PHASE_COMMAND, S_PHASE_MASK);
		}
		target_recv_byte();
		break;

	case TARGET_WAIT_CMD_BYTE:
		if(ctrl & S_SEL)
			return;
		if(ctrl & S_ATN) {
			logerror("%s: Parity error? Say what?\n", tag());
			scsi_state = IDLE;
			break;
		}

		if(command_done()) {
			scsi_cmdsize = data_buffer_pos;
			scsi_bus->ctrl_wait(scsi_refid, 0, S_ACK);
			scsi_command();
			scsi_state = TARGET_NEXT_CONTROL;
			step(false);
		} else
			target_recv_byte();
		break;

	default:
		logerror("%s: step() unexpected state %d.%d\n",
					tag(),
					scsi_state & STATE_MASK, (scsi_state & SUB_MASK) >> SUB_SHIFT);
		exit(0);
	}
}

void nscsi_full_device::target_recv_byte()
{
	scsi_bus->ctrl_wait(scsi_refid, S_ACK, S_ACK);
	scsi_state = (scsi_state & STATE_MASK) | (RECV_BYTE_T_WAIT_ACK_1 << SUB_SHIFT);
	scsi_bus->ctrl_w(scsi_refid, S_REQ, S_REQ);
	step(false);
}

void nscsi_full_device::target_send_byte(UINT8 val)
{
	scsi_bus->ctrl_wait(scsi_refid, S_ACK, S_ACK);
	scsi_state = (scsi_state & STATE_MASK) | (SEND_BYTE_T_WAIT_ACK_1 << SUB_SHIFT);
	scsi_bus->data_w(scsi_refid, val);
	scsi_bus->ctrl_w(scsi_refid, S_REQ, S_REQ);
	step(false);
}

UINT8 nscsi_full_device::scsi_get_data(int id, int pos)
{
	switch(id) {
	case SBUF_MAIN:
		return scsi_cmdbuf[pos];
	case SBUF_SENSE:
		return scsi_sense_buffer[pos];
	default:
		fatalerror("nscsi_full_device::scsi_get_data - unknown id\n");
	}
}

void nscsi_full_device::scsi_put_data(int id, int pos, UINT8 data)
{
	switch(id) {
	case SBUF_MAIN:
		scsi_cmdbuf[pos] = data;
		break;
	case SBUF_SENSE:
		scsi_sense_buffer[pos] = data;
		break;
	default:
		fatalerror("nscsi_full_device::scsi_put_data - unknown id\n");
	}
}

void nscsi_full_device::target_send_buffer_byte()
{
	target_send_byte(scsi_get_data(data_buffer_id, data_buffer_pos++));
}

bool nscsi_full_device::command_done()
{
	if(!data_buffer_pos)
		return false;
	UINT8 h = scsi_cmdbuf[0];
	switch(h >> 5) {
	case 0: return data_buffer_pos == 6;
	case 1: return data_buffer_pos == 10;
	case 2: return data_buffer_pos == 10;
	case 3: return true;
	case 4: return true;
	case 5: return data_buffer_pos == 12;
	case 6: return true;
	case 7: return true;
	}
	return true;
}

nscsi_full_device::control *nscsi_full_device::buf_control_push()
{
	if(buf_control_wpos == int(ARRAY_LENGTH(buf_control)))
		throw emu_fatalerror("%s: buf_control overflow\n", tag());

	control *c = buf_control + buf_control_wpos;
	buf_control_wpos++;
	return c;
}

nscsi_full_device::control *nscsi_full_device::buf_control_pop()
{
	if(buf_control_rpos == buf_control_wpos)
		throw emu_fatalerror("%s: buf_control underflow\n", tag());

	control *c = buf_control + buf_control_rpos;
	buf_control_rpos++;
	if(buf_control_rpos == buf_control_wpos)
		buf_control_rpos = buf_control_wpos = 0;
	return c;
}

void nscsi_full_device::scsi_status_complete(UINT8 st)
{
	control *c;
	c = buf_control_push();
	c->action = BC_STATUS;
	c->param1 = st;
	c = buf_control_push();
	c->action = BC_MESSAGE_1;
	c->param1 = SM_COMMAND_COMPLETE;
	c = buf_control_push();
	c->action = BC_BUS_FREE;
}

void nscsi_full_device::scsi_data_in(int buf, int size)
{
	control *c;
	c = buf_control_push();
	c->action = BC_DATA_IN;
	c->param1 = buf;
	c->param2 = size;
}

void nscsi_full_device::scsi_data_out(int buf, int size)
{
	control *c;
	c = buf_control_push();
	c->action = BC_DATA_OUT;
	c->param1 = buf;
	c->param2 = size;
}

void nscsi_full_device::sense(bool deferred, UINT8 key)
{
	memset(scsi_sense_buffer, 0, sizeof(scsi_sense_buffer));
	scsi_sense_buffer[0] = deferred ? 0x71 : 0x70;
	scsi_sense_buffer[2] = key;
}

void nscsi_full_device::scsi_unknown_command()
{
	logerror("%s: Unhandled command %s", tag(), command_names[scsi_cmdbuf[0]]);
	for(int i=0; i != scsi_cmdsize; i++)
		logerror(" %02x", scsi_cmdbuf[i]);
	logerror("\n");

	scsi_status_complete(SS_CHECK_CONDITION);
	sense(false, 5);
}

void nscsi_full_device::scsi_command()
{
	switch(scsi_cmdbuf[0]) {
	case SC_REQUEST_SENSE:
		logerror("%s: command REQUEST SENSE\n", tag());
		scsi_data_in(SBUF_SENSE, 8);
		scsi_status_complete(SS_GOOD);
		break;
	default:
		scsi_unknown_command();
		break;
	}
}

void nscsi_full_device::scsi_message()
{
	if(scsi_cmdbuf[0] & 0x80) {
		scsi_identify = scsi_cmdbuf[0];
		return;
	}

	logerror("%s: Unknown message", tag());
	for(int i=0; i != scsi_cmdsize; i++)
		logerror(" %02x", scsi_cmdbuf[i]);
	logerror("\n");
}

int nscsi_full_device::get_lun(int def)
{
	if(scsi_identify & 0x80)
		return scsi_identify & 0x7f;
	return def;
}

void nscsi_full_device::bad_lun()
{
	scsi_status_complete(SS_CHECK_CONDITION);
	sense(false, 2);
}

// Arbitration delay (2.4us)
attotime nscsi_full_device::scsi_arbitation_delay()
{
	return attotime::from_nsec(2400);
}

// Assertion period (90ns)
attotime nscsi_full_device::scsi_assertion_period()
{
	return attotime::from_nsec(90);
}

// Bus clear delay (800ns)
attotime nscsi_full_device::scsi_bus_clear_delay()
{
	return attotime::from_nsec(800);
}

// Bus free delay (800ns)
attotime nscsi_full_device::scsi_bus_free_delay()
{
	return attotime::from_nsec(800);
}

// Bus set delay (1.8us)
attotime nscsi_full_device::scsi_bus_set_delay()
{
	return attotime::from_nsec(1800);
}

// Bus settle delay (400ns)
attotime nscsi_full_device::scsi_bus_settle_delay()
{
	return attotime::from_nsec(400);
}

// Cable skew delay (10ns)
attotime nscsi_full_device::scsi_cable_skew_delay()
{
	return attotime::from_nsec(10);
}

// Data release delay (400ns)
attotime nscsi_full_device::scsi_data_release_delay()
{
	return attotime::from_nsec(40);
}

// Deskew delay (45ns)
attotime nscsi_full_device::scsi_deskew_delay()
{
	return attotime::from_nsec(45);
}

// Disconnection delay (200us)
attotime nscsi_full_device::scsi_disconnection_delay()
{
	return attotime::from_usec(200);
}

// Hold time (45ns)
attotime nscsi_full_device::scsi_hold_time()
{
	return attotime::from_nsec(45);
}

// Negation period (90ns)
attotime nscsi_full_device::scsi_negation_period()
{
	return attotime::from_nsec(90);
}

// Reset hold time (25us)
attotime nscsi_full_device::scsi_reset_hold_time()
{
	return attotime::from_usec(25);
}

// Selection abort time (200us)
attotime nscsi_full_device::scsi_selection_abort_time()
{
	return attotime::from_usec(200);
}

// Selection timeout delay (250ms)
attotime nscsi_full_device::scsi_selection_timeout_delay()
{
	return attotime::from_msec(250);
}

// Fast assertion period (30ns)
attotime nscsi_full_device::scsi_fast_assertion_period()
{
	return attotime::from_nsec(30);
}

// Fast cable skew delay (5ns)
attotime nscsi_full_device::scsi_fast_cable_skew_delay()
{
	return attotime::from_nsec(5);
}

// Fast deskew delay (20ns)
attotime nscsi_full_device::scsi_fast_deskew_delay()
{
	return attotime::from_nsec(20);
}

// Fast hold time (10ns)
attotime nscsi_full_device::scsi_fast_hold_time()
{
	return attotime::from_nsec(10);
}

// Fast negation period (30ns)
attotime nscsi_full_device::scsi_fast_negation_period()
{
	return attotime::from_nsec(30);
}
