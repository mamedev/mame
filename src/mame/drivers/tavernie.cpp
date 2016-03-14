// license:BSD-3-Clause
// copyright-holders: Robbbert
/***************************************************************************

    Tavernier CPU09 and IVG09 (Realisez votre ordinateur individuel)

    2013-12-08 Skeleton driver.

    This system was described in a French computer magazine.

    CPU09 includes 6809, 6821, 6840, 6850, cassette, rs232
    IVG09 includes 6845, another 6821, beeper
    IFD09 includes WD1795

ToDo:
    - Attributes ram
    - Graphics
    - Character rom is not dumped
    - Graphics rom is not dumped
    - 3x 7611 proms not dumped
    - Fix cassette
    - Test FDC
    - Need software

List of commands (must be in UPPERCASE):
A -
B -
C -
D - Dump memory (^X to break)
G -
I -
L - Load cassette
M -
N -
O -
P - Save cassette
Q -
R - Display/Alter Registers
S -
T -
U -
V -
W -
X - 'erreur de chargement dos'
Y -
Z - more scan lines per row (cursor is bigger)


****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "video/mc6845.h"
#include "machine/keyboard.h"
#include "imagedev/cassette.h"
#include "machine/wd_fdc.h"
#include "sound/wave.h"
#include "sound/beep.h"
#include "tavernie.lh"

#define KEYBOARD_TAG "keyboard"

class tavernie_state : public driver_device
{
public:
	tavernie_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_cass(*this, "cassette"),
		m_pia_ivg(*this, "pia_ivg"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_beep(*this, "beeper"),
		m_maincpu(*this, "maincpu"),
		m_acia(*this, "acia"),
		m_palette(*this, "palette")
	{
	}

	DECLARE_READ_LINE_MEMBER(ca1_r);
	DECLARE_READ8_MEMBER(pa_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_WRITE8_MEMBER(pa_ivg_w);
	DECLARE_READ8_MEMBER(pb_ivg_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE8_MEMBER(ds_w);
	DECLARE_MACHINE_RESET(cpu09);
	DECLARE_MACHINE_RESET(ivg09);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	MC6845_UPDATE_ROW(crtc_update_row);

	const UINT8 *m_p_chargen;
	optional_shared_ptr<UINT8> m_p_videoram;

private:
	UINT8 m_term_data;
	UINT8 m_pa;
	required_device<cassette_image_device> m_cass;
	optional_device<pia6821_device> m_pia_ivg;
	optional_device<fd1795_t> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<beep_device> m_beep;
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
public:
	optional_device<palette_device> m_palette;
};


static ADDRESS_MAP_START(cpu09_mem, AS_PROGRAM, 8, tavernie_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x1fff) AM_NOP
	AM_RANGE(0xeb00, 0xeb03) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xeb04, 0xeb04) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0xeb05, 0xeb05) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0xeb08, 0xeb0f) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xec00, 0xefff) AM_RAM // 1Kx8 RAM MK4118
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ivg09_mem, AS_PROGRAM, 8, tavernie_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x1fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x2003) AM_DEVREADWRITE("pia_ivg", pia6821_device, read, write)
	AM_RANGE(0x2080, 0x2080) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x2081, 0x2081) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("fdc", fd1795_t, read, write)
	AM_RANGE(0xe080, 0xe080) AM_WRITE(ds_w)
	AM_RANGE(0xeb00, 0xeb03) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xeb04, 0xeb04) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0xeb05, 0xeb05) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0xeb08, 0xeb0f) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xec00, 0xefff) AM_RAM // 1Kx8 RAM MK4118
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( cpu09 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "IRQ PTM") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "IRQ ACIA") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x60, 0x40, "Terminal") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "110 baud" )
	PORT_DIPSETTING(    0x20, "300 baud" )
	PORT_DIPSETTING(    0x40, "1200 baud" )
	PORT_DIPSETTING(    0x60, "IVG09 (mc6845)" )
INPUT_PORTS_END

static INPUT_PORTS_START( ivg09 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "IRQ PTM") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "IRQ ACIA") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x60, 0x60, "Terminal") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "110 baud" )
	PORT_DIPSETTING(    0x20, "300 baud" )
	PORT_DIPSETTING(    0x40, "1200 baud" )
	PORT_DIPSETTING(    0x60, "IVG09 (mc6845)" )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( tavernie_state, cpu09)
{
	m_term_data = 0;
}

MACHINE_RESET_MEMBER( tavernie_state, ivg09)
{
	m_p_chargen = memregion("chargen")->base();
	m_beep->set_state(1);
	m_term_data = 0;
	m_pia_ivg->cb1_w(1);
}

static SLOT_INTERFACE_START( ifd09_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

// can support 3 drives
WRITE8_MEMBER( tavernie_state::ds_w )
{
	floppy_image_device *floppy = nullptr;
	if ((data & 3) == 1) floppy = m_floppy0->get_device();
	//if ((data & 3) == 2) floppy = m_floppy1->get_device();
	//if ((data & 3) == 3) floppy = m_floppy2->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		m_fdc->dden_w(!BIT(data, 2));
	}
}


MC6845_UPDATE_ROW( tavernie_state::crtc_update_row )
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
		if (ra > 7)
			gfx = inv;  // some blank spacing lines
		else
		{
			chr = m_p_videoram[mem];
			gfx = m_p_chargen[(chr<<4) | ra] ^ inv;
		}

		/* Display a scanline of a character */
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


READ8_MEMBER( tavernie_state::pa_r )
{
	return ioport("DSW")->read() | m_pa;
}


/*
d0: A16
d1: A17 (for 256kb expansion)
d2, d3, d4: to 40-pin port
d5: S3
d6: S4
d7: cassout
*/
WRITE8_MEMBER( tavernie_state::pa_w )
{
	m_pa = data & 0x9f;
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
}

// centronics
WRITE8_MEMBER( tavernie_state::pb_w )
{
}

// cass in
READ_LINE_MEMBER( tavernie_state::ca1_r )
{
	return (m_cass->input() > +0.01);
}

READ8_MEMBER( tavernie_state::pb_ivg_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( tavernie_state::pa_ivg_w )
{
// bits 0-3 are attribute bits
}

WRITE8_MEMBER( tavernie_state::kbd_put )
{
	m_term_data = data;
	m_pia_ivg->cb1_w(0);
	m_pia_ivg->cb1_w(1);
}

WRITE_LINE_MEMBER( tavernie_state::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

static MACHINE_CONFIG_START( cpu09, tavernie_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(cpu09_mem)
	MCFG_MACHINE_RESET_OVERRIDE(tavernie_state, cpu09)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_CASSETTE_ADD( "cassette" )

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(tavernie_state, pa_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(tavernie_state, ca1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tavernie_state, pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(tavernie_state, pb_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)
	// all i/o lines connect to the 40-pin expansion connector
	MCFG_PTM6840_INTERNAL_CLOCK(XTAL_4MHz / 4)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT1_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_PTM6840_IRQ_CB(INPUTLINE("maincpu", M6809_IRQ_LINE))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(tavernie_state, write_acia_clock))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ivg09, cpu09 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ivg09_mem)
	MCFG_MACHINE_RESET_OVERRIDE(tavernie_state, ivg09)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 25*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*10-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_DEFAULT_LAYOUT(layout_tavernie)

	/* sound hardware */
	MCFG_SOUND_ADD("beeper", BEEP, 950) // guess
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(tavernie_state, kbd_put))

	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1008000) // unknown clock
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(tavernie_state, crtc_update_row)

	MCFG_DEVICE_ADD("pia_ivg", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(tavernie_state, pb_ivg_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(tavernie_state, pa_ivg_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("beeper", beep_device, set_state))

	MCFG_FD1795_ADD("fdc", XTAL_8MHz / 8)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ifd09_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cpu09 )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )
ROM_END

ROM_START( ivg09 )
	ROM_REGION( 0x19f0, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )
	// these 2 are not used. Boottav is copied to ram at c100
	ROM_LOAD_OPTIONAL( "promon.bin",   0x1000, 0x0800, CRC(43256bf2) SHA1(e81acb5b659d50d7b019b97ad5d2a8f129da39f6) )
	ROM_LOAD_OPTIONAL( "boottav.bin",  0x1800, 0x01f0, CRC(ae1a858d) SHA1(ab2144a00afd5b75c6dcb15c2c3f9d6910a159ae) )

	// charrom is missing, using one from 'c10' for now
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT    CLASS          INIT    COMPANY        FULLNAME   FLAGS */
COMP( 1982, cpu09,  0,      0,       cpu09,   cpu09,   driver_device,   0,   "C. Tavernier",  "CPU09", MACHINE_NOT_WORKING )
COMP( 1983, ivg09,  cpu09,  0,       ivg09,   ivg09,   driver_device,   0,   "C. Tavernier",  "CPU09 with IVG09 and IFD09", MACHINE_NOT_WORKING )
