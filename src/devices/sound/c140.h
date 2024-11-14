// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* c140.h */

#ifndef MAME_SOUND_C140_H
#define MAME_SOUND_C140_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> c140_device
 // Verified from schematics (24 bit address, 12(16? for C219) bit data)

class c140_device : public device_t,
					public device_sound_interface,
					public device_rom_interface<25, 1, 0, ENDIANNESS_BIG>
{
public:
	c140_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto int1_callback() { return m_int1_callback.bind(); }

	u8 c140_r(offs_t offset);
	void c140_w(offs_t offset, u8 data);

	// little endian: Swap even and odd word
	u8 c140_le_r(offs_t offset) { return c140_r(offset ^ 1); }
	void c140_le_w(offs_t offset, u8 data) { c140_w(offset ^ 1, data);}

protected:
	c140_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void rom_bank_pre_change() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual int find_sample(int adrs, int bank, int voice);

	static constexpr unsigned MAX_VOICE = 24;

	struct C140_VOICE
	{
		C140_VOICE() { }

		s32    ptoffset     = 0;
		s32    pos          = 0;
		s32    key          = 0;
		//--work
		s32    lastdt       = 0;
		s32    prevdt       = 0;
		s32    dltdt        = 0;
		//--reg
		s32    rvol         = 0;
		s32    lvol         = 0;
		s32    frequency    = 0;
		s32    bank         = 0;
		s32    mode         = 0;

		s32    sample_start = 0;
		s32    sample_end   = 0;
		s32    sample_loop  = 0;
	};

	void init_voice(C140_VOICE *v);
	const inline bool ch_looped(C140_VOICE *v) { return BIT(v->mode, 4); } // shared as c140 and c219

	virtual const inline bool ch_mulaw(C140_VOICE *v) { return BIT(v->mode, 3); }
	// bit 6 used, unknown

	u8 keyon_status_read(u16 offset);

	TIMER_CALLBACK_MEMBER(int1_on);

	devcb_write_line m_int1_callback;

	int m_sample_rate;
	sound_stream *m_stream;
	/* internal buffers */
	std::unique_ptr<s16[]> m_mixer_buffer_left;
	std::unique_ptr<s16[]> m_mixer_buffer_right;

	int m_baserate;
	u8 m_REG[0x200];

	s16 m_pcmtbl[256];

	C140_VOICE m_voi[MAX_VOICE];

	emu_timer *m_int1_timer;
};

class c219_device : public c140_device
{
public:
	c219_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 c219_r(offs_t offset);
	void c219_w(offs_t offset, u8 data);

	// little endian: Swap even and odd word
	u8 c219_le_r(offs_t offset) { return c219_r(offset ^ 1); }
	void c219_le_w(offs_t offset, u8 data) { c219_w(offset ^ 1, data);}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual int find_sample(int adrs, int bank, int voice) override;

	virtual const inline bool ch_mulaw(C140_VOICE *v) override { return BIT(v->mode, 0); }
private:
	// bit 1 used, unknown
	const inline bool ch_noise(C140_VOICE *v) { return BIT(v->mode, 2); }
	const inline bool ch_inv_lout(C140_VOICE *v) { return BIT(v->mode, 3); }
	const inline bool ch_inv_sign(C140_VOICE *v) { return BIT(v->mode, 6); }
	u16 m_lfsr;
};

DECLARE_DEVICE_TYPE(C140, c140_device)
DECLARE_DEVICE_TYPE(C219, c219_device)

#endif // MAME_SOUND_C140_H
