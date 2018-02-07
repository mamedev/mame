// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Skeleton driver for XaviX TV PNP console and childs (Let's! Play TV Classic)

    CPU is an M6502 derivative where opcode 0x22 has 3 bytes of operands.
    Definitely not: 65C816 or Mitsu M740.

    Code at F34F is thus:

    F34F:  STA $6200,X
    F352:  INX
    F353:  BNE $F34F
    F355:  UNK 12 FA 80
    F359:  UNK 12 A8 80
    F35D:  UNK 12 1B 80
    F361:  SEC
    F362:  LDA #$CD
    F364:  SBC #$CA

    later on

    F3C9:  UNK 00 E4 C4

    TODO:
    - identify CPU
    - figure out ROM banking

    Notes from http://www.videogameconsolelibrary.com/pg00-xavix.htm#page=reviews (thanks Guru!)

    XaviXPORT arrived on the scene with 3 game titles (XaviX Tennis, XaviX Bowling and XaviX Baseball) using their
    original XaviX Multiprocessor.  This proprietary chip is reported to contain an 8-bit high speed central processing
    unit (6502) at 21 MHz, picture processor, sound processor, DMA controller, 1K bytes high speed RAM, universal timer,
    AD/Converter and I/O device control.  Each cartridge comes with a wireless peripheral to be used with the game (Baseball Bat,
    Tennis Racquet, etc.) that requires "AA" batteries.  The XaviXPORT system retailed for $79.99 USD with the cartridges
    retailing for $49.99 USD.

    The following year at CES 2005, SSD COMPANY LIMITED introduced two new XaviXPORT titles (XaviX Golf and XaviX Bass Fishing) each
    containing the upgraded "Super XaviX".  This new chip is said to sport a 16-bit high central processing unit (65816) at 43 MHz.
    SSD COMPANY LIMITED is already working on their next chip called "XaviX II" that is said to be a 32-bit RISC processor
    with 3D capabilities.

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/xavix.h"
#include "machine/timer.h"
#include "screen.h"
#include "speaker.h"


#define MAIN_CLOCK XTAL(21'477'272)

class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void xavix(machine_config &config);

	DECLARE_WRITE8_MEMBER(xavix_7900_w);

	DECLARE_WRITE8_MEMBER(xavix_7980_w);
	DECLARE_WRITE8_MEMBER(xavix_7981_w);
	DECLARE_WRITE8_MEMBER(xavix_7982_w);
	DECLARE_WRITE8_MEMBER(xavix_7983_w);
	DECLARE_WRITE8_MEMBER(xavix_7984_w);
	DECLARE_WRITE8_MEMBER(xavix_7985_w);
	DECLARE_WRITE8_MEMBER(xavix_7986_w);
	DECLARE_WRITE8_MEMBER(xavix_7987_w);
	DECLARE_READ8_MEMBER(xavix_7980_r);


	DECLARE_WRITE8_MEMBER(xavix_7ff9_w);
	DECLARE_WRITE8_MEMBER(xavix_7ffa_w);
	DECLARE_WRITE8_MEMBER(xavix_7ffb_w);
	DECLARE_WRITE8_MEMBER(xavix_7ffe_w);
	DECLARE_WRITE8_MEMBER(xavix_7fff_w);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	uint8_t m_xavix_7981_data;
	uint8_t m_xavix_7982_data;
	uint8_t m_xavix_7983_data;

	uint8_t m_xavix_7984_data;
	uint8_t m_xavix_7985_data;

	uint8_t m_xavix_7986_data;
	uint8_t m_xavix_7987_data;

	uint8_t m_xavix_7ff9_data;
	uint8_t m_xavix_7ffa_data;
	uint8_t m_xavix_7ffb_data;
	uint8_t m_xavix_7ffe_data;
	uint8_t m_xavix_7fff_data;
	
	uint8_t get_vectors(int which, int half);

};

void xavix_state::video_start()
{
}

uint32_t xavix_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}


WRITE8_MEMBER(xavix_state::xavix_7980_w)
{
	logerror("%s: xavix_7980_w %02x (79xx unk trigger for 7981-7987?)\n", machine().describe_context(), data);

	uint32_t source = (m_xavix_7983_data << 16) | (m_xavix_7982_data<<8) | m_xavix_7981_data;
	uint16_t dest = (m_xavix_7985_data<<8) | m_xavix_7984_data;
	uint16_t len = (m_xavix_7987_data<<8) | m_xavix_7986_data;

	// TODO: don't do tag lookups here once satisfied this is correct
	const uint32_t rgnlen = memregion("bios")->bytes();
	uint8_t *rgn = memregion("bios")->base();
	
	source &= rgnlen-1;
	logerror("  (possible DMA op SRC %08x DST %04x LEN %04x)\n", source, dest, len);

	address_space& destspace = m_maincpu->space(AS_PROGRAM);

	for (int i = 0; i < len; i++)
	{
		uint8_t dat = rgn[(source + i) & (rgnlen-1)];
		destspace.write_byte(dest + i, dat);
	}
}

WRITE8_MEMBER(xavix_state::xavix_7981_w)
{
	logerror("%s: xavix_7981_w %02x (79xx unk triplet byte 1)\n", machine().describe_context(), data);
	m_xavix_7981_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7982_w)
{
	logerror("%s: xavix_7982_w %02x (79xx unk triplet byte 2)\n", machine().describe_context(), data);
	m_xavix_7982_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7983_w)
{
	logerror("%s: xavix_7983_w %02x (79xx unk triplet byte 3)\n", machine().describe_context(), data);
	m_xavix_7983_data = data;
	// this would mean Taito Nostalgia relies on mirroring tho, as it has the high bits set... so could just be wrong
	logerror("  (possible DMA ROM source of %02x%02x%02x)\n", m_xavix_7983_data, m_xavix_7982_data, m_xavix_7981_data);
}

WRITE8_MEMBER(xavix_state::xavix_7984_w)
{
	logerror("%s: xavix_7984_w %02x (79xx unk pair 1 byte 1)\n", machine().describe_context(), data);
	m_xavix_7984_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7985_w)
{
	logerror("%s: xavix_7985_w %02x (79xx unk pair 1 byte 2)\n", machine().describe_context(), data);
	m_xavix_7985_data = data;

	logerror("  (possible DMA dest of %02x%02x)\n", m_xavix_7985_data, m_xavix_7984_data);
}

WRITE8_MEMBER(xavix_state::xavix_7986_w)
{
	logerror("%s: xavix_7986_w %02x (79xx unk pair 2 byte 1)\n", machine().describe_context(), data);
	m_xavix_7986_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7987_w)
{
	logerror("%s: xavix_7987_w %02x (79xx unk pair 2 byte 2)\n", machine().describe_context(), data);
	m_xavix_7987_data = data;

	logerror("  (possible DMA len of %02x%02x)\n", m_xavix_7987_data, m_xavix_7986_data);
}

READ8_MEMBER(xavix_state::xavix_7980_r)
{
	logerror("%s: xavix_7980_r (79xx unk operation status?)\n", machine().describe_context());
	return 0x00;
}



WRITE8_MEMBER(xavix_state::xavix_7ff9_w)
{
	logerror("%s: xavix_7ff9_w %02x (trigger for 7ffx region?)\n", machine().describe_context(), data);
	m_xavix_7ff9_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7ffa_w)
{
	logerror("%s: xavix_7ffa_w %02x (7ffx pair 1, byte 1?)\n", machine().describe_context(), data);
	m_xavix_7ffa_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7ffb_w)
{
	logerror("%s: xavix_7ffb_w %02x (7ffx pair 1, byte 2?)\n", machine().describe_context(), data);
	m_xavix_7ffb_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7ffe_w)
{
	logerror("%s: xavix_7ffe_w %02x (7ffx pair 2, byte 1?)\n", machine().describe_context(), data);
	m_xavix_7ffe_data = data;
}

WRITE8_MEMBER(xavix_state::xavix_7fff_w)
{
	logerror("%s: xavix_7fff_w %02x (7ffx pair 2, byte 2?)\n", machine().describe_context(), data);
	m_xavix_7fff_data = data;
}


WRITE8_MEMBER(xavix_state::xavix_7900_w)
{
	logerror("%s: xavix_7900_w %02x (---FIRST WRITE ON STARTUP---)\n", machine().describe_context(), data);
}



TIMER_DEVICE_CALLBACK_MEMBER(xavix_state::scanline_cb)
{
//	int scanline = param;
}

INTERRUPT_GEN_MEMBER(xavix_state::interrupt)
{
//	if (m_xavix_7ff9_data != 0)
//		m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);

	if (m_xavix_7ff9_data != 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
}




static ADDRESS_MAP_START( xavix_map, AS_PROGRAM, 8, xavix_state )
	AM_RANGE(0x000000, 0x0001ff) AM_RAM
	AM_RANGE(0x000200, 0x003fff) AM_RAM // ends up jumping to code here? must be a DMA?

	AM_RANGE(0x006200, 0x0062ff) AM_RAM // cleared to 0x80

	AM_RANGE(0x006900, 0x006900) AM_WRITENOP // startup (taitons1)

	AM_RANGE(0x006fc0, 0x006fc0) AM_WRITENOP // startup

	AM_RANGE(0x006fd8, 0x006fd8) AM_WRITENOP // startup (taitons1)

	AM_RANGE(0x006fe9, 0x006fe9) AM_WRITENOP // startup

	AM_RANGE(0x006ff1, 0x006ff1) AM_WRITENOP // startup (taitons1)

	AM_RANGE(0x006ff8, 0x006ff8) AM_READNOP AM_WRITENOP // startup

	// taitons1 after 75f7/75f8
	//AM_RANGE(0x0075f6, 0x0075f6) AM_WRITE(xavix_75f6_w)
	// taitons1 written as a pair
	//AM_RANGE(0x0075f7, 0x0075f7) AM_WRITE(xavix_75f7_w)
	//AM_RANGE(0x0075f8, 0x0075f8) AM_WRITE(xavix_75f8_w)
	// taitons1 written after 75f6, then read
	//AM_RANGE(0x0075f9, 0x0075f9) AM_READWRITE(xavix_75f9_r, xavix_75f9_w)
	// taitons1 written other 75xx operations
	//AM_RANGE(0x0075ff, 0x0075ff) AM_WRITE(xavix_75ff_w)

	AM_RANGE(0x007810, 0x007810) AM_WRITENOP // startup

	AM_RANGE(0x007900, 0x007900) AM_WRITE(xavix_7900_w) // startup
	AM_RANGE(0x007902, 0x007902) AM_WRITENOP // startup

	// DMA trigger for below (written after the others) waits on status of bit 1 in a loop
	AM_RANGE(0x007980, 0x007980) AM_READWRITE(xavix_7980_r, xavix_7980_w)
	// DMA source
	AM_RANGE(0x007981, 0x007981) AM_WRITE(xavix_7981_w)
	AM_RANGE(0x007982, 0x007982) AM_WRITE(xavix_7982_w)
	AM_RANGE(0x007983, 0x007983) AM_WRITE(xavix_7983_w)
	// DMA dest
	AM_RANGE(0x007984, 0x007984) AM_WRITE(xavix_7984_w)
	AM_RANGE(0x007985, 0x007985) AM_WRITE(xavix_7985_w)
	// DMA length
	AM_RANGE(0x007986, 0x007986) AM_WRITE(xavix_7986_w)
	AM_RANGE(0x007987, 0x007987) AM_WRITE(xavix_7987_w)

	AM_RANGE(0x007a00, 0x007a00) AM_READNOP // startup (taitons1)
	AM_RANGE(0x007a01, 0x007a01) AM_READNOP // startup (taitons1)

	AM_RANGE(0x007a03, 0x007a03) AM_READNOP AM_WRITENOP // startup

	// maybe irq enable, written after below
	AM_RANGE(0x007ff9, 0x007ff9) AM_WRITE(xavix_7ff9_w)
	// an IRQ vector (nmi?)
	AM_RANGE(0x007ffa, 0x007ffa) AM_WRITE(xavix_7ffa_w)
	AM_RANGE(0x007ffb, 0x007ffb) AM_WRITE(xavix_7ffb_w)
	// an IRQ vector (irq?)
	AM_RANGE(0x007ffe, 0x007ffe) AM_WRITE(xavix_7ffe_w)
	AM_RANGE(0x007fff, 0x007fff) AM_WRITE(xavix_7fff_w)

	AM_RANGE(0x008000, 0x1fffff) AM_ROM AM_REGION("bios", 0x008000)
ADDRESS_MAP_END

static INPUT_PORTS_START( xavix )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* correct, 4bpp gfxs */
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( xavix )
	GFXDECODE_ENTRY( "bios", 0, charlayout,     0, 1 )
GFXDECODE_END


void xavix_state::machine_start()
{
}

void xavix_state::machine_reset()
{
	m_xavix_7981_data = 0;
	m_xavix_7982_data = 0;
	m_xavix_7983_data = 0;

	m_xavix_7984_data = 0;
	m_xavix_7985_data = 0;

	m_xavix_7986_data = 0;
	m_xavix_7987_data = 0;

	m_xavix_7ff9_data = 0;
	m_xavix_7ffa_data = 0;
	m_xavix_7ffb_data = 0;
	m_xavix_7ffe_data = 0;
	m_xavix_7fff_data = 0;
}

typedef device_delegate<uint8_t (int which, int half)> xavix_interrupt_vector_delegate;

uint8_t xavix_state::get_vectors(int which, int half)
{
//	logerror("get_vectors %d %d\n", which, half);

	if (which == 0) // NMI
	{
		if (half == 0)
			return m_xavix_7ffb_data;
		else
			return m_xavix_7ffa_data;
	}
	else
	{
		if (half == 0)
			return m_xavix_7fff_data;
		else
			return m_xavix_7ffe_data;
	}
}

MACHINE_CONFIG_START(xavix_state::xavix)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",XAVIX,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(xavix_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", xavix_state,  interrupt)
	MCFG_XAVIX_VECTOR_CALLBACK(xavix_state, get_vectors)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", xavix_state, scanline_cb, "screen", 0, 1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(xavix_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xavix)

	MCFG_PALETTE_ADD("palette", 16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( taitons1 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taitonostalgia1.u3", 0x000000, 0x200000, CRC(25bd8c67) SHA1(a109cd2da6aa4596e3ca3abd1afce2d0001a473f) )
ROM_END

ROM_START( rad_ping )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "pingpong.bin", 0x000000, 0x100000, CRC(629f7f47) SHA1(2bb19fd202f1e6c319d2f7d18adbfed8a7669235) )
ROM_END

ROM_START( xavtenni )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xavixtennis.bin", 0x000000, 0x800000, CRC(23a1d918) SHA1(2241c59e8ea8328013e55952ebf9060ea0a4675b) )
ROM_END

/* The 'XaviXPORT' isn't a real console, more of a TV adapter, all the actual hardware (CPU including video hw, sound hw) is in the cartridges and controllers
   and can vary between games, see notes at top of driver.

   According to sources XaviX Tennis should be a standard XaviX CPU, but at the very least makes significantly more use of custom opcodes than Taito Nostalgia
   and Radica Ping Pong which only appears to use the call far / return far for extended memory space.
   
   Furthermore it also seems to require some regular 6502 opcodes to be replaced with custom ones, yet the other games expect these to act normally.  This
   leads me to believe that XaviX Tennis is more likely to be a Super XaviX title.

   The CPU die on XaviX Tennis is internally marked as NEC 85054-611

*/
CONS( 2004, xavtenni,  0,   0,  xavix,  xavix, xavix_state, 0, "SSD Company LTD", "XaviX Tennis (XaviXPORT)", MACHINE_IS_SKELETON )

/* Standalone TV Games 

  These all have jumps to the 3xxx region, which appears to be RAM (or possibly a banked space?) hopefully it's just missing some kind of DMA transfer and
  isn't some additional internal ROM.

*/
CONS( 2006, taitons1,  0,   0,  xavix,  xavix, xavix_state, 0, "Bandai / SSD Company LTD / Taito", "Let's! TV Play Classic - Taito Nostalgia 1", MACHINE_IS_SKELETON )

CONS( 2000, rad_ping,  0,   0,  xavix,  xavix, xavix_state, 0, "Radica / SSD Company LTD / Simmer Technology", "Play TV Ping Pong", MACHINE_IS_SKELETON ) // "Simmer Technology" is also known as "Hummer Technology Co., Ltd"
