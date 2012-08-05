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
|                               |   Spartan   |     +---------+  |
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
     OSC: 20MHz & 44.8MHz
  EEPROM: AT93C46 @ IC6
     RAM: Cypress CY7C1021B 64K x 16 Static RAM (44-pin TSOP) x 3 (silkscreened WORK, FRAM0 & FRAM1)
   Other: Renesas M5296FP Watchdog Timer IC with +5v constant-voltage power supply
          SW3 - TEST Push Button
          BT1 - 0.22F Memory Backup Capacitor
          LT1086 - 1.5A Low Dropout Positive Regulator
          LM1085 - 3A Low Dropout Positive Regulator
          TA8200AH - Dual Audio Power Amplifier (13Watts per channel)
          TDP1030 - 2-In-1 Low-Side Switch for Motor, Solenoid and Lamp Drive

ROMS: F34-01 (IC13 M27C160 silkscreened PCM)
      F34-02 (IC2  M27C160 silkscreened PRG)

There are two known types of PCB.

One has the secondary label 351100195 with a serial number labeled:
 S/N: SICABN/Cxxxx (xxxx=number of PCB)
 SPC INV CAB NO COIN

The other has the secondary label 351100210 with a serial number labeled:
 S/N: SIURxxxx (xxxx=number of PCB)
 SPACE INVADERS U/R

    Memory map:
    000000-1fffff: program ROM
    200000-20ffff: Work RAM
    400000-400001: ??? (M5296 Watchdog??)
    600000-61ffff: VRAM

    I/O map:
    port 2 bit 6: FPGA chip select
    port 2 bit 7: FPGA clock in

    port 3 bit 0: FPGA status (1 for ready)
    port 3 bit 1: FPGA download successful (1 if OK, 0 if failed)
    port 3 bit 2: EEPROM chip select
    port 3 bit 3: EEPROM clock
    port 3 bit 4: EEPROM data to EEPROM
    port 3 bit 5: EEPROM data from EEPROM

    port 6 bit 3: FPGA data bit in

    port G bit 0: framebuffer bank select?  toggled each frame

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
#include "cpu/h83002/h8.h"
#include "sound/okim9810.h"
#include "machine/eeprom.h"

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

	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_READ8_MEMBER(port3_r);
	DECLARE_WRITE8_MEMBER(port3_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(porta_r);

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_device> m_eeprom;
	required_shared_ptr<UINT16> m_vram;

	// driver_device overrides
	virtual void video_start();
};


void invqix_state::video_start()
{
}

UINT32 invqix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;

	for(y=0;y<256;y++)
	{
		for(x=0;x<256;x++)
		{
			UINT8 r,g,b;
			int pen_data;

			pen_data = (m_vram[x+y*256]);
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

	return 0;
}

READ8_MEMBER(invqix_state::port1_r)
{
	return 0xff;
}

READ8_MEMBER(invqix_state::port2_r)
{
	return 1;
}

WRITE8_MEMBER(invqix_state::port2_w)
{

}

READ8_MEMBER(invqix_state::port3_r)
{
	return (m_eeprom->read_bit() << 5) | 0x03;
}

WRITE8_MEMBER(invqix_state::port3_w)
{
	m_eeprom->set_cs_line(((data >> 2) & 1) ^ 1);
	m_eeprom->write_bit((data >> 4) & 1);
	m_eeprom->set_clock_line((data >> 3) & 1);
}

READ8_MEMBER(invqix_state::port6_r)
{
	return 0;
}

WRITE8_MEMBER(invqix_state::port6_w)
{

}

READ8_MEMBER(invqix_state::porta_r)
{
	return 0xff;
}

static ADDRESS_MAP_START(invqix_prg_map, AS_PROGRAM, 32, invqix_state)
	AM_RANGE(0x000000, 0x1fffff) AM_ROM AM_REGION("program", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x600000, 0x61ffff) AM_RAM AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(invqix_io_map, AS_IO, 8, invqix_state)
	AM_RANGE(H8_PORT_1, H8_PORT_1) AM_READ(port1_r)
	AM_RANGE(H8_PORT_2, H8_PORT_2) AM_READWRITE(port2_r, port2_w)
	AM_RANGE(H8_PORT_3, H8_PORT_3) AM_READWRITE(port3_r, port3_w)
	AM_RANGE(H8_PORT_6, H8_PORT_6) AM_READWRITE(port6_r, port6_w)
	AM_RANGE(H8_PORT_A, H8_PORT_A) AM_READ(porta_r)
	AM_RANGE(H8_PORT_G, H8_PORT_G) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( invqix )
INPUT_PORTS_END

static MACHINE_CONFIG_START( invqix, invqix_state )
	MCFG_CPU_ADD("maincpu", H8S2394, XTAL_20MHz)
	MCFG_CPU_PROGRAM_MAP(invqix_prg_map)
	MCFG_CPU_IO_MAP(invqix_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq1_line_hold)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold, 60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(invqix_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 256, 0, 240)

	MCFG_PALETTE_LENGTH(65536)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM9810_ADD("oki", XTAL_4_096MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_EEPROM_93C46_ADD("eeprom")
	MCFG_EEPROM_DEFAULT_VALUE(0)
MACHINE_CONFIG_END

ROM_START( invqix )
	ROM_REGION(0x200000, "program", 0)
	ROM_LOAD( "f34-02.ic2",   0x000000, 0x200000, CRC(035ace40) SHA1(e61f180024102c7a136b1c7f974c71e5dc698a1e) )

	ROM_REGION(0x1000000, "oki", 0)
	ROM_LOAD( "f34-01.ic13",  0x000000, 0x200000, CRC(7b055722) SHA1(8152bf04a58de15aefc4244e40733275e21818e1) )

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP( "93c46.ic6", 0x000000, 0x000080, CRC(564b744e) SHA1(4d9ea7dc253797c513258d07a936dfb63d8ed18c) )
ROM_END

GAME(2003, invqix, 0, invqix, invqix, invqix_state, 0, ROT270, "Namco/Taito", "Space Invaders / Qix Silver Anniversary Edition (Ver. 2.03)", GAME_NOT_WORKING )
