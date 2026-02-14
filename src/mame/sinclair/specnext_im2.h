// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_IM2_H
#define MAME_SINCLAIR_SPECNEXT_IM2_H

#pragma once

#include "machine/z80daisy.h"

class specnext_im2_device : public device_t, public device_z80daisy_interface
{

public:
	specnext_im2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_callback() { return m_irq_cb.bind(); }

	void vector_w(u8 vector) { m_vector = vector; }
	void irq_w(int state);

	virtual int z80daisy_irq_state() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	devcb_write_line m_irq_cb;

private:
	u8 m_state;
	u8 m_vector;

};

DECLARE_DEVICE_TYPE(SPECNEXT_IM2, specnext_im2_device)

#endif // MAME_SINCLAIR_SPECNEXT_IM2_H
