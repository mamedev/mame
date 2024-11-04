// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_BUS_GAMATE_PROTECTION_H
#define MAME_BUS_GAMATE_PROTECTION_H

#pragma once

DECLARE_DEVICE_TYPE(GAMATE_PROT, gamate_protection_device)

class gamate_protection_device : public device_t
{
public:
	// construction/destruction
	gamate_protection_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool is_protection_passed();

	void prot_w(int state);
	int prot_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_is_protection_passed;
	int m_has_failed;
	int m_passed_write;
	int m_inpos;
	uint8_t m_inbyte;
	uint8_t m_inseq;

	// this string is contained within the protection mapper, you must write it to unlock the cartridge
	const uint8_t m_prot_string[15] = { 0x42, 0x49, 0x54, 0x20, 0x43, 0x4F, 0x52, 0x50, 0x4F, 0x52, 0x41, 0x54, 0x49, 0x4F, 0x4E }; // "BIT CORPORATION"
};

#endif // MAME_BUS_GAMATE_PROTECTION_H

