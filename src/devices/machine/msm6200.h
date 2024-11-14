// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    OKI MSM6200 keyboard controller (HLE)
***************************************************************************/

#ifndef MAME_MACHINE_MSM6200_H
#define MAME_MACHINE_MSM6200_H

#pragma once

class msm6200_device : public device_t
{
public:
	msm6200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

	void write(offs_t offset, u8 data);
	u8 read();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	optional_ioport_array<38> m_keys;
	optional_ioport m_velocity;

	devcb_write_line m_irq_cb;

	u8 m_cmd, m_row, m_key_data;
	u8 m_key_state;
	u8 m_last_state[38];
};

// device type definition
DECLARE_DEVICE_TYPE(MSM6200, msm6200_device)

#endif // MAME_MACHINE_MSM6200_H
