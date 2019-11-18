// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

TV test pattern generators

Radio, 1983, N5
    http://radioway.ru/1983/05/generator_telesignalov.html
    http://radioway.ru/1984/04/generator_telesignalov.html

Radio, 1985, N6
    http://radioway.ru/1985/06/generator_ispytatelnyh_signalov.html

***************************************************************************/

#include "emu.h"

#include "machine/netlist.h"

#include "video/fixfreq.h"

#include "netlist/devices/net_lib.h"

#include "machine/nl_tp1983.h"
#include "machine/nl_tp1985.h"

#include "screen.h"

#include <cmath>


namespace {

#define MASTER_CLOCK    (4000000)
#define V_TOTAL_PONG    315
#define H_TOTAL_PONG    256     // tbc

class tp1983_state : public driver_device
{
public:
	tp1983_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	void tp1983(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override { }
	virtual void machine_reset() override { }

	virtual void video_start() override { }

private:
};


class tp1985_state : public driver_device
{
public:
	tp1985_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	void tp1985(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override { }
	virtual void machine_reset() override { }

	virtual void video_start() override { }

private:
	NETDEV_ANALOG_CALLBACK_MEMBER(video_out_cb);
};

NETDEV_ANALOG_CALLBACK_MEMBER(tp1985_state::video_out_cb)
{
	m_video->update_composite_monochrome(4.0 - data, time);
}


static INPUT_PORTS_START(tp1983)
INPUT_PORTS_END

static INPUT_PORTS_START(tp1985)
INPUT_PORTS_END


void tp1983_state::tp1983(machine_config &config)
{
	NETLIST_CPU(config, m_maincpu, NETLIST_CLOCK).set_source(netlist_tp1983);

	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0").set_params("videomix", m_video, FUNC(fixedfreq_device::update_composite_monochrome));

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL_PONG-64,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_vert_params(V_TOTAL_PONG-19,V_TOTAL_PONG-16,V_TOTAL_PONG-12,V_TOTAL_PONG);
	m_video->set_fieldcount(1);
	m_video->set_threshold(1);
	m_video->set_gain(0.36);
}

void tp1985_state::tp1985(machine_config &config)
{
	NETLIST_CPU(config, m_maincpu, NETLIST_CLOCK).set_source(netlist_tp1985);

	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0").set_params("videomix", FUNC(tp1985_state::video_out_cb));

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL_PONG-64,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_vert_params(V_TOTAL_PONG-19,V_TOTAL_PONG-16,V_TOTAL_PONG-12,V_TOTAL_PONG);
	m_video->set_fieldcount(1);
	m_video->set_threshold(1);
	m_video->set_gain(0.36);
}


ROM_START( tp1983 ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( tp1985 ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

} // anonymous namespace


SYST(  1983, tp1983, 0, 0, tp1983,   tp1983,    tp1983_state,   empty_init, "Radio", "TV Test Pattern Generator 1983", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
SYST(  1985, tp1985, 0, 0, tp1985,   tp1985,    tp1985_state,   empty_init, "Radio", "TV Test Pattern Generator 1985", MACHINE_NO_SOUND_HW)
