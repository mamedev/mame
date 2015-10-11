// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    invqix.c

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

class invqix_state : public driver_device
{
public:
	invqix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_vram(*this, "vram")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER(port3_r);
	DECLARE_WRITE16_MEMBER(port3_w);
	DECLARE_READ16_MEMBER(port5_r);
	DECLARE_WRITE16_MEMBER(port5_w);
	DECLARE_READ16_MEMBER(port6_r);
	DECLARE_WRITE16_MEMBER(port6_w);
	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portg_r);

	DECLARE_WRITE16_MEMBER(vctl_w);

protected:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<UINT16> m_vram;

	// driver_device overrides
	virtual void video_start();

private:
	UINT16 m_vctl;      // 0000 for normal, 0001 for flip, 0100 when going to change (blank?)
};


void invqix_state::video_start()
{
	save_item(NAME(m_vctl));
}

UINT32 invqix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;

	// this means freeze or blank or something
	if (m_vctl == 0x100)
	{
		return 0;
	}

	if (m_vctl == 0x0000)
	{
		for(y=0;y<256;y++)
		{
			for(x=0;x<256;x++)
			{
				UINT8 r,g,b;
				int pen_data;

				pen_data = (m_vram[(x+y*256)]);
				b = (pen_data & 0x001f);
				g = (pen_data & 0x03e0) >> 5;
				r = (pen_data & 0x7c00) >> 10;
				r = (r << 3) | (r & 0x7);
				g = (g << 3) | (g & 0x7);
				b = (b << 3) | (b & 0x7);

				if(cliprect.contains(x, y))
					bitmap.pix32(y, x) = r << 16 | g << 8 | b;
			}
		}
	}
	else if (m_vctl == 0x0001)  // flip
	{
		for(y=0;y<256;y++)
		{
			for(x=0;x<256;x++)
			{
				UINT8 r,g,b;
				int pen_data;

				pen_data = (m_vram[(256-x)+((256-y)*256)]);
				b = (pen_data & 0x001f);
				g = (pen_data & 0x03e0) >> 5;
				r = (pen_data & 0x7c00) >> 10;
				r = (r << 3) | (r & 0x7);
				g = (g << 3) | (g & 0x7);
				b = (b << 3) | (b & 0x7);

				if(cliprect.contains(x, y))
					bitmap.pix32(y, x) = r << 16 | g << 8 | b;
			}
		}
	}
	else
	{
		logerror("invqix: Unhandled vctl %04x\n", m_vctl);
	}

	return 0;
}

READ16_MEMBER(invqix_state::port3_r)
{
	return (m_eeprom->do_read() << 5) | 0x03;
}

WRITE16_MEMBER(invqix_state::port3_w)
{
	m_eeprom->cs_write((data >> 2) & 1);
	m_eeprom->di_write((data >> 4) & 1);
	m_eeprom->clk_write((data >> 3) & 1);
}

READ16_MEMBER(invqix_state::port5_r)
{
	return 0;
}

WRITE16_MEMBER(invqix_state::port5_w)
{
}

READ16_MEMBER(invqix_state::port6_r)
{
	return 0;
}

WRITE16_MEMBER(invqix_state::port6_w)
{
}

READ16_MEMBER(invqix_state::porta_r)
{
	return 0xf0;
}

READ16_MEMBER(invqix_state::portg_r)
{
	return 0;
}

WRITE16_MEMBER(invqix_state::vctl_w)
{
	m_vctl = data;
}

static ADDRESS_MAP_START(invqix_prg_map, AS_PROGRAM, 16, invqix_state)
	AM_RANGE(0x000000, 0x1fffff) AM_ROM AM_REGION("program", 0)
	AM_RANGE(0x200000, 0x21ffff) AM_RAM
	AM_RANGE(0x400000, 0x400001) AM_DEVWRITE8("oki", okim9810_device, write_TMP_register, 0x00ff)
	AM_RANGE(0x400000, 0x400001) AM_DEVWRITE8("oki", okim9810_device, write, 0xff00)
	AM_RANGE(0x400002, 0x400003) AM_DEVREAD8("oki", okim9810_device, read, 0xff00)
	AM_RANGE(0x600000, 0x61ffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x620004, 0x620005) AM_WRITE(vctl_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(invqix_io_map, AS_IO, 16, invqix_state)
	AM_RANGE(h8_device::PORT_1, h8_device::PORT_1) AM_READ_PORT("P1")
	AM_RANGE(h8_device::PORT_2, h8_device::PORT_2) AM_READ_PORT("SYSTEM") AM_WRITENOP
	AM_RANGE(h8_device::PORT_3, h8_device::PORT_3) AM_READWRITE(port3_r, port3_w)
	AM_RANGE(h8_device::PORT_4, h8_device::PORT_4) AM_READ_PORT("P4")
	AM_RANGE(h8_device::PORT_5, h8_device::PORT_5) AM_READWRITE(port5_r, port5_w)
	AM_RANGE(h8_device::PORT_6, h8_device::PORT_6) AM_READWRITE(port6_r, port6_w)
	AM_RANGE(h8_device::PORT_A, h8_device::PORT_A) AM_READ(porta_r)
	AM_RANGE(h8_device::PORT_G, h8_device::PORT_G) AM_READ(portg_r) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( invqix )
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1 ) // coin 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 ) PORT_NAME("Left 1 player start")   // start A-1 ("left start" - picks Space Invaders)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 ) // service
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN2 )   // coin 2
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START3 ) PORT_NAME("Left 2 players start")   // start A-2
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

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
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( invqix, invqix_state )
	MCFG_CPU_ADD("maincpu", H8S2394, XTAL_20MHz)
	MCFG_CPU_PROGRAM_MAP(invqix_prg_map)
	MCFG_CPU_IO_MAP(invqix_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", invqix_state,  irq1_line_hold)
	MCFG_CPU_PERIODIC_INT_DRIVER(invqix_state, irq0_line_hold,  60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(invqix_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 256, 0, 240)
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_PALETTE_ADD("palette", 65536)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM9810_ADD("oki", XTAL_4_096MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DEFAULT_VALUE(0)
MACHINE_CONFIG_END

ROM_START( invqix )
	ROM_REGION(0x200000, "program", 0)
	ROM_LOAD( "f34-02.ic2",   0x000000, 0x200000, CRC(035ace40) SHA1(e61f180024102c7a136b1c7f974c71e5dc698a1e) )

	ROM_REGION(0x1000000, "oki", 0)
	ROM_LOAD( "f34-01.ic13",  0x000000, 0x200000, CRC(7b055722) SHA1(8152bf04a58de15aefc4244e40733275e21818e1) ) /* Can also be labeled F34-03 based on ROM chip type */

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP( "93c46.ic6", 0x000000, 0x000080, CRC(564b744e) SHA1(4d9ea7dc253797c513258d07a936dfb63d8ed18c) )
ROM_END

GAME( 2003, invqix, 0, invqix, invqix, driver_device, 0, ROT270, "Taito / Namco", "Space Invaders / Qix Silver Anniversary Edition (Ver. 2.03)", MACHINE_SUPPORTS_SAVE )
