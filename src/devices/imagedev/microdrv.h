// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    microdrv.h

    MAME interface to the Sinclair Microdrive image abstraction code

*********************************************************************/

#ifndef MAME_IMAGEDEV_MICRODRV_H
#define MAME_IMAGEDEV_MICRODRV_H

#pragma once

#include "magtape.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> microdrive_image_device

class microdrive_image_device : public microtape_image_device
{
public:
	// construction/destruction
	microdrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~microdrive_image_device() override = default;

	auto comms_out_wr_callback() { return m_write_comms_out.bind(); }
	auto data1_out_wr_callback() { return m_write_data1_out.bind(); }
	auto data2_out_wr_callback() { return m_write_data2_out.bind(); }
	auto gap_out_wr_callback() { return m_write_gap_out.bind(); }
	auto tx_pair_rd_callback() { return m_read_tx_pair.bind(); }

	// device_image_interface implementation
	std::pair<std::error_condition, std::string> call_load() override;
	void call_unload() override;
	std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	bool is_creatable() const noexcept override { return true; }
	const char *image_interface() const noexcept override { return "ql_cass"; }
	const char *file_extensions() const noexcept override { return "mdv,mdr"; }

	// specific implementation
	void clk_w(int state);
	void comms_in_w(int state);
	void erase_w(int state);
	void read_write_w(int state);
	int data1_r() const;
	int data2_r() const;

protected:
	// device_t implementation
	void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(bit_timer);

private:
	void tape_from_image(const uint8_t *image);
	void tape_to_image(uint8_t *image) const;
	void save_image();

	devcb_write_line m_write_comms_out;
	devcb_write_line m_write_data1_out;
	devcb_write_line m_write_data2_out;
	devcb_write_line m_write_gap_out;
	devcb_read16 m_read_tx_pair;

	int m_clk;
	int m_comms_in;
	int m_comms_out;
	int m_erase;
	int m_read_write;

	// tape timeline: one entry per byte pair position
	std::unique_ptr<uint8_t[]> m_left;
	std::unique_ptr<uint8_t[]> m_right;
	std::unique_ptr<uint8_t[]> m_erased;

	int m_bit_offset;
	int m_byte_pair_offset;
	int m_gap_level;
	bool m_dirty;

	emu_timer *m_bit_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(MICRODRIVE, microdrive_image_device)

#endif // MAME_IMAGEDEV_MICRODRV_H
