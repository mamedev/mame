// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_MISC_H
#define MAME_BUS_NEOGEO_PROT_MISC_H

#pragma once

DECLARE_DEVICE_TYPE(NEOBOOT_PROT, neoboot_prot_device)


class neoboot_prot_device :  public device_t
{
public:
	// construction/destruction
	neoboot_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void cx_decrypt(uint8_t* sprrom, uint32_t sprrom_size);
	void sx_decrypt(uint8_t* fixed, uint32_t fixed_size, int value);

	void kof97oro_px_decode(uint8_t* cpurom, uint32_t cpurom_size);

	void kf10thep_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);

	void kf2k5uni_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void kf2k5uni_sx_decrypt(uint8_t* fixedrom, uint32_t fixedrom_size);
	void kf2k5uni_mx_decrypt(uint8_t* audiorom, uint32_t audiorom_size);

	void decrypt_kof2k4se_68k(uint8_t* cpurom, uint32_t cpurom_size);

	void lans2004_vx_decrypt(uint8_t* ymsndrom, uint32_t ymsndrom_size);
	void lans2004_decrypt_68k(uint8_t* cpurom, uint32_t cpurom_size);

	void samsho5b_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void samsho5b_vx_decrypt(uint8_t* ymsndrom, uint32_t ymsndrom_size);

	uint16_t mslug5p_prot_r();
	//void ms5plus_bankswitch_w(offs_t offset, uint16_t data);
	uint32_t mslug5p_bank_base(uint16_t sel);

	void mslug5b_vx_decrypt(uint8_t* ymsndrom, uint32_t ymsndrom_size);
	void mslug5b_cx_decrypt(uint8_t* sprrom, uint32_t sprrom_size);

	void kog_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);

	void svcboot_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void svcboot_cx_decrypt(uint8_t* sprrom, uint32_t sprrom_size);
	void svcplus_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void svcplus_px_hack(uint8_t* cpurom, uint32_t cpurom_size);
	void svcplusa_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void svcsplus_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void svcsplus_px_hack(uint8_t* cpurom, uint32_t cpurom_size);

	void kof2002b_gfx_decrypt(uint8_t *src, int size);
	void kf2k2mp_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void kf2k2mp2_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);

	void kof10th_decrypt(uint8_t* cpurom, uint32_t cpurom_size);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

#endif // MAME_BUS_NEOGEO_PROT_MISC_H
