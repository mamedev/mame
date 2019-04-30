// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/***************************************************************************

    Palestra-02 (a Pong clone)

    https://www.mobygames.com/game/dedicated-console/palestra-02
        database entry
    http://www.ccjvq.com/slydc/palestra02.rar
        schematics
    http://discreteconsoles.blogspot.com/2015/10/emulation-of-ay-3-8500-1-and-clones-soon.html
        photos
    https://www.youtube.com/watch?v=3XZxkTvOF4Y
        gameplay video

    To do:
    - write 74H53 device
    - trace the boards (schematic contains several errors)
    - hook up inputs

***************************************************************************/

#include "emu.h"

#include "machine/netlist.h"

#include "video/fixfreq.h"

#include "netlist/devices/net_lib.h"

#include "machine/nl_palestra.h"

#include "screen.h"

#include <cmath>


#define MASTER_CLOCK    (4000000)
#define V_TOTAL_PONG    315
#define H_TOTAL_PONG    256     // tbc

class palestra_state : public driver_device
{
public:
	palestra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	void palestra(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override { };
	virtual void machine_reset() override { };

	virtual void video_start() override { };

private:
};


static INPUT_PORTS_START(palestra)
INPUT_PORTS_END


void palestra_state::palestra(machine_config &config)
{
	NETLIST_CPU(config, m_maincpu, NETLIST_CLOCK)
		.set_source(netlist_palestra);

	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0").set_params("videomix", FUNC(fixedfreq_device::update_composite_monochrome), "fixfreq");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL_PONG-64,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_vert_params(V_TOTAL_PONG-19,V_TOTAL_PONG-16,V_TOTAL_PONG-12,V_TOTAL_PONG);
	m_video->set_fieldcount(1);
	m_video->set_threshold(1);
	m_video->set_gain(0.36);
}


ROM_START( palestra ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

CONS(  1978, palestra, 0,   0, palestra,   palestra,    palestra_state,   empty_init, "LPO", "Palestra-02", MACHINE_IS_SKELETON)
