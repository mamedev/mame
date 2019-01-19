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
	neoboot_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void cx_decrypt(u8* sprrom, u32 sprrom_size);
	void sx_decrypt(u8* fixed, u32 fixed_size, int value);

	void kof97oro_px_decode(u8* cpurom, u32 cpurom_size);

	void kf10thep_px_decrypt(u8* cpurom, u32 cpurom_size);

	void kf2k5uni_px_decrypt(u8* cpurom, u32 cpurom_size);
	void kf2k5uni_sx_decrypt(u8* fixedrom, u32 fixedrom_size);
	void kf2k5uni_mx_decrypt(u8* audiorom, u32 audiorom_size);

	void decrypt_kof2k4se_68k(u8* cpurom, u32 cpurom_size);

	void lans2004_vx_decrypt(u8* ymsndrom, u32 ymsndrom_size);
	void lans2004_decrypt_68k(u8* cpurom, u32 cpurom_size);

	void samsho5b_px_decrypt(u8* cpurom, u32 cpurom_size);
	void samsho5b_vx_decrypt(u8* ymsndrom, u32 ymsndrom_size);

	DECLARE_READ16_MEMBER(mslug5p_prot_r);
	//DECLARE_WRITE16_MEMBER(ms5plus_bankswitch_w);
	u32 mslug5p_bank_base(u16 sel);

	void kog_px_decrypt(u8* cpurom, u32 cpurom_size);

	void svcboot_px_decrypt(u8* cpurom, u32 cpurom_size);
	void svcboot_cx_decrypt(u8*sprrom, u32 sprrom_size);
	void svcplus_px_decrypt(u8* cpurom, u32 cpurom_size);
	void svcplus_px_hack(u8* cpurom, u32 cpurom_size);
	void svcplusa_px_decrypt(u8* cpurom, u32 cpurom_size);
	void svcsplus_px_decrypt(u8* cpurom, u32 cpurom_size);
	void svcsplus_px_hack(u8* cpurom, u32 cpurom_size);

	void kof2002b_gfx_decrypt(u8 *src, int size);
	void kf2k2mp_decrypt(u8* cpurom, u32 cpurom_size);
	void kf2k2mp2_px_decrypt(u8* cpurom, u32 cpurom_size);

	void kof10th_decrypt(u8* cpurom, u32 cpurom_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // MAME_BUS_NEOGEO_PROT_MISC_H
