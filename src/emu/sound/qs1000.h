/***************************************************************************

    qs1000.h

    QS1000 device emulator.

***************************************************************************/

#pragma once

#ifndef __QS1000_H__
#define __QS1000_H__

#include "cpu/mcs51/mcs51.h"
#include "sound/okiadpcm.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_QS1000_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, QS1000, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define QS1000_INTERFACE(name) \
	const qs1000_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define QS1000_CHANNELS			32
#define QS1000_ADDRESS_MASK		0xffffff

class qs1000_device;


struct qs1000_interface
{
	bool					m_external_rom;

	devcb_read8				m_in_p1_cb;
	devcb_read8				m_in_p2_cb;
	devcb_read8				m_in_p3_cb;

	devcb_write8			m_out_p1_cb;
	devcb_write8			m_out_p2_cb;
	devcb_write8			m_out_p3_cb;

	devcb_write8			m_serial_w;
};

// ======================> qs1000_device

class qs1000_device :	public device_t,
						public device_sound_interface,
						public device_memory_interface,
						public qs1000_interface
{

public:
	// construction/destruction
	qs1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// external
	void serial_in(UINT8 data);
	void set_irq(int state);

protected:
	// device-level overrides
	const rom_entry *device_rom_region() const;
	machine_config_constructor device_mconfig_additions() const;
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( wave_w );

	DECLARE_READ8_MEMBER( p0_r );
	DECLARE_WRITE8_MEMBER( p0_w );

	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );

	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );

	DECLARE_READ8_MEMBER( p3_r );
	DECLARE_WRITE8_MEMBER( p3_w );

	enum
	{
		QS1000_KEYON   = 1,
		QS1000_PLAYING = 2,
		QS1000_ADPCM   = 4,
	};

	void start_voice(int ch);
	void set_voice_regs(int ch);

	// Callbacks
	devcb_resolved_read8			m_p1_r_func;
	devcb_resolved_read8			m_p2_r_func;
	devcb_resolved_read8			m_p3_r_func;
	devcb_resolved_write8			m_p1_w_func;
	devcb_resolved_write8			m_p2_w_func;
	devcb_resolved_write8			m_p3_w_func;

	// Internal state
	const address_space_config		m_space_config;
	sound_stream *					m_stream;
	direct_read_data *				m_direct;
	required_device<i8052_device>	m_cpu;

	// Wavetable engine
	UINT8							m_serial_data_in;
	UINT8							m_wave_regs[18];

	struct qs1000_channel
	{
		UINT32			m_acc;
		INT32			m_adpcm_signal;
		UINT32			m_start;
		UINT32			m_addr;
		UINT32			m_adpcm_addr;
		UINT32			m_loop_start;
		UINT32			m_loop_end;
		UINT16			m_freq;
		UINT16			m_flags;

		UINT8			m_regs[16];	// FIXME

		oki_adpcm_state	m_adpcm;
	};

	qs1000_channel					m_channels[QS1000_CHANNELS];
};


// device type definition
extern const device_type QS1000;


#endif /* __QS1000_H__ */
