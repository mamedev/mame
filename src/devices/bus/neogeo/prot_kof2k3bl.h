// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_KOF2K3BL_H
#define MAME_BUS_NEOGEO_PROT_KOF2K3BL_H

#pragma once

DECLARE_DEVICE_TYPE(NG_KOF2K3BL_PROT, kof2k3bl_prot_device)


class kof2k3bl_prot_device :  public device_t
{
public:
	// construction/destruction
	kof2k3bl_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ16_MEMBER(protection_r);
	DECLARE_WRITE16_MEMBER(kof2003_w);
	DECLARE_WRITE16_MEMBER(kof2003p_w);
	DECLARE_READ16_MEMBER(overlay_r);
	void bl_px_decrypt(u8* cpurom, u32 cpurom_size);
	void pl_px_decrypt(u8* cpurom, u32 cpurom_size);
	void upl_px_decrypt(u8* cpurom, u32 cpurom_size);
	u32 get_bank_base() {return m_bank_base; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u16 m_overlay;
	u32 m_bank_base;
	u16 m_cartridge_ram[0x1000]; // bootlegs
};

#endif // MAME_BUS_NEOGEO_PROT_KOF2K3BL_H
