// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_MACHINE_K573FPGA_H
#define MAME_MACHINE_K573FPGA_H

#pragma once

#include "sound/mas3507d.h"
#include "machine/ds2401.h"

DECLARE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device)

class k573fpga_device : public device_t
{
public:
	k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_ddrsbm_fpga(bool flag) { use_ddrsbm_fpga = flag; }

	void set_ram(u16 *v) { ram = v; }
	u16 get_decrypted();

	void set_crypto_key1(u16 v) { crypto_key1 = v; }
	void set_crypto_key2(u16 v) { crypto_key2 = v; }
	void set_crypto_key3(u8 v) { crypto_key3 = v; }
	u16 get_crypto_key1() const { return crypto_key1; }
	u16 get_crypto_key2() const { return crypto_key2; }
	u8 get_crypto_key3() const { return crypto_key3; }

	uint32_t get_mp3_cur_adr() { return mp3_cur_adr; }
	void set_mp3_cur_adr(u32 v) { mp3_cur_adr = v; }

	uint32_t get_mp3_end_adr() { return mp3_end_adr; }
	void set_mp3_end_adr(u32 v) { mp3_end_adr = v; }

	u16 i2c_read();
	void i2c_write(u16 data);

	u16 get_mpeg_ctrl();
	void set_mpeg_ctrl(u16 data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u16 *ram;

	u16 crypto_key1, crypto_key2;
	u8 crypto_key3;

	u32 mp3_cur_adr, mp3_end_adr, mpeg_ctrl_flag;
	bool use_ddrsbm_fpga;

	u16 decrypt_default(u16 data);
	u16 decrypt_ddrsbm(u16 data);
};

#endif // MAME_MACHINE_K573FPGA_H
