// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_MACHINE_DC7085_H
#define MAME_MACHINE_DC7085_H

#pragma once

class dc7085_device : public device_t
{
public:
	dc7085_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	auto int_cb() { return m_int_cb.bind(); }

protected:
	// standard device_interface overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line m_int_cb;

	u16 status_r();
	u16 rxbuffer_r();
	u16 txparams_r();
	u16 modem_status_r();
	void control_w(u16 data);
	void lineparams_w(u16 data);
	void txparams_w(u16 data);
	void txdata_w(u16 data);
private:
};

DECLARE_DEVICE_TYPE(DC7085, dc7085_device)

#endif // MAME_MACHINE_DC7085_H
