// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#pragma once

#include "banked_cart.h"

#ifndef __PVC_PROT__
#define __PVC_PROT__

extern const device_type PVC_PROT;

#define MCFG_PVC_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PVC_PROT, 0)


class pvc_prot_device :  public device_t
{
public:
	// construction/destruction
	pvc_prot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void pvc_write_unpack_color();
	void pvc_write_pack_color();
	void pvc_write_bankswitch(address_space &space);
	DECLARE_READ16_MEMBER(pvc_prot_r);
	DECLARE_WRITE16_MEMBER(pvc_prot_w);
	void install_pvc_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	neogeo_banked_cart_device* m_bankdev;

	UINT16 m_cartridge_ram[0x1000];

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
