// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Designs DAVE emulation

**********************************************************************/

#pragma once

#ifndef __DAVE__
#define __DAVE__

#include "emu.h"



///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_DAVE_ADD(_tag, _clock, _program_map, _io_map) \
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker") \
	MCFG_SOUND_ADD(_tag, DAVE, _clock) \
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25) \
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25) \
	MCFG_DEVICE_ADDRESS_MAP(AS_PROGRAM, _program_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_IO, _io_map)


#define MCFG_DAVE_IRQ_CALLBACK(_write) \
	devcb = &dave_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_DAVE_LH_CALLBACK(_write) \
	devcb = &dave_device::set_lh_wr_callback(*device, DEVCB_##_write);

#define MCFG_DAVE_RH_CALLBACK(_write) \
	devcb = &dave_device::set_rh_wr_callback(*device, DEVCB_##_write);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> dave_device

class dave_device : public device_t,
					public device_memory_interface,
					public device_sound_interface
{
public:
	dave_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<dave_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_lh_wr_callback(device_t &device, _Object object) { return downcast<dave_device &>(device).m_write_lh.set_callback(object); }
	template<class _Object> static devcb_base &set_rh_wr_callback(device_t &device, _Object object) { return downcast<dave_device &>(device).m_write_rh.set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(z80_program_map, 8);
	virtual DECLARE_ADDRESS_MAP(z80_io_map, 8);

	DECLARE_WRITE_LINE_MEMBER( int1_w );
	DECLARE_WRITE_LINE_MEMBER( int2_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	DECLARE_READ8_MEMBER( program_r );
	DECLARE_WRITE8_MEMBER( program_w );

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

private:
	enum
	{
		TIMER_1HZ,
		TIMER_50HZ
	};

	enum
	{
		IRQ_50HZ_DIVIDER    = 0x01,
		IRQ_50HZ_LATCH      = 0x02,
		IRQ_1HZ_DIVIDER     = 0x04,
		IRQ_1HZ_LATCH       = 0x08,
		IRQ_INT1            = 0x10,
		IRQ_INT1_LATCH      = 0x20,
		IRQ_INT2            = 0x40,
		IRQ_INT2_LATCH      = 0x80,
		IRQ_LATCH           = IRQ_INT2_LATCH | IRQ_INT1_LATCH | IRQ_1HZ_LATCH | IRQ_50HZ_LATCH
	};

	void update_interrupt();

	const address_space_config m_program_space_config;
	const address_space_config m_io_space_config;

	devcb_write_line m_write_irq;
	devcb_write8 m_write_lh;
	devcb_write8 m_write_rh;

	UINT8 m_segment[4];

	UINT8 m_irq_status;
	UINT8 m_irq_enable;

	emu_timer *m_timer_1hz;
	emu_timer *m_timer_50hz;

	/* SOUND SYNTHESIS */
	UINT8 m_regs[32];
	int m_period[4];
	int m_count[4];
	int m_level[4];

	/* these are used to force channels on/off */
	/* if one of the or values is 0x0ff, this means
	 the volume will be forced on,else it is dependant on
	 the state of the wave */
	int m_level_or[8];
	/* if one of the values is 0x00, this means the
	 volume is forced off, else it is dependant on the wave */
	int m_level_and[8];

	/* these are the current channel volumes in MAME form */
	int m_mame_volumes[8];

	/* update step */
	//int m_update_step;

	sound_stream *m_sound_stream_var;
};


// device type definition
extern const device_type DAVE;



#endif
