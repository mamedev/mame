// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0360PRI_H
#define MAME_VIDEO_TC0360PRI_H

#pragma once

class tc0360pri_device : public device_t
{
public:
	tc0360pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t   m_regs[16];
};

DECLARE_DEVICE_TYPE(TC0360PRI, tc0360pri_device)

#define MCFG_TC0360PRI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0360PRI, 0)

#endif // MAME_VIDEO_TC0360PRI_H
