// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    BASF 7100

    Models:
    - 7120: 24k disk controller memory, 3x 5.25" single sided
    - 7125: 32k disk controller memory, 3x 5.25" double sided
    - 7130: 32k disk controller memory, 1x 5.25" double sided, 1x Winchester

    Hardware:
    - Z-80A
    - I8259A
    - I8251
    - COM8116
    - 3x 8255A
    - 64k memory
    - 5.0688 MHz XTAL
    - 4x DSW8
    - External ports: Printer, Dialer, Data Comm I/O, Aux I/O

    Video board:
    - Motorola 160A002-B (?)
    - 2x 8255A
    - 18.720 MHz XTAL

    Floppy controller:
    - Z-80A
    - 24k/32k memory
    - FD1791B-02

    Aux I/O board:
    - I8251
    - COM8116
    - 5.0688 MHz XTAL

    Keyboard:
    - 30293E-054 20-04592-054 (?)

    TODO:
    - The floppy is partially hooked up, everything else needs to be done

    Notes:
    - Runs the BOS operating system, possibly also CP/M?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class basf7100_state : public driver_device
{
public:
	basf7100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi%u", 0U),
		m_fdccpu(*this, "fdccpu"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_chargen(*this, "chargen"),
		m_bootview(*this, "bootview"),
		m_fdc_intrq_vector(0),
		m_fdc_drq(false)
	{ }

	void basf7100(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device_array<i8255_device, 5> m_ppi;
	required_device<z80_device> m_fdccpu;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_chargen;
	memory_view m_bootview;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	void fdc_mem_map(address_map &map);
	void fdc_io_map(address_map &map);

	uint8_t mmio_r(offs_t offset);
	void mmio_w(offs_t offset, uint8_t data);

	void sod_high_w(uint8_t data);
	void sod_low_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void fdc_drq_w(int state);
	uint8_t fdc_ctrl_r();
	void fdc_ctrl_w(uint8_t data);
	void fdc_intrq_vector_w(uint8_t data);
	void unk_cc_w(uint8_t data);

	IRQ_CALLBACK_MEMBER(fdccpu_irq_callback);
	uint8_t int_flags_r();

	uint16_t m_sod;
	uint8_t m_fdc_intrq_vector;
	bool m_fdc_drq;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void basf7100_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("shared_ram");
	map(0x8000, 0xffff).ram();
	map(0xff00, 0xffff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
	map(0x0000, 0xffff).view(m_bootview);
	m_bootview[0](0x0000, 0x001f).mirror(0xffe0).rom().region("maincpu", 0);
}

void basf7100_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
//	map(0x08, 0x09).mirror(0x02) 8259
	map(0x0c, 0x0f).rw(m_ppi[2], FUNC(i8255_device::read), FUNC(i8255_device::write));
//	map(0x10, 0x11) 8251 (primary)
//	map(0x12, 0x12) baud rate
//	map(0x14, 0x15) 8251 (secondary)
//	map(0x16, 0x16) baud rate
//	map(0x17, 0x17) rs232 flags/control
	map(0x18, 0x18).r(FUNC(basf7100_state::int_flags_r));
//	map(0x1c, 0x1f) switches
//	map(0xb0, 0xb3) display hardware clear
	map(0xb8, 0xbb).rw(m_ppi[3], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbc, 0xbf).lr8(NAME([this] (offs_t offset) -> uint8_t { return m_ppi[4]->read(offset ^ 3); }));
	map(0xbc, 0xbf).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ppi[4]->write(offset ^ 3, data); }));
	map(0xc0, 0xc3).rw(m_fdc, FUNC(fd1791_device::read), FUNC(fd1791_device::write));
	map(0xc4, 0xc4).rw(FUNC(basf7100_state::fdc_ctrl_r), FUNC(basf7100_state::fdc_ctrl_w));
	map(0xca, 0xca).w(FUNC(basf7100_state::fdc_intrq_vector_w));
	map(0xcc, 0xcc).w(FUNC(basf7100_state::unk_cc_w));
}

void basf7100_state::fdc_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("shared_ram");
	map(0x8000, 0xefff).ram();
	map(0xf000, 0xfbff).ram();
	map(0xfc00, 0xffff).rom().region("fdccpu", 0);
	map(0xff00, 0xffff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
}

void basf7100_state::fdc_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( basf7100 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout crt8002_charlayout =
{
	8, 12,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START( gfx_crt8002 )
	GFXDECODE_ENTRY( "chargen", 0, crt8002_charlayout, 0, 1 )
GFXDECODE_END

void basf7100_state::sod_high_w(uint8_t data)
{
	m_sod = (data << 8) | (m_sod & 0x00ff);
}

void basf7100_state::sod_low_w(uint8_t data)
{
	// 7654----  sod low byte
	// ----3210  set to input (clear in progress flag?)

	m_sod = (m_sod & 0xff00) | ((data & 0xf0) << 0);
}

uint32_t basf7100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 26; y++)
	{
		for (int x = 0; x < 80; x++)
		{
			uint16_t addr = m_sod + y * 80 + x;
			uint8_t code = m_maincpu->space(AS_PROGRAM).read_byte(addr);

			// draw 12 lines
			for (int i = 0; i < 12; i++)
			{
				uint8_t data = m_chargen[(((code & 0x7f) << 4) + i)];

				if (BIT(code, 7))
					data ^= 0xff;

				// 8 pixels of the character
				bitmap.pix(y * 12 + i, x * 8 + 0) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 1) = BIT(data, 6) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 2) = BIT(data, 5) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 3) = BIT(data, 4) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 4) = BIT(data, 3) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 5) = BIT(data, 2) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 6) = BIT(data, 1) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 12 + i, x * 8 + 7) = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}


//**************************************************************************
//  FLOPPY
//**************************************************************************

static void basf7100_floppies(device_slot_interface &device)
{
	device.option_add("basf6106", FLOPPY_525_SSSD);
}

void basf7100_state::fdc_drq_w(int state)
{
	m_fdc_drq = bool(state);
}

uint8_t basf7100_state::fdc_ctrl_r()
{
	// 7-------  unknown, checked after seek cmd
	// -654321-  unknown
	// -------0  fdc drq

	uint8_t data = 0x00;

	data |= 1 << 7;
	data |= (m_fdc_drq ? 0x01 : 0x00);

	return data;
}

void basf7100_state::fdc_ctrl_w(uint8_t data)
{
	// 7654----  unknown
	// ----3---  motor on?
	// -----2--  unknown
	// ------10  drive select

	if (data != 0x08)
		logerror("fdc_ctrl_w: %04x\n", data);
	
	floppy_image_device *floppy = nullptr;

	if ((data & 0x03) < 3)
		floppy = m_floppy[data & 0x03]->get_device();

	m_fdc->set_floppy(floppy);

	// motor runs all the time for now
	if (floppy)
		floppy->mon_w(0);

	// set to mfm
	m_fdc->dden_w(0);
}

void basf7100_state::fdc_intrq_vector_w(uint8_t data)
{
	m_fdc_intrq_vector = data;
}

void basf7100_state::unk_cc_w(uint8_t data)
{
	logerror("unk_cc_w: %02x\n", data);

	m_bootview.disable();
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t basf7100_state::mmio_r(offs_t offset)
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

void basf7100_state::mmio_w(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_IO).write_byte(offset, data);
}

IRQ_CALLBACK_MEMBER( basf7100_state::fdccpu_irq_callback )
{
	return m_fdc_intrq_vector;
}

uint8_t basf7100_state::int_flags_r()
{
	// 765-----  unknown
	// ---4----  unknown (used, needs to be set at least 2 times to continue booting)
	// ----3---  unknown (used)
	// -----210  unknown

	return m_screen->vblank() ? 0x10 : 0x00;
}

void basf7100_state::machine_start()
{
	// register for save states
	save_item(NAME(m_sod));
	save_item(NAME(m_fdc_intrq_vector));
	save_item(NAME(m_fdc_drq));
}

void basf7100_state::machine_reset()
{
	m_bootview.select(0);
	m_fdccpu->set_pc(0xfc00);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void basf7100_state::basf7100(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &basf7100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &basf7100_state::io_map);

	Z80(config, m_fdccpu, 4000000);
	m_fdccpu->set_addrmap(AS_PROGRAM, &basf7100_state::fdc_mem_map);
	m_fdccpu->set_addrmap(AS_IO, &basf7100_state::fdc_io_map);
	m_fdccpu->set_irq_acknowledge_callback(FUNC(basf7100_state::fdccpu_irq_callback));

	I8255(config, m_ppi[0]);
	// port a: input (switches?)
	// port b: input (keyboard data)
	// port c: input (keyboard strobe), output (bell)

	I8255(config, m_ppi[1]);
	// port a: output (printer data)
	// port b: output (leds?)
	// port c: input (printer handshake)

	I8255(config, m_ppi[2]);
	// port a: input (rs232 flags, auto dial)
	// port b: output (auto dialer digits)
	// port c: input (auto dialer status), output (rs232 and auto dialer control)

	I8255(config, m_ppi[3]);
	// port a: output (cursor column)
	// port b: output (cursor row, graphics enable)
	// port c: output (highlight mode)

	I8255(config, m_ppi[4]);
	// port a: output (roll offset)
	m_ppi[4]->out_pb_callback().set(FUNC(basf7100_state::sod_high_w));
	m_ppi[4]->out_pc_callback().set(FUNC(basf7100_state::sod_low_w));

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_size(640, 288); // wrong because of wrong charrom
	m_screen->set_visarea_full();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_screen_update(FUNC(basf7100_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_crt8002);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// floppy
	FD1791(config, m_fdc, 1000000);
	m_fdc->intrq_wr_callback().set_inputline(m_fdccpu, INPUT_LINE_IRQ0);
	m_fdc->drq_wr_callback().set(FUNC(basf7100_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( basf7120 )
	ROM_REGION(0x20, "maincpu", 0)
	ROM_LOAD("19-2113-2.u21", 0x00, 0x20, CRC(4405e26f) SHA1(0f93c47e9f546b42a85e5eced58337e0add443c4)) // IM5610CJE

	ROM_REGION(0x400, "fdccpu", 0)
	ROM_LOAD("19-2130-2h.u45", 0x000, 0x400, CRC(cb077c69) SHA1(dfa16082b88275442c48082aeb5f62fe1238ae3e)) // 2708

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD("19-2114-01e.bin", 0x000, 0x400, CRC(d694b5dd) SHA1(6262379ba565c1de072b2b21dc3141db1ec5129c))
	
	ROM_REGION(0x50, "floppy_pal", 0)
	ROM_LOAD("19-2131-1.u23", 0x00, 0x28, CRC(f37ed4bc) SHA1(824b4405f396c262cf8116f85eb0b548eabb4c04)) // PAL10L8MJ
	ROM_LOAD("19-2132-1.u24", 0x28, 0x28, CRC(b918ff18) SHA1(c6d7cd9642ed32e56b5c1df1ddf3afe09d744ebc)) // PAL10L8MJ
	
	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("video.30", 0x000, 0x100, CRC(89175ac9) SHA1(69b2055bee87e11cc74c70cef2f2bebcbd0004c9)) // N82S129N (label missing)

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("8002.bin", 0x000, 0x800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690)) // placeholder, wrong
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME  FLAGS
COMP( 1982, basf7120, 0,      0,      basf7100, basf7100, basf7100_state, empty_init, "BASF",   "7120",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
