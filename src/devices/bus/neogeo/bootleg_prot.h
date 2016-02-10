// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#pragma once

#include "banked_cart.h"

#ifndef __NGBOOTLEG_PROT__
#define __NGBOOTLEG_PROT__

extern const device_type NGBOOTLEG_PROT;

#define MCFG_NGBOOTLEG_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NGBOOTLEG_PROT, 0)


class ngbootleg_prot_device :  public device_t
{
public:
	// construction/destruction
	ngbootleg_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void neogeo_bootleg_cx_decrypt(UINT8*sprrom, UINT32 sprrom_size);
	void neogeo_bootleg_sx_decrypt(UINT8* fixed, UINT32 fixed_size, int value);
	void kof97oro_px_decode(UINT8* cpurom, UINT32 cpurom_size);
	void kof10thBankswitch(address_space &space, UINT16 nBank);
	DECLARE_READ16_MEMBER(kof10th_RAM2_r);
	DECLARE_READ16_MEMBER(kof10th_RAMB_r);
	DECLARE_WRITE16_MEMBER(kof10th_custom_w);
	DECLARE_WRITE16_MEMBER(kof10th_bankswitch_w);
	void install_kof10th_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev, UINT8* cpurom, UINT32 cpurom_size, UINT8* fixedrom, UINT32 fixedrom_size);
	void decrypt_kof10th(UINT8* cpurom, UINT32 cpurom_size);
	void kf10thep_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k5uni_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k5uni_sx_decrypt(UINT8* fixedrom, UINT32 fixedrom_size);
	void kf2k5uni_mx_decrypt(UINT8* audiorom, UINT32 audiorom_size);
	void decrypt_kf2k5uni(UINT8* cpurom, UINT32 cpurom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size);
	void kof2002b_gfx_decrypt(UINT8 *src, int size);
	void kf2k2mp_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k2mp2_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void cthd2003_neogeo_gfx_address_fix_do(UINT8* sprrom, UINT32 sprrom_size, int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift);
	void cthd2003_neogeo_gfx_address_fix(UINT8* sprrom, UINT32 sprrom_size, int start, int end);
	void cthd2003_c(UINT8* sprrom, UINT32 sprrom_size, int pow);
	void decrypt_cthd2003(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size);
	DECLARE_WRITE16_MEMBER(cthd2003_bankswitch_w);
	void patch_cthd2003(cpu_device* maincpu, neogeo_banked_cart_device* bankdev, UINT8* cpurom, UINT32 cpurom_size);
	void ct2k3sp_sx_decrypt(UINT8* fixedrom, UINT32 fixedrom_size);
	void decrypt_ct2k3sp(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size);
	void decrypt_ct2k3sa(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size);
	void patch_ct2k3sa(UINT8* cpurom, UINT32 cpurom_size);
	void decrypt_kof2k4se_68k(UINT8* cpurom, UINT32 cpurom_size);
	void lans2004_vx_decrypt(UINT8* ymsndrom, UINT32 ymsndrom_size);
	void lans2004_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size);
	DECLARE_READ16_MEMBER(mslug5_prot_r);
	DECLARE_WRITE16_MEMBER(ms5plus_bankswitch_w);
	void install_ms5plus_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	void svcboot_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcboot_cx_decrypt(UINT8*sprrom, UINT32 sprrom_size);
	void svcplus_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcplus_px_hack(UINT8* cpurom, UINT32 cpurom_size);
	void svcplusa_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcsplus_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void svcsplus_px_hack(UINT8* cpurom, UINT32 cpurom_size);
	DECLARE_READ16_MEMBER(kof2003_r);
	DECLARE_WRITE16_MEMBER(kof2003_w);
	DECLARE_WRITE16_MEMBER(kof2003p_w);
	DECLARE_READ16_MEMBER(kof2003_overlay_r);
	void kf2k3bl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k3bl_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev, UINT8* cpurom, UINT32 cpurom_size);
	void kf2k3pl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void kf2k3pl_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev, UINT8* cpurom, UINT32 cpurom_size);
	UINT16 kof2k3_overlay;

	void kf2k3upl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void samsho5b_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void samsho5b_vx_decrypt(UINT8* ymsndrom, UINT32 ymsndrom_size);
	void matrimbl_decrypt(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size);

	UINT16 m_cartridge_ram[0x1000]; // bootlegs

	// for kof10th
	UINT8* m_mainrom;
	UINT8* m_fixedrom;
	neogeo_banked_cart_device* m_bankdev;
	UINT16 m_cartridge_ram2[0x10000];

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
