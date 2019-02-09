// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_SOUND_8950INTF_H
#define MAME_SOUND_8950INTF_H

#pragma once


class y8950_device : public device_t,
	public device_sound_interface,
	public device_rom_interface
{
public:
	y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }
	auto keyboard_read() { return m_keyboard_read_handler.bind(); }
	auto keyboard_write() { return m_keyboard_write_handler.bind(); }
	auto io_read() { return m_io_read_handler.bind(); }
	auto io_write() { return m_io_write_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 status_port_r();
	u8 read_port_r();
	void control_port_w(u8 data);
	void write_port_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	virtual void rom_bank_updated() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void irq_handler(int irq);
	void timer_handler(int c, const attotime &period);
	void update_request() { m_stream->update(); }

	unsigned char port_handler_r() { return m_io_read_handler(0); }
	void port_handler_w(unsigned char data) { m_io_write_handler(offs_t(0), data); }
	unsigned char keyboard_handler_r() { return m_keyboard_read_handler(0); }
	void keyboard_handler_w(unsigned char data) { m_keyboard_write_handler(offs_t(0), data); }

	static uint8_t static_read_byte(device_t *param, offs_t offset) { return downcast<y8950_device *>(param)->read_byte(offset); }
	static void static_write_byte(device_t *param, offs_t offset, uint8_t data) { return downcast<y8950_device *>(param)->space().write_byte(offset, data); }

	static void static_irq_handler(device_t *param, int irq) { downcast<y8950_device *>(param)->irq_handler(irq); }
	static void static_timer_handler(device_t *param, int c, const attotime &period) { downcast<y8950_device *>(param)->timer_handler(c, period); }
	static void static_update_request(device_t *param, int interval) { downcast<y8950_device *>(param)->update_request(); }

	static unsigned char static_port_handler_r(device_t *param) { return downcast<y8950_device *>(param)->port_handler_r(); }
	static void static_port_handler_w(device_t *param, unsigned char data) { downcast<y8950_device *>(param)->port_handler_w(data); }
	static unsigned char static_keyboard_handler_r(device_t *param) { return downcast<y8950_device *>(param)->keyboard_handler_r(); }
	static void static_keyboard_handler_w(device_t *param, unsigned char data) { downcast<y8950_device *>(param)->keyboard_handler_w(data); }

	// internal state
	sound_stream *  m_stream;
	emu_timer *     m_timer[2];
	void *          m_chip;
	devcb_write_line m_irq_handler;
	devcb_read8 m_keyboard_read_handler;
	devcb_write8 m_keyboard_write_handler;
	devcb_read8 m_io_read_handler;
	devcb_write8 m_io_write_handler;
};

DECLARE_DEVICE_TYPE(Y8950, y8950_device)

#endif // MAME_SOUND_8950INTF_H
