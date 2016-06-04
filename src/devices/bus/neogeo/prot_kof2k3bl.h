// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#pragma once

#ifndef __KOF2K3BL_PROT__
#define __KOF2K3BL_PROT__

extern const device_type KOF2K3BL_PROT;

#define MCFG_KOF2K3BL_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, KOF2K3BL_PROT, 0)


class kof2k3bl_prot_device :  public device_t
{
public:
	// construction/destruction
	kof2k3bl_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER(protection_r);
	DECLARE_WRITE16_MEMBER(kof2003_w);
	DECLARE_WRITE16_MEMBER(kof2003p_w);
	DECLARE_READ16_MEMBER(overlay_r);
	void bl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void pl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	void upl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	UINT32 get_bank_base() {return m_bank_base; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16 m_overlay;
	UINT32 m_bank_base;
	UINT16 m_cartridge_ram[0x1000]; // bootlegs
};

#endif
