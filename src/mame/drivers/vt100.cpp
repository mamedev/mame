// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT100 driver by Miodrag Milanovic

        29/04/2009 Preliminary driver.

        TODO: keyboard doesn't work properly, kb uart comms issue?
        TODO: split keyboard off as a synchronous serial device?
        TODO: vt100 gives a '2' error on startup indicating bad nvram checksum
              adding the serial nvram support should fix this
        TODO: support for the on-AVO character set roms
        TODO: finish support for the on-cpu board alternate character set rom

        An enormous amount of useful info can be derived from the VT125 technical manual:
        http://www.bitsavers.org/pdf/dec/terminal/vt100/EK-VT100-TM-003_VT100_Technical_Manual_Jul82.pdf starting on page 6-70, pdf page 316
        And its schematics:
        http://bitsavers.org/pdf/dec/terminal/vt125/MP01053_VT125_Mar82.pdf
****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "sound/beep.h"
#include "video/vtvideo.h"
#include "vt100.lh"

#define RS232_TAG       "rs232"
#define COM5016T_TAG    "com5016t"

class vt100_state : public driver_device
{
public:
	vt100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "vt100_video"),
		m_speaker(*this, "beeper"),
		m_uart(*this, "i8251"),
		m_dbrg(*this, COM5016T_TAG),
		m_p_ram(*this, "p_ram")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<vt100_video_device> m_crtc;
	required_device<beep_device> m_speaker;
	required_device<i8251_device> m_uart;
	required_device<com8116_device> m_dbrg;
	DECLARE_READ8_MEMBER(vt100_flags_r);
	DECLARE_WRITE8_MEMBER(vt100_keyboard_w);
	DECLARE_READ8_MEMBER(vt100_keyboard_r);
	DECLARE_WRITE8_MEMBER(vt100_baud_rate_w);
	DECLARE_WRITE8_MEMBER(vt100_nvr_latch_w);
	DECLARE_READ8_MEMBER(vt100_read_video_ram_r);
	DECLARE_WRITE_LINE_MEMBER(vt100_clear_video_interrupt);
	required_shared_ptr<UINT8> m_p_ram;
	bool m_keyboard_int;
	bool m_receiver_int;
	bool m_vertical_int;
	bool m_key_scan;
	UINT8 m_key_code;
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_vt100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vt100_vertical_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	IRQ_CALLBACK_MEMBER(vt100_irq_callback);
	UINT8 bit_sel(UINT8 data);
};




static ADDRESS_MAP_START(vt100_mem, AS_PROGRAM, 8, vt100_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_ROM  // ROM ( 4 * 2K)
	AM_RANGE( 0x2000, 0x2bff ) AM_RAM AM_SHARE("p_ram") // Screen and scratch RAM
	AM_RANGE( 0x2c00, 0x2fff ) AM_RAM  // AVO Screen RAM
	AM_RANGE( 0x3000, 0x3fff ) AM_RAM  // AVO Attribute RAM (4 bits wide)
	// 0x4000, 0x7fff is unassigned
	AM_RANGE( 0x8000, 0x9fff ) AM_ROM  // Program memory expansion ROM (4 * 2K)
	AM_RANGE( 0xa000, 0xbfff ) AM_ROM  // Program memory expansion ROM (1 * 8K)
	// 0xc000, 0xffff is unassigned
ADDRESS_MAP_END

static ADDRESS_MAP_START(vt180_mem, AS_PROGRAM, 8, vt100_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_ROM
	AM_RANGE( 0x2000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vt180_io, AS_IO, 8, vt100_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

// 0 - XMIT flag H
// 1 - Advance Video L
// 2 - Graphics Flag L
// 3 - Option present H
// 4 - Even field L
// 5 - NVR data H
// 6 - LBA 7 H
// 7 - Keyboard TBMT H
READ8_MEMBER( vt100_state::vt100_flags_r )
{
	UINT8 ret = 0;
	ret |= m_crtc->lba7_r(space, 0) << 6;
	ret |= m_keyboard_int << 7;
	return ret;
}

UINT8 vt100_state::bit_sel(UINT8 data)
{
	if (!BIT(data,7)) return 0x70;
	if (!BIT(data,6)) return 0x60;
	if (!BIT(data,5)) return 0x50;
	if (!BIT(data,4)) return 0x40;
	if (!BIT(data,3)) return 0x30;
	if (!BIT(data,2)) return 0x20;
	if (!BIT(data,1)) return 0x10;
	if (!BIT(data,0)) return 0x00;
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(vt100_state::keyboard_callback)
{
	UINT8 i, code;
	char kbdrow[8];
	if (m_key_scan)
	{
		for(i = 0; i < 16; i++)
		{
			sprintf(kbdrow,"LINE%X", i);
			code =  ioport(kbdrow)->read();
			if (code < 0xff)
			{
				m_keyboard_int = 1;
				m_key_code = i | bit_sel(code);
				m_maincpu->set_input_line(0, HOLD_LINE);
				break;
			}
		}
	}
}


WRITE8_MEMBER( vt100_state::vt100_keyboard_w )
{
	m_speaker->set_frequency(786); // 7.945us per serial clock = ~125865.324hz, / 160 clocks per char = ~ 786 hz
	output_set_value("online_led",BIT(data, 5) ? 0 : 1);
	output_set_value("local_led", BIT(data, 5));
	output_set_value("locked_led",BIT(data, 4) ? 0 : 1);
	output_set_value("l1_led", BIT(data, 3) ? 0 : 1);
	output_set_value("l2_led", BIT(data, 2) ? 0 : 1);
	output_set_value("l3_led", BIT(data, 1) ? 0 : 1);
	output_set_value("l4_led", BIT(data, 0) ? 0 : 1);
	m_key_scan = BIT(data, 6);
	m_speaker->set_state(BIT(data, 7));
}

READ8_MEMBER( vt100_state::vt100_keyboard_r )
{
	return m_key_code;
}

WRITE8_MEMBER( vt100_state::vt100_baud_rate_w )
{
	m_dbrg->str_w(data & 0x0f);
	m_dbrg->stt_w(data >> 4);
}

WRITE8_MEMBER( vt100_state::vt100_nvr_latch_w )
{
}

static ADDRESS_MAP_START(vt100_io, AS_IO, 8, vt100_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// 0x00, 0x01 PUSART  (Intel 8251)
	AM_RANGE (0x00, 0x00) AM_DEVREADWRITE("i8251", i8251_device, data_r, data_w)
	AM_RANGE (0x01, 0x01) AM_DEVREADWRITE("i8251", i8251_device, status_r, control_w)
	// 0x02 Baud rate generator
	AM_RANGE (0x02, 0x02) AM_WRITE(vt100_baud_rate_w)
	// 0x22 Modem buffer
	// AM_RANGE (0x22, 0x22)
	// 0x42 Flags buffer
	AM_RANGE (0x42, 0x42) AM_READ(vt100_flags_r)
	// 0x42 Brightness D/A latch
	AM_RANGE (0x42, 0x42) AM_DEVWRITE("vt100_video", vt100_video_device, brightness_w)
	// 0x62 NVR latch
	AM_RANGE (0x62, 0x62) AM_WRITE(vt100_nvr_latch_w)
	// 0x82 Keyboard UART data output
	AM_RANGE (0x82, 0x82) AM_READ(vt100_keyboard_r)
	// 0x82 Keyboard UART data input
	AM_RANGE (0x82, 0x82) AM_WRITE(vt100_keyboard_w)
	// 0xA2 Video processor DC012
	AM_RANGE (0xa2, 0xa2) AM_DEVWRITE("vt100_video", vt100_video_device, dc012_w)
	// 0xC2 Video processor DC011
	AM_RANGE (0xc2, 0xc2) AM_DEVWRITE("vt100_video", vt100_video_device, dc011_w)
	// 0xE2 Graphics port
	// AM_RANGE (0xe2, 0xe2)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vt100 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num Enter") PORT_CODE(KEYCODE_ENTER_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num Return") PORT_CODE(KEYCODE_ENTER_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line feed") PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_START("LINEA")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("No scroll") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_START("LINEB")
		PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Setup") PORT_CODE(KEYCODE_F5)
	PORT_START("LINEC")
		PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_START("LINED")
		PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_START("LINEE")
		PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_START("LINEF")
		PORT_BIT(0x7F, IP_ACTIVE_LOW,  IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // Always return 0x7f on last scan line
INPUT_PORTS_END

UINT32 vt100_state::screen_update_vt100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_crtc->video_update(bitmap, cliprect);
	return 0;
}


//Interrupts
// in latch A3 - keyboard
//          A4 - receiver
//          A5 - vertical fequency
//          all other set to 1
IRQ_CALLBACK_MEMBER(vt100_state::vt100_irq_callback)
{
	UINT8 ret = 0xc7 | (m_keyboard_int << 3) | (m_receiver_int << 4) | (m_vertical_int << 5);
	m_receiver_int = 0;
	return ret;
}

void vt100_state::machine_start()
{
}

void vt100_state::machine_reset()
{
	m_keyboard_int = 0;
	m_receiver_int = 0;
	m_vertical_int = 0;
	m_speaker->set_frequency(786); // 7.945us per serial clock = ~125865.324hz, / 160 clocks per char = ~ 786 hz
	output_set_value("online_led",1);
	output_set_value("local_led", 0);
	output_set_value("locked_led",1);
	output_set_value("l1_led", 1);
	output_set_value("l2_led", 1);
	output_set_value("l3_led", 1);
	output_set_value("l4_led", 1);

	m_key_scan = 0;
}

READ8_MEMBER( vt100_state::vt100_read_video_ram_r )
{
	return m_p_ram[offset];
}

WRITE_LINE_MEMBER( vt100_state::vt100_clear_video_interrupt )
{
	m_vertical_int = 0;
}

INTERRUPT_GEN_MEMBER(vt100_state::vt100_vertical_interrupt)
{
	m_vertical_int = 1;
	device.execute().set_input_line(0, HOLD_LINE);
}

/* F4 Character Displayer */
static const gfx_layout vt100_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 2 x 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( vt100 )
	GFXDECODE_ENTRY( "chargen", 0x0000, vt100_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( vt100, vt100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_24_8832MHz / 9)
	MCFG_CPU_PROGRAM_MAP(vt100_mem)
	MCFG_CPU_IO_MAP(vt100_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vt100_state,  vt100_vertical_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(vt100_state,vt100_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*10, 25*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*10-1, 0, 25*10-1)
	MCFG_SCREEN_UPDATE_DRIVER(vt100_state, screen_update_vt100)
	MCFG_SCREEN_PALETTE("vt100_video:palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "vt100_video:palette", vt100)
//  MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_DEFAULT_LAYOUT( layout_vt100 )

	MCFG_DEVICE_ADD("vt100_video", VT100_VIDEO, 0)
	MCFG_VT_SET_SCREEN("screen")
	MCFG_VT_CHARGEN("chargen")
	MCFG_VT_VIDEO_RAM_CALLBACK(READ8(vt100_state, vt100_read_video_ram_r))
	MCFG_VT_VIDEO_CLEAR_VIDEO_INTERRUPT_CALLBACK(WRITELINE(vt100_state, vt100_clear_video_interrupt))

	MCFG_DEVICE_ADD("i8251", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("i8251", i8251_device, write_dsr))

	MCFG_DEVICE_ADD(COM5016T_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("i8251", i8251_device, write_txc))

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", vt100_state, keyboard_callback, attotime::from_hz(800))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( vt180, vt100 )
	MCFG_CPU_ADD("z80cpu", Z80, XTAL_24_8832MHz / 9)
	MCFG_CPU_PROGRAM_MAP(vt180_mem)
	MCFG_CPU_IO_MAP(vt180_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( vt102, vt100 )
	MCFG_CPU_REPLACE("maincpu",I8085A, XTAL_24_8832MHz / 9)
	MCFG_CPU_PROGRAM_MAP(vt100_mem)
	MCFG_CPU_IO_MAP(vt100_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vt100_state,  vt100_vertical_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(vt100_state,vt100_irq_callback)
MACHINE_CONFIG_END

/* VT1xx models:
 * VT100 - 1978 base model. the 'later' rom is from 1979 or 1980.
 *    The vt100 had a whole series of -XX models branching off of it; the
       ones I know of are described here, as well as anything special about
       them:
 *    VT100-AA - standard model with 120vac cable \__voltage can be switched
 *    VT100-AB - standard model with 240vac cable /  inside any VT100 unit
 *    VT100-W* - word processing series:
 *     VT100-WA/WB - has special LA120 AVO board preinstalled, WP romset?,
        English WP keyboard, no alt charset rom, LA120? 23-069E2 AVO rom.
       (The WA and WB variants are called the '-02' variant on the schematics)
 *    VT100-WC through WZ: foreign language word processing series:
      (The WC through WK variants are called the '-03' variant on the schematics)
 *     VT100-WC/WD - has AVO board preinstalled, WP romset?, French Canadian
        WP keyboard, has 23-094E2 alt charset rom, 23-093E2 AVO rom.
 *     VT100-WE/WF - has AVO board preinstalled, WP romset?, French
        WP keyboard, has 23-094E2 alt charset rom, 23-093E2 AVO rom.
 *     VT100-WG/WH - has AVO board preinstalled, WP romset?, Dutch
        WP keyboard, has 23-094E2 alt charset rom, 23-093E2 AVO rom.
 *     VT100-WJ/WK - has AVO board preinstalled, WP romset?, German
        WP keyboard, has 23-094E2 alt charset rom, 23-093E2 AVO rom.
 *     VT100-WY/WZ - has AVO board preinstalled, WP romset?, English
        WP keyboard, has 23-094E2 alt charset rom, 23-093E2 AVO rom.
       The WP romset supports English, French, Dutch and German languages but
        will only display text properly in the non-English languages if the
        23-094E2 alt charset rom AND the foreign language 23-093E2
        AVO rom are populated.
 *    VT100-NA/NB - ? romset with DECFORM keycaps
 *    VT100 with vt1xx-ac kit - adds serial printer interface (STP)
       pcb, replaces roms with the 095e2/096e2/139e2/140e2 STP set
 * VT101 - 1981 cost reduced unexpandable vt100; Is the same as a stock
   unexpanded vt100. It has no AVO nor the upgrade connector for it, and no
   video input port.) Has its own firmware.
   Shares same pcb with vt102 and vt131, but STP/AVO are unpopulated;
 * VT102 - 1981 cost reduced unexpandable vt100 with built in AVO and STP
   Is the same as a stock vt100 with the AVO and STP expansions installed,
   but all on one pcb. Does NOT support the AVO extended character roms, nor
   the word processing rom set. Has its own firmware.
   Shares same pcb with vt101 and vt131, has STP and AVO populated.
 * VT103 - 1980 base model vt100 with an integrated TU58 tape drive, and an
   LSI-11 backplane, which an LSI-11 cpu card is used in, hence the computer
   is effectively a tiny lsi-11 (pdp-11) built in a vt100 case. uses same roms
   as vt100 for the vt100 portion, and tu58 has its own cpu and rom. It can
   have the normal vt100 romset variant, and also can have the multiple word
   processing variations (which use the same roms as the vt100 ones do).
 * VT104 doesn't exist.
 * VT105 - 1978 vt100 with the WG waveform generator board installed
   (for simple chart-type line-compare-made raster graphics using some built
   in functions), AVO optional; was intended for use on the MINC analog data
   acquisition computer.
 * VT110 - 1978 vt100 with a DPM01 DECDataway serial multiplexer installed
    The DPM01 supposedly has its own processor and roms.
 * vt125 - 1982? base model (stock vt100 firmware plus extra gfx board
   firmware and processor) vt100 with the ReGIS graphical language board
   (aka GPO) installed (almost literally a vk100-on-a-board, but with added
   backwards compatibility mode for vt105/WG, and 2 bits per pixel color),
   AVO optional; Includes a custom 'dumb' STP board.
 * vt131 - 1982 cost reduced version of vt132, no longer has the vt100
   expansion backplane; has the AVO advanced video board built in, as well
   as the parallel port interface board, and supports serial block mode.
   Shares same pcb with vt101 and vt102, has STP and AVO populated.
 * vt132 - 1980? base vt100 with AVO, STP, and its own 23-099e2/23-100e2
   AVO character rom set. Has its own base firmware roms which support block
   serial mode.
 * vt180 - 1980 vt10x (w/vt100 expansion backplane) with a z80 daughterboard
   installed;
   The daughterboard has two roms on it: 23-017e3-00 and 23-021e3-00
   (both are 0x1000 long, 2332 mask roms)
 * vk100 'gigi'- graphical terminal; the vt125 GPO board is a very close derivative;
   relatively little info so far but progress has been made.
   see vk100.c for current driver for this

 * Upgrade kits for vt1xx:
 * VT1xx-AA : p/n 5413206 20ma current loop interface pcb for vt100
 * VT1xx-AB : p/n 5413097 AVO board (AVO roms could be optionally ordered along with
              this board if needed)
 * VT1xx-AC : STP serial printer board (includes a special romset)
 * VT1xx-CA : p/n 5413206? 20ma current loop interface pcb for vt101/vt102/vt131
 * VT1xx-CB or CL: GPO "ReGIS" board vt100->vt125 upgrade kit (p/n 5414275 paddle board and 5414277 gpo board)
 * VT1xx-CE : DECWord Conversion kit
 * VT1xx-FB : Anti-glare kit

 * Info about mask roms and other nasties:
 * A normal 2716 rom has pin 18: /CE; pin 20: /OE; pin 21: VPP (acts as CE2)
 * The vt100 23-031e2/23-061e2, 23-032e2, 23-033e2, and 23-034e2 mask roms
   have the follwing enables:
       23-031e2/23-061e2: pin 18:  CS2; pin 20:  CS1; pin 21:  CS3
       23-032e2:          pin 18: /CS2; pin 20:  CS1; pin 21:  CS3
       23-033e2:          pin 18:  CS2; pin 20:  CS1; pin 21: /CS3
       23-034e2:          pin 18: /CS2; pin 20:  CS1; pin 21: /CS3
       (This is cute because it technically means the roms can be put in the
       4 sockets in ANY ORDER and will still work properly since the cs2 and
       cs3 pins make them self-decode and activate at their proper address)
       (This same cute trick is almost certainly also done with the
       23-180e2, 181e2, 182e2 183e2 romset, as well as the
       23-095e2,096e2,139e2,140e2 set and probably others as well)
 * The vt100/101/102/103/etc 23-018e2-00 character set rom at location e4 is a 24 pin 2316 mask rom with enables as such: pin 18: CS2; pin 20: /CS1; pin 21: /CS3
 * The optional 23-094e2-00 alternate character set rom at location e9 is a 24 pin 2316 mask rom with enables as such: pin 18: /CS2; pin 20: /CS1; pin 21: /CS3
       Supposedly the 23-094e2 rom is meant for vt100-WC or -WF systems, (which are French Canadian and French respectively), implying that it has European language specific accented characters on it. It is probably used in all the -W* systems.
       Pin 21 can be jumpered to +5v for this socket at location e9 by removing jumper w4 and inserting jumper w5, allowing a normal 2716 eprom to be used.
 * The optional AVO character set roms (see below) have: pin 18: /CS2*; pin 20: /CS1; pin 21: CS3 hence they match a normal 2716
   *(this is marked on the image as if it was CS2 but the input is tied to gnd meaning it must be /CS2)

 * The AVO itself can hold up to four roms on it (see http://www.bitsavers.org/pdf/dec/terminal/vt100/MP00633_VT100_Mar80.pdf
   and http://vt100.net/dec/ek-vt1ac-ug-002.pdf )
   and these roms can depending on jumpers be mapped at 0x8000, OR overlay the main code roms at 0x0000-0x1fff!
   They may even allow banking between the main code roms and the overlay roms, I haven't traced the schematic.
   At least sixteen of these AVO roms were made, and are used as such:
   (based on EK-VT100-TM-003_VT100_Technical_Manual_Jul82.pdf)
 * No roms - normal vt100 system with AVO installed
 * 23-069E2 (location e21) - meant for vt100-wa and -wb 'LA120' 'word processing' systems (the mapping of the rom for this system is different than for the ones below)
 * 23-099E2 (location e21) and 23-100E2 (location e17) - meant for vt132 but only with the OLD vt132 main romset of 095,096,097,098E2
 * 23-093E2 (location e21) - meant for vt100 wc through wz 'foreign language' word processing systems
 * 23-184E2 and 23-185E2 - meant for vt100 with STP printer option board installed, version 1, comes with vt1xx-ac kit
 * 23-186E2 and 23-187E2 - meant for vt100 with STP printer option board installed, version 2, comes with vt1xx-ac kit
 * 23-224E2, 23-225E2, 23-226E2, 23-227E2 - meant for vt132 but only with the NEW vt132 main romset of 180,181,182,183E2
 * 23-236E2, 23-237E2, 23-238E2, 23-239E2 - meant for vt132 but only with the NEW vt132 main romset of 180,181,182,183E2, unknown difference to above (PROM VS MASK ROM? same contents?)
 */

/* ROM definition */
ROM_START( vt100 ) // This is from the schematics at http://www.bitsavers.org/pdf/dec/terminal/vt100/MP00633_VT100_Mar80.pdf
// This is the standard VT100 cpu board with the 'normal' roms (but later rev of eprom 0) populated
// This romset is also used for the vt103, vt105, vt110, vt125, and vt180
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt100" )
	ROM_SYSTEM_BIOS( 0, "vt100o", "VT100 older roms" )
	ROMX_LOAD( "23-031e2-00.e56", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1)) // version 1 1978 'earlier rom', dump needed, correct for earlier vt100s
	ROM_SYSTEM_BIOS( 1, "vt100", "VT100 newer roms" )
	ROMX_LOAD( "23-061e2-00.e56", 0x0000, 0x0800, CRC(3dae97ff) SHA1(e3437850c33565751b86af6c2fe270a491246d15), ROM_BIOS(2)) // version 2 1979 or 1980 'later rom', correct for later vt100s
	ROM_LOAD( "23-032e2-00.e52", 0x0800, 0x0800, CRC(3d86db99) SHA1(cdd8bdecdc643442f6e7d2c83cf002baf8101867))
	ROM_LOAD( "23-033e2-00.e45", 0x1000, 0x0800, CRC(384dac0a) SHA1(22aaf5ab5f9555a61ec43f91d4dea3029f613e64))
	ROM_LOAD( "23-034e2-00.e40", 0x1800, 0x0800, CRC(4643184d) SHA1(27e6c19d9932bf13fdb70305ef4d806e90d60833))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL("23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional ?word processing? alternate character set rom
ROM_END

#if 0
ROM_START( vt100wp ) // This is from the schematics at http://www.bitsavers.org/pdf/dec/terminal/vt100/MP00633_VT100_Mar80.pdf
// This is the standard vt100 cpu board, with the ?word processing? romset, included in the VT1xx-CE kit?
// the vt103 can also use this rom set (-04 and -05 revs have it by default, -05 rev also has the optional alt charset rom by default)
// NOTE: this is actually the same as the newer VT132 romset; vt132 has different AVO roms as well.
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-180e2-00.e56", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-181e2-00.e52", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-182e2-00.e45", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-183e2-00.e40", 0x1800, 0x0800, NO_DUMP)

	ROM_REGION(0x1000, "avo", 0)
	ROM_LOAD( "23-184e2-00.bin", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-185e2-00.bin", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-186e2-00.bin", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-187e2-00.bin", 0x1800, 0x0800, NO_DUMP)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // REQUIRED foreign language alternate character set rom
ROM_END

ROM_START( vt132 ) // This is from anecdotal evidence and vt100.net, as the vt132 schematics are not scanned
// but is pretty much confirmed by page 433 in http://bitsavers.trailing-edge.com/www.computer.museum.uq.edu.au/pdf/EK-VT100-TM-003%20VT100%20Series%20Video%20Terminal%20Technical%20Manual.pdf
// VT100 board with block serial roms, AVO with special roms, STP, custom firmware with block serial mode
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// OLDER vt132 romset
	ROM_LOAD( "23-095e2-00.e56", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-096e2-00.e52", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-097e2-00.e45", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-098e2-00.e40", 0x1800, 0x0800, NO_DUMP)

	// NEWER vt132 (and STP?) romset
	ROM_LOAD( "23-180e2-00.e56", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-181e2-00.e52", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-182e2-00.e45", 0x1000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-183e2-00.e40", 0x1800, 0x0800, NO_DUMP)

	// AVO roms for OLDER romset only
	ROM_REGION(0x1000, "avo", 0)
	ROM_LOAD( "23-099e2-00.e21", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-100e2-00.e17", 0x0800, 0x0800, NO_DUMP)
	// other 2 sockets are empty

	// AVO roms for NEWER romset only
	ROM_LOAD( "23-224e2-00.e21", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-225e2-00.e17", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-226e2-00.e15", 0x1000, 0x0800, NO_DUMP) // loc is a guess
	ROM_LOAD( "23-227e2-00.e13", 0x1800, 0x0800, NO_DUMP) // loc is a guess
	// alt rev of newer avo roms, tech manual implies above are PROMS below are MASK ROMS? same data?
	ROM_LOAD( "23-236e2-00.e21", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-237e2-00.e17", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-238e2-00.e15", 0x1000, 0x0800, NO_DUMP) // loc is a guess
	ROM_LOAD( "23-239e2-00.e13", 0x1800, 0x0800, NO_DUMP) // loc is a guess

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom
ROM_END

ROM_START( vt100stp ) // This is from the VT180 technical manual at http://www.bitsavers.org/pdf/dec/terminal/vt180/EK-VT18X-TM-001_VT180_Technical_Man_Feb83.pdf
// This is the standard vt100 cpu board, but with the rom set included with the VT1xx-AC kit
// which is only used when the part 54-14260-00 STP 'printer port expansion' card is installed into the terminal board.
// Or as http://bitsavers.trailing-edge.com/www.computer.museum.uq.edu.au/pdf/EK-VT100-TM-003%20VT100%20Series%20Video%20Terminal%20Technical%20Manual.pdf
// on page 433: VT100 WC or WK uses these as well.
// This romset adds the Set-up C page to the setup menu (press keypad 5 twice once you hit set-up)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-095e2-00.e56", 0x0000, 0x0800, NO_DUMP)
	ROM_LOAD( "23-096e2-00.e52", 0x0800, 0x0800, NO_DUMP)
	ROM_LOAD( "23-139e2-00.e45", 0x1000, 0x0800, NO_DUMP) // revision 2?; revision 1 is 23-097e2
	ROM_LOAD( "23-140e2-00.e40", 0x1800, 0x0800, NO_DUMP) // revision 2?; revision 1 is 23-098e2
	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional ?word processing? alternate character set rom
	ROM_REGION(0x10000, "stpcpu",ROMREGION_ERASEFF)
// expansion board for a vt100 with a processor on it and dma, intended to act as a ram/send buffer for the STP printer board.
// It can be populated with two banks of two eproms each, each bank either contains 2k or 4k eproms depending on the w2/w3 and w4/w5 jumpers.
// It also has two proms on the cpu board. I don't know if it is technically necessary to have this board installed if an STP module is installed, but due to the alt stp romset, it probably is.
	ROM_LOAD( "23-003e3-00.e10", 0x0000, 0x1000, NO_DUMP) // "EPROM 0" bank 0
	ROM_LOAD( "23-004e3-00.e4", 0x1000, 0x1000, NO_DUMP) // "EPROM 1" bank 0
	ROM_LOAD( "23-005e3-00.e9", 0x2000, 0x1000, NO_DUMP) // "EPROM 2" bank 1
	ROM_LOAD( "23-006e3-00.e3", 0x3000, 0x1000, NO_DUMP) // "EPROM 3" bank 1
	//ROM_REGION(0x0800, "avo",0)
	//ROM_LOAD( "23-???e2-00.e34", 0x0000, 0x0800, NO_DUMP) // ? second gfx rom?
	ROM_REGION(0x0400, "proms",0)
	ROM_LOAD( "23-312a1-07.e26", 0x0000, 0x0200, NO_DUMP) // "PROM A"; handles 8085 i/o? mapping (usart, timer, dma, comm, etc)
	ROM_LOAD( "23-313a1-07.e15", 0x0200, 0x0200, NO_DUMP) // "PROM B"; handles firmware rom mapping and memory size/page select; bit 0 = ram page, bits 1-3 unused, bits 4-7 select one eprom each
ROM_END

ROM_START( vt103 ) // This is from the schematics at http://www.bitsavers.org/pdf/dec/terminal/vt103/MP00731_VT103_Aug80.pdf
// This is the standard VT100 cpu board with the 'normal' roms (but later rev of eprom 0) populated but with an
// LSI-11 backplane (instead of a normal VT100 one, hence it cannot use the AVO, WG, GPO, or VT180 Z80 boards) and
// DEC TU58 dual 256k tape drive integrated; It was intended that you would put an LSI-11 cpu card in there, which
// Would talk to the terminal as its input/output device. Several LSI-11 cpu cards were available?
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-061e2-00.e56", 0x0000, 0x0800, CRC(3dae97ff) SHA1(e3437850c33565751b86af6c2fe270a491246d15)) // version 2 1980 'later rom'
	ROM_LOAD( "23-032e2-00.e52", 0x0800, 0x0800, CRC(3d86db99) SHA1(cdd8bdecdc643442f6e7d2c83cf002baf8101867))
	ROM_LOAD( "23-033e2-00.e45", 0x1000, 0x0800, CRC(384dac0a) SHA1(22aaf5ab5f9555a61ec43f91d4dea3029f613e64))
	ROM_LOAD( "23-034e2-00.e40", 0x1800, 0x0800, CRC(4643184d) SHA1(27e6c19d9932bf13fdb70305ef4d806e90d60833))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom

	ROM_REGION(0x0800, "tapecpu", 0) // rom for the 8085 cpu in the integrated serial tu58-xa drive
	ROM_LOAD( "23-089e2.e1", 0x0000, 0x0800, CRC(8614dd4c) SHA1(1b554e6c98bddfc6bc48d81c990deea43cf9df7f)) // Label: "23-089E2 // P8316E - AMD // 35227 8008NPP"

	ROM_REGION(0x80000, "lsi11cpu", 0) // rom for the LSI-11 cpu board
	ROM_LOAD_OPTIONAL( "unknown.bin", 0x00000, 0x80000, NO_DUMP)
ROM_END
#endif

ROM_START( vt105 ) // This is from anecdotal evidence and vt100.net, as the vt105 schematics are not scanned
// This is the standard VT100 cpu board with the 'normal' roms (but later rev of eprom 0) populated but with a
// WG waveform generator board factory installed; this makes the terminal act like a vt55 with vt100 terminal capability
// The VT105 was intended for use on the MINC analog data acquisition computer
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-061e2-00.e56", 0x0000, 0x0800, CRC(3dae97ff) SHA1(e3437850c33565751b86af6c2fe270a491246d15)) // version 2 1980 'later rom'
	ROM_LOAD( "23-032e2-00.e52", 0x0800, 0x0800, CRC(3d86db99) SHA1(cdd8bdecdc643442f6e7d2c83cf002baf8101867))
	ROM_LOAD( "23-033e2-00.e45", 0x1000, 0x0800, CRC(384dac0a) SHA1(22aaf5ab5f9555a61ec43f91d4dea3029f613e64))
	ROM_LOAD( "23-034e2-00.e40", 0x1800, 0x0800, CRC(4643184d) SHA1(27e6c19d9932bf13fdb70305ef4d806e90d60833))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom
ROM_END

#if 0
ROM_START( vt110 )
// This is the standard VT100 cpu board with the 'normal' roms (but later rev of eprom 0) populated but with a
// DECDataway DPM01 board, which adds 4 or 5 special network-addressable 50ohm? current loop serial lines
// and may add its own processor and ram to control them. see http://bitsavers.org/pdf/dec/terminal/EK-VT110_UG-001_Dec78.pdf
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-061e2-00.e56", 0x0000, 0x0800, CRC(3dae97ff) SHA1(e3437850c33565751b86af6c2fe270a491246d15)) // version 2 1980 'later rom'
	ROM_LOAD( "23-032e2-00.e52", 0x0800, 0x0800, CRC(3d86db99) SHA1(cdd8bdecdc643442f6e7d2c83cf002baf8101867))
	ROM_LOAD( "23-033e2-00.e45", 0x1000, 0x0800, CRC(384dac0a) SHA1(22aaf5ab5f9555a61ec43f91d4dea3029f613e64))
	ROM_LOAD( "23-034e2-00.e40", 0x1800, 0x0800, CRC(4643184d) SHA1(27e6c19d9932bf13fdb70305ef4d806e90d60833))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL ( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom
//DECDataway board roms go here!
ROM_END

ROM_START( vt125 ) // This is from bitsavers and vt100.net, as the vt125 schematics are not scanned
// This is the standard VT100 cpu board with the 'normal' roms (but later rev of eprom 0) populated but with a
// special "GPO" ReGIS cpu+ram card 54-14277 installed which provides a framebuffer, text rotation, custom ram fonts, and many other features.
// Comes with a custom 'dumb' STP card 54-14275 as well.
// VT125 upgrade kit (upgrade from vt100 or vt105) was called VT1xx-CB or CL
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-061e2-00.e56", 0x0000, 0x0800, CRC(3dae97ff) SHA1(e3437850c33565751b86af6c2fe270a491246d15)) // version 2 1980 'later rom'
	ROM_LOAD( "23-032e2-00.e52", 0x0800, 0x0800, CRC(3d86db99) SHA1(cdd8bdecdc643442f6e7d2c83cf002baf8101867))
	ROM_LOAD( "23-033e2-00.e45", 0x1000, 0x0800, CRC(384dac0a) SHA1(22aaf5ab5f9555a61ec43f91d4dea3029f613e64))
	ROM_LOAD( "23-034e2-00.e40", 0x1800, 0x0800, CRC(4643184d) SHA1(27e6c19d9932bf13fdb70305ef4d806e90d60833))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL ( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom

	// "GPO" aka vt125 "mono board" roms and proms
	ROM_REGION(0x10000, "monocpu", ROMREGION_ERASEFF) // roms for the 8085 subcpu
	ROM_LOAD( "23-043e4-00.e22", 0x0000, 0x2000, NO_DUMP) // 2364/MK36xxx mask rom
	ROM_LOAD( "23-044e4-00.e23", 0x2000, 0x2000, NO_DUMP) // 2364/MK36xxx mask rom
	ROM_LOAD( "23-045e4-00.e24", 0x4000, 0x2000, NO_DUMP) // 2364/MK36xxx mask rom
	// E25 socket is empty

	ROM_REGION(0x100, "dir", ROMREGION_ERASEFF ) // vt125 direction prom, same as on vk100, 82s135 equiv
	ROM_LOAD( "23-059b1.e41", 0x0000, 0x0100, CRC(4b63857a) SHA1(3217247d983521f0b0499b5c4ef6b5de9844c465))

	ROM_REGION(0x100, "trans", ROMREGION_ERASEFF ) // vt125 x translate prom, same as on vk100, 82s135 equiv
	ROM_LOAD( "23-060b1.e60", 0x0000, 0x0100, CRC(198317fc) SHA1(00e97104952b3fbe03a4f18d800d608b837d10ae))

	ROM_REGION(0x500, "proms", ROMREGION_ERASEFF) // vt125 mono board proms
	ROM_LOAD( "23-067b1.e135", 0x0000, 0x0100, NO_DUMP) // 82s135, waitstate prom
	ROM_LOAD( "23-068b1.e64", 0x0100, 0x0100, NO_DUMP) // 82s135, sync_a prom
	ROM_LOAD( "23-069b1.e66", 0x0200, 0x0100, NO_DUMP) // 82s135, sync_b prom
	ROM_LOAD( "23-070b1.e71", 0x0300, 0x0100, NO_DUMP) // 82s135, vector prom
	ROM_LOAD( "23-582a2.e93", 0x0400, 0x0100, NO_DUMP) // 82s129, ras/erase prom
ROM_END
#endif

ROM_START( vt101 ) // p/n 5414185-01 'unupgradable/low cost' vt101/vt102/vt131 mainboard
// does not have integrated STP or AVO populated
// 8085 based instead of I8080
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-028e4-00.e71", 0x0000, 0x2000, CRC(fccce02c) SHA1(f3e3e93a857443685b816cab4fb52e34c0bc72b1)) // rom is unique to vt101; "CN55004N 8232 // DEC TP03 // 23-028E4-00" 24-pin mask rom (mc68764 pinout)
	//e69 socket is empty/unpopulated on vt101?
	//e67 socket is empty/unpopulated on vt101?

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e3", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e4", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom
ROM_END


ROM_START( vt102 ) // p/n 5414185-01 'unupgradable/low cost' vt101/vt102/vt131 mainboard
// has integrated STP and AVO both populated
// ROMS have the set up page C in them
// 8085 based instead of I8080
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt102" )
	ROM_SYSTEM_BIOS( 0, "vt102o", "VT102 older roms" )
	ROMX_LOAD( "23-042e4-00.e71", 0x0000, 0x2000, CRC(e8aa006c) SHA1(8ac2a84a8d2a9fa0c6cd583ae35e4c21f863b45b), ROM_BIOS(1)) // shared with vt131
	ROMX_LOAD( "23-041e4-00.e69", 0x8000, 0x2000, CRC(b11d331e) SHA1(8b0f885c7e032d1d709e3913d279d6950bbd4b6a), ROM_BIOS(1)) // shared with vt131
	ROM_SYSTEM_BIOS( 1, "vt102", "VT102 newer roms" )
	ROMX_LOAD( "23-226e4-00.e71", 0x0000, 0x2000, CRC(85c9279a) SHA1(3283d27e9c45d9e384227a7e6e98ee8d54b92bcb), ROM_BIOS(2)) // shared with vt131
	ROMX_LOAD( "23-225e4-00.e69", 0x8000, 0x2000, CRC(3567c760) SHA1(672473162e9c92cd237e4dbf92c2700a31c5374b), ROM_BIOS(2)) // shared with vt131
	//e67 socket is empty but populated on vt102

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e3", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e4", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom
ROM_END

ROM_START( vt131 ) // p/n 5414185-01 'unupgradable/low cost' vt101/vt131 mainboard with vt132-style block serial mode
// has integrated STP and AVO both populated
// ROMS have the set up page C in them
// 8085 based instead of I8080
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-226e4-00.e71", 0x0000, 0x2000, CRC(85c9279a) SHA1(3283d27e9c45d9e384227a7e6e98ee8d54b92bcb)) // shared with vt102
	ROM_LOAD( "23-225e4-00.e69", 0x8000, 0x2000, CRC(3567c760) SHA1(672473162e9c92cd237e4dbf92c2700a31c5374b)) // shared with vt102
	ROM_LOAD( "23-280e2-00.e67", 0xA000, 0x0800, CRC(71b4172e) SHA1(5a82c7dc313bb92b9829eb8350840e072825a797)) // called "VT131 ROM" in the vt101 quick reference guide; pins 20, 18 and 21 are /CE /CE2 and /CE3 on this mask rom

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e3", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL( "23-094e2-00.e4", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom
ROM_END

ROM_START( vt180 )
// This is the standard VT100 cpu board with the 'normal' roms (but later rev of eprom 0) populated but with a
// Z80 daughterboard added to the expansion slot, and replacing the STP adapter (STP roms are replaced with the normal set)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-061e2-00.e56", 0x0000, 0x0800, CRC(3dae97ff) SHA1(e3437850c33565751b86af6c2fe270a491246d15)) // version 2 1980 'later rom'
	ROM_LOAD( "23-032e2-00.e52", 0x0800, 0x0800, CRC(3d86db99) SHA1(cdd8bdecdc643442f6e7d2c83cf002baf8101867))
	ROM_LOAD( "23-033e2-00.e45", 0x1000, 0x0800, CRC(384dac0a) SHA1(22aaf5ab5f9555a61ec43f91d4dea3029f613e64))
	ROM_LOAD( "23-034e2-00.e40", 0x1800, 0x0800, CRC(4643184d) SHA1(27e6c19d9932bf13fdb70305ef4d806e90d60833))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "23-018e2-00.e4", 0x0000, 0x0800, CRC(6958458b) SHA1(103429674fc01c215bbc2c91962ae99231f8ae53))
	ROM_LOAD_OPTIONAL ( "23-094e2-00.e9", 0x0800, 0x0800, NO_DUMP) // optional (comes default with some models) alternate character set rom

	ROM_REGION(0x10000, "z80cpu", 0) // z80 daughterboard
	ROM_LOAD( "23-021e3-00.bin", 0x0000, 0x1000, CRC(a2a575d2) SHA1(47a2c40aaec89e8476240f25515d75ab157f2911))
	ROM_LOAD( "23-017e3-00.bin", 0x1000, 0x1000, CRC(4bdd2398) SHA1(84f288def6c143a2d2ed9dedf947c862c66bb18e))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY                     FULLNAME       FLAGS */
COMP( 1978, vt100,    0,      0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT100",MACHINE_NOT_WORKING)
//COMP( 1978, vt100wp,  vt100,  0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT100-Wx", MACHINE_NOT_WORKING)
//COMP( 1978, vt100stp, vt100,  0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT100 w/VT1xx-AC STP", MACHINE_NOT_WORKING)
COMP( 1981, vt101,    vt102,      0,       vt102,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT101", MACHINE_NOT_WORKING)
COMP( 1981, vt102,    0,      0,       vt102,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT102", MACHINE_NOT_WORKING)
//COMP( 1979, vt103,    vt100,  0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT103", MACHINE_NOT_WORKING)
COMP( 1978, vt105,    vt100,  0,       vt100,     vt100, driver_device,   0,   "Digital Equipment Corporation", "VT105", MACHINE_NOT_WORKING)
//COMP( 1978, vt110,    vt100,  0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT110", MACHINE_NOT_WORKING)
//COMP( 1981, vt125,    vt100,  0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT125", MACHINE_NOT_WORKING)
COMP( 1981, vt131,    vt102,  0,       vt102,     vt100, driver_device,   0,   "Digital Equipment Corporation", "VT131", MACHINE_NOT_WORKING)
//COMP( 1979, vt132,    vt100,  0,       vt100,     vt100, driver_device,   0,  "Digital Equipment Corporation", "VT132", MACHINE_NOT_WORKING)
COMP( 1983, vt180,    vt100,  0,       vt180,     vt100, driver_device,   0,   "Digital Equipment Corporation", "VT180", MACHINE_NOT_WORKING)
