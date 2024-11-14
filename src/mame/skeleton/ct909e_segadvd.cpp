// license:BSD-3-Clause
// copyright-holders:David Haywood


/*****************************************************************************

  Portable DVD Player with built in Sega Master System games
  has AT Games strings in ROM
  sold in Spain as MeGaTrix

  uses a Cheertek CT909E-LF System on a Chip
  CPU core is likely LEON, a VHDL implementation of SPARC v8

*****************************************************************************/


#include "emu.h"

#include "cpu/sparc/sparc.h"

#include "screen.h"
#include "speaker.h"


namespace {

class ct909e_megatrix_state : public driver_device
{
public:
	ct909e_megatrix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void megatrix(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void uart1_data_w(u32 data);
	u32 uart1_status_r();

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


void ct909e_megatrix_state::uart1_data_w(u32 data)
{
	data &= 0x000000ff;
	if (data >= 0x20 && data < 0x7f)
		logerror("UART 1 sending '%c'\n", data);
	else
		logerror("UART 1 sending 0x%02X\n", data);
}

u32 ct909e_megatrix_state::uart1_status_r()
{
	return 0x00000006;
}

void ct909e_megatrix_state::mem_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("maincpu", 0);
	map(0x40000000, 0x407fffff).ram().share("dram");
	map(0x80000070, 0x80000073).w(FUNC(ct909e_megatrix_state::uart1_data_w));
	map(0x80000074, 0x80000077).r(FUNC(ct909e_megatrix_state::uart1_status_r));
}

uint32_t ct909e_megatrix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ct909e_megatrix_state::machine_start()
{
	m_maincpu->set_state_int(SPARC_PSR, 0xa0000000);
}

void ct909e_megatrix_state::machine_reset()
{
}

static INPUT_PORTS_START( megatrix )
INPUT_PORTS_END

void ct909e_megatrix_state::megatrix(machine_config &config)
{
	SPARCV8(config, m_maincpu, 100'000'000); // unknown frequency
	m_maincpu->set_addrmap(0, &ct909e_megatrix_state::mem_map);
	m_maincpu->set_addrmap(0x19, &ct909e_megatrix_state::mem_map);
	m_maincpu->set_addrmap(0x1b, &ct909e_megatrix_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(ct909e_megatrix_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( megatrix )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "segadvd_en29lv320b_007f22f9.bin", 0x000000, 0x400000, CRC(33fea2ff) SHA1(85295fd31a06149112295a9b6a8218a4a4c50893) )
ROM_END

} // anonymous namespace


CONS( 2007, megatrix,    0,       0,      megatrix, megatrix, ct909e_megatrix_state, empty_init, "<unknown>", "MeGaTrix (Spain)", MACHINE_IS_SKELETON )
