// license:BSD-3-Clause
// copyright-holders:Bernd Wiebelt
/***************************************************************************

    Midway Omega Race hardware

    driver by Bernd Wiebelt

    Games supported:
        * Omega Race [3 sets]
        * Delta Race (Omega Race bootleg by Allied Leisure)

    Known bugs:
        * none at this time

****************************************************************************

    This driver is dedicated to my loving wife Natalia Wiebelt
                                          and my daughter Lara Anna Maria
    Summer 1997 Bernd Wiebelt

    Many thanks to Al Kossow for the original sources and the solid documentation.
    Without him, I could never had completed this driver.


    --------


    Omega Race Memory Map
    Version 1.1 (Jul 24,1997)
    ---------------------

    0000 - 3fff PROM
    4000 - 4bff RAM (3k)
    5c00 - 5cff NVRAM (256 x 4bits)
    8000 - 8fff Vec RAM (4k)
    9000 - 9fff Vec ROM (4k)

    15 14 13 12 11 10
    --+--+--+--+--+--
    0  0  0  0                       M8 - 2732  (4k)
    0  0  0  1                       L8 - 2732
    0  0  1  0                       K8 - 2732
    0  0  1  1                       J8 - 2732

    0  1  -  0  0  0                 RAM (3k)
    0  1  -  0  0  1
    0  1  -  0  1  0

    0  1  -  1  1  1                 4 Bit BB RAM (d0-d3)

    1  -  -  0  0                    Vec RAM (4k)
    1  -  -  0  1
    1  -  -  1  0                    Vec ROM (2k) E1
    1  -  -  1  1                    Vec ROM (2k) F1

    I/O Ports

    8   Start/ (VG start)
    9   WDOG/  (Reset watchdog)
    A   SEQRES/ (VG stop/reset?)
    B   RDSTOP/ d7 = stop (VG running if 0)

    10 I    DIP SW C4 (game ship settings)

            6 5  4 3  2 1
                          1st bonus ship at
            | |  | |  0 0  40,000
            | |  | |  0 1  50,000
            | |  | |  1 0  70,000
            | |  | |  1 1 100,000
            | |  | |      2nd and  3rd bonus ships
            | |  0 0      150,000   250,000
            | |  0 1      250,000   500,000
            | |  1 0      500,000   750,000
            | |  1 1      750,000 1,500,000
            | |           ships per credit
            0 0           1 credit = 2 ships / 2 credits = 4 ships
            0 1           1 credit = 2 ships / 2 credits = 5 ships
            1 0           1 credit = 3 ships / 2 credits = 6 ships
            1 1           1 credit = 3 ships / 2 credits = 7 ships

    11 I    7 = Test
            6 = P1 Fire
            5 = P1 Thrust
            4 = Tilt

            1 = Coin 2
            0 = Coin 1

    12 I    7 = 1P1CR
            6 = 1P2CR

            3 = 2P2CR -+
            2 = 2P1CR  |
            1 = P2Fire |
            0 = P2Thr -+ cocktail only

    13 O    7 =
            6 = screen reverse
            5 = 2 player 2 credit start LED
            4 = 2 player 1 credit start LED
            3 = 1 player 1 credit start LED
            2 = 1 player 1 credit start LED
            1 = coin meter 2
            0 = coin meter 1

    14 O    sound command (interrupts sound Z80)

    15 I    encoder 1 (d7-d2)

            The encoder is a 64 position Gray code encoder.

            Unlike the quadrature inputs on Atari and Sega games,
            Omega Race's controller is an absolute angle.

            0x00, 0x04, 0x14, 0x10, 0x18, 0x1c, 0x5c, 0x58,
            0x50, 0x54, 0x44, 0x40, 0x48, 0x4c, 0x6c, 0x68,
            0x60, 0x64, 0x74, 0x70, 0x78, 0x7c, 0xfc, 0xf8,
            0xf0, 0xf4, 0xe4, 0xe0, 0xe8, 0xec, 0xcc, 0xc8,
            0xc0, 0xc4, 0xd4, 0xd0, 0xd8, 0xdc, 0x9c, 0x98,
            0x90, 0x94, 0x84, 0x80, 0x88, 0x8c, 0xac, 0xa8,
            0xa0, 0xa4, 0xb4, 0xb0, 0xb8, 0xbc, 0x3c, 0x38,
            0x30, 0x34, 0x24, 0x20, 0x28, 0x2c, 0x0c, 0x08

    16 I    encoder 2 (d5-d0)

            The encoder is a 64 position Gray code encoder.

    17 I    DIP SW C6 (coin/cocktail settings)

            8  7  6 5 4  3 2 1
                                 coin switch 1
            |  |  | | |  0 0 0   1 coin  2 credits
            |  |  | | |  0 0 1   1 coin  3 credits
            |  |  | | |  0 1 0   1 coin  5 credits
            |  |  | | |  0 1 1   4 coins 5 credits
            |  |  | | |  1 0 0   3 coins 4 credits
            |  |  | | |  1 0 1   2 coins 3 credits
            |  |  | | |  1 1 0   2 coins 1 credit
            |  |  | | |  1 1 1   1 coin  1 credit
            |  |  | | |
            |  |  | | |          coin switch 2
            |  |  0 0 0          1 coin  2 credits
            |  |  0 0 1          1 coin  3 credits
            |  |  0 1 0          1 coin  5 credits
            |  |  0 1 1          4 coins 5 credits
            |  |  1 0 0          3 coins 4 credits
            |  |  1 0 1          2 coins 3 credits
            |  |  1 1 0          2 coins 1 credit
            |  |  1 1 1          1 coin  1 credit
            |  |
            |  0                 coin play
            |  1                 free play
            |
            0                    normal
            1                    cocktail

    display list format: (4 byte opcodes)

    +------+------+------+------+------+------+------+------+
    |DY07   DY06   DY05   DY04   DY03   DY02   DY01   DY00  | 0
    +------+------+------+------+------+------+------+------+
    |OPCD3  OPCD2  OPCD1  OPCD0  DY11   DY10   DY09   DY08  | 1 OPCD 1111 = ABBREV/
    +------+------+------+------+------+------+------+------+
    |DX07   DX06   DX05   DX04   DX03   DX02   DX01   DX00  | 2
    +------+------+------+------+------+------+------+------+
    |INTEN3 INTEN2 INTEN1 INTEN0 DX11   DX10   DX09   DX08  | 3
    +------+------+------+------+------+------+------+------+

        Draw relative vector       0x80      1000YYYY YYYYYYYY IIIIXXXX XXXXXXXX

        Draw relative vector
        and load scale             0x90      1001YYYY YYYYYYYY SSSSXXXX XXXXXXXX

        Beam to absolute
        screen position            0xA0      1010YYYY YYYYYYYY ----XXXX XXXXXXXX

        Halt                       0xB0      1011---- --------

        Jump to subroutine         0xC0      1100AAAA AAAAAAAA

        Return from subroutine     0xD0      1101---- --------

        Jump to new address        0xE0      1110AAAA AAAAAAAA

        Short vector draw          0xF0      1111YYYY IIIIXXXX


    Sound Z80 Memory Map

    0000 ROM
    1000 RAM

    15 14 13 12 11 10
                0           2k prom (K5)
                1           2k prom (J5)
             1              1k RAM  (K4,J4)

    I/O (write-only)

    0,1                     8912 (K3)
    2,3                     8912 (J3)


    I/O (read-only)

    0                       input port from main CPU.
                            main CPU writing port generated INT
    Sound Commands:

    0 - reset sound CPU

***************************************************************************/

#include "emu.h"


#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "video/avgdvg.h"
#include "video/vector.h"

#include "screen.h"
#include "speaker.h"

#include "omegrace.lh"

namespace {

class omegrace_state : public driver_device
{
public:
	omegrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_dvg(*this, "dvg")
		, m_soundlatch(*this, "soundlatch")
		, m_leds(*this, "led%u", 0U)
		, m_spinner(*this, "SPIN%u", 0)
	{ }

	void omegrace(machine_config &config);

	void init_omegrace();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dvg_device> m_dvg;
	required_device<generic_latch_8_device> m_soundlatch;
	output_finder<4> m_leds;
	required_ioport_array<2> m_spinner;

	void main_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_port(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(periodic_int);
	uint8_t vg_go_r();
	static constexpr uint8_t encode_spinner(uint8_t data);
	uint8_t spinner1_r();
	uint8_t spinner2_r();
	void outputs_w(uint8_t data);

	emu_timer *m_gbnmi = nullptr;
};


/*************************************
 *
 *  Machine init
 *
 *************************************/

void omegrace_state::machine_start()
{
	m_leds.resolve();

	// Interrupt caused by overflow pulses from 74LS161 clocked by 74LS393 dividing .75 MHz output of 74LS161
	attotime period = attotime::from_hz(12_MHz_XTAL/16/256/12); // ~250 Hz
	m_gbnmi = timer_alloc(FUNC(omegrace_state::periodic_int), this);
	m_gbnmi->adjust(period, 0, period); // first NMI must not arrive immediately
}

void omegrace_state::machine_reset()
{
	/* Omega Race expects the vector processor to be ready. */
	m_dvg->reset_w();
}

TIMER_CALLBACK_MEMBER(omegrace_state::periodic_int)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



/*************************************
 *
 *  Vector processor interaction
 *
 *************************************/

uint8_t omegrace_state::vg_go_r()
{
	if (!machine().side_effects_disabled())
		m_dvg->go_w();
	return 0;
}


/*************************************
 *
 *  Input handlers
 *
 *************************************/

constexpr uint8_t omegrace_state::encode_spinner(uint8_t data)
{
	data &= 0x3f;
	return data ^ (data >> 1) ^ 0x3f; // Inverted 6-bit Gray code
}

uint8_t omegrace_state::spinner1_r()
{
	return encode_spinner(m_spinner[0]->read()) << 2;
}

uint8_t omegrace_state::spinner2_r()
{
	return encode_spinner(m_spinner[1]->read());
}



/*************************************
 *
 *  Output handlers
 *
 *************************************/

void omegrace_state::outputs_w(uint8_t data)
{
	/* bits 0 and 1 are coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 0x01);
	machine().bookkeeping().coin_counter_w(1,data & 0x02);

	/* bits 2 to 5 are the start leds (4 and 5 cocktail only) */
	m_leds[0] = BIT(~data, 2);
	m_leds[1] = BIT(~data, 3);
	m_leds[2] = BIT(~data, 4);
	m_leds[3] = BIT(~data, 5);

	/* bit 6 flips screen (not supported) */
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void omegrace_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x4bff).ram();
	map(0x5c00, 0x5cff).ram().share("nvram");           // NVRAM
	map(0x8000, 0x8fff).ram();                          // vector RAM
	map(0x9000, 0x9fff).rom().region("vectorrom", 0);   // vector ROM
}


void omegrace_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x08, 0x08).r(FUNC(omegrace_state::vg_go_r));
	map(0x09, 0x09).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x0a, 0x0a).w(m_dvg, FUNC(dvg_device::reset_w));
	map(0x0b, 0x0b).portr("AVGDVG");             /* vg_halt */
	map(0x10, 0x10).portr("DSW1");               /* DIP SW C4 */
	map(0x17, 0x17).portr("DSW2");               /* DIP SW C6 */
	map(0x11, 0x11).portr("IN0");                /* Player 1 input */
	map(0x12, 0x12).portr("IN1");                /* Player 2 input */
	map(0x13, 0x13).w(FUNC(omegrace_state::outputs_w)); /* coin counters, leds, flip screen */
	map(0x14, 0x14).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x15, 0x15).r(FUNC(omegrace_state::spinner1_r)); /* 1st controller */
	map(0x16, 0x16).r(FUNC(omegrace_state::spinner2_r)); /* 2nd controller (cocktail) */
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void omegrace_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().mirror(0x800);
	map(0x1000, 0x13ff).ram();
}


void omegrace_state::sound_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read)); // the game reads from ay1 port b, but ay8912 only has port a
	map(0x00, 0x01).w("ay1", FUNC(ay8912_device::address_data_w));
	map(0x02, 0x03).w("ay2", FUNC(ay8912_device::address_data_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( omegrace )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "1st Bonus Life" )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "40k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPSETTING(    0x02, "70k" )
	PORT_DIPSETTING(    0x03, "100k" )
	PORT_DIPNAME( 0x0c, 0x0c, "2nd & 3rd Bonus Life" )  PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "150k 250k" )
	PORT_DIPSETTING(    0x04, "250k 500k" )
	PORT_DIPSETTING(    0x08, "500k 750k" )
	PORT_DIPSETTING(    0x0c, "750k 1500k" )
	PORT_DIPNAME( 0x30, 0x30, "Credit(s)/Ships" )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "1C/2S 2C/4S" )
	PORT_DIPSETTING(    0x10, "1C/2S 2C/5S" )
	PORT_DIPSETTING(    0x20, "1C/3S 2C/6S" )
	PORT_DIPSETTING(    0x30, "1C/3S 2C/7S" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN0")       /* port 0x11 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")       /* port 0x12 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("2 Players Start (1 credit)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("2 Players Start (2 credits)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("1 Player Start (1 credit)")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("1 Player Start (2 credits)")

	PORT_START("SPIN0")     /* port 0x15 - spinner */
	PORT_BIT( 0x3f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(12) PORT_KEYDELTA(10)

	PORT_START("SPIN1")     /* port 0x16 - second spinner */
	PORT_BIT( 0x3f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(12) PORT_KEYDELTA(10) PORT_COCKTAIL

	PORT_START("AVGDVG")    /* port 0x0b */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("dvg", dvg_device, done_r)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void omegrace_state::omegrace(machine_config &config)
{
	/* basic machine hardware */

	/* main CPU */
	/* XTAL101 Crystal @ 12mhz */
	/* through 74LS161, Pin 13 = divide by 4 */
	Z80(config, m_maincpu, 12_MHz_XTAL/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &omegrace_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &omegrace_state::port_map);

	/* audio CPU */
	/* XTAL101 Crystal @ 12mhz */
	/* through 74LS161, Pin 12 = divide by 8 */
	/* Fed to CPU as 1.5mhz though line J4-D */
	Z80(config, m_audiocpu, 12_MHz_XTAL/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &omegrace_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &omegrace_state::sound_port);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 5101 + battery

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_hz(12_MHz_XTAL/16/256/256));

	/* video hardware */
	VECTOR(config, "vector", 0);
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_VECTOR));
	screen.set_refresh_hz(40);
	screen.set_size(400, 300);
	screen.set_visarea(522, 1566, 522, 1566);
	screen.set_screen_update("vector", FUNC(vector_device::screen_update));

	DVG(config, m_dvg, 0);
	m_dvg->set_vector("vector");
	m_dvg->set_memory(m_maincpu, AS_PROGRAM, 0x8000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);

	/* XTAL101 Crystal @ 12mhz */
	/* through 74LS92, Pin 8 = divide by 12 */
	AY8912(config, "ay1", 12_MHz_XTAL/12).add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8912(config, "ay2", 12_MHz_XTAL/12).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( omegrace )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "omega.m7",     0x0000, 0x1000, CRC(0424d46e) SHA1(cc1ac6c06ba6f6e8466fa08286a0c70b5335af33) )
	ROM_LOAD( "omega.l7",     0x1000, 0x1000, CRC(edcd7a7d) SHA1(5d142de2f48b01d563578a54fd5540e5d0ac8f4c) )
	ROM_LOAD( "omega.k7",     0x2000, 0x1000, CRC(6d10f197) SHA1(9609a0cbeeef2efa10d49cde9f0afdca96e9c2f8) )
	ROM_LOAD( "omega.j7",     0x3000, 0x1000, CRC(8e8d4b54) SHA1(944192c0f6f0cdb25d492ee9f33959d38a1062f2) )

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "omega.e1",     0x0000, 0x0800, CRC(1d0fdf3a) SHA1(3333397a9745874cea1dd6a1bda783cc59393b55) )
	ROM_LOAD( "omega.f1",     0x0800, 0x0800, CRC(d44c0814) SHA1(2f216ee6de88bbe09775619003aee2d5aa8c554d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.k5",     0x0000, 0x0800, CRC(7d426017) SHA1(370f0fb5608819de873c845f6010cbde75a9818e) )

	ROM_REGION( 0x100, "dvg:prom", 0 )
	ROM_LOAD( "dvgprom.bin",  0x0000, 0x0100, CRC(d481e958) SHA1(d8790547dc539e25984807573097b61ec3ffe614) )
ROM_END

ROM_START( omegrace2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "o.r._1a.m7",                 0x0000, 0x1000, CRC(f8539d46) SHA1(bb0c6bc2a84e904d1cb00822052c53c0a8ff1083) )
	ROM_LOAD( "o.r._2a.l7",                 0x1000, 0x1000, CRC(0ff70783) SHA1(5fceaaea1439c3ae408a45f4d7839ec56b71504c) )
	ROM_LOAD( "o.r._3a.k7",                 0x2000, 0x1000, CRC(6349130d) SHA1(a1ff62044d9e59294f56079e704beeebc65a56aa) )
	ROM_LOAD( "o.r._4a.j7",                 0x3000, 0x1000, CRC(0a5ef64a) SHA1(42bcc5d5bfe11af4b26ba7753d83e121eef4b597) )

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "o.r._vector_i_6-1-81.e1",    0x0000, 0x0800, CRC(1d0fdf3a) SHA1(3333397a9745874cea1dd6a1bda783cc59393b55) )
	ROM_LOAD( "o.r._vector_ii_6-1-81.f1",   0x0800, 0x0800, CRC(d44c0814) SHA1(2f216ee6de88bbe09775619003aee2d5aa8c554d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "o.r.r._audio_6-1-81.k5",     0x0000, 0x0800, CRC(7d426017) SHA1(370f0fb5608819de873c845f6010cbde75a9818e) )

	ROM_REGION( 0x100, "dvg:prom", 0 )
	ROM_LOAD( "dvgprom.bin",    0x0000, 0x0100, CRC(d481e958) SHA1(d8790547dc539e25984807573097b61ec3ffe614) )
ROM_END

ROM_START( omegrace3 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	/* ROMs typewritten as O.R. with date 7/27 written by hand in red ink */
	ROM_LOAD( "omega_1_7-27.m7",            0x0000, 0x1000, CRC(f8539d46) SHA1(bb0c6bc2a84e904d1cb00822052c53c0a8ff1083) )
	ROM_LOAD( "omega_2_7-27.l7",            0x1000, 0x1000, CRC(9c3edb1a) SHA1(d2487a402586339969070545e4ab24ced2e7243a) )
	ROM_LOAD( "omega_3_7-27.k7",            0x2000, 0x1000, CRC(6349130d) SHA1(a1ff62044d9e59294f56079e704beeebc65a56aa) )
	ROM_LOAD( "omega_4_7-27.j7",            0x3000, 0x1000, CRC(0a5ef64a) SHA1(42bcc5d5bfe11af4b26ba7753d83e121eef4b597) )

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "o.r._vector_i_6-1-81.e1",    0x0000, 0x0800, CRC(1d0fdf3a) SHA1(3333397a9745874cea1dd6a1bda783cc59393b55) )
	ROM_LOAD( "o.r._vector_ii_6-1-81.f1",   0x0800, 0x0800, CRC(d44c0814) SHA1(2f216ee6de88bbe09775619003aee2d5aa8c554d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "o.r.r._audio_6-1-81.k5",     0x0000, 0x0800, CRC(7d426017) SHA1(370f0fb5608819de873c845f6010cbde75a9818e) )

	ROM_REGION( 0x100, "dvg:prom", 0 )
	ROM_LOAD( "dvgprom.bin",    0x0000, 0x0100, CRC(d481e958) SHA1(d8790547dc539e25984807573097b61ec3ffe614) )
ROM_END

ROM_START( deltrace )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "omega.m7",     0x0000, 0x1000, CRC(0424d46e) SHA1(cc1ac6c06ba6f6e8466fa08286a0c70b5335af33) )
	ROM_LOAD( "omega.l7",     0x1000, 0x1000, CRC(edcd7a7d) SHA1(5d142de2f48b01d563578a54fd5540e5d0ac8f4c) )
	ROM_LOAD( "omega.k7",     0x2000, 0x1000, CRC(6d10f197) SHA1(9609a0cbeeef2efa10d49cde9f0afdca96e9c2f8) )
	ROM_LOAD( "delta.j7",     0x3000, 0x1000, CRC(8ef9541e) SHA1(89e34f50a958ac60c5f223bcb6c1c14796b903c7) )

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "omega.e1",     0x0000, 0x0800, CRC(1d0fdf3a) SHA1(3333397a9745874cea1dd6a1bda783cc59393b55) )
	ROM_LOAD( "omega.f1",     0x0800, 0x0800, CRC(d44c0814) SHA1(2f216ee6de88bbe09775619003aee2d5aa8c554d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.k5",     0x0000, 0x0800, CRC(7d426017) SHA1(370f0fb5608819de873c845f6010cbde75a9818e) )

	ROM_REGION( 0x100, "dvg:prom", 0 )
	ROM_LOAD( "dvgprom.bin",  0x0000, 0x0100, CRC(d481e958) SHA1(d8790547dc539e25984807573097b61ec3ffe614) )
ROM_END


/*************************************
 *
 *  Game specific initialization
 *
 *************************************/

void omegrace_state::init_omegrace()
{
	int len = memregion("dvg:prom")->bytes();
	uint8_t *prom = memregion("dvg:prom")->base();

	/* Omega Race has two pairs of the state PROM output
	 * lines swapped before going into the decoder.
	 * Since all other avg/dvg games connect the PROM
	 * in a consistent way to the decoder, we swap the bits
	 * here. */
	for (int i = 0; i < len; i++)
		prom[i] = bitswap<8>(prom[i],7,6,5,4,1,0,3,2);
}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL(1981, omegrace,  0,        omegrace, omegrace, omegrace_state, init_omegrace, ROT0, "Midway", "Omega Race (set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_omegrace )
GAMEL(1981, omegrace2, omegrace, omegrace, omegrace, omegrace_state, init_omegrace, ROT0, "Midway", "Omega Race (set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_omegrace )
GAMEL(1981, omegrace3, omegrace, omegrace, omegrace, omegrace_state, init_omegrace, ROT0, "Midway", "Omega Race (set 3, 7/27)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_omegrace )
GAMEL(1981, deltrace,  omegrace, omegrace, omegrace, omegrace_state, init_omegrace, ROT0, "bootleg (Allied Leisure)", "Delta Race", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_omegrace )
