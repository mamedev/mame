// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#pragma once

#ifndef __CTHD_PROT__
#define __CTHD_PROT__

extern const device_type CTHD_PROT;

#define MCFG_CTHD_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CTHD_PROT, 0)


class cthd_prot_device :  public device_t
{
public:
	// construction/destruction
	cthd_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void fix_do(uint8_t* sprrom, uint32_t sprrom_size, int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift);
	void gfx_address_fix(uint8_t* sprrom, uint32_t sprrom_size, int start, int end);
	void cthd2003_c(uint8_t* sprrom, uint32_t sprrom_size, int pow);

	void decrypt_cthd2003(uint8_t* sprrom, uint32_t sprrom_size, uint8_t* audiorom, uint32_t audiorom_size, uint8_t* fixedrom, uint32_t fixedrom_size);
	void patch_cthd2003(uint8_t* cpurom, uint32_t cpurom_size);
	//void cthd2003_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t get_bank_base(uint16_t sel);

	void ct2k3sp_sx_decrypt(uint8_t* fixedrom, uint32_t fixedrom_size);
	void decrypt_ct2k3sp(uint8_t* sprrom, uint32_t sprrom_size, uint8_t* audiorom, uint32_t audiorom_size, uint8_t* fixedrom, uint32_t fixedrom_size);

	void patch_ct2k3sa(uint8_t* cpurom, uint32_t cpurom_size);
	void decrypt_ct2k3sa(uint8_t* sprrom, uint32_t sprrom_size, uint8_t* audiorom, uint32_t audiorom_size);

	void matrimbl_decrypt(uint8_t* sprrom, uint32_t sprrom_size, uint8_t* audiorom, uint32_t audiorom_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
