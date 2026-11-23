// license:BSD-3-Clause
// copyright-holders:Kirk
#ifndef MAME_SKELETON_ALPHASMART_USB_SOCKET_H
#define MAME_SKELETON_ALPHASMART_USB_SOCKET_H

#include "alphasmart_usb.h"
#include "emuopts.h"
#include "osdfile.h"
#include "util/multibyte.h"

DECLARE_DEVICE_TYPE(ALPHASMART_USB, alphasmart_usb_device)

class alphasmart_usb_device : public device_t, public device_image_interface
{
public:
	alphasmart_usb_device(machine_config const &config, char const *tag, device_t *owner, uint32_t clock = 0)
		: device_t(config, ALPHASMART_USB, tag, owner, clock), device_image_interface(config, *this), m_irq_cb(*this) { }
	alphasmart_usb_chip chip;
	auto irq_callback() { return m_irq_cb.bind(); }
	void gpio_w(uint8_t direction) { chip.lines(!BIT(direction, 2), !BIT(direction, 1)); update_irq(); }
	// The firmware detects the bus-powered USB interface through its I2C
	// pull-ups. Leaving SDA high without VBUS falsely enters USB mode at boot.
	int sda_r() { return chip.vbus && chip.sda_r(); }
	int scl_r() { return chip.vbus; }
	int irq_r() { return chip.vbus && chip.irq_r(); }
	int suspend_r() { return !chip.vbus; }
	virtual char const *image_interface() const noexcept override { return "alphasmart_usb_socket"; }
	virtual char const *file_extensions() const noexcept override { return ""; }
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool core_opens_image_file() const noexcept override { return false; }
	virtual char const *image_type_name() const noexcept override { return "usb"; }
	virtual char const *image_brief_type_name() const noexcept override { return "usb"; }

protected:
	virtual void device_start() override
	{
		if (machine().options().verbose())
			chip.trace = [this](char kind, uint8_t value) { logerror("D11 I2C %c %02x\n", kind, value); };
		m_timer = timer_alloc(FUNC(alphasmart_usb_device::poll), this);
		m_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
	}
	virtual void device_reset() override { chip.reset(); }
	virtual std::pair<std::error_condition, std::string> call_load() override
	{
		if (std::string_view(filename()).substr(0, 7) != "socket.")
			return { image_error::INVALIDIMAGE, "Use socket.127.0.0.1:13240 for the USB packet listener" };
		return { listen(), "" };
	}
	virtual void call_unload() override { disconnected(); }

private:
	osd_file::ptr m_socket;
	devcb_write_line m_irq_cb;
	emu_timer *m_timer = nullptr;
	std::vector<uint8_t> m_rx, m_tx;
	bool m_seen_data = false;
	void update_irq()
	{
		// The SoC can clear its pending latch before firmware has serviced
		// the peripheral. Keep the level asserted until the D11 source clears.
		m_irq_cb(chip.vbus && !chip.irq_r());
	}
	std::error_condition listen()
	{
		uint64_t size;
		m_seen_data = false;
		return osd_file::open(filename(), OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, m_socket, size);
	}
	void disconnected()
	{
		m_socket.reset(); chip.vbus = false; m_rx.clear(); m_tx.clear();
		update_irq();
	}
	TIMER_CALLBACK_MEMBER(poll)
	{
		update_irq();
		if (chip.connected()) chip.frame = (chip.frame + 1) & 0x7ff;
		if (!exists()) return;
		if (!m_socket) { if (listen()) return; }
		uint8_t buffer[256]; uint32_t actual = 0;
		auto const err = m_socket->read(buffer, 0, sizeof(buffer), actual);
		if (err && err != std::errc::operation_would_block) { disconnected(); return; }
		if (!err && !actual && m_seen_data) { disconnected(); return; }
		if (actual) { m_seen_data = true; m_rx.insert(m_rx.end(), buffer, buffer + actual); }
		// D11P, sequence, operation (response: status), endpoint, payload length.
		while (m_rx.size() >= 20)
		{
			unsigned const length = get_u32be(&m_rx[16]);
			if (get_u32be(&m_rx[0]) != 0x44313150 || length > 8) { disconnected(); return; }
			if (m_rx.size() < 20 + length) break;
			unsigned const op = get_u32be(&m_rx[8]), endpoint = get_u32be(&m_rx[12]);
			uint8_t data[8]{};
			std::copy_n(m_rx.begin() + 20, length, data);
			unsigned count = length, status = 0;
			switch (op)
			{
			case 0: data[0] = chip.vbus; data[1] = chip.connected(); data[2] = chip.enable; data[3] = chip.address; count = 4; break;
			case 1: chip.vbus = true; chip.reset(true); count = 0; break;
			case 2: chip.reset(true); count = 0; break;
			case 3: case 4: case 5:
				if (op == 5) count = 8;
				status = chip.packet(endpoint, op == 5, op == 3, data, count);
				if (machine().options().verbose() && status != 1)
					logerror("D11 USB op=%u ep=%u len=%u status=%u irq=%04x\n", op, endpoint, count, status, chip.interrupts);
				if (op != 5 || status) count = 0;
				break;
			case 6: chip.vbus = false; count = 0; break;
			default: status = 3; count = 0; break;
			}
			update_irq();
			size_t const base = m_tx.size(); m_tx.resize(base + 20 + count);
			put_u32be(&m_tx[base], 0x44313150); put_u32be(&m_tx[base + 4], get_u32be(&m_rx[4]));
			put_u32be(&m_tx[base + 8], status); put_u32be(&m_tx[base + 12], endpoint); put_u32be(&m_tx[base + 16], count);
			std::copy_n(data, count, m_tx.begin() + base + 20);
			m_rx.erase(m_rx.begin(), m_rx.begin() + 20 + length);
		}
		if (!m_tx.empty())
		{
			actual = 0;
			auto const error = m_socket->write(m_tx.data(), 0, m_tx.size(), actual);
			if (error && error != std::errc::operation_would_block) { disconnected(); return; }
			m_tx.erase(m_tx.begin(), m_tx.begin() + actual);
		}
	}
};

DEFINE_DEVICE_TYPE(ALPHASMART_USB, alphasmart_usb_device, "alphasmart_usb", "PDIUSBD11 USB socket")

#endif
