// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_CR560B_H
#define MAME_MACHINE_CR560B_H

#pragma once

#include "imagedev/cdromimg.h"
#include "sound/cdda.h"


class cr560b_device : public cdrom_image_device, public device_mixer_interface
{
public:
	cr560b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto stch_cb() { return m_stch_cb.bind(); }
	auto sten_cb() { return m_sten_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }
	auto dten_cb() { return m_dten_cb.bind(); }
	auto scor_cb() { return m_scor_cb.bind(); }

	uint8_t read();
	void write(uint8_t data);

	void cmd_w(int state);
	void enable_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

private:
	uint32_t lba_to_msf(int32_t lba);
	int32_t msf_to_lba(uint32_t msf);
	int size_to_track_type();

	TIMER_CALLBACK_MEMBER(frame_cb);

	TIMER_CALLBACK_MEMBER(stch);
	void status_change(uint8_t status);

	TIMER_CALLBACK_MEMBER(sten);
	void status_enable(uint8_t output_length);

	void audio_end_cb(int state);

	// commands
	void cmd_seek();
	void cmd_motor_on();
	void cmd_motor_off();
//	void cmd_diag();
	void cmd_read_status();
	//void cmd_drawer_open();
	//void cmd_drawer_close();
	//void cmd_set_mode();
	//void cmd_reset();
	//void cmd_flush();
	//void cmd_lock();
	//void cmd_pause_resume();
	void cmd_play_msf();
	void cmd_play_track();
	void cmd_read();
	//void cmd_read_subq();

	void cmd_data_path_check();
	//void cmd_read_status();
	void cmd_read_error();
	void cmd_version();
	//void cmd_mode_sense();
	//void cmd_read_capacity();
	//void cmd_read_header();
	//void cmd_read_subq();
	//void cmd_read_upc();
	//void cmd_read_isrc();
	//void cmd_read_disc_code();
	//void cmd_read_disc_info();
	//void cmd_read_toc();
	//void cmd_read_session_info();
	//void cmd_read_device_driver();

	// drive status
	static constexpr uint8_t STATUS_DOOR_CLOSED = 0x80; // unverified, not used
	static constexpr uint8_t STATUS_MEDIA = 0x40;
	static constexpr uint8_t STATUS_MOTOR = 0x20;
	static constexpr uint8_t STATUS_ERROR = 0x10;
	static constexpr uint8_t STATUS_SUCCESS = 0x08; // last command has successfully executed
	static constexpr uint8_t STATUS_PLAYING = 0x04;
	static constexpr uint8_t STATUS_DOOR_LOCKED = 0x02; // unverified, not used
	static constexpr uint8_t STATUS_READY = 0x01;

	// audio status
	static constexpr uint8_t AUDIO_STATUS_INVALID = 0x00;
	static constexpr uint8_t AUDIO_STATUS_PLAY = 0x11;
	static constexpr uint8_t AUDIO_STATUS_PAUSED = 0x12;
	static constexpr uint8_t AUDIO_STATUS_COMPLETED = 0x13;
	static constexpr uint8_t AUDIO_STATUS_ERROR = 0x14;
	static constexpr uint8_t AUDIO_STATUS_NO_STATUS = 0x15;

	required_device<cdda_device> m_cdda;

	devcb_write_line m_stch_cb;
	devcb_write_line m_sten_cb;
	devcb_write_line m_drq_cb;
	devcb_write_line m_dten_cb;
	devcb_write_line m_scor_cb;

	emu_timer *m_frame_timer;
	emu_timer *m_stch_timer;
	emu_timer *m_sten_timer;

	uint8_t m_input_fifo[16];
	uint8_t m_input_fifo_pos;

	uint8_t m_output_fifo[16];
	uint8_t m_output_fifo_pos;
	uint8_t m_output_fifo_length;

	uint8_t m_status;
	uint16_t m_sector_size;

	uint32_t m_transfer_lba;
	uint16_t m_transfer_sectors;
	uint32_t m_transfer_length;
	uint8_t m_transfer_buffer[2352];
	uint16_t m_transfer_buffer_pos;

	// external lines
	bool m_enabled;
	bool m_cmd;

	bool m_status_ready;
	bool m_data_ready;
};

// device type declaration
DECLARE_DEVICE_TYPE(CR560B, cr560b_device)

#endif // MAME_MACHINE_CR560B_H
