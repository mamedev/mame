// license:BSD-3-Clause
// copyright-holders:Cowering, Olivier Galibert
/*
  Mannesmann Tally MT735 portable thermal printer
  Also released by Siemens as HighPrint 730/735 Compact?

  Designed by Siemens Office Products Division

  Features:
    - 6ppm
    - 1MB of RAM (178KB used for downloaded soft fonts, 50KB input buffer)
    - 18 resident Portrait and Landscape fonts
    - print resolution of 300x300 dpi
    - four standard emulations (HP LaserJet Series II, HP DeskJet Plus, IBM Proprinter X24, Epson LQ850)
    - one Centronics parallel port
    - 5 buttons (Power, On-line, Print Quality, Form-feed, Copy)
    - 5 LEDs (Battery Check, Charger Active, Ribbon/Paper, Power, On-Line)

  2 dipswitches (strings from ROM)
   - = switched off
   o = switched on

  - - - - PC-8
  - - - o HP Roman-8
  - - o - PC-8 Den/Nor
  - - o o ISO 04 UK
  - o - - ISO 21 GER
  - o - o ISO 69 FRA
  - o o - ISO 15 ITA
  - o o o ISO 60 NOR
  o - - - ISO 11 SWN
  o - - o ISO 17 SPA
  o - o - ISO 06 ASCII US
  o - o o ISO 16 PORT
  o o - - ISO 14 JAP
  o o - o ISO 02 IRV / Denmark 1
  o o o - ECMA-94
  o o o o Legal / PC-8/DOWN LOAD
          - -     HP LaserJet Series II
          - o     HP DeskJet Plus
          o -     IBM Proprinter 4207 001
          o o     EPSON LQ850
              - - DIN A4
              - o US legal
              o - US letter
              o o undefined

  o CR=CR+LF
  - CR=CR
    o Select intern fixed ON
    - Select intern fixed OFF
      o Landscape
      - Portrait
        o no function
        - no function
          o AGM ON
          - AGM OFF
            o LF=LF+CR,FF=FF+CR
            - LF=LF,FF=FF
              o no function
              - no function
                o LF=LF+CR,FF=FF+CR
                - LF=LF,FF=FF+CR

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"


namespace {

class mt735_state : public driver_device
{
public:
	mt735_state(const machine_config &mconfig, device_type type, const char *tag);

	void mt735(machine_config &config);

private:
	required_device<m68000_device> m_cpu;

	uint8_t p4_r();
	uint8_t p5_r();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void mt735_map(address_map &map) ATTR_COLD;
};

mt735_state::mt735_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_cpu(*this, "maincpu")
{
}

void mt735_state::machine_start()
{
}

void mt735_state::machine_reset()
{
}

uint8_t mt735_state::p4_r()
{
	logerror("p4_r (%06x)\n", m_cpu->pc());
	return 0xe0;
}

uint8_t mt735_state::p5_r()
{
	logerror("p5_r (%06x)\n", m_cpu->pc());
	return 0x00;
}

void mt735_state::mt735_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x278000, 0x287fff).ram();
	map(0x400000, 0x4fffff).ram();
	map(0xff8004, 0xff8004).r(FUNC(mt735_state::p4_r));
	map(0xff8005, 0xff8005).r(FUNC(mt735_state::p5_r));
}

static INPUT_PORTS_START( mt735 )
INPUT_PORTS_END

void mt735_state::mt735(machine_config &config)
{
	M68000(config, m_cpu, XTAL(48'000'000)/6);
	m_cpu->set_addrmap(AS_PROGRAM, &mt735_state::mt735_map);
}

ROM_START( mt735 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spg_m_e_ic103.bin", 0, 0x20000, CRC(1ab58bc9) SHA1(c10d50f38819c037d28435b77e09f2b6923e8369) )
	ROM_LOAD16_BYTE( "spg_m_o_ic102.bin", 1, 0x20000, CRC(84d8446b) SHA1(b1cedd8b09556eb8118f79b012aeec5b61e3ff32) )
ROM_END

} // anonymous namespace


COMP( 1990, mt735, 0, 0, mt735, mt735, mt735_state, empty_init, "Mannesmann Tally", "MT735", MACHINE_NOT_WORKING|MACHINE_NO_SOUND ) // Program ROM Datecode: 19901126, Internal Font ROM Datecode: 19900807
