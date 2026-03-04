// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "nscsi_hle.h"

#include "util/multibyte.h"


#define LOG_UNSUPPORTED (1U << 1)
#define LOG_STATE       (1U << 2)
#define LOG_DATA        (1U << 3)
#define LOG_DATA_SENT   (1U << 4)

#define VERBOSE (LOG_UNSUPPORTED)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

nscsi_full_device::nscsi_full_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	nscsi_device_interface(mconfig, *this),
	nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
{
}

const char *const nscsi_full_device::command_names[256] = {
	/* 00 */ "TEST_UNIT_READY", "REWIND/REZERO_UNIT", "REQUEST_BLOCK_ADDRESS", "REQUEST_SENSE", "FORMAT/FORMAT_UNIT", "READ_BLOCK_LIMITS", "?", "INITIALIZE_ELEMENT_STATUS/REASSIGN_BLOCKS",
	/* 08 */ "GET_MESSAGE_6/READ_6/RECEIVE", "?", "PRINT/SEND_MESSAGE_6/SEND_6/WRITE_6", "SEEK_6/SLEW_AND_PRINT", "SEEK_BLOCK", "?", "?", "READ_REVERSE",
	/* 10 */ "SYNCHRONIZE_BUFFER/WRITE_FILEMARKS", "SPACE", "INQUIRY", "VERIFY_6", "RECOVER_BUFFERED_DATA", "MODE_SELECT_6", "RESERVE_6/RESERVE_UNIT", "RELEASE_6/RELEASE_UNIT",
	/* 18 */ "COPY", "ERASE", "MODE_SENSE_6", "LOAD_UNLOAD/SCAN/STOP_PRINT/START_STOP_UNIT", "RECEIVE_DIAGNOSTIC_RESULTS", "SEND_DIAGNOSTIC", "PREVENT_ALLOW_MEDIUM_REMOVAL", "?",
	/* 20 */ "?", "?", "?", "READ_FORMAT_CAPACITIES", "SET_WINDOW", "GET_WINDOW/READ_CAPACITY/READ_CD_RECORDED_CAPACITY", "?", "?",

	/* 28 */ "GET_MESSAGE_10/READ_10", "READ_GENERATION", "SEND_MESSAGE_10/SEND_10/WRITE_10", "LOCATE/POSITION_TO_ELEMENT/SEEK_10", "ERASE_10", "READ_UPDATED_BLOCK", "WRITE_AND_VERIFY_10", "VERIFY_10",
	/* 30 */ "SEARCH_DATA_HIGH_10", "OBJECT_POSITION/SEARCH_DATA_EQUAL_10", "SEARCH_DATA_LOW_10", "SET_LIMITS_10", "PREFETCH/READ_POSITION", "SYNCHRONIZE_CACHE", "LOCK_UNLOCK_CACHE", "READ_DEFECT_DATA_10",
	/* 38 */ "MEDIUM_SCAN", "COMPARE", "COPY_AND_VERIFY", "WRITE_BUFFER", "READ_BUFFER", "UPDATE_BLOCK", "READ_LONG", "WRITE_LONG",
	/* 40 */ "CHANGE_DEFINITION", "WRITE_SAME", "READ_SUB_CHANNEL", "READ_TOC_PMA_ATIP", "READ_HEADER", "PLAY_AUDIO_10", "GET_CONFIGURATION", "PLAY_AUDIO_MSF",
	/* 48 */ "PLAY_AUDIO_TRACK_INDEX", "PLAY_TRACK_RELATIVE_10", "GET_EVENT_STATUS_NOTIFICATION", "PAUSE_RESUME", "LOG_SELECT", "LOG_SENSE", "STOP_PLAY_SCAN", "?",
	/* 50 */ "XDWRITE", "READ_DISC_INFORMATION/XPWRITE", "READ_TRACK_INFORMATION/XDREAD", "RESERVE_TRACK", "SEND_OPC_INFORMATION", "MODE_SELECT_10", "RESERVE_10", "RELEASE_10",
	/* 58 */ "REPAIR_TRACK", "READ_MASTER_CUE", "MODE_SENSE_10", "CLOSE_TRACK_SESSION", "READ_BUFFER_CAPACITY", "SEND_CUE_SHEET", "PERSISTENT_RESERVE_IN", "PERSISTENT_RESERVE_OUT",
	/* 80 */ "XDWRITE_EXTENDED", "REBUILD", "REGENERATE", "EXTENDED_COPY", "RECEIVE_COPY_RESULTS", "?", "?", "?",
	/* 88 */ "?", "?", "WRITE_16", "?", "?", "?", "?", "?",
	/* 90 */ "?", "SYNCHRONIZE_CACHE_16", "?", "WRITE_SAME_16", "?", "?", "?", "?",
	/* 98 */ "?", "?", "?", "?", "?", "?", "READ_CAPACITY_166/READ_LONG_16", "WRITE_LONG_16",
	/* a0 */ "REPORT_LUNS", "BLANK", "SEND_EVENT", "REPORT_DEVICE_IDENTIFIER/SEND_KEY", "SET_DEVICE_IDENTIFIER/REPORT_KEY", "PLAY_AUDIO_12", "LOAD_UNLOAD_MEDIUM", "MOVE_MEDIUM_ATTACHED/SET_READ_AHEAD",
	/* a8 */ "READ_12", "PLAY_RELATIVE_12", "WRITE_12", "?", "ERASE_12/GET_PERFORMANCE", "READ_DVD_STRUCTURE", "WRITE_AND_VERIFY_12", "VERIFY_12",
	/* b0 */ "SEARCH_DATA_HIGH_12", "SEARCH_DATA_EQUAL_12", "SEARCH_DATA_LOW_12", "SET_LIMITS_12", "READ_ELEMENT_STATUS_ATTACHED", "?", "SET_STREAMING", "READ_DEFECT_DATA_12",
	/* b8 */ "?", "READ_CD_MSF", "SCAN_MMC", "SET_CD_SPEED", "PLAY_CD/SPARE_IN", "MECHANISM_STATUS/SPARE_OUT", "READ_CD", "SEND_DVD_STRUCTURE",
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
	m_scsi_timer = timer_alloc(FUNC(nscsi_full_device::update_tick), this);
	save_item(NAME(m_scsi_cmdbuf));
	save_item(NAME(m_scsi_sense_buffer));
	save_item(NAME(m_scsi_cmdsize));
	save_item(NAME(m_scsi_identify));
	save_item(NAME(m_scsi_state));
	save_item(NAME(m_scsi_substate));
	save_item(NAME(m_scsi_initiator_id));
	save_item(NAME(m_data_buffer_id));
	save_item(NAME(m_data_buffer_size));
	save_item(NAME(m_data_buffer_pos));
	save_item(NAME(m_buf_control_rpos));
	save_item(NAME(m_buf_control_wpos));
	for(int i=0; i<32; i++) {
		save_item(NAME(m_buf_control[i].m_action), i);
		save_item(NAME(m_buf_control[i].m_param1), i);
		save_item(NAME(m_buf_control[i].m_param2), i);
	}
}

void nscsi_full_device::device_reset()
{
	m_scsi_state = m_scsi_substate = IDLE;
	m_buf_control_rpos = m_buf_control_wpos = 0;
	m_scsi_identify = 0;
	m_data_buffer_size = 0;
	m_data_buffer_pos = 0;
	m_scsi_bus->data_w(m_scsi_refid, 0);
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_SEL|S_BSY|S_RST, S_ALL);
	sense(false, SK_NO_SENSE);
}

TIMER_CALLBACK_MEMBER(nscsi_full_device::update_tick)
{
	step(param);
}

void nscsi_full_device::scsi_ctrl_changed()
{
	step(false);
}

void nscsi_full_device::step(bool timeout)
{
	uint32_t ctrl = m_scsi_bus->ctrl_r();
	uint32_t data = m_scsi_bus->data_r();
	if(ctrl & S_RST) {
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
		m_scsi_state = IDLE;
		LOG("scsi bus reset\n");
		m_scsi_state = m_scsi_substate = IDLE;
		m_buf_control_rpos = m_buf_control_wpos = 0;
		m_scsi_identify = 0;
		m_data_buffer_size = 0;
		m_data_buffer_pos = 0;
		return;
	}

	LOGMASKED(LOG_STATE, "state=%d.%d %s\n",
		m_scsi_state & STATE_MASK, (m_scsi_state & SUB_MASK) >> SUB_SHIFT,
		timeout ? "timeout" : "change");

	switch(m_scsi_state & SUB_MASK ? m_scsi_state & SUB_MASK : m_scsi_state & STATE_MASK) {
	case IDLE:
		if(((ctrl & (S_SEL|S_BSY)) == S_SEL) && (m_scsi_id != -1) && ((data & (1 << m_scsi_id)) != 0)) {
			for(m_scsi_initiator_id = 0; m_scsi_initiator_id != 16 && (m_scsi_initiator_id == m_scsi_id || (data & (1 << m_scsi_initiator_id))); m_scsi_initiator_id++) {};
			if(m_scsi_initiator_id == 16)
				m_scsi_initiator_id = -1;
			m_scsi_state = TARGET_SELECT_WAIT_BUS_SETTLE;
			m_scsi_timer->adjust(scsi_bus_settle_delay(), true);
		}
		break;

	case TARGET_SELECT_WAIT_BUS_SETTLE:
		if((ctrl & (S_SEL|S_BSY)) == S_SEL) {
			m_scsi_state = TARGET_SELECT_WAIT_SEL_0;
			m_scsi_bus->ctrl_w(m_scsi_refid, S_BSY, S_BSY);
		} else
			m_scsi_state = IDLE;
		break;

	case TARGET_SELECT_WAIT_SEL_0:
		if(ctrl & S_SEL)
			break;
		buf_control_push()->m_action = BC_MSG_OR_COMMAND;
		m_scsi_state = TARGET_NEXT_CONTROL;
		step(false);
		break;

	case RECV_BYTE_T_WAIT_ACK_1 << SUB_SHIFT:
		if(ctrl & S_ACK) {
			scsi_put_data(m_data_buffer_id, m_data_buffer_pos++, m_scsi_bus->data_r());
			m_scsi_state = (m_scsi_state & STATE_MASK) | (RECV_BYTE_T_WAIT_ACK_0 << SUB_SHIFT);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_REQ);
		}
		break;

	case RECV_BYTE_T_WAIT_ACK_0 << SUB_SHIFT:
		if(!(ctrl & S_ACK)) {
			m_scsi_state &= STATE_MASK;
			m_scsi_bus->ctrl_wait(m_scsi_refid, 0, S_ACK);
			attotime delay = scsi_data_byte_period();
			if (delay == attotime::zero) {
				step(false);
			} else {
				m_scsi_timer->adjust(delay, false);
			}
		}
		break;

	case SEND_BYTE_T_WAIT_ACK_1 << SUB_SHIFT:
		if(ctrl & S_ACK) {
			m_scsi_state = (m_scsi_state & STATE_MASK) | (SEND_BYTE_T_WAIT_ACK_0 << SUB_SHIFT);
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_REQ);
		}
		break;

	case SEND_BYTE_T_WAIT_ACK_0 << SUB_SHIFT:
		if(!(ctrl & S_ACK)) {
			m_scsi_state &= STATE_MASK;
			m_scsi_bus->ctrl_wait(m_scsi_refid, 0, S_ACK);
			attotime delay = scsi_data_byte_period();
			if (delay == attotime::zero) {
				step(false);
			} else {
				m_scsi_timer->adjust(delay, false);
			}
		}
		break;

	case TARGET_NEXT_CONTROL: {
		control *ctl = buf_control_pop();
		switch(ctl->m_action) {
		case BC_MSG_OR_COMMAND:
			m_data_buffer_id = SBUF_MAIN;
			m_data_buffer_pos = 0;
			if(ctrl & S_ATN) {
				m_scsi_state = TARGET_WAIT_MSG_BYTE;
				m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_MSG_OUT, S_PHASE_MASK);
			} else {
				m_scsi_state = TARGET_WAIT_CMD_BYTE;
				m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_COMMAND, S_PHASE_MASK);
			}
			target_recv_byte();
			break;

		case BC_STATUS:
			m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_STATUS, S_PHASE_MASK);
			target_send_byte(ctl->m_param1);
			break;

		case BC_DATA_IN:
			m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_DATA_IN, S_PHASE_MASK);
			m_data_buffer_id = ctl->m_param1;
			m_data_buffer_size = ctl->m_param2;
			m_data_buffer_pos = 0;
			if (m_data_buffer_size > 0) {
				m_scsi_state = TARGET_WAIT_DATA_IN_BYTE;
			}
			else {
				m_scsi_state = TARGET_NEXT_CONTROL;
			}
			step(false);
			break;

		case BC_DATA_OUT:
			m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_DATA_OUT, S_PHASE_MASK);
			m_data_buffer_id = ctl->m_param1;
			m_data_buffer_size = ctl->m_param2;
			m_data_buffer_pos = 0;
			if (m_data_buffer_size > 0) {
				m_scsi_state = TARGET_WAIT_DATA_OUT_BYTE;
			}
			else {
				m_scsi_state = TARGET_NEXT_CONTROL;
			}
			step(false);
			break;

		case BC_MESSAGE_1:
			m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_MSG_IN, S_PHASE_MASK);
			target_send_byte(ctl->m_param1);
			break;

		case BC_BUS_FREE:
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY|S_SEL|S_RST, S_ALL);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
			m_scsi_state = IDLE;
			break;
		};
		break;
	}

	case TARGET_WAIT_DATA_IN_BYTE:
		if(m_data_buffer_pos == m_data_buffer_size-1)
			m_scsi_state = TARGET_NEXT_CONTROL;
		target_send_buffer_byte();
		break;

	case TARGET_WAIT_DATA_OUT_BYTE:
		if(m_data_buffer_pos == m_data_buffer_size-1)
			m_scsi_state = TARGET_NEXT_CONTROL;
		target_recv_byte();
		break;

	case TARGET_WAIT_MSG_BYTE:
		if(ctrl & S_SEL)
			return;
		if(!(ctrl & S_ATN)) {
			m_scsi_cmdsize = m_data_buffer_pos;
			scsi_message();
			m_data_buffer_id = SBUF_MAIN;
			m_data_buffer_pos = 0;
			m_scsi_state = TARGET_WAIT_CMD_BYTE;
			m_scsi_bus->ctrl_w(m_scsi_refid, S_PHASE_COMMAND, S_PHASE_MASK);
		}
		target_recv_byte();
		break;

	case TARGET_WAIT_CMD_BYTE:
		if(ctrl & S_SEL)
			return;
		if(ctrl & S_ATN) {
			LOG("Parity error? Say what?\n");
			m_scsi_state = IDLE;
			break;
		}

		if(scsi_command_done(m_scsi_cmdbuf[0], m_data_buffer_pos)) {
			m_scsi_cmdsize = m_data_buffer_pos;
			m_scsi_bus->ctrl_wait(m_scsi_refid, 0, S_ACK);
			scsi_command();
			m_scsi_state = TARGET_NEXT_CONTROL;
			attotime delay = scsi_data_command_delay();
			if (delay == attotime::zero) {
				step(false);
			} else {
				m_scsi_timer->adjust(delay, false);
			}
		} else
			target_recv_byte();
		break;

	default:
		LOG("step() unexpected state %d.%d\n",
			m_scsi_state & STATE_MASK, (m_scsi_state & SUB_MASK) >> SUB_SHIFT);
		exit(0);
	}
}

void nscsi_full_device::target_recv_byte()
{
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_ACK, S_ACK);
	m_scsi_state = (m_scsi_state & STATE_MASK) | (RECV_BYTE_T_WAIT_ACK_1 << SUB_SHIFT);
	m_scsi_bus->ctrl_w(m_scsi_refid, S_REQ, S_REQ);
	step(false);
}

void nscsi_full_device::target_send_byte(uint8_t val)
{
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_ACK, S_ACK);
	m_scsi_state = (m_scsi_state & STATE_MASK) | (SEND_BYTE_T_WAIT_ACK_1 << SUB_SHIFT);
	m_scsi_bus->data_w(m_scsi_refid, val);
	m_scsi_bus->ctrl_w(m_scsi_refid, S_REQ, S_REQ);
	step(false);
}

uint8_t nscsi_full_device::scsi_get_data(int id, int pos)
{
	switch(id) {
	case SBUF_MAIN:
		LOGMASKED(LOG_DATA, "scsi_get_data MAIN, id:%d pos:%d data:%02x %c\n", id, pos, m_scsi_cmdbuf[pos], m_scsi_cmdbuf[pos] >= 0x20 && m_scsi_cmdbuf[pos] < 0x7f ? (char)m_scsi_cmdbuf[pos] : ' ');
		return m_scsi_cmdbuf[pos];
	case SBUF_SENSE:
		return m_scsi_sense_buffer[pos];
	default:
		fatalerror("nscsi_full_device::scsi_get_data - unknown id\n");
	}
}

void nscsi_full_device::scsi_put_data(int id, int pos, uint8_t data)
{
	switch(id) {
	case SBUF_MAIN:
		LOGMASKED(LOG_DATA, "nscsi_bus: scsi_put_data MAIN, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		m_scsi_cmdbuf[pos] = data;
		break;
	case SBUF_SENSE:
		m_scsi_sense_buffer[pos] = data;
		break;
	default:
		fatalerror("nscsi_full_device::scsi_put_data - unknown id %d\n", id);
	}
}

void nscsi_full_device::target_send_buffer_byte()
{
	target_send_byte(scsi_get_data(m_data_buffer_id, m_data_buffer_pos++));
}

bool nscsi_full_device::scsi_command_done(uint8_t command, uint8_t length)
{
	if(!length)
		return false;
	switch(command >> 5) {
	case 0: return length == 6;
	case 1: return length == 10;
	case 2: return length == 10;
	case 3: return true;
	case 4: return true;
	case 5: return length == 12;
	case 6: return true;
	case 7: return length == 32;
	case 8: return length == 16;
	case 9: return length == 16;
	}
	return true;
}

nscsi_full_device::control *nscsi_full_device::buf_control_push()
{
	if(m_buf_control_wpos == int(std::size(m_buf_control)))
		throw emu_fatalerror("%s: buf_control overflow\n", tag());

	control *c = m_buf_control + m_buf_control_wpos;
	m_buf_control_wpos++;
	return c;
}

nscsi_full_device::control *nscsi_full_device::buf_control_pop()
{
	if(m_buf_control_rpos == m_buf_control_wpos)
		throw emu_fatalerror("%s: buf_control underflow\n", tag());

	control *c = m_buf_control + m_buf_control_rpos;
	m_buf_control_rpos++;
	if(m_buf_control_rpos == m_buf_control_wpos)
		m_buf_control_rpos = m_buf_control_wpos = 0;
	return c;
}

void nscsi_full_device::scsi_status_complete(uint8_t st)
{
	control *c;
	c = buf_control_push();
	c->m_action = BC_STATUS;
	c->m_param1 = st;
	c = buf_control_push();
	c->m_action = BC_MESSAGE_1;
	c->m_param1 = SM_COMMAND_COMPLETE;
	c = buf_control_push();
	c->m_action = BC_BUS_FREE;
}

void nscsi_full_device::scsi_data_in(int buf, int size)
{
	if((VERBOSE & LOG_DATA_SENT) && buf == 0) {
		std::string dt = "";
		int sz = size;
		if(sz > 50)
			sz = 50;
		for(int i=0; i<sz; i++)
			dt += util::string_format(" %02x", m_scsi_cmdbuf[i]);
		if(size > sz)
			dt += " ...";
		LOGMASKED(LOG_DATA_SENT, "Sending data (%d)%s\n", size, dt);
	}
	control *c;
	c = buf_control_push();
	c->m_action = BC_DATA_IN;
	c->m_param1 = buf;
	c->m_param2 = size;
}

void nscsi_full_device::scsi_data_out(int buf, int size)
{
	control *c;
	c = buf_control_push();
	c->m_action = BC_DATA_OUT;
	c->m_param1 = buf;
	c->m_param2 = size;
}

//////////////////////////////////////////////////////////////////////////////

void nscsi_full_device::set_sense_data(const u8 sense_key, const u16 sense_key_code, const sense_data *data)
{
	assert(sizeof(m_scsi_sense_buffer) >= 18);
	assert(sense_key <= 0x0f);
	memset(m_scsi_sense_buffer, 0, 18);
	if (data) {
		m_scsi_sense_buffer[0] = (data->invalid ? 0 : 0x80) // even though SCSI-2 section 8.2.14 implies valid bit should always be set, other sections such as 10.2.12 disagree!
							 | (data->deferred ? 0x71 : 0x70);
		m_scsi_sense_buffer[2] = (data->filemark ? 0x80 : 0)
							 | (data->eom ? 0x40 : 0)
							 | (data->bad_len ? 0x20 : 0); // "incorrect length indicator"
		put_s32be(&m_scsi_sense_buffer[3], data->info);
	}
	else
		m_scsi_sense_buffer[0] = 0xf0;
	m_scsi_sense_buffer[2] |= sense_key;
	m_scsi_sense_buffer[7] = 10; // additional sense length
	put_u16be(&m_scsi_sense_buffer[12], sense_key_code);
}

void nscsi_full_device::sense(bool deferred, uint8_t key, uint8_t asc, uint8_t ascq)
{
	sense_data s;
	s.deferred = deferred;
	set_sense_data(key, (asc << 8) | ascq, &s);
}

void nscsi_full_device::report_condition(const u8 sense_key, const u16 sense_key_code, const sense_data *data)
{
	set_sense_data(sense_key, sense_key_code, data);
	scsi_status_complete(SS_CHECK_CONDITION);
}

void nscsi_full_device::report_bad_lun(const u8 cmd, const u8 lun)
{
	LOG("%s (0x%02x) lun=%d\n    *** BAD LUN\n", command_names[cmd], cmd, lun);
	report_condition(SK_ILLEGAL_REQUEST, SKC_LOGICAL_UNIT_NOT_SUPPORTED);
}

void nscsi_full_device::report_bad_cmd(const u8 cmd)
{
	LOG("%s (0x%02x)\n    *** BAD COMMAND\n", command_names[cmd], cmd);
	report_condition(SK_ILLEGAL_REQUEST, SKC_INVALID_COMMAND_OPERATION_CODE);
}

void nscsi_full_device::report_filemark(const s32 info, const bool eom)
{
	LOG("    *** FILEMARK info=%d\n", info);
	sense_data s;
	s.filemark = true;
	s.eom = eom;
	s.info = info;
	report_condition(SK_NO_SENSE, SKC_FILEMARK_DETECTED, &s);
}

void nscsi_full_device::report_bom(const s32 info)
{
	LOG("    *** BOM info=%d\n", info);
	sense_data s;
	s.eom = true;
	s.info = info;
	report_condition(SK_NO_SENSE, SKC_BEGINNING_OF_PARTITION_MEDIUM_DETECTED, &s);
}

void nscsi_full_device::report_ew(const s32 info)
{
	LOG("    EW info=%d\n", info);
	sense_data s;
	s.eom = true;
	s.info = info;
	report_condition(SK_NO_SENSE, SKC_END_OF_PARTITION_MEDIUM_DETECTED, &s);
}

void nscsi_full_device::report_eod(const s32 info, const bool eom)
{
	LOG("    *** EOD info=%d\n", info);
	sense_data s;
	s.eom = eom;
	s.info = info;
	report_condition(SK_BLANK_CHECK, SKC_END_OF_DATA_DETECTED, &s);
}

void nscsi_full_device::report_eom(const bool write, const s32 info, const bool invalid)
{
	LOG("    *** EOM info=%d invalid=%d\n", info, invalid);
	sense_data s;
	s.invalid = invalid;
	s.eom = true;
	s.info = info;
	report_condition(write ? SK_VOLUME_OVERFLOW : SK_MEDIUM_ERROR, SKC_END_OF_PARTITION_MEDIUM_DETECTED, &s);
}

void nscsi_full_device::report_bad_len(const bool over, const s32 info)
{
	LOG("    *** %sLENGTH BLOCK info=%d\n", over ? "OVER" : "UNDER", info);
	sense_data s;
	s.bad_len = true;
	s.info = info;
	report_condition(SK_ILLEGAL_REQUEST, SKC_NO_ADDITIONAL_SENSE_INFORMATION, &s);
}

void nscsi_full_device::report_bad_cdb_field()
{
	LOG("    *** BAD CDB FIELD\n");
	report_condition(SK_ILLEGAL_REQUEST, SKC_INVALID_FIELD_IN_CDB);
}

void nscsi_full_device::report_bad_pl_field()
{
	LOG("    *** BAD PARAMETER LIST FIELD\n");
	report_condition(SK_ILLEGAL_REQUEST, SKC_INVALID_FIELD_IN_PARAMETER_LIST);
}

void nscsi_full_device::report_bad_pl_len()
{
	LOG("    *** BAD PARAMETER LIST LENGTH\n");
	report_condition(SK_ILLEGAL_REQUEST, SKC_PARAMETER_LIST_LENGTH_ERROR);
}

void nscsi_full_device::report_no_saving_params()
{
	LOG("    *** NO SAVING PARAMETERS\n");
	report_condition(SK_ILLEGAL_REQUEST, SKC_SAVING_PARAMETERS_NOT_SUPPORTED);
}

void nscsi_full_device::report_no_medium()
{
	LOG("    *** NO MEDIUM\n");
	report_condition(SK_NOT_READY, SKC_MEDIUM_NOT_PRESENT);
}

void nscsi_full_device::report_medium_changed()
{
	LOG("    MEDIUM CHANGED\n");
	report_condition(SK_UNIT_ATTENTION, SKC_NOT_READY_TO_READY_TRANSITION_MEDIUM_MAY_HAVE_CHANGED);
}

void nscsi_full_device::report_read_only()
{
	LOG("    *** READ ONLY\n");
	report_condition(SK_DATA_PROTECT, SKC_WRITE_PROTECTED);
}

void nscsi_full_device::report_read_failure()
{
	LOG("    *** READ FAILURE\n");
	report_condition(SK_MEDIUM_ERROR, SKC_UNRECOVERED_READ_ERROR);
}

void nscsi_full_device::report_write_failure()
{
	LOG("    *** WRITE FAILURE\n");
	report_condition(SK_MEDIUM_ERROR, SKC_WRITE_ERROR);
}

void nscsi_full_device::report_erase_failure()
{
	LOG("    *** ERASE FAILURE\n");
	report_condition(SK_MEDIUM_ERROR, SKC_ERASE_FAILURE);
}

//////////////////////////////////////////////////////////////////////////////

void nscsi_full_device::scsi_unknown_command()
{
	std::string txt = util::string_format("Unhandled command %s (%d):", command_names[m_scsi_cmdbuf[0]], m_scsi_cmdsize);
	for(int i=0; i != m_scsi_cmdsize; i++)
		txt += util::string_format(" %02x", m_scsi_cmdbuf[i]);
	LOGMASKED(LOG_UNSUPPORTED, "%s\n", txt);

	scsi_status_complete(SS_CHECK_CONDITION);
	sense(false, SK_ILLEGAL_REQUEST);
}

void nscsi_full_device::scsi_command()
{
	const u8 cmd = m_scsi_cmdbuf[0];
	const u8 lun = get_lun(m_scsi_cmdbuf[1] >> 5); // LUN may be overridden by IDENTIFY, per SCSI-2 section 7.2.2
	switch(cmd) {
	case SC_REZERO_UNIT:
		LOG("command REZERO UNIT\n");
		scsi_status_complete(SS_GOOD);
		break;
	case SC_REQUEST_SENSE:
		handle_request_sense(lun);
		break;
	default:
		scsi_unknown_command();
		break;
	}
}

void nscsi_full_device::handle_request_sense(const u8 lun) // mandatory; SCSI-2 section 8.2.14
{
	const u8 alloc_len = m_scsi_cmdbuf[4]; // allocation length
	LOG("command REQUEST SENSE lun=%d alloc_len=%d\n", lun, alloc_len);
	if ((m_scsi_cmdbuf[1] & 0x1f) || m_scsi_cmdbuf[2] || m_scsi_cmdbuf[3]) // error: reserved bits set
		return report_bad_cdb_field();

	assert(sizeof(m_scsi_sense_buffer) >= 18);
	scsi_data_in(SBUF_SENSE, std::min(18, (const int)alloc_len));
	scsi_status_complete(SS_GOOD);
}

void nscsi_full_device::scsi_message()
{
	if(m_scsi_cmdbuf[0] & 0x80) {
		m_scsi_identify = m_scsi_cmdbuf[0];
		return;
	}

	std::string txt = "Unknown message";
	for(int i=0; i != m_scsi_cmdsize; i++)
		txt += util::string_format(" %02x", m_scsi_cmdbuf[i]);
	LOGMASKED(LOG_UNSUPPORTED, "%s\n", txt);
}

int nscsi_full_device::get_lun(int def)
{
	if(m_scsi_identify & 0x80)
		// lower 3 bits contain LUNTRN
		return m_scsi_identify & 0x07;
	return def;
}

void nscsi_full_device::bad_lun()
{
	scsi_status_complete(SS_CHECK_CONDITION);
	sense(false, SK_ILLEGAL_REQUEST, SK_ASC_LOGICAL_UNIT_NOT_SUPPORTED);
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
	return attotime::from_nsec(400);
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

// Byte transfer rate (immediate)
attotime nscsi_full_device::scsi_data_byte_period()
{
	return attotime::zero;
}

// Command execution delay (immediate)
attotime nscsi_full_device::scsi_data_command_delay()
{
	return attotime::zero;
}
