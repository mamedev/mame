// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_EVIO_BOW_H
#define MAME_TVGAMES_XAVIX_IO_EVIO_BOW_H

#pragma once

#include "machine/timer.h"

class xavix_evio_bow_device : public device_t
{
public:
	xavix_evio_bow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto input_callback() { return m_input_cb.bind(); }
	u8 quadrature_r() const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 GRAY[4] = { 0, 2, 3, 1 };

	TIMER_CALLBACK_MEMBER(sample_input);
	TIMER_CALLBACK_MEMBER(step_phase);

	devcb_read8 m_input_cb;
	emu_timer *m_sample_timer;
	emu_timer *m_phase_timer;

	s16 m_pending;
	u8 m_last;
	u8 m_phase;
};

DECLARE_DEVICE_TYPE(XAVIX_EVIO_BOW, xavix_evio_bow_device)

#endif // MAME_TVGAMES_XAVIX_IO_EVIO_BOW_H
