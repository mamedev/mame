// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko
#ifndef MAME_USSR_TIAMC1_A_H
#define MAME_USSR_TIAMC1_A_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiamc1_sound_device

class tiamc1_sound_device : public device_t, public device_sound_interface
{
public:
	tiamc1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void tiamc1_timer0_w(offs_t offset, uint8_t data);
	void tiamc1_timer1_w(offs_t offset, uint8_t data);
	void tiamc1_timer1_gate_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct timer8253chan
	{
		timer8253chan() { }

		uint16_t count = 0;
		uint16_t cnval = 0;
		uint8_t bcdMode = 0;
		uint8_t cntMode = 0;
		uint8_t valMode = 0;
		uint8_t gate = 0;
		uint8_t output = 0;
		uint8_t loadCnt = 0;
		uint8_t enable = 0;
	};

	struct timer8253struct
	{
		struct timer8253chan channel[3];
	};


	void timer8253_tick(struct timer8253struct *t,int chn);
	void timer8253_wr(struct timer8253struct *t, int reg, uint8_t val);
	char timer8253_get_output(struct timer8253struct *t, int chn);
	void timer8253_set_gate(struct timer8253struct *t, int chn, uint8_t gate);

	sound_stream *m_channel;
	int m_timer1_divider;

	timer8253struct m_timer0;
	timer8253struct m_timer1;
};

DECLARE_DEVICE_TYPE(TIAMC1, tiamc1_sound_device)

#endif // MAME_USSR_TIAMC1_A_H
