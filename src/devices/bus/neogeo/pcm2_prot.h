// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


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
	pcm2_prot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void neo_pcm2_snk_1999(UINT8* ymrom, UINT32 ymsize, int value);
	void neo_pcm2_swap(UINT8* ymrom, UINT32 ymsize, int value);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
