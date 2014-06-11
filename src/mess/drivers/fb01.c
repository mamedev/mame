/***************************************************************************

  Yamaha FB-01

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "bus/midi/midi.h"
#include "fb01.lh"


class fb01_state : public driver_device
{
public:
	fb01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_upd71051(*this, "upd71051")
		, m_midi_thru(*this, "mdthru")
	{
	}

	DECLARE_WRITE_LINE_MEMBER(write_usart_clock);
	DECLARE_WRITE_LINE_MEMBER(midi_in);

private:
	required_device<i8251_device> m_upd71051;
	required_device<midi_port_device> m_midi_thru;
};


static ADDRESS_MAP_START(fb01_mem, AS_PROGRAM, 8, fb01_state)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("nvram")  // 2 * 8KB S-RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(fb01_io, AS_IO, 8, fb01_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// 00-01  YM2164
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("ym2164", ym2151_device, register_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("ym2164", ym2151_device, status_r, data_w)

 	// 10-11  USART uPD71051C  4MHz & 4MHz / 8
	AM_RANGE(0x10, 0x10) AM_DEVREADWRITE("upd71051", i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_DEVREADWRITE("upd71051", i8251_device, status_r, control_w)

	// 20     PANEL SWITCH
	AM_RANGE(0x20, 0x20) AM_READ_PORT("PANEL")

	// 30-31  HD44780A
	AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("hd44780", hd44780_device, control_read, control_write)
	AM_RANGE(0x31, 0x31) AM_DEVREADWRITE("hd44780", hd44780_device, data_read, data_write)
ADDRESS_MAP_END


static INPUT_PORTS_START( fb01 )
	PORT_START("PANEL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("System Set Up")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Select")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Assign")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Function")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Function")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Select")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("-1/No")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+1/Yes")
INPUT_PORTS_END


WRITE_LINE_MEMBER(fb01_state::write_usart_clock)
{
	m_upd71051->write_txc(state);
	m_upd71051->write_rxc(state);
}


WRITE_LINE_MEMBER(fb01_state::midi_in)
{
	m_midi_thru->write_txd(state);
	m_upd71051->write_rxd(state);
}


static HD44780_PIXEL_UPDATE(fb01_pixel_update)
{
	if ( pos < 8 && line < 2 )
	{
		bitmap.pix16(y, line*6*8 + pos*6 + x) = state;
	}
}


static MACHINE_CONFIG_START( fb01, fb01_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)
	MCFG_CPU_PROGRAM_MAP(fb01_mem)
	MCFG_CPU_IO_MAP(fb01_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(6*16, 9)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9-1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT( layout_fb01 )

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 8)   // 2x8 displayed as 1x16
	MCFG_HD44780_PIXEL_UPDATE_CB(fb01_pixel_update)

	MCFG_DEVICE_ADD("upd71051", I8251, XTAL_4MHz)
	MCFG_I8251_RXRDY_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0)) MCFG_DEVCB_XOR(1)  // inverted -> Z80 INT
	MCFG_I8251_TXRDY_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0)) MCFG_DEVCB_XOR(1)  // inverted -> Z80 INT
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("mdout", midi_port_device, write_txd))

	MCFG_DEVICE_ADD("usart_clock", CLOCK, XTAL_4MHz / 8) // 500KHz
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(fb01_state, write_usart_clock))

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(WRITELINE(fb01_state, midi_in))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_MIDI_PORT_ADD("mdthru", midiout_slot, "midiout")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_YM2151_ADD("ym2164", XTAL_4MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( fb01 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("fb01.ic11", 0, 0x8000, CRC(7357e9a4) SHA1(049c482d6c91b7e2846757dd0f5138e0d8b687f0))
ROM_END


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT   MACHINE  INPUT  INIT                  COMPANY   FULLNAME  FLAGS */
CONS( 1986, fb01, 0,      0,       fb01,    fb01,  driver_device,   0,   "Yamaha", "FB-01",  GAME_NOT_WORKING )

