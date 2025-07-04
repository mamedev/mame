// license:BSD-3-Clause
// copyright-holders:giulioz
#ifndef MAME_SOUND_ROLAND_SA_H
#define MAME_SOUND_ROLAND_SA_H

#pragma once

class roland_sa_device : public device_t, public device_sound_interface
{
public:
	roland_sa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_int_callback.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void load_roms(uint8_t *ic5, uint8_t *ic6, uint8_t *ic7);
	void set_sr_mode(bool mode);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned NUM_VOICES = 16;
	static constexpr unsigned PARTS_PER_VOICE = 10;

	uint16_t samples_exp[0x20000];
	bool samples_exp_sign[0x20000];
	uint16_t samples_delta[0x20000];
	bool samples_delta_sign[0x20000];

	uint32_t phase_exp_table[0x10000];
	uint16_t samples_exp_table[0x8000];

	struct SA_Part
	{
		uint32_t sub_phase;
		uint32_t env_value;
	};

	devcb_write_line m_int_callback;

	sound_stream *m_stream;                          // stream handle
	SA_Part m_parts[NUM_VOICES][PARTS_PER_VOICE];    // channel memory
	uint8_t m_ctrl_mem[0x2000];                      // RAM IC12 (as the CPU writes it)
	uint8_t m_irq_id;                                // voice/part that triggered the IRQ
	bool m_irq_triggered;                            // if there is an IRQ currently waiting
	bool m_sr_mode;                                  // sample rate mode (true = 20 KHz, false = 32 KHz)
};

DECLARE_DEVICE_TYPE(ROLAND_SA, roland_sa_device)

#endif // MAME_SOUND_ROLAND_SA_H
