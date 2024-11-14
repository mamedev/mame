// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/flashbeats.cpp
    Sega "Flash Beats" game

    Hardware:
        - H8/3007 CPU
        - 68000 + SCSP for sound effects
        - DSB2 MPEG board with another 68000
        - Two Sega I/O chips: 315-5338A and 315-5296
        - Pinball-style dot matrix VFD display
        - 5 display tubes containing an unknown number of RGB LEDs behind
          a diffuser

****************************************************************************/

#include "emu.h"
#include "cpu/h8/h83006.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "315_5296.h"
#include "315_5338a.h"
#include "machine/te7750.h"
#include "sound/scsp.h"
#include "screen.h"
#include "speaker.h"


namespace {

class flashbeats_state : public driver_device
{
public:
	flashbeats_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scspcpu(*this, "scspcpu"),
		m_scsp(*this, "scsp"),
		m_sound_ram(*this, "sound_ram"),
		m_eeprom(*this, "eeprom"),
		m_315_5296(*this, "segaio1"),
		m_315_5338a(*this, "segaio2")
	{
	}

	void flashbeats(machine_config &config);
	void flashbeats_map(address_map &map) ATTR_COLD;
	void main_scsp_map(address_map &map) ATTR_COLD;
	void scsp_mem(address_map &map) ATTR_COLD;

	[[maybe_unused]] uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	virtual void machine_reset() override ATTR_COLD;

	void scsp_irq(offs_t offset, uint8_t data);
	uint8_t p6_r();
	void p6_w(uint8_t data);

	required_device<h83007_device> m_maincpu;
	required_device<m68000_device> m_scspcpu;
	required_device<scsp_device> m_scsp;
	required_shared_ptr<uint16_t> m_sound_ram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<sega_315_5296_device> m_315_5296;
	required_device<sega_315_5338a_device> m_315_5338a;
};


void flashbeats_state::machine_reset()
{
	uint8_t *ROM = memregion("scspcpu")->base();
	memcpy(m_sound_ram, ROM, 0x400);
	m_scspcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_scspcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

uint32_t flashbeats_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

uint8_t flashbeats_state::p6_r()
{
	return (m_eeprom->do_read() << 3);
}

void flashbeats_state::p6_w(uint8_t data)
{
	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data >> 2) & 1);
	m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}

void flashbeats_state::flashbeats_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x20ffff).ram();
	map(0x400000, 0x40007f).rw(m_315_5296, FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0xff00);
	map(0x600000, 0x60001f).rw("telio", FUNC(te7752_device::read), FUNC(te7752_device::write)).umask16(0xff00);
	map(0x800000, 0x80001f).rw(m_315_5338a, FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write)).umask16(0xff00);
	map(0xa00000, 0xa0ffff).ram();
	map(0xa10000, 0xa10fff).ram();
}

void flashbeats_state::main_scsp_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("sound_ram");
	map(0x100000, 0x100fff).rw("scsp", FUNC(scsp_device::read), FUNC(scsp_device::write));
	map(0x600000, 0x67ffff).rom().region("scspcpu", 0);
}

void flashbeats_state::scsp_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("sound_ram");
}

void flashbeats_state::flashbeats(machine_config &config)
{
	/* basic machine hardware */
	H83007(config, m_maincpu, 16_MHz_XTAL); // 16 MHz oscillator next to chip, also 16 MHz causes SCI0 and 1 rates to be 31250 (MIDI)
	m_maincpu->set_addrmap(AS_PROGRAM, &flashbeats_state::flashbeats_map);
	m_maincpu->read_port6().set(FUNC(flashbeats_state::p6_r));
	m_maincpu->write_port6().set(FUNC(flashbeats_state::p6_w));

	M68000(config, m_scspcpu, 11289600);
	m_scspcpu->set_addrmap(AS_PROGRAM, &flashbeats_state::main_scsp_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	SEGA_315_5296(config, m_315_5296, 8_MHz_XTAL);

	SEGA_315_5338A(config, m_315_5338a, 32_MHz_XTAL);

	te7752_device &te7752(TE7752(config, "telio"));
	te7752.ios_cb().set_constant(1);
	//te7752.out_port2_cb().set(FUNC(flashbeats_state::te7752_port2_w));
	//te7752.out_port3_cb().set(FUNC(flashbeats_state::te7752_port3_w));
	//te7752.out_port4_cb().set(FUNC(flashbeats_state::te7752_port4_w));
	//te7752.out_port5_cb().set(FUNC(flashbeats_state::te7752_port5_w));
	//te7752.out_port6_cb().set(FUNC(flashbeats_state::te7752_port6_w));
	//te7752.out_port7_cb().set(FUNC(flashbeats_state::te7752_port7_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SCSP(config, m_scsp, 22579200); // TODO : Unknown clock, divider
	m_scsp->set_addrmap(0, &flashbeats_state::scsp_mem);
	m_scsp->irq_cb().set(FUNC(flashbeats_state::scsp_irq));
	m_scsp->add_route(0, "lspeaker", 1.0);
	m_scsp->add_route(1, "rspeaker", 1.0);
}

void flashbeats_state::scsp_irq(offs_t offset, uint8_t data)
{
}

static INPUT_PORTS_START( flashbeats )
INPUT_PORTS_END

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( flsbeats )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "epr-21609_rom1.ic18", 0x000000, 0x080000, CRC(130a0a62) SHA1(400f24304959547b188ed874653ae2e1e77092fe) )

	ROM_REGION(0x280000, "scspcpu", 0)
	ROM_LOAD16_WORD_SWAP( "epr-21610_rom4.ic14", 0x000000, 0x080000, CRC(c877e0e6) SHA1(595f143fb3789852a4af9d2920cbaefabecfa45c) )
	ROM_LOAD16_WORD_SWAP( "epr-21611_rom3.ic4", 0x080000, 0x200000, CRC(2f5dc574) SHA1(f0b8d076b0fc8e94582de0ca17ecd5c8b90bedc4) )

	ROM_REGION(0x20000, "dsb2", 0)
	ROM_LOAD16_WORD_SWAP( "epr-21612.ic2", 0x000000, 0x020000, CRC(6912e1cb) SHA1(3497d6ae0b9be00116a3278f46d738c4c6f26d20) )

	ROM_REGION(0x2000000, "mpeg", 0)
	ROM_LOAD( "mpr-21601_n26_9852k7016.ic18", 0x0000000, 0x400000, CRC(d23e2b7b) SHA1(8c26a740fee0adc4d45d34786b0c28abb105324d) )
	ROM_LOAD( "mpr-21602_n27_9852k7017.ic19", 0x0400000, 0x400000, CRC(e143960b) SHA1(7ace5cae6f2a8868d74d4397a9c1b0a0f6f26c9f) )
	ROM_LOAD( "mpr-21603_n28_9852k7018.ic20", 0x0800000, 0x400000, CRC(136b69d8) SHA1(ad81be6f0383f29306c8d9f21d1e7440172ebf97) )
	ROM_LOAD( "mpr-21604_n29_9852k7019.ic21", 0x0c00000, 0x400000, CRC(87b15ea4) SHA1(eacc0f575926e68a195090370cb9e9e06b38e404) )
	ROM_LOAD( "mpr-21605_n30_9852k7023.ic22", 0x1000000, 0x400000, CRC(fb1a802e) SHA1(6fa99694973fd3cdd1a0f74a655583aadf5adc8f) )
	ROM_LOAD( "mpr-21606_n31_9852k7020.ic23", 0x1400000, 0x400000, CRC(d9aba5e0) SHA1(6f3f1b01174d771a56f92aeead998827da97ac22) )
	ROM_LOAD( "mpr-21607_n32_9852k7021.ic24", 0x1800000, 0x400000, CRC(f3dd07c6) SHA1(aa9d056e8ff5d2282917a09c42711062b8df989a) )
	ROM_LOAD( "mpr-21608_n33_9852k7022.ic25", 0x1c00000, 0x400000, CRC(be4db836) SHA1(93d4cbb3bb299e3cf1dda105670e3923751c28ad) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   MACHINE       INPUT       CLASS              INIT     MONITOR   COMPANY  FULLNAME      FLAGS
GAME( 1998, flsbeats, 0,    flashbeats,   flashbeats, flashbeats_state,  empty_init, ROT0,    "Sega", "Flash Beats", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
