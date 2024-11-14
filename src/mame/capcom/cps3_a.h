// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Capcom CPS-3 Sound Hardware

****************************************************************************/
#ifndef MAME_CAPCOM_CPS3_A_H
#define MAME_CAPCOM_CPS3_A_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct cps3_voice
{
	cps3_voice() :
		pos(0),
		frac(0)
	{
		memset(regs, 0, sizeof(uint32_t)*8);
	}

	uint32_t regs[8];
	uint32_t pos;
	uint32_t frac;
};

// ======================> cps3_sound_device

class cps3_sound_device : public device_t, public device_sound_interface
{
public:
	cps3_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~cps3_sound_device() { }

	void set_base(int8_t* base) { m_base = base; }

	void sound_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sound_r(offs_t offset, uint32_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream;
	cps3_voice m_voice[16];
	uint16_t     m_key;
	int8_t*      m_base;
};

DECLARE_DEVICE_TYPE(CPS3, cps3_sound_device)

#endif // MAME_CAPCOM_CPS3_A_H
