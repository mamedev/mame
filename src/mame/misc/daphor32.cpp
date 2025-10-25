// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*******************************************************************************

    Daphor 32

    TODO:
    - everything, no schematics available
    - interrupts
    - fix the keyboard, using the ROM to lookup codes
    - somehow use EF9364 for cursor

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/pit8253.h"
#include "video/ef9364.h"

#include "screen.h"


namespace {

class daphor_state : public driver_device
{
public:
	daphor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//, m_ef9364(*this, "ef9364")
		, m_videoram(*this, "videoram")
		, m_palette(*this, "palette")
	{ }

	void daphor32(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ppi2_portb_w(uint8_t data);
	void ppi3_portb_w(uint8_t data);

	uint8_t ppi1_porta_r();
	uint8_t ppi1_portc_r();
	void ppi1_portb_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);
	void ppi2_portc_w(uint8_t data);
	void ppi3_portc_w(uint8_t data);

	void kbd_put(uint8_t data);

	//required_device<ef9364_device> m_ef9364;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<palette_device> m_palette;

	uint8_t m_ppi_c;

	uint8_t m_kbd_data;
};


void daphor_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x8fff).ram();
	map(0xe000, 0xffff).ram().share(m_videoram);
}

void daphor_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x88, 0x8b).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x90, 0x93).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x98, 0x9b).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa3).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


void daphor_state::kbd_put(uint8_t data)
{
	// assert strobe
	m_kbd_data = data | 0x80;
}


uint8_t daphor_state::ppi1_porta_r()
{
	return m_kbd_data;
}

void daphor_state::ppi1_portb_w(uint8_t data)
{
	logerror("%s ppi1_portb_w: %02x\n", machine().describe_context(), data);
}

uint8_t daphor_state::ppi1_portc_r()
{
	m_ppi_c ^= 0x01; // FIXME: what is this, maybe VSYNC?
	logerror("%s ppi1_portc_r: %02x\n", machine().describe_context(), m_ppi_c);

	return m_ppi_c;
}

void daphor_state::ppi1_portc_w(uint8_t data)
{
	logerror("%s ppi1_portc_w: %02x\n", machine().describe_context(), data);
}


void daphor_state::ppi2_portb_w(uint8_t data)
{
	logerror("%s ppi2_portb_w: %02x\n", machine().describe_context(), data);
}

void daphor_state::ppi2_portc_w(uint8_t data)
{
	logerror("%s ppi2_portc_w: %02x\n", machine().describe_context(), data);
}


void daphor_state::ppi3_portb_w(uint8_t data)
{
	logerror("%s ppi3_portb_w: %02x\n", machine().describe_context(), data);
}

void daphor_state::ppi3_portc_w(uint8_t data)
{
	logerror("%s ppi3_portc_w: %02x\n", machine().describe_context(), data);
}


void daphor_state::machine_start()
{
	m_ppi_c = 0;
	m_kbd_data = 0x00;

	save_item(NAME(m_kbd_data));
}


uint32_t daphor_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	for (int y = 0; y < 200; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 32; x++)
		{
			uint8_t const pixels = m_videoram[(y * 32) + x];

			*scanline++ = palette[BIT(pixels, 0)];
			*scanline++ = palette[BIT(pixels, 1)];
			*scanline++ = palette[BIT(pixels, 2)];
			*scanline++ = palette[BIT(pixels, 3)];
			*scanline++ = palette[BIT(pixels, 4)];
			*scanline++ = palette[BIT(pixels, 5)];
			*scanline++ = palette[BIT(pixels, 6)];
			*scanline++ = palette[BIT(pixels, 7)];
		}
	}

	return 0;
}


void daphor_state::daphor32(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", 4_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &daphor_state::mem_map);
	maincpu.set_addrmap(AS_IO, &daphor_state::io_map);

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("computer", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("computer", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("computer", FUNC(rs232_port_device::write_rts));

	rs232_port_device &computer(RS232_PORT(config, "computer", default_rs232_devices, nullptr));
	computer.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	computer.cts_handler().set("uart", FUNC(i8251_device::write_cts));
	computer.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));

	pit8253_device &pit(PIT8253(config, "pit"));
	pit.out_handler<0>().set("uart", FUNC(i8251_device::write_txc));
	pit.out_handler<1>().set("uart", FUNC(i8251_device::write_rxc));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set(FUNC(daphor_state::ppi1_porta_r)); // keyboard
	ppi1.out_pb_callback().set(FUNC(daphor_state::ppi1_portb_w));
	ppi1.in_pc_callback().set(FUNC(daphor_state::ppi1_portc_r));
	ppi1.out_pc_callback().set(FUNC(daphor_state::ppi1_portc_w));

	i8255_device &ppi2(I8255(config, "ppi2"));
	//ppi2.out_pa_callback().set_log("PPI2 - unmapped write port A");
	ppi2.out_pb_callback().set(FUNC(daphor_state::ppi2_portb_w));
	//ppi2.in_pc_callback().set_log("PPI2 - unmapped read port C");
	ppi2.out_pc_callback().set(FUNC(daphor_state::ppi2_portc_w));

	i8255_device &ppi3(I8255(config, "ppi3"));
	ppi3.out_pa_callback().set([this](uint8_t data) { logerror("%s ppi3 - unmapped write port A %02x\n", machine().describe_context(), data); });
	ppi3.out_pb_callback().set(FUNC(daphor_state::ppi3_portb_w));
	//ppi3.in_pc_callback().set_log("ppi3 - unmapped read port C");
	ppi3.out_pc_callback().set(FUNC(daphor_state::ppi3_portc_w));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(daphor_state::kbd_put));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(daphor_state::screen_update));
	//screen.set_screen_update("ef9364", FUNC(ef9364_device::screen_update));
	screen.set_size(32 * 8, 25 * 8);
	screen.set_visarea(0, 32 * 8 - 1, 0, 25 * 8 - 1);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	//EF9364(config, m_ef9364, 1.008_MHz_XTAL);
	//m_ef9364->set_screen("screen");
	//m_ef9364->set_palette_tag("palette");
	//m_ef9364->set_nb_of_pages(1);
}


ROM_START(daphor32)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("1_d32.m1", 0x0000, 0x1000, CRC(b72a2cfc) SHA1(e56c1bc5b6c81dd0d5ec7d651332f0035a11d06b))
	ROM_LOAD("2_d32.m2", 0x1000, 0x1000, CRC(6d18ae5b) SHA1(bdccf5eaa98050d2557d73397a27287da0ac01cd))
	ROM_LOAD("3_d32.m3", 0x2000, 0x1000, CRC(076510dd) SHA1(7d1f26f6833da663b73d7572f46870268e33d688))
	ROM_LOAD("4_d32.m4", 0x3000, 0x1000, CRC(d9f8f700) SHA1(62d44ef8db0bcf725aa065e0145b6f64ad19f545))
	ROM_LOAD("5_d32.m5", 0x4000, 0x2000, CRC(fe18f9e0) SHA1(a49949e380b230d6e1fac9b95b93831376dadf19))

	ROM_REGION(0x800, "ef9364", ROMREGION_ERASE00)

	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD("k.bin", 0x0000, 0x1000, CRC(728227f0) SHA1(b488f269da08cfd3aa3f5cd631506f63a7822b9b))

	ROM_REGION(0x0800, "prom", 0)
	ROM_LOAD("63s281.bin", 0x0000, 0x0800, CRC(32a86e40) SHA1(0dff63d8ed33f6b8cce6f342ef6a40065323a6d5))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT  CLASS          INIT         COMPANY          FULLNAME      FLAGS
COMP( 1983, daphor32, 0,      0,      daphor32,  0,     daphor_state,  empty_init,  "Dasit S.P.A.",  "Daphor 32",  MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
