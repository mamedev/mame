// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_I82586_H
#define MAME_MACHINE_I82586_H

#pragma once

#define MCFG_I82586_IRQ_CB(_out_irq) \
	devcb = &i82586_base_device::static_set_out_irq_callback(*device, DEVCB_##_out_irq);

class i82586_base_device :
	public device_t,
	public device_memory_interface,
	public device_network_interface
{
public:
	template<class _Object> static devcb_base &static_set_out_irq_callback(device_t &device, _Object object) { return downcast<i82586_base_device &>(device).m_out_irq.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER(ca);

protected:
	i82586_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 datawidth, u8 addrwidth);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual std::vector<std::pair<int, const address_space_config *>> memory_space_config() const override;

	address_space_config m_space_config;
	address_space *m_space;

	devcb_write_line m_out_irq;
};

class i82586_device : public i82586_base_device
{
public:
	i82586_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

class i82596_base_device : public i82586_base_device
{
public:
	void port(u32 data); // cpu access interface

protected:
	i82596_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 datawidth, u8 addrwidth);

	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

class i82596sx_device : public i82596_base_device
{
public:
	i82596sx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

private:
};

class i82596dx_device : public i82596_base_device
{
public:
	i82596dx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

private:
};

DECLARE_DEVICE_TYPE(I82586, i82586_device)
DECLARE_DEVICE_TYPE(I82596SX, i82596sx_device)
DECLARE_DEVICE_TYPE(I82596DX, i82596dx_device)

#endif // MAME_MACHINE_I82586_H
