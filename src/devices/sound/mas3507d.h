// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SOUND_MAS3507D_H
#define MAME_SOUND_MAS3507D_H

#pragma once

#include "mp3_audio.h"

class mas3507d_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	mas3507d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto mpeg_frame_sync_cb() { return cb_mpeg_frame_sync.bind(); }
	auto demand_cb() { return cb_demand.bind(); }

	int i2c_scl_r();
	int i2c_sda_r();
	void i2c_scl_w(bool line);
	void i2c_sda_w(bool line);

	void sid_w(uint8_t byte);

	void reset_playback();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void i2c_nak();
	bool i2c_device_got_address(uint8_t address);
	void i2c_device_got_byte(uint8_t byte);
	void i2c_device_got_stop();

	void mem_write(int bank, uint32_t adr, uint32_t val);
	void run_program(uint32_t adr);
	void reg_write(uint32_t adr, uint32_t val);

	void fill_buffer();
	void append_buffer(std::vector<write_stream_view> &outputs, int &pos, int scount);

	int gain_to_db(double val);
	float gain_to_percentage(int val);

	devcb_write_line cb_mpeg_frame_sync;
	devcb_write_line cb_demand;

	enum {
		CMD_DEV_WRITE = 0x3a,
		CMD_DEV_READ = 0x3b,

		CMD_DATA_WRITE = 0x68,
		CMD_DATA_READ = 0x69,
		CMD_CONTROL_WRITE = 0x6a
	};

	enum i2c_bus_state_t : uint8_t { IDLE = 0, STARTED, NAK, ACK, ACK2 };
	enum i2c_bus_address_t : uint8_t { UNKNOWN = 0, VALIDATED, WRONG };
	enum i2c_subdest_t : uint8_t { UNDEFINED = 0, CONTROL, DATA_READ, DATA_WRITE, BAD };
	enum i2c_command_t : uint8_t { CMD_BAD = 0, CMD_RUN, CMD_READ_CTRL, CMD_WRITE_REG, CMD_WRITE_MEM, CMD_READ_REG, CMD_READ_MEM };

	i2c_bus_state_t i2c_bus_state;
	i2c_bus_address_t i2c_bus_address;
	i2c_subdest_t i2c_subdest;
	i2c_command_t i2c_command;

	sound_stream *stream;

	std::array<uint8_t, 0xe00> mp3data;
	std::array<short, 1152*2> samples;

	bool i2c_scli, i2c_sclo, i2c_sdai, i2c_sdao;
	int i2c_bus_curbit;
	uint8_t i2c_bus_curval;
	int i2c_bytecount;
	uint32_t i2c_io_bank, i2c_io_adr, i2c_io_count, i2c_io_val;
	uint32_t i2c_sdao_data;

	uint32_t mp3data_count;
	uint32_t decoded_frame_count;
	int32_t sample_count, samples_idx;
	int32_t frame_channels;

	bool is_muted;
	float gain_ll, gain_rr;

	std::unique_ptr<mp3_audio> mp3dec;
};


// device type definition
DECLARE_DEVICE_TYPE(MAS3507D, mas3507d_device)

#endif // MAME_SOUND_MAS3507D_H
