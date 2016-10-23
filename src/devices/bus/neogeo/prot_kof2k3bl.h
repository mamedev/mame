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
	kof2k3bl_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void kof2003_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void kof2003p_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t overlay_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bl_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void pl_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void upl_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	uint32_t get_bank_base() {return m_bank_base; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t m_overlay;
	uint32_t m_bank_base;
	uint16_t m_cartridge_ram[0x1000]; // bootlegs
};

#endif
