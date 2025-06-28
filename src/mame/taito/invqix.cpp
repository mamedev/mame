// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    invqix.cpp

Space Invaders / Qix Silver Anniversary Edition
Taito/Namco America inc.

INVADERS-QIX    Secondary label 351100210 (or 351100195 see below)
K11T0976B

+----------------------------------------------------------------+
|        "PCM"          "FRAM1"     "FRAM0"        "PRG"         |
|   +--------------+  +---------+ +---------+ +--------------+   |
|   |  F34-01 IC13 |  |CY7C1021B| |CY7C1021B| |  F34-02  IC2 |   |
|   +--------------+  +---------+ +---------+ +--------------+   |
|      +------+                                                  |
|      | OKI  |   LT1086                                         |
|      |M9810B|                 +-------------+                  |
|      |      |                 |             |                  |
|      +------+   LM1085        |   XILINX    | 44.8MHz          |
|      4.096MHz                 |   Spartan   |     +---------+  |
|                               |   XC2S50    |     |CY7C1021B|  |
| +---+                         |             |     +---------+  |
| | T |          +-------+      |             |       "WORK"     |
| | A |          |THS8134|      +-------------+                  |
| | 2 |          +-------+                                       |
| | 8 |                          93C46                           |
| | 0 |                +-------+  20MHz  +-------+               |
| | 0 |                |M5296FP|         | H8S/  |      BT1 SW3  |
| | A |                +-------+         | 2394  |               |
| | H |                                  | TE20  |               |
| +---+     TPD1030                      +-------+               |
|                X2                                              |
|                                                                |
+-------------+          JAMMA Connector           +-------------+
              +------------------------------------+

Main CPU: Renesas HD6412394TE20 H8S/2394 (ROMless microcontroller @ 20MHz)
   Sound: OKI MSM9810B 8-channel ADPCM audio
   Video: Sigma Xilinx Spartan XC2S50 FPGA
          TI THS8134 Triple 8-Bit, 80 MSPS Video D/A Converter
     OSC: 44.8MHz, 20MHz & 4.096MHz
  EEPROM: AT93C46 @ IC6
     RAM: Cypress CY7C1021B 64K x 16 Static RAM (44-pin TSOP) x 3 (silkscreened WORK, FRAM0 & FRAM1)
   Other: Renesas M5296FP Watchdog Timer IC with +5v constant-voltage power supply
          SW3 - TEST Push Button
          BT1 - 0.22F Memory Backup Capacitor
          LT1086 - 1.5A Low Dropout Positive Regulator
          LM1085 - 3A Low Dropout Positive Regulator
          TA8200AH - Dual Audio Power Amplifier (13Watts per channel)
          TDP1030 - 2-In-1 Low-Side Switch for Motor, Solenoid and Lamp Drive

Label    ROM Type       PCB silkscreened info   Notes
--------------------------------------------------------------------------
F34-01   ST 27C160      IC13 MSM27C1602 PCM     PCM Sound samples
F34-02   ST 27C160      IC2 M27C160 PRG         H8S/2394 program code
F34-03   OKI 27C1602B   IC13 MSM27C1602 PCM     Replaces F34-01 (See NOTE)

NOTE: There are known to exist PCBs labeled as K11T0976C vs the K11T0976B as listed above. This PCB
      has the OKI PCM rom labeled as F34-03 instead of F34-01 even though the data has NOT changed.
      F34-01 is the EEPROM ST 27C160 version
      F34-03 is OKI R27C1602B OTP-ROM version

R27C1602B is a 16Mbit electrically one time programmable ROM that can electrically switch between
           1,048,576-word x 16-bit and 2,097,152-word x 8-bit by the state of the BYTE# pin.

There are two known types of serial number labels.

One has the secondary label 351100195 with a serial number labeled:
 S/N: SICABN/Cxxxx (xxxx=number of PCB)
 SPC INV CAB NO COIN

The other has the secondary label 351100210 with a serial number labeled:
 S/N: SIURxxxx (xxxx=number of PCB)
 SPACE INVADERS U/R

Both boards have the same program code and either can be set to Coin or Free Play
as well as Up Right, Cocktail or Flip Screen from the service menu.

    Memory map:
    000000-1fffff: program ROM
    200000-20ffff: Work RAM
    400000-400003: OKI M9810
    600000-61ffff: VRAM
    620000-620005: video registers

    I/O map:
    port 1: player 2 inputs

    port 2 bit 3: must be "1" to avoid SERVICE ERROR
    port 2 bit 6: FPGA chip select
    port 2 bit 7: FPGA clock in

    port 3 bit 0: FPGA status (1 for ready)
    port 3 bit 1: FPGA download successful (1 if OK, 0 if failed)
    port 3 bit 2: EEPROM chip select
    port 3 bit 3: EEPROM clock
    port 3 bit 4: EEPROM data to EEPROM
    port 3 bit 5: EEPROM data from EEPROM

    port 4: player 1 inputs

    port 6 bit 3: FPGA data bit in

    port G bit 0: watchdog (toggled each frame)

    IRQ0 and IRQ1 are valid.  Mainline explicitly waits on IRQ1, but IRQ0 does a ton of processing.
    No other IRQ vectors are valid.

    main loop at 117ea:
    117ea: jsr WaitForIRQ1
    117ee: jsr ToggleBit0OfPortG
    117f2: jsr 11306
    117f6: jsr 1918
    117fa: bra 117ea

***************************************************************************/

#include "emu.h"
#include "cpu/h8/h8s2357.h"
#include "sound/okim9810.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class invqix_state : public driver_device
{
public:
	invqix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_vram(*this, "vram")
	{ }

	void invqix(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t port3_r();
	void port3_w(uint8_t data);
	uint8_t port5_r();
	void port5_w(uint8_t data);
	uint8_t port6_r();
	void port6_w(uint8_t data);
	uint8_t porta_r();
	uint8_t portg_r();

	void vctl_w(uint16_t data);

	void prg_map(address_map &map) ATTR_COLD;

	// devices
	required_device<h8s2394_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<uint16_t> m_vram;

	uint16_t m_vctl = 0;      // 0000 for normal, 0001 for flip, 0100 when going to change (blank?)
};


void invqix_state::video_start()
{
	save_item(NAME(m_vctl));
}

uint32_t invqix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// this means freeze or blank or something
	if (m_vctl == 0x100)
	{
		return 0;
	}

	if ((m_vctl & 0xfffe) == 0x0000)
	{
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			uint16_t const *const src = &m_vram[((BIT(m_vctl, 0) ? (255 - y) : y) << 8)];
			uint32_t *const dst = &bitmap.pix(y);
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				uint16_t const pen_data = src[BIT(m_vctl, 0) ? (255 - x) : x];
				uint8_t const b = pal5bit(pen_data & 0x001f);
				uint8_t const g = pal5bit((pen_data & 0x03e0) >> 5);
				uint8_t const r = pal5bit((pen_data & 0x7c00) >> 10);
				dst[x] = (r << 16) | (g << 8) | b;
			}
		}
	}
	else
	{
		logerror("invqix: Unhandled vctl %04x\n", m_vctl);
	}

	return 0;
}

uint8_t invqix_state::port3_r()
{
	return (m_eeprom->do_read() << 5) | 0x03;
}

void invqix_state::port3_w(uint8_t data)
{
	m_eeprom->cs_write(BIT(data, 2));
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->clk_write(BIT(data, 3));
}

uint8_t invqix_state::port5_r()
{
	return 0;
}

void invqix_state::port5_w(uint8_t data)
{
}

uint8_t invqix_state::port6_r()
{
	return 0;
}

void invqix_state::port6_w(uint8_t data)
{
}

uint8_t invqix_state::porta_r()
{
	return 0xf0;
}

uint8_t invqix_state::portg_r()
{
	return 0;
}

void invqix_state::vctl_w(uint16_t data)
{
	m_vctl = data;
}

void invqix_state::prg_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("program", 0);
	map(0x200000, 0x21ffff).ram();
	map(0x400001, 0x400001).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x400000, 0x400000).w("oki", FUNC(okim9810_device::write));
	map(0x400002, 0x400002).r("oki", FUNC(okim9810_device::read));
	map(0x600000, 0x61ffff).ram().share(m_vram);
	map(0x620004, 0x620005).w(FUNC(invqix_state::vctl_w));
}

static INPUT_PORTS_START( invqix )
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) // coin 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Left 1 player start")   // start A-1 ("left start" - picks Space Invaders)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )   // coin 2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Left 2 players start")   // start A-2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("Right 2 players start") // start B-2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Right 1 player start")   // start B-1 ("Right start" - picks Qix)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void invqix_state::invqix(machine_config &config)
{
	H8S2394(config, m_maincpu, XTAL(20'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &invqix_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(invqix_state::irq1_line_hold));
	m_maincpu->set_periodic_int(FUNC(invqix_state::irq0_line_hold), attotime::from_hz(60));
	m_maincpu->read_port1().set_ioport("P1");
	m_maincpu->read_port2().set_ioport("SYSTEM");
	m_maincpu->write_port2().set_nop();
	m_maincpu->read_port3().set(FUNC(invqix_state::port3_r));
	m_maincpu->write_port3().set(FUNC(invqix_state::port3_w));
	m_maincpu->read_port4().set_ioport("P4");
	m_maincpu->read_port5().set(FUNC(invqix_state::port5_r));
	m_maincpu->write_port5().set(FUNC(invqix_state::port5_w));
	m_maincpu->read_port6().set(FUNC(invqix_state::port6_r));
	m_maincpu->write_port6().set(FUNC(invqix_state::port6_w));
	m_maincpu->read_porta().set(FUNC(invqix_state::porta_r));
	m_maincpu->read_portg().set(FUNC(invqix_state::portg_r));
	m_maincpu->write_portg().set_nop();

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(invqix_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 256-1, 0, 240-1);

	SPEAKER(config, "speaker", 2).front();

	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "speaker", 0.80, 0);
	oki.add_route(1, "speaker", 0.80, 1);

	EEPROM_93C46_16BIT(config, "eeprom").default_value(0);
}

ROM_START( invqix )
	ROM_REGION16_BE(0x200000, "program", 0)
	ROM_LOAD16_WORD_SWAP( "f34-02.ic2",   0x000000, 0x200000, CRC(035ace40) SHA1(e61f180024102c7a136b1c7f974c71e5dc698a1e) ) // VER 2.03 2003/10/25 18:15

	ROM_REGION(0x1000000, "oki", 0)
	ROM_LOAD( "f34-01.ic13",  0x000000, 0x200000, CRC(7b055722) SHA1(8152bf04a58de15aefc4244e40733275e21818e1) ) /* Can also be labeled F34-03 based on ROM chip type */

	ROM_REGION16_BE(0x80, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP( "93c46.ic6", 0x000000, 0x000080, CRC(564b744e) SHA1(4d9ea7dc253797c513258d07a936dfb63d8ed18c) )
ROM_END

} // anonymous namespace


GAME( 2003, invqix, 0, invqix, invqix, invqix_state, empty_init, ROT270, "Taito / Namco", "Space Invaders / Qix Silver Anniversary Edition (Ver. 2.03)", MACHINE_SUPPORTS_SAVE )
