// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Konami Ultra Sports hardware

    Driver by Ville Linde
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "machine/eepromser.h"
#include "machine/upd4701.h"
#include "sound/k054539.h"
#include "sound/k056800.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class ultrsprt_state : public driver_device
{
public:
	ultrsprt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056800(*this, "k056800"),
		m_workram(*this, "workram"),
		m_palette(*this, "palette"),
		m_eeprom(*this, "eeprom"),
		m_upd(*this, "upd%u", 1),
		m_vrambank(*this, "vram"),
		m_service(*this, "SERVICE")
	{ }

	void ultrsprt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static const u32 VRAM_PAGES      = 2;
	static const u32 VRAM_PAGE_BYTES = 512 * 1024;

	required_device<ppc_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k056800_device> m_k056800;
	required_shared_ptr<u32> m_workram;
	required_device<palette_device> m_palette;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device_array<upd4701_device, 2> m_upd;

	required_memory_bank m_vrambank;

	required_ioport m_service;

	u8 eeprom_r();
	void eeprom_w(u8 data);
	u16 upd1_r(offs_t offset);
	u16 upd2_r(offs_t offset);
	void int_ack_w(u32 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	std::unique_ptr<u32[]> m_vram;
	u32 m_cpu_vram_page;
};


/*****************************************************************************/

u32 ultrsprt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto const vram = util::big_endian_cast<u8 const>(m_vram.get()) + (m_cpu_vram_page ^ 1) * VRAM_PAGE_BYTES;

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		int fb_index = y * 1024;
		u16 *dest = &bitmap.pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			u8 const p1 = vram[fb_index + x + 512];

			if (p1 == 0)
				*dest++ = vram[fb_index + x];
			else
				*dest++ = 0x100 + p1;
		}
	}

	return 0;
}


/*****************************************************************************/

void ultrsprt_state::int_ack_w(u32 data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}


u8 ultrsprt_state::eeprom_r()
{
	return m_service->read();
}

void ultrsprt_state::eeprom_w(u8 data)
{
	/*
	    .... ...x - EEPROM DI
	    .... ..x. - EEPROM CLK
	    .... .x.. - EEPROM /CS
	    .... x... - VRAM page (CPU access)
	    ...x .... - Coin counter
	    ..x. .... - Watchdog /Reset
	    .x.. .... - Trackball /Reset
	    x... .... - Sound CPU /Reset
	*/
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->clk_write(!BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 2));

	u32 vram_page = (data & 0x08) >> 3;
	if (vram_page != m_cpu_vram_page)
	{
		m_vrambank->set_entry(vram_page);
		m_cpu_vram_page = vram_page;
	}

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	for (auto &upd : m_upd)
	{
		upd->resetx_w(!BIT(data, 6));
		upd->resety_w(!BIT(data, 6));
	}
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}

u16 ultrsprt_state::upd1_r(offs_t offset)
{
	return m_upd[0]->read_xy(offset * 2) | (m_upd[0]->read_xy(offset * 2 + 1) << 8);
}

u16 ultrsprt_state::upd2_r(offs_t offset)
{
	return m_upd[1]->read_xy(offset * 2) | (m_upd[1]->read_xy(offset * 2 + 1) << 8);
}

/*****************************************************************************/

void ultrsprt_state::main_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).bankrw(m_vrambank);
	map(0x70000000, 0x70000000).rw(FUNC(ultrsprt_state::eeprom_r), FUNC(ultrsprt_state::eeprom_w));
	map(0x70000020, 0x70000023).r(FUNC(ultrsprt_state::upd1_r));
	map(0x70000040, 0x70000043).r(FUNC(ultrsprt_state::upd2_r));
	map(0x70000080, 0x7000008f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x700000c0, 0x700000cf).nopw(); // Written following DMA interrupt - unused int ack?
	map(0x700000e0, 0x700000e3).w(FUNC(ultrsprt_state::int_ack_w));
	map(0x7f000000, 0x7f01ffff).ram().share(m_workram);
	map(0x7f700000, 0x7f703fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x7f800000, 0x7f9fffff).mirror(0x00600000).rom().region("program", 0);
}


/*****************************************************************************/

void ultrsprt_state::sound_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();
	map(0x00100000, 0x00101fff).ram();
	map(0x00200000, 0x0020000f).rw(m_k056800, FUNC(k056800_device::sound_r), FUNC(k056800_device::sound_w));
	map(0x00400000, 0x004002ff).rw("k054539", FUNC(k054539_device::read), FUNC(k054539_device::write));
}


/*****************************************************************************/

static INPUT_PORTS_START( ultrsprt )
	PORT_START("P1X")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("P1Y")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_PLAYER(1)

	PORT_START("P2X")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("P2Y")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(80) PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd1", FUNC(upd4701_device::left_w))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER("upd1", FUNC(upd4701_device::right_w))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd1", FUNC(upd4701_device::middle_w))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd2", FUNC(upd4701_device::left_w))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER("upd2", FUNC(upd4701_device::right_w))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd2", FUNC(upd4701_device::middle_w))

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) // VRAM page flip status?
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
INPUT_PORTS_END


/*****************************************************************************/

void ultrsprt_state::machine_start()
{
	// set conservative DRC options
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	// configure fast RAM regions for DRC
	m_maincpu->ppcdrc_add_fastram(0xff000000, 0xff01ffff, false, m_workram);

	m_vram = std::make_unique<u32[]>(VRAM_PAGE_BYTES / sizeof(u32) * VRAM_PAGES);

	m_vrambank->configure_entries(0, VRAM_PAGES, m_vram.get(), VRAM_PAGE_BYTES);

	save_pointer(NAME(m_vram), VRAM_PAGE_BYTES / sizeof(u32) * VRAM_PAGES);
	save_item(NAME(m_cpu_vram_page));
}

void ultrsprt_state::machine_reset()
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_cpu_vram_page = 0;
	m_vrambank->set_entry(m_cpu_vram_page);
}


/*****************************************************************************/

void ultrsprt_state::ultrsprt(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ultrsprt_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(ultrsprt_state::irq1_line_assert));

	M68000(config, m_audiocpu, 8000000); // Unconfirmed
	m_audiocpu->set_addrmap(AS_PROGRAM, &ultrsprt_state::sound_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	UPD4701A(config, m_upd[0]);
	m_upd[0]->set_portx_tag("P1X");
	m_upd[0]->set_porty_tag("P1Y");

	UPD4701A(config, m_upd[1]);
	m_upd[1]->set_portx_tag("P2X");
	m_upd[1]->set_porty_tag("P2Y");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60); // TODO: Determine correct timings
	screen.set_size(640, 480);
	screen.set_visarea(0, 511, 0, 399);
	screen.set_screen_update(FUNC(ultrsprt_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 8192);

	// sound hardware
	K056800(config, m_k056800, XTAL(18'432'000));
	m_k056800->int_callback().set_inputline(m_audiocpu, M68K_IRQ_6);

	SPEAKER(config, "speaker", 2).front();

	k054539_device &k054539(K054539(config, "k054539", XTAL(18'432'000)));
	k054539.timer_handler().set_inputline("audiocpu", M68K_IRQ_5);
	k054539.add_route(0, "speaker", 1.0, 0);
	k054539.add_route(1, "speaker", 1.0, 1);
}


/*****************************************************************************/

ROM_START( fiveside )
	ROM_REGION32_BE(0x200000, "program", 0) // PowerPC program ROMs
	ROM_LOAD32_BYTE("479uaa01.bin", 0x000000, 0x80000, CRC(1bc4893d) SHA1(2c9df38ecb7efa7b686221ee98fa3aad9a63e152))
	ROM_LOAD32_BYTE("479uaa02.bin", 0x000001, 0x80000, CRC(ae74a6d0) SHA1(6113c2eea1628b22737c7b87af0e673d94984e88))
	ROM_LOAD32_BYTE("479uaa03.bin", 0x000002, 0x80000, CRC(5c0b176f) SHA1(9560259bc081d4cfd72eb485c3fdcecf484ba7a8))
	ROM_LOAD32_BYTE("479uaa04.bin", 0x000003, 0x80000, CRC(01a3e4cb) SHA1(819df79909d57fa12481698ffdb32b00586131d8))

	ROM_REGION(0x20000, "audiocpu", 0) // M68K program
	ROM_LOAD("479_a05.bin", 0x000000, 0x20000, CRC(251ae299) SHA1(5ffd74357e3c6ddb3a208c39a3b32b53fea90282))

	ROM_REGION(0x100000, "k054539", 0) // Sound ROMs
	ROM_LOAD("479_a06.bin", 0x000000, 0x80000, CRC(8d6ac8a2) SHA1(7c4b8bd47cddc766cbdb6a486acc9221be55b579))
	ROM_LOAD("479_a07.bin", 0x080000, 0x80000, CRC(75835df8) SHA1(105b95c16f2ce6902c2e4c9c2fd9f2f7a848c546))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fiveside.nv", 0x0000, 0x0080, CRC(aad11072) SHA1(8f777ee47801faa7ce8420c3052034720225aae7) )
ROM_END

} // Anonymous namespace


// Undumped: Ultra Hockey
GAME(1995, fiveside, 0, ultrsprt, ultrsprt, ultrsprt_state, empty_init, ROT90, "Konami", "Five a Side Soccer (ver UAA)", 0)
