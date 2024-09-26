// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    BASF 7100

    This system is based on (or even identical to) the DigiLog Microterm II

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

    TODO:
    - Dump real character ROM
    - Improve video emulation
    - Find documentation for switches S2, S3, S4 (might be app. specific)
    - Serial interrupts, flags, control

    Notes:
    - Runs the BOS operating system, possibly also CP/M?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/wd_fdc.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "basf7100_kbd.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


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
		m_fdccpu(*this, "fdccpu"),
		m_pic(*this, "pic"),
		m_ppi(*this, "ppi%u", 0U),
		m_usart(*this, "usart%u", 0U),
		m_centronics(*this, "centronics"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_shared_ram(*this, "shared_ram"),
		m_chargen(*this, "chargen"),
		m_bootview(*this, "bootview"),
		m_cursor_col(0x00), m_cursor_row(0x00),
		m_highlight(0x00),
		m_roll_offset(0x00),
		m_sod(0x0000),
		m_fdc_intrq_vector(0),
		m_fdc_drq(false),
		m_int_flags(0x00)
	{ }

	void basf7100(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_fdccpu;
	required_device<pic8259_device> m_pic;
	required_device_array<i8255_device, 5> m_ppi;
	required_device_array<i8251_device, 2> m_usart;
	required_device<centronics_device> m_centronics;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_shared_ram;
	required_region_ptr<uint8_t> m_chargen;
	memory_view m_bootview;

	enum : uint8_t
	{
		INT_KEYBOARD   = 0x08,
		INT_VBLANK     = 0x10,
		INT_CENTRONICS = 0x40
	};

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void fdc_mem_map(address_map &map) ATTR_COLD;
	void fdc_io_map(address_map &map) ATTR_COLD;

	uint8_t mmio_r(offs_t offset);
	void mmio_w(offs_t offset, uint8_t data);

	void keyboard_w(uint8_t data);

	void cursor_col_w(uint8_t data);
	void cursor_row_w(uint8_t data);
	void highlight_w(uint8_t data);
	void roll_offset_w(uint8_t data);
	void sod_high_w(uint8_t data);
	void sod_low_w(uint8_t data);
	void vblank_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void centronics_ctrl_w(uint8_t data);
	void centronics_busy_w(int state);

	void fdc_drq_w(int state);
	uint8_t fdc_ctrl_r();
	void fdc_ctrl_w(uint8_t data);
	void fdc_intrq_vector_w(uint8_t data);
	void unk_cc_w(uint8_t data);

	IRQ_CALLBACK_MEMBER(maincpu_irq_callback);
	IRQ_CALLBACK_MEMBER(fdccpu_irq_callback);

	uint8_t m_cursor_col;
	uint8_t m_cursor_row;
	uint8_t m_highlight;
	uint8_t m_roll_offset;
	uint16_t m_sod;
	uint8_t m_fdc_intrq_vector;
	bool m_fdc_drq;
	uint8_t m_int_flags;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void basf7100_state::mem_map(address_map &map)
{
	map(0x0000, 0x9fff).ram().share("shared_ram");
	map(0xa000, 0xffff).ram();
	map(0xff00, 0xffff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
	map(0x0000, 0xffff).view(m_bootview);
	m_bootview[0](0x0000, 0x001f).mirror(0xffe0).rom().region("maincpu", 0);
}

void basf7100_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x09).mirror(0x02).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0c, 0x0f).rw(m_ppi[2], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x11).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x12, 0x12).w("brg0", FUNC(com8116_device::stt_str_w)); // or str_stt_w
	map(0x14, 0x15).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x16, 0x16).w("brg1", FUNC(com8116_device::stt_str_w)); // or str_stt_w
//  map(0x17, 0x17) rs232 flags/control
	map(0x18, 0x18).lr8(NAME([this] () -> uint8_t { return m_int_flags; }));
	map(0x1c, 0x1c).portr("S1");
	map(0x1d, 0x1d).portr("S2");
	map(0x1e, 0x1e).portr("S3");
	map(0x1f, 0x1f).portr("S4");
//  map(0xb0, 0xb3) display hardware clear
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
	map(0x0000, 0x9fff).ram().share("shared_ram");
	map(0xa000, 0xfbff).ram();
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
	PORT_START("S1")
	PORT_DIPNAME(0x0f, 0x0e, "Baud Rate")      PORT_DIPLOCATION("S1:1,2,3,4")
	PORT_DIPSETTING(   0x00, "50")
	PORT_DIPSETTING(   0x01, "75")
	PORT_DIPSETTING(   0x02, "110")
	PORT_DIPSETTING(   0x03, "135")
	PORT_DIPSETTING(   0x04, "150")
	PORT_DIPSETTING(   0x05, "300")
	PORT_DIPSETTING(   0x06, "600")
	PORT_DIPSETTING(   0x07, "1200")
	PORT_DIPSETTING(   0x08, "1800")
	PORT_DIPSETTING(   0x09, "2005")
	PORT_DIPSETTING(   0x0a, "2400")
	PORT_DIPSETTING(   0x0b, "3600")
	PORT_DIPSETTING(   0x0c, "4800")
	PORT_DIPSETTING(   0x0d, "7200")
	PORT_DIPSETTING(   0x0e, "9600")
	PORT_DIPSETTING(   0x0f, "19800")
	PORT_DIPNAME(0x30, 0x30, "Data Bits")      PORT_DIPLOCATION("S1:5,6")
	PORT_DIPSETTING(   0x00, "5")
	PORT_DIPSETTING(   0x10, "6")
	PORT_DIPSETTING(   0x20, "7")
	PORT_DIPSETTING(   0x30, "8")
	PORT_DIPNAME(0x40, 0x00, "Parity Enabled") PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(   0x00, DEF_STR(No))
	PORT_DIPSETTING(   0x40, DEF_STR(Yes))
	PORT_DIPNAME(0x80, 0x80, "Parity")         PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(   0x00, "Odd")
	PORT_DIPSETTING(   0x80, "Even")

	PORT_START("S2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "S2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "S2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "S2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "S2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "S2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "S2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "S2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "S2:8")

	PORT_START("S3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "S3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "S3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "S3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "S3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "S3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "S3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "S3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "S3:8")

	PORT_START("S4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "S4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "S4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "S4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "S4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "S4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "S4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "S4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "S4:8")
INPUT_PORTS_END

void basf7100_state::keyboard_w(uint8_t data)
{
	// PPI OBF
	m_int_flags &= ~INT_KEYBOARD;
	m_int_flags |= BIT(data, 1) ? INT_KEYBOARD : 0x00;

	m_pic->ir3_w(BIT(data, 1));

	// TODO: keyboard bell
}


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

void basf7100_state::cursor_col_w(uint8_t data)
{
	m_cursor_col = data;
}

void basf7100_state::cursor_row_w(uint8_t data)
{
	// 7-------  graphics enable?
	// -65-----  unknown
	// ---43210  cursor row

	m_cursor_row = data & 0x1f;
}

void basf7100_state::highlight_w(uint8_t data)
{
	logerror("highlight = %02x\n", data);
	m_highlight = data;
}

void basf7100_state::roll_offset_w(uint8_t data)
{
	m_roll_offset = data;
}

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

void basf7100_state::vblank_w(int state)
{
	m_int_flags &= ~INT_VBLANK;
	m_int_flags |= state ? INT_VBLANK : 0x00;

	m_pic->ir4_w(state);
}

uint32_t basf7100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint16_t addr = m_sod + (m_roll_offset * 16);

	for (int y = 0; y < 24; y++)
	{
		uint8_t line_attr = 0x00;

		for (int x = 0; x < 80; x++)
		{
			if (addr >= (m_sod + 0x780))
				addr = m_sod;

			uint8_t code = m_shared_ram[addr];

			// draw 12 lines
			for (int i = 0; i < 12; i++)
			{
				uint8_t data = m_chargen[(((code & 0x7f) << 4) + i)];

				// character attributes
				bool inverse = false;
				bool half = false;
				bool underline = false;
				bool blink = false;

				// global attributes
				if (BIT(m_highlight, 3) && BIT(code, 7))
				{
					inverse = bool(BIT(m_highlight, 4));
					half = bool(BIT(m_highlight, 5));
					underline = bool(BIT(m_highlight, 6));
					blink = bool(BIT(m_highlight, 7));
				}

				// line attribute
				if (BIT(m_highlight, 1))
				{
					// control characters
					if ((code & 0xf0) == 0x90) // only 0x90-0x9f, not 0x10-0x1f?
						line_attr = code & 0x0f;

					inverse = bool(BIT(line_attr, 0));
					half = bool(BIT(line_attr, 1));
					underline = bool(BIT(line_attr, 2));
					blink = bool(BIT(line_attr, 3));
				}

				// correct some issues because of the wrong charrom
				if ((code & 0x7f) == 0 || ((code & 0x7f) >= 0x10 && (code & 0x7f) <= 0x14))
					data = 0x00;

				// display of control characters
				if (BIT(m_highlight, 0) == 0 && (code & 0x60) == 0)
					data = 0x00;

				if (underline && i == 10)
					data = 0xff;

				if (blink && m_screen->frame_number() & 0x08) // timing?
					data = 0x00;

				if (inverse)
					data ^= 0xff;

				if (y == m_cursor_row && x == m_cursor_col && m_screen->frame_number() & 0x08)
					data = 0xff;

				// 8 pixels of the character
				bitmap.pix(y * 12 + i, x * 8 + 0) = palette[BIT(data, 7) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 1) = palette[BIT(data, 6) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 2) = palette[BIT(data, 5) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 3) = palette[BIT(data, 4) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 4) = palette[BIT(data, 3) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 5) = palette[BIT(data, 2) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 6) = palette[BIT(data, 1) ? 2 - (half ? 1 : 0) : 0];
				bitmap.pix(y * 12 + i, x * 8 + 7) = palette[BIT(data, 0) ? 2 - (half ? 1 : 0) : 0];
			}

			// next char
			addr++;
		}
	}

	return 0;
}


//**************************************************************************
//  CENTRONICS
//**************************************************************************

void basf7100_state::centronics_ctrl_w(uint8_t data)
{
	// ppi configured in mode1 (strobed output)
	m_centronics->write_strobe(BIT(data, 7));
}

void basf7100_state::centronics_busy_w(int state)
{
	m_int_flags &= ~INT_CENTRONICS;
	m_int_flags |= state ? 0x00 : INT_CENTRONICS;

	m_pic->ir6_w(state);
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

IRQ_CALLBACK_MEMBER( basf7100_state::maincpu_irq_callback )
{
	uint32_t vector = 0;

	vector |= m_pic->acknowledge() << 16;
	vector |= m_pic->acknowledge();
	vector |= m_pic->acknowledge() << 8;

	return vector;
}

IRQ_CALLBACK_MEMBER( basf7100_state::fdccpu_irq_callback )
{
	return m_fdc_intrq_vector;
}

void basf7100_state::machine_start()
{
	// register for save states
	save_item(NAME(m_cursor_col));
	save_item(NAME(m_cursor_row));
	save_item(NAME(m_highlight));
	save_item(NAME(m_roll_offset));
	save_item(NAME(m_sod));
	save_item(NAME(m_fdc_intrq_vector));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_int_flags));
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
	m_maincpu->set_irq_acknowledge_callback(FUNC(basf7100_state::maincpu_irq_callback));

	Z80(config, m_fdccpu, 4000000);
	m_fdccpu->set_addrmap(AS_PROGRAM, &basf7100_state::fdc_mem_map);
	m_fdccpu->set_addrmap(AS_IO, &basf7100_state::fdc_io_map);
	m_fdccpu->set_irq_acknowledge_callback(FUNC(basf7100_state::fdccpu_irq_callback));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8251(config, m_usart[0], 0); // unknown clock
	m_usart[0]->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart[0]->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	com8116_device &brg0(COM8116(config, "brg0", 5.0688_MHz_XTAL));
	brg0.fr_handler().set(m_usart[0], FUNC(i8251_device::write_rxc));
	brg0.ft_handler().set(m_usart[0], FUNC(i8251_device::write_txc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_usart[0], FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_usart[0], FUNC(i8251_device::write_dsr));

	I8251(config, m_usart[1], 0); // unknown clock
	m_usart[1]->txd_handler().set("auxrs232", FUNC(rs232_port_device::write_txd));
	m_usart[1]->rts_handler().set("auxrs232", FUNC(rs232_port_device::write_rts));
	m_usart[1]->dtr_handler().set("auxrs232", FUNC(rs232_port_device::write_dtr));

	com8116_device &brg1(COM8116(config, "brg1", 5.0688_MHz_XTAL));
	brg1.fr_handler().set(m_usart[1], FUNC(i8251_device::write_rxc));
	brg1.ft_handler().set(m_usart[1], FUNC(i8251_device::write_txc));

	rs232_port_device &auxrs232(RS232_PORT(config, "auxrs232", default_rs232_devices, nullptr));
	auxrs232.rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	auxrs232.cts_handler().set(m_usart[1], FUNC(i8251_device::write_cts));
	auxrs232.dsr_handler().set(m_usart[1], FUNC(i8251_device::write_dsr));

	I8255(config, m_ppi[0]);
	// port a: input (switches?)
	m_ppi[0]->in_pb_callback().set("keyboard", FUNC(basf7100_kbd_device::read));
	m_ppi[0]->out_pc_callback().set(FUNC(basf7100_state::keyboard_w));

	I8255(config, m_ppi[1]);
	m_ppi[1]->out_pa_callback().set("centronics_latch", FUNC(output_latch_device::write));
	// port b: output (leds?)
	m_ppi[1]->out_pc_callback().set(FUNC(basf7100_state::centronics_ctrl_w));

	I8255(config, m_ppi[2]);
	// port a: input (rs232 flags, auto dial)
	// port b: output (auto dialer digits)
	// port c: input (auto dialer status), output (rs232 and auto dialer control)

	I8255(config, m_ppi[3]);
	m_ppi[3]->out_pa_callback().set(FUNC(basf7100_state::cursor_col_w));
	m_ppi[3]->out_pb_callback().set(FUNC(basf7100_state::cursor_row_w));
	m_ppi[3]->out_pc_callback().set(FUNC(basf7100_state::highlight_w));

	I8255(config, m_ppi[4]);
	m_ppi[4]->out_pa_callback().set(FUNC(basf7100_state::roll_offset_w));
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
	m_screen->screen_vblank().set(FUNC(basf7100_state::vblank_w));

	GFXDECODE(config, "gfxdecode", "palette", gfx_crt8002);

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	// centronics
	output_latch_device &centronics_latch(OUTPUT_LATCH(config, "centronics_latch"));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(centronics_latch);
	m_centronics->ack_handler().set(m_ppi[1], FUNC(i8255_device::pc6_w));
	m_centronics->busy_handler().set(FUNC(basf7100_state::centronics_busy_w));

	// floppy
	FD1791(config, m_fdc, 1000000);
	m_fdc->intrq_wr_callback().set_inputline(m_fdccpu, INPUT_LINE_IRQ0);
	m_fdc->drq_wr_callback().set(FUNC(basf7100_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "floppy_list").set_original("basf7100");

	// keyboard
	basf7100_kbd_device &keyboard(BASF7100_KBD(config, "keyboard"));
	keyboard.int_handler().set(m_ppi[0], FUNC(i8255_device::pc2_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( basf7120 )
	ROM_REGION(0x20, "maincpu", 0)
	ROM_LOAD("19-2113-2.u21", 0x00, 0x20, CRC(4405e26f) SHA1(0f93c47e9f546b42a85e5eced58337e0add443c4)) // IM5610CJE

	ROM_REGION(0x400, "fdccpu", 0)
	ROM_LOAD("19-2130-2h.u45", 0x000, 0x400, CRC(cb077c69) SHA1(dfa16082b88275442c48082aeb5f62fe1238ae3e)) // 2708

	ROM_REGION(0x50, "floppy_pal", 0)
	ROM_LOAD("19-2131-1.u23", 0x00, 0x28, CRC(f37ed4bc) SHA1(824b4405f396c262cf8116f85eb0b548eabb4c04)) // PAL10L8MJ
	ROM_LOAD("19-2132-1.u24", 0x28, 0x28, CRC(b918ff18) SHA1(c6d7cd9642ed32e56b5c1df1ddf3afe09d744ebc)) // PAL10L8MJ

	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("video.30", 0x000, 0x100, CRC(89175ac9) SHA1(69b2055bee87e11cc74c70cef2f2bebcbd0004c9)) // N82S129N (label missing)

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("160a002-b.bin", 0x000, 0x800, BAD_DUMP CRC(8787145c) SHA1(91d8c79f901a41117e99325a53b677f06baf1074)) // handcrafted, wrong
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME  FLAGS
COMP( 1982, basf7120, 0,      0,      basf7100, basf7100, basf7100_state, empty_init, "BASF",   "7120",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
