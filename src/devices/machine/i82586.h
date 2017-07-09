// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_I82586_H
#define MAME_MACHINE_I82586_H

#pragma once

#define MCFG_I82586_IRQ_CB(_out_irq) \
	devcb = &i82586_base_device::static_set_out_irq_callback(*device, DEVCB_##_out_irq);

// callbacks for shared memory access
#define MCFG_I82586_SMA_CB(_sma_r, _sma_w) \
	devcb = &i82586_device::static_set_sma_r_callback(*device, DEVCB_##_sma_r); \
	devcb = &i82586_device::static_set_sma_w_callback(*device, DEVCB_##_sma_w);

#define MCFG_I82596_SMA_CB(_sma_r, _sma_w) \
	devcb = &i82596_device::static_set_sma_r_callback(*device, DEVCB_##_sma_r); \
	devcb = &i82596_device::static_set_sma_w_callback(*device, DEVCB_##_sma_w);

class i82586_base_device :
	public device_t,
	public device_network_interface
{
public:
	template<class _Object> static devcb_base &static_set_out_irq_callback(device_t &device, _Object object) { return downcast<i82586_base_device &>(device).m_out_irq.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER(ca);

protected:
	i82586_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line m_out_irq;
};

class i82586_device : public i82586_base_device
{
public:
	i82586_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &static_set_sma_r_callback(device_t &device, int channel, _Object object) { return downcast<i82586_device &>(device).m_sma_r.set_callback(object); }
	template<class _Object> static devcb_base &static_set_sma_w_callback(device_t &device, int channel, _Object object) { return downcast<i82586_device &>(device).m_sma_w.set_callback(object); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read16 m_sma_r;
	devcb_write16 m_sma_w;
};

class i82596_device : public i82586_base_device
{
public:
	i82596_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &static_set_sma_r_callback(device_t &device, int channel, _Object object) { return downcast<i82596_device &>(device).m_sma_r.set_callback(object); }
	template<class _Object> static devcb_base &static_set_sma_w_callback(device_t &device, int channel, _Object object) { return downcast<i82596_device &>(device).m_sma_w.set_callback(object); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read32 m_sma_r;
	devcb_write32 m_sma_w;
};

DECLARE_DEVICE_TYPE(I82586, i82586_device)
DECLARE_DEVICE_TYPE(I82596, i82596_device)

#endif
