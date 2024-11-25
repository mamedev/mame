// license:GPL-2.0+
// copyright-holders:Peter Trauner
#ifndef MAME_SOUND_LYNX_H
#define MAME_SOUND_LYNX_H

#pragma once


class lynx_sound_device : public device_t, public device_sound_interface
{
public:
	typedef device_delegate<void (void)> timer_delegate;

	lynx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	void count_down(int nr);
	template <typename... T> void set_timer_delegate(T &&... args) { m_timer_delegate.set(std::forward<T>(args)...); }

protected:
	struct LYNX_AUDIO {
		struct {
			//bool reset_done()     const { return BIT(control1, 6); }       // Reset timer done flag (not implemented)
			bool   integrate_mode() const { return BIT(control1, 5); }       // Integrate mode
			bool   reload_en()      const { return BIT(control1, 4); }       // Reload enable
			bool   count_en()       const { return BIT(control1, 3); }       // Count enable
			u8     timer_clock()    const { return BIT(control1, 0, 3); }    // Timer clock
			bool   linked()         const { return timer_clock() == 0b111; } // Linked timer?

			//bool last_clock()     const { return BIT(control2, 2); }       // Last clock (not implemented)
			//bool borrow_in()      const { return BIT(control2, 1); }       // Borrow in (not implemented)
			//bool borrow_out()     const { return BIT(control2, 0); }       // Borrow out (not implemented)

			s8 volume = 0;
			u8 feedback = 0;
			s8 output = 0;
			u8 shifter = 0;
			u8 bakup = 0;
			u8 control1 = 0;
			u8 counter = 0;
			u8 control2 = 0;
		} reg;
		u8 attenuation = 0;
		u16 mask = 0; // 12-bit
		u16 shifter = 0; // 12-bit
		s16 ticks = 0;
		s16 count = 0;
	};

	lynx_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	void reset_channel(LYNX_AUDIO *channel);
	void shift(int chan_nr);
	void execute(int chan_nr);
	void init();
	void register_save();

	sound_stream *m_mixer_channel;
	timer_delegate m_timer_delegate;   // this calls lynx_timer_count_down from the driver state

	std::unique_ptr<int[]> m_shift_mask;
	std::unique_ptr<int[]> m_shift_xor;
	u8 m_attenuation_enable = 0;
	u8 m_master_enable = 0;
	LYNX_AUDIO m_audio[4];
};


class lynx2_sound_device : public lynx_sound_device
{
public:
	lynx2_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
};


DECLARE_DEVICE_TYPE(LYNX_SND,  lynx_sound_device)
DECLARE_DEVICE_TYPE(LYNX2_SND, lynx2_sound_device)

#endif // MAME_SOUND_LYNX_H
