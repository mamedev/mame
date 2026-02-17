// license:BSD-3-Clause
// copyright-holders:Mietek Bak

// best read together with SCSI-2 draft spec (rev 10L 7-SEP-93)
// https://www.staff.uni-mainz.de/tacke/scsi/SCSI2.html

#include "emu.h"

#include "tape.h"

#include "util/multibyte.h"
#include "util/tape_file_interface.h"

#include <algorithm>
#include <cassert>
#include <cstring>

// #define VERBOSE         LOG_GENERAL
// #define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace {

//////////////////////////////////////////////////////////////////////////////

// constants

static constexpr u32 TAPE_DEFAULT_FIXED_BLOCK_LEN = 512;

static constexpr int TAPE_RW_BUF_ID = 2;
static constexpr int TAPE_PL_BUF_ID = 3;


template <size_t N, typename T>
void clear_response(uint8_t (&buf)[N], T len)
{
	assert(std::size(buf) >= len);
	std::fill_n(buf, len, 0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE(NSCSI_TAPE, nscsi_tape_device, "scsi_tape", "SCSI tape");

//////////////////////////////////////////////////////////////////////////////

// construction

nscsi_tape_device::nscsi_tape_device(const machine_config &config, device_type type, const char *tag, device_t *owner, u32 clock)
	: nscsi_full_device(config, type, tag, owner, clock)
	, m_image(*this, "image")
	, m_sequence_counter(0)
	, m_has_tape(false)
	, m_tape_changed(false)
	, m_fixed_block_len(TAPE_DEFAULT_FIXED_BLOCK_LEN)
	, m_rw_buf_size(m_fixed_block_len)
	, m_rw_pending(false)
{
}

nscsi_tape_device::nscsi_tape_device(const machine_config &config, const char *tag, device_t *owner, u32 clock)
	: nscsi_tape_device(config, NSCSI_TAPE, tag, owner, clock)
{
}

//////////////////////////////////////////////////////////////////////////////

// device_t implementation

void nscsi_tape_device::device_add_mconfig(machine_config &config)
{
	SIMH_TAPE_IMAGE(config, m_image).set_interface("tape");
}

//////////////////////////////////////////////////////////////////////////////

// nscsi_full_device implementation

void nscsi_tape_device::device_start()
{
	nscsi_full_device::device_start();
	m_sequence_counter = m_image->sequence_counter();
	m_has_tape = m_image->get_file();
	m_rw_buf = std::make_unique<u8[]>(m_rw_buf_size);
	save_item(NAME(m_sequence_counter));
	save_item(NAME(m_has_tape));
	save_item(NAME(m_tape_changed));
	save_item(NAME(m_fixed_block_len));
	save_item(NAME(m_rw_buf_size));
	save_pointer(NAME(m_rw_buf), m_rw_buf_size);
	save_item(NAME(m_rw_pending));
	save_item(NAME(m_rw_len));
	save_item(NAME(m_rw_blocks_num));
	save_item(NAME(m_rw_req_blocks_num));
	save_item(NAME(m_rw_req_block_len));
	save_item(NAME(m_rw_fixed_blocks));
	save_item(NAME(m_r_suppress_bad_len));
	save_item(NAME(m_pl_buf));
	save_item(NAME(m_pl_len));
}

void nscsi_tape_device::device_reset()
{
	nscsi_full_device::device_reset();
	m_sequence_counter = m_image->sequence_counter();
	m_has_tape = m_image->get_file();
	m_tape_changed = false;
	m_fixed_block_len = TAPE_DEFAULT_FIXED_BLOCK_LEN;
	m_rw_pending = false;
}

void nscsi_tape_device::scsi_command()
{
	const u8 cmd = scsi_cmdbuf[0];
	const u8 lun = get_lun(scsi_cmdbuf[1] >> 5); // LUN may be overridden by IDENTIFY, per SCSI-2 section 7.2.2
	switch (cmd) { // these commands must be handled here, before tape changing logic
		case SC_INQUIRY:                        return handle_inquiry(lun);
		case SC_REQUEST_SENSE:                  return handle_request_sense(lun);
	}
	if (m_sequence_counter != m_image->sequence_counter()) { // tape just changed
		m_sequence_counter = m_image->sequence_counter();
		const bool had_tape = m_has_tape;
		m_has_tape = m_image->get_file();
		m_tape_changed = true;
		m_rw_pending = false;
		if (had_tape) { // we report no tape to let initiator know old tape has been removed
			LOG("command %s (ignored)\n", command_names[cmd], cmd);
			return report_no_medium();
		}
	}
	if (m_tape_changed) { // tape changed earlier
		m_tape_changed = false;
		if (m_has_tape) { // we report tape changed to let initiator know new tape has been inserted
			LOG("command %s (ignored)\n", command_names[cmd], cmd);
			return report_medium_changed();
		}
	}
	if (lun) // error: we don't support LUNs other than 0 for other commands
		return report_bad_lun(cmd, lun);

	assert(m_has_tape == (bool)m_image->get_file());
	switch (cmd) {
		case SC_MODE_SELECT_6:                  return handle_mode_select_6();
		case SC_MODE_SENSE_6:                   return handle_mode_sense_6();
		case SC_SEND_DIAGNOSTIC:                return handle_send_diagnostic();
		case SC_TEST_UNIT_READY:                return handle_test_unit_ready();
		case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:   return handle_prevent_allow_medium_removal();
		case SC_ERASE:                          return handle_erase();
		case SC_LOAD_UNLOAD:                    return handle_load_unload();
		case SC_LOCATE:                         return handle_locate();
		case SC_READ_6:                         return handle_read_6();
		case SC_READ_BLOCK_LIMITS:              return handle_read_block_limits();
		case SC_READ_POSITION:                  return handle_read_position();
		case SC_RELEASE_UNIT:                   return handle_release_unit();
		case SC_RESERVE_UNIT:                   return handle_reserve_unit();
		case SC_REWIND:                         return handle_rewind();
		case SC_SPACE:                          return handle_space();
		case SC_WRITE_6:                        return handle_write_6();
		case SC_WRITE_FILEMARKS:                return handle_write_filemarks();
		// TODO: support more optional commands
		case SC_CHANGE_DEFINITION:
		case SC_COMPARE:
		case SC_COPY:
		case SC_COPY_AND_VERIFY:
		case SC_LOG_SELECT:
		case SC_LOG_SENSE:
		case SC_MODE_SELECT_10:
		case SC_MODE_SENSE_10:
		case SC_READ_BUFFER:
		case SC_READ_REVERSE:
		case SC_RECEIVE_DIAGNOSTIC_RESULTS:
		case SC_RECOVER_BUFFERED_DATA:
		case SC_VERIFY_6:
		case SC_WRITE_BUFFER:
		default:
			nscsi_full_device::scsi_command();
	}
}

u8 nscsi_tape_device::scsi_get_data(int id, int pos)
{
	switch (id) {
		// we're handling READ(6); pos = 0 .. (m_rw_req_blocks_num * m_rw_req_block_len - 1)
		case TAPE_RW_BUF_ID: {
			const int relative_pos = pos % m_rw_req_block_len;
			if (m_rw_pending && relative_pos == 0) { // we need to fill buffer by reading another block
				assert(m_rw_len == 0);
				continue_handling_read_6();
			}
			if (m_rw_len == 0) // buffer is still empty, so reading block failed; condition has already been reported; all we can do here is pretend to read 0 from buffer
				return 0;

			m_rw_len--;
			return m_rw_buf[relative_pos];
		}
		default:
			return nscsi_full_device::scsi_get_data(id, pos);
	}
}

void nscsi_tape_device::scsi_put_data(int id, int pos, u8 data)
{
	switch (id) {
		// we're handling WRITE(6); pos == 0 .. (m_rw_req_blocks_num * m_rw_req_block_len - 1)
		case TAPE_RW_BUF_ID: {
			if (m_rw_len == m_rw_req_block_len) // buffer is still full, so writing block failed; condition has already been reported; all we can do is pretend to write to buffer
				return;

			const int relative_pos = pos % m_rw_req_block_len;
			m_rw_buf[relative_pos] = data;
			m_rw_len++;
			if (m_rw_pending && relative_pos == m_rw_req_block_len - 1) { // we need to empty buffer by writing another block
				assert(m_rw_len == m_rw_req_block_len);
				continue_handling_write_6();
			}
			return;
		}
		// we're handling MODE SELECT(6); pos == 0 .. (m_pl_len - 1)
		case TAPE_PL_BUF_ID:
			m_pl_buf[pos] = data;
			if (pos == m_pl_len - 1)
				continue_handling_mode_select_6();
			return;

		default:
			return nscsi_full_device::scsi_put_data(id, pos, data);
	}
}

//////////////////////////////////////////////////////////////////////////////

// command handling

void nscsi_tape_device::handle_inquiry(const u8 lun) // mandatory; SCSI-2 section 8.2.5
{
	const bool vpd_enable = scsi_cmdbuf[1] & 0x01; // should we respond with vital product data
	const u8 page_code = scsi_cmdbuf[2];
	const u8 alloc_len = scsi_cmdbuf[4]; // allocation length
	LOG("command INQUIRY lun=%d vpd_enable=%d page_code=0x%02x alloc_len=%d\n", lun, vpd_enable, page_code, alloc_len);
	if ((scsi_cmdbuf[1] & 0x1e) || scsi_cmdbuf[3]) // error: reserved bits set
		return report_bad_cdb_field();

	if (vpd_enable || page_code) // error: we don't support vital product data or pages other than 0
		return report_bad_cdb_field();

	clear_response(scsi_cmdbuf, 36);
	scsi_cmdbuf[0] = (lun == 0) ? 0x01 : 0x7f; // we support tape device on LUN 0 only, per 7.5.3(a)
	scsi_cmdbuf[1] = 0x80; // we support removing tape
	scsi_cmdbuf[2] = 0x02; // we're compliant with SCSI-2 only
	scsi_cmdbuf[3] = 0x02; // we use SCSI-2 response format
	scsi_cmdbuf[4] = 32; // additional length
	strncpy((char *)&scsi_cmdbuf[8], "MAME", 8); // vendor
	strncpy((char *)&scsi_cmdbuf[16], "SCSI tape drive", 16); // product
	strncpy((char *)&scsi_cmdbuf[32], "1.0", 4); // revision
	for (u32 i = 8; i < 36; i++) {
		if (scsi_cmdbuf[i] == 0)
			scsi_cmdbuf[i] = ' '; // pad strings with spaces
	}
	scsi_data_in(SBUF_MAIN, std::min(36, (const int)alloc_len));
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_mode_select_6() // mandatory; SCSI-2 sections 8.2.8, 8.3.3, 10.3.3
{
	const bool page_format = scsi_cmdbuf[1] & 0x10; // ignored; we always treat pages according to SCSI-2
	const bool save_pages = scsi_cmdbuf[1] & 0x01;
	const u8 pl_len = scsi_cmdbuf[4]; // parameter list length
	LOG("command MODE SELECT(6) page_format=%d save_pages=%d pl_len=%d\n", page_format, save_pages, pl_len);
	if ((scsi_cmdbuf[1] & 0x0e) || scsi_cmdbuf[2] || scsi_cmdbuf[3]) // error: reserved bits set
		return report_bad_cdb_field();

	if (save_pages) // error: we don't support saving pages
		return report_bad_cdb_field();

	if (pl_len == 0) // success: we have nothing to do
		return scsi_status_complete(SS_GOOD);

	m_pl_len = pl_len;
	scsi_data_out(TAPE_PL_BUF_ID, pl_len);
	// we continue in continue_handling_mode_select_6 through scsi_put_data
}

void nscsi_tape_device::continue_handling_mode_select_6()
{
	// this function is called by scsi_put_data when parameter list buffer is filled
	if (m_pl_len < 4) // error: truncated header
		return report_bad_pl_len();

	const u8 buf_mode = (m_pl_buf[2] & 0x70) >> 4; // ignored; we don't support buffering
	const u8 speed = m_pl_buf[2] & 0x0f; // ignored; we're always fast
	const u8 bd_len = m_pl_buf[3]; // block descriptor length
	LOG("    buf_mode=0x%02x speed=0x%02x bd_len=%d\n", buf_mode, speed, bd_len);
	if (m_pl_len < 4 + bd_len) // error: truncated block descriptor
		return report_bad_pl_len();

	if (bd_len && bd_len != 8) // error: we don't accept more than 1 block descriptor
		return report_bad_pl_field();

	if (bd_len == 8) { // 1 block descriptor
		const u8 density_code = m_pl_buf[4];
		const u32 blocks_num = get_u24be(&m_pl_buf[5]); // number of blocks
		const u32 block_len = get_u24be(&m_pl_buf[9]); // block length
		LOG("    density_code=0x%02x blocks_num=%d block_len=%d\n", density_code, blocks_num, block_len);
		if (density_code) // error: we don't support changing density
			return report_bad_pl_field();

		if (blocks_num) // error: we don't support changing block length for only some amount of blocks
			return report_bad_pl_field();

		if (block_len == 0) // error: requested block length is bad
			return report_bad_pl_field();

		m_fixed_block_len = block_len;
		if (m_fixed_block_len > m_rw_buf_size) {
			m_rw_buf_size = m_fixed_block_len;
			m_rw_buf = std::make_unique<u8[]>(m_rw_buf_size);
		}
	}
	u32 page_num = 0;
	u32 page_pos = 4 + bd_len;
	while (page_pos < m_pl_len) { // we accept any number of pages
		if (m_pl_len < page_pos + 2) // error: truncated page
			return report_bad_pl_len();

		const u8 page_code = m_pl_buf[page_pos] & 0x3f;
		const u8 page_len = m_pl_buf[page_pos + 1];
		LOG("    page_num=%d page_code=0x%02x page_len=%d\n", page_num, page_code, page_len);
		if (m_pl_len < page_pos + 2 + page_len) // error: truncated page
			return report_bad_pl_len();

		switch (page_code) {
			case SPC_MEDIUM_PARTITION_PAGE_1: { // TODO: test more software and see if these pages need to be accommodated
				if (page_len < 6 || page_len % 2) // error: malformed page
					return report_bad_pl_field();

				const u8 map = m_pl_buf[page_pos + 2]; // "maximum additional partitions"
				const u8 apd = m_pl_buf[page_pos + 3]; // "additional partitions defined"
				const bool fdp = m_pl_buf[page_pos + 4] & 0x80; // "fixed data partitions"
				const bool sdp = m_pl_buf[page_pos + 4] & 0x40; // "select data partitions"
				const bool idp = m_pl_buf[page_pos + 4] & 0x20; // "initiator-defined partitions"
				const u8 psum = (m_pl_buf[page_pos + 4] & 0x18) >> 3; // "partition size unit of measure"
				const u8 mfr = m_pl_buf[page_pos + 5]; // "medium format recognition"
				LOG("        map=%d apd=%d fdp=%d sdp=%d idp=%d psum=0x%x mfr=0x%02x\n", map, apd, fdp, sdp, idp, psum, mfr);
				u32 psd_num = 0; // partition size descriptor number
				u32 psd_pos = 8; // partition size descriptor position
				while (psd_pos < page_len) {
					const u16 ps = get_u16be(&m_pl_buf[page_pos + psd_pos]); // "partition size"
					LOG("        psd_num=%d ps=%d\n", psd_num, ps);
					psd_num++;
					psd_pos += 2;
				}
				break;
			}
			default:
				LOG("        *** BAD PAGE\n");
		}
		page_num++;
		page_pos += 2 + page_len;
	}
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_mode_sense_6() // mandatory; SCSI-2 sections 8.2.10, 8.3.3, 10.3.3
{
	const bool bd_disable = scsi_cmdbuf[1] & 0x80; // should we not respond with block descriptor
	const u8 page_control = scsi_cmdbuf[2] >> 6;
	const u8 page_code = scsi_cmdbuf[2] & 0x3f;
	LOG("command MODE SENSE(6) bd_disable=%d page_control=0x%02x page_code=0x%02x\n", bd_disable, page_control, page_code);
	if ((scsi_cmdbuf[1] & 0x17) || scsi_cmdbuf[3]) // error: reserved bits set
		return report_bad_cdb_field();

	if (page_code && page_code != SPC_RETURN_ALL_MODE_PAGES) // error: we don't support pages other than 0
		return report_bad_cdb_field();

	// TODO: support more pages, such as SPC_DATA_COMPRESSION_PAGE, and SPC_DEVICE_CONFIGURATION_PAGE with BIS bit set to indicate we support reporting block addresses in response to READ POSITION
	if (page_control == SPC_SAVED_VALUES) // error: we don't support saving parameters
		return report_no_saving_params();

	const u8 resp_len = bd_disable ? 4 : 12; // response length
	clear_response(scsi_cmdbuf, resp_len);
	scsi_cmdbuf[0] = resp_len - 1; // mode data length does not include itself, per SCSI-2 section 8.3.3
	scsi_cmdbuf[2] = (m_has_tape && m_image->get_file()->is_read_only()) ? 0x80 : 0; // device-specific parameter
	scsi_cmdbuf[3] = bd_disable ? 0 : 8; // block descriptor length
	if (!bd_disable) { // respond with block descriptor
		scsi_cmdbuf[4] = m_has_tape ? m_image->get_file()->get_density_code() : 0; // density code
		put_u24be(&scsi_cmdbuf[9], m_fixed_block_len); // block length
	}
	scsi_data_in(SBUF_MAIN, resp_len);
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_send_diagnostic() // mandatory; SCSI-2 section 8.2.15
{
	const bool page_format = scsi_cmdbuf[1] & 0x10;
	const bool self_test = scsi_cmdbuf[1] & 0x04;
	const bool device_offline = scsi_cmdbuf[1] & 0x02;
	const bool unit_offline = scsi_cmdbuf[1] & 0x01;
	const u16 pl_len = get_u16be(&scsi_cmdbuf[3]); // parameter list length
	LOG("command SEND DIAGNOSTIC page_format=%d self_test=%d device_offline=%d unit_offline=%d pl_len=%d\n", page_format, self_test, device_offline, unit_offline, pl_len);
	if ((scsi_cmdbuf[1] & 0x08) || scsi_cmdbuf[2]) // error: reserved bits set
		return report_bad_cdb_field();

	if (pl_len) // error: we don't support any parameters
		return report_bad_cdb_field();

	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_test_unit_ready() // mandatory; SCSI-2 section 8.2.16
{
	LOG("command TEST UNIT READY\n");
	if ((scsi_cmdbuf[1] & 0x1f) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4]) // error: reserved bits set
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_prevent_allow_medium_removal() // optional; SCSI-2 section 9.2.4
{
	const bool prevent = scsi_cmdbuf[4] & 0x01; // should we prevent or allow removing tape
	LOG("command %s MEDIUM REMOVAL\n", prevent ? "PREVENT" : "ALLOW");
	if ((scsi_cmdbuf[1] & 0x1f) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || (scsi_cmdbuf[4] & 0xfe)) // error: reserved bits set
		return report_bad_cdb_field();

	if (prevent) {
		if (!m_has_tape) // error: no tape
			return report_no_medium();

		// TODO: prevent removing tape, once MAME supports it
		return scsi_status_complete(SS_GOOD);
	}
	else { // allow
		// TODO: allow removing tape, once MAME supports it
		return scsi_status_complete(SS_GOOD);
	}
}

void nscsi_tape_device::handle_erase() // mandatory; SCSI-2 section 10.2.1
{
	const bool immed = scsi_cmdbuf[1] & 0x02; // ignored; we don't support buffering
	const bool eom = scsi_cmdbuf[1] & 0x01; // should we erase to EOM instead of 1 block; "long"
	LOG("command ERASE immed=%d eom=%d\n", immed, eom);
	if ((scsi_cmdbuf[1] & 0x1c) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4]) // error: reserved bits set
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	if (m_image->get_file()->is_read_only()) // error: tape is read-only
		return report_read_only();

	try {
		m_image->get_file()->erase(eom);
		return scsi_status_complete(SS_GOOD);
	}
	catch (std::exception &e) { // error: writing to tape failed
		osd_printf_error("ERASE FAILURE: nscsi_tape_device::handle_erase: %s\n", e.what());
		return report_erase_failure();
	}
}

void nscsi_tape_device::handle_load_unload() // optional; SCSI-2 section 10.2.2
{
	const bool immed = scsi_cmdbuf[1] & 0x01; // ignored; we don't support buffering
	const bool eom = scsi_cmdbuf[4] & 0x04; // should we rewind to EOM instead of BOM on unload
	const bool retension = scsi_cmdbuf[4] & 0x02; // ignored; we're always tense
	const bool load = scsi_cmdbuf[4] & 0x01; // should we load or unload tape
	LOG("command %s immed=%d eom=%d retension=%d\n", load ? "LOAD" : "UNLOAD", immed, eom, retension);
	if ((scsi_cmdbuf[1] & 0x1e) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || (scsi_cmdbuf[4] & 0xf8)) // error: reserved bits set
		return report_bad_cdb_field();

	if (load && eom) // error: invalid combination of command options
		return report_bad_cdb_field();

	if (load) {
		if (!m_has_tape) // error: no tape
			return report_no_medium();

		m_image->get_file()->rewind(false);
		return scsi_status_complete(SS_GOOD);
	}
	else { // unload
		if (m_has_tape) // rewind tape if it's there
			m_image->get_file()->rewind(eom);
		return scsi_status_complete(SS_GOOD);
	}
}

void nscsi_tape_device::handle_locate() // optional; SCSI-2 section 10.2.3
{
	const bool block_addr_type = scsi_cmdbuf[1] & 0x04; // ignored; we always use logical block addresses
	const bool change_partition = scsi_cmdbuf[1] & 0x02;
	const bool immed = scsi_cmdbuf[1] & 0x01; // ignored; we don't support buffering
	const u32 req_block_addr = get_u32be(&scsi_cmdbuf[3]); // requested block address to locate
	const u8 partition = scsi_cmdbuf[8];
	LOG("command LOCATE block_addr_type=%d change_partition=%d immed=%d req_block_addr=%d partition=0x%02x\n", block_addr_type, change_partition, immed, req_block_addr, partition);
	if ((scsi_cmdbuf[1] & 0x18) || scsi_cmdbuf[2] || scsi_cmdbuf[7]) // error: reserved bits set
		return report_bad_cdb_field();

	if (change_partition) // error: we don't support partitions other than 0
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	try {
		const auto status = m_image->get_file()->locate_block(req_block_addr);
		switch (status) {
			case tape_status::OK: // success: we located requested block
				return scsi_status_complete(SS_GOOD);

			case tape_status::EOD: // error: we reached EOD
			case tape_status::EOD_EW: // error: we reached EOD and we're also between EW and EOM
				return report_eod(0, status == tape_status::EOD_EW);

			case tape_status::EOM: // error: we reached EOM
				return report_eom(false);

			default:
				assert(false);
		}
	}
	catch (std::exception &e) { // error: reading from tape failed
		osd_printf_error("READ FAILURE: nscsi_tape_device::handle_locate: %s\n", e.what());
		return report_read_failure();
	}
}

void nscsi_tape_device::handle_read_6() // mandatory; SCSI-2 section 10.2.4
{
	const bool suppress = scsi_cmdbuf[1] & 0x02; // should we suppress indicating incorrect length
	const bool fixed = scsi_cmdbuf[1] & 0x01; // should we read many fixed-length blocks instead of 1 variable-length block
	const u32 req_items_num = get_u24be(&scsi_cmdbuf[2]); // requested number of blocks or bytes to read; "transfer length"
	LOG("command READ(6) suppress=%d fixed=%d req_items_num=%d\n", suppress, fixed, req_items_num);
	if (scsi_cmdbuf[1] & 0x1c) // error: reserved bits set
		return report_bad_cdb_field();

	if (fixed && suppress) // error: invalid combination of command options
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	if (req_items_num == 0) // success: we have nothing to do
		return scsi_status_complete(SS_GOOD);

	if (!fixed && req_items_num > m_rw_buf_size) {
		m_rw_buf_size = req_items_num;
		m_rw_buf = std::make_unique<u8[]>(m_rw_buf_size);
	}
	m_rw_pending = true;
	m_rw_len = 0;
	m_rw_blocks_num = 0;
	m_rw_req_blocks_num = fixed ? req_items_num : 1;
	m_rw_req_block_len = fixed ? m_fixed_block_len : req_items_num;
	m_rw_fixed_blocks = fixed;
	m_r_suppress_bad_len = suppress;
	assert(m_rw_req_block_len <= m_rw_buf_size);
	scsi_data_in(TAPE_RW_BUF_ID, m_rw_req_blocks_num * m_rw_req_block_len);
	// we continue in continue_handling_read_6 through scsi_get_data
}

void nscsi_tape_device::continue_handling_read_6()
{
	// this function is called by scsi_get_data every time read/write buffer is emptied
	assert(m_has_tape);
	assert(m_rw_len == 0);
	try {
		const auto result = m_image->get_file()->read_block(m_rw_buf.get(), m_rw_buf_size);
		const auto status = result.first;
		const u32 block_len = result.second;
		switch (status) {
			case tape_status::OK: { // success: we read another block
				const bool over = block_len > m_rw_req_block_len;
				const bool under = block_len < m_rw_req_block_len;
				if (over || (under && !m_r_suppress_bad_len)) { // error: we read block of incorrect length
					m_rw_pending = false;
					return report_bad_len(over, m_rw_fixed_blocks ? (m_rw_req_blocks_num - m_rw_blocks_num) : (m_rw_req_block_len - block_len));
				}
				if (under && m_r_suppress_bad_len)
					LOG("    *** UNDERLENGTH\n");
				m_rw_len = block_len;
				m_rw_blocks_num++;
				if (m_rw_blocks_num == m_rw_req_blocks_num) { // success: we're done
					m_rw_pending = false;
					return scsi_status_complete(SS_GOOD);
				}
				return; // we should read some more blocks
			}
			case tape_status::FILEMARK: // error: we reached filemark
			case tape_status::FILEMARK_EW: // error: we reached filemark and we're also between EW and EOM
				m_rw_pending = false;
				assert(block_len == 0);
				return report_filemark(m_rw_fixed_blocks ? (m_rw_req_blocks_num - m_rw_blocks_num) : m_rw_req_block_len, status == tape_status::FILEMARK_EW);

			case tape_status::EOD: // error: we reached EOD
			case tape_status::EOD_EW: // error: we reached EOD and we're also between EW and EOM
				m_rw_pending = false;
				assert(block_len == 0);
				return report_eod(m_rw_fixed_blocks ? (m_rw_req_blocks_num - m_rw_blocks_num) : m_rw_req_block_len, status == tape_status::EOD_EW);

			case tape_status::EOM: // error: we reached EOM
				m_rw_pending = false;
				assert(block_len == 0);
				return report_eom(false, m_rw_fixed_blocks ? (m_rw_req_blocks_num - m_rw_blocks_num) : m_rw_req_block_len);

			default:
				assert(false);
		}
	}
	catch (std::exception &e) { // error: reading from tape failed
		osd_printf_error("READ FAILURE: nscsi_tape_device::continue_handling_read_6: %s\n", e.what());
		return report_read_failure();
	}
}

void nscsi_tape_device::handle_read_block_limits() // mandatory; SCSI-2 section 10.2.5
{
	LOG("command READ BLOCK LIMITS\n");
	if ((scsi_cmdbuf[1] & 0x1f) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4]) // error: reserved bits set
		return report_bad_cdb_field();

	clear_response(scsi_cmdbuf, 6);
	put_u24be(&scsi_cmdbuf[1], 0xffffff); // 16MB; "maximum block length limit"
	put_u16be(&scsi_cmdbuf[4], m_fixed_block_len); // "minimum block length limit"
	scsi_data_in(SBUF_MAIN, 6);
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_read_position() // optional; SCSI-2 section 10.2.6
{
	const bool block_addr_type = scsi_cmdbuf[1] & 0x01; // ignored; we always use logical block addresses
	LOG("command READ POSITION block_addr_type=%d\n", block_addr_type);
	if ((scsi_cmdbuf[1] & 0x1e) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4] || scsi_cmdbuf[5] || scsi_cmdbuf[6] || scsi_cmdbuf[7] || scsi_cmdbuf[8]) // error: reserved bits set
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	try {
		const auto result = m_image->get_file()->read_position();
		const auto status = result.first;
		const u32 block_addr = result.second;
		switch (status) {
			case tape_status::OK: // we're right after some block (at EOD, or at filemark, or at another block); block address is valid
				LOG("    block_addr=%d\n", block_addr);
				break;
			case tape_status::BOM: // we're at BOM; block address is valid and 0
				LOG("    BOM block_addr=%d\n", block_addr);
				assert(block_addr == 0);
				break;
			case tape_status::EW: // we're between EW and EOM; block address is valid
				LOG("    EW block_addr=%d\n", block_addr);
				break;
			case tape_status::UNKNOWN: // we're after EOD or inside some block; block address is not valid
			case tape_status::UNKNOWN_EW: // we're after EOD or inside some block and also between EW and EOM; block address is not valid
				LOG("    UNKNOWN%s block_addr=%d\n", status == tape_status::UNKNOWN_EW ? " EW" : " ", block_addr);
				assert(block_addr == 0);
				break;
			case tape_status::EOM: // we're at EOM; block address is not valid
				LOG("    EOM block_addr=%d\n", block_addr);
				assert(block_addr == 0);
				break;
			default:
				assert(false);
		}
		const bool bom = status == tape_status::BOM;
		const bool eom = status == tape_status::EW
					  || status == tape_status::UNKNOWN_EW
					  || status == tape_status::EOM;
		const bool bpu = status == tape_status::UNKNOWN
					  || status == tape_status::UNKNOWN_EW
					  || status == tape_status::EOM;
		clear_response(scsi_cmdbuf, 20);
		scsi_cmdbuf[0] = (bom ? 0x80 : 0) // is position at BOM
					   | (eom ? 0x40 : 0) // is position between EW and EOM
					   | (bpu ? 0x04 : 0); // is next block address invalid; "block position unknown"
		put_u32be(&scsi_cmdbuf[4], block_addr); // address of next block to be read/written; "first block location"
		put_u32be(&scsi_cmdbuf[8], block_addr); // address of last buffered block; we don't support buffering, so we set it to next block address; "last block location"
		scsi_data_in(SBUF_MAIN, 20);
		scsi_status_complete(SS_GOOD);
	}
	catch (std::exception &e) { // error: reading from tape failed
		osd_printf_error("READ FAILURE: nscsi_tape_device::handle_read_position: %s\n", e.what());
		return report_read_failure();
	}
}

void nscsi_tape_device::handle_release_unit() // mandatory; SCSI-2 section 10.2.9
{
	const bool tpr = scsi_cmdbuf[1] & 0x10; // "third-party release"
	const u8 tpdi = (scsi_cmdbuf[1] & 0x0e) >> 1; // "third-party device id"
	LOG("command RELEASE UNIT tpr=%d tpdi=%d\n", tpr, tpdi);
	if ((scsi_cmdbuf[1] & 0x01) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4]) // error: reserved bits set
		return report_bad_cdb_field();

	// TODO: release unit, once MAME supports multiple initiators
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_reserve_unit() // mandatory; SCSI-2 section 10.2.10
{
	const bool tpr = scsi_cmdbuf[1] & 0x10; // "third-party release"
	const u8 tpdi = (scsi_cmdbuf[1] & 0x0e) >> 1; // "third-party device id"
	LOG("command RESERVE UNIT tpr=%d tpdi=%d\n", tpr, tpdi);
	if ((scsi_cmdbuf[1] & 0x01) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4]) // error: reserved bits set
		return report_bad_cdb_field();

	// TODO: reserve unit, once MAME supports multiple initiators
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_rewind() // mandatory; SCSI-2 section 10.2.11
{
	const bool immed = scsi_cmdbuf[1] & 0x01; // ignored; we don't support buffering
	LOG("command REWIND immed=%d\n", immed);
	if ((scsi_cmdbuf[1] & 0x1e) || scsi_cmdbuf[2] || scsi_cmdbuf[3] || scsi_cmdbuf[4]) // error: reserved bits set
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	m_image->get_file()->rewind(false);
	scsi_status_complete(SS_GOOD);
}

void nscsi_tape_device::handle_space() // mandatory; SCSI-2 section 10.2.12
{
	const u8 items_type = scsi_cmdbuf[1] & 0x07; // what type of items should we space over
	const u32 req_dir_items_num = get_s24be(&scsi_cmdbuf[2]); // requested direction and number of items to space over; "count"
	LOG("command SPACE items_type=0x%02x req_dir_items_num=%d\n", items_type, req_dir_items_num);
	if (scsi_cmdbuf[1] & 0x18) // error: reserved bits set
		return report_bad_cdb_field();

	if (items_type != STC_BLOCKS && items_type != STC_FILEMARKS && items_type != STC_END_OF_DATA) // error: we don't support spacing over setmarks or sequential filemarks
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	try {
		if (items_type == STC_END_OF_DATA) { // spacing EOD needs special handling
			const auto status = m_image->get_file()->space_eod();
			switch (status) {
				case tape_status::OK: // success: we reached EOD
					return scsi_status_complete(SS_GOOD);

				case tape_status::EOM: // error: we reached EOM
					return report_eom(false, 0, true);

				default:
					assert(false);
			}
		}
		if (req_dir_items_num == 0) // success: we have nothing to do
			return scsi_status_complete(SS_GOOD);

		const bool marks = items_type == STC_FILEMARKS;
		const bool reverse = req_dir_items_num < 0;
		const u32 req_items_num = reverse ? -req_dir_items_num : req_dir_items_num;
		const auto result = marks ? (reverse ? m_image->get_file()->space_filemarks_reverse(req_items_num)
											 : m_image->get_file()->space_filemarks(req_items_num))
								  : (reverse ? m_image->get_file()->space_blocks_reverse(req_items_num)
											 : m_image->get_file()->space_blocks(req_items_num));
		const auto status = result.first;
		const u32 items_num = result.second;
		switch (status) {
			case tape_status::OK: // success: we reached requested item
				assert(items_num == req_items_num);
				return scsi_status_complete(SS_GOOD);

			case tape_status::BOM: // error: we reached BOM
				return report_bom(req_items_num - items_num);

			case tape_status::FILEMARK: // error: we reached filemark
			case tape_status::FILEMARK_EW: // error: we reached filemark and we're also between EW and EOM
				return report_filemark(req_items_num - items_num, status == tape_status::FILEMARK_EW);

			case tape_status::EOD: // error: we reached EOD
			case tape_status::EOD_EW: // error: we reached EOD and we're also between EW and EOM
				return report_eod(req_items_num - items_num, status == tape_status::EOD_EW);

			case tape_status::EOM: // error: we reached EOM
				return report_eom(false, req_items_num - items_num);

			default:
				assert(false);
		}
	}
	catch (std::exception &e) { // error: reading from tape failed
		osd_printf_error("READ FAILURE: nscsi_tape_device::handle_space: %s\n", e.what());
		return report_read_failure();
	}
}

void nscsi_tape_device::handle_write_6() // mandatory; SCSI-2 section 10.2.14
{
	const bool fixed = scsi_cmdbuf[1] & 0x01; // should we write many fixed-length blocks instead of 1 variable-length block
	const u32 req_items_num = get_u24be(&scsi_cmdbuf[2]); // requested number of blocks or bytes to write; "transfer length"
	LOG("command WRITE(6) fixed=%d req_items_num=%d\n", fixed, req_items_num);
	if (scsi_cmdbuf[1] & 0x1e) // error: reserved bits set
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	if (m_image->get_file()->is_read_only()) // error: tape is read-only
		return report_read_only();

	if (req_items_num == 0) // success: we have nothing to do
		return scsi_status_complete(SS_GOOD);

	if (!fixed && req_items_num > m_rw_buf_size) {
		m_rw_buf_size = req_items_num;
		m_rw_buf = std::make_unique<u8[]>(m_rw_buf_size);
	}
	m_rw_pending = true;
	m_rw_len = 0;
	m_rw_blocks_num = 0;
	m_rw_req_blocks_num = fixed ? req_items_num : 1;
	m_rw_req_block_len = fixed ? m_fixed_block_len : req_items_num;
	m_rw_fixed_blocks = fixed;
	assert(m_rw_req_block_len <= m_rw_buf_size);
	scsi_data_out(TAPE_RW_BUF_ID, m_rw_req_blocks_num * m_rw_req_block_len);
	// we continue in continue_handling_write_6 through scsi_put_data
}

void nscsi_tape_device::continue_handling_write_6()
{
	// this function is called by scsi_put_data every time read/write buffer is filled
	assert(m_has_tape);
	assert(m_rw_len == m_rw_req_block_len);
	try {
		const auto status = m_image->get_file()->write_block(m_rw_buf.get(), m_rw_req_block_len);
		switch (status) {
			case tape_status::OK: // success: we wrote another block
				m_rw_len = 0;
				m_rw_blocks_num++;
				if (m_rw_blocks_num == m_rw_req_blocks_num) { // success: we're done
					m_rw_pending = false;
					return scsi_status_complete(SS_GOOD);
				}
				return; // we should write some more blocks

			case tape_status::EW: // success: we wrote another block, but we're between EW and EOM
				m_rw_len = 0;
				m_rw_blocks_num++;
				m_rw_pending = false;
				return report_ew(m_rw_fixed_blocks ? (m_rw_req_blocks_num - m_rw_blocks_num) : m_rw_req_block_len);

			case tape_status::EOM: // error: we reached EOM
				m_rw_pending = false;
				return report_eom(true, m_rw_fixed_blocks ? (m_rw_req_blocks_num - m_rw_blocks_num) : m_rw_req_block_len);

			default:
				assert(false);
		}
	}
	catch (std::exception &e) { // error: writing to tape failed
		osd_printf_error("WRITE FAILURE: nscsi_tape_device::continue_handling_write_6: %s\n", e.what());
		return report_write_failure();
	}
}

void nscsi_tape_device::handle_write_filemarks() // mandatory; SCSI-2 section 10.2.15
{
	const bool setmarks = scsi_cmdbuf[1] & 0x02; // should we write setmarks instead of filemarks
	const bool immed = scsi_cmdbuf[1] & 0x01; // ignored; we don't support buffering
	const u32 req_marks_num = get_u24be(&scsi_cmdbuf[2]); // requested number of marks to write; "transfer length"
	LOG("command WRITE FILEMARKS setmarks=%d immed=%d req_marks_num=%d\n", setmarks, immed, req_marks_num);
	if (scsi_cmdbuf[1] & 0x1c) // error: reserved bits set
		return report_bad_cdb_field();

	if (setmarks) // error: we don't support writing setmarks
		return report_bad_cdb_field();

	if (!m_has_tape) // error: no tape
		return report_no_medium();

	if (m_image->get_file()->is_read_only()) // error: tape is read-only
		return report_read_only();

	try {
		const auto status = m_image->get_file()->write_filemarks(req_marks_num);
		switch (status) {
			case tape_status::OK: // success: we wrote all filemarks
				return scsi_status_complete(SS_GOOD);

			case tape_status::EW: // success: we wrote all filemarks, but we're between EW and EOM
				return report_ew();

			case tape_status::EOM: // error: we reached EOM
				return report_eom(true, req_marks_num);

			default:
				assert(false);
		}
	}
	catch (std::exception &e) { // error: writing to tape failed
		osd_printf_error("WRITE FAILURE: nscsi_tape_device::handle_write_filemarks: %s\n", e.what());
		return report_write_failure();
	}
}

//////////////////////////////////////////////////////////////////////////////
