// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_K054000_H
#define MAME_VIDEO_K054000_H

#pragma once


class k054000_device : public device_t
{
public:
	k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k054000_device() {}

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

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
