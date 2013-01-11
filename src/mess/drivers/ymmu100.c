/*************************************************************************************

    Yamaha MU-100 : 32-voice polyphonic/multitimbral General MIDI/GS/XG tone module
    Preliminary driver by R. Belmont

    CPU: Hitachi H8S/2655 (HD6432655F), strapped for mode 4 (24-bit address, 16-bit data, no internal ROM)
    Sound ASIC: Yamaha XS725A0
    RAM: 1 MSM51008 (1 meg * 1 bit = 128KBytes)

    I/O ports from service manual:

    Port 1:
        0 - LCD data, SW data, LED 1
        1 - LCD data, SW data, LED 2
        2 - LCD data, SW data, LED 3
        3 - LCD data, SW data, LED 4
        4 - LCD data, SW data, LED 5
        5 - LCD data, SW strobe data
        6 - LCD data, SW strobe data
        7 - LCD data, SW data, LED 6

    Port 2:
        0 - (out) LCD control RS
        1 - (out) LCD control R/W
        2 - (out) LCD control E
        3 - (out) LCD contrast A
        4 - (out) LCD contrast B
        5 - (out) LCD contrast C
        6 - (out) 1 MHz clock for serial
        7 - NC

    Port 3:
        4 - (out) A/D gain control 1
        5 - (out) A/D gain control 2

    Port 5:
        3 - (out) Reset signal for rotary encoder

    Port 6:
        1 - NC
        2 - (out) PB select (SW1)
        3 - (out) PB select (SW2)
        4 - (out) reset PB
        5 - (out) reset SWP30 (sound chip)
        6 - NC
        7 - (in) Plug detection for A/D input

    Port A:
        5 - (in) Off Line Detection
        6 - (out) Signal for rotary encoder (REB)
        7 - (out) Signal for rotary encoder (REA)

    Port F:
        0 - (out) LED,SW Strobe data latch
        1 - (out) SW data read control
        2 - (out) PB select (SW4)

    Port G:
        0 - (out) PB select (SW3)

    Analog input channels:
        0 - level input R
        2 - level output L
        4 - host SW type switch position
        6 - battery voltage
        7 - model check (GND for MU100)

**************************************************************************************/

#include "emu.h"
#include "cpu/h83002/h8.h"
#include "video/hd44780.h"

static INPUT_PORTS_START( mu100 )
INPUT_PORTS_END

class mu100_state : public driver_device
{
public:
	enum {
		P2_LCD_RS     = 0x01,
		P2_LCD_RW     = 0x02,
		P2_LCD_ENABLE = 0x04
	};

	mu100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_lcd(*this, "lcd")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcd;

	UINT8 cur_p1, cur_p2;

	DECLARE_READ8_MEMBER(adc7_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(p2_r);

	virtual void palette_init();
};


static ADDRESS_MAP_START( mu100_map, AS_PROGRAM, 16, mu100_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x200000, 0x21ffff) AM_RAM // 128K work RAM
ADDRESS_MAP_END

// model detect.  pulled to GND (0) on MU100.
READ8_MEMBER(mu100_state::adc7_r)
{
	return 0;
}

WRITE8_MEMBER(mu100_state::p1_w)
{
	cur_p1 = data;
}

READ8_MEMBER(mu100_state::p1_r)
{
	if((cur_p2 & P2_LCD_ENABLE)) {
		if(cur_p2 & P2_LCD_RW) {
			if(cur_p2 & P2_LCD_RS)
				return m_lcd->data_read(space, offset);
			else
				return m_lcd->control_read(space, offset);
		} else
			return 0x00;
	}

	return 0xff;
}

WRITE8_MEMBER(mu100_state::p2_w)
{
	// LCB enable edge
	if(!(cur_p2 & P2_LCD_ENABLE) && (data & P2_LCD_ENABLE)) {
		if(!(cur_p2 & P2_LCD_RW)) {
			if(cur_p2 & P2_LCD_RS)
				m_lcd->data_write(space, offset, cur_p1);
			else
				m_lcd->control_write(space, offset, cur_p1);
		}
	}
	cur_p2 = data;
}

READ8_MEMBER(mu100_state::p2_r)
{
	return cur_p2;
}

void mu100_state::palette_init()
{
	palette_set_color(machine(), 0, MAKE_RGB(255, 255, 255));
	palette_set_color(machine(), 1, MAKE_RGB(0, 0, 0));
}

static ADDRESS_MAP_START( mu100_iomap, AS_IO, 8, mu100_state )
	AM_RANGE(H8_PORT_1,  H8_PORT_1)  AM_READWRITE(p1_r, p1_w)
	AM_RANGE(H8_PORT_2,  H8_PORT_2)  AM_READWRITE(p2_r, p2_w)
	AM_RANGE(H8_ADC_7_H, H8_ADC_7_L) AM_READ(adc7_r)
ADDRESS_MAP_END

static HD44780_INTERFACE( lcd_config )
{
	4,
	20,
	NULL
};

static MACHINE_CONFIG_START( mu100, mu100_state )
	MCFG_CPU_ADD( "maincpu", H8S2655, XTAL_16MHz )
	MCFG_CPU_PROGRAM_MAP( mu100_map )
	MCFG_CPU_IO_MAP( mu100_iomap )

	MCFG_HD44780_ADD("lcd", lcd_config)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate, asynchronous updating anyway */
	MCFG_SCREEN_UPDATE_DEVICE("lcd", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_PALETTE_LENGTH(2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

ROM_START( mu100 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x2800000, "waverom", 0 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x800000, 0x1000000, CRC(d4483a43) SHA1(5bfd0762dea8598eda19db20251dac20e31fa02c) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x800002, 0x1000000, CRC(c5af4501) SHA1(1c88de197c36382311053add8b19a5740802cb78) )

	ROM_REGION( 0x0860, "lcd", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END

CONS( 1997, mu100, 0, 0, mu100, mu100, driver_device, 0, "Yamaha", "MU100", GAME_NOT_WORKING )
