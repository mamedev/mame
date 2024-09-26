// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************

    Konami 054539 PCM Sound Chip

*********************************************************/

#ifndef MAME_SOUND_K054539_H
#define MAME_SOUND_K054539_H

#pragma once

#include "dirom.h"

#define K054539_CB_MEMBER(_name)   void _name(double left, double right)

class k054539_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface<24>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // effector and/or some registers aren't verified/emulated

	// control flags, may be set at DRIVER_INIT().
	enum {
		RESET_FLAGS     = 0,
		REVERSE_STEREO  = 1,
		DISABLE_REVERB  = 2,
		UPDATE_AT_KEYON = 4
	};

	using apan_delegate = device_delegate<void (double left, double right)>;

	// construction/destruction
	k054539_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto timer_handler() { return m_timer_handler.bind(); }

	template <typename... T> void set_analog_callback(T &&... args) { m_apan_cb.set(std::forward<T>(args)...); }

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	void init_flags(int flags);

	/*
	  Note that the eight PCM channels of a K054539 do not have separate
	  volume controls. Considering the global attenuation equation may not
	  be entirely accurate, k054539_set_gain() provides means to control
	  channel gain. It can be called anywhere but preferably from
	  DRIVER_INIT().

	  Parameters:
	      channel : 0 - 7
	      gain    : 0.0=silent, 1.0=no gain, 2.0=twice as loud, etc.
	*/
	void set_gain(int channel, double gain);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	TIMER_CALLBACK_MEMBER(call_timer_handler);

private:
	struct channel {
		uint32_t pos;
		uint32_t pfrac;
		int32_t val;
		int32_t pval;
	};

	double voltab[256];
	double pantab[0xf];

	double gain[8];
	uint8_t posreg_latch[8][3];
	int flags;

	unsigned char regs[0x230];
	std::unique_ptr<uint8_t []> ram;
	int reverb_pos;

	int32_t cur_ptr;
	int cur_limit;
	uint32_t rom_addr;

	channel channels[8];
	sound_stream *stream;

	emu_timer          *m_timer;
	uint32_t             m_timer_state;
	devcb_write_line   m_timer_handler;
	apan_delegate m_apan_cb;

	bool regupdate();
	void keyon(int channel);
	void keyoff(int channel);
	void init_chip();
};

DECLARE_DEVICE_TYPE(K054539, k054539_device)

#endif // MAME_SOUND_K054539_H
