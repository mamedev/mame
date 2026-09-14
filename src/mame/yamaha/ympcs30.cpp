// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*

    Skeleton driver for Yamaha PortaSound PCS-30 keyboard with Playcard support

    CPU: Toshiba TMPZ84C00P (Z80 clone)
    Sound: Yamaha YM2142 (GE8)
    Other: Yamaha YM3514 (64 pins)

	TODO:
	- scanning a playcard generates a NMI; the handler reads data from E02x until completion or failure
	- tempo IRQ
	- sound, layout
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "speaker.h"

namespace {

class pcs30_state : public driver_device
{
public:
	pcs30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keys(*this, "KEY%u", 0U)
		, m_switch(*this, "switch%u", 0U)
		, m_led(*this, "led%u", 0U)
	{ }

	void pcs30(machine_config &config);

	template <int Num> DECLARE_INPUT_CHANGED_MEMBER(switch_w);
	template <int Num> ioport_value switch_r() { return m_switch[Num]; }

private:
	virtual void driver_start() override;

	void pcs30_map(address_map &map) ATTR_COLD;

	void keys_w(offs_t offset, u8 data);
	u8 keys_r();
	void led_w(offs_t offset, u8 data);

	required_device<z80_device> m_maincpu;
	required_ioport_array<9> m_keys;
	output_finder<2> m_switch;
	output_finder<30> m_led;

	u16 m_key_sel;
	u8 m_led_num[2];
};

void pcs30_state::driver_start()
{
	m_switch[0] = m_switch[1] = 1;
	m_key_sel = 0;
	m_led_num[0] = m_led_num[1] = 0;

	save_item(NAME(m_key_sel));
	save_item(NAME(m_led_num));
}

void pcs30_state::pcs30_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xe000, 0xe001).w(FUNC(pcs30_state::keys_w));
	map(0xe001, 0xe001).r(FUNC(pcs30_state::keys_r));
	map(0xe002, 0xe003).w(FUNC(pcs30_state::led_w));
	// e010-e013: ?
	// e020-e027: playcard interface
	// e030: YM2142 (write only)
}


void pcs30_state::pcs30(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000); // TODO: verify clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pcs30_state::pcs30_map);

	SPEAKER(config, "speaker").front_center();
}

template <int Num>
INPUT_CHANGED_MEMBER(pcs30_state::switch_w)
{
	if (!oldval && newval)
		m_switch[Num] = param;
}

void pcs30_state::keys_w(offs_t offset, u8 data)
{
	if (!offset)
		m_key_sel = (m_key_sel & 0xff00) | data;
	else
		m_key_sel = (m_key_sel & 0x00ff) | (data << 8);
}

u8 pcs30_state::keys_r()
{
	u8 data = 0xff;
	for (int i = 0; i < m_keys.size(); i++)
		if (BIT(m_key_sel, i))
			data &= m_keys[i]->read();

	return data;
}

void pcs30_state::led_w(offs_t offset, u8 data)
{
	m_led_num[offset] = data;
	for (int i = 0; i < m_led.size(); i++)
		m_led[i] = (m_led_num[0] == i) || (m_led_num[1] == i);
}


INPUT_PORTS_START(pcs30)
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("C6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("B5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("A#5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("A5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("G#5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("G5")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("F#5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("F5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("E5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("D#5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("D5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("C#5")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("C5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("B4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("A#4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("A4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("G#4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("G4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("F#4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("F4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("E4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("D#4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("D4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("C#4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("B3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("A#3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("A3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("G#3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("G3")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("F#3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("F3")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(pcs30_state::switch_r<0>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Start/Stop")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(pcs30_state::switch_r<1>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Auto Bass Chord On/Off")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Auto Play")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Free Tempo")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Lamp Cancel")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Stop")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Orchestra (Violin)")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<0>), 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Orchestra (Organ)")    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<0>), 0x02)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Orchestra (Clarinet)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<0>), 0x04)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Orchestra (Piano)")    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<0>), 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Orchestra (Guitar)")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<0>), 0x10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Orchestra (Vibes)")    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<0>), 0x20)

	PORT_START("SWITCH2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm (March)")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<1>), 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm (Disco)")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<1>), 0x02)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm (16 Beat)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<1>), 0x04)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm (Rhumba)")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<1>), 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm (Swing)")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<1>), 0x10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm (Waltz)")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pcs30_state::switch_w<1>), 0x20)

INPUT_PORTS_END


ROM_START( pcs30 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pcs30.bin", 0x0000, 0x8000, CRC(d63bc611) SHA1(7ad9c200ec4ea586efe1e9e5b5ad9bc8ca03407d)) // YM2211-22712
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT         COMPANY   FULLNAME  FLAGS
SYST( 1984, pcs30,   0,      0,      pcs30,   pcs30,  pcs30_state, empty_init,  "Yamaha", "PCS-30", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
