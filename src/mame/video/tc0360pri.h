// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0360PRI_H
#define MAME_VIDEO_TC0360PRI_H

#pragma once

class tc0360pri_device : public device_t
{
public:
	tc0360pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t   m_regs[16];
};

DECLARE_DEVICE_TYPE(TC0360PRI, tc0360pri_device)


#endif // MAME_VIDEO_TC0360PRI_H
