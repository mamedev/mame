// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_VIDEO_TOPCAT_H_
#define MAME_VIDEO_TOPCAT_H_

#pragma once

class topcat_device : public device_t
{
public:
	topcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	topcat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override;
	virtual void device_reset() override;

	DECLARE_READ8_MEMBER(address_r);
	DECLARE_WRITE8_MEMBER(address_w);
	DECLARE_READ8_MEMBER(register_r);
	DECLARE_WRITE8_MEMBER(register_w);
};

DECLARE_DEVICE_TYPE(TOPCAT, topcat_device)
#endif // MAME_VIDEO_TOPCAT_H_
