// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#ifndef MAME_SOUND_SEGAPCM_H
#define MAME_SOUND_SEGAPCM_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template<unsigned MaxVoices, unsigned Divider, unsigned AddrBits>
class segapcm_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface<AddrBits>
{
protected:
	static constexpr unsigned MAX_VOICES = MaxVoices;  // max voices
	static constexpr unsigned CLOCK_DIVIDER = Divider; // clock divider for generate output rate

	segapcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	virtual uint32_t get_bank(uint8_t ctrl) { return 0; }

	struct voice_t
	{
		void tick();

		segapcm_device<MaxVoices, Divider, AddrBits> *host;

		uint32_t addr = ~0; // current address (16.8 fixed point)
		uint16_t loop = ~0; // loop address
		uint8_t end = ~0;   // end address (bit 16-8 valid)
		uint8_t freq = ~0;  // frequency
		uint8_t lvol = ~0;  // left volume
		uint8_t rvol = ~0;  // right volume
		uint8_t ctrl = ~0;  // control bits
		int32_t lout = 0;   // left output
		int32_t rout = 0;   // right output
	};

	// read/write handlers
	uint8_t voice_addr_r(offs_t offset);
	uint8_t voice_loop_r(offs_t offset);
	uint8_t voice_end_r(offs_t offset);
	uint8_t voice_freq_r(offs_t offset);
	uint8_t voice_lvol_r(offs_t offset);
	uint8_t voice_rvol_r(offs_t offset);
	uint8_t voice_ctrl_r(offs_t offset);

	void voice_addr_w(offs_t offset, uint8_t data);
	void voice_loop_w(offs_t offset, uint8_t data);
	void voice_end_w(offs_t offset, uint8_t data);
	void voice_freq_w(offs_t offset, uint8_t data);
	void voice_lvol_w(offs_t offset, uint8_t data);
	void voice_rvol_w(offs_t offset, uint8_t data);
	void voice_ctrl_w(offs_t offset, uint8_t data);

	required_shared_ptr<uint8_t> m_ram;

private:
	voice_t m_voice[MaxVoices];
	sound_stream* m_stream;
};


class segapcm_discrete_device : public segapcm_device<8, 64, 16>
{
public:
	segapcm_discrete_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;
};

class sega_315_5218_device : public segapcm_device<16, 128, 21>
{
public:
	static constexpr int BANK_256    = 11;
	static constexpr int BANK_512    = 12;
	static constexpr int BANK_12M    = 13;
	static constexpr int BANK_MASK7  = 0x70 << 16;
	static constexpr int BANK_MASKF  = 0xf0 << 16;
	static constexpr int BANK_MASKF8 = 0xf8 << 16;

	sega_315_5218_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_bank(int bank) { m_bankshift = (bank & 0xf); m_bankmask = (0x70 | ((bank >> 16) & 0xfc)); }

	void map(address_map &map) ATTR_COLD;

protected:
	virtual uint32_t get_bank(uint8_t ctrl) override { return (ctrl & m_bankmask) << m_bankshift; }

private:
	int m_bankshift;
	int m_bankmask;
};

DECLARE_DEVICE_TYPE(SEGAPCM_DISCRETE, segapcm_discrete_device)
DECLARE_DEVICE_TYPE(SEGA_315_5218,    sega_315_5218_device)

//**************************************************************************
//  EXTERNAL TEMPLATE INSTANTIATIONS
//**************************************************************************

extern template class segapcm_device<8, 64, 16>;
extern template class segapcm_device<16, 128, 21>;

#endif // MAME_SOUND_SEGAPCM_H
