// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*

adp Gauselmann (Merkur) games from '90 running on similar hardware.
(68k + HD63484 + YM2149)

Skeleton driver by TS

TODO:
 - protection in Fashion Gambler (NVRam based?) Update 2023: outdated note?
 - fstation: Suntris crashes when started it up, culprit is HD63484 paint command
   trying to write (to ROM) with negative XY values;

Supported games :
- Quick Jack      ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1993")
- Skat TV           ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")
- Skat TV v. TS3  ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1995")
- Fashion Gambler ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1997")
- Funny Land de Luxe ("Copyright 1992-99 by Stella International Germany")
- Fun Station Spielekoffer 9 Spiele ("COPYRIGHT BY ADP LUEBBECKE GERMANY 2000")


Skat TV (Version TS3)
Three board stack.

CPU Board:
----------
 ____________________________________________________________
 |           ______________  ______________     ___________ |
 | 74HC245N  | t1 i       |  |KM681000ALP7|     |+        | |
 | 74HC573   |____________|  |____________|     |  3V Bat | |
 |                                              |         | |
 |           ______________  ______________     |        -| |
 |           | t1 ii      |  |KM681000ALP7|     |_________| |
 |     |||   |____________|  |____________| |||             |
 |     |||   ___________                    |||  M62X42B    |
 | X   |||   |         |                    |||             |
 |     |||   |68EC000 8|  74HC32   74HC245  |||  MAX691CPE  |
 |     |||   |         |  74AC138  74HC573  |||    74HC32   |
 |           |         |                                    |
 | 74HC573   |_________|  74HC08   74HC10  74HC32  74HC21   |
 |__________________________________________________________|

Parts:

 68EC000FN8         - Motorola 68k CPU
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM
 OKIM62X42B         - Real-time Clock ic With Built-in Crystal
 MAX691CPE          - P Reset ic With Watchdog And Battery Switchover
 X                    - 8MHz xtal
 3V Bat             - Lithium 3V power module

Video Board:
------------
 ____________________________________________________________
 |           ______________  ______________                 |
 |           | t2 i       |  |KM681000ALP7|     74HC573     |
 |           |____________|  |____________|                *|
 |                                              74HC573    *|
 |           ______________  ______________                *|
 |           | t2 ii      |  |KM681000ALP7|               P3|
 |       ||| |____________|  |____________|   |||          *|
 |       ||| ___________                      |||          *|
 |       ||| |         |                      |||          *|
 |       ||| | HD63484 |  74HC04   74HC00     |||         P6|
 |       ||| |         |  74HC74   74HC08     |||  74HC245  |
 |           |         |                                    |
 | 74HC573   |_________|  74HC166  74HC166 74HC166 74HC166  |
 |__________________________________________________________|

Parts:

 HD63484CP8         - Advanced CRT Controller
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM

Connectors:

 Two connectors to link with CPU Board
 Two connectors to link with Sound and I/O Board
 P3  - Monitor
 P6  - Lightpen

Sound  and I/O board:
---------------------
"Steuereinheit 68000"
 _________________________________________________________________________________
 |                        TS271CN    74HC02                        ****  ****    |
 |*         74HC573      ________________                          P1    P2     *|
 |*                      | YM2149F      |                                       *|
 |*         74HC574  ||| |______________|   74HC393  74HC4015 ||| MX7224KN      *|
 |P3                 |||                                      |||              P6|
 |*         74HC245  ||| ________________   3.6864M  74HC125  ||| TL7705ACP     *|
 |*   L4974A         ||| |SCN68681C1N40 |                     |||               *|
 |*                  ||| |______________|   74HC32   74AC138  |||               *|
 |P7                 |||                                      |||              P8|
 |*                        TC428CPA                                             *|
 |*                                                                             *|
 |*    P11  P12    P13    P14       P15   P16   P17      P18   P19   P20  P21   *|
 |P9   **** *****  *****  ****  OO  ****  ****  *******  ****  ****  ***  *** P10|
 |_______________________________________________________________________________|

Parts:

 YM2149F         - Yamaha PSG
 SCN68681C1N40   - Dual Asynchronous Receiver/transmitter (DUART);
 TS271CN         - Programmable Low Power CMOS Single Op-amp
 MX7224KN        - Maxim CMOS 8-bit DAC with Output Amplifier
 TL7705ACP       - Supply Voltage Supervisor
 TC428CPA        - Dual CMOS High-speed Driver
 L4974A          - ST 3.5A Switching Regulator
 OO              - LEDs (red); "Fehlerdiagnose siehe Fehlertable"

Connectors:

 Two connectors to link with Video Board
 P1  - Türöffnungen [1-6]
 P2  - PSG In/Out [1-6]
 P3  - Lautsprecher [1-6]
 P6  - Service - Test Gerät [1-6]
 P7  - Maschine [1-8]
 P8  - Münzeinheit [1-8]
 P9  - Akzeptor [1-4]
 P10 - Fadenfoul [1-4]
 P11 - Netzteil [1-5]
 P12 - Serienplan [1-8]
 P13 - Serienplan 2 [1-8]
 P14 - Münzeinheit 2 [1-8]
 P15 - I2C-Bus [1-4]
 P16 - Kodierg. [1-4]
 P17 - TTL-Ein-/Ausgänge (PSG-Port) [1-10]
 P18 - RS485 Aus [1-2]
 P19 - RS485 Ein [1-2]
 P20 - Serielle-S. [1-5]
 P21 - Türschalter [1-4]

There's also (external) JAMMA adapter - 4th board filled with resistors and diodes.



Funny Land de Luxe
------------------

Video board has additional chips:
  - Altera EPM7032 (PLD)
  - SG-615PH (32.0000M oscillator)
  - Bt481 (RAMDAC)



Quick Jack administration/service mode:
- hold down Start and Joker buttons at start
- enter the default CODENUMBER 54321 using the hand buttons
- confirm the CODENUMBER with start

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/microtch.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/hd63484.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class adp_state : public driver_device
{
public:
	adp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_microtouch(*this, "microtouch"),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_acrtc(*this, "acrtc"),
		m_palette(*this, "palette"),
		m_nvram(*this, "nvram"),
		m_in0(*this, "IN0")
	{ }

	void skattv(machine_config &config);
	void quickjac(machine_config &config);
	void fashiong(machine_config &config);
	void fstation(machine_config &config);
	void funland(machine_config &config);
	void skattva(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<microtouch_device> m_microtouch;
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<hd63484_device> m_acrtc;
	required_device<palette_device> m_palette;
	required_device<nvram_device> m_nvram;
	required_ioport m_in0;

	/* misc */
	uint8_t m_mux_data;

	/* devices */
	uint16_t input_r();
	void input_w(uint16_t data);
	void adp_palette(palette_device &device) const;
	void fstation_palette(palette_device &device) const;
	//INTERRUPT_GEN_MEMBER(adp_int);
	void skattva_nvram_init(nvram_device &nvram, void *base, size_t size);

	void adp_hd63484_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;
	void fashiong_hd63484_map(address_map &map) ATTR_COLD;
	void fstation_hd63484_map(address_map &map) ATTR_COLD;
	void fstation_mem(address_map &map) ATTR_COLD;
	void funland_mem(address_map &map) ATTR_COLD;
	void quickjac_mem(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void skattv_mem(address_map &map) ATTR_COLD;
	void skattva_mem(address_map &map) ATTR_COLD;
};

void adp_state::skattva_nvram_init(nvram_device &nvram, void *base, size_t size)
{
/*
    00F6FA: 4EB9 0001 A4B2             jsr     $1a4b2.l
    00F700: 0CB8 2400 0018 E450        cmpi.l  #$24000018, $e450.w
    00F708: 6606                       bne     $f710
    00F70A: 4EB9 0001 D7F4             jsr     $1d7f4.l                 ; initializes the HD63484
    00F710: 11FC 0010 E8AD             move.b  #$10, $e8ad.w
*/
	uint16_t *ram = (uint16_t *)base;
	ram[0x2450 >> 1] = 0x2400;
	ram[0x2452 >> 1] = 0x0018;
	ram[0x0000 >> 1] = 0x3141;
	ram[0x0002 >> 1] = 0x5926;
}



/***************************************************************************

    68681 DUART <-> Microtouch touch screen controller communication

***************************************************************************/

void adp_state::fc7_map(address_map &map)
{
	map(0xfffff9, 0xfffff9).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}

void adp_state::machine_start()
{
	save_item(NAME(m_mux_data));
}

void adp_state::machine_reset()
{
	m_mux_data = 0;
}

void adp_state::adp_palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
	{
		int const r = 0x21 * BIT(i, 0) + 0x47 * BIT(i, 3) + 0x97 * BIT(i, 0);
		int const g = 0x21 * BIT(i, 1) + 0x47 * BIT(i, 3) + 0x97 * BIT(i, 1);
		int const b = 0x21 * BIT(i, 2) + 0x47 * BIT(i, 3) + 0x97 * BIT(i, 2);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void adp_state::fstation_palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
		palette.set_pen_color(i, rgb_t(pal3bit(i>>5), pal3bit(i>>2), pal2bit(i>>0)));
}

uint16_t adp_state::input_r()
{
	uint16_t data = 0xffff;

	data &= ~(BIT(m_in0->read(), m_mux_data) ? 0x0000 : 0x0004);

	return data;
}

void adp_state::input_w(uint16_t data)
{
	m_mux_data++;
	m_mux_data &= 0x0f;
}

void adp_state::skattv_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x800080, 0x800083).rw(m_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x800100, 0x800101).rw(FUNC(adp_state::input_r), FUNC(adp_state::input_w));
	map(0x800140, 0x800143).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)).umask16(0x00ff); //18b too
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xffc000, 0xffffff).ram().share("nvram");
}

void adp_state::skattva_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x400000, 0x40001f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
	map(0x800080, 0x800083).rw(m_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x800100, 0x800101).portr("IN0");
	map(0x800140, 0x800143).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)).umask16(0x00ff); //18b too
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xffc000, 0xffffff).ram().share("nvram");
}

void adp_state::quickjac_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x400000, 0x40001f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
	map(0x800080, 0x800083).rw(m_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16)); // bad
	map(0x800100, 0x800101).portr("IN0");
	map(0x800140, 0x800143).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)).umask16(0x00ff); //18b too
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xff0000, 0xffffff).ram().share("nvram");
}

void adp_state::funland_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40001f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
	map(0x800080, 0x800083).rw(m_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x800089, 0x800089).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x80008b, 0x80008b).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x80008d, 0x80008d).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x800100, 0x800101).portr("IN0");
	map(0x800140, 0x800143).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)).umask16(0x00ff); //18b too
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xfc0000, 0xffffff).ram().share("nvram");
}

void adp_state::fstation_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x800080, 0x800083).rw(m_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x800100, 0x800101).rw(FUNC(adp_state::input_r), FUNC(adp_state::input_w));
	map(0x800140, 0x800143).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)).umask16(0x00ff); //18b too
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xfc0000, 0xffffff).ram().share("nvram");
}


static INPUT_PORTS_START( quickjac )
	PORT_START("PA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Collect")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hand 1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hand 2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hand 3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hand 4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hand 5")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Joker")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Low Battery" )
	PORT_DIPSETTING(     0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x00, DEF_STR( On ) )
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( skattv )
	PORT_START("PA")
	PORT_BIT( 0x9f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_HBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN5    )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN6    )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BILL1    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("IN0")
	PORT_DIPNAME( 0x0001,0x0001, "SW0" ) //vblank status?
	PORT_DIPSETTING(     0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0004,0x0004, "SW2" ) //another up button
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,0x0008, "SW3" )
	PORT_DIPSETTING(     0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x0020,0x0020, "SW5" )
	PORT_DIPSETTING(     0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_DIPNAME( 0x0100,0x0100, "SW8" )
	PORT_DIPSETTING(     0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,0x0200, "SW9" )    //button 2
	PORT_DIPSETTING(     0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_DIPNAME( 0x0800,0x0800, "SW11" )
	PORT_DIPSETTING(     0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000,0x1000, "SW12" )   //button 3
	PORT_DIPSETTING(     0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000,0x2000, "SW13" )
	PORT_DIPSETTING(     0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( skattva )
	PORT_START("PA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_HBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x9e, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( fstation )
	PORT_START("PA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_HBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0010,0x0010, "SW4" )
	PORT_DIPSETTING(     0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020,0x0020, "SW5" )
	PORT_DIPSETTING(     0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x0080,0x0080, "SW7" )
	PORT_DIPSETTING(     0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100,0x0100, "SW8" )
	PORT_DIPSETTING(     0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,0x0200, "SW9" )
	PORT_DIPSETTING(     0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400,0x0400, "SW10" )
	PORT_DIPSETTING(     0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800,0x0800, "SW11" )
	PORT_DIPSETTING(     0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000,0x1000, "SW12" )
	PORT_DIPSETTING(     0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000,0x2000, "SW13" )
	PORT_DIPSETTING(     0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

/*
INTERRUPT_GEN_MEMBER(adp_state::adp_int)
{
    device.execute().set_input_line(1, HOLD_LINE); // ??? All irqs have the same vector, and the mask used is 0 or 7
}
*/

void adp_state::adp_hd63484_map(address_map &map)
{
	map(0x00000, 0x1ffff).mirror(0x60000).ram();
	map(0x80000, 0x9ffff).mirror(0x60000).rom().region("gfx1", 0);
}

void adp_state::fashiong_hd63484_map(address_map &map)
{
	map(0x00000, 0x1ffff).mirror(0x60000).ram();
	map(0x80000, 0xfffff).rom().region("gfx1", 0);
}

void adp_state::fstation_hd63484_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("gfx1", 0);
	map(0x80000, 0xfffff).ram();
}

void adp_state::quickjac(machine_config &config)
{
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &adp_state::quickjac_mem);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &adp_state::fc7_map);

	MC68681(config, m_duart, XTAL(8'664'000) / 2);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	m_duart->a_tx_cb().set(m_microtouch, FUNC(microtouch_device::rx));
	m_duart->inport_cb().set_ioport("DSW1");

	MICROTOUCH(config, m_microtouch, 9600).stx().set(m_duart, FUNC(mc68681_device::rx_a_w));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	MSM6242(config, "rtc", XTAL(32'768));
	//rtc.out_int_handler().set(FUNC(adp_state::rtc_irq));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(384, 280);
	screen.set_visarea_full();
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(adp_state::adp_palette), 0x10);

	HD63484(config, m_acrtc, 0).set_addrmap(0, &adp_state::adp_hd63484_map);

	SPEAKER(config, "mono").front_center();
	ym2149_device &aysnd(YM2149(config, "aysnd", 3686400/2));
	aysnd.port_a_read_callback().set_ioport("PA");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.10);
}

void adp_state::skattv(machine_config &config)
{
	quickjac(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &adp_state::skattv_mem);
}

void adp_state::skattva(machine_config &config)
{
	quickjac(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &adp_state::skattva_mem);
	m_nvram->set_custom_handler(FUNC(adp_state::skattva_nvram_init));
}

void adp_state::fashiong(machine_config &config)
{
	skattv(config);
	m_acrtc->set_addrmap(0, &adp_state::fashiong_hd63484_map);
}

void adp_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void adp_state::funland(machine_config &config)
{
	quickjac(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &adp_state::funland_mem);

	PALETTE(config.replace(), m_palette, palette_device::BLACK, 0x100);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &adp_state::ramdac_map);

	m_acrtc->set_addrmap(0, &adp_state::fstation_hd63484_map);
}

void adp_state::fstation(machine_config &config)
{
	funland(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &adp_state::fstation_mem);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(640, 480);
	screen.set_visarea_full();

	m_palette->set_init(FUNC(adp_state::fstation_palette));
}


ROM_START( quickjac )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "quick_jack_index_a.1.u2.bin", 0x00000, 0x10000, CRC(c2fba6fe) SHA1(f79e5913f9ded1e370cc54dd55860263b9c51d61) )
	ROM_LOAD16_BYTE( "quick_jack_index_a.2.u6.bin", 0x00001, 0x10000, CRC(210cb89b) SHA1(8eac60d40b60e845f9c02fee6c447f125ba5d1ab) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.1.u2.bin", 0x00000, 0x20000, CRC(73c27fc6) SHA1(12429bc0009b7754e08d2b6a5e1cd8251ab66e2d) )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.2.u6.bin", 0x00001, 0x20000, CRC(61d55be2) SHA1(bc17dc91fd1ef0f862eb0d7dbbbfa354a8403eb8) )
ROM_END

ROM_START( skattv )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f2_i.bin", 0x00000, 0x20000, CRC(3cb8b431) SHA1(e7930876b6cd4cba837c3da05d6948ef9167daea) )
	ROM_LOAD16_BYTE( "f2_ii.bin", 0x00001, 0x20000, CRC(0db1d2d5) SHA1(a29b0299352e0b2b713caf02aa7978f2a4b34e37) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "f1_i.bin", 0x00000, 0x20000, CRC(4869a889) SHA1(ad9f3fcdfd3630f9ad5b93a9d2738de9fc3514d3) )
	ROM_LOAD16_BYTE( "f1_ii.bin", 0x00001, 0x20000, CRC(17681537) SHA1(133685854b2080aaa3d0cced0287bc454d1f3bfc) )
ROM_END

ROM_START( skattva )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.1.u2.bin", 0x00000, 0x20000, CRC(68f82fe8) SHA1(d5f9cb600531cdd748616d8c042b6a151ebe205a) )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.2.u6.bin", 0x00001, 0x20000, CRC(4f927832) SHA1(bbe013005fd00dd42d12939eab5c80ec44a54b71) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.1.u2.bin", 0x00000, 0x20000, CRC(de6f275b) SHA1(0c396fa4d1975c8ccc4967d330b368c0697d2124) )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.2.u5.bin", 0x00001, 0x20000, CRC(af3e60f9) SHA1(c88976ea42cf29a092fdee18377b32ffe91e9f33) )
ROM_END

ROM_START( fashiong )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_i.bin", 0x00000, 0x80000, CRC(827a164d) SHA1(dc16380226cabdefbfd893cb50cbfca9e134be40) )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_ii.bin", 0x00001, 0x80000, CRC(5a2466d1) SHA1(c113a2295beed2011c70887a1f2fcdec00b055cb) )

	ROM_REGION16_BE( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_i.bin", 0x00000, 0x80000, CRC(d1ee9133) SHA1(e5fdfa303a3317f8f5fbdc03438ee97415afff4b) )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_ii.bin", 0x00001, 0x80000, CRC(07b1e722) SHA1(594cbe9edfea6b04a4e49d1c1594f1c3afeadef5) )

	ROM_REGION( 0x4000, "user1", 0 )
	//nvram - 16 bit
	ROM_LOAD16_BYTE( "m48z08post.bin", 0x0000, 0x2000, CRC(2d317a04) SHA1(c690c0d4b2259231d642ab5a30fcf389ba987b70) )
	ROM_LOAD16_BYTE( "m48z08posz.bin", 0x0001, 0x2000, CRC(7c5a4b78) SHA1(262d0d7f5b24e356ab54eb2450bbaa90e3fb5464) )
ROM_END

ROM_START( fashiong2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_f3_i.u2", 0x00000, 0x80000, CRC(2939279a) SHA1(75798ea41dd713d294ea341cbcdb73a76d9f63f4) )
	ROM_LOAD16_BYTE( "fashion_gambler_f3_ii.u6.bin", 0x00001, 0x80000, CRC(7d48e9ab) SHA1(603e946b95c53ee75c9ca10751316e723242424f) )

	ROM_REGION16_BE( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_video_f2_i.u2", 0x00000, 0x80000, CRC(54ea6f10) SHA1(a1284ec34e4e78acba08dc00d5ba47c3457531f8) )
	ROM_LOAD16_BYTE( "fashion_gambler_video_f2_ii.u5", 0x00001, 0x80000, CRC(c292a278) SHA1(9f66531ae9f202d364f47c7ed3551483fc9d27b0) )

	ROM_REGION( 0x4000, "user1", 0 )
	//nvram - 16 bit - taken from parent
	ROM_LOAD16_BYTE( "m48z08post.bin", 0x0000, 0x2000, CRC(2d317a04) SHA1(c690c0d4b2259231d642ab5a30fcf389ba987b70) )
	ROM_LOAD16_BYTE( "m48z08posz.bin", 0x0001, 0x2000, CRC(7c5a4b78) SHA1(262d0d7f5b24e356ab54eb2450bbaa90e3fb5464) )
ROM_END

ROM_START( funlddlx )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fldl_f6_1.bin", 0x00001, 0x80000, CRC(85c74040) SHA1(24a7d3e6acbaf73ef9817379bef64c38a9ff7896) )
	ROM_LOAD16_BYTE( "fldl_f6_2.bin", 0x00000, 0x80000, CRC(93bf1a4b) SHA1(5b4353feba1e0d4402cd26f4855e3803e6be43b9) )

	ROM_REGION16_BE( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "flv_f1_i.bin", 0x00000, 0x80000, CRC(286fccdc) SHA1(dd23deda625e486a7cfe1f3268731d10053a96e9) )
	ROM_LOAD16_BYTE( "flv_f1_ii.bin", 0x00001, 0x80000, CRC(2aa904e6) SHA1(864530b136dd488d619cc95f48e7dce8d93d88e0) )
ROM_END

ROM_START( fstation )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spielekoffer_9_sp_fun_station_f1.i", 0x00000, 0x80000, CRC(4572efbd) SHA1(e0a91d32ab4096767cafb743523d038f5e0d3238) )
	ROM_LOAD16_BYTE( "spielekoffer_9_sp_fun_station_f1.ii", 0x00001, 0x80000, CRC(a972184d) SHA1(1849e71e696039f07b7b67c4172c7999e81664c3) )

	ROM_REGION16_BE( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "spielekoffer_video_9_sp_f1.i", 0x00000, 0x80000, CRC(b6eb971e) SHA1(14e3272c66a82db0f77123974eea28f308209b1b) )
	ROM_LOAD16_BYTE( "spielekoffer_video_9_sp_f1.ii", 0x00001, 0x80000, CRC(64138dcb) SHA1(1b629915cba32f8f6164ae5075c175b522b4a323) )
ROM_END

} // Anonymous namespace


GAME( 1993, quickjac,  0,        quickjac, quickjac, adp_state, empty_init, ROT0, "ADP",     "Quick Jack",                        0 )
GAME( 1994, skattv,    0,        skattv,   skattv,   adp_state, empty_init, ROT0, "ADP",     "Skat TV",                           0 )
GAME( 1995, skattva,   skattv,   skattva,  skattva,  adp_state, empty_init, ROT0, "ADP",     "Skat TV (version TS3)",             0 )
GAME( 1997, fashiong,  0,        fashiong, skattv,   adp_state, empty_init, ROT0, "ADP",     "Fashion Gambler (set 1)",           0 )
GAME( 1997, fashiong2, fashiong, fashiong, skattv,   adp_state, empty_init, ROT0, "ADP",     "Fashion Gambler (set 2)",           0 )
GAME( 1999, funlddlx,  0,        funland,  skattv,   adp_state, empty_init, ROT0, "Stella",  "Funny Land de Luxe",                MACHINE_NOT_WORKING ) // keeps looping between F_IN and FOUL messages
GAME( 2000, fstation,  0,        fstation, fstation, adp_state, empty_init, ROT0, "ADP",     "Fun Station Spielekoffer 9 Spiele", MACHINE_NOT_WORKING ) // suntris crashes when executing HD63484 paint commands
