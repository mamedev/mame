// license:BSD-3-Clause
// copyright-holders:Robbbert,Barry Rodewald
/***************************************************************************

    Skeleton driver for Husky Hunter 2

    2014-03-06 Skeleton [Robbbert]
    2014-03-07 Fixed screen [Barry Rodewald]
    2014-03-07 Added more stuff [Robbbert]

    http://www.seasip.info/VintagePC/husky2.html

    No schematics or manuals available.

    TODO:
    - Add ports 83 and 85
    - Need software
    - Add whatever image devices existed
    - Probably lots of other stuff

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/hd61830.h"
#include "machine/mm58274c.h"
#include "rendlay.h"
#include "sound/speaker.h"
#include "machine/nsc810.h"
#include "bus/rs232/rs232.h"
#include "machine/nvram.h"
#include "machine/bankdev.h"

class hunter2_state : public driver_device
{
public:
	hunter2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_rs232(*this, "serial")
		, m_rom(*this, "roms")
		, m_ram(*this, "rams")
		, m_nvram(*this, "nvram")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
	{ }

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(serial_dsr_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_READ8_MEMBER(serial_rx_r);
	DECLARE_WRITE8_MEMBER(display_ctrl_w);
	DECLARE_WRITE8_MEMBER(port80_w);
	DECLARE_WRITE8_MEMBER(serial_tx_w);
	DECLARE_WRITE8_MEMBER(serial_dtr_w);
	DECLARE_WRITE8_MEMBER(serial_rts_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_WRITE8_MEMBER(irqctrl_w);
	DECLARE_WRITE8_MEMBER(memmap_w);
	DECLARE_PALETTE_INIT(hunter2);
	DECLARE_DRIVER_INIT(hunter2);
	DECLARE_WRITE_LINE_MEMBER(timer0_out);
	DECLARE_WRITE_LINE_MEMBER(timer1_out);
	DECLARE_WRITE_LINE_MEMBER(cts_w);
	DECLARE_WRITE_LINE_MEMBER(rxd_w);

private:
	UINT8 m_keydata;
	UINT8 m_irq_mask;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<rs232_port_device> m_rs232;
	required_memory_region m_rom;
	required_memory_region m_ram;
	required_device<nvram_device> m_nvram;
	required_device<address_map_bank_device> m_bank1;
	required_device<address_map_bank_device> m_bank2;
	required_device<address_map_bank_device> m_bank3;
};

static ADDRESS_MAP_START(hunter2_banked_mem, AS_PROGRAM, 8, hunter2_state)
	AM_RANGE(0x00000, 0x2ffff) AM_ROM AM_REGION("roms", 0x0000)
	AM_RANGE(0x30000, 0x3ffff) AM_NOP
	AM_RANGE(0x40000, 0xfffff) AM_RAM AM_REGION("rams", 0x0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(hunter2_mem, AS_PROGRAM, 8, hunter2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_DEVREADWRITE("bank1", address_map_bank_device, read8, write8)
	AM_RANGE(0x4000, 0x7fff) AM_DEVREADWRITE("bank2", address_map_bank_device, read8, write8)
	AM_RANGE(0x8000, 0xbfff) AM_DEVREADWRITE("bank3", address_map_bank_device, read8, write8)
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hunter2_io, AS_IO, 8, hunter2_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x1f) AM_DEVREADWRITE("iotimer", nsc810_device, read, write)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("lcdc", hd61830_device, data_w)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("lcdc", hd61830_device, status_r, control_w)
	AM_RANGE(0x3e, 0x3e) AM_DEVREAD("lcdc", hd61830_device, data_r)
	AM_RANGE(0x40, 0x4f) AM_DEVREADWRITE("rtc", mm58274c_device, read, write)
	AM_RANGE(0x60, 0x60) AM_WRITE(display_ctrl_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(port80_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(serial_tx_w)
	AM_RANGE(0x82, 0x82) AM_WRITE(serial_dtr_w)
	AM_RANGE(0x84, 0x84) AM_WRITE(serial_rts_w)
	AM_RANGE(0x86, 0x86) AM_WRITE(speaker_w)
	AM_RANGE(0xbb, 0xbb) AM_WRITE(irqctrl_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(memmap_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( hunter2 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc/Brk") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lbl/Ins") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
INPUT_PORTS_END

READ8_MEMBER( hunter2_state::keyboard_r )
{
	UINT8 i,data = 0xff;
	for (i = 0; i < 7; i++)
	{
		if (!BIT(m_keydata, i))
		{
			char kbdrow[6];
			sprintf(kbdrow,"X%d", i);
			data &= ioport(kbdrow)->read();
		}
	}
	return data;
}

READ8_MEMBER( hunter2_state::serial_dsr_r )
{
	UINT8 res = 0x00;

	if(m_rs232->dsr_r())
		res |= 0x80;

	return res;
}

WRITE8_MEMBER( hunter2_state::keyboard_w )
{
	m_keydata = data;
	//logerror("Key row select %02x\n",data);
}

READ8_MEMBER( hunter2_state::serial_rx_r )
{
	UINT8 res = 0x28;

	if(m_rs232->rxd_r())
		res |= 0x01;
	if(m_rs232->dcd_r())
		res |= 0x02;

	return res;
}

WRITE8_MEMBER( hunter2_state::display_ctrl_w )
{
/* according to the website,
Bit 2: Backlight toggle
Bits 1,0,7,6: Contrast level.
00 is being written here
*/
}

WRITE8_MEMBER( hunter2_state::port80_w )
{
}

WRITE8_MEMBER( hunter2_state::serial_tx_w )
{
	m_rs232->write_txd(data & 0x01);
//  logerror("TXD write %02x\n",data);
}

WRITE8_MEMBER( hunter2_state::serial_dtr_w )
{
	m_rs232->write_dtr(data & 0x01);
//  logerror("DTR write %02x\n",data);
}

WRITE8_MEMBER( hunter2_state::serial_rts_w )
{
	m_rs232->write_rts(data & 0x01);
//  logerror("RTS write %02x\n",data);
}

WRITE8_MEMBER( hunter2_state::speaker_w )
{
	m_speaker->level_w(BIT(data, 0));
}

/*
Bit 0 = Enable normal interrupts
Bit 1 = Enable RSTC interrupts
Bit 2 = Enable RSTB interrupts
Bit 3 = Enable RSTA interrupts
*/
WRITE8_MEMBER( hunter2_state::irqctrl_w )
{
	m_irq_mask = data;
	if(!(data & 0x08))
		m_maincpu->set_input_line(NSC800_RSTA, CLEAR_LINE);
	if(!(data & 0x04))
		m_maincpu->set_input_line(NSC800_RSTB, CLEAR_LINE);
	if(!(data & 0x02))
		m_maincpu->set_input_line(NSC800_RSTC, CLEAR_LINE);
}

/*
data   bank0    bank1    bank2
00     00       01       02
01     00       01       03
02     00       01       04
....
09     00       01       11
80     16       17       18
....
8F     61       62       63
*/
WRITE8_MEMBER( hunter2_state::memmap_w )
{
	if (data < 0x0a)
	{
		m_bank1->set_bank(0);
		m_bank2->set_bank(1);
		m_bank3->set_bank(2 + data);
	}
	else
	if (data >= 0x80)
	{
		UINT8 bank = data & 0x0f;
		m_bank1->set_bank(16 + (bank*3));
		m_bank2->set_bank(17 + (bank*3));
		m_bank3->set_bank(18 + (bank*3));
	}
}

void hunter2_state::machine_reset()
{
	m_keydata = 0xff;
	m_irq_mask = 0;
	m_bank1->set_bank(0);
	m_bank2->set_bank(1);
	m_bank3->set_bank(2);
}

// it is presumed that writing to rom will go nowhere
DRIVER_INIT_MEMBER( hunter2_state, hunter2 )
{
	UINT8 *ram = m_ram->base();

	m_nvram->set_base(ram,m_ram->bytes());
}

PALETTE_INIT_MEMBER(hunter2_state, hunter2)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

WRITE_LINE_MEMBER(hunter2_state::timer0_out)
{
	if(state == ASSERT_LINE)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE_LINE_MEMBER(hunter2_state::timer1_out)
{
	if(m_irq_mask & 0x08)
		m_maincpu->set_input_line(NSC800_RSTA, state);
}

WRITE_LINE_MEMBER(hunter2_state::cts_w)
{
	if(BIT(m_irq_mask, 1))
	{
		m_maincpu->set_input_line(NSC800_RSTC, state);
		popmessage("CTS: RSTC set %i\n",state);
	}
}

WRITE_LINE_MEMBER(hunter2_state::rxd_w)
{
	if(BIT(m_irq_mask, 2))
		m_maincpu->set_input_line(NSC800_RSTB, ASSERT_LINE);
}

static MACHINE_CONFIG_START( hunter2, hunter2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(hunter2_mem)
	MCFG_CPU_IO_MAP(hunter2_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(80)
	MCFG_SCREEN_UPDATE_DEVICE("lcdc", hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(240, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 63)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(hunter2_state, hunter2)
	MCFG_DEVICE_ADD("lcdc", HD61830, XTAL_4_9152MHz/2/2) // unknown clock

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD("rtc", MM58274C, 0)
	// this is all a guess
	MCFG_MM58274C_MODE24(0) // 12 hour
	MCFG_MM58274C_DAY1(1)   // monday

	MCFG_NSC810_ADD("iotimer",XTAL_4MHz,XTAL_4MHz)
	MCFG_NSC810_PORTA_READ(READ8(hunter2_state,keyboard_r))
	MCFG_NSC810_PORTB_READ(READ8(hunter2_state,serial_dsr_r))
	MCFG_NSC810_PORTB_WRITE(WRITE8(hunter2_state,keyboard_w))
	MCFG_NSC810_PORTC_READ(READ8(hunter2_state,serial_rx_r))
	MCFG_NSC810_TIMER0_OUT(WRITELINE(hunter2_state,timer0_out))
	MCFG_NSC810_TIMER1_OUT(WRITELINE(hunter2_state,timer1_out))

	MCFG_RS232_PORT_ADD("serial",default_rs232_devices,NULL)
	MCFG_RS232_CTS_HANDLER(WRITELINE(hunter2_state,cts_w))
	MCFG_RS232_RXD_HANDLER(WRITELINE(hunter2_state,rxd_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("bank1", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(hunter2_banked_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

	MCFG_DEVICE_ADD("bank2", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(hunter2_banked_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

	MCFG_DEVICE_ADD("bank3", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(hunter2_banked_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hunter2 )
	ROM_REGION(0x30000, "roms", ROMREGION_ERASEFF) // board has space for 6 roms, but only 2 are populated normally
	ROM_LOAD( "tr032kx8mrom0.ic50", 0x0000, 0x8000, CRC(694d252c) SHA1(b11dbf24faf648596d92b1823e25a8e4fb7f542c) )
	ROM_LOAD( "tr032kx8mrom1.ic51", 0x8000, 0x8000, CRC(82901642) SHA1(d84f2bbd2e9e052bd161a313c240a67918f774ad) )

	// 48 x 4k blocks plus 1 sinkhole
	ROM_REGION(0xc4000, "rams", ROMREGION_ERASEVAL(0xa5)) // board can have up to 736k of ram, but 192k is standard
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE           INIT      COMPANY   FULLNAME       FLAGS */
COMP( 1981, hunter2, 0,      0,       hunter2,   hunter2, hunter2_state,  hunter2,  "Husky", "Hunter 2", MACHINE_NOT_WORKING )
