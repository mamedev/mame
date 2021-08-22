// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMOPM_H
#define MAME_SOUND_YMOPM_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opm.h"


// ======================> ym2151_device

DECLARE_DEVICE_TYPE(YM2151, ym2151_device);

class ym2151_device : public ymfm_device_base<ymfm::ym2151>
{
	using parent = ymfm_device_base<ymfm::ym2151>;

public:
	// constructor
	ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers, handled by the interface
	auto port_write_handler() { return io_write_handler(); }

	// write access, handled by the chip implementation
	virtual void write(offs_t offset, u8 data) override;
	virtual void address_w(u8 data) override;
	virtual void data_w(u8 data) override;

	// reset line, active LOW
	DECLARE_WRITE_LINE_MEMBER(reset_w);

protected:
	// internal state
	uint8_t m_reset_state;           // reset line state
};


// ======================> ym2164_device

DECLARE_DEVICE_TYPE(YM2164, ym2164_device);

class ym2164_device : public ymfm_device_base<ymfm::ym2164>
{
public:
	// constructor
	ym2164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers, handled by the interface
	auto port_write_handler() { return io_write_handler(); }
};

#endif // MAME_SOUND_YMOPM_H
