// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Texas Instruments Professional Computer (a MS-DOS compatible PC)

    to do:

    - native keyboard (not dumped)
    - cursor blink rate, blink attr
    - diags: pass all tests (interrupt test depends on timing of LOOP insn)
    - sound
    - printer
    - hdc
    - other cards (clock, serial)?
    - softlist

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/keyboard.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


#define LOG_DISK      (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_DISK)
#include "logmacro.h"

#define LOGDISK(...) LOGMASKED(LOG_DISK, __VA_ARGS__)


namespace {

class tipc_state : public driver_device
{
public:
	tipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsw0(*this, "DSW0")
		, m_upd8251(*this, "upd8251")
		, m_pit8253(*this, "pit8253")
		, m_pic8259(*this, "pic8259")
		, m_rs232(*this, "rs232")
		, m_irq2_merger(*this, "merge_irq2")
		, m_irq3_merger(*this, "merge_irq3")
		, m_ram(*this, RAM_TAG)
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_vdu_char_rom(*this, "vduchar")
		, m_crtc_gfx(*this, "crtc_gfx%u", 0U)
		, m_fdc(*this, "fdc")
		, m_floppies(*this, "fdc:%u", 0U)
	{ }

	void tipc(machine_config &config) ATTR_COLD;

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	enum
	{
		CPU_SBUF = 0,
		SBUF_WRITE,
		CPU_FDC,
		SBUF_READ
	};

	void tipc_io(address_map &map) ATTR_COLD;
	void tipc_map(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr_changed);

	void u47_w(uint8_t data);
	void u51_w(uint8_t data);

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);

	void fdc_drq_w(int state);

	uint8_t char_ram_r(offs_t offset);
	void char_ram_w(offs_t offset, uint8_t data);

	uint8_t attr_r(offs_t offset);
	void attr_w(offs_t offset, uint8_t data);

	uint8_t misc_r(offs_t offset);
	void misc_w(offs_t offset, uint8_t data);

	void palette_w(offs_t offset, uint8_t data);

	void vsync_changed(int state);

	required_device<cpu_device> m_maincpu;
	required_ioport m_dsw0;
	required_device<i8251_device> m_upd8251;
	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device>  m_pic8259;
	required_device<rs232_port_device> m_rs232;
	required_device<input_merger_all_high_device> m_irq2_merger;
	required_device<input_merger_all_high_device> m_irq3_merger;
	required_device<ram_device> m_ram;
	required_device<sy6545_1_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_vdu_char_rom;
	required_shared_ptr_array<uint8_t, 3> m_crtc_gfx;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppies;

	uint8_t m_u47;

	int m_buf_mode, side;
	uint16_t m_buf_ptr;
	std::unique_ptr<uint8_t[]> m_buf;

	uint8_t m_crtc_attr_latch, m_crtc_misc_latch, m_crtc_misc_buffer, m_crtc_palette[3];
	std::unique_ptr<uint8_t[]> m_crtc_ram, m_crtc_attr;

	floppy_image_device *m_floppy = nullptr;
};


static DEVICE_INPUT_DEFAULTS_START( kbd_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void tipc_state::machine_start()
{
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());
	m_buf = make_unique_clear<uint8_t[]>(1024);

	m_crtc_ram = make_unique_clear<uint8_t[]>(2048);
	m_crtc_attr = make_unique_clear<uint8_t[]>(2048);
}

void tipc_state::machine_reset()
{
	m_buf_ptr = 0;
	m_buf_mode = 0;
	side = 0;
	m_floppy = nullptr;

	m_u47 = 0;
	m_crtc_attr_latch = m_crtc_misc_latch = m_crtc_misc_buffer = 0;
}


void tipc_state::u47_w(uint8_t data)
{
	m_pit8253->write_gate0(BIT(data, 0)); // FIXME replace with input merger?

	m_irq3_merger->in_w<1>(BIT(data, 1));
	m_irq2_merger->in_w<1>(BIT(data, 2));

	if (BIT(m_u47 ^ data, 1) && !BIT(data, 1))
		m_irq3_merger->in_w<2>(0);
	if (BIT(m_u47 ^ data, 2) && !BIT(data, 2))
		m_irq2_merger->in_w<2>(0);

	m_fdc->dden_w(BIT(data, 3));
	side = !BIT(data, 5);
	if (m_floppy) m_floppy->ss_w(side);
	m_buf_mode = BIT(data, 6, 2);

	m_u47 = data;

	LOG("U47 <- %02x, mode %d, side %d\n", data, m_buf_mode, side);
}

void tipc_state::u51_w(uint8_t data)
{
	LOG("U51 <- %02x\n", data);

	data ^= 255;

	switch (data & 3)
	{
	case 1:
		m_floppy = m_floppies[0]->get_device();
		break;

	case 2:
		m_floppy = m_floppies[1]->get_device();
		break;

	case 0:
		m_floppy = nullptr;
		break;
	}

	m_fdc->set_floppy(m_floppy);
	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_floppy->ss_w(side);
		LOG("floppy ON %d ready? %d\n", data & 3, m_floppy->ready_r());
	}

	if (m_floppies[0]->get_device())
		m_floppies[0]->get_device()->mon_w(BIT(data, 4));
	if (m_floppies[1]->get_device())
		m_floppies[1]->get_device()->mon_w(BIT(data, 5));
}

void tipc_state::fdc_w(offs_t offset, uint8_t data)
{
	LOGDISK("disk mode %d fdc_w %d <- %02x\n", m_buf_mode, offset, data);
	switch (m_buf_mode)
	{
	case CPU_FDC:
		m_fdc->write(offset, data);
		break;

	case CPU_SBUF:
		if (offset == 2)
		{
			m_buf_ptr = 0;
		}
		else if (offset < 2)
		{
			m_buf[m_buf_ptr++] = data;
			m_buf_ptr &= 1023;
		}
		break;

	default:
		break;
	}
}

uint8_t tipc_state::fdc_r(offs_t offset)
{
	uint8_t data = 0;
	switch (m_buf_mode)
	{
	case CPU_FDC:
		data = m_fdc->read(offset);
		break;

	case CPU_SBUF:
		if (offset < 2)
		{
			data = m_buf[m_buf_ptr++];
			m_buf_ptr &= 1023;
		}
		break;

	default:
		break;
	}
	LOGDISK("disk mode %d fdc_r %d == %02x\n", m_buf_mode, offset, data);
	return data;
}

void tipc_state::fdc_drq_w(int state)
{
	if (!state) return;

	switch (m_buf_mode)
	{
	case SBUF_READ:
		LOGDISK("drq sbuf %d == %02x\n", m_buf_mode, m_buf[m_buf_ptr]);
		m_fdc->data_w(m_buf[m_buf_ptr++]);
		m_buf_ptr &= 1023;
		break;

	case SBUF_WRITE:
		m_buf[m_buf_ptr] = m_fdc->data_r();
		LOGDISK("drq sbuf %d <- %02x\n", m_buf_mode, m_buf[m_buf_ptr]);
		m_buf_ptr += 1;
		m_buf_ptr &= 1023;
		break;

	default:
		LOGDISK("unexpected drq in mode %d\n");
		break;
	}
}


void tipc_state::char_ram_w(offs_t offset, uint8_t data)
{
	m_crtc_ram[offset] = data;
	m_crtc_attr[offset] = m_crtc_attr_latch;
}

uint8_t tipc_state::char_ram_r(offs_t offset)
{
	m_crtc_attr_latch = m_crtc_attr[offset];
	return m_crtc_ram[offset];
}

void tipc_state::attr_w(offs_t offset, uint8_t data)
{
	m_crtc_attr_latch = data;
}

uint8_t tipc_state::attr_r(offs_t offset)
{
	return m_crtc_attr_latch;
}

/*
 * TechRef pp. 2-51, B-2
 *
 * b7 - interrupt enable
 * b6 - text screen enable
 * b5 - undocumented
 */
void tipc_state::misc_w(offs_t offset, uint8_t data)
{
	m_crtc_misc_latch = data;
	m_crtc_misc_buffer &= ~(1 << 3);
	m_crtc_misc_buffer |= BIT(data, 7) << 3;
}

/*
 * Bits 0-2 loop back color outputs (TechRef p. 2-56)
 *
 * Self test expects values 0xf0 and 0xf7 (ROM listing p. 90)
 */
uint8_t tipc_state::misc_r(offs_t offset)
{
	m_crtc_misc_buffer ^= 7;
	LOG("crtc misc buf %02x\n", 0xf0 | m_crtc_misc_buffer);
	return 0xf0 | m_crtc_misc_buffer;
}

// 0=blue, 1=green, 2=red
void tipc_state::palette_w(offs_t offset, uint8_t data)
{
	m_crtc_palette[offset >> 4] = data;
}

void tipc_state::vsync_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state & BIT(m_crtc_misc_latch, 7));
	if (BIT(m_crtc_misc_latch, 7)) m_crtc_misc_buffer |= state << 3;
}

/*
 * attribute bits (TechRef p. 2-42)
 *
 * 0    intensity level 1 (blue)
 * 1    intensity level 2 (red)
 * 2    intensity level 3 (green)
 * 3    character enable
 * 4    reverse
 * 5    underline
 * 6    blink
 * 7    alt character set
 *
 * attribute priority, from high to low (p. 2-55)
 *
 * color
 * reverse video and cursor
 * character enable
 * blink
 * underline
 */
MC6845_UPDATE_ROW(tipc_state::crtc_update_row)
{
	const pen_t *const pen = m_palette->pens();

	de = BIT(m_crtc_misc_latch, 6);

	for (int column = 0; column < 90; column++)
	{
		u8 code_0 = m_crtc_gfx[0][y * 92 + (column ^ 1)];
		u8 code_1 = m_crtc_gfx[1][y * 92 + (column ^ 1)];
		u8 code_2 = m_crtc_gfx[2][y * 92 + (column ^ 1)];

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = BIT(code_0, 7) | (BIT(code_1, 7) << 2) | (BIT(code_2, 7) << 1);
			color = BIT(m_crtc_palette[0], color) | (BIT(m_crtc_palette[2], color) << 1) | (BIT(m_crtc_palette[1], color) << 2);

			bitmap.pix(y, x) = pen[color];

			code_0 <<= 1;
			code_1 <<= 1;
			code_2 <<= 1;
		}
	}

	for (int column = 0; column < x_count; column++)
	{
		u8 code = m_crtc_ram[((ma + column) & 0x7ff)];
		u8 attr = m_crtc_attr[((ma + column) & 0x7ff)];
		u16 addr = (code << 4) | ra;
		u16 data = (m_vdu_char_rom[addr & 0xfff] ^ 0xff) << 1;

		if (BIT(data, 8))
			data = bitswap<9>(data, 7, 7, 6, 5, 4, 3, 2, 1, 1);

		if (column == cursor_x)
		{
			data = 0x1ff;
		}

		// underline
		if (BIT(attr, 5) && ra == 11) data = 0x1ff;

		// blink TODO

		// character enable
		if (!BIT(attr, 3)) data = 0;

		// reverse video
		if (BIT(attr, 4)) data ^= 0x1ff;

		for (int bit = 0; bit < 9; bit++)
		{
			int x = (column * 9) + bit;
			int color = (de && BIT(data, 8)) ? (attr & 7) : 0;

			if (color) bitmap.pix(y, x) = pen[color];

			data <<= 1;
		}
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(tipc_state::crtc_addr_changed)
{
}


static void pc_dd_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

static INPUT_PORTS_START(tipc)
	PORT_START("DSW0")
	PORT_DIPNAME( 0x04, 0x04, "Monitor type")
	PORT_DIPSETTING(    0x00, "50 Hz" )
	PORT_DIPSETTING(    0x04, "60 Hz" )
	PORT_DIPNAME( 0x03, 0x02, "Floppy type")
	PORT_DIPSETTING(    0x01, "640 KB DSQD" )
	PORT_DIPSETTING(    0x02, "320 KB DSDD" )
	PORT_DIPSETTING(    0x03, "160 KB SSDD" )
INPUT_PORTS_END

void tipc_state::tipc_map(address_map &map)
{
	map.unmap_value_high();

	map(0xc0000, 0xc7fff).ram().share(m_crtc_gfx[0]);
	map(0xc8000, 0xcffff).ram().share(m_crtc_gfx[1]);
	map(0xd0000, 0xd7fff).ram().share(m_crtc_gfx[2]);
	map(0xde000, 0xde7ff).rw(FUNC(tipc_state::char_ram_r), FUNC(tipc_state::char_ram_w)).mirror(0x800);
	map(0xdf000, 0xdf000).r(FUNC(tipc_state::misc_r));
	map(0xdf010, 0xdf03f).w(FUNC(tipc_state::palette_w));
	map(0xdf800, 0xdf800).rw(FUNC(tipc_state::attr_r), FUNC(tipc_state::attr_w));
	map(0xdf810, 0xdf810).w(m_crtc, FUNC(sy6545_1_device::address_w));
	map(0xdf811, 0xdf811).r(m_crtc, FUNC(sy6545_1_device::status_r));
	map(0xdf812, 0xdf812).w(m_crtc, FUNC(sy6545_1_device::register_w));
	map(0xdf813, 0xdf813).r(m_crtc, FUNC(sy6545_1_device::register_r));
	map(0xdf820, 0xdf820).w(FUNC(tipc_state::misc_w));
	map(0xf4000, 0xfffff).rom().region("bios", 0);
}

void tipc_state::tipc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).w(FUNC(tipc_state::u47_w));
	map(0x0001, 0x0001).portr("DSW0"); // U48 input buffer
	map(0x0002, 0x0002).nopw(); // U49 latch (printer data)
	map(0x0003, 0x0003).nopw(); // U50 latch (printer, leds)
	map(0x0004, 0x0004).w(FUNC(tipc_state::u51_w));
	map(0x0010, 0x0011).rw(m_upd8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0014, 0x0017).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0018, 0x0019).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0020, 0x0023).rw(FUNC(tipc_state::fdc_r), FUNC(tipc_state::fdc_w));
	map(0x0030, 0x0033).noprw(); // HDC
}

void tipc_state::tipc(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(15'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tipc_state::tipc_map);
	m_maincpu->set_addrmap(AS_IO, &tipc_state::tipc_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic8259, FUNC(pic8259_device::inta_cb));

	I8251(config, m_upd8251, 0);
	m_upd8251->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_upd8251->txd_handler().append(m_upd8251, FUNC(i8251_device::write_dsr)); // keyboard loops back tx to dsr
	m_upd8251->dtr_handler().set_inputline(m_maincpu, INPUT_LINE_NMI).invert();
	m_upd8251->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	m_upd8251->txrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir7_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard"); // FIXME
	m_rs232->rxd_handler().set(m_upd8251, FUNC(i8251_device::write_rxd));
	m_rs232->cts_handler().set(m_upd8251, FUNC(i8251_device::write_cts));
	m_rs232->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(kbd_rs232_defaults));

	clock_device &keyboard_tx_clock(CLOCK(config, "keyboard_tx_clock", XTAL(15'000'000) / (3 * 256))); // 300
	keyboard_tx_clock.signal_handler().set(m_upd8251, FUNC(i8251_device::write_txc));

	clock_device &keyboard_rx_clock(CLOCK(config, "keyboard_rx_clock", XTAL(15'000'000) / (3 * 32))); // 2400
	keyboard_rx_clock.signal_handler().set(m_upd8251, FUNC(i8251_device::write_rxc));

	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>(XTAL(15'000'000) / (3 * 4)); // speaker
	m_pit8253->set_clk<1>(XTAL(15'000'000) / (3 * 8)); // timer A
	m_pit8253->out_handler<1>().set(m_irq3_merger, FUNC(input_merger_all_high_device::in_w<0>)).invert();
	m_pit8253->out_handler<1>().append(m_irq3_merger, FUNC(input_merger_all_high_device::in_w<2>)).invert();
	m_pit8253->set_clk<2>(XTAL(15'000'000) / (3 * 8)); // timer B
	m_pit8253->out_handler<2>().set(m_irq2_merger, FUNC(input_merger_all_high_device::in_w<0>)).invert();
	m_pit8253->out_handler<2>().append(m_irq2_merger, FUNC(input_merger_all_high_device::in_w<2>)).invert();

	INPUT_MERGER_ALL_HIGH(config, m_irq2_merger).output_handler().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	INPUT_MERGER_ALL_HIGH(config, m_irq3_merger).output_handler().set(m_pic8259, FUNC(pic8259_device::ir3_w));

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	FD1793(config, m_fdc, 1000000);
	m_fdc->intrq_wr_callback().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_fdc->drq_wr_callback().set(FUNC(tipc_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_dd_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_dd_floppies, "525dd", floppy_image_device::default_pc_floppy_formats);

	// CRT Controller board

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(18'000'000), 936, 0, 720, 320, 0, 300);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	SY6545_1(config, m_crtc, XTAL(18'000'000) / 9);
	m_crtc->set_char_width(9);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen("screen");
	m_crtc->set_update_row_callback(FUNC(tipc_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(tipc_state::crtc_addr_changed));
	m_crtc->out_vsync_callback().set(FUNC(tipc_state::vsync_changed));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("512K, 768K");
}

ROM_START(tipc)
	ROM_REGION(0x10000, "bios", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v124")
	ROM_SYSTEM_BIOS(0, "v124", "v1.24")
	// XU62, U63
	ROMX_LOAD("u62.bin", 0xa000, 0x1000, CRC(7b057200) SHA1(2a7a7cd3a36e023110ae7a04dff109982a0fa95d), ROM_BIOS(0))
	ROMX_LOAD("u63.bin", 0xb000, 0x1000, CRC(9facde50) SHA1(07e561f8812fc43133414ac62d350fba729433b3), ROM_BIOS(0))

	ROM_REGION(0x2000, "vduchar", 0)
	ROM_LOAD("u25.bin", 0x00000, 0x1000, CRC(c4e9ac25) SHA1(d7b0e6d5d2a4e5feab3bcf9ab5fd1d7c15401883)) // p/n 2223065-1
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT     CLASS        INIT        COMPANY              FULLNAME                 FLAGS
COMP( 1983, tipc,    0,       0,      tipc,    tipc,     tipc_state,  empty_init, "Texas Instruments", "Professional Computer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
