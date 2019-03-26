// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_RAD_EU3A05GPIO_H
#define MAME_MACHINE_RAD_EU3A05GPIO_H


class radica6502_gpio_device : public device_t
{
public:
	radica6502_gpio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_space_read0_cb.bind(); }
	auto read_1_callback() { return m_space_read1_cb.bind(); }
	auto read_2_callback() { return m_space_read2_cb.bind(); }

	DECLARE_READ8_MEMBER(gpio_r);
	DECLARE_WRITE8_MEMBER(gpio_w);

	DECLARE_WRITE8_MEMBER(gpio_unk_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read8 m_space_read0_cb;
	devcb_read8 m_space_read1_cb;
	devcb_read8 m_space_read2_cb;

	uint8_t read_port_data(int which);
	uint8_t read_direction(int which);
	void write_port_data(int which, uint8_t data);
	void write_direction(int which, uint8_t data);

	uint8_t m_ddr[3];
	uint8_t m_unk[3];
};

DECLARE_DEVICE_TYPE(RADICA6502_GPIO, radica6502_gpio_device)

#endif // MAME_MACHINE_RAD_EU3A05GPIO_H
