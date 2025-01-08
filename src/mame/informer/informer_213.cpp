// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 213 (IBM 374/SNA V.22)
    Informer 213 AE (VT-100)

    Hardware:
    - EF68B09EP
    - 2x TC5564PL-15 [8k] (next to CG ROM)
    - 1x TC5565APL-15 [8k] + 2x TMS4464-15NL [32k] (next to CPU)
    - Z0853006PSC SCC
    - ASIC (INFORMER 44223)
    - 18.432 MHz XTAL

    TODO:
    - Figure out the ASIC and how it's connected

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "machine/z80scc.h"
#include "informer_213_kbd.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/printer.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class informer_213_state : public driver_device
{
public:
	informer_213_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram%u", 0U),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_scc(*this, "scc"),
		m_beep(*this, "beep"),
		m_nvram_bank(*this, "banked"),
		m_vram(*this, "vram"),
		m_aram(*this, "aram"),
		m_chargen(*this, "chargen")
	{ }

	void informer_213(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<nvram_device, 2> m_nvram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scc8530_device> m_scc;
	required_device<beep_device> m_beep;
	required_memory_bank m_nvram_bank;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_aram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);
	void vram_start_addr_w(offs_t offset, uint8_t data);
	void vram_end_addr_w(offs_t offset, uint8_t data);
	void vram_start_addr2_w(offs_t offset, uint8_t data);
	void cursor_addr_w(offs_t offset, uint8_t data);
	void cursor_start_w(uint8_t data);
	void cursor_end_w(uint8_t data);
	void screen_ctrl_w(uint8_t data);

	void bell_w(uint8_t data);

	void kbd_int_w(int state);
	uint8_t firq_vector_r();

	std::unique_ptr<uint8_t[]> m_banked_ram;

	uint16_t m_vram_start_addr;
	uint16_t m_vram_end_addr;
	uint16_t m_vram_start_addr2;
	uint8_t m_cursor_start;
	uint8_t m_cursor_end;
	uint16_t m_cursor_addr;
	uint8_t m_screen_ctrl;
	uint8_t m_firq_vector;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void informer_213_state::mem_map(address_map &map)
{
	map(0x0000, 0x0000).rw("kbd", FUNC(informer_213_kbd_hle_device::read), FUNC(informer_213_kbd_hle_device::write));
	map(0x0006, 0x0007).unmapr().w(FUNC(informer_213_state::vram_start_addr_w));
	map(0x0008, 0x0009).unmapr().w(FUNC(informer_213_state::vram_end_addr_w));
	map(0x000a, 0x000b).unmapr().w(FUNC(informer_213_state::vram_start_addr2_w));
	map(0x000d, 0x000d).unmapr().w(FUNC(informer_213_state::cursor_start_w));
	map(0x000e, 0x000e).unmapr().w(FUNC(informer_213_state::cursor_end_w));
	map(0x000f, 0x0010).unmapr().w(FUNC(informer_213_state::cursor_addr_w));
	map(0x0021, 0x0021).w(FUNC(informer_213_state::screen_ctrl_w));
	map(0x0040, 0x0043).rw(m_scc, FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	map(0x0060, 0x0060).w(FUNC(informer_213_state::bell_w));
	map(0x0100, 0x03ff).ram().share("nvram0"); // might not be all nvram
	map(0x0400, 0x07ff).bankrw("banked"); // unknown size, data written 0x540 to 0x749
	map(0x0800, 0x5fff).ram();
	map(0x6000, 0x6fff).ram().share("vram");
	map(0x7000, 0x7fff).ram().share("aram");
	map(0x8000, 0xffff).rom().region("maincpu", 0);
	map(0xfff7, 0xfff7).r(FUNC(informer_213_state::firq_vector_r));
	map(0xfff9, 0xfff9).lr8(NAME([this] () { return m_scc->m1_r(); })); // irq vector
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( informer_213 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t informer_213_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t chargen_base = (m_chargen.bytes() == 0x4000) ? 0x2000 : 0;

	// line at which vram splits into two windows
	int split = 24 - ((m_vram_start_addr2 - m_vram_start_addr) / 80);

	for (int y = 0; y < 26; y++)
	{
		uint8_t line_attr = 0;

		for (int x = 0; x < 80; x++)
		{
			uint16_t addr;

			// vram is split into 3 areas: display window 1, display window 2, status bar
			if (y >= 24)
				addr = 0x6000 + (y - 24) * 80 + x;
			else if (y >= split)
				addr = m_vram_start_addr + (y - split) * 80 + x;
			else
				addr = m_vram_start_addr2 + y * 80 + x;

			uint8_t code = m_program.read_byte(addr);
			uint8_t attr = m_program.read_byte(addr+0x1000);

			//logerror("%02d.%02d %04x %02x %02x\n", x, y, addr, code, attr);

			if (code == 0xc0 || code == 0xe8)
				line_attr = attr;

			// draw 9 lines
			for (int i = 0; i < 9; i++)
			{
				uint8_t data = m_chargen[chargen_base | ((code << 4) + i)];

				// conceal
				if (line_attr & 0x08 || attr & 0x08)
					data = 0x00;

				// reverse video
				if (line_attr & 0x10 || attr & 0x10)
					data ^= 0xff;

				// underline (not supported by terminal?)
				if (line_attr & 0x20 || attr & 0x20)
					data = +data;

				// blink (todo: timing)
				if (line_attr & 0x40 || attr & 0x40)
					data = m_screen->frame_number() & 0x20 ? 0x00 :data;

				if (code == 0xc0 || code == 0xe8)
					data = 0;

				if (BIT(m_screen_ctrl, 5))
					data ^= 0xff;

				// cursor
				if (BIT(m_cursor_start, 5) == 1 && addr == m_cursor_addr && y < 24)
					if (i >= (m_cursor_start & 0x0f) && i < (m_cursor_end & 0x0f))
						if (!(BIT(m_screen_ctrl, 4) == 0 && (m_screen->frame_number() & 0x20)))
							data = 0xff;

				// 6 pixels of the character
				bitmap.pix(y * 9 + i, x * 6 + 0) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 9 + i, x * 6 + 1) = BIT(data, 6) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 9 + i, x * 6 + 2) = BIT(data, 5) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 9 + i, x * 6 + 3) = BIT(data, 4) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 9 + i, x * 6 + 4) = BIT(data, 3) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y * 9 + i, x * 6 + 5) = BIT(data, 2) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

void informer_213_state::vram_start_addr_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_vram_start_addr = (m_vram_start_addr & 0xff00) | (data << 0);
	else
		m_vram_start_addr = (m_vram_start_addr & 0x00ff) | (data << 8);

	if (offset)
		logerror("vram_start_addr_w: %04x\n", m_vram_start_addr);
}

void informer_213_state::vram_end_addr_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_vram_end_addr = (m_vram_end_addr & 0xff00) | (data << 0);
	else
		m_vram_end_addr = (m_vram_end_addr & 0x00ff) | (data << 8);

	if (offset)
		logerror("vram_end_addr_w: %04x\n", m_vram_end_addr);
}

void informer_213_state::vram_start_addr2_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_vram_start_addr2 = (m_vram_start_addr2 & 0xff00) | (data << 0);
	else
		m_vram_start_addr2 = (m_vram_start_addr2 & 0x00ff) | (data << 8);

	if (offset)
		logerror("vram_start_addr2_w: %04x\n", m_vram_start_addr2);
}

void informer_213_state::cursor_start_w(uint8_t data)
{
	logerror("cursor_start_w: %02x\n", data);

	// 76------  unknown
	// --5-----  cursor visible
	// ---4----  unknown
	// ----3210  cursor starting line, values seen: 0 (block/off), 6 (underline)

	m_cursor_start = data;
}

void informer_213_state::cursor_end_w(uint8_t data)
{
	logerror("cursor_end_w: %02x\n", data);

	// 7654----  unknown
	// ----3210  cursor ending line, values seen: 9 (block), 8 (underline), 1 (off)

	m_cursor_end = data;
}

void informer_213_state::cursor_addr_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_cursor_addr = (m_cursor_addr & 0xff00) | (data << 0);
	else
		m_cursor_addr = (m_cursor_addr & 0x00ff) | (data << 8);

	if (offset)
		logerror("cursor_addr_w: %04x\n", m_cursor_addr);
}

void informer_213_state::screen_ctrl_w(uint8_t data)
{
	logerror("screen_ctrl_w: %02x\n", data);

	// 76------  unknown
	// --5-----  screen reverse
	// ---4----  cursor blink/steady
	// ----3210  unknown

	m_screen_ctrl = data;
}

void informer_213_state::vblank_w(int state)
{
	if (state)
	{
		// real source if this interrupt is unknown, deactivated for now
#if 0
		m_firq_vector = 0x10;
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
#endif
	}
}

static const gfx_layout char_layout =
{
	6,9,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void informer_213_state::bell_w(uint8_t data)
{
	logerror("bell_w: %02x\n", data);

	// 7654----  unknown
	// ----3---  ram bank
	// -----2--  beeper
	// ------10  unknown

	m_beep->set_state(BIT(data, 2));
	m_nvram_bank->set_entry(BIT(data, 3));
}

void informer_213_state::kbd_int_w(int state)
{
	m_firq_vector = 0x14;
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

uint8_t informer_213_state::firq_vector_r()
{
	uint8_t tmp = m_firq_vector;
	m_firq_vector = 0x00;

	return tmp;
}

void informer_213_state::machine_start()
{
	m_banked_ram = make_unique_clear<uint8_t[]>(0x800);
	membank("banked")->configure_entries(0, 2, m_banked_ram.get(), 0x400);

	m_nvram[1]->set_base(m_banked_ram.get(), 0x800);

	m_maincpu->space(AS_PROGRAM).specific(m_program);

	// register for save states
	save_item(NAME(m_vram_start_addr));
	save_item(NAME(m_vram_start_addr2));
	save_item(NAME(m_vram_end_addr));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_screen_ctrl));
	save_item(NAME(m_firq_vector));
	save_item(NAME(m_cursor_start));
	save_item(NAME(m_cursor_end));
}

void informer_213_state::machine_reset()
{
	m_firq_vector = 0x00;
	m_vram_start_addr = 0x0084;
	m_vram_start_addr2 = 0x0084;
	m_vram_end_addr = 0xffff;
	m_cursor_start = 0x20;
	m_cursor_end = 0x09;
	m_cursor_addr = 0xffff;
	m_screen_ctrl = 0x18;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void in213_printer_devices(device_slot_interface &device)
{
	device.option_add("printer", SERIAL_PRINTER);
}

void informer_213_state::informer_213(machine_config &config)
{
	MC6809(config, m_maincpu, 18.432_MHz_XTAL / 4); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &informer_213_state::mem_map);

	NVRAM(config, m_nvram[0], nvram_device::DEFAULT_ALL_0);
	NVRAM(config, m_nvram[1], nvram_device::DEFAULT_ALL_0);

	SCC8530N(config, m_scc, 18.432_MHz_XTAL / 5);
	m_scc->out_txda_callback().set("host", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set("host", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set("host", FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_int_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);

	rs232_port_device &rs232_host(RS232_PORT(config, "host", default_rs232_devices, nullptr));
	rs232_host.rxd_handler().set(m_scc, FUNC(scc85c30_device::rxa_w));
	rs232_host.dcd_handler().set(m_scc, FUNC(scc85c30_device::dcda_w));
	rs232_host.cts_handler().set(m_scc, FUNC(scc85c30_device::ctsa_w));

	RS232_PORT(config, "printer", in213_printer_devices, nullptr);

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_color(rgb_t::amber());
	m_screen->set_size(480, 234);
	m_screen->set_visarea_full();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
//  m_screen->set_raw(18.432_MHz_XTAL, 0, 0, 0, 0, 0, 0);
	m_screen->set_screen_update(FUNC(informer_213_state::screen_update));
	m_screen->screen_vblank().set(FUNC(informer_213_state::vblank_w));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	// sound
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beep", 500).add_route(ALL_OUTPUTS, "mono", 0.50); // frequency unknown

	informer_213_kbd_hle_device &kbd(INFORMER_213_KBD_HLE(config, "kbd"));
	kbd.int_handler().set(FUNC(informer_213_state::kbd_int_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( in213 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_DEFAULT_BIOS("26")
	ROM_SYSTEM_BIOS(0,  "21",  "v2.1")
	// 79687-101  213 SNA 201C  CK=1C22 V2.1 (checksum matches)
	ROMX_LOAD("79687-101.bin", 0x0000, 0x8000, CRC(1ff023f3) SHA1(cbb027769d7744072045e60b020826f4f4bfe1b6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1,  "26",  "v2.6")
	// 79687-305  PTF02 SNA  V2.6 CK=24EE (checksum matches)
	ROMX_LOAD("79687-305.bin", 0x0000, 0x8000, CRC(0638c6d6) SHA1(1906f835f255d595c5743b453614ba21acb5acae), ROM_BIOS(1))

	ROM_REGION(0x2000, "chargen", 0)
	// 79688-003  ICT 213/CG.  CK=C4E0 (checksum matches)
	// 79688-003  ICT 213/374  CK = C4E0 (checksum matches)
	ROM_LOAD("79688-003.bin", 0x0000, 0x2000, CRC(75e0da94) SHA1(c10c71fcf980a5f868a85bc264661183fa69fa72))
ROM_END

ROM_START( in213ae )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_DEFAULT_BIOS("17")
	ROM_SYSTEM_BIOS(0,  "16",  "v1.6")
	// 79750-304-213AE_V1.6_CK-B1B8
	ROMX_LOAD("79750-304.bin", 0x0000, 0x8000, CRC(82ffe69e) SHA1(3803100aeb8f5e484bc9f4c533ef4f25223c9023), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1,  "17",  "v1.7")
	// 79750-305  213 AE V.32  CK=9D06 (checksum matches)
	ROMX_LOAD("79750-305.bin", 0x0000, 0x8000, CRC(322edc85) SHA1(7e8d95ef8550d133163ffc89dd75ed80883dee0c), ROM_BIOS(1))

	ROM_REGION(0x4000, "chargen", 0)
	// 79747-002  V.32 ME C.G.  V3.1 CK=D68C (checksum matches)
	ROM_LOAD("79747-002.bin", 0x0000, 0x4000, CRC(7425327f) SHA1(e3e67305b3b8936683724d1347a451fffe96bf0e))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT   COMPAT  MACHINE       INPUT         CLASS               INIT        COMPANY     FULLNAME           FLAGS
COMP( 1990, in213  , 0,       0,      informer_213, informer_213, informer_213_state, empty_init, "Informer", "Informer 213",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1992, in213ae, 0,       0,      informer_213, informer_213, informer_213_state, empty_init, "Informer", "Informer 213 AE", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
