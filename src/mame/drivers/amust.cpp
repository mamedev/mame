// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Amust Compak - also known as Amust Executive 816.

2014-03-21 Skeleton driver. [Robbbert]

An unusual-looking CP/M computer.
There are no manuals or schematics known to exist.
The entire driver is guesswork.
The board has LH0080 (Z80A), 2x 8251, 2x 8255, 8253, uPD765A and a HD46505SP-2.
The videoram is a 6116 RAM. There is a piezo beeper. There are 3 crystals,
X1 = 4.9152 (serial chips?), X2 = 16 (CPU), X3 = 14.31818 MHz (Video?).
There are numerous jumpers, all of which perform unknown functions.

The keyboard is a plug-in unit, same idea as Kaypro and Zorba. It has these
chips: INS8035N-6, F74145, 74LS373N, SN75451BP, 2716 rom with label KBD-3.
Crystal: 3.579545 MHz

The main rom is identical between the 2 halves, except that the initial
crtc parameters are slightly different. I've chosen to ignore the first
half. (perhaps 50/60 Hz selectable by jumper?)

Preliminary I/O ports
---------------------
00-01 uart 1
02-03 uart 2
04-07 ppi 1
08-0b ppi 2
0d-0f crtc
10-11 fdc
14-17 pit

PIT.
Having the PIT on ports 14-17 seems to make sense. It sets counters 1 and 2
to mode 3, binary, initial count = 0x80. Counter 0 not used?


Floppy Parameters:
------------------
Double Density
Two Side
80 track
1024 byte sectors
5 sectors/track
800k capacity
128 directory entries
2k block size
Skew 1,3,5,2,4


Stuff that doesn't make sense:
------------------------------
1. To access the screen, it waits for IRQ presumably from sync pulse. It sets INT
mode 0 which means a page-zero jump, but doesn't write anything to the zero-page ram.
That's why I added a RETI at 0038 and set the vector to there. A bit later it writes
a jump at 0000. Then it sets the interrupting device to the fdc (not sure how yet),
then proceeds to overwrite all of page-zero with the disk contents. This of course
kills the jump it just wrote, and my RETI. So it runs into the weeds at high speed.
What should happen is after loading the boot sector succesfully it will jump to 0000,
otherwise it will write BOOT NG to the screen and you're in the monitor. The bios
contains no RETI instructions.
2. At F824 it copies itself to the same address which is presumably shadow ram. But
it never switches to it. The ram is physically in the machine.


Monitor Commands:
-----------------
B = Boot from floppy
(YES! Most useless monitor ever)


ToDo:
- Everything
- Need software
- Keyboard controller needs to be emulated
- If booting straight to CP/M, the load message should be in the middle of the screen.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/upd765.h"
#include "machine/keyboard.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "sound/beep.h"


class amust_state : public driver_device
{
public:
	enum
	{
		TIMER_BEEP_OFF
	};

	amust_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_beep(*this, "beeper")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	DECLARE_DRIVER_INIT(amust);
	DECLARE_MACHINE_RESET(amust);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_READ8_MEMBER(port05_r);
	DECLARE_READ8_MEMBER(port06_r);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_READ8_MEMBER(port09_r);
	DECLARE_READ8_MEMBER(port0a_r);
	DECLARE_WRITE8_MEMBER(port0a_w);
	DECLARE_WRITE8_MEMBER(port0d_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	INTERRUPT_GEN_MEMBER(irq_vs);
	MC6845_UPDATE_ROW(crtc_update_row);
	UINT8 *m_p_videoram;
	const UINT8 *m_p_chargen;
	required_device<palette_device> m_palette;
private:
	UINT8 m_port04;
	UINT8 m_port06;
	UINT8 m_port08;
	UINT8 m_port0a;
	UINT8 m_term_data;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

void amust_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BEEP_OFF:
		m_beep->set_state(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in amust_state::device_timer");
	}
}

//WRITE8_MEMBER( amust_state::port00_w )
//{
//  membank("bankr0")->set_entry(BIT(data, 6));
//  m_fdc->dden_w(BIT(data, 5));
//  floppy_image_device *floppy = NULL;
//  if (BIT(data, 0)) floppy = m_floppy0->get_device();
//  m_fdc->set_floppy(floppy);
//  if (floppy)
//      floppy->ss_w(BIT(data, 4));
//}

static ADDRESS_MAP_START(amust_mem, AS_PROGRAM, 8, amust_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
ADDRESS_MAP_END

static ADDRESS_MAP_START(amust_io, AS_IO, 8, amust_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	//AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	AM_RANGE(0x01, 0x01) AM_READ(port01_r)
	AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi1", i8255_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi2", i8255_device, read, write)
	AM_RANGE(0x0d, 0x0d) AM_READNOP AM_WRITE(port0d_w)
	AM_RANGE(0x0e, 0x0e) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x0f, 0x0f) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x10, 0x11) AM_DEVICE("fdc", upd765a_device, map)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE("pit", pit8253_device, read, write)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( amust_floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_525_QD )
	SLOT_INTERFACE( "drive1", FLOPPY_525_QD )
SLOT_INTERFACE_END

/* Input ports */
static INPUT_PORTS_START( amust )
	PORT_START("P9")
	// bits 6,7 not used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // code @ FB83
	PORT_DIPNAME( 0x10, 0x10, "Boot to Monitor" ) // code @ F895
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f, 0x01, "Unknown" ) // code @ FC99
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
INPUT_PORTS_END

READ8_MEMBER( amust_state::port00_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( amust_state::port01_r )
{
	return 0xff;
}

// bodgy
INTERRUPT_GEN_MEMBER( amust_state::irq_vs )
{
	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xff);
}

READ8_MEMBER( amust_state::port04_r )
{
	return m_port04;
}

WRITE8_MEMBER( amust_state::port04_w )
{
	m_port04 = data;
}

READ8_MEMBER( amust_state::port05_r )
{
	return 0;
}

READ8_MEMBER( amust_state::port06_r )
{
	return m_port06;
}

// BIT 5 low while writing to screen
WRITE8_MEMBER( amust_state::port06_w )
{
	m_port06 = data;
}

READ8_MEMBER( amust_state::port08_r )
{
	return m_port08;
}

// lower 8 bits of video address
WRITE8_MEMBER( amust_state::port08_w )
{
	m_port08 = data;
}

/*
d0 - something to do with type of disk
d1 -
d2 -
d3 -
d4 - H = go to monitor; L = boot from disk
d5 - status of disk-related; loops till NZ
d6 -
d7 -
*/
READ8_MEMBER( amust_state::port09_r )
{
	printf("%s\n",machine().describe_context());
	return ioport("P9")->read();
}

READ8_MEMBER( amust_state::port0a_r )
{
	return m_port0a;
}

/* Bits 7,6,5,3 something to do
with selecting which device causes interrupt?
50, 58 = video sync
70 disk
D0 ?
Bit 4 low = beeper.
Lower 3 bits = upper part of video address */
WRITE8_MEMBER( amust_state::port0a_w )
{
	m_port0a = data;

	if (!BIT(data, 4))
	{
		m_beep->set_state(1);
		timer_set(attotime::from_msec(150), TIMER_BEEP_OFF);
	}
}

WRITE8_MEMBER( amust_state::port0d_w )
{
	UINT16 video_address = m_port08 | ((m_port0a & 7) << 8);
	m_p_videoram[video_address] = data;
}

WRITE8_MEMBER( amust_state::kbd_put )
{
	m_term_data = data;
}

/* F4 Character Displayer */
static const gfx_layout amust_charlayout =
{
	8, 8,                  /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START( amust )
	GFXDECODE_ENTRY( "chargen", 0x0000, amust_charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( amust_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		if (ra < 8)
			gfx = m_p_chargen[(chr<<3) | ra] ^ inv;
		else
			gfx = inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

MACHINE_RESET_MEMBER( amust_state, amust )
{
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("videoram")->base();
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_beep->set_frequency(800);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.write_byte(0x38, 0xed);
	space.write_byte(0x39, 0x4d);
	m_port04 = 0;
	m_port06 = 0;
	m_port08 = 0;
	m_port0a = 0;
	m_maincpu->set_state_int(Z80_PC, 0xf800);
}

DRIVER_INIT_MEMBER( amust_state, amust )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0xf800]);
	membank("bankr0")->configure_entry(0, &main[0x10800]);
	membank("bankw0")->configure_entry(0, &main[0xf800]);
}

static MACHINE_CONFIG_START( amust, amust_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(amust_mem)
	MCFG_CPU_IO_MAP(amust_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", amust_state, irq_vs)
	MCFG_MACHINE_RESET_OVERRIDE(amust_state, amust)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", amust)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_14_31818MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(amust_state, crtc_update_row)

	MCFG_DEVICE_ADD("keybd", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(amust_state, kbd_put))
	MCFG_UPD765A_ADD("fdc", false, true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", amust_floppies, "drive0", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", amust_floppies, "drive1", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	//MCFG_DEVICE_ADD("uart1", I8251, 0)
	//MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	//MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("uart2", I8251, 0)
	//MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	//MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	//MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_rxd))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_cts))
	//MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_dsr))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)

	MCFG_DEVICE_ADD("ppi1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(amust_state, port04_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(amust_state, port04_w))
	MCFG_I8255_IN_PORTB_CB(READ8(amust_state, port05_r))
	MCFG_I8255_IN_PORTC_CB(READ8(amust_state, port06_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(amust_state, port06_w))

	MCFG_DEVICE_ADD("ppi2", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(amust_state, port08_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(amust_state, port08_w))
	MCFG_I8255_IN_PORTB_CB(READ8(amust_state, port09_r))
	MCFG_I8255_IN_PORTC_CB(READ8(amust_state, port0a_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(amust_state, port0a_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( amust )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon_h.rom", 0x10000, 0x1000, CRC(10dceac6) SHA1(1ef80039063f7a6455563d59f1bcc23e09eca369) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "cg4.rom", 0x000, 0x800, CRC(52e7b9d8) SHA1(cc6d457634eb688ccef471f72bddf0424e64b045) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "kbd_3.rom", 0x000, 0x800, CRC(d9441b35) SHA1(ce250ab1e892a13fd75182703f259855388c6bf4) )

	ROM_REGION( 0x800, "videoram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY       FULLNAME       FLAGS */
COMP( 1983, amust,  0,      0,       amust,     amust,   amust_state,   amust,  "Amust", "Amust Executive 816", MACHINE_NOT_WORKING )
