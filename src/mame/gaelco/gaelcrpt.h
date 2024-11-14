// license:BSD-3-Clause
// copyright-holders:Manuel Abadia

#ifndef MAME_GAELCO_GAELCRPT_H
#define MAME_GAELCO_GAELCRPT_H

#pragma once

DECLARE_DEVICE_TYPE(GAELCO_VRAM_ENCRYPTION, gaelco_vram_encryption_device)


class gaelco_vram_encryption_device :  public device_t
{
public:
	// construction/destruction
	gaelco_vram_encryption_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(uint8_t param1, uint16_t param2) { m_param1 = param1; m_param2 = param2; }

	uint16_t gaelco_decrypt(cpu_device &cpu, int offset, int data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int decrypt(int const enc_prev_word, int const dec_prev_word, int const enc_word);

	int32_t m_lastpc = 0, m_lastoffset = 0, m_lastencword = 0, m_lastdecword = 0;

	// config
	uint8_t m_param1;
	uint16_t m_param2;
};

#endif // MAME_GAELCO_GAELCRPT_H
