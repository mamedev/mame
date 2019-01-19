// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_PCM2_H
#define MAME_BUS_NEOGEO_PROT_PCM2_H

#pragma once


DECLARE_DEVICE_TYPE(NG_PCM2_PROT, pcm2_prot_device)


class pcm2_prot_device : public device_t
{
public:
	// construction/destruction
	pcm2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void decrypt(u8* ymrom, u32 ymsize, int value);
	void swap(u8* ymrom, u32 ymsize, int value);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // MAME_BUS_NEOGEO_PROT_PCM2_H
