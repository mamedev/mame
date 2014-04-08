// license:MAME
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
half.

Floppy Parameters:
Double Density
Two Side
80 track
1024 byte sectors
5 sectors/track
800k capacity
128 directory entries
2k block size
Skew 1,3,5,2,4


Monitor Commands:
B = Boot from floppy
(YES! Most useless monitor ever)


ToDo:
- Everything
- Need software
- If booting straight to CP/M, the load message should be in the middle of the screen.
- Beeper is a low pulse on bit 0 of port 0b - enable a pit event?

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/upd765.h"
#include "machine/keyboard.h"
//#include "machine/pit8253.h"
//#include "machine/i8255.h"
//#include "machine/i8251.h"


class amust_state : public driver_device
{
public:
	amust_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	DECLARE_DRIVER_INIT(amust);
	DECLARE_MACHINE_RESET(amust);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_READ8_MEMBER(port09_r);
	DECLARE_READ8_MEMBER(port0a_r);
	DECLARE_WRITE8_MEMBER(port0a_w);
	DECLARE_WRITE8_MEMBER(port0d_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	INTERRUPT_GEN_MEMBER(irq_vs);
	UINT8 *m_p_videoram;
	const UINT8 *m_p_chargen;
	required_device<palette_device> m_palette;
private:
	UINT8 m_port08;
	UINT8 m_port0a;
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

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
	AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	AM_RANGE(0x08, 0x08) AM_WRITE(port08_w)
	AM_RANGE(0x09, 0x09) AM_READ(port09_r)
	AM_RANGE(0x0a, 0x0a) AM_READWRITE(port0a_r,port0a_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(port0d_w)
	AM_RANGE(0x0e, 0x0e) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x0f, 0x0f) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x10, 0x11) AM_DEVICE("fdc", upd765a_device, map)
	//AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	//AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	//AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	//AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
	//AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi1", i8255_device, read, write)
	//AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi2", i8255_device, read, write)
	//AM_RANGE(0x14, 0x17) AM_DEVREADWRITE("pit", pit8253_device, read, write)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( amust_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

/* Input ports */
static INPUT_PORTS_START( amust )
INPUT_PORTS_END

READ8_MEMBER( amust_state::port00_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( amust_state::port08_w )
{
	m_port08 = data;
}

// bit 4: H = go to monitor; L = boot from disk
READ8_MEMBER( amust_state::port09_r )
{
	return 0xff;
}

READ8_MEMBER( amust_state::port0a_r )
{
	return m_port0a;
}

WRITE8_MEMBER( amust_state::port0a_w )
{
	m_port0a = data;
}

WRITE8_MEMBER( amust_state::port0d_w )
{
	UINT16 video_address = m_port08 | ((m_port0a & 7) << 8);
	m_p_videoram[video_address] = data;
}

// bodgy
INTERRUPT_GEN_MEMBER( amust_state::irq_vs )
{
	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xcf);
}

//static I8255_INTERFACE( ppi1_intf )
//{
//  DEVCB_DRIVER_MEMBER(amust_state, ppi1_pa_r),   // Port A read
//  DEVCB_DRIVER_MEMBER(amust_state, ppi1_pa_w),   // Port A write
//  DEVCB_DRIVER_MEMBER(amust_state, ppi1_pb_r),   // Port B read
//  DEVCB_DRIVER_MEMBER(amust_state, ppi1_pb_w),   // Port B write
//  DEVCB_DRIVER_MEMBER(amust_state, ppi1_pc_r),   // Port C read
//  DEVCB_DRIVER_MEMBER(amust_state, ppi1_pc_w),   // Port C write
//};

//static I8255_INTERFACE( ppi2_intf )
//{
//  DEVCB_DRIVER_MEMBER(amust_state, ppi2_pa_r),   // Port A read
//  DEVCB_DRIVER_MEMBER(amust_state, ppi2_pa_w),   // Port A write
//  DEVCB_DRIVER_MEMBER(amust_state, ppi2_pb_r),   // Port B read
//  DEVCB_DRIVER_MEMBER(amust_state, ppi2_pb_w),   // Port B write
//  DEVCB_DRIVER_MEMBER(amust_state, ppi2_pc_r),   // Port C read
//  DEVCB_DRIVER_MEMBER(amust_state, ppi2_pc_w),   // Port C write
//};

WRITE8_MEMBER( amust_state::kbd_put )
{
	m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(amust_state, kbd_put)
};


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

MC6845_UPDATE_ROW( amust_update_row )
{
	amust_state *state = device->machine().driver_data<amust_state>();
	const rgb_t *palette = state->m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = state->m_p_videoram[mem];
		if (ra < 8)
			gfx = state->m_p_chargen[(chr<<3) | ra] ^ inv;
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

static MC6845_INTERFACE( amust_crtc )
{
	false,
	0,0,0,0,                /* visarea adjustment */
	8,                      /* number of dots per character */
	NULL,
	amust_update_row,       /* handler to display a scanline */
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

MACHINE_RESET_MEMBER( amust_state, amust )
{
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("videoram")->base();
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.write_byte(8, 0xc9);
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

	/* Devices */
	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_14_31818MHz / 8, amust_crtc)
	MCFG_ASCII_KEYBOARD_ADD("keybd", keyboard_intf)
	MCFG_UPD765A_ADD("fdc", false, true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", amust_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", amust_floppies, "525dd", floppy_image_device::default_floppy_formats)

	//MCFG_DEVICE_ADD("uart1", I8251, 0)
	//MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	//MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	//MCFG_DEVICE_ADD("uart2", I8251, 0)
	//MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	//MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	//MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "serial_terminal")
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_rxd))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_cts))
	//MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_dsr))

	//MCFG_DEVICE_ADD("pit", PIT8253, 0)

	//MCFG_I8255A_ADD("ppi1", ppi1_intf)
	//MCFG_I8255A_ADD("ppi2", ppi2_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( amust )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon_h.rom", 0x10000, 0x1000, CRC(10dceac6) SHA1(1ef80039063f7a6455563d59f1bcc23e09eca369) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "cg4.rom", 0x000, 0x800, CRC(52e7b9d8) SHA1(cc6d457634eb688ccef471f72bddf0424e64b045) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "kbd_3.rom", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "videoram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY       FULLNAME       FLAGS */
COMP( 1983, amust,  0,      0,       amust,     amust,   amust_state,   amust,  "Amust",       "Compak", GAME_IS_SKELETON)
