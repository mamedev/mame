// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mcms.h

    Implementation of the Mountain Computer Music System.
    This was sold standalone and also used as part of the alphaSyntauri
    and SoundChaser systems.

*********************************************************************/

#ifndef __A2BUS_MCMS__
#define __A2BUS_MCMS__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_mcms1_device;

class mcms_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	mcms_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER(voiceregs_w);
	DECLARE_WRITE8_MEMBER(control_w);
	UINT8 get_pen_rand(void) { m_stream->update(); return m_rand; }

	void set_bus_device(a2bus_mcms1_device *pDev) { m_pBusDevice = pDev; }

	template<class _Object> static devcb_base &set_irq_cb(device_t &device, _Object wr) { return downcast<mcms_device &>(device).m_write_irq.set_callback(wr); }
	devcb_write_line m_write_irq;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream *m_stream;
	emu_timer *m_timer, *m_clrtimer;
	a2bus_mcms1_device *m_pBusDevice;
	bool m_enabled;
	UINT8 m_vols[16];
	UINT8 m_table[16];
	UINT16 m_freq[16];
	UINT16 m_acc[16];
	UINT8 m_mastervol;
	UINT8 m_rand;
};

// card 1
class a2bus_mcms1_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mcms1_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	a2bus_mcms1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// comms from card 2 (oscillator parameter writes)
	mcms_device *get_engine(void);

	DECLARE_WRITE_LINE_MEMBER(irq_w);

	required_device<mcms_device> m_mcms;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual bool take_c800() override { return false; }
};

// card 2
class a2bus_mcms2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mcms2_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	a2bus_mcms2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual bool take_c800() override { return false; }

private:
	a2bus_mcms1_device *m_card1;    // card 1 for passthrough
	mcms_device *m_engine;
};

// device type definition
extern const device_type A2BUS_MCMS1;
extern const device_type A2BUS_MCMS2;

#endif /* __A2BUS_MCMS__ */
