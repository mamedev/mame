// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

Pong (c) 1972 Atari
Pong Doubles (c) 1973 Atari
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

***************************************************************************/

#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "sound/dac.h"
#include "video/fixfreq.h"

#include "breakout.lh"

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
 *
 * http://www.youtube.com/watch?v=pDrRnJOCKZc
 */

#define MASTER_CLOCK    7159000
#define V_TOTAL_PONG         (0x105+1)       // 262
#define H_TOTAL_PONG         (0x1C6+1)       // 454

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

#define MASTER_CLOCK_BREAKOUT    (14318000 / 2)

#define V_TOTAL_BREAKOUT         (0xFC)       // 252
#define H_TOTAL_BREAKOUT         (448)    // 448

#if 0
#define HBSTART                 (H_TOTAL_PONG)
#define HBEND                   (80)
#define VBSTART                 (V_TOTAL_PONG)
#define VBEND                   (16)
#endif

enum input_changed_enum
{
	IC_PADDLE1,
	IC_PADDLE2,
	IC_COIN,
	IC_SWITCH,
	IC_VR1,
	IC_VR2
};

NETLIST_EXTERNAL(pongdoubles)
NETLIST_EXTERNAL(pong_fast)
NETLIST_EXTERNAL(breakout)

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
	required_device<netlist_mame_device_t> m_maincpu;
	required_device<fixedfreq_device> m_video;
	required_device<dac_device> m_dac; /* just to have a sound device */

	NETDEV_ANALOG_CALLBACK_MEMBER(sound_cb)
	{
		m_dac->write_unsigned8(64*data);
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
	required_device<netlist_mame_logic_input_t> m_sw1a;
	required_device<netlist_mame_logic_input_t> m_sw1b;

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

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
	required_device<netlist_mame_analog_output_t> m_led_serve;
	required_device<netlist_mame_analog_output_t> m_lamp_credit1;
	required_device<netlist_mame_analog_output_t> m_lamp_credit2;
	required_device<netlist_mame_analog_output_t> m_coin_counter;

	required_device<netlist_mame_logic_input_t> m_sw1_1;
	required_device<netlist_mame_logic_input_t> m_sw1_2;
	required_device<netlist_mame_logic_input_t> m_sw1_3;
	required_device<netlist_mame_logic_input_t> m_sw1_4;

	NETDEV_ANALOG_CALLBACK_MEMBER(serve_cb)
	{
		output_set_value("serve_led", (data < 3.5) ? 1 : 0);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(credit1_cb)
	{
		output_set_value("lamp_credit1", (data < 2.0) ? 0 : 1);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(credit2_cb)
	{
		output_set_value("lamp_credit2", (data < 2.0) ? 0 : 1);
	}

	NETDEV_ANALOG_CALLBACK_MEMBER(coin_counter_cb)
	{
		coin_counter_w(machine(), 0, (data > 2.0) ? 0 : 1);
	}

	DECLARE_INPUT_CHANGED_MEMBER(cb_free_play)
	{
		m_sw1_1->write((newval>>0) & 1);
		m_sw1_2->write((newval>>1) & 1);
		m_sw1_3->write((newval>>2) & 1);
		m_sw1_4->write((newval>>3) & 1);
	}

protected:

	// driver_device overrides
	virtual void machine_start() override { };
	virtual void machine_reset() override { };
	virtual void video_start() override  { };

private:

};

static NETLIST_START(pong)

	MEMREGION_SOURCE("maincpu")
	PARAM(NETLIST.USE_DEACTIVATE, 1)
	INCLUDE(pong_schematics)

NETLIST_END()

INPUT_CHANGED_MEMBER(pong_state::input_changed)
{
	int numpad = (FPTR) (param);

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

static MACHINE_CONFIG_START( pong, pong_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(pong)

	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr0", "ic_b9_R.R")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr1", "ic_a9_R.R")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot0", "ic_b9_POT.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot1", "ic_a9_POT.DIAL")
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1a", "sw1a.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1b", "sw1b.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "coinsw", "coinsw.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "antenna", "antenna.IN", 0, 0x01)

	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "snd0", "sound", pong_state, sound_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "vid0", "videomix", fixedfreq_device, update_vid, "fixfreq")

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen")
	MCFG_FIXFREQ_MONITOR_CLOCK(MASTER_CLOCK)
	MCFG_FIXFREQ_HORZ_PARAMS(H_TOTAL_PONG-67,H_TOTAL_PONG-40,H_TOTAL_PONG-8,H_TOTAL_PONG)
	MCFG_FIXFREQ_VERT_PARAMS(V_TOTAL_PONG-22,V_TOTAL_PONG-19,V_TOTAL_PONG-12,V_TOTAL_PONG)
	MCFG_FIXFREQ_FIELDCOUNT(1)
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.11)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 48000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( breakout, breakout_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(breakout)

	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot1", "POTP1.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot2", "POTP2.DIAL")
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "coinsw1", "COIN1.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "coinsw2", "COIN2.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "startsw1", "START1.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "startsw2", "START2.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "servesw", "SERVE.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw4", "S4.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw3", "S3.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw2", "S2.POS", 0, 0x01)

	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1_1", "S1_1.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1_2", "S1_2.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1_3", "S1_3.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1_4", "S1_4.POS", 0, 0x01)

	MCFG_NETLIST_LOGIC_INPUT("maincpu", "antenna", "antenna.IN", 0, 0x01)

	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "snd0", "sound", breakout_state, sound_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "vid0", "videomix", fixedfreq_device, update_vid, "fixfreq")

	// Leds and lamps

	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "led_serve", "CON_P", breakout_state, serve_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "lamp_credit1", "CON_CREDIT1", breakout_state, credit1_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "lamp_credit2", "CON_CREDIT2", breakout_state, credit2_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "coin_counter", "CON_T", breakout_state, coin_counter_cb, "")

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen")
	/* The Pixel width is a 2,1,2,1,2,1,1,1 repeating pattern
	 * Thus we must use double resolution horizontally
	 */
	MCFG_FIXFREQ_MONITOR_CLOCK(MASTER_CLOCK_BREAKOUT*2)
	MCFG_FIXFREQ_HORZ_PARAMS((H_TOTAL_BREAKOUT-104)*2,(H_TOTAL_BREAKOUT-72)*2,(H_TOTAL_BREAKOUT-8)*2,  (H_TOTAL_BREAKOUT)*2)
	MCFG_FIXFREQ_VERT_PARAMS(V_TOTAL_BREAKOUT-22,V_TOTAL_BREAKOUT-23,V_TOTAL_BREAKOUT-4, V_TOTAL_BREAKOUT)
	MCFG_FIXFREQ_FIELDCOUNT(1)
	MCFG_FIXFREQ_SYNC_THRESHOLD(1.0)
	MCFG_FIXFREQ_GAIN(1.5)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 48000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pongf, pong )

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_NETLIST_SETUP(pong_fast)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pongd, pong_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(pongdoubles)

#if 0
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr0", "ic_b9_R.R")
	MCFG_NETLIST_ANALOG_INPUT_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr1", "ic_a9_R.R")
	MCFG_NETLIST_ANALOG_INPUT_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
#endif
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot0", "A10_POT.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot1", "B10_POT.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot2", "B9B_POT.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot3", "B9A_POT.DIAL")
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1a", "DIPSW1.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1b", "DIPSW2.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "coinsw", "COIN_SW.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "startsw", "START_SW.POS", 0, 0x01)
#if 0
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "antenna", "antenna.IN", 0, 0x01)
#endif

	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "snd0", "AUDIO", pong_state, sound_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "vid0", "videomix", fixedfreq_device, update_vid, "fixfreq")

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen")
	MCFG_FIXFREQ_MONITOR_CLOCK(MASTER_CLOCK)
	MCFG_FIXFREQ_HORZ_PARAMS(H_TOTAL_PONG-67,H_TOTAL_PONG-52,H_TOTAL_PONG-8,H_TOTAL_PONG)
	MCFG_FIXFREQ_VERT_PARAMS(V_TOTAL_PONG-22,V_TOTAL_PONG-19,V_TOTAL_PONG-12,V_TOTAL_PONG)
	MCFG_FIXFREQ_FIELDCOUNT(1)
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.11)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 48000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pong ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /* enough for netlist */
	ROM_LOAD( "pong.netlist", 0x000000, 0x00457f, CRC(72d5e4fe) SHA1(7bb15828223c34915c5e2869dd7917532a4bb7b4) )
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

GAME( 1972, pong,      0, pong,     pong,      driver_device,  0, ROT0,  "Atari", "Pong (Rev E) external [TTL]", MACHINE_SUPPORTS_SAVE)
GAME( 1972, pongf,     0, pongf,    pong,      driver_device,  0, ROT0,  "Atari", "Pong (Rev E) [TTL]", MACHINE_SUPPORTS_SAVE)
GAME( 1973, pongd,     0, pongd,    pongd,     driver_device,  0, ROT0,  "Atari", "Pong Doubles [TTL]", MACHINE_SUPPORTS_SAVE)
GAMEL( 1976, breakout,  0, breakout, breakout,  driver_device,  0, ROT90, "Atari", "Breakout [TTL]", MACHINE_SUPPORTS_SAVE, layout_breakout)

// 100% TTL
//GAME( 1973, coupedav,   pongd,    pongd,    pongd,     driver_device,  0, ROT0,  "Atari France", "Coupe Davis [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1973, pongbarl,   pong,     pong,     pong,      driver_device,  0, ROT0,  "Atari", "Pong In-A-Barrel [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, cktpong,    pong,     pong,     pong,      driver_device,  0, ROT0,  "Atari / National Entertainment Co.", "Cocktail Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, drpong,     pong,     pong,     pong,      driver_device,  0, ROT0,  "Atari", "Dr. Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, pupppong,   pong,     pong,     pong,      driver_device,  0, ROT0,  "Atari", "Puppy Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, snoopong,   pong,     pong,     pong,      driver_device,  0, ROT0,  "Atari", "Snoopy Pong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAME( 1974, suprpong,   0,        suprpong, pong,      driver_device,  0, ROT0,  "Atari", "Superpong [TTL]", MACHINE_SUPPORTS_SAVE)
//GAMEL( 1976, breakckt,  breakout, breakout, breakout,  driver_device,  0, ROT90, "Atari", "Breakout Cocktail [TTL]", MACHINE_SUPPORTS_SAVE, layout_breakckt)
//GAMEL( 1976, consolet,  breakout, breakout, breakout,  driver_device,  0, ROT90, "Atari Europe", "Consolette [TTL]", MACHINE_SUPPORTS_SAVE, layout_consolet)
