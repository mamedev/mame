// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_SOUND_3812INTF_H
#define MAME_SOUND_3812INTF_H


class ym3812_device : public device_t, public device_sound_interface
{
public:
	ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 status_port_r();
	u8 read_port_r();
	void control_port_w(u8 data);
	void write_port_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum
	{
		TIMER_A,
		TIMER_B,
		TIMER_IRQ_SYNC
	};

	void irq_handler(int irq);
	void timer_handler(int c, const attotime &period);
	void update_request() { m_stream->update(); }

	void calculate_rates();

	static void static_irq_handler(device_t *param, int irq) { downcast<ym3812_device *>(param)->irq_handler(irq); }
	static void static_timer_handler(device_t *param, int c, const attotime &period) { downcast<ym3812_device *>(param)->timer_handler(c, period); }
	static void static_update_request(device_t *param, int interval) { downcast<ym3812_device *>(param)->update_request(); }

	sound_stream *  m_stream;
	emu_timer *     m_timer[3];
	void *          m_chip;
	devcb_write_line m_irq_handler;
};

DECLARE_DEVICE_TYPE(YM3812, ym3812_device)


#endif // MAME_SOUND_3812INTF_H
