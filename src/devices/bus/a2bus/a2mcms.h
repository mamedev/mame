// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mcms.h

    Implementation of the Mountain Computer Music System.
    This was sold standalone and also used as part of the alphaSyntauri
    and SoundChaser systems.

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2MCMS_H
#define MAME_BUS_A2BUS_A2MCMS_H

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_mcms1_device;

class mcms_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	mcms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void voiceregs_w(offs_t offset, uint8_t data);
	void control_w(offs_t offset, uint8_t data);
	uint8_t get_pen_rand(void) { m_stream->update(); return m_rand; }

	void set_bus_device(a2bus_mcms1_device *pDev) { m_pBusDevice = pDev; }

	auto irq_cb() { return m_write_irq.bind(); }
	devcb_write_line m_write_irq;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(set_irq_tick);
	TIMER_CALLBACK_MEMBER(clr_irq_tick);

private:
	sound_stream *m_stream;
	emu_timer *m_timer;
	emu_timer *m_clrtimer;
	a2bus_mcms1_device *m_pBusDevice;
	bool m_enabled;
	uint8_t m_vols[16];
	uint8_t m_table[16];
	uint16_t m_freq[16];
	uint16_t m_acc[16];
	uint8_t m_mastervol;
	uint8_t m_rand;
};

// card 1
class a2bus_mcms1_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mcms1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// comms from card 2 (oscillator parameter writes)
	mcms_device *get_engine(void);

	required_device<mcms_device> m_mcms;

protected:
	a2bus_mcms1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override { return false; }

private:
	void irq_w(int state);
};

// card 2
class a2bus_mcms2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mcms2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_mcms2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override { return false; }

private:
	a2bus_mcms1_device *m_card1;    // card 1 for passthrough
	mcms_device *m_engine;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_MCMS1, a2bus_mcms1_device)
DECLARE_DEVICE_TYPE(A2BUS_MCMS2, a2bus_mcms2_device)

#endif // MAME_BUS_A2BUS_A2MCMS_H
