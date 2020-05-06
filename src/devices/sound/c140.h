// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* c140.h */

#ifndef MAME_SOUND_C140_H
#define MAME_SOUND_C140_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> c140_device

class c140_device : public device_t,
	public device_sound_interface,
	public device_rom_interface
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
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	virtual void rom_bank_updated() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	virtual inline bool is_c219() { return false; }
private:
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
	int find_sample(int adrs, int bank, int voice);

	TIMER_CALLBACK_MEMBER(int1_on);

	devcb_write_line m_int1_callback;

	int m_sample_rate;
	sound_stream *m_stream;
	/* internal buffers */
	std::unique_ptr<s16[]> m_mixer_buffer_left;
	std::unique_ptr<s16[]> m_mixer_buffer_right;

	int m_baserate;
	u8 m_REG[0x200];

	s16 m_pcmtbl[8];        //2000.06.26 CAB

	C140_VOICE m_voi[MAX_VOICE];

	emu_timer *m_int1_timer;
};

class c219_device : public c140_device
{
public:
	c219_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual inline bool is_c219() override { return true; }
};

DECLARE_DEVICE_TYPE(C140, c140_device)
DECLARE_DEVICE_TYPE(C219, c219_device)

#endif // MAME_SOUND_C140_H
