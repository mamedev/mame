// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Memory Card Reader
 *
 */
#ifndef MAME_MACHINE_K573_MCR_H
#define MAME_MACHINE_K573_MCR_H

#pragma once



DECLARE_DEVICE_TYPE(KONAMI_573_MEMORY_CARD_READER, k573mcr_device)

class k573mcr_device : public device_t
{
public:
	k573mcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;

	virtual const tiny_rom_entry *device_rom_region() const override;
};

#endif // MAME_MACHINE_K573_MCR_H
