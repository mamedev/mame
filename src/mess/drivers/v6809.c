// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Vegas 6809

        Skeleton driver

Devices:

MC6809 cpu
MC6840 timer
MM58174 RTC
MB8876 (or FD1791) FDC
SY6545-1 CRTC
2x MC6821 PIA
2x MC6850 ACIA

Memory ranges:

0000-EFFF RAM
F000-F7FF Devices
F800-FFFF ROM

Monitor commands:

D boot from floppy (launch Flex OS)
F relaunch Flex
G go
M modify memory (. to exit)

ToDo:

   - Colours (Looks like characters 0xc0-0xff produce coloured lores gfx).

   - Connect the RTC interrupt pin (not supported currently)

   - Find the missing character generator rom.

   - Enable sound, when what seems to be a 6840 bug is fixed.

   - Schematic is almost useless, riddled with omissions and errors.
     All documents are in French, so no help there. The parts list
     only has half of the parts.

   - Need software


****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/clock.h"
#include "video/mc6845.h"
#include "machine/6850acia.h"
#include "machine/mm58274c.h"
#include "machine/keyboard.h"
#include "sound/speaker.h"
#include "machine/wd_fdc.h"

#define KEYBOARD_TAG "keyboard"

class v6809_state : public driver_device
{
public:
	v6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_address(0),
		m_pia0(*this, "pia0"),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_speaker(*this, "speaker"),
		m_acia0(*this, "acia0"),
		m_acia1(*this, "acia1"),
		m_palette(*this, "palette")
	{
	}

	DECLARE_WRITE8_MEMBER(speaker_en_w);
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(v6809_address_w);
	DECLARE_WRITE8_MEMBER(v6809_register_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_MACHINE_RESET(v6809);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	UINT8 *m_p_videoram;
	const UINT8 *m_p_chargen;
	UINT16 m_video_address;

private:
	bool m_speaker_en;
	UINT8 m_video_index;
	UINT8 m_term_data;
	UINT8 m_vidbyte;
	required_device<pia6821_device> m_pia0;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<mb8876_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<speaker_sound_device> m_speaker;
	required_device<acia6850_device> m_acia0;
	required_device<acia6850_device> m_acia1;
public:
	required_device<palette_device> m_palette;
};


static ADDRESS_MAP_START(v6809_mem, AS_PROGRAM, 8, v6809_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_MIRROR(0xfe) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(v6809_address_w)
	AM_RANGE(0xf001, 0xf001) AM_MIRROR(0xfe) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(v6809_register_w)
	AM_RANGE(0xf200, 0xf200) AM_MIRROR(0xff) AM_WRITE(videoram_w)
	AM_RANGE(0xf504, 0xf504) AM_MIRROR(0x36) AM_DEVREADWRITE("acia0", acia6850_device, status_r, control_w) // modem
	AM_RANGE(0xf505, 0xf505) AM_MIRROR(0x36) AM_DEVREADWRITE("acia0", acia6850_device, data_r, data_w)
	AM_RANGE(0xf50c, 0xf50c) AM_MIRROR(0x36) AM_DEVREADWRITE("acia1", acia6850_device, status_r, control_w) // printer
	AM_RANGE(0xf50d, 0xf50d) AM_MIRROR(0x36) AM_DEVREADWRITE("acia1", acia6850_device, data_r, data_w)
	AM_RANGE(0xf600, 0xf603) AM_MIRROR(0x3c) AM_DEVREADWRITE("fdc", mb8876_t, read, write)
	AM_RANGE(0xf640, 0xf64f) AM_MIRROR(0x30) AM_DEVREADWRITE("rtc", mm58274c_device, read, write)
	AM_RANGE(0xf680, 0xf683) AM_MIRROR(0x3c) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0xf6c8, 0xf6cf) AM_MIRROR(0x08) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xf6d0, 0xf6d3) AM_MIRROR(0x0c) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(v6809_io, AS_PROGRAM, 8, v6809_state)
	AM_RANGE(0x0064, 0x0064) AM_WRITENOP
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( v6809 )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( v6809_state, v6809)
{
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("videoram")->base();
	m_term_data = 0;
	m_pia0->cb1_w(1);
}

// **** Video ****

/* F4 Character Displayer */
static const gfx_layout v6809_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( v6809 )
	GFXDECODE_ENTRY( "chargen", 0x0000, v6809_charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( v6809_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

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

MC6845_ON_UPDATE_ADDR_CHANGED( v6809_state::crtc_update_addr )
{
/* not sure what goes in here - parameters passed are device, address, strobe */
	m_video_address = address & 0x7ff;
}

WRITE8_MEMBER( v6809_state::videoram_w )
{
	m_vidbyte = data;
}

WRITE8_MEMBER( v6809_state::v6809_address_w )
{
	m_crtc->address_w( space, 0, data );

	m_video_index = data & 0x1f;

	if (m_video_index == 31)
		m_p_videoram[m_video_address] = m_vidbyte;
}

WRITE8_MEMBER( v6809_state::v6809_register_w )
{
	UINT16 temp = m_video_address;

	m_crtc->register_w( space, 0, data );

	// Get transparent address
	if (m_video_index == 18)
		m_video_address = ((data & 7) << 8 ) | (temp & 0xff);
	else
	if (m_video_index == 19)
		m_video_address = data | (temp & 0xff00);
}

// **** Keyboard ****

WRITE8_MEMBER( v6809_state::kbd_put )
{
	m_term_data = data;
	m_pia0->cb1_w(0);
	m_pia0->cb1_w(1);
}

WRITE_LINE_MEMBER( v6809_state::write_acia_clock )
{
	m_acia0->write_txc(state);
	m_acia0->write_rxc(state);
	m_acia1->write_txc(state);
	m_acia1->write_rxc(state);
}

READ8_MEMBER( v6809_state::pb_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// can support 4 drives
WRITE8_MEMBER( v6809_state::pa_w )
{
	floppy_image_device *floppy = NULL;
	if ((data & 3) == 0) floppy = m_floppy0->get_device();
	//if ((data & 3) == 1) floppy = m_floppy1->get_device();
	//if ((data & 3) == 2) floppy = m_floppy2->get_device();
	//if ((data & 3) == 3) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

// Bits 2 and 3 go to the floppy connector but are not documented

	if (floppy)
	{
		floppy->mon_w(0);
		m_fdc->dden_w(BIT(data, 4));
	}
}

// this should output 1 to enable sound, then output 0 after a short time
// however it continuously outputs 1
WRITE8_MEMBER( v6809_state::speaker_en_w )
{
	m_speaker_en = data;
}

WRITE8_MEMBER( v6809_state::speaker_w )
{
//  if (m_speaker_en)
//      m_speaker->level_w(data);
}

static SLOT_INTERFACE_START( v6809_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


// *** Machine ****

static MACHINE_CONFIG_START( v6809, v6809_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(v6809_mem)
	MCFG_CPU_IO_MAP(v6809_io)
	MCFG_MACHINE_RESET_OVERRIDE(v6809_state, v6809)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", sy6545_1_device, screen_update)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", v6809)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_MC6845_ADD("crtc", SY6545_1, "screen", XTAL_16MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(v6809_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(v6809_state, crtc_update_addr)

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(v6809_state, kbd_put))

// port A = drive select and 2 control lines ; port B = keyboard
// CB2 connects to the interrupt pin of the RTC (the rtc code doesn't support it)
	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(v6809_state, pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(v6809_state, pa_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))

// no idea what this does
	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(XTAL_16MHz / 4)
	MCFG_PTM6840_EXTERNAL_CLOCKS(4000000/14, 4000000/14, 4000000/14/8)
	MCFG_PTM6840_OUT1_CB(WRITE8(v6809_state, speaker_w))
	MCFG_PTM6840_OUT2_CB(WRITE8(v6809_state, speaker_en_w))
	MCFG_PTM6840_IRQ_CB(INPUTLINE("maincpu", M6809_IRQ_LINE))

	MCFG_DEVICE_ADD("acia0", ACIA6850, 0)

	MCFG_DEVICE_ADD("acia1", ACIA6850, 0)

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 10)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(v6809_state, write_acia_clock))

	MCFG_DEVICE_ADD("rtc", MM58274C, 0)
// this is all guess
	MCFG_MM58274C_MODE24(0) // 12 hour
	MCFG_MM58274C_DAY1(1)   // monday

	MCFG_MB8876_ADD("fdc", XTAL_16MHz / 16)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", v6809_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( v6809 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v6809.rom", 0xf800, 0x0800, CRC(54bf5f32) SHA1(10d1d70f0b51e2b90e5c29249d3eab4c6b0033a1) )

	ROM_REGION( 0x800, "videoram", ROMREGION_ERASE00 )

	/* character generator not dumped, using the one from 'h19' for now */
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, BAD_DUMP CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT     COMPANY      FULLNAME       FLAGS */
COMP( 1982, v6809,  0,      0,       v6809,     v6809, driver_device,   0,     "Microkit", "Vegas 6809", MACHINE_NOT_WORKING )
