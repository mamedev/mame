// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Yamaha MU-5 Tone Generator
    Preliminary driver by R. Belmont

    CPU: H8/3002 (HD6413002F10)
    Sound: YMW-258-F (aka "MultiPCM")
*/

#include "emu.h"

#include "cpu/h8/h83002.h"
#include "sound/multipcm.h"
#include "bus/midi/midi.h"

#include "mu5lcd.h"

#include "screen.h"
#include "speaker.h"


namespace {

class mu5_state : public driver_device
{
public:
	mu5_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ymw258(*this, "ymw258"),
		m_lcd(*this, "lcd"),
		m_key(*this, "S%c", 'A'),
		m_matrixsel(0)
	{ }

	void mu5(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<h83002_device> m_maincpu;
	required_device<multipcm_device> m_ymw258;
	required_device<mu5lcd_device> m_lcd;
	required_ioport_array<6> m_key;

	void mu5_map(address_map &map) ATTR_COLD;
	void ymw258_map(address_map &map) ATTR_COLD;

	u8 m_lcd_ctrl = 0U;
	u8 m_lcd_data = 0U;

	void lcd_ctrl_w(u8 data);
	u8 lcd_ctrl_r();
	void lcd_data_w(u8 data);
	u8 lcd_data_r();

	u8 m_matrixsel;
	u8 matrix_r();
};

void mu5_state::mu5_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram();
	map(0x400000, 0x400007).rw(m_ymw258, FUNC(multipcm_device::read), FUNC(multipcm_device::write)).umask16(0xffff);
}

void mu5_state::ymw258_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
}

void mu5_state::machine_start()
{
	save_item(NAME(m_lcd_ctrl));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_matrixsel));
}

void mu5_state::machine_reset()
{
	m_lcd_ctrl = 0;
	m_lcd_data = 0;
	m_matrixsel = 0;
}

u8 mu5_state::matrix_r()
{
	u8 data = 0x3f;

	for (int i = 0; i < 6; i++)
		if (!BIT(m_matrixsel, i))
			data &= m_key[i]->read();

	return data;
}

void mu5_state::lcd_ctrl_w(u8 data)
{
	// bit 2 = rs
	// bit 1 = r/w
	// bit 0 = e

	bool e_edge = (data ^ m_lcd_ctrl) & 1;
	m_lcd_ctrl = data;
	if(e_edge) {
		switch(m_lcd_ctrl & 7) {
		case 0:
			m_lcd->ir_w(m_lcd_data);
			break;
		case 3:
			m_lcd_data = m_lcd->status_r();
			break;
		case 4:
			m_lcd->dr_w(m_lcd_data);
			break;
		case 7:
			m_lcd_data = m_lcd->dr_r();
			break;
		}
	}
}

u8 mu5_state::lcd_ctrl_r()
{
	return m_lcd_ctrl;
}

void mu5_state::lcd_data_w(u8 data)
{
	m_lcd_data = data;
}

u8 mu5_state::lcd_data_r()
{
	return m_lcd_data;
}

static INPUT_PORTS_START(mu5)
	PORT_START("SA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part Down") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part Up") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value Down") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value Up") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit") PORT_CODE(KEYCODE_N)

	PORT_START("SB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x38, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)

	PORT_START("SD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Octave Up") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Octave Down") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mstr Tune") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trns Pose") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute Lock") PORT_CODE(KEYCODE_D)

	PORT_START("SE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Velo City") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Locl Ctrl") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dump Out") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Init All") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Vol") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pan") PORT_CODE(KEYCODE_Y)

	PORT_START("SF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI Ch") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Note Shft") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part Tune") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bend Rnge") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+/-") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

void mu5_state::mu5(machine_config &config)
{
	/* basic machine hardware */
	H83002(config, m_maincpu, 10_MHz_XTAL); // clock verified by schematics
	m_maincpu->set_addrmap(AS_PROGRAM, &mu5_state::mu5_map);
	m_maincpu->read_adc<7>().set_constant(0x3ff); // battery level
	m_maincpu->read_port4().set([this]() -> u8 { return m_matrixsel; });
	m_maincpu->write_port4().set([this](u8 data) { m_matrixsel = data; });
	m_maincpu->read_port6().set(FUNC(mu5_state::lcd_ctrl_r));
	m_maincpu->write_port6().set(FUNC(mu5_state::lcd_ctrl_w));
	m_maincpu->read_port7().set(FUNC(mu5_state::matrix_r));
	m_maincpu->read_porta().set_constant(0);
	m_maincpu->read_portb().set(FUNC(mu5_state::lcd_data_r));
	m_maincpu->write_portb().set(FUNC(mu5_state::lcd_data_w));

	MU5LCD(config, m_lcd);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MULTIPCM(config, m_ymw258, 9.4_MHz_XTAL); // clock verified by schematics
	m_ymw258->set_addrmap(0, &mu5_state::ymw258_map);
	m_ymw258->add_route(0, "lspeaker", 1.0);
	m_ymw258->add_route(1, "rspeaker", 1.0);

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout", midiout_slot, "midiout"));
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START( mu5 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("yamaha_mu5_program_xq201a0.bin", 0x000000, 0x020000, CRC(e9b3ec26) SHA1(cfee69699de78e2792dac24d428a120021ba147b))

	ROM_REGION(0x200000, "ymw258", 0)
	ROM_LOAD("yamaha_mu5_waverom_xp50280-801.bin", 0x000000, 0x200000, CRC(e0913030) SHA1(369f8df4942b6717c142ca8c4913e556dafae187))

ROM_END

} // anonymous namespace


CONS(1994, mu5, 0, 0, mu5, mu5, mu5_state, empty_init, "Yamaha", "MU-5", MACHINE_NOT_WORKING )
