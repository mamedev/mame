// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont, Chris Toshok
/*
 * ansi_disk.h - a Winchester disk drive on the ANSI device-level interface
 * (ANSI X3.101, "Interfaces between rigid disk drive(s) and host(s)")
 *
 * Derived from omti8621.cpp.
 */

#ifndef MAME_MACHINE_ANSI_DISK_H
#define MAME_MACHINE_ANSI_DISK_H

#pragma once

#include "imagedev/harddriv.h"

class ansi_disk_device : public harddisk_image_base_device
{
public:
	// a drive model: its attribute table's model ID and geometry, and the
	// spindle speed that paces its sector pulses
	struct model
	{
		const char *name;
		uint16_t model_id;
		uint16_t cylinders;
		uint8_t heads;
		uint16_t sectors; // sector pulses per track
		uint16_t rpm;
	};

	// device-level interface commands
	enum : uint8_t
	{
		CMD_REPORT_ILLEGAL_COMMAND     = 0x00,
		CMD_CLEAR_FAULT                = 0x01,
		CMD_CLEAR_ATTENTION            = 0x02,
		CMD_SEEK                       = 0x03,
		CMD_REZERO                     = 0x04,
		CMD_REPORT_SENSE_BYTE_2        = 0x0d,
		CMD_REPORT_SENSE_BYTE_1        = 0x0e,
		CMD_REPORT_GENERAL_STATUS      = 0x0f,

		CMD_REPORT_ATTRIBUTE           = 0x10,
		CMD_SET_ATTENTION              = 0x11,
		CMD_SELECTIVE_RESET            = 0x14,
		CMD_SEEK_TO_LANDING_ZONE       = 0x15,
		CMD_REFORMAT_TRACK             = 0x16,

		CMD_REPORT_CYL_ADDR_HIGH       = 0x29,
		CMD_REPORT_CYL_ADDR_LOW        = 0x2a,
		CMD_REPORT_READ_PERMIT_HIGH    = 0x2b,
		CMD_REPORT_READ_PERMIT_LOW     = 0x2c,
		CMD_REPORT_WRITE_PERMIT_HIGH   = 0x2d,
		CMD_REPORT_WRITE_PERMIT_LOW    = 0x2e,
		CMD_REPORT_TEST_BYTE           = 0x2f,

		CMD_ATTENTION_CONTROL          = 0x40,
		CMD_WRITE_CONTROL              = 0x41,
		CMD_LOAD_CYL_ADDR_HIGH         = 0x42,
		CMD_LOAD_CYL_ADDR_LOW          = 0x43,
		CMD_SELECT_HEAD                = 0x44,

		CMD_LOAD_ATTRIBUTE_NUMBER      = 0x50,
		CMD_LOAD_ATTRIBUTE             = 0x51,
		CMD_READ_CONTROL               = 0x53,
		CMD_OFFSET_CONTROL             = 0x54,
		CMD_SPIN_CONTROL               = 0x55,
		CMD_LOAD_SECT_PER_TRACK_HIGH   = 0x56, // MSB
		CMD_LOAD_SECT_PER_TRACK_MEDIUM = 0x57, // MedSB
		CMD_LOAD_SECT_PER_TRACK_LOW    = 0x58, // LSB
		CMD_LOAD_BYTES_PER_SECT_HIGH   = 0x59, // MSB
		CMD_LOAD_BYTES_PER_SECT_MEDIUM = 0x5a, // MedSB
		CMD_LOAD_BYTES_PER_SECT_LOW    = 0x5b, // LSB

		CMD_LOAD_READ_PERMIT_HIGH      = 0x6b,
		CMD_LOAD_READ_PERMIT_LOW       = 0x6c,
		CMD_LOAD_WRITE_PERMIT_HIGH     = 0x6d,
		CMD_LOAD_WRITE_PERMIT_LOW      = 0x6e,
		CMD_LOAD_TEST_BYTE             = 0x6f
	};

	typedef delegate<void (ansi_disk_device *, bool)> bool_cb;
	typedef delegate<void (ansi_disk_device *, uint8_t)> read_data_cb;
	typedef delegate<void (ansi_disk_device *)> pulse_cb;

	// construction/destruction
	ansi_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration: the bytes per sector the host formats (the drive is
	// hard-sectored, the controller decides what fits between two pulses)
	// and the image file extensions
	void set_sector_size(uint16_t bytes) { m_sector_size = bytes; }
	void set_extensions(const char *extensions) { m_extensions = extensions; }

	// image-level overrides
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return m_extensions; }
	virtual const char *image_type_name() const noexcept override { return "winchester"; }
	virtual const char *image_brief_type_name() const noexcept override { return "disk"; }

	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	void set_attention_cb(bool_cb cb);
	void set_busy_cb(bool_cb cb);
	void set_read_data_cb(read_data_cb cb);
	void set_ref_clock_tick_cb(pulse_cb cb);
	void set_index_pulse_cb(pulse_cb cb);
	void set_sector_pulse_cb(pulse_cb cb);

	// the read and write gates
	void assert_read_gate();
	void deassert_read_gate();

	void assert_write_gate();
	void write_sector_next_byte(uint8_t data);
	void deassert_write_gate();

	void select();

	uint8_t execute_command(uint8_t command, uint8_t parameter);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void select_model(const model &m);

private:
	emu_timer *m_time_dependent_timer;
	void start_time_dependent_command(attotime duration);
	TIMER_CALLBACK_MEMBER(finish_time_dependent_command);

	// the timer that drives the read_sector code
	emu_timer *m_read_timer;
	TIMER_CALLBACK_MEMBER(read_clock_tick);

	// the timer that drives the write_sector code
	emu_timer *m_ref_timer;
	TIMER_CALLBACK_MEMBER(ref_clock_tick);

	// our attention line
	bool m_attention;
	void set_attention_line(bool state);

	bool m_busy;
	void set_busy_line(bool state);

	bool_cb cur_attention_cb;
	bool_cb cur_busy_cb;
	read_data_cb cur_read_data_cb;
	pulse_cb cur_ref_clock_tick_cb;
	pulse_cb cur_index_pulse_cb;
	pulse_cb cur_sector_pulse_cb;

	bool m_selected;

	void clear_sb1(uint8_t value);

	uint16_t m_sector_size;
	const char *m_extensions;

	uint16_t m_model_id;
	uint16_t m_cylinders;
	uint16_t m_heads;
	uint16_t m_sectors;
	uint32_t m_sector_count;

	// these two cylinder bytes are for the actual current cylinder address
	uint8_t m_current_cylinder_high;
	uint8_t m_current_cylinder_low;
	// and these next two are the values loaded by the load_cylinder commands.
	// they will become the current ones above upon a successful seek.
	uint8_t m_load_cylinder_high;
	uint8_t m_load_cylinder_low;

	uint8_t m_attribute_number;
	uint8_t m_test_byte;

	uint8_t m_selected_head;
	uint8_t m_general_status;
	uint8_t m_sense_byte_1;
	uint8_t m_sense_byte_2;
	bool m_write_enabled;
	bool m_attention_enabled;

	// configuration data
	uint8_t report_attribute();
	void load_attribute(uint8_t attribute_value);

	bool m_attributes_initialized;
	uint8_t m_ansi_attributes[0x48];

	uint32_t m_cursor;
	std::unique_ptr<uint8_t []> m_buffer;

	attotime m_sector_clock_freq;
	emu_timer *m_sector_timer;
	TIMER_CALLBACK_MEMBER(sector_callback);
	int m_pulsed_sector;
};

DECLARE_DEVICE_TYPE(ANSI_DISK, ansi_disk_device)

#endif // MAME_MACHINE_ANSI_DISK_H
