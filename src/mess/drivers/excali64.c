// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

Excalibur 64 kit computer, designed and sold in Australia by BGR Computers.

Skeleton driver created on 2014-12-09.

Chips: Z80A, 8251, 8253, 8255, 6845

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"
//#include "machine/clock.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
//#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "sound/speaker.h"


class excali64_state : public driver_device
{
public:
	excali64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
	{ }

	DECLARE_DRIVER_INIT(excali64);
	DECLARE_READ8_MEMBER(ppic_r);
	DECLARE_WRITE8_MEMBER(ppic_w);
	DECLARE_READ8_MEMBER(port50_r);
	DECLARE_WRITE8_MEMBER(port70_w);
	DECLARE_WRITE8_MEMBER(video_w);
	MC6845_UPDATE_ROW(update_row);
	DECLARE_WRITE_LINE_MEMBER(crtc_de);
	DECLARE_WRITE_LINE_MEMBER(crtc_vs);

	const UINT8 *m_p_chargen;
	UINT8 *m_p_videoram;
	required_device<palette_device> m_palette;
	
private:
	UINT8 m_sys_status;
	bool m_crtc_vs;
	bool m_crtc_de;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
};

static ADDRESS_MAP_START(excali64_mem, AS_PROGRAM, 8, excali64_state)
	AM_RANGE(0x0000, 0x1FFF) AM_ROM
	AM_RANGE(0x2000, 0x2FFF) AM_ROM AM_WRITE(video_w)
	AM_RANGE(0x3000, 0x3FFF) AM_ROM
	AM_RANGE(0x4000, 0xFFFF) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(excali64_io, AS_IO, 8, excali64_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_DEVREADWRITE("uart",i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0x0c) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x31, 0x31) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x50, 0x5f) AM_READ(port50_r)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0x0c) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0x70, 0x7f) AM_WRITE(port70_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( excali64 )
INPUT_PORTS_END

READ8_MEMBER( excali64_state::ppic_r )
{
	UINT8 data = 0xf7;
	data |= (m_cass->input() > 0.1) << 3;
	return data;
}

/*
d0 : /rom ; screen
d1 : ram on
d2 : /low ; high res
d3 : dispen
d4 : vsync
*/
READ8_MEMBER( excali64_state::port50_r )
{
	UINT8 data = m_sys_status & 7;
	data |= (UINT8)m_crtc_vs << 4;
	data |= (UINT8)m_crtc_de << 5;
	return data;
}

WRITE8_MEMBER( excali64_state::ppic_w )
{
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
}

/*
d0,1,2 : same as port50
d7 : 2nd col
*/
WRITE8_MEMBER( excali64_state::port70_w )
{
	m_sys_status = data;
}

WRITE8_MEMBER( excali64_state::video_w )
{
	m_p_videoram[offset] = data;
}

WRITE_LINE_MEMBER( excali64_state::crtc_de )
{
	m_crtc_de = state;
}

WRITE_LINE_MEMBER( excali64_state::crtc_vs )
{
	m_crtc_vs = state;
}

DRIVER_INIT_MEMBER( excali64_state, excali64 )
{
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("videoram")->base();
}

MC6845_UPDATE_ROW( excali64_state::update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx=0;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		UINT8 inv=0;
		if (x == cursor_x) inv=0xff;
		mem = (ma + x) & 0xfff;
		chr = m_p_videoram[mem];
		gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 0)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 7)];
	}
}

static MACHINE_CONFIG_START( excali64, excali64_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(excali64_mem)
	MCFG_CPU_IO_MAP(excali64_io)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	//MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz / 8) /* Timer 0: tone gen for speaker */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("speaker", speaker_sound_device, level_w))
	MCFG_PIT8253_CLK1(XTAL_16MHz / 8) /* Timer 1: baud rate gen for 8251 */
	//MCFG_PIT8253_OUT1_HANDLER(WRITELINE(excali64_state, write_uart_clock))
	//MCFG_PIT8253_CLK2(XTAL_16MHz / 8) /* Timer 2: not used */

	MCFG_DEVICE_ADD("ppi", I8255A, 0 )
	//MCFG_I8255_IN_PORTA_CB(READ8(excali64_state, ppia_r))
	//MCFG_I8255_OUT_PORTA_CB(WRITE8(excali64_state, ppia_w))
	//MCFG_I8255_IN_PORTB_CB(READ8(excali64_state, ppib_r))
	//MCFG_I8255_OUT_PORTB_CB(WRITE8(excali64_state, ppib_w))
	MCFG_I8255_IN_PORTC_CB(READ8(excali64_state, ppic_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(excali64_state, ppic_w))

	//MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	//MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(excali64_state, write_acia_clock))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 25*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*10-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_16MHz / 16) // 1MHz for lowres; 2MHz for highres
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(excali64_state, update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(excali64_state, crtc_de))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(excali64_state, crtc_vs))

	/* Devices */
	MCFG_CASSETTE_ADD( "cassette" )
	MACHINE_CONFIG_END

/* ROM definition */
ROM_START( excali64 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "rom_1.bin", 0x0000, 0x4000, CRC(e129a305) SHA1(e43ec7d040c2b2e548d22fd6bbc7df8b45a26e5a) )
	ROM_LOAD( "rom_2.bin", 0x2000, 0x2000, CRC(916d9f5a) SHA1(91c527cce963481b7bebf077e955ca89578bb553) )

	ROM_REGION(0x1000, "videoram", ROMREGION_ERASE00)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "genex_3.bin", 0x0000, 0x1000, CRC(b91619a9) SHA1(2ced636cb7b94ba9d329868d7ecf79963cefe9d9) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     CLASS             INIT        COMPANY         FULLNAME        FLAGS */
COMP( 1984, excali64, 0,      0,       excali64,  excali64, excali64_state, excali64,  "BGR Computers", "Excalibur 64", GAME_IS_SKELETON )
