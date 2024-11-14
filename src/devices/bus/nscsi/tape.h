// license:BSD-3-Clause
// copyright-holders:Mietek Bak

#ifndef MAME_BUS_NSCSI_TAPE_H
#define MAME_BUS_NSCSI_TAPE_H

#pragma once

#include "imagedev/simh_tape_image.h"
#include "machine/nscsi_bus.h"

DECLARE_DEVICE_TYPE(NSCSI_TAPE, nscsi_tape_device);

//////////////////////////////////////////////////////////////////////////////

class nscsi_tape_device : public nscsi_full_device
{
public:
	// construction
	nscsi_tape_device(const machine_config &config, const char *tag, device_t *owner, u32 clock = 0);

protected:
	nscsi_tape_device(const machine_config &config, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// nscsi_full_device implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void scsi_command() override;
	virtual u8 scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int id, int pos, u8 data) override;

	// command handling
	void handle_inquiry(const u8 lun);
	void handle_mode_select_6();
	void continue_handling_mode_select_6();
	void handle_mode_sense_6();
	void handle_send_diagnostic();
	void handle_test_unit_ready();
	void handle_prevent_allow_medium_removal();
	void handle_erase();
	void handle_load_unload();
	void handle_locate();
	void handle_read_6();
	void continue_handling_read_6();
	void handle_read_block_limits();
	void handle_read_position();
	void handle_release_unit();
	void handle_reserve_unit();
	void handle_rewind();
	void handle_space();
	void handle_write_6();
	void continue_handling_write_6();
	void handle_write_filemarks();

	// basic state
	required_device<simh_tape_image_device> m_image; // tape image
	u32 m_sequence_counter; // tape image identifier
	bool m_has_tape; // is tape image file available; cached value of m_image->get_file()
	bool m_tape_changed; // should we report medium changed next time we receive command
	u32 m_fixed_block_len; // fixed-length block length

	// state for READ and WRITE
	u32 m_rw_buf_size; // size of read/write buffer
	std::unique_ptr<u8[]> m_rw_buf; // read/write buffer
	bool m_rw_pending; // should we try to read/write additional blocks
	u32 m_rw_len; // length of valid data in read/write buffer
	u32 m_rw_blocks_num; // number of blocks read/written so far
	u32 m_rw_req_blocks_num; // requested number of blocks to read/write
	u32 m_rw_req_block_len; // requested block length
	bool m_rw_fixed_blocks; // should we read/write many fixed-length blocks instead of 1 variable-length block
	bool m_r_suppress_bad_len; // should we suppress indicating incorrect length for read blocks

	// state for MODE SELECT
	u8 m_pl_buf[256]; // parameter list buffer
	u32 m_pl_len; // length of valid data in parameter list buffer
};

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_BUS_NSCSI_TAPE_H
