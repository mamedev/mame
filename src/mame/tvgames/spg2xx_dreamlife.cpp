// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

#include "emu.h"
#include "spg2xx.h"

#include "machine/eepromser.h"

class dreamlif_state : public spg2xx_game_state
{
public:
	dreamlif_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom")
	{ }

	void dreamlif(machine_config &config);

private:
	uint16_t portb_r();
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_device<eeprom_serial_93cxx_device> m_eeprom;
};


uint16_t dreamlif_state::portb_r()
{
	uint16_t ret = 0x0000;
	logerror("%s: portb_r\n", machine().describe_context());
	ret |= m_eeprom->do_read() << 3;
	return ret;
}

void dreamlif_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb_w (%04x)\n", machine().describe_context(), data);
	m_eeprom->di_write(BIT(data, 2));
	m_eeprom->cs_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}



static INPUT_PORTS_START( dreamlif )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("B")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("C")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Yes")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("No")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be low or the Tiger logo gets skipped, also must be low for service mode (hold pause while booting) to work
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Pause")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( dsgnwrld )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Select")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be low or the Tiger logo gets skipped, also must be low for service mode (hold pause while booting) to work
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Pause")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void dreamlif_state::dreamlif(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &dreamlif_state::mem_map_4m);

	spg2xx_base(config);

	EEPROM_93C66_16BIT(config, m_eeprom); // HT93LC66A

	m_maincpu->porta_in().set_ioport("P1");
	m_maincpu->portb_in().set(FUNC(dreamlif_state::portb_r));
	m_maincpu->portb_out().set(FUNC(dreamlif_state::portb_w));

}

ROM_START( dreamlif )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dreamlife.bin", 0x000000, 0x800000, CRC(632e0237) SHA1(a8586e8a626d75cf7782f13cfd9f1b938af23d56) )
ROM_END

ROM_START( dsgnwrld )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "designersworld.bin", 0x000000, 0x800000, CRC(b3987161) SHA1(f7d03b172fd0accc6370d9ccc340b3aa6317426f) )
ROM_END


// Hasbro games
CONS( 2005, dreamlif,  0,        0, dreamlif, dreamlif,   dreamlif_state, empty_init, "Hasbro", "Dream Life (Version 1.0, Feb 07 2005)",  MACHINE_IMPERFECT_SOUND )
CONS( 2005, dsgnwrld,  0,        0, dreamlif, dsgnwrld,   dreamlif_state, empty_init, "Hasbro", "Designer's World (Version 1.0, Dec 20 2005)",  MACHINE_IMPERFECT_SOUND )
