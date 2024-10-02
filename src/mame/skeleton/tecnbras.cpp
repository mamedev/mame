// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

  TECNBRAS dot matrix display (70x7 pixels)
  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

    The display is composed of 14 blocks of 5x7 LEDs

    These LEDs are driven by several 74xx chips:
    * one 74138
    * several 74164 and ULN2003 chips

  Changelog:

   2014 JUN 23 [Felipe Sanches]:
   * Initial driver skeleton

================
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

#include <algorithm>

#include "tecnbras.lh"


namespace {

class tecnbras_state : public driver_device
{
public:
	tecnbras_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmds(*this, "dmd_%u", 0U)
	{ }

	void tecnbras(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void set_x_position_w(offs_t offset, uint8_t data);
	void print_column_w(offs_t offset, uint8_t data);

	//void tecnbras_io_w(uint8_t data);
	//uint8_t tecnbras_io_r();
	void i80c31_io(address_map &map) ATTR_COLD;
	void i80c31_prg(address_map &map) ATTR_COLD;

	required_device<i80c31_device> m_maincpu;
	output_finder<14 * 7> m_dmds;

	int m_xcoord = 0;
	char m_digit[14][7]{};
};

void tecnbras_state::i80c31_prg(address_map &map)
{
	map(0x0000, 0x7FFF).rom();
	map(0x8000, 0xFFFF).ram();
}

#define DMD_OFFSET 24 //This is a guess. We should verify the real hardware behaviour
void tecnbras_state::i80c31_io(address_map &map)
{
	map(0x0100+DMD_OFFSET, 0x0145+DMD_OFFSET).w(FUNC(tecnbras_state::set_x_position_w));
	map(0x06B8, 0x06BC).w(FUNC(tecnbras_state::print_column_w));
}

void tecnbras_state::set_x_position_w(offs_t offset, uint8_t data)
{
	m_xcoord = offset;
}

void tecnbras_state::print_column_w(offs_t offset, uint8_t data)
{
	int const x = m_xcoord + offset;
	int const ch = x / 5;
	if (ch < std::size(m_digit)) {
		int const row = x % 5;
		for (int i = 0; i < 7; i++) {
			m_digit[ch][i] &= ~(1 << row);
			m_digit[ch][i] |= BIT(data, 7 - i) ? (1 << row) : 0;
			m_dmds[(ch * 7) + i] = 0x1F & m_digit[ch][i];
		}
	}
}

void tecnbras_state::machine_start()
{
	m_dmds.resolve();

	save_item(NAME(m_xcoord));
	save_item(NAME(m_digit));

	m_xcoord = 0;
	for (auto &elem : m_digit)
		std::fill(std::begin(elem), std::end(elem), 0);

#if 0
	for (int x = 0; x < std::size(m_digit); x++)
		for (int y = 0; y < 7; y++)
			m_dmds[(x * 7) + y] = y;
#endif
}

void tecnbras_state::machine_reset()
{
}

void tecnbras_state::tecnbras(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, 12_MHz_XTAL); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &tecnbras_state::i80c31_prg);
	m_maincpu->set_addrmap(AS_IO, &tecnbras_state::i80c31_io);
	m_maincpu->port_out_cb<1>().set_nop(); // buzzer ?

/* TODO: Add an I2C RTC (Philips PCF8583P)
   pin 6 (SCL): cpu T0/P3.4 (pin 14)
   pin 5 (SDA): cpu T1/P3.5 (pin 15)
*/

/*
    TODO: Add a speaker
    CPU P1.0 (pin 1)
*/

/*
    TODO: Add a communications port to receive commands from the remote control
*/

	/* video hardware */
	config.set_default_layout(layout_tecnbras);
}

ROM_START( tecnbras )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tecnbras.u2",  0x0000, 0x8000, CRC(1a1e18fc) SHA1(8907e72f0356a2e2e1097dabac6d6b0b3d717f85) )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT        COMPANY     FULLNAME                            FLAGS
COMP( 200?, tecnbras, 0,      0,      tecnbras, 0,     tecnbras_state, empty_init, "Tecnbras", "Dot Matrix Display (70x7 pixels)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
