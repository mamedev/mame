// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*******************************************************************************

    Tesla PMD 85 GPIO/0 connector

    Port A of the I/O board 8255 and its mode 2 handshake lines, used by the
    PMD 32 disk units.

*******************************************************************************/

#ifndef MAME_BUS_PMD85_GPIO_H
#define MAME_BUS_PMD85_GPIO_H

#pragma once


class device_pmd85_gpio_interface;

class pmd85_gpio_slot_device : public device_t, public device_single_card_slot_interface<device_pmd85_gpio_interface>
{
public:
	template <typename T>
	pmd85_gpio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: pmd85_gpio_slot_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	pmd85_gpio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// to the host 8255
	auto stb_callback() { return m_stb_cb.bind(); } // /STBA (PC4)
	auto ack_callback() { return m_ack_cb.bind(); } // /ACKA (PC6)

	// from the host 8255
	uint8_t pa_r();
	void pa_w(uint8_t data);
	void pc_w(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	friend class device_pmd85_gpio_interface;

	devcb_write_line m_stb_cb;
	devcb_write_line m_ack_cb;

	device_pmd85_gpio_interface *m_card;
};


class device_pmd85_gpio_interface : public device_interface
{
public:
	virtual ~device_pmd85_gpio_interface();

	// host 8255 port A output latch and port C
	virtual uint8_t pa_r() = 0;
	virtual void pa_w(uint8_t data) = 0;
	virtual void pc_w(uint8_t data) = 0;

protected:
	device_pmd85_gpio_interface(const machine_config &mconfig, device_t &device);

	// pulse the host 8255 /STBA or /ACKA
	void strobe_host();
	void acknowledge_host();

private:
	pmd85_gpio_slot_device *const m_slot;
};


DECLARE_DEVICE_TYPE(PMD85_GPIO_SLOT, pmd85_gpio_slot_device)

void pmd85_gpio_devices(device_slot_interface &device);

#endif // MAME_BUS_PMD85_GPIO_H
