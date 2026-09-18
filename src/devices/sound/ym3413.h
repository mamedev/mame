// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_YM3413_H
#define MAME_SOUND_YM3413_H

#pragma once

#include <memory>

class ym3413_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	ym3413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// external delay RAM size in 16-bit words (two byte-wide accesses per word)
	ym3413_device &set_ram_words(u32 words) { m_ram_words = words; return *this; }

	// control data port (CDI), one byte at a time
	void cd_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned STEPS = 64;

	// one step at which the serial input pair is handed to the program and the
	// delay address counter decrements (see the notes in the source file)
	static constexpr unsigned FRAME_STEP = 60;

	static constexpr unsigned COEF_FRAC = 7;
	static constexpr unsigned ACC_BITS = 26;

	void command(u8 cmd, u8 data);
	void run_frame(s16 in_l, s16 in_r);

	static s16 export_acc(s32 acc, bool shift);

	sound_stream *m_stream;
	u32 m_ram_words;
	u32 m_ram_mask;
	std::unique_ptr<s16 []> m_ram;

	// control data port
	u8 m_cd_buf[4];
	u8 m_cd_count;

	// registers visible from the control data port
	u8 m_control;       // command 00
	u8 m_mode;          // command 01
	u8 m_pointer;       // command 02
	u8 m_bank;          // command 03
	u8 m_level;         // command 06
	u8 m_micro[4][STEPS];
	s8 m_coef[STEPS];
	u16 m_addr[STEPS / 2];

	// datapath
	s16 m_r[8];         // register file loaded from delay memory / serial input
	s16 m_t[16];        // register file loaded from the accumulator
	s32 m_acc;
	s16 m_mdr;          // memory read data
	s16 m_mdr_next;
	u8 m_mdr_step;
	s16 m_out_bus;
	s16 m_out[2];
	s16 m_in[2];
	u16 m_counter;
};

DECLARE_DEVICE_TYPE(YM3413, ym3413_device)

#endif // MAME_SOUND_YM3413_H
