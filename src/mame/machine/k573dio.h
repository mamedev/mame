// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef _K573DIO_H_
#define _K573DIO_H_

#include "sound/mas3507d.h"
#include "machine/ds2401.h"

#define MCFG_KONAMI_573_DIGITAL_IO_BOARD_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, KONAMI_573_DIGITAL_IO_BOARD, _clock)

#define MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( _output_cb )  \
	downcast<k573dio_device *>(device)->set_output_cb(DEVCB_##_output_cb);

class k573dio_device : public device_t
{
public:
	k573dio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _write> void set_output_cb(_write _output_cb)
	{
		output_cb.set_callback(_output_cb);
	}

	required_device<mas3507d_device> mas3507d;
	required_device<ds2401_device> digital_id;

	DECLARE_ADDRESS_MAP(amap, 16);

	uint16_t a00_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t a02_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t a04_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t a06_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t a0a_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t a80_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mpeg_start_adr_high_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_start_adr_low_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_end_adr_high_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_end_adr_low_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_key_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mas_i2c_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mas_i2c_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ram_write_adr_high_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ram_write_adr_low_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ram_read_adr_high_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ram_read_adr_low_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_7_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_key_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mpeg_key_3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t digital_id_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void digital_id_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t fpga_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fpga_firmware_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_5_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_write8 output_cb;

	std::unique_ptr<uint16_t[]> ram;
	uint32_t ram_adr;
	uint8_t output_data[8];

	void output(int offset, uint16_t data);
};

extern const device_type KONAMI_573_DIGITAL_IO_BOARD;

#endif
