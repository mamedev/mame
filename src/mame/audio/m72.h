// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    M72 audio interface

****************************************************************************/

#include "emu.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"

class m72_audio_device : public device_t,
									public device_sound_interface
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~m72_audio_device() {}

	enum
	{
		YM2151_ASSERT,
		YM2151_CLEAR,
		Z80_ASSERT,
		Z80_CLEAR
	};

	void ym2151_irq_handler(int state);
	void sound_command_byte_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sample_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sample_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* the port goes to different address bits depending on the game */
	void set_sample_start( int start );
	void vigilant_sample_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shisen_sample_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rtype2_sample_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void poundfor_sample_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	uint8_t m_irqvector;
	uint32_t m_sample_addr;
	optional_region_ptr<uint8_t> m_samples;
	uint32_t m_samples_size;
	address_space *m_space;
	optional_device<dac_byte_interface> m_dac;
	required_device<generic_latch_8_device> m_soundlatch;

	void setvector_callback(void *ptr, int32_t param);
};

extern const device_type M72;
