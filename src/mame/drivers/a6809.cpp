// license:BSD-3-Clause
// copyright-holders:Robbbert and unknown others
/***************************************************************************

    Acorn 6809

    2009-05-12 Skeleton driver.
    2013-12-16 Fixed video [Robbbert]

    Acorn System 3 update?
    http://acorn.chriswhy.co.uk/8bit_Upgrades/Acorn_6809_CPU.html

    This is a few boards emulated together:
    - 6809 main board
    - 80x25 video board
    - FDC board (to come)

The video board has a mc6845, and the characters do not go to a character
generator, instead they go to a SAA5050, which has its own CG inbuilt, plus
many other features.

ToDo:
    - FDC (address A00)
    - Centronics Printer (VIA port A)

Commands:
C - Echo to printer ( C '+' turns it on)
D - boot from disk
G - Go
L - Load tape
P - Run from  pc reg
R - Display regs
S - Save tape
T - Trace
V - Set breakpoint

M - Modify memory
MV - Modify memory at breakpoint address
MG - Modify memory from Go address
MP - Modify memory from pc-reg address
MR - Modify regs

While modifying memory:
,  show next address
-  show previous address
enter  show next address
;  exit


****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/keyboard.h"
#include "video/saa5050.h"
#include "video/mc6845.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


class a6809_state : public driver_device
{
public:
	a6809_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_via(*this, "via")
		, m_cass(*this, "cassette")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "mc6845")
	{ }

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(videoram_r);
	DECLARE_WRITE8_MEMBER(a6809_address_w);
	DECLARE_WRITE8_MEMBER(a6809_register_w);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_MACHINE_RESET(a6809);
	TIMER_DEVICE_CALLBACK_MEMBER(a6809_c);
	TIMER_DEVICE_CALLBACK_MEMBER(a6809_p);
	required_shared_ptr<UINT8> m_p_videoram;
	UINT16 m_start_address;
	UINT16 m_cursor_address;
private:
	UINT8 m_cass_data[4];
	bool m_cass_state;
	bool m_cassold;
	UINT8 m_video_index;
	required_device<via6522_device> m_via;
	required_device<cassette_image_device> m_cass;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
};


static ADDRESS_MAP_START(a6809_mem, AS_PROGRAM, 8, a6809_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x03ff) AM_RAM
	AM_RANGE(0x0400,0x07ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0800,0x0800) AM_DEVREAD("mc6845", mc6845_device, status_r) AM_WRITE(a6809_address_w)
	AM_RANGE(0x0801,0x0801) AM_DEVREAD("mc6845", mc6845_device, register_r) AM_WRITE(a6809_register_w)
	AM_RANGE(0x0900,0x090f) AM_MIRROR(0xf0) AM_DEVREADWRITE("via", via6522_device, read, write)
	AM_RANGE(0xf000,0xf7ff) // optional ROM
	AM_RANGE(0xf800,0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( a6809_io, AS_IO, 8, a6809_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( a6809 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER( a6809_state, a6809)
{
	m_via->write_pb0(0);
	m_via->write_pb1(0);
	m_via->write_pb2(0);
	m_via->write_pb3(0);
	m_via->write_pb4(0);
	m_via->write_pb5(0);
	m_via->write_pb6(0);
	m_via->write_pb7(0);
}

READ8_MEMBER( a6809_state::videoram_r )
{
	offset += m_start_address;

	if (m_cursor_address == offset)
		return 0x7f; // cheap cursor
	else
		return m_p_videoram[offset&0x3ff];
}

WRITE8_MEMBER( a6809_state::a6809_address_w )
{
	m_crtc->address_w( space, 0, data );

	m_video_index = data & 0x1f;
}

WRITE8_MEMBER( a6809_state::a6809_register_w )
{
	UINT16 temp = m_start_address;
	UINT16 temq = m_cursor_address;

	m_crtc->register_w( space, 0, data );

	// Get start address
	if (m_video_index == 12)
		m_start_address = ((data & 7) << 8 ) | (temp & 0xff);
	else
	if (m_video_index == 13)
		m_start_address = data | (temp & 0x3f00);

	else
	// Get cursor address
	if (m_video_index == 14)
		m_cursor_address = ((data & 7) << 8 ) | (temq & 0xff);
	else
	if (m_video_index == 15)
		m_cursor_address = data | (temq & 0x3f00);
}

WRITE_LINE_MEMBER( a6809_state::cass_w )
{
	m_cass_state = state;
}

TIMER_DEVICE_CALLBACK_MEMBER(a6809_state::a6809_c)
{
	m_cass_data[3]++;

	if (m_cass_state != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cass_state;
	}

	if (m_cass_state)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(a6809_state::a6809_p)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_via->write_pb7(m_cass_data[1] < 12);
		m_cass_data[1] = 0;
	}
}

WRITE8_MEMBER( a6809_state::kbd_put )
{
	UINT8 d = data & 0x7f;
	if (d == 8) d = 0x7f; // allow backspace to work

	m_via->write_pb0((d>>0)&1);
	m_via->write_pb1((d>>1)&1);
	m_via->write_pb2((d>>2)&1);
	m_via->write_pb3((d>>3)&1);
	m_via->write_pb4((d>>4)&1);
	m_via->write_pb5((d>>5)&1);
	m_via->write_pb6((d>>6)&1);
	m_via->write_cb1(1);
	m_via->write_cb1(0);
}

static MACHINE_CONFIG_START( a6809, a6809_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(a6809_mem)
	MCFG_CPU_IO_MAP(a6809_io)
	MCFG_MACHINE_RESET_OVERRIDE(a6809_state, a6809)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(40 * 12, 25 * 20)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 12 - 1, 0, 25 * 20 - 1)
	MCFG_SCREEN_UPDATE_DEVICE("saa5050", saa5050_device, screen_update)
	MCFG_PALETTE_ADD("palette", 8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_DEVICE_ADD("via", VIA6522, XTAL_4MHz / 4)
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(a6809_state, cass_w))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_MC6845_ADD("mc6845", HD6845, "screen", XTAL_4MHz / 2)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(12)

	MCFG_DEVICE_ADD("saa5050", SAA5050, 6000000)
	MCFG_SAA5050_D_CALLBACK(READ8(a6809_state, videoram_r))
	MCFG_SAA5050_SCREEN_SIZE(40, 25, 40)

	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(a6809_state, kbd_put))
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_TIMER_DRIVER_ADD_PERIODIC("a6809_c", a6809_state, a6809_c, attotime::from_hz(4800))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("a6809_p", a6809_state, a6809_p, attotime::from_hz(40000))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( a6809 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "acorn6809.bin", 0x000, 0x800, CRC(5fa5b632) SHA1(b14a884bf82a7a8c23bc03c2e112728dd1a74896) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "acorn6809.ic11", 0x0000, 0x0100, CRC(7908317d) SHA1(e0f1e5bd3a8598d3b62bc432dd1f3892ed7e66d8) ) // address decoder
ROM_END

/* Driver */

/*    YEAR   NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1980, a6809,  0,      0,       a6809,     a6809, driver_device,   0,     "Acorn",  "System 3 (6809 CPU)", 0 )
