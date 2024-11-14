// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Multi Session Unit
 *
 */
#ifndef MAME_KONAMI_K573MSU_H
#define MAME_KONAMI_K573MSU_H

#pragma once



DECLARE_DEVICE_TYPE(KONAMI_573_MULTI_SESSION_UNIT, k573msu_device)

class k573msu_device : public device_t
{
public:
	k573msu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

#endif // MAME_KONAMI_K573MSU_H
