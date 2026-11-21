// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
#ifndef MAME_NOKIA_NGAGE_BT_H
#define MAME_NOKIA_NGAGE_BT_H

#pragma once

#include <memory>

struct BtChip;

class ngage_bt_device : public device_t
{
public:
	ngage_bt_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~ngage_bt_device() override;

	auto irq_cb() { return m_irq_cb.bind(); }
	u16 read(u8 reg);
	void write(u8 reg, u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update();
	TIMER_CALLBACK_MEMBER(reset_done);
	TIMER_CALLBACK_MEMBER(inquiry_done);

	std::unique_ptr<BtChip> m_chip;
	devcb_write_line m_irq_cb;
	emu_timer *m_reset_timer = nullptr;
	emu_timer *m_inquiry_timer = nullptr;
};

DECLARE_DEVICE_TYPE(NGAGE_BT, ngage_bt_device)

#endif // MAME_NOKIA_NGAGE_BT_H
