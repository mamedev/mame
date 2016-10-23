// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#pragma once

#ifndef __SEGAPCM_H__
#define __SEGAPCM_H__

#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7  (0x70<<16)
#define   BANK_MASKF  (0xf0<<16)
#define   BANK_MASKF8 (0xf8<<16)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGAPCM_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, SEGAPCM, _clock)
#define MCFG_SEGAPCM_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, SEGAPCM, _clock)

#define MCFG_SEGAPCM_BANK(_bank) \
	segapcm_device::set_bank(*device, _bank);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class segapcm_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface
{
public:
	segapcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~segapcm_device() { }

	// static configuration
	static void set_bank(device_t &device, int bank) { downcast<segapcm_device &>(device).m_bank = bank; }

	void sega_pcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sega_pcm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_low[16];
	int m_bank;
	int m_bankshift;
	int m_bankmask;
	sound_stream* m_stream;
};

extern const device_type SEGAPCM;


#endif /* __SEGAPCM_H__ */
