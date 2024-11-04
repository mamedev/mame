// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_XAVIX_ANPORT_H
#define MAME_TVGAMES_XAVIX_ANPORT_H

class xavix_anport_device : public device_t
{
public:
	xavix_anport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_in0_cb.bind(); }
	auto read_1_callback() { return m_in1_cb.bind(); }
	auto read_2_callback() { return m_in2_cb.bind(); }
	auto read_3_callback() { return m_in3_cb.bind(); }

	uint8_t mouse_7b00_r();
	uint8_t mouse_7b01_r();
	uint8_t mouse_7b10_r();
	uint8_t mouse_7b11_r();

	void mouse_7b00_w(uint8_t data);
	void mouse_7b01_w(uint8_t data);
	void mouse_7b10_w(uint8_t data);
	void mouse_7b11_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8 m_in0_cb;
	devcb_read8 m_in1_cb;
	devcb_read8 m_in2_cb;
	devcb_read8 m_in3_cb;
};

DECLARE_DEVICE_TYPE(XAVIX_ANPORT, xavix_anport_device)

#endif // MAME_TVGAMES_XAVIX_ANPORT_H
