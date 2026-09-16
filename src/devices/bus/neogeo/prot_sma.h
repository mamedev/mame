// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_SMA_H
#define MAME_BUS_NEOGEO_PROT_SMA_H

#pragma once


DECLARE_DEVICE_TYPE(NG_SMA_PROT, sma_prot_device)


class sma_prot_device : public device_t
{
public:
	// construction/destruction
	sma_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	//void kof99_bankswitch_w(uint16_t data);
	//void garou_bankswitch_w(uint16_t data);
	//void garouh_bankswitch_w(uint16_t data);
	//void mslug3_bankswitch_w(uint16_t data);
	//void kof2000_bankswitch_w(uint16_t data);
	uint16_t prot_9a37_r();
	uint16_t random_r();
	uint32_t kof99_bank_base(uint16_t sel);
	uint32_t garou_bank_base(uint16_t sel);
	uint32_t garouh_bank_base(uint16_t sel);
	uint32_t mslug3_bank_base(uint16_t sel);
	uint32_t mslug3a_bank_base(uint16_t sel);
	uint32_t kof2000_bank_base(uint16_t sel);
	void kof99_decrypt_68k(uint8_t* base);
	void garou_decrypt_68k(uint8_t* base);
	void garouh_decrypt_68k(uint8_t* base);
	void mslug3_decrypt_68k(uint8_t* base);
	void mslug3a_decrypt_68k(uint8_t* base);
	void kof2000_decrypt_68k(uint8_t* base);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t     m_sma_rng;
};

#endif // MAME_BUS_NEOGEO_PROT_SMA_H
