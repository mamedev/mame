// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Intertec SuperBrain

2013-08-19 Skeleton

Chips: 2x Z80; FD1791; 2x 8251; 8255; BR1941; CRT8002; KR3600; DP8350
Xtals: 16.0, 10.92, 5.0688
Disk parameters: 512 bytes x 10 sectors x 35 tracks. 1 and 2-sided disks supported.
Sound: Beeper

The boot prom is shared between both cpus. This feat is accomplished by holding the sub cpu
 in reset, until the main cpu has prepared a few memory locations. The first thing in the rom
 is to check these locations, and then program flow splits into 2 sections, one for each cpu.
The main cpu does a busreq to gain access to the sub cpu's 1k static ram. When the sub cpu
 responds with busack, then the main cpu switches bank2. In emulation, it isn't actually
 necessary to stop the sub cpu because of other handshaking. Our Z80 emulation doesn't
 support the busack signal anyway, so we just assume it is granted immediately.

The schematic in parts is difficult to read. Some assumptions have been made.

To Do:
- Without a disk in, it should display a message to insert a disk. Doesn't happen.
  It thinks a disk is in and tries to execute garbage in the disk buffer instead.
- Port 08 is largely a guess.
- No work done on the keyboard. Should be able to hook up the generic ascii keyboard.
- No software available to try.
- Bug in floppy code means the motor will turn off after a few seconds. It should stay on.
- Video chips need to be emulated (CRT8002 and DP8350), attributes etc.
- After each line of characters, an interrupt should be generated. 25 IRQs per screen.
- Hook the outputs of the baud rate generator to the uarts.
- Probably lots of other stuff.

*************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/com8116.h"
#include "sound/beep.h"

class sbrain_state : public driver_device
{
public:
	sbrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_beep(*this, "beeper"),
		m_brg(*this, "brg"),
		m_u0(*this, "uart0"),
		m_u1(*this, "uart1"),
		m_ppi(*this, "ppi"),
		m_fdc (*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_vs(*this, "VS"),
		m_bankr0(*this, "bankr0"),
		m_bankw0(*this, "bankw0"),
		m_bank2(*this, "bank2") {}

public:
	const UINT8 *m_p_chargen;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<UINT8> m_p_videoram;
	DECLARE_DRIVER_INIT(sbrain);
	DECLARE_MACHINE_RESET(sbrain);
	DECLARE_READ8_MEMBER(ppi_pa_r);
	DECLARE_WRITE8_MEMBER(ppi_pa_w);
	DECLARE_READ8_MEMBER(ppi_pb_r);
	DECLARE_WRITE8_MEMBER(ppi_pb_w);
	DECLARE_READ8_MEMBER(ppi_pc_r);
	DECLARE_WRITE8_MEMBER(ppi_pc_w);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(baud_w);
	DECLARE_WRITE_LINE_MEMBER(fr_w);
	DECLARE_WRITE_LINE_MEMBER(ft_w);
private:
	UINT8 m_porta;
	UINT8 m_portb;
	UINT8 m_portc;
	UINT8 m_port08;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<beep_device> m_beep;
	required_device<com8116_device> m_brg;
	required_device<i8251_device> m_u0;
	required_device<i8251_device> m_u1;
	required_device<i8255_device> m_ppi;
	required_device<fd1791_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_ioport m_vs;
	required_memory_bank m_bankr0;
	required_memory_bank m_bankw0;
	required_memory_bank m_bank2;
};

static ADDRESS_MAP_START( sbrain_mem, AS_PROGRAM, 8, sbrain_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x4000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xbfff ) AM_RAMBANK("bank2")
	AM_RANGE( 0xc000, 0xf7ff ) AM_RAM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrain_io, AS_IO, 8, sbrain_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, data_r, data_w)
	AM_RANGE(0x41, 0x41) AM_MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, status_r, control_w)
	//AM_RANGE(0x48, 0x4f) chr_int_latch
	//AM_RANGE(0x50, 0x57) key_in
	AM_RANGE(0x58, 0x58) AM_MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x59, 0x59) AM_MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0x60, 0x67) AM_WRITE(baud_w)
	AM_RANGE(0x68, 0x6b) AM_MIRROR(4) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrain_submem, AS_PROGRAM, 8, sbrain_state )
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
	AM_RANGE( 0x8800, 0x8bff ) AM_RAM AM_REGION("subcpu", 0x8800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrain_subio, AS_IO, 8, sbrain_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x08) AM_READWRITE(port08_r,port08_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("fdc", fd1791_t, read, write)
ADDRESS_MAP_END

// bit 0 is wrong, maybe the whole byte is wrong
READ8_MEMBER( sbrain_state::port08_r )
{
	return m_port08;
}

/* Misc disk functions
d0 : disk busy?
d1 : SEL A (drive 0?)
d2 : SEL B (drive 1?)
d3 : SEL C
d4 : SEL D
d5 : side select
d6,7 : not used
*/
WRITE8_MEMBER( sbrain_state::port08_w )
{
	m_port08 = data | 0xc0;

	floppy_image_device *floppy = nullptr;
	if (BIT(m_port08, 1)) floppy = m_floppy0->get_device();
	if (BIT(m_port08, 2)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->ss_w(BIT(m_port08, 5)); // might need inverting
	}

	// refresh motor on, hopefully this will keep them turning
	m_floppy0->get_device()->mon_w(0); // motors run all the time
	m_floppy1->get_device()->mon_w(0);
}

WRITE_LINE_MEMBER( sbrain_state::fr_w )
{
}

WRITE_LINE_MEMBER( sbrain_state::ft_w )
{
}

WRITE8_MEMBER( sbrain_state::baud_w )
{
	m_brg->str_w(data & 0x0f);
	m_brg->stt_w(data >> 4);
}

READ8_MEMBER( sbrain_state::ppi_pa_r )
{
	return m_porta;
}

/* Video functions:
d0,1 : 11 = alphanumeric; 10 = external ;other = graphics
d2 : Underline
d3,4 : not used
d5 : strike through
d6 : 1=60hz 0=50hz
d7 : reverse video
*/
WRITE8_MEMBER( sbrain_state::ppi_pa_w )
{
	m_porta = data;
}

/* Inputs
d0 : data ready from keyboard
d1 : key held down
d2 : Vert Blank
d3 : not used
d4 : /capslock
d5 : disk is busy
d6 : Ring Indicator line from main rs232 port, 1=normal, 0=set
d7 : cpu2 /busak line
*/
READ8_MEMBER( sbrain_state::ppi_pb_r )
{
	return m_portb | 0x50 | m_vs->read() | (BIT(m_port08, 0) << 5) | ((UINT8)BIT(m_portc, 5) << 7);
}

WRITE8_MEMBER( sbrain_state::ppi_pb_w )
{
	m_portb = data & 8;
}

READ8_MEMBER( sbrain_state::ppi_pc_r )
{
	return m_portc;
}

/* System
d0 : 1 = bank 0 disabled
d1 : character blanking
d2 : 1=enable rom, 0=enable ram bank 0
d3 : cpu2 reset line
d4 : 1=enable ram bank 2, 0=bank 2 uses disk buffer
d5 : cpu2 /busreq line
d6 : beeper
d7 : keyboard, 1=enable comms, 0=reset
*/
WRITE8_MEMBER( sbrain_state::ppi_pc_w )
{
	m_portc = data;
	m_beep->set_state(BIT(data, 6));
	m_bankr0->set_entry(BIT(data, 2));
	m_bank2->set_entry(BIT(data, 4));

	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( sbrain )
	/* vblank */
	PORT_START("VS")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
INPUT_PORTS_END

DRIVER_INIT_MEMBER( sbrain_state, sbrain )
{
	UINT8 *main = memregion("maincpu")->base();
	UINT8 *sub = memregion("subcpu")->base();

	m_bankr0->configure_entry(0, &main[0x0000]);
	m_bankr0->configure_entry(1, &sub[0x0000]);
	m_bankw0->configure_entry(0, &main[0x0000]);
	m_bank2->configure_entry(0, &sub[0x8000]);
	m_bank2->configure_entry(1, &main[0x8000]);
}

static SLOT_INTERFACE_START( sbrain_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

MACHINE_RESET_MEMBER( sbrain_state, sbrain )
{
	m_p_chargen = memregion("chargen")->base();
	m_bankr0->set_entry(1); // point at rom
	m_bankw0->set_entry(0); // always write to ram
	m_bank2->set_entry(1); // point at maincpu bank
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // hold subcpu in reset
}

UINT32 sbrain_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x+ma];

					if (chr) gfx = m_p_chargen[(chr<<4) | ra ];
				}
				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=80;
	}
	return 0;
}

static MACHINE_CONFIG_START( sbrain, sbrain_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(sbrain_mem)
	MCFG_CPU_IO_MAP(sbrain_io)
	MCFG_MACHINE_RESET_OVERRIDE(sbrain_state, sbrain)
	MCFG_CPU_ADD("subcpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(sbrain_submem)
	MCFG_CPU_IO_MAP(sbrain_subio)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::amber)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(sbrain_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 800)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* Devices */
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(sbrain_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(sbrain_state, ppi_pa_w))
	MCFG_I8255_IN_PORTB_CB(READ8(sbrain_state, ppi_pb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(sbrain_state, ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(sbrain_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sbrain_state, ppi_pc_w))

	MCFG_DEVICE_ADD("uart0", I8251, 0)

	MCFG_DEVICE_ADD("uart1", I8251, 0)

	MCFG_DEVICE_ADD("brg", COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(sbrain_state, fr_w))
	MCFG_COM8116_FT_HANDLER(WRITELINE(sbrain_state, ft_w))

	MCFG_FD1791_ADD("fdc", XTAL_16MHz / 16)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", sbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", sbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

ROM_START( sbrain )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "subcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "superbrain.bin", 0x0000, 0x0800, CRC(b6a2e6a5) SHA1(a646faaecb9ac45ee1a42764628e8971524d5c13) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

COMP( 1981, sbrain, 0, 0, sbrain, sbrain, sbrain_state, sbrain, "Intertec", "Superbrain", MACHINE_NOT_WORKING )
