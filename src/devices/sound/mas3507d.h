// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SOUND_MAS3507D_H
#define MAME_SOUND_MAS3507D_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mas3507d_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	mas3507d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	int i2c_scl_r();
	int i2c_sda_r();
	void i2c_scl_w(bool line);
	void i2c_sda_w(bool line);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum { IDLE, STARTED, NAK, ACK, ACK2 } i2c_bus_state;
	enum { UNKNOWN, VALIDATED, WRONG } i2c_bus_address;

	bool i2c_scli, i2c_sclo, i2c_sdai, i2c_sdao;
	int i2c_bus_curbit;
	uint8_t i2c_bus_curval;

	void i2c_nak();
	bool i2c_device_got_address(uint8_t address);
	void i2c_device_got_byte(uint8_t byte);
	void i2c_device_got_stop();


	enum { UNDEFINED, CONTROL, DATA, BAD } i2c_subdest;
	enum { CMD_BAD, CMD_RUN, CMD_READ_CTRL, CMD_WRITE_REG, CMD_WRITE_MEM, CMD_READ_REG, CMD_READ_MEM } i2c_command;
	int i2c_bytecount;
	uint32_t i2c_io_bank, i2c_io_adr, i2c_io_count, i2c_io_val;


	void mem_write(int bank, uint32_t adr, uint32_t val);
	void run_program(uint32_t adr);
	void reg_write(uint32_t adr, uint32_t val);
};


// device type definition
DECLARE_DEVICE_TYPE(MAS3507D, mas3507d_device)

#endif // MAME_SOUND_MAS3507D_H
