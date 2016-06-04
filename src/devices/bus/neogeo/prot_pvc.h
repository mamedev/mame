// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#pragma once

#ifndef __PVC_PROT__
#define __PVC_PROT__

extern const device_type PVC_PROT;

#define MCFG_PVC_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PVC_PROT, 0)


class pvc_prot_device :  public device_t
{
public:
	// construction/destruction
	pvc_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void pvc_write_unpack_color();
	void pvc_write_pack_color();
//	void pvc_write_bankswitch(address_space &space);
	UINT32 get_bank_base();
	DECLARE_READ16_MEMBER(protection_r);
	DECLARE_WRITE16_MEMBER(protection_w);

	UINT16 m_cart_ram[0x1000];

	void mslug5_decrypt_68k(UINT8* rom, UINT32 size);
	void svc_px_decrypt(UINT8* rom, UINT32 size);
	void kf2k3pcb_decrypt_68k(UINT8* rom, UINT32 size);
	void kof2003_decrypt_68k(UINT8* rom, UINT32 size);
	void kof2003h_decrypt_68k(UINT8* rom, UINT32 size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
