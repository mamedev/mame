// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_SOUND_ST0032_H
#define MAME_SOUND_ST0032_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> st0032_sound_device

class st0032_sound_device : public device_t,
					public device_sound_interface
{
public:
	st0032_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

public:
	void st0032_snd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t st0032_snd_r(offs_t offset);
	void st0032_sndctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t st0032_sndctrl_r();

private:
	static constexpr unsigned ST0032_VOICES = 16;

	sound_stream *m_stream;
	required_region_ptr<uint8_t> m_rom;
	uint16_t m_sound_regs[0x100];
	int m_vpos[ST0032_VOICES];
	int m_frac[ST0032_VOICES];
	int m_lponce[ST0032_VOICES];
	uint16_t m_ctrl;
};

DECLARE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device)

#endif // MAME_SOUND_ST0032_H
