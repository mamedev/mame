// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EU3A05GPIO_H
#define MAME_TVGAMES_ELAN_EU3A05GPIO_H


class elan_eu3a05gpio_device : public device_t
{
public:
	elan_eu3a05gpio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto write_0_callback() { return m_write_0_callback.bind(); }
	auto write_1_callback() { return m_write_1_callback.bind(); }
	auto write_2_callback() { return m_write_2_callback.bind(); }
	auto read_0_callback() { return m_read_0_callback.bind(); }
	auto read_1_callback() { return m_read_1_callback.bind(); }
	auto read_2_callback() { return m_read_2_callback.bind(); }

	uint8_t gpio_r(offs_t offset);
	void gpio_w(offs_t offset, uint8_t data);

	void gpio_unk_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write8 m_write_0_callback;
	devcb_write8 m_write_1_callback;
	devcb_write8 m_write_2_callback;
	devcb_read8 m_read_0_callback;
	devcb_read8 m_read_1_callback;
	devcb_read8 m_read_2_callback;

	uint8_t read_port_data(int which);
	uint8_t read_direction(int which);
	void write_port_data(int which, uint8_t data);
	void write_direction(int which, uint8_t data);

	uint8_t m_ddr[3];
	uint8_t m_unk[3];
};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_GPIO, elan_eu3a05gpio_device)

#endif // MAME_TVGAMES_RAD_EU3A05GPIO_H
