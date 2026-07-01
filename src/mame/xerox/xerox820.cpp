// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

        Xerox 820

        12/05/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - Xerox 820
        - floppy format has 3xcd at the end of track data
            :u109: write track 0
            :u109: track description 16xff ... 109xff 3xcd
        - keyboard conflicts with optional serial terminal
    - Xerox 820-II
        - floppy (read/write to FDC triggers Z80 WAIT)
        - Winchester
            - Shugart SA1004 (chs=256,4,40 ss=256)
            - Shugart SA606 (chs=160,6, ss=256)
            - Shugart SA1403D controller
    - Xerox 16/8
    - Emerald Microware X120 board
    - type in Monitor v1.0 from manual
    - ASCII keyboard
    - low-profile keyboard

    http://www.vintagesbc.it/?page_id=233
    http://mccworkshop.com/computers/comphistory7.htm
    http://bitsavers.org/bits/Xerox/820/
    http://bitsavers.org/bits/Xerox/820-II/
    http://www.classiccmp.org/dunfield/img54306/system.htm

    Note:
    - MK-82 have same roms as original Big Board
    - MK-83 have 256K of RAM

    8-inch formats
    77 tracks, 1 head, 26 sectors, 128 bytes sector length, first sector id 1
    77 tracks, 1 head, 26 sectors, 256 bytes sector length, first sector id 1

    5.25-inch formats
    40 tracks, 1 head, 18 sectors, 128 bytes sector length, first sector id 1
    40 tracks, 2 heads, 18 sectors, 128 bytes sector length, first sector id 1

    SmartROM and Plus2 ROM both come for 2.5MHz or 4MHz systems, and there is another distinction between variants for generic or Xerox keyboards
    http://www.microcodeconsulting.com/z80/plus2.htm
    http://www.microcodeconsulting.com/z80/smartrom.htm

*/


#include "emu.h"
#include "xerox820.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/flopimg.h"
#include "formats/imd_dsk.h"


/* Read/Write Handlers */

uint8_t xerox820_state::fdc_r(offs_t offset)
{
	// The first-gen 820's FD1771 data bus is inverted (m_fdc_xor = 0xff); the 820-II's
	// FD1797 is not (m_fdc_xor = 0x00).  With the 0xff inversion the WD status bit 7
	// (not-ready) reads back inverted, so the boot ROM's media-detect (selunt, F65D
	// jp m,F5F6) misreads a ready drive as not-ready and gives up.
	return m_fdc->read(offset) ^ m_fdc_xor;
}

void xerox820_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ m_fdc_xor);
}

void xerox820_base_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scroll = (offset >> 8) & 0x1f;
}

#ifdef UNUSED_CODE
void xerox820_state::x120_system_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       DSEL0       drive select bit 0 (01=A, 10=B, 00=C, 11=D)
	    1       DSEL1       drive select bit 1
	    2       SIDE        side select
	    3       VATT        video attribute (0=inverse, 1=blinking)
	    4       BELL        bell trigger
	    5       DENSITY     density (0=double, 1=single)
	    6       _MOTOR      disk motor (0=on, 1=off)
	    7       BANK        memory bank switch (0=RAM, 1=ROM/video)

	*/
}
#endif

void xerox820ii_state::bell_w(offs_t offset, uint8_t data)
{
	m_speaker->level_w(offset);
}

void xerox820ii_state::slden_w(offs_t offset, uint8_t data)
{
	// Port 0x30 = Select Single Density (FM), port 0x31 = Select Double Density (MFM)
	// (Technical Reference, I/O Port Assignments).  wd_fdc dden_w() is active for FM,
	// so the offset must be inverted: 0x30 -> FM (dden=1), 0x31 -> MFM (dden=0).
	m_dbslot->density_w(!offset);
}

void xerox820ii_state::chrom_w(offs_t offset, uint8_t data)
{
	m_chrom = offset;
}

void xerox820ii_state::lowlite_w(uint8_t data)
{
	m_lowlite = data;
}

void xerox820ii_state::sync_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		/* set external clocks for synchronous sio A */
	}
	else
	{
		/* set internal clocks for asynchronous sio A */
	}
}

/* Memory Maps */

void xerox820_state::xerox820_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).view(m_view);
	m_view[0](0x0000, 0x3fff).ram();
	m_view[1](0x0000, 0x0fff).rom().region(Z80_TAG, 0);
	m_view[1](0x3000, 0x3fff).ram().share("video_ram");
	map(0x4000, 0xffff).ram();
}

// The I/O ports common to every machine; the disk controller at 0x10-0x13 is
// added by the per-personality maps below (FD1771 on the 820, the daughterboard
// slot on the 820-II).
void xerox820_base_state::io_common(address_map &map)
{
	map(0x00, 0x00).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::str_w));
	map(0x04, 0x07).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0b).mirror(0xff00).rw(Z80PIO_GP_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0c, 0x0c).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::stt_w));
	map(0x14, 0x14).mirror(0x0003).select(0xff00).w(FUNC(xerox820ii_state::scroll_w));
	map(0x18, 0x1b).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).mirror(0xff00).rw(m_kbpio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

void xerox820_state::xerox820_io(address_map &map)
{
	io_common(map);
	map(0x10, 0x13).mirror(0xff00).rw(FUNC(xerox820_state::fdc_r), FUNC(xerox820_state::fdc_w));
}

void xerox820ii_state::xerox820ii_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).view(m_view);
	m_view[0](0x0000, 0xbfff).ram();
	m_view[1](0x0000, 0x1fff).rom().region(Z80_TAG, 0);
	m_view[1](0x3000, 0x3fff).ram().share("video_ram");
	// 16/8: in the banked-ROM view the 0x4000-0xBFFF window reaches the 8086's resident
	// RAM (Z80 0x4000 -> 8086 0xF8000, i.e. 8086 addr = Z80 offset + 0xF4000), so the
	// monitor's F033 block-move can read/write the 8086 mailbox (0x4600) + ROM signature
	// (0x8FF5).  The coprocessor slot models the window + the bus arbiter; an empty slot
	// (plain 820-II) reads 0xFF.
	m_view[1](0x4000, 0xbfff).rw(m_copro, FUNC(xerox820_copro_slot_device::shared_ram_r), FUNC(xerox820_copro_slot_device::shared_ram_w));
	map(0xc000, 0xffff).ram();
}

// 820-II: the disk controller (FD1797 floppy card or SASI host adapter) lives in
// the daughterboard slot at 0x10-0x13; the slot abstracts the two personalities.
// The 16/8 coprocessor card installs its A0/A1 control ports into this space itself.
void xerox820ii_state::xerox820ii_io(address_map &map)
{
	io_common(map);
	map(0x10, 0x13).mirror(0xff00).rw(m_dbslot, FUNC(xerox820_dbslot_device::io_r), FUNC(xerox820_dbslot_device::io_w));
	map(0x28, 0x29).mirror(0xff00).w(FUNC(xerox820ii_state::bell_w));
	map(0x30, 0x31).mirror(0xff00).w(FUNC(xerox820ii_state::slden_w));
	map(0x34, 0x35).mirror(0xff00).w(FUNC(xerox820ii_state::chrom_w));
	map(0x36, 0x36).mirror(0xff00).w(FUNC(xerox820ii_state::lowlite_w));
	map(0x68, 0x69).mirror(0xff00).w(FUNC(xerox820ii_state::sync_w));
}

// The 16/8 5.25" disk subsystems (floppy-only and rigid+floppy) are both the
// EM-II (DEM): a WD1002-05 + the genuine 537p3682 box ROM, modelled as the
// xerox820_emii copro card (bus/xerox820/copro.cpp).  The earlier x1685/x1685s
// reconstructions (rx024/sdvr port-ROMs) are superseded and have been removed.

void xerox820_state::mk83_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x2fff).view(m_view);
	m_view[0](0x0000, 0x2fff).ram();
	m_view[1](0x0000, 0x0fff).rom().region(Z80_TAG, 0);
	map(0x3000, 0x6fff).ram();
	map(0x7000, 0x7fff).ram().share("video_ram");
	map(0x8000, 0xffff).ram();
}


/* Input Ports */

static INPUT_PORTS_START( xerox820 )
	// inputs defined in machine/keyboard.c
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(bigboard_state::beep_timer)
{
	m_beeper->set_state(0);
}

/* Z80 PIO */

uint8_t xerox820_base_state::kbpio_pa_r()
{
	/*

	    bit     signal          description

	    0       (disk daughterboard personality, 820-II)
	    1       (disk daughterboard personality / A-E swap, 820-II)
	    2
	    3       PBRDY           keyboard data available
	    4       8/N5            8"/5.25" disk select (0=5.25", 1=8")
	    5       400/460         double sided disk detect (only on Etch 2 PCB) (0=SS, 1=DS)
	    6
	    7

	*/

	uint8_t data = 0;

	// keyboard
	data |= m_kbpio->rdy_b() << 3;

	// disk daughterboard personality (PA0-1) + media sense (PA4-5)
	data |= disk_pa_bits();

	return data;
}

void xerox820_base_state::kbpio_pa_w(uint8_t data)
{
	/*

	    bit     signal          description

	    0       _DVSEL1         drive select 1
	    1       _DVSEL2         drive select 2
	    2       SIDE            side select
	    3
	    4
	    5
	    6       NCSET2          display character set (inverted and connected to chargen A10)
	    7       BANK            bank switching (0=RAM, 1=ROM/videoram)

	*/

	// drive select / side -> the disk controller (FD1771 main board or the
	// 820-II daughterboard slot)
	disk_drvsel_w(data);

	/* display character set */
	m_ncset2 = !BIT(data, 6);

	/* bank switching */
	m_view.select(BIT(data, 7));
}

// The 820 / Big Board main-board FD1771 floppy select.
void xerox820_state::disk_drvsel_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	if (m_drvsel_binary)
	{
		// Big Board: the low 2 bits are a binary drive-unit number (drive 0 = 00),
		// demuxed to one select line.  The Xerox 820 instead uses one bit per drive.
		switch (data & 0x03)
		{
		case 0: floppy = m_floppy0->get_device(); break;
		case 1: floppy = m_floppy1->get_device(); break;
		}
	}
	else
	{
		if (BIT(data, 0)) floppy = m_floppy0->get_device();
		if (BIT(data, 1)) floppy = m_floppy1->get_device();
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		int _8n5 = (floppy->get_form_factor() == floppy_image::FF_8);

		if (m_8n5 != _8n5)
		{
			m_8n5 = _8n5;

			m_fdc->set_unscaled_clock(m_8n5 ? 20_MHz_XTAL / 10 : 20_MHz_XTAL / 20);
		}

		m_400_460 = !floppy->twosid_r();

		floppy->mon_w(0);

		floppy->ss_w(BIT(data, 2));
	}
}

// The 820-II disk daughterboard reports its personality on PA bits 0-1, which the
// ROSR 'signon' routine reads (port 0x1C):
//   bit 0 = 1: main-board FD1797 floppy controller (WD1797 driver path)
//           0: Shugart SASI host adapter (installs the SA1403D driver)
//   bit 1 = A/E swap gate (signon FC95 `and 2; jr nz`): 1 = no swap, so drive A is
//           the floppy (boot floppy-first); 0 = swap A<->E (A is the rigid disk).
// The slot's card reports the personality bits; the media sense (PA4-5) follows
// the connected drive.
uint8_t xerox820ii_state::disk_pa_bits()
{
	return m_dbslot->personality()
		| (m_dbslot->media_8inch() << 4)
		| (m_dbslot->media_twosided() << 5);
}

void bigboard_state::kbpio_pa_w(uint8_t data)
{
	xerox820_base_state::kbpio_pa_w(data);

	/* beeper on bigboard */
	if (BIT(data, 5) & (!m_bit5))
	{
		m_beep_timer->adjust(attotime::from_msec(40));
		m_beeper->set_state(1);
	}
	m_bit5 = BIT(data, 5);
}

uint8_t xerox820_base_state::kbpio_pb_r()
{
	/*

	    bit     description

	    0       KB0
	    1       KB1
	    2       KB2
	    3       KB3
	    4       KB4
	    5       KB5
	    6       KB6
	    7       KB7

	*/

	return m_kbd->read() ^ 0xff;
}

/* Z80 CTC */

TIMER_DEVICE_CALLBACK_MEMBER( xerox820_base_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
}

/* Z80 Daisy Chain */

static const z80_daisy_config xerox820_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};

// The disk daughterboard slot sits in the IM2 daisy chain: the SASI host
// adapter's "u8" Z80PIO drives IRQ0 and must be vectored here (otherwise its
// interrupts mis-vector and starve the keyboard PIO); the FD1797 floppy card
// has no daisy presence (it interrupts the Z80 via /NMI), so the slot is
// transparent in that personality.
static const z80_daisy_config xerox820ii_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ DBSLOT_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};



/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(xerox820_base_state::quickload_cb)
{
	m_view.select(0);

	address_space &prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");
	}

	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	uint16_t quickload_size = image.length();
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	// Roughly set SP basing on the BDOS position
	m_maincpu->set_state_int(Z80_SP, mem_avail + 384);
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}



/* WD1771 Interface */

static void xerox820_floppies(device_slot_interface &device)
{
	device.option_add("sa400", FLOPPY_525_SSSD_35T); // Shugart SA-400, 35 trk drive
	device.option_add("sa400l", FLOPPY_525_SSSD); // Shugart SA-400, 40 trk drive
	device.option_add("sa450", FLOPPY_525_DD); // Shugart SA-450
	device.option_add("sa800", FLOPPY_8_SSDD); // Shugart SA-800
	device.option_add("sa850", FLOPPY_8_DSDD); // Shugart SA-850
}

// Big Board 8" CP/M disks ship as IMD images.
static void bigboard_floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_IMD_FORMAT);
}

// FDC INTRQ and DRQ are OR'd and gated by the Z80 /HALT line to drive /NMI.
// The BIOS issues an FDC command, executes HALT, and each DRQ (and the final
// INTRQ) fires an NMI that moves one byte / completes the command.  The gate
// must be evaluated on BOTH of its inputs -- the FDC DRQ/INTRQ edges and the
// Z80 /HALT edge -- otherwise the case where the next DRQ asserts just before
// the CPU reaches HALT is lost and the CPU hangs in HALT forever.  m_cpu_halted
// is tracked via the Z80 halt_cb so /NMI tracks (/HALT & (DRQ|INTRQ)).
void xerox820_base_state::update_nmi()
{
	int state = (m_cpu_halted && (m_fdc_irq || m_fdc_drq)) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

void xerox820_base_state::cpu_halt_w(int state)
{
	m_cpu_halted = bool(state);

	update_nmi();
}

void xerox820_base_state::fdc_intrq_w(int state)
{
	m_fdc_irq = state;

	update_nmi();
}

void xerox820_base_state::fdc_drq_w(int state)
{
	m_fdc_drq = state;

	update_nmi();
}

/* Video */

uint32_t xerox820_base_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=(m_scroll + 1) * 0x80;
	pen_t const *const pen=m_palette->pens();

	m_framecnt++;

	for (uint8_t y = 0; y < 24; y++)
	{
		if (ma > 0xb80) ma = 0;

		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint32_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 80; x++)
			{
				uint8_t gfx;
				if (ra < 8)
				{
					uint8_t chr = m_video_ram[x & XEROX820_VIDEORAM_MASK] ^ 0x80;

					/* Take care of flashing characters */
					if ((chr < 0x80) && (m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = m_char_rom->base()[(m_ncset2 << 10) | (chr<<3) | ra ];
				}
				else
					gfx = 0xff;

				/* Display a scanline of a character (7 pixels) */
				*p++ = pen[0];
				*p++ = pen[BIT(gfx, 4) ^ 1];
				*p++ = pen[BIT(gfx, 3) ^ 1];
				*p++ = pen[BIT(gfx, 2) ^ 1];
				*p++ = pen[BIT(gfx, 1) ^ 1];
				*p++ = pen[BIT(gfx, 0) ^ 1];
				*p++ = pen[0];
			}
		}
		ma+=128;
	}
	return 0;
}

/* Machine Initialization */

void xerox820_base_state::machine_start()
{
	// state saving
	save_item(NAME(m_scroll));
	save_item(NAME(m_ncset2));
	save_item(NAME(m_vatt));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_cpu_halted));

	m_ncset2 = 0;
}

void xerox820_state::machine_start()
{
	xerox820_base_state::machine_start();

	save_item(NAME(m_8n5));
	save_item(NAME(m_400_460));
}

void xerox820_state::machine_reset()
{
	m_view.select(1);

	m_fdc->reset();
}

void bigboard_state::machine_reset()
{
	m_view.select(1);

	/* bigboard has a one-pulse output to drive a user-supplied beeper */
	m_beeper->set_state(0);

	m_fdc->reset();

	// The Big Board's monitor selects drives by a binary unit number in the low
	// bits of the system PIO port (drive 0 = 0), not the Xerox 820's bit-per-drive.
	m_drvsel_binary = true;

	// 8" Shugart drives spin continuously (no motor-control line); force the
	// motor on so the WD1771 sees READY for the boot ROM's RESTORE.
	if (floppy_image_device *fd = m_floppy0->get_device()) fd->mon_w(0);
	if (floppy_image_device *fd = m_floppy1->get_device()) fd->mon_w(0);
}

void xerox820ii_state::machine_start()
{
	xerox820_base_state::machine_start();
}

void xerox820ii_state::machine_reset()
{
	m_view.select(1);

	// The disk daughterboard (FD1797 or SASI) and the coprocessor card each reset
	// themselves: the FD1797 card primes its data rate from the connected drive's
	// form factor, and the 16/8 board holds the 8086 in reset until the Z80 starts it.

	m_sio->synca_w(1);
	m_sio->syncb_w(1);
}


/* F4 Character Displayer */
static const gfx_layout xerox820_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout xerox820_gfxlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_xerox820 )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_xerox820ii )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, xerox820_gfxlayout, 0, 1 )
GFXDECODE_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/* Machine Drivers */

void xerox820_state::xerox820(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 20_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820_state::xerox820_mem);
	m_maincpu->set_addrmap(AS_IO, &xerox820_state::xerox820_io);
	m_maincpu->set_daisy_config(xerox820_daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(xerox820_state::screen_update));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* devices */
	Z80PIO(config, m_kbpio, 20_MHz_XTAL / 8);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(xerox820_state::kbpio_pa_r));
	m_kbpio->out_pa_callback().set(FUNC(xerox820_state::kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(xerox820_state::kbpio_pb_r));

	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 20_MHz_XTAL / 8));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, 20_MHz_XTAL / 8);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	//TIMER(config, "ctc").configure_periodic(FUNC(xerox820_state::ctc_tick), attotime::from_hz(20_MHz_XTAL / 8));

	m_maincpu->halt_cb().set(FUNC(xerox820_state::cpu_halt_w)); // FDC DRQ/INTRQ -> /NMI is /HALT-gated

	FD1771(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(FUNC(xerox820_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xerox820_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, FD1771_TAG":0", xerox820_floppies, "sa400l", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1771_TAG":1", xerox820_floppies, "sa400l", floppy_image_device::default_mfm_floppy_formats);

	Z80SIO(config, m_sio, 20_MHz_XTAL / 8); // MK3884 (SIO/0)
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.rxd_handler().append(m_sio, FUNC(z80sio_device::synca_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.rxd_handler().append(m_sio, FUNC(z80sio_device::syncb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	rs232b.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	XEROX_820_KEYBOARD(config, m_kbd);
	m_kbd->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("xerox820");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(xerox820_state::quickload_cb));
}

void bigboard_state::bigboard(machine_config &config)
{
	xerox820(config);

	// The Big Board uses 8" SSSD drives (Shugart SA-800) read by the WD1771 in FM
	// at the 8" 2 MHz clock, not the Xerox 820's 5.25" SA-400 at 1 MHz; its disks
	// ship as IMD.
	m_fdc->set_clock(20_MHz_XTAL / 10);
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":0", xerox820_floppies, "sa800", bigboard_floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":1", xerox820_floppies, "sa800", bigboard_floppy_formats);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 950).add_route(ALL_OUTPUTS, "mono", 1.00); /* bigboard only */
	TIMER(config, m_beep_timer).configure_generic(FUNC(bigboard_state::beep_timer));
}

void bigboard_state::bigboard5(machine_config &config)
{
	xerox820(config);

	// A common field modification ran the Big Board from 5.25" SA-400 drives in
	// place of the stock 8" SA-800.  The WD1771 (FM/single density) stays at its
	// 5.25" 1 MHz clock (the Xerox 820 default), and the monitor auto-selects the
	// 5.25" disk parameters from the drive's form factor (kbpio_pa_r bit 4).
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":0", xerox820_floppies, "sa400l", bigboard_floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":1", xerox820_floppies, "sa400l", bigboard_floppy_formats);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 950).add_route(ALL_OUTPUTS, "mono", 1.00); /* bigboard only */
	TIMER(config, m_beep_timer).configure_generic(FUNC(bigboard_state::beep_timer));
}

// Shared 820-II hardware plus the disk-controller daughterboard slot.  disk_card
// selects the personality: the FD1797 floppy controller ("fdc"/"fdc5") or the
// Shugart SASI host adapter ("sasi"), with the 16/8 expansion-box variant
// ("fdcbox5").
void xerox820ii_state::common(machine_config &config, const char *disk_card)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820ii_state::xerox820ii_mem);
	m_maincpu->set_addrmap(AS_IO, &xerox820ii_state::xerox820ii_io);
	m_maincpu->set_daisy_config(xerox820ii_daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(xerox820ii_state::screen_update));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820ii);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	Z80PIO(config, m_kbpio, 16_MHz_XTAL / 4);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(xerox820ii_state::kbpio_pa_r));
	m_kbpio->out_pa_callback().set(FUNC(xerox820ii_state::kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(xerox820ii_state::kbpio_pb_r));

	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 16_MHz_XTAL / 4));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	m_maincpu->halt_cb().set(FUNC(xerox820ii_state::cpu_halt_w)); // FDC DRQ/INTRQ -> /NMI is /HALT-gated

	// disk-controller daughterboard slot.  The FD1797 floppy card's INTRQ/DRQ
	// feed the /HALT-gated /NMI; the SASI host adapter's u8 PIO drives IRQ0 (and
	// joins the IM2 daisy chain).
	XEROX820_DBSLOT(config, m_dbslot, xerox820_dbslot_cards, disk_card);
	m_dbslot->intrq_wr_callback().set(FUNC(xerox820ii_state::fdc_intrq_w));
	m_dbslot->drq_wr_callback().set(FUNC(xerox820ii_state::fdc_drq_w));
	m_dbslot->int_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// coprocessor slot: empty (plain 820-II) or the 16/8 8086 board, which installs
	// its shared-RAM window + A0/A1 control ports into the Z80 spaces when it starts
	XEROX820_COPRO_SLOT(config, m_copro, xerox820_copro_cards, nullptr);
	m_copro->set_maincpu(m_maincpu);

	Z80SIO(config, m_sio, 16_MHz_XTAL / 4); // MK3884 (SIO/0)
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	// the keyboard (standard 820-II or the LP keyboard) is added by the machine
	// configs below, per the fitted configuration

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("xerox820ii");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(xerox820ii_state::quickload_cb));
}

// the standard 820-II ASCII keyboard
void xerox820ii_state::add_820ii_kbd(machine_config &config)
{
	XEROX_820II_KEYBOARD(config, m_kbd);
	m_kbd->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));
}

// the position-encoded Low Profile Keyboard (the "RX" monitor's keyboard; fitted
// to the 820-II LP and every 16/8)
void xerox820ii_state::add_lpk(machine_config &config)
{
	XEROX_LPK(config, m_kbd);
	m_kbd->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));
}

// 820-II, FD1797 floppy controller, 8" drives.
void xerox820ii_state::xerox820ii(machine_config &config)
{
	common(config, "fdc");
	add_820ii_kbd(config);
}

// 820-II, FD1797 floppy controller, 5.25" drives.
void xerox820ii_state::xerox820ii5(machine_config &config)
{
	common(config, "fdc5");
	add_820ii_kbd(config);
}

// 820-II, Shugart SASI host adapter: 8" floppies + ST-506 rigid disk on the SA1403D.
void xerox820ii_state::xerox820iis(machine_config &config)
{
	common(config, "sasi");
	add_820ii_kbd(config);
}

// 820-II with the Low Profile Keyboard (for the LPK-family u36 ROMs, e.g. v016/v018)
void xerox820ii_state::xerox820iilp(machine_config &config)
{
	common(config, "fdc");
	add_lpk(config);
}

// The 16/8 option board: plug the 8086 coprocessor card into the slot.  The card
// installs the shared-RAM window + A0/A1 control ports into the Z80 spaces itself.
void xerox820ii_state::add_8086(machine_config &config)
{
	m_copro->set_default_option("16_8");
}

// the 16/8 machines: 820-II hardware + the 8086 coprocessor card + the LP keyboard
void xerox820ii_state::xerox168(machine_config &config)  { common(config, "fdc");    add_8086(config); add_lpk(config); } // 16/8, 8" floppy
void xerox820ii_state::xerox1685(machine_config &config) { common(config, "fdc5");   add_8086(config); add_lpk(config); } // 16/8, 5.25" floppy (FD1797 daughterboard; mixed SD/DD)
void xerox820ii_state::xerox168s(machine_config &config) { common(config, "sasi");   add_8086(config); add_lpk(config); } // 16/8, SASI
void xerox820ii_state::xerox168em(machine_config &config) // 16/8 + Expansion Module II (EM-II / WD1002-05)
{
	// The EM-II is a slot-connected expansion: the copro/8086-slot card owns
	// everything (8086 + the WD1002-05 disk at $A8-$AF + the 537p3682 box ROM),
	// installing its own ports.  No disk daughterboard.
	common(config, nullptr);
	m_copro->set_default_option("emii_rgd5");
	add_lpk(config);
}

void xerox820_state::mk83(machine_config & config)
{
	xerox820(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820_state::mk83_mem);
}

/* ROMs */

ROM_START( bigboard )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "bigboard.u67", 0x0000, 0x0800, CRC(5a85a228) SHA1(d51a2cbd0aae80315bda9530275aabfe8305364e) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "bigboard.u73", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

#define rom_mk82 rom_bigboard
#define rom_bigboard5 rom_bigboard

ROM_START( x820 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v20" )
	ROM_SYSTEM_BIOS( 0, "v10", "Xerox Monitor v1.0" )
	ROMX_LOAD( "x820v10.u64", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "x820v10.u63", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v20", "Xerox Monitor v2.0" )
	ROMX_LOAD( "x820v20.u64", 0x0000, 0x0800, CRC(2fc227e2) SHA1(b4ea0ae23d281a687956e8a514cb364a1372678e), ROM_BIOS(1) )
	ROMX_LOAD( "x820v20.u63", 0x0800, 0x0800, CRC(bc11f834) SHA1(4fd2b209a6e6ff9b0c41800eb5228c34a0d7f7ef), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "smart23", "MICROCode SmartROM v2.3" )
	ROMX_LOAD( "mxkx25a.u64", 0x0000, 0x0800, CRC(7ec5f100) SHA1(5d0ff35a51aa18afc0d9c20ef99ff5d9d3f2075b), ROM_BIOS(2) )
	ROMX_LOAD( "mxkx25b.u63", 0x0800, 0x0800, CRC(a7543798) SHA1(886e617e1003d13f86f33085cbd49391b77291a3), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "plus2", "MICROCode Plus2 v0.2a" )
	ROMX_LOAD( "p2x25a.u64",  0x0000, 0x0800, CRC(3ccd7a8f) SHA1(6e46c88f03fc7289595dd6bec95e23bb13969525), ROM_BIOS(3) )
	ROMX_LOAD( "p2x25b.u63",  0x0800, 0x0800, CRC(1e580391) SHA1(e91f8ce82586df33c0d6d02eb005e8079f4de67d), ROM_BIOS(3) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "x820.u92", 0x0000, 0x0800, CRC(b823fa98) SHA1(ad0ea346aa257a53ad5701f4201896a2b3a0f928) )
ROM_END

// The base 820-II boot ROM set, shared by the FD1797 floppy (5.25"/8") and the SASI
// disk-board variants (the ROM auto-detects the daughterboard via the PA bits).
#define ROM_X820II_CONTENTS \
	ROM_REGION( 0x2000, Z80_TAG, 0 ) \
	ROM_DEFAULT_BIOS( "v50" ) /* the standard keyboard and the in-ROM SASI driver both live in u33-u35 (the missing v500.u36 is not needed for floppy or SASI operation).  The v50v018 "Typewriter" build gates keyboard input differently (no echo from the standard keyboard). */ \
	/* v4.00/v4.01 u33-u35 and v4.04 u36 are not chip reads: they are ROM images recovered from Balcones/Xerox distribution master disks in the Don Maslin 820-II archive (bitsavers 820ii_images) and functionally validated by booting.  Images + recovered source: https://github.com/davidlrand/mame-system-media (Xerox-820-line-roms) */ \
	ROM_SYSTEM_BIOS( 0, "v400", "Balcones Operating System v4.00" ) /* u33-u35 split from ROM400.COM, master disk B16D35 */ \
	ROMX_LOAD( "v400.u33", 0x0000, 0x0800, CRC(0d1bcaa8) SHA1(a6ac83f8584d19f7a08e666cb5d4b62620d7d3c0), ROM_BIOS(0) ) \
	ROMX_LOAD( "v400.u34", 0x0800, 0x0800, CRC(f1df9e29) SHA1(79f38880c3aed9ddf2ccba4ddb11128586dc9c25), ROM_BIOS(0) ) \
	ROMX_LOAD( "v400.u35", 0x1000, 0x0800, CRC(68357ed7) SHA1(78498542f4d8506edf5f2b3b9ed0fde3fd72f85b), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "v401", "Balcones Operating System v4.01" ) /* u33-u35 decoded from U3x.HEX, "820-II ROM IMAGES MASTER" disk B17D7 */ \
	ROMX_LOAD( "v401.u33", 0x0000, 0x0800, CRC(fe9fa596) SHA1(194162b3d063b2d1bcad03d0bee51dabce2d1985), ROM_BIOS(1) ) \
	ROMX_LOAD( "v401.u34", 0x0800, 0x0800, CRC(d3137de3) SHA1(d1de4e11f29799b2024af0412415a07984e58f3a), ROM_BIOS(1) ) \
	ROMX_LOAD( "v401.u35", 0x1000, 0x0800, CRC(bf3096fb) SHA1(136f6b1cf2cd93f0b908688394675ef69883f47b), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "v402", "Balcones Operating System v4.02" ) \
	ROMX_LOAD( "u33.4.02.rom", 0x0000, 0x0800, CRC(d9eb668e) SHA1(6acbef96e4e6526c58e068b7849fb9cce2ea2a10), ROM_BIOS(2) ) \
	ROMX_LOAD( "u34.4.02.rom", 0x0800, 0x0800, CRC(62181209) SHA1(2238aec096d19af9307bb294532f66f53dd7dfc3), ROM_BIOS(2) ) \
	ROMX_LOAD( "u35.4.02.rom", 0x1000, 0x0800, CRC(e22fbf6d) SHA1(6c162f79d42611176b0f1c0e8a4eeb07492beca1), ROM_BIOS(2) ) \
	ROMX_LOAD( "u36.rx11.4.02.rom", 0x1800, 0x0800, CRC(b6a239ce) SHA1(330d28fa8ec006d48d948b1c5e714ffced88fe90), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "v403", "Balcones Operating System v4.03" ) \
	ROMX_LOAD( "v403.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROMX_LOAD( "v403.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROMX_LOAD( "v403.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROMX_LOAD( "v403.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "v404", "Balcones Operating System v4.04" ) \
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(4) ) \
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(4) ) \
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(4) ) \
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, CRC(97047d38) SHA1(f36506635653736b8d754d2c04f608180602b5a2), ROM_BIOS(4) ) /* RX ver 016 (27-Sep-83), best-available pair for v4.04, functionally validated; decoded from U36.ROM, master disk B17D7 */ \
	ROM_SYSTEM_BIOS( 5, "v50", "Balcones Operating System v5.0" ) \
	ROMX_LOAD( "u33.5.0_537p10828.bin", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(5) ) \
	ROMX_LOAD( "u34.5.0_537p10829.bin", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(5) ) \
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(5) ) \
	ROMX_LOAD( "v500.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS( 6, "v50v018", "Balcones Operating System v5.0 v018" ) \
	ROMX_LOAD( "537p10828.u33.5.0.bin", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(6) ) \
	ROMX_LOAD( "537p10829.u34.5.0.bin", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(6) ) \
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(6) ) /* good dump from Balcones v5.0 source disk B23D13; was BAD_DUMP cc4e1c2b */ \
	ROMX_LOAD( "537p10831.u36.5.0.bin", 0x1800, 0x0800, CRC(cda7f598) SHA1(08ffd18959e1708136076c82486b8d121a04fa23), ROM_BIOS(6) ) \
	ROM_REGION( 0x1000, "chargen", 0 ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(1) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(1) ) \
	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(2) ) \
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(2)  ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(3) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(3) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(4) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(4) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(5) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(5) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(6) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(6) )

ROM_START( x820ii )  ROM_X820II_CONTENTS ROM_END // 820-II, 8" floppy
ROM_START( x820ii5 ) ROM_X820II_CONTENTS ROM_END // 820-II, 5.25" floppy
ROM_START( x820iis ) ROM_X820II_CONTENTS ROM_END // 820-II, SASI hard disk
ROM_START( x820iilp ) ROM_X820II_CONTENTS ROM_END // 820-II, low-profile keyboard

ROM_START( x168 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v50" ) // 16/8 ships with the Low Profile Keyboard, which only the v5.0 "RX" monitor decodes (v4.04 can't, and echoes "what?")
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" ) // Changes sign-on message from Xerox 820-II to Xerox
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(0) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(0) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(0) )
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, CRC(97047d38) SHA1(f36506635653736b8d754d2c04f608180602b5a2), ROM_BIOS(0) ) // fitted for low-profile keyboard only; RX ver 016, recovered from master disk B17D7 (see x820ii)

	ROM_SYSTEM_BIOS( 1, "v50", "Balcones Operating System v5.0" ) // Operating system modifications for DEM and new 5.25" disk controller (4 new boot ROMs)
	ROMX_LOAD( "l5.u33.rom", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(1) )
	ROMX_LOAD( "l5.u34.rom", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(1) )
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(1) ) // was BAD_DUMP l5.u35.rom (44c8dbf8): same ROM as 537p10830 but with data bit 7 stuck high
	ROMX_LOAD( "u36.rx024.rom", 0x1800, 0x0800, CRC(a7f1d677) SHA1(8c2a442f3a691f2e181a640d65f767ce3b51d711), ROM_BIOS(1) ) // fitted for low-profile keyboard only

	// the 8086 boot ROM lives on the 16/8 coprocessor card (bus/xerox820/copro.cpp)

	ROM_REGION( 0x1000, "chargen", 0 )
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) )

	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(1) )
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(1)  )
ROM_END

ROM_START( mk83 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "2732mk83.bin", 0x0000, 0x1000, CRC(a845c7e1) SHA1(3ccf629c5cd384953794ac4a1d2b45678bd40e92) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "2716mk83.bin", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

ROM_START( mojmikro )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_LOAD( "mikro-s.u67", 0x0000, 0x0800, CRC(56a329a8) SHA1(22a5d6bef121d14eddc0c25e85b8a73f6ca6a65f))
	ROM_REGION( 0x0800, "chargen", ROMREGION_ERASEFF ) // MMSCHAR YU 8.1.1987
	ROM_LOAD( "mmschar-yu.u73", 0x0000, 0x0800, CRC(ebcc72d3) SHA1(1c3f90b1d2e57586dcd32385d0aaa09e56662e32))
ROM_END

// The 16/8 variants carry the 16/8 boot ROM set (which adds the 8086 ROM and differs
// from the plain 820-II); like x168 they group under x820ii.  Same ROMs across the three
// disk personalities, so they re-list x168's set (MAME de-duplicates the files by hash).
//
// Every 16/8 machine runs the v5.0 "RX" monitor (the default): only v5.0 decodes the Low
// Profile Keyboard fitted to the 16/8 (v4.04 can't -- it echoes "what?").  The v4.04 BIOS
// is offered for parity with x168 but is NOT a valid 16/8 configuration; on x168em it is
// outright dead, because EM-II/DEM support is monitor rev 500 ("Expansion Box II") which
// v4.04 predates -- under v4.04 ddskld never runs and the WD1002-05 disk is invisible.
// Always run the 16/8 on v5.0.
#define ROM_X168_CONTENTS \
	ROM_REGION( 0x2000, Z80_TAG, 0 ) \
	ROM_DEFAULT_BIOS( "v50" ) \
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" ) \
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(0) ) \
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(0) ) \
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(0) ) \
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, CRC(97047d38) SHA1(f36506635653736b8d754d2c04f608180602b5a2), ROM_BIOS(0) ) /* RX ver 016, recovered from master disk B17D7 (see x820ii) */ \
	ROM_SYSTEM_BIOS( 1, "v50", "Balcones Operating System v5.0" ) \
	ROMX_LOAD( "l5.u33.rom", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(1) ) \
	ROMX_LOAD( "l5.u34.rom", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(1) ) \
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(1) ) \
	ROMX_LOAD( "u36.rx024.rom", 0x1800, 0x0800, CRC(a7f1d677) SHA1(8c2a442f3a691f2e181a640d65f767ce3b51d711), ROM_BIOS(1) ) \
	ROM_REGION( 0x1000, "chargen", 0 ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) ) \
	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(1) ) \
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(1) )

// 16/8, 5.25" floppy on the FD1797 daughterboard (the standard non-EM-II 5.25" path;
// mixed SD/DD media).  The 8086 ROM comes from the 16/8 copro card.
ROM_START( x1685 )
	ROM_X168_CONTENTS
ROM_END

// 16/8 + Expansion Module II (the slot-connected EM-II).  The genuine box ROM (537p3682)
// and the 8086 ROM come from the EM-II copro card; the 5.25" disks are all-DD WD1002-05 media.
ROM_START( x168em )
	ROM_X168_CONTENTS
ROM_END

ROM_START( x168s ) ROM_X168_CONTENTS ROM_END // 16/8, SASI hard disk

/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE      INPUT     CLASS             INIT        COMPANY                       FULLNAME                          FLAGS
COMP( 1980, bigboard, 0,        0,      bigboard,    xerox820, bigboard_state,   empty_init, "Digital Research Computers", "Big Board",                      0 )
COMP( 1980, bigboard5,bigboard, 0,      bigboard5,   xerox820, bigboard_state,   empty_init, "Digital Research Computers", "Big Board (5.25\" drives)",      MACHINE_NOT_WORKING )
COMP( 1981, x820,     bigboard, 0,      xerox820,    xerox820, xerox820_state,   empty_init, "Xerox",                      "Xerox 820",                      MACHINE_NO_SOUND_HW )
COMP( 1982, mk82,     bigboard, 0,      bigboard,    xerox820, bigboard_state,   empty_init, "Scomar",                     "MK-82",                          0 )
COMP( 1983, x820ii,   0,        0,      xerox820ii,  xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II (8\" floppy)",      0 )
COMP( 1983, x820iilp, x820ii,   0,      xerox820iilp, xerox820, xerox820ii_state, empty_init, "Xerox",                     "Xerox 820-II (low profile keyboard)", 0 )
COMP( 1983, x820ii5,  x820ii,   0,      xerox820ii5, xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II (5.25\" floppy)",   0 )
COMP( 1983, x820iis,  x820ii,   0,      xerox820iis, xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II (SASI hard disk)",  0 )
COMP( 1983, x168,     x820ii,   0,      xerox168,    xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (8\" floppy)",        0 )
COMP( 1983, x1685,    x820ii,   0,      xerox1685,   xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (5.25\" floppy)",     0 ) // FD1797 5.25" daughterboard (mixed SD/DD); the EM-II 5.25" box is x168em
COMP( 1983, x168em,   x820ii,   0,      xerox168em,  xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (Expansion Module II)", 0 ) // EM-II (DEM): slot-connected WD1002-05 + the genuine 537p3682 box ROM, 5.25" ST-506 rigid + floppy
COMP( 1983, x168s,    x820ii,   0,      xerox168s,   xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (SASI hard disk)",    0 )
COMP( 1983, mk83,     bigboard, 0,      mk83,        xerox820, xerox820_state,   empty_init, "Scomar",                     "MK-83",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1985, mojmikro, bigboard, 0,      bigboard,    xerox820, bigboard_state,   empty_init, "<unknown>",                  "Moj mikro Slovenija",            0 )
