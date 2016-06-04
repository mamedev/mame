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
	kof98_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void decrypt_68k(UINT8* cpurom, UINT32 cpurom_size);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_READ16_MEMBER(protection_r);
	int m_prot_state;
	UINT16 m_default_rom[2];

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
