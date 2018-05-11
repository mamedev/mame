// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Aaron Giles
/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#ifndef MAME_SOUND_RF5C68_H
#define MAME_SOUND_RF5C68_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RF5C68_SAMPLE_END_CB(_class, _method) \
	downcast<rf5c68_device &>(*device).set_end_callback(rf5c68_device::sample_end_cb_delegate(&_class::_method, #_class "::" #_method, this));

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define RF5C68_SAMPLE_END_CB_MEMBER(_name)   void _name(int channel)


// ======================> rf5c68_device

class rf5c68_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	typedef device_delegate<void (int channel)> sample_end_cb_delegate;

	rf5c68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename Object> void set_end_callback(Object &&cb) { m_sample_end_cb = std::forward<Object>(cb); }

	DECLARE_READ8_MEMBER( rf5c68_r );
	DECLARE_WRITE8_MEMBER( rf5c68_w );

	DECLARE_READ8_MEMBER( rf5c68_mem_r );
	DECLARE_WRITE8_MEMBER( rf5c68_mem_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;
private:
	static constexpr unsigned NUM_CHANNELS = 8;

	struct pcm_channel
	{
		pcm_channel() { }

		uint8_t       enable = 0;
		uint8_t       env    = 0;
		uint8_t       pan    = 0;
		uint8_t       start  = 0;
		uint32_t      addr   = 0;
		uint16_t      step   = 0;
		uint16_t      loopst = 0;
	};

	address_space                                *m_data;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_cache;
	sound_stream*                                 m_stream;
	pcm_channel                                   m_chan[NUM_CHANNELS];
	uint8_t                                       m_cbank;
	uint16_t                                      m_wbank;
	uint8_t                                       m_enable;

	sample_end_cb_delegate m_sample_end_cb;
};

DECLARE_DEVICE_TYPE(RF5C68, rf5c68_device)

#endif // MAME_SOUND_RF5C68_H
