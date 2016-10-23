// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#pragma once

#ifndef __KOF98_PROT__
#define __KOF98_PROT__

extern const device_type KOF98_PROT;

#define MCFG_KOF98_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, KOF98_PROT, 0)


class kof98_prot_device :  public device_t
{
public:
	// construction/destruction
	kof98_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void decrypt_68k(uint8_t* cpurom, uint32_t cpurom_size);
	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	int m_prot_state;
	uint16_t m_default_rom[2];

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
