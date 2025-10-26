// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK "Menestrel" sound interface.

    Six channels of filtered square wave output from two 8253 timers.
    Stereo output via DIN connector, no balance or volume controls.

    Has software support in ROM 370.

    Not implemented: filter, clock phase offset.

***************************************************************************/

#include "emu.h"
#include "menestrel.h"

#include "machine/pit8253.h"
#include "sound/spkrdev.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_menestrel_device

class bk_menestrel_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_menestrel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device_array<pit8253_device, 2> m_timer;
	required_ioport m_config;
	required_device<speaker_sound_device> m_lspeaker;
	required_device<speaker_sound_device> m_rspeaker;

	uint16_t m_data;

	int m_out[6];

	template <int channel> void lspeaker_changed(int state);
	template <int channel> void rspeaker_changed(int state);
	void out5_changed(int state);
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START(config)
	PORT_START("CONFIG")
	PORT_DIPNAME(0x01, 0x00, "Timer interrupt")
	PORT_DIPSETTING(0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(0x00, DEF_STR(No) )
INPUT_PORTS_END

ioport_constructor bk_menestrel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(config);
}

static const double speaker_levels[] = {-1.0, -0.33333, 0.33333, 1.0};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_menestrel_device - constructor
//-------------------------------------------------

bk_menestrel_device::bk_menestrel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_MENESTREL, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
	, m_timer(*this, "timer%u", 0U)
	, m_config(*this, "CONFIG")
	, m_lspeaker(*this, "lspeaker")
	, m_rspeaker(*this, "rspeaker")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bk_menestrel_device::device_add_mconfig(machine_config &config)
{
	PIT8253(config, m_timer[0], 0);
	m_timer[0]->set_clk<0>(XTAL(1'000'000));
	m_timer[0]->out_handler<0>().set(FUNC(bk_menestrel_device::lspeaker_changed<0>));
	m_timer[0]->set_clk<1>(XTAL(1'000'000));
	m_timer[0]->out_handler<1>().set(FUNC(bk_menestrel_device::lspeaker_changed<1>));
	m_timer[0]->set_clk<2>(XTAL(1'000'000));
	m_timer[0]->out_handler<2>().set(FUNC(bk_menestrel_device::lspeaker_changed<2>));

	PIT8253(config, m_timer[1], 0);
	m_timer[1]->set_clk<0>(XTAL(1'000'000));
	m_timer[1]->out_handler<0>().set(FUNC(bk_menestrel_device::rspeaker_changed<0>));
	m_timer[1]->set_clk<1>(XTAL(1'000'000));
	m_timer[1]->out_handler<1>().set(FUNC(bk_menestrel_device::rspeaker_changed<1>));
	m_timer[1]->set_clk<2>(XTAL(1'000'000));
	m_timer[1]->out_handler<2>().set(FUNC(bk_menestrel_device::out5_changed));

	SPEAKER(config, "dd1").front_left();
	SPEAKER_SOUND(config, m_lspeaker);
	m_lspeaker->set_levels(4, speaker_levels);
	m_lspeaker->add_route(ALL_OUTPUTS, "dd1", 0.25);

	SPEAKER(config, "dd2").front_right();
	SPEAKER_SOUND(config, m_rspeaker);
	m_rspeaker->set_levels(4, speaker_levels);
	m_rspeaker->add_route(ALL_OUTPUTS, "dd2", 0.25);
}


template <int channel> void bk_menestrel_device::lspeaker_changed(int state)
{
	if (state > 1) return;
	m_out[channel] = state;
	m_lspeaker->level_w(m_out[0] + m_out[1] + m_out[2]);
}

template <int channel> void bk_menestrel_device::rspeaker_changed(int state)
{
	if (state > 1) return;
	m_out[3 + channel] = state;
	m_rspeaker->level_w(m_out[3] + m_out[4] + m_out[5]);
}

void bk_menestrel_device::out5_changed(int state)
{
	if (state > 1) return;
	if (m_config->read())
	{
		m_slot->irq2_w(!state);
	}
	else
	{
		m_out[5] = state;
		m_rspeaker->level_w(m_out[3] + m_out[4] + m_out[5]);
	}
}


void bk_menestrel_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_out));

	std::fill(std::begin(m_out), std::end(m_out), 0);
}

void bk_menestrel_device::device_reset()
{
	m_slot->irq2_w(0);
	m_lspeaker->level_w(0);
	m_rspeaker->level_w(0);
}

void bk_menestrel_device::io_w(uint16_t data, bool word)
{
	if (BIT(data ^ m_data, 12) && !BIT(data, 12)) // ~WR edge
	{
		if (!BIT(data, 10)) m_timer[0]->write(bitswap<2>(data, 9, 8), data & 255);
		if (!BIT(data, 11)) m_timer[1]->write(bitswap<2>(data, 9, 8), data & 255);
	}

	if (BIT(data ^ m_data, 15))
	{
		const int gate = BIT(data, 15);
		m_timer[0]->write_gate0(gate);
		m_timer[0]->write_gate1(gate);
		m_timer[0]->write_gate2(gate);
		m_timer[1]->write_gate0(gate);
		m_timer[1]->write_gate1(gate);
		m_timer[1]->write_gate2(gate);
	}

	m_data = data;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_MENESTREL, device_bk_parallel_interface, bk_menestrel_device, "bk_menestrel", "Menestrel Interface")
