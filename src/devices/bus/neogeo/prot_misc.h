// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#pragma once

#ifndef __NEOBOOT_PROT__
#define __NEOBOOT_PROT__

extern const device_type NEOBOOT_PROT;

#define MCFG_NEOBOOT_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NEOBOOT_PROT, 0)


class neoboot_prot_device :  public device_t
{
public:
	// construction/destruction
	neoboot_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void cx_decrypt(UINT8* sprrom, UINT32 sprrom_size);
	void sx_decrypt(UINT8* fixed, UINT32 fixed_size, int value);

	void kof97oro_px_decode(UINT8* cpurom, UINT32 cpurom_size);

	void kf10thep_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);

	void kf2k5uni_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k5uni_sx_decrypt(UINT8* fixedrom, UINT32 fixedrom_size);
	void kf2k5uni_mx_decrypt(UINT8* audiorom, UINT32 audiorom_size);
	
	void decrypt_kof2k4se_68k(UINT8* cpurom, UINT32 cpurom_size);
	
	void lans2004_vx_decrypt(UINT8* ymsndrom, UINT32 ymsndrom_size);
	void lans2004_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size);
	
	void samsho5b_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void samsho5b_vx_decrypt(UINT8* ymsndrom, UINT32 ymsndrom_size);
	
	DECLARE_READ16_MEMBER(mslug5p_prot_r);
	//DECLARE_WRITE16_MEMBER(ms5plus_bankswitch_w);
	UINT32 mslug5p_bank_base(UINT16 sel);

	void kog_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	
	void svcboot_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcboot_cx_decrypt(UINT8*sprrom, UINT32 sprrom_size);
	void svcplus_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcplus_px_hack(UINT8* cpurom, UINT32 cpurom_size);
	void svcplusa_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcsplus_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcsplus_px_hack(UINT8* cpurom, UINT32 cpurom_size);

	void kof2002b_gfx_decrypt(UINT8 *src, int size);
	void kf2k2mp_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k2mp2_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);

	void kof10th_decrypt(UINT8* cpurom, UINT32 cpurom_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
