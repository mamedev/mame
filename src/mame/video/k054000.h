// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_K054000_H
#define MAME_VIDEO_K054000_H

#pragma once


#define MCFG_K054000_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K054000, 0)

class k054000_device : public device_t
{
public:
	k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k054000_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE16_MEMBER( lsb_w );
	DECLARE_READ16_MEMBER( lsb_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t    m_regs[0x20];
};

DECLARE_DEVICE_TYPE(K054000, k054000_device)

#endif // MAME_VIDEO_K054000_H
