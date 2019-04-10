// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_VIDEO_K051733_H
#define MAME_VIDEO_K051733_H

#pragma once


class k051733_device : public device_t
{
public:
	k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	uint8_t    m_ram[0x20];
	uint8_t    m_rng;
};

DECLARE_DEVICE_TYPE(K051733, k051733_device)

#endif // MAME_VIDEO_K051733_H
