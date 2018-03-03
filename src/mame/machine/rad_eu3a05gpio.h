// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_AUDIO_RAD_EU3A05GPIO_H
#define MAME_AUDIO_RAD_EU3A05GPIO_H

#define MCFG_RADICA6502_GPIO_READ_PORT0_CB(_devcb) \
	devcb = &downcast<radica6502_gpio_device &>(*device).set_gpio_read_0_callback(DEVCB_##_devcb);

#define MCFG_RADICA6502_GPIO_READ_PORT1_CB(_devcb) \
	devcb = &downcast<radica6502_gpio_device &>(*device).set_gpio_read_1_callback(DEVCB_##_devcb);

#define MCFG_RADICA6502_GPIO_READ_PORT2_CB(_devcb) \
	devcb = &downcast<radica6502_gpio_device &>(*device).set_gpio_read_2_callback(DEVCB_##_devcb);


class radica6502_gpio_device : public device_t
{
public:
	radica6502_gpio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_gpio_read_0_callback(Object &&cb) { return m_space_read0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_gpio_read_1_callback(Object &&cb) { return m_space_read1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_gpio_read_2_callback(Object &&cb) { return m_space_read2_cb.set_callback(std::forward<Object>(cb)); }

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

#endif // MAME_AUDIO_RAD_EU3A05GPIO_H
