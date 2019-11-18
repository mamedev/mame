// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Designs DAVE emulation

**********************************************************************/

#ifndef MAME_SOUND_DAVE_H
#define MAME_SOUND_DAVE_H

#pragma once


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> dave_device

class dave_device : public device_t,
					public device_memory_interface,
					public device_sound_interface
{
public:
	dave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr() { return m_write_irq.bind(); }
	auto lh_wr() { return m_write_lh.bind(); }
	auto rh_wr() { return m_write_rh.bind(); }

	virtual void z80_program_map(address_map &map);
	virtual void z80_io_map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER( int1_w );
	DECLARE_WRITE_LINE_MEMBER( int2_w );

	void io_map(address_map &map);
	void program_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

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

	uint8_t m_segment[4];

	uint8_t m_irq_status;
	uint8_t m_irq_enable;

	emu_timer *m_timer_1hz;
	emu_timer *m_timer_50hz;

	/* SOUND SYNTHESIS */
	uint8_t m_regs[32];
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
DECLARE_DEVICE_TYPE(DAVE, dave_device)

#endif // MAME_SOUND_DAVE_H
