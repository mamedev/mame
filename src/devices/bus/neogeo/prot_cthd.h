// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_CTHD_H
#define MAME_BUS_NEOGEO_PROT_CTHD_H

#pragma once

DECLARE_DEVICE_TYPE(NG_CTHD_PROT, cthd_prot_device)


class cthd_prot_device :  public device_t
{
public:
	// construction/destruction
	cthd_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void fix_do(u8* sprrom, u32 sprrom_size, int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift);
	void gfx_address_fix(u8* sprrom, u32 sprrom_size, int start, int end);
	void cthd2003_c(u8* sprrom, u32 sprrom_size, int pow);

	void decrypt_cthd2003(u8* sprrom, u32 sprrom_size, u8* audiorom, u32 audiorom_size, u8* fixedrom, u32 fixedrom_size);
	void patch_cthd2003(u8* cpurom, u32 cpurom_size);
	//DECLARE_WRITE16_MEMBER(cthd2003_bankswitch_w);
	u32 get_bank_base(u16 sel);

	void ct2k3sp_sx_decrypt(u8* fixedrom, u32 fixedrom_size);
	void decrypt_ct2k3sp(u8* sprrom, u32 sprrom_size, u8* audiorom, u32 audiorom_size, u8* fixedrom, u32 fixedrom_size);

	void patch_ct2k3sa(u8* cpurom, u32 cpurom_size);
	void decrypt_ct2k3sa(u8* sprrom, u32 sprrom_size, u8* audiorom, u32 audiorom_size);

	void matrimbl_decrypt(u8* sprrom, u32 sprrom_size, u8* audiorom, u32 audiorom_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // MAME_BUS_NEOGEO_PROT_CTHD_H
