// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

Pong (c) 1972 Atari
Pong Doubles (c) 1973 Atari
Rebound (c) 1974 Atari
Breakout (c) 1976 Atari

driver by Couriersud


 Atari Pong Games List - Data based, in part from:

 - "Andy's collection of Bronzeage Atari Video Arcade PCBs"
 http://www.andysarcade.net/personal/bronzeage/index.htm

 - "Atari's Technical Manual Log"
 http://www.atarigames.com/manuals.txt

 Suspected "same games" are grouped together.  These are usually the exact same game but different cabinet/name.

 Technical Manual #s      Game Name(s)                                                    Atari Part #'s                     Data
 -----------------------+---------------------------------------------------------------+----------------------------------+---------+
 TM-013,029               Pong (1972)                                                     A001433                            NO
 TM-0??                   Pong In-A-Barrel (1973)                                         A001433?                           NO
 TM-015                   Cocktail Pong (1974)                                            A001433?                           NO
 TM-0??                   Dr. Pong/Puppy Pong/Snoopy Pong (1974)                          A001433?                           NO
 422,TM-029               Superpong (1974)                                                A000423                            NO
 TM-014,029               Pong Doubles/Coupe Davis (1973)                                 A000785                            NO
 TM-001,023,029,032       Rebound/Spike/Volleyball (1974)                                 A000517,A000846,SPIKE-(A or B)     NO
 TM-058                   Breakout/Breakout Cocktail/Consolette (1976)                    A004533                            NO

Notes:

TODO: Please see netlist include files
TODO: Breakout Cocktail and Consolette are believed to use the Breakout PCB with different
      cabinet designs, this needs to be verified.
TODO: Coupe Davis is believed to use the Pong Doubles PCB, just a different cabinet design,
      this needs to be verified.
TODO: Dr. Pong, Pong In-A-Barrel, Puppy Pong, Snoopy Pong, and Cocktail Pong are all
      believed to use the Pong (Rev E) PCB, but different cabinet designs; this needs to
      be verified.
TODO: Superpong is believed to use the Pong (Rev E) PCB with some minor modifications, this
      needs to be verified.
TODO: Volleyball...

***************************************************************************/

#include "emu.h"

#include "machine/netlist.h"

#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "video/fixfreq.h"

#include "netlist/devices/net_lib.h"

#include "machine/nl_breakout.h"
#include "machine/nl_rebound.h"
#include "machine/nl_pongf.h"
#include "machine/nl_pongdoubles.h"

#include "screen.h"
#include "speaker.h"

#include "rebound.lh"
#include "breakout.lh"

#include <cmath>


/*
 * H count width to 512
 * Reset at 1C6 = 454
 * V count width to 512, counts on HReset
 * Reset at 105 = 261

 * Clock = 7.159 MHz

 * ==> 15.768 Khz Horz Freq
 * ==> 60.41 Refresh

 * HBlank 0 to 79
 * HSync 32 to 63
 * VBlank 0 to 15
 * VSync 4 to 7

 * Video = (HVID & VVID ) & (NET & PAD1 & PAD2)

 * Net at 256H alternating at 4V
 *
 * Pong videos:
 *
 * https://www.youtube.com/watch?v=pDrRnJOCKZc (no longer available)
 *
 * https://www.youtube.com/watch?v=fiShX2pTz9A
 * https://www.youtube.com/watch?v=YmzH4E3x1_g
 *
 * Breakout videos:
 *
 * https://www.youtube.com/watch?v=NOGO49j5gCE
 *
 */

static const int NS_PER_CLOCK_PONG  = static_cast<int>((double) NETLIST_INTERNAL_RES / (double) 7159000 + 0.5);
static const int MASTER_CLOCK_PONG  = static_cast<int>((double) NETLIST_INTERNAL_RES / (double) NS_PER_CLOCK_PONG + 0.5);

#define V_TOTAL_PONG    (0x105+1)       // 262
#define H_TOTAL_PONG    (0x1C6+1)       // 454

/*
 * Breakout's H1 signal:
 *
 *  __    _    __    _    __    _
 * |  |__| |__|  |__| |__|  |__| |_
 *  2  2  1  2  2  2 1  2  2  2 1
 *
 *  ==> Pixel width is 2:2:1:2:2:1:2:2 .....
 *
 *  4 Pixels = 7 cycles ==> 256 / 4 * 7 = 448
 *
 *  7 cycles ==> 14 Y1 cycles
 *
 */

//#define MASTER_CLOCK_BREAKOUT    (14318000)
static const int NS_PER_CLOCK_BREAKOUT           = static_cast<int>((double) NETLIST_INTERNAL_RES / (double) 14318000 + 0.5);
static const int MASTER_CLOCK_BREAKOUT  = static_cast<int>((double) NETLIST_INTERNAL_RES / (double) NS_PER_CLOCK_BREAKOUT + 0.5);

static const int V_TOTAL_BREAKOUT       = (0xFC);       // 252
static const int H_TOTAL_BREAKOUT       = (448*2);      // 448

enum input_changed_enum
{
	IC_PADDLE1,
	IC_PADDLE2,
	IC_COIN,
	IC_SWITCH,
	IC_VR1,
	IC_VR2
};

class ttl_mono_state : public driver_device
{
public:
	ttl_mono_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_video(*this, "fixfreq"),
			m_dac(*this, "dac")                /* just to have a sound device */
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;
	required_device<dac_word_interface> m_dac; /* just to have a sound device */

	NETDEV_ANALOG_CALLBACK_MEMBER(sound_cb_analog)
	{
		m_dac->write(std::round(16384 * data));
	}

	NETDEV_LOGIC_CALLBACK_MEMBER(sound_cb_logic)
	{
		m_dac->write(16384 * data);
	}

protected:

	// driver_device overrides
	virtual void machine_start() override { };
	virtual void machine_reset() override { };

	virtual void video_start() override { };

private:

};

class pong_state : public ttl_mono_state
{
public:
	pong_state(const machine_config &mconfig, device_type type, const char *tag)
		: ttl_mono_state(mconfig, type, tag),
			m_sw1a(*this, "maincpu:sw1a"),
			m_sw1b(*this, "maincpu:sw1b")
	{
	}

	// sub devices
	required_device<netlist_mame_logic_input_device> m_sw1a;
	required_device<netlist_mame_logic_input_device> m_sw1b;

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

	void pongd(machine_config &config);
	void pong(machine_config &config);
	void pongf(machine_config &config);

	NETLIST_START(pong)

		MEMREGION_SOURCE("maincpu")
		PARAM(NETLIST.USE_DEACTIVATE, 1)
		INCLUDE(pong_schematics)

	NETLIST_END()

protected:

	// driver_device overrides
	virtual void machine_start() override { };
	virtual void machine_reset() override { };
	virtual void video_start() override  { };

private:

};

class breakout_state : public ttl_mono_state
{
public:
	breakout_state(const machine_config &mconfig, device_type type, const char *tag)
		: ttl_mono_state(mconfig, type, tag),
		m_led_serve(*this, "maincpu:led_serve"),
		m_lamp_credit1(*this, "maincpu:lamp_credit1"),
		m_lamp_credit2(*this, "maincpu:lamp_credit2"),
		m_coin_counter(*this, "maincpu:coin_counter"),
		m_sw1_1(*this, "maincpu:sw1_1"),
		m_sw1_2(*this, "maincpu:sw1_2"),
		m_sw1_3(*this, "maincpu:sw1_3"),
		m_sw1_4(*this, "maincpu:sw1_4")
	{
	}
	required_device<netlist_mame_analog_output_device> m_led_serve;
	required_device<netlist_mame_analog_output_device> m_lamp_credit1;
	required_device<netlist_mame_analog_output_device> m_lamp_credit2;
	required_device<netlist_mame_analog_output_device> m_coin_counter;

	required_device<netlist_mame_logic_input_device> m_sw1_1;
	required_device<netlist_mame_logic_input_device> m_sw1_2;
	required_device<netlist_mame_logic_input_device> m_sw1_3;
	required_device<netlist_mame_logic_input_device> m_sw1_4;

	NETDEV_ANALOG_CALLBACK_MEMBER(serve_cb)
	{
		output().set_value("serve_led", (data < 3.5) ? 1 : 0);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(credit1_cb)
	{
		output().set_value("lamp_credit1", (data < 2.0) ? 0 : 1);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(credit2_cb)
	{
		output().set_value("lamp_credit2", (data < 2.0) ? 0 : 1);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(coin_counter_cb)
	{
		machine().bookkeeping().coin_counter_w(0, (data > 2.0) ? 0 : 1);
	}

	DECLARE_INPUT_CHANGED_MEMBER(cb_free_play)
	{
		m_sw1_1->write((newval>>0) & 1);
		m_sw1_2->write((newval>>1) & 1);
		m_sw1_3->write((newval>>2) & 1);
		m_sw1_4->write((newval>>3) & 1);
	}

	void breakout(machine_config &config);
protected:

	// driver_device overrides
	virtual void machine_start() override { };
	virtual void machine_reset() override { };
	virtual void video_start() override  { };

private:

};

class rebound_state : public ttl_mono_state
{
public:
	rebound_state(const machine_config &mconfig, device_type type, const char *tag)
		: ttl_mono_state(mconfig, type, tag)
		, m_sw1a(*this, "maincpu:dsw1a")
		, m_sw1b(*this, "maincpu:dsw1b")
	{
	}

	// sub devices
	required_device<netlist_mame_logic_input_device> m_sw1a;
	required_device<netlist_mame_logic_input_device> m_sw1b;

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

	NETDEV_ANALOG_CALLBACK_MEMBER(led_credit_cb)
	{
		output().set_value("credit_led", (data < 3.5) ? 1 : 0);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(coin_counter_cb)
	{
		machine().bookkeeping().coin_counter_w(0, (data < 1.0));
	}

	void rebound(machine_config &config);

protected:

	// driver_device overrides
	virtual void machine_start() override { };
	virtual void machine_reset() override { };
	virtual void video_start() override  { };

private:

};


INPUT_CHANGED_MEMBER(pong_state::input_changed)
{
	int numpad = param;

	switch (numpad)
	{
	case IC_SWITCH:
		m_sw1a->write(newval ? 1 : 0);
		m_sw1b->write(newval ? 1 : 0);
		break;
	}
}

INPUT_CHANGED_MEMBER(rebound_state::input_changed)
{
	int numpad = param;

	switch (numpad)
	{
	case IC_SWITCH:
		m_sw1a->write(newval ? 1 : 0);
		m_sw1b->write(newval ? 1 : 0);
		break;
	}
}


static INPUT_PORTS_START( pong )
	PORT_START( "PADDLE0" ) /* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0)   NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot0")

	PORT_START( "PADDLE1" ) /* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(2) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot1")

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw")

	PORT_DIPNAME( 0x06, 0x00, "Game Won" )          PORT_DIPLOCATION("SW1A:1,SW1B:1") PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_SWITCH)
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x06, "15" )

	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Antenna") NETLIST_LOGIC_PORT_CHANGED("maincpu", "antenna")

	PORT_START("VR1")
	PORT_ADJUSTER( 50, "VR1 - 50k, Paddle 1 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr0")
	PORT_START("VR2")
	PORT_ADJUSTER( 50, "VR2 - 50k, Paddle 2 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr1")

INPUT_PORTS_END

static INPUT_PORTS_START( pongd )
	PORT_START( "PADDLE0" ) /* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0)   NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot0")

	PORT_START( "PADDLE1" ) /* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(2) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot1")

	PORT_START( "PADDLE2" ) /* fake input port for player 3 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(3) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot2")

	PORT_START( "PADDLE3" ) /* fake input port for player 4 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(4) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot3")

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw")

#if 0
	PORT_DIPNAME( 0x06, 0x00, "Game Won" )          PORT_DIPLOCATION("SW1A:1,SW1B:1") PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_SWITCH)
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x06, "15" )

	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Antenna") NETLIST_LOGIC_PORT_CHANGED("maincpu", "antenna")

	PORT_START("VR1")
	PORT_ADJUSTER( 50, "VR1 - 50k, Paddle 1 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr0")
	PORT_START("VR2")
	PORT_ADJUSTER( 50, "VR2 - 50k, Paddle 2 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr1")
#endif
INPUT_PORTS_END

static INPUT_PORTS_START( breakout )

	PORT_START( "PADDLE0" ) /* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(1) PORT_KEYDELTA(200) PORT_CENTERDELTA(0)   NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot1")

	PORT_START( "PADDLE1" ) /* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(1) PORT_KEYDELTA(200) PORT_CENTERDELTA(0) PORT_PLAYER(2) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot2")

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "servesw")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Antenna") NETLIST_LOGIC_PORT_CHANGED("maincpu", "antenna")


	PORT_START("DIPS")
	PORT_DIPNAME( 0x01, 0x00, "Balls" )          PORT_DIPLOCATION("SW4:1") NETLIST_LOGIC_PORT_CHANGED("maincpu", "sw4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW3:1") NETLIST_LOGIC_PORT_CHANGED("maincpu", "sw3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:1") NETLIST_LOGIC_PORT_CHANGED("maincpu", "sw2")
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW1:1,2,3,4") PORT_CHANGED_MEMBER(DEVICE_SELF, breakout_state, cb_free_play, 0)
	PORT_DIPSETTING(    0x00, "No Free Play" )
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x40, "400" )
	PORT_DIPSETTING(    0x50, "500" )
	PORT_DIPSETTING(    0x60, "600" )
	PORT_DIPSETTING(    0x70, "700" )
	PORT_DIPSETTING(    0x80, "800" )

INPUT_PORTS_END

static INPUT_PORTS_START( rebound )
// FIXME later
	PORT_START( "PADDLE0" ) /* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(1) PORT_KEYDELTA(100) PORT_CENTERDELTA(0)   NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot1")

	PORT_START( "PADDLE1" ) /* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(1) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(2) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot2")

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Antenna") NETLIST_LOGIC_PORT_CHANGED("maincpu", "antenna")

	PORT_START("DIPS")
	PORT_DIPNAME( 0x03, 0x00, "Game Won" )          PORT_DIPLOCATION("SW1A:1,SW1A:2") PORT_CHANGED_MEMBER(DEVICE_SELF, rebound_state, input_changed, IC_SWITCH)
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x03, "15" )

	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1A:3") NETLIST_LOGIC_PORT_CHANGED("maincpu", "dsw2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


void pong_state::pong(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, "maincpu", NETLIST_CLOCK).set_source(this, &pong_state::NETLIST_NAME(pong));

	NETLIST_ANALOG_INPUT(config, "maincpu:vr0", "ic_b9_R.R").set_mult_offset(1.0 / 100.0 * RES_K(50), RES_K(56) );
	NETLIST_ANALOG_INPUT(config, "maincpu:vr1", "ic_a9_R.R").set_mult_offset(1.0 / 100.0 * RES_K(50), RES_K(56) );
	NETLIST_ANALOG_INPUT(config, "maincpu:pot0", "ic_b9_POT.DIAL");
	NETLIST_ANALOG_INPUT(config, "maincpu:pot1", "ic_a9_POT.DIAL");
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1a", "sw1a.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1b", "sw1b.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coinsw", "coinsw.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:antenna", "antenna.IN", 0);

	NETLIST_LOGIC_OUTPUT(config, "maincpu:snd0", 0).set_params("sound", FUNC(pong_state::sound_cb_logic));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("videomix", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	//SCREEN(config, "screen", SCREEN_TYPE_VECTOR);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK_PONG);
	m_video->set_horz_params(H_TOTAL_PONG-67,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_vert_params(V_TOTAL_PONG-22,V_TOTAL_PONG-19,V_TOTAL_PONG-12,V_TOTAL_PONG);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.11);
	m_video->set_horz_scale(4);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

void breakout_state::breakout(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, "maincpu", NETLIST_CLOCK).set_source(NETLIST_NAME(breakout));

	NETLIST_ANALOG_INPUT(config, "maincpu:pot1", "POTP1.DIAL");
	NETLIST_ANALOG_INPUT(config, "maincpu:pot2", "POTP2.DIAL");
	NETLIST_LOGIC_INPUT(config, "maincpu:coinsw1", "COIN1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coinsw2", "COIN2.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw1", "START1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw2", "START2.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:servesw", "SERVE.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw4", "S4.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw3", "S3.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw2", "S2.POS", 0);

	NETLIST_LOGIC_INPUT(config, "maincpu:sw1_1", "S1_1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1_2", "S1_2.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1_3", "S1_3.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1_4", "S1_4.POS", 0);

	NETLIST_LOGIC_INPUT(config, "maincpu:antenna", "antenna.IN", 0);

	NETLIST_ANALOG_OUTPUT(config, "maincpu:snd0", 0).set_params("sound", FUNC(breakout_state::sound_cb_analog));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("videomix", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));

	// Leds and lamps

	NETLIST_ANALOG_OUTPUT(config, "maincpu:led_serve", 0).set_params("CON_P", FUNC(breakout_state::serve_cb));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:lamp_credit1", 0).set_params("CON_CREDIT1", FUNC(breakout_state::credit1_cb));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:lamp_credit2", 0).set_params("CON_CREDIT2", FUNC(breakout_state::credit2_cb));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:coin_counter", 0).set_params("CON_T", FUNC(breakout_state::coin_counter_cb));

	/* video hardware */
	FIXFREQ(config, m_video).set_screen("screen");
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	//SCREEN(config, "screen", SCREEN_TYPE_VECTOR);
	/* The Pixel width is a 2,1,2,1,2,1,1,1 repeating pattern
	 * Thus we must use double resolution horizontally
	 */
	m_video->set_monitor_clock(MASTER_CLOCK_BREAKOUT);
	m_video->set_horz_params((H_TOTAL_BREAKOUT-208),(H_TOTAL_BREAKOUT-144),(H_TOTAL_BREAKOUT-16),  (H_TOTAL_BREAKOUT));
	m_video->set_vert_params(V_TOTAL_BREAKOUT-22,V_TOTAL_BREAKOUT-23,V_TOTAL_BREAKOUT-4, V_TOTAL_BREAKOUT);
	m_video->set_fieldcount(1);
	m_video->set_threshold(1.0);
	m_video->set_gain(1.5);
	m_video->set_horz_scale(2);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

void pong_state::pongf(machine_config &config)
{
	pong(config);

	/* basic machine hardware */

	subdevice<netlist_mame_device>("maincpu")->set_setup_func(NETLIST_NAME(pongf));
}

void pong_state::pongd(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, "maincpu", NETLIST_CLOCK).set_source(NETLIST_NAME(pongdoubles));

	NETLIST_ANALOG_INPUT(config, "maincpu:pot0", "A10_POT.DIAL");
	NETLIST_ANALOG_INPUT(config, "maincpu:pot1", "B10_POT.DIAL");
	NETLIST_ANALOG_INPUT(config, "maincpu:pot2", "B9B_POT.DIAL");
	NETLIST_ANALOG_INPUT(config, "maincpu:pot3", "B9A_POT.DIAL");
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1a", "DIPSW1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:sw1b", "DIPSW2.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coinsw", "COIN_SW.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw", "START_SW.POS", 0);

#if 0
	NETLIST_LOGIC_INPUT(config, "maincpu:antenna", "antenna.IN", 0, 0x01)
#endif

	NETLIST_ANALOG_OUTPUT(config, "maincpu:snd0", 0).set_params("AUDIO", FUNC(pong_state::sound_cb_analog));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("videomix", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK_PONG);
	m_video->set_horz_params(H_TOTAL_PONG-67,H_TOTAL_PONG-52,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_vert_params(V_TOTAL_PONG-22,V_TOTAL_PONG-19,V_TOTAL_PONG-12,V_TOTAL_PONG);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.11);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

void rebound_state::rebound(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, "maincpu", NETLIST_CLOCK).set_source(NETLIST_NAME(rebound));

	// FIXME: Later
	NETLIST_ANALOG_INPUT(config, "maincpu:pot1", "POTP1.DIAL");
	NETLIST_ANALOG_INPUT(config, "maincpu:pot2", "POTP2.DIAL");
	NETLIST_LOGIC_INPUT(config, "maincpu:antenna", "antenna.IN", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coinsw", "COIN1_SW.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw", "START_SW.POS", 0);

	NETLIST_LOGIC_INPUT(config, "maincpu:dsw1a", "DSW1a.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:dsw1b", "DSW1b.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:dsw2", "DSW2.POS", 0);

	NETLIST_ANALOG_OUTPUT(config, "maincpu:snd0", 0).set_params("sound", FUNC(rebound_state::sound_cb_analog));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("videomix", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));

	NETLIST_ANALOG_OUTPUT(config, "maincpu:led_credit", 0).set_params("CON11", FUNC(rebound_state::led_credit_cb));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:coin_counter", 0).set_params("CON10", FUNC(rebound_state::coin_counter_cb));

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK_PONG);
	//m_video->set_horz_params(H_TOTAL_PONG-67,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_horz_params(H_TOTAL_PONG-51,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG);
	m_video->set_vert_params(V_TOTAL_PONG-22,V_TOTAL_PONG-19,V_TOTAL_PONG-12,V_TOTAL_PONG);
	m_video->set_fieldcount(1);
	m_video->set_threshold(1.0);
	m_video->set_gain(0.6);
	m_video->set_horz_scale(2);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	//FIXME: this is not related to reality at all.
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pong ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /* enough for netlist */
	ROM_LOAD( "pong.netlist", 0x000000, 0x00473b, CRC(eadaf087) SHA1(4cb9a79f5cb53502105974be61b99ff16ee930e9) )
ROM_END

ROM_START( breakout )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pongf ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pongd ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( rebound ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END


/*   // 100% TTL - NO ROMS

ROM_START( pongbarl ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( coupedav ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( cktpong ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( drpong ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pupppong ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( snoopong ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( suprpong ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( breakckt ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( consolet ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END
*/

GAME(  1972, pong,      0, pong,     pong,      pong_state,     empty_init, ROT0,  "Atari", "Pong (Rev E) external [TTL]", MACHINE_SUPPORTS_SAVE)
GAME(  1972, pongf,     0, pongf,    pong,      pong_state,     empty_init, ROT0,  "Atari", "Pong (Rev E) [TTL]", MACHINE_SUPPORTS_SAVE)
GAME(  1973, pongd,     0, pongd,    pongd,     pong_state,     empty_init, ROT0,  "Atari", "Pong Doubles [TTL]", MACHINE_SUPPORTS_SAVE)
GAMEL( 1974, rebound,   0, rebound,  rebound,   rebound_state,  empty_init, ROT0,  "Atari", "Rebound (Rev B) [TTL]", MACHINE_SUPPORTS_SAVE, layout_rebound)
GAMEL( 1976, breakout,  0, breakout, breakout,  breakout_state, empty_init, ROT90, "Atari", "Breakout [TTL]", MACHINE_SUPPORTS_SAVE, layout_breakout)

// 100% TTL
//GAMEL(1974, spike,      rebound,  rebound,  rebound,  rebound_state,  empty_init, ROT0,  "Atari/Kee", "Spike [TTL]", MACHINE_IS_SKELETON)
//GAMEL(1974, volleyball, rebound,  rebound,  rebound,  rebound_state,  empty_init, ROT0,  "Atari", "Volleyball [TTL]", MACHINE_IS_SKELETON)
//GAME( 1973, coupedav,   pongd,    pongd,    pongd,    pong_state,     empty_init, ROT0,  "Atari France", "Coupe Davis [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1973, pongbarl,   pong,     pong,     pong,     pong_state,     empty_init, ROT0,  "Atari", "Pong In-A-Barrel [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, cktpong,    pong,     pong,     pong,     pong_state,     empty_init, ROT0,  "Atari / National Entertainment Co.", "Cocktail Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, drpong,     pong,     pong,     pong,     pong_state,     empty_init, ROT0,  "Atari", "Dr. Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, pupppong,   pong,     pong,     pong,     pong_state,     empty_init, ROT0,  "Atari", "Puppy Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, snoopong,   pong,     pong,     pong,     pong_state,     empty_init, ROT0,  "Atari", "Snoopy Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, suprpong,   0,        suprpong, pong,     pong_state,     empty_init, ROT0,  "Atari", "Superpong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAMEL( 1976, breakckt,  breakout, breakout, breakout, breakout_state, empty_init, ROT90, "Atari", "Breakout Cocktail [TTL]", MACHINE_SUPPORTS_SAVE, layout_breakckt)
//GAMEL( 1976, consolet,  breakout, breakout, breakout, breakout_state, empty_init, ROT90, "Atari Europe", "Consolette [TTL]", MACHINE_SUPPORTS_SAVE, layout_consolet)
