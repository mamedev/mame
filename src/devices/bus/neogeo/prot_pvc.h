// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_PVC_H
#define MAME_BUS_NEOGEO_PROT_PVC_H

#pragma once

DECLARE_DEVICE_TYPE(NG_PVC_PROT, pvc_prot_device)


class pvc_prot_device :  public device_t
{
public:
	// construction/destruction
	pvc_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void pvc_write_unpack_color();
	void pvc_write_pack_color();
//  void pvc_write_bankswitch(address_space &space);
	uint32_t get_bank_base();
	uint16_t protection_r(offs_t offset);
	void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t m_cart_ram[0x1000];

	void mslug5_decrypt_68k(uint8_t* rom, uint32_t size);
	void svc_px_decrypt(uint8_t* rom, uint32_t size);
	void kf2k3pcb_decrypt_68k(uint8_t* rom, uint32_t size);
	void kof2003_decrypt_68k(uint8_t* rom, uint32_t size);
	void kof2003h_decrypt_68k(uint8_t* rom, uint32_t size);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

#endif // MAME_BUS_NEOGEO_PROT_PVC_H
