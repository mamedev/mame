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

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "sound/wave.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "tavernie.lh"

class tavernie_state : public driver_device
{
public:
	tavernie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_cass(*this, "cassette")
		, m_pia_ivg(*this, "pia_ivg")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_beep(*this, "beeper")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
	{
	}

	void ivg09(machine_config &config);
	void cpu09(machine_config &config);

private:
	DECLARE_READ_LINE_MEMBER(ca1_r);
	DECLARE_READ8_MEMBER(pa_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_WRITE8_MEMBER(pa_ivg_w);
	DECLARE_READ8_MEMBER(pb_ivg_r);
	void kbd_put(u8 data);
	DECLARE_WRITE8_MEMBER(ds_w);
	DECLARE_MACHINE_RESET(cpu09);
	DECLARE_MACHINE_RESET(ivg09);
	MC6845_UPDATE_ROW(crtc_update_row);

	void cpu09_mem(address_map &map);
	void ivg09_mem(address_map &map);

	uint8_t m_term_data;
	uint8_t m_pa;
	optional_shared_ptr<uint8_t> m_p_videoram;
	required_device<cassette_image_device> m_cass;
	optional_device<pia6821_device> m_pia_ivg;
	optional_device<fd1795_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<beep_device> m_beep;
	required_device<cpu_device> m_maincpu;
	optional_device<palette_device> m_palette;
	optional_region_ptr<u8> m_p_chargen;
};


void tavernie_state::cpu09_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x1000, 0x1fff).noprw();
	map(0xeb00, 0xeb03).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xeb04, 0xeb05).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xeb08, 0xeb0f).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xec00, 0xefff).ram(); // 1Kx8 RAM MK4118
	map(0xf000, 0xffff).rom().region("roms", 0);
}

void tavernie_state::ivg09_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x1000, 0x1fff).ram().share("videoram");
	map(0x2000, 0x2003).rw(m_pia_ivg, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2080, 0x2080).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2081, 0x2081).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe000, 0xe003).rw(m_fdc, FUNC(fd1795_device::read), FUNC(fd1795_device::write));
	map(0xe080, 0xe080).w(FUNC(tavernie_state::ds_w));
	map(0xeb00, 0xeb03).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xeb04, 0xeb05).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xeb08, 0xeb0f).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xec00, 0xefff).ram(); // 1Kx8 RAM MK4118
	map(0xf000, 0xffff).rom().region("roms", 0);
}


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
	m_beep->set_state(1);
	m_term_data = 0;
	m_pia_ivg->cb1_w(1);
}

static void ifd09_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

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
	uint8_t chr,gfx=0;
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		uint8_t inv=0;
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
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( tavernie_state::pa_ivg_w )
{
// bits 0-3 are attribute bits
}

void tavernie_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_pia_ivg->cb1_w(0);
	m_pia_ivg->cb1_w(1);
}

void tavernie_state::cpu09(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tavernie_state::cpu09_mem);
	MCFG_MACHINE_RESET_OVERRIDE(tavernie_state, cpu09)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.05);

	/* Devices */
	CASSETTE(config, m_cass);

	pia6821_device &pia(PIA6821(config, "pia", 0));
	pia.readpa_handler().set(FUNC(tavernie_state::pa_r));
	pia.readca1_handler().set(FUNC(tavernie_state::ca1_r));
	pia.writepa_handler().set(FUNC(tavernie_state::pa_w));
	pia.writepb_handler().set(FUNC(tavernie_state::pb_w));
	pia.irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	pia.irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	ptm6840_device &ptm(PTM6840(config, "ptm", 4_MHz_XTAL / 4));
	// all i/o lines connect to the 40-pin expansion connector
	ptm.set_external_clocks(0, 0, 0);
	ptm.o2_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	ptm.irq_callback().set_inputline("maincpu", M6809_IRQ_LINE);

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 153600));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));
}

void tavernie_state::ivg09(machine_config &config)
{
	cpu09(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &tavernie_state::ivg09_mem);
	MCFG_MACHINE_RESET_OVERRIDE(tavernie_state, ivg09)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(80*8, 25*10);
	screen.set_visarea(0, 80*8-1, 0, 25*10-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	config.set_default_layout(layout_tavernie);

	/* sound hardware */
	BEEP(config, m_beep, 950).add_route(ALL_OUTPUTS, "mono", 0.50); // guess

	/* Devices */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(tavernie_state::kbd_put));

	mc6845_device &crtc(MC6845(config, "crtc", 1008000)); // unknown clock
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(tavernie_state::crtc_update_row), this);

	PIA6821(config, m_pia_ivg, 0);
	m_pia_ivg->readpb_handler().set(FUNC(tavernie_state::pb_ivg_r));
	m_pia_ivg->writepa_handler().set(FUNC(tavernie_state::pa_ivg_w));
	m_pia_ivg->cb2_handler().set(m_beep, FUNC(beep_device::set_state));

	FD1795(config, m_fdc, 8_MHz_XTAL / 8);
	FLOPPY_CONNECTOR(config, "fdc:0", ifd09_floppies, "525dd", floppy_image_device::default_floppy_formats).enable_sound(true);
}

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

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT        COMPANY         FULLNAME                      FLAGS
COMP( 1982, cpu09, 0,      0,      cpu09,   cpu09, tavernie_state, empty_init, "C. Tavernier", "CPU09",                      MACHINE_NOT_WORKING )
COMP( 1983, ivg09, cpu09,  0,      ivg09,   ivg09, tavernie_state, empty_init, "C. Tavernier", "CPU09 with IVG09 and IFD09", MACHINE_NOT_WORKING )
