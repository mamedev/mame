// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Sanyo MBC-200

Machine MBC-1200 is identical but sold outside of Japan

16 x HM6116P-3 2K x 8 SRAM soldered onboard (so 32k ram)
4 x HM6116P-3 2K x 8 SRAM socketed (so 8k ram)
4 x MB83256 32K x 8 socketed (128k rom)
Floppy = 5.25"
MBC1200 has one floppy while MBC1250 has 2. The systems are otherwise identical.

Keyboard communicates serially to UART at E0,E1. The keyboard uses an undumped
8748 microcontroller with a 12*8 key matrix. The input codes are not ASCII, so
using custom code until the required details become available.

On back side:
- keyboard DIN connector
- Centronics printer port
- RS-232C 25pin connector

SBASIC:
Running programs: the file names used within SBASIC must be in
uppercase. For example, run "DEMO" .
You can also run a basic program from CP/M: sbasic "GRAPHICS" .
To Break, press either ^N or ^O (display freezes), then ^C .
Some control keys: 0x14 = Home; 0x8 = Left/BS; 0xA = Down; 0xB = Up; 0xC = Right.
GAIJI.BAS doesn't work because GAIJI.FNT is missing.

TODO:
- Less expensive synchronisation between CPUs
- UART connections
- PPI 9F PA0 and PA1 are connected to jumpers
- Any other devices?

****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/output_latch.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class mbc200_state : public driver_device
{
public:
	mbc200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_ppi_m(*this, "ppi_m")
		, m_ppi_s(*this, "ppi_s")
		, m_vram(*this, "vram")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0)
		, m_printer(*this, "printer")
	{ }

	void mbc200(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 boot_m1_r(offs_t offset);
	u8 sub_io_r(offs_t offset);
	void sub_io_w(offs_t offset, u8 data);
	u8 ps_porta_r();
	template <unsigned Bit> void ps_porta_w(int state);
	void ps_portc_w(u8 data);
	u8 pm_porta_r();
	u8 pm_portc_r();
	void pm_portb_w(u8 data);
	void pm_portc_w(u8 data);
	u8 keyboard_r(offs_t offset);
	void kbd_put(u8 data);
	MC6845_UPDATE_ROW(update_row);

	void main_mem(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	void main_opcodes(address_map &map) ATTR_COLD;
	void sub_mem(address_map &map) ATTR_COLD;
	void sub_io(address_map &map) ATTR_COLD;

	required_device<palette_device> m_palette;
	required_device<mc6845_device> m_crtc;
	required_device<i8255_device> m_ppi_m;
	required_device<i8255_device> m_ppi_s;
	required_shared_ptr<u8> m_vram;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<mb8876_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<centronics_device> m_printer;

	u8 m_cpu_m_sound = 0U;
	u8 m_cpu_s_sound = 0U;
	u8 m_pm_portc = 0xffU;
	u8 m_ps_porta = 0xffU;
	u8 m_comm_data = 0xffU;
	u8 m_term_data = 0U;
};


void mbc200_state::main_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_ram);
}

void mbc200_state::main_opcodes(address_map &map)
{
	// set up on reset
}

void mbc200_state::main_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	//map(0xe0, 0xe1).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xe0, 0xe1).mirror(0x02).r(FUNC(mbc200_state::keyboard_r)).nopw();
	map(0xe4, 0xe7).rw(m_fdc, FUNC(mb8876_device::read), FUNC(mb8876_device::write));
	map(0xe8, 0xeb).rw(m_ppi_m, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xec, 0xed).mirror(0x02).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
}


void mbc200_state::sub_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram();
	//map(0x4000, 0x7fff).bankr(...); missing dumps of MB83256 ROMs at 13J, 11J, 10J and 12J
	map(0x8000, 0xffff).ram().share("vram");
}

void mbc200_state::sub_io(address_map &map)
{
	// I/O decoding is sloppy:
	// A7 low selects PPI 9F (read/write)
	// A6 low selects CRTC 11D (write only)
	// A5 low selects main CPU communication (read/write)
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(mbc200_state::sub_io_r), FUNC(mbc200_state::sub_io_w));
}



u8 mbc200_state::boot_m1_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0xffff, &m_ram[0]);
		m_maincpu->space(AS_OPCODES).install_rom(0x0000, 0xffff, &m_ram[0]);
	}
	return m_ram[0x8000 | offset];
}


u8 mbc200_state::sub_io_r(offs_t offset)
{
	u8 result = 0xff;

	if (!BIT(offset, 7))
		result &= m_ppi_s->read(offset & 0x03);

	if (!BIT(offset, 5))
		result &= m_ppi_m->acka_r();

	return result;
}

void mbc200_state::sub_io_w(offs_t offset, u8 data)
{
	if (!BIT(offset, 7))
		m_ppi_s->write(offset & 0x03, data);

	if (!BIT(offset, 6))
	{
		if (BIT(offset, 0))
			m_crtc->register_w(data);
		else
			m_crtc->address_w(data);
	}

	if (!BIT(offset, 5))
	{
		m_comm_data = data;
		m_ppi_m->pc4_w(0);
		m_ppi_m->pc4_w(1);
	}
}


u8 mbc200_state::ps_porta_r()
{
	return m_ps_porta;
}

template <unsigned Bit>
void mbc200_state::ps_porta_w(int state)
{
	if (state)
		m_ps_porta |= u8(1) << Bit;
	else
		m_ps_porta &= ~(u8(1) << Bit);
}

void mbc200_state::ps_portc_w(u8 data)
{
	// FIXME: PC0, PC1, PC2 select ROM bank

	m_pm_portc = BIT(data, 3) ? 0xff : 0xfe;

	m_cpu_s_sound = BIT(data, 4); // used by beep command in basic
	m_speaker->level_w(m_cpu_m_sound + m_cpu_s_sound);

	m_printer->write_init(BIT(data, 6));
	m_printer->write_strobe(BIT(~data, 7));
}

u8 mbc200_state::pm_porta_r()
{
	// Gets whatever happens to be on the slave CPU's data bus.
	// However, we know this only happens on I/O writes with A5 low.
	return m_comm_data;
}

u8 mbc200_state::pm_portc_r()
{
	return m_pm_portc;
}

// Writing to PPI port B ($E9).  Being programmed for output, read operations will get the current value.
void mbc200_state::pm_portb_w(u8 data)
{
	ps_porta_w<5>(BIT(data, 0));

	m_cpu_m_sound = BIT(data, 1); // key-click
	m_speaker->level_w(m_cpu_m_sound + m_cpu_s_sound);

	// The BIOS supports up to 4 drives, (2 internal + 2 external)
	// E: and F: are virtual swaps of A: and B:
	u8 const drivenum = (data & 0x70) >> 4;
	floppy_image_device *floppy = nullptr;
	if (drivenum < 4)
		floppy = m_floppy[drivenum]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 7));
	}
}

void mbc200_state::pm_portc_w(u8 data)
{
	ps_porta_w<6>(BIT(data, 5)); // IBF
	ps_porta_w<7>(BIT(data, 7)); // /OBF
}


/* Input ports */
static INPUT_PORTS_START( mbc200 )
INPUT_PORTS_END

u8 mbc200_state::keyboard_r(offs_t offset)
{
	u8 data = 0;
	if (offset)
	{
		if (m_term_data)
		{
			data = 2;
			// handle CTRL key pressed
			if (m_term_data < 0x20)
			{
				data |= 8;
				m_term_data |= 0x40;
			}
		}
	}
	else
	{
		data = m_term_data;
		m_term_data = 0;
	}

	return data;
}

// convert standard control keys to expected code;
void mbc200_state::kbd_put(u8 data)
{
	switch (data)
	{
		case 0x0e:
			m_term_data = 0xe2;
			break;
		case 0x0f:
			m_term_data = 0xe3;
			break;
		case 0x08:
			m_term_data = 0xe4;
			break;
		case 0x09:
			m_term_data = 0xe5;
			break;
		case 0x0a:
			m_term_data = 0xe6;
			break;
		case 0x0d:
			m_term_data = 0xe7;
			break;
		case 0x1b:
			m_term_data = 0xe8;
			break;
		default:
			m_term_data = data;
	}
}

void mbc200_state::machine_start()
{
	save_item(NAME(m_cpu_m_sound));
	save_item(NAME(m_cpu_s_sound));
	save_item(NAME(m_pm_portc));
	save_item(NAME(m_ps_porta));
	save_item(NAME(m_comm_data));
	save_item(NAME(m_term_data));
}

void mbc200_state::machine_reset()
{
	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x0000, 0x7fff);
	m_maincpu->space(AS_PROGRAM).install_rom(0x0000, 0x0fff, 0x7000, &m_rom[0]);
	m_maincpu->space(AS_OPCODES).install_rom(0x0000, 0x0fff, 0x7000, &m_rom[0]);
	m_maincpu->space(AS_OPCODES).install_read_handler(0x8000, 0xffff, emu::rw_delegate(*this, FUNC(mbc200_state::boot_m1_r)));
}

static void mbc200_floppies(device_slot_interface &device)
{
	device.option_add("qd", FLOPPY_525_QD);
}

MC6845_UPDATE_ROW( mbc200_state::update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = (ma+x)*4+ra;
		u8 gfx = m_vram[mem & 0x7fff];
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

static const gfx_layout charlayout =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_mbc200 )
	GFXDECODE_ENTRY( "subcpu", 0x1800, charlayout, 0, 1 )
GFXDECODE_END


void mbc200_state::mbc200(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 4); // NEC D780C-1
	m_maincpu->set_addrmap(AS_PROGRAM, &mbc200_state::main_mem);
	m_maincpu->set_addrmap(AS_IO, &mbc200_state::main_io);
	m_maincpu->set_addrmap(AS_OPCODES, &mbc200_state::main_opcodes);

	z80_device &subcpu(Z80(config, "subcpu", 16_MHz_XTAL / 4)); // NEC D780C-1
	subcpu.set_addrmap(AS_PROGRAM, &mbc200_state::sub_mem);
	subcpu.set_addrmap(AS_IO, &mbc200_state::sub_io);

	config.set_perfect_quantum(m_maincpu); // lazy way to keep CPUs in sync

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 816, 0, 640, 420, 0, 400);
	screen.set_screen_update(m_crtc, FUNC(hd6845s_device::screen_update));
	screen.set_color(rgb_t::green());
	GFXDECODE(config, "gfxdecode", m_palette, gfx_mbc200);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8); // HD46505SP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mbc200_state::update_row));

	// sound
	SPEAKER(config, "mono").front_center();
	static const double speaker_levels[4] = { 0.0, 0.6, 1.0 };
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
	m_speaker->set_levels(3, speaker_levels);

	I8255(config, m_ppi_m);
	m_ppi_m->in_pa_callback().set(FUNC(mbc200_state::pm_porta_r));
	m_ppi_m->in_pc_callback().set(FUNC(mbc200_state::pm_portc_r));
	m_ppi_m->out_pb_callback().set(FUNC(mbc200_state::pm_portb_w));
	m_ppi_m->out_pc_callback().set(FUNC(mbc200_state::pm_portc_w));

	I8255(config, m_ppi_s).out_pc_callback();
	m_ppi_s->in_pa_callback().set(FUNC(mbc200_state::ps_porta_r));
	m_ppi_s->out_pb_callback().set("printdata", FUNC(output_latch_device::write));
	m_ppi_s->out_pc_callback().set(FUNC(mbc200_state::ps_portc_w));

	i8251_device &uart1(I8251(config, "uart1", 0)); // INS8251N
	//uart1.txd_handler().set(...); to keyboard
	uart1.rts_handler().set("uart1", FUNC(i8251_device::write_cts));

	i8251_device &uart2(I8251(config, "uart2", 0)); // INS8251A
	uart2.txd_handler().set("rs232c", FUNC(rs232_port_device::write_txd));
	uart2.rts_handler().set("rs232c", FUNC(rs232_port_device::write_rts));
	uart2.dtr_handler().set("rs232c", FUNC(rs232_port_device::write_dtr));

	MB8876(config, m_fdc, 16_MHz_XTAL / 16);

	FLOPPY_CONNECTOR(config,  m_floppy[0], mbc200_floppies, "qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config,  m_floppy[1], mbc200_floppies, "qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config,  m_floppy[2], mbc200_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config,  m_floppy[3], mbc200_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// printer
	CENTRONICS(config, m_printer, centronics_devices, "printer");
	m_printer->set_output_latch(OUTPUT_LATCH(config, "printdata"));
	m_printer->busy_handler().set(FUNC(mbc200_state::ps_porta_w<2>)).invert();
	m_printer->perror_handler().set(FUNC(mbc200_state::ps_porta_w<3>));
	m_printer->select_handler().set(FUNC(mbc200_state::ps_porta_w<4>));

	// RS-232C
	rs232_port_device &rs232c(RS232_PORT(config, "rs232c", default_rs232_devices, nullptr));
	rs232c.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232c.cts_handler().set("uart2", FUNC(i8251_device::write_cts));
	rs232c.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));

	// keyboard
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(mbc200_state::kbd_put));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("mbc200");
}

/* ROM definition */
ROM_START( mbc200 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "d2732a.bin",  0x0000, 0x1000, CRC(bf364ce8) SHA1(baa3a20a5b01745a390ef16628dc18f8d682d63b))

	ROM_REGION( 0x2000, "subcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m5l2764.bin", 0x0000, 0x2000, CRC(377300a2) SHA1(8563172f9e7f84330378a8d179f4138be5fda099))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1982, mbc200, 0,      0,      mbc200,  mbc200, mbc200_state, empty_init, "Sanyo", "MBC-200", MACHINE_SUPPORTS_SAVE )
