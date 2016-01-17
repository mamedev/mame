// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        SM1800 (original name is CM1800 in cyrilic letters)

        10/12/2010 Skeleton driver.

        On board hardware :
            KR580VM80A central processing unit (i8080)
            KR580VG75  programmable CRT video display controller (i8275)
            KR580VV55  programmable parallel interface (i8255)
            KR580IK51  programmable serial interface/communication controller (i8251)
            KR580VV79  programmable peripheral device, keyboard and display controller (i8279)

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "video/i8275.h"


class sm1800_state : public driver_device
{
public:
	sm1800_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_uart(*this, "i8251"),
	m_ppi(*this, "i8255"),
	m_crtc(*this, "i8275"),
	m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8255_device> m_ppi;
	required_device<i8275_device> m_crtc;
	required_device<palette_device> m_palette;
	DECLARE_WRITE8_MEMBER(sm1800_8255_portb_w);
	DECLARE_WRITE8_MEMBER(sm1800_8255_portc_w);
	DECLARE_READ8_MEMBER(sm1800_8255_porta_r);
	DECLARE_READ8_MEMBER(sm1800_8255_portc_r);
	UINT8 m_irq_state;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(sm1800);
	INTERRUPT_GEN_MEMBER(sm1800_vblank_interrupt);
	IRQ_CALLBACK_MEMBER(sm1800_irq_callback);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );
};

static ADDRESS_MAP_START(sm1800_mem, AS_PROGRAM, 8, sm1800_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
	//AM_RANGE( 0x0fb0, 0x0fff ) AM_DEVWRITE("i8275", i8275_device, dack_w)
	AM_RANGE( 0x1000, 0x17ff ) AM_RAM // videoram looks like 1080-17FF, normal ascii
ADDRESS_MAP_END

static ADDRESS_MAP_START( sm1800_io, AS_IO, 8, sm1800_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x3c, 0x3d ) AM_DEVREADWRITE("i8275", i8275_device, read, write)
	AM_RANGE( 0x5c, 0x5c) AM_DEVREADWRITE("i8251", i8251_device, data_r, data_w)
	AM_RANGE( 0x5d, 0x5d) AM_DEVREADWRITE("i8251", i8251_device, status_r, control_w)
	AM_RANGE( 0x6c, 0x6f ) AM_DEVREADWRITE("i8255", i8255_device, read, write)
	//AM_RANGE( 0x74, 0x74 ) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
	//AM_RANGE( 0x75, 0x75 ) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sm1800 )
INPUT_PORTS_END

IRQ_CALLBACK_MEMBER(sm1800_state::sm1800_irq_callback)
{
	return 0xff;
}

void sm1800_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(sm1800_state::sm1800_vblank_interrupt)
{
	m_maincpu->set_input_line(0, m_irq_state ?  HOLD_LINE : CLEAR_LINE);
	m_irq_state ^= 1;
}

I8275_DRAW_CHARACTER_MEMBER( sm1800_state::crtc_display_pixels )
{
	int i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *charmap = memregion("chargen")->base();
	UINT8 pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp)
		pixels = 0;

	if (lten)
		pixels = 0xff;

	if (rvv)
		pixels ^= 0xff;

	for(i=0;i<8;i++)
		bitmap.pix32(y, x + i) = palette[(pixels >> (7-i)) & 1 ? (hlgt ? 2 : 1) : 0];
}

WRITE8_MEMBER( sm1800_state::sm1800_8255_portb_w )
{
}

WRITE8_MEMBER( sm1800_state::sm1800_8255_portc_w )
{
}

READ8_MEMBER( sm1800_state::sm1800_8255_porta_r )
{
	return 0xff;
}

READ8_MEMBER( sm1800_state::sm1800_8255_portc_r )
{
	return 0;
}

PALETTE_INIT_MEMBER(sm1800_state, sm1800)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, 0xa0, 0xa0, 0xa0); // white
	palette.set_pen_color(2, rgb_t::white); // highlight
}


/* F4 Character Displayer */
static const gfx_layout sm1800_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( sm1800 )
	GFXDECODE_ENTRY( "chargen", 0x0000, sm1800_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( sm1800, sm1800_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(sm1800_mem)
	MCFG_CPU_IO_MAP(sm1800_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sm1800_state,  sm1800_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(sm1800_state,sm1800_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DEVICE("i8275", i8275_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(sm1800_state, sm1800)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sm1800)

	/* Devices */
	MCFG_DEVICE_ADD("i8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(sm1800_state, sm1800_8255_porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(sm1800_state, sm1800_8255_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(sm1800_state, sm1800_8255_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sm1800_state, sm1800_8255_portc_w))

	MCFG_DEVICE_ADD("i8275", I8275, 2000000)
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(sm1800_state, crtc_display_pixels)

	MCFG_DEVICE_ADD("i8251", I8251, 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sm1800 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "prog.bin", 0x0000, 0x0800, CRC(55736ad5) SHA1(b77f720f1b64b208dd6a5d4f9c9521d1284028e9))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "font.bin", 0x0000, 0x0800, CRC(28ed9ebc) SHA1(f561136962a06a5dcb5a0436931d29e940155d24))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( ????, sm1800,  0,       0,     sm1800,    sm1800, driver_device,     0,   "<unknown>", "SM1800", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
