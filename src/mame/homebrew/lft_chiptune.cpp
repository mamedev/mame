// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

 ATmega88-based chiptune players by Linus Åkesson and kryo

**********************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK    8000000

class lft_chiptune_state : public driver_device
{
public:
	lft_chiptune_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
	{
	}

	void chiptune(machine_config &config);

protected:
	void prg_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	required_device<atmega88_device> m_maincpu;
	required_device<dac_byte_interface> m_dac;
};

//**************************************************************************
//  MEMORY
//**************************************************************************

void lft_chiptune_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void lft_chiptune_state::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}

//**************************************************************************
//  MACHINE
//**************************************************************************

static INPUT_PORTS_START( empty_input )
INPUT_PORTS_END

void lft_chiptune_state::chiptune(machine_config &config)
{
	/* basic machine hardware */
	ATMEGA88(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lft_chiptune_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &lft_chiptune_state::data_map);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->gpio_out<atmega88_device::GPIOD>().set(m_dac, FUNC(dac_8bit_r2r_device::write));

	/* sound hardware */
	SPEAKER(config, "avr8").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(0, "avr8", 0.9);
}

ROM_START( powernin )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "powernin.bin", 0x0000, 0x2000, CRC(67458936) SHA1(26a86846a24dd974723a66bea6c22baf51c7bec9) )
	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(bd7bc39f) SHA1(9d0ac37bb3ec8c95990fd37a962a17a95ce97aa0) )
ROM_END

ROM_START( hwchiptn )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hwchiptn.bin", 0x0000, 0x2000, CRC(0706eda8) SHA1(df227467bf4901978493efccaef6c4dfc32d5e62) )
	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom.raw", 0x0000, 0x0200, CRC(bd7bc39f) SHA1(9d0ac37bb3ec8c95990fd37a962a17a95ce97aa0) )
ROM_END

} // anonymous namespace


/*   YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT        CLASS               INIT        COMPANY                 FULLNAME */
CONS(2007, hwchiptn, 0,      0,      chiptune,   empty_input, lft_chiptune_state, empty_init, u8"Linus Åkesson / kryo", "The Hardware Chiptune Project", 0)
CONS(2009, powernin, 0,      0,      chiptune,   empty_input, lft_chiptune_state, empty_init, u8"Linus Åkesson", "Power Ninja Action Challenge", 0)
