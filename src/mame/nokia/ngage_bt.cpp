// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
/*
    CBUS Bluetooth controller, using the N-Gage QD study's transport and
    standalone Bluetooth 1.1 HCI model in ngage_bt_chip.h.
*/

#include "emu.h"
#include "ngage_bt.h"

#include "ngage_bt_chip.h"

DEFINE_DEVICE_TYPE(NGAGE_BT, ngage_bt_device, "ngage_bt", "N-Gage LRB-2 Bluetooth")

ngage_bt_device::ngage_bt_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NGAGE_BT, tag, owner, clock)
	, m_chip(std::make_unique<BtChip>())
	, m_irq_cb(*this)
{
}

ngage_bt_device::~ngage_bt_device() = default;

void ngage_bt_device::device_start()
{
	m_reset_timer = timer_alloc(FUNC(ngage_bt_device::reset_done), this);
	m_inquiry_timer = timer_alloc(FUNC(ngage_bt_device::inquiry_done), this);

	save_item(NAME(m_chip->in_reset));
	save_item(NAME(m_chip->events));
	save_item(NAME(m_chip->reg0));
	save_item(NAME(m_chip->reason));
	save_item(NAME(m_chip->tx));
	save_item(NAME(m_chip->tx_idx));
	save_item(NAME(m_chip->name_w));
	save_item(NAME(m_chip->par));
	save_item(NAME(m_chip->par_set));
	save_item(NAME(m_chip->name_len));
	save_item(NAME(m_chip->inq_run));
	save_item(NAME(m_chip->inq_done));
	save_item(NAME(m_chip->rep_pend));
	save_item(NAME(m_chip->rep_cs));
	save_item(NAME(m_chip->rep_status));
	save_item(NAME(m_chip->rep_src));
	save_item(NAME(m_chip->rep_idx));
	save_item(NAME(m_chip->rep_op));
	save_item(NAME(m_chip->rep_n));
	save_item(NAME(m_chip->rx_kind));
	save_item(NAME(m_chip->rx_len));
	save_item(NAME(m_chip->rx_idx));
}

void ngage_bt_device::device_reset()
{
	bt_chip_init(m_chip.get());
	m_reset_timer->adjust(attotime::never);
	m_inquiry_timer->adjust(attotime::never);
}

u16 ngage_bt_device::read(u8 reg)
{
	u16 const data = bt_chip_read(m_chip.get(), reg);
	update();
	return data;
}

void ngage_bt_device::write(u8 reg, u16 data)
{
	bt_chip_write(m_chip.get(), reg, data);
	update();
}

void ngage_bt_device::update()
{
	BtChip &chip = *m_chip;
	// The QD model uses a provisional 10 ms controller reset delay.
	// An inquiry length unit is 1.28 s in Bluetooth 1.1.
	if (chip.inq_stop)
		m_inquiry_timer->adjust(attotime::never);
	if (chip.inq_start)
		m_inquiry_timer->adjust(attotime::from_msec(1280 * chip.inq_start));
	if (chip.reset_req)
		m_reset_timer->adjust(attotime::from_msec(10));

	chip.inq_stop = chip.reset_req = chip.rx_new = false;
	chip.inq_start = chip.tx_done = 0;
	if (chip.irq)
	{
		chip.irq = false;
		m_irq_cb(ASSERT_LINE);
		m_irq_cb(CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(ngage_bt_device::reset_done)
{
	bt_chip_reset_done(m_chip.get());
	update();
}

TIMER_CALLBACK_MEMBER(ngage_bt_device::inquiry_done)
{
	bt_chip_inquiry_done(m_chip.get());
	update();
}
