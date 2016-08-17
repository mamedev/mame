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
	cthd_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void fix_do(UINT8* sprrom, UINT32 sprrom_size, int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift);
	void gfx_address_fix(UINT8* sprrom, UINT32 sprrom_size, int start, int end);
	void cthd2003_c(UINT8* sprrom, UINT32 sprrom_size, int pow);

	void decrypt_cthd2003(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size);
	void patch_cthd2003(UINT8* cpurom, UINT32 cpurom_size);
	//DECLARE_WRITE16_MEMBER(cthd2003_bankswitch_w);
	UINT32 get_bank_base(UINT16 sel);

	void ct2k3sp_sx_decrypt(UINT8* fixedrom, UINT32 fixedrom_size);
	void decrypt_ct2k3sp(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size);

	void patch_ct2k3sa(UINT8* cpurom, UINT32 cpurom_size);
	void decrypt_ct2k3sa(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size);

	void matrimbl_decrypt(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
