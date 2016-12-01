// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#pragma once

#ifndef __SMA_PROT__
#define __SMA_PROT__

extern const device_type SMA_PROT;

#define MCFG_SMA_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SMA_PROT, 0)


class sma_prot_device :  public device_t
{
public:
	// construction/destruction
	sma_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	//DECLARE_WRITE16_MEMBER( kof99_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( garou_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( garouh_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( mslug3_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( kof2000_bankswitch_w );
	DECLARE_READ16_MEMBER( prot_9a37_r );
	DECLARE_READ16_MEMBER( random_r );
	uint32_t kof99_bank_base(uint16_t sel);
	uint32_t garou_bank_base(uint16_t sel);
	uint32_t garouh_bank_base(uint16_t sel);
	uint32_t mslug3_bank_base(uint16_t sel);
	uint32_t kof2000_bank_base(uint16_t sel);
	void kof99_decrypt_68k(uint8_t* base);
	void garou_decrypt_68k(uint8_t* base);
	void garouh_decrypt_68k(uint8_t* base);
	void mslug3_decrypt_68k(uint8_t* base);
	void kof2000_decrypt_68k(uint8_t* base);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t     m_sma_rng;
};

#endif
