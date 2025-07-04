// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Designs DAVE emulation

**********************************************************************/

#ifndef MAME_ENTERPRISE_DAVE_H
#define MAME_ENTERPRISE_DAVE_H

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
	virtual ~dave_device();

	auto irq_wr() { return m_write_irq.bind(); }
	auto lh_wr() { return m_write_lh.bind(); }
	auto rh_wr() { return m_write_rh.bind(); }

	virtual void z80_program_map(address_map &map) ATTR_COLD;
	virtual void z80_io_map(address_map &map) ATTR_COLD;

	void int1_w(int state);
	void int2_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

protected:
	// device_t implentation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implentation
	virtual space_config_vector memory_space_config() const override;

	// sound stream update implentation
	virtual void sound_stream_update(sound_stream &stream) override;

	TIMER_CALLBACK_MEMBER(update_1hz_timer);
	TIMER_CALLBACK_MEMBER(update_50hz_timer);

	uint8_t program_r(offs_t offset);
	void program_w(offs_t offset, uint8_t data);

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

private:
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

	sound_stream *m_sound_stream_var;
};


// device type declaration
DECLARE_DEVICE_TYPE(DAVE, dave_device)

#endif // MAME_ENTERPRISE_DAVE_H
