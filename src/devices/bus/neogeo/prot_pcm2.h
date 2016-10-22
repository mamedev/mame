// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#pragma once

#ifndef __PCM2_PROT__
#define __PCM2_PROT__

extern const device_type PCM2_PROT;

#define MCFG_PCM2_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PCM2_PROT, 0)


class pcm2_prot_device :  public device_t
{
public:
	// construction/destruction
	pcm2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void decrypt(uint8_t* ymrom, uint32_t ymsize, int value);
	void swap(uint8_t* ymrom, uint32_t ymsize, int value);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
