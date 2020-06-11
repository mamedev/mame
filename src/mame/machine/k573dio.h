// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_K573DIO_H
#define MAME_MACHINE_K573DIO_H

#pragma once

#include "machine/k573fpga.h"
#include "machine/ds2401.h"

class k573dio_device : public device_t
{
public:
	k573dio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto output_callback() { return output_cb.bind(); }

	void amap(address_map &map);
	void set_ddrsbm_fpga(bool flag) { is_ddrsbm_fpga = flag; }

	uint16_t a00_r();
	uint16_t a02_r();
	uint16_t a04_r();
	uint16_t a06_r();
	uint16_t a0a_r();
	void a10_w(uint16_t data);
	uint16_t a80_r();
	uint16_t ac4_r();

	void mpeg_start_adr_high_w(uint16_t data);
	void mpeg_start_adr_low_w(uint16_t data);
	void mpeg_end_adr_high_w(uint16_t data);
	void mpeg_end_adr_low_w(uint16_t data);
	uint16_t mpeg_key_1_r();
	void mpeg_key_1_w(uint16_t data);
	uint16_t mas_i2c_r();
	void mas_i2c_w(uint16_t data);
	uint16_t mpeg_ctrl_r();
	void mpeg_ctrl_w(uint16_t data);
	void ram_write_adr_high_w(uint16_t data);
	void ram_write_adr_low_w(uint16_t data);
	uint16_t ram_r();
	void ram_w(uint16_t data);
	void ram_read_adr_high_w(uint16_t data);
	void ram_read_adr_low_w(uint16_t data);
	uint16_t mp3_frame_count_high_r();
	uint16_t mp3_frame_count_low_r();
	void output_0_w(uint16_t data);
	void output_1_w(uint16_t data);
	void output_7_w(uint16_t data);
	void output_3_w(uint16_t data);
	void mpeg_key_2_w(uint16_t data);
	void mpeg_key_3_w(uint16_t data);
	uint16_t digital_id_r();
	void digital_id_w(uint16_t data);
	uint16_t fpga_status_r();
	void fpga_firmware_w(uint16_t data);
	void output_4_w(uint16_t data);
	void output_2_w(uint16_t data);
	void output_5_w(uint16_t data);
	uint16_t mp3_unk_r();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<k573fpga_device> k573fpga;
	required_device<ds2401_device> digital_id;
	required_device<mas3507d_device> mas3507d;
	devcb_write8 output_cb;

	std::unique_ptr<uint16_t[]> ram;
	uint32_t ram_adr, ram_read_adr;
	uint8_t output_data[8];

	void output(int offset, uint16_t data);

	bool is_ddrsbm_fpga;
	u16 crypto_key1;
};

DECLARE_DEVICE_TYPE(KONAMI_573_DIGITAL_IO_BOARD, k573dio_device)

#endif // MAME_MACHINE_K573DIO_H
