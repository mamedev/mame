// license:BSD-3-Clause
// copyright-holders:David Shah
/*****************************************************************************

  MAME/MESS VTxx APU CORE

	This core emulates the audio functionality of the V.R. Technology VTxx series
	of "enhanced NES" system on chips.

	The VTxx APU is effectively two NES APUs with added PCM functionality.

	APU 1 ("XOP1"), is a full-featured NES APU mapped at 0x4000-0x4013, and has
	full NES functionality plus a new PCM mode in addition to NES-style DPCM,
	with controls for that at 0x4030 and 0x4031.

	APU2 ("XOP2"), has all functions except DPCM/PCM and is mapped at
	0x4020-0x402F.

	Newer devices, from the VT09 (?) onwards add new banking registers for PCM,
	seperate from the CPU's PRG banking registers, located at 0x4125, 0x4126 (high
	address bank) and 0x4127, 0x4128 (relative high address bank).

	Even newer devices from the VT33 (??) onwards have a new two-channel PCM mode,
	limited info known. Possible map below, credit to NewRisingSun for reverse
	engineering this.
	0x4031,
	0x4032: select channel 1 or 2 respectively and set volume
	0x4033: bit 7: global enable, bit 4: ch1 enable, bit3: ch2 enable
	0x4012: set start address bits 13..6 of selected channel
	0x4035: set start address bits 20..14 of selected channel
	0x4036: set start address bits 24..21 of selected channel, and start playing
	Playback seems to be stopped by a 0xFF sample.
	This mode will be called enhanced PCM for lack of a better name
 *****************************************************************************/

#ifndef MAME_SOUND_NES_VT_APU_H
#define MAME_SOUND_NES_VT_APU_H

#pragma once

#include "nes_apu.h"

struct apu_vt_t {
	struct vt03_pcm_t
	{
		vt03_pcm_t()
		{
			for (auto & elem : regs)
				elem = 0;
		}

		float phaseacc = 0.0;
		uint8_t regs[4];
		uint32_t address = 0;
		uint32_t length = 0;
		uint32_t remaining_bytes = 0;
		bool enabled = false, irq_occurred = false;
		uint8_t vol = 0;
	};

	struct vt3x_pcm_t
	{
		vt3x_pcm_t()
		{
			for (auto & elem : regs)
				elem = 0;
		}

		uint8_t rate = 0;
		float phaseacc = 0.0;
		uint32_t address = 0;
		uint8_t volume = 0;
		bool enabled = false, playing = false;
		uint8_t regs[4];
		int8_t curr = 0;
	};

	uint8_t extra_regs[7]; //4030 .. 4037

	apu_vt_t()
	{
		for (auto & elem : extra_regs)
			elem = 0;
	}


	int vt3x_sel_channel = 0;
	vt03_pcm_t vt03_pcm;
	vt3x_pcm_t vt33_pcm[2];
	bool use_vt03_pcm = false, use_vt3x_pcm = false;
};

// This represents the "slave APU", which is modified to disable DPCM
class nesapu_vt_slave_device : public nesapu_device
{
public:
	nesapu_vt_slave_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

class nesapu_vt_device : public nesapu_device
{
public:
	nesapu_vt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void device_add_mconfig(machine_config &config) override;

	template <class Object> devcb_base &set_rom_read_callback(Object &&cb) { return m_rom_read_cb.set_callback(std::forward<Object>(cb)); }
	auto rom_read() { return m_rom_read_cb.bind(); }

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:

	virtual void device_start() override;


	uint8_t vt_apu_read(uint8_t address);
	void vt_apu_write(uint8_t address, uint8_t data);
	void vt_apu_regwrite(uint8_t address, uint8_t data);

	apu_vt_t m_apu_vt;

	required_device<nesapu_vt_slave_device> m_xop2;

	s8 vt03_pcm(apu_vt_t::vt03_pcm_t *ch);
	s8 vt3x_pcm(apu_vt_t::vt3x_pcm_t *ch);
	void reset_vt03_pcm(apu_vt_t::vt03_pcm_t *ch);
	void start_vt3x_pcm(apu_vt_t::vt3x_pcm_t *ch);
private:
	devcb_read8 m_rom_read_cb;

};

DECLARE_DEVICE_TYPE(NES_VT_APU, nesapu_vt_device)
DECLARE_DEVICE_TYPE(NES_VT_APU_SLAVE, nesapu_vt_slave_device)

#endif // MAME_SOUND_NES_VT_APU_H
