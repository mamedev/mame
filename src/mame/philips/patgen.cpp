// license:GPL-2.0+
// copyright-holders: stonedDiscord
/***************************************************************************

  Philips PM-5xxx test pattern generators

================

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "speaker.h"

#include "pm5644.lh"

namespace {

class patgen_state : public driver_device
{
public:
	patgen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_leds(*this, "led%u", 0U)
	{ }

	void patgen(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void i80c31_data(address_map &map) ATTR_COLD;
	void i80c31_io(address_map &map) ATTR_COLD;
	void i80c31_prg(address_map &map) ATTR_COLD;

    void led_w(offs_t offset, uint8_t data);
	void key_row_w(u8 data);
	u8 keyboard_r();

	required_device<cpu_device> m_maincpu;
	output_finder<28> m_leds;
};

void patgen_state::i80c31_prg(address_map &map)
{
	map(0x0000, 0xFFFF).rom();
}

void patgen_state::i80c31_io(address_map &map)
{
    map(0x4000, 0x4002).w(FUNC(patgen_state::led_w));
    map(0x4000, 0x4000).r(FUNC(patgen_state::keyboard_r));
    //map(0x6000, 0x6000).w(FUNC(patgen_state::sig_latch_w)); // extra latch outputs
}

void patgen_state::i80c31_data(address_map &map)
{
	map(0x0000, 0x1FFF).ram();
}

void patgen_state::led_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		uint8_t led_index = ((offset & 0x0F) * 10) + i;
		bool led_value = BIT(data, i);
		m_leds[led_index] = led_value;
	}
}

u8 patgen_state::keyboard_r()
{
    u8 kb_state = 0xff;

    return kb_state;
}

void patgen_state::machine_start()
{
	m_leds.resolve();
}

void patgen_state::machine_reset()
{
}

static INPUT_PORTS_START( patgen )
	// D5 goes low when 3 7 11 or 15 are pressed
	// D6 goes low when 2 6 10 14 or 16 are pressed
	// D7 goes low when 1 5 9 13 17 are pressed

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S4-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S8-A") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S12-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("VERT ID SELECT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("SW")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S3-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S7-A") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S11-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S15-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S2-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S6-A") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S10-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S14-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S16-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S1-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S5-A") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S9-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S13-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S17-A") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

void patgen_state::patgen(machine_config &config)
{
	I80C31(config, m_maincpu, 16_MHz_XTAL); // Philips PCB80C31BH
    m_maincpu->set_addrmap(AS_PROGRAM, &patgen_state::i80c31_prg);
	m_maincpu->set_addrmap(AS_DATA, &patgen_state::i80c31_data);
	m_maincpu->set_addrmap(AS_IO, &patgen_state::i80c31_io);

	SPEAKER(config, "mono").front_center();

    config.set_default_layout(layout_pm5644);
}

ROM_START( pm5644g913 )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD( "4008_102_58761_CSUM_1800.v12",      0x00000, 0x10000, CRC(a2091bcf) SHA1(53872f64a5f3e574731ce33f2cb0a6107b0d7b99) )
    ROM_LOAD( "4008_102_58802_CSUM_A64A.v101",     0x080000, 0x80000, CRC(3c1297fb) SHA1(9121da63c8990ed05c928f50d4027f06adb38606) )
    ROM_LOAD( "4008_102_58812_CSUM_61E5.v103",     0x100000, 0x80000, CRC(546d325e) SHA1(8896540598b9ad5969d714d634ca48f9056523ab) )
    ROM_LOAD( "4008_102_58822_CSUM_3FCA.v105",     0x180000, 0x80000, CRC(67d34ad4) SHA1(6bae1bf35dab6ac56d1bf4f87b8b71644cfa3a57) )
    ROM_LOAD( "4008_102_58832_CSUM_757D.v107",     0x200000, 0x80000, CRC(aa0556ef) SHA1(12b2b01d0c77ac84c428eb2f037c649eb1189b52) )
    ROM_LOAD( "4008_102_58842_CSUM_E882.v109",     0x280000, 0x80000, CRC(086a26ad) SHA1(737a8575f09734feec8120cada62332cff420987) )
    ROM_LOAD( "4008_102_58852_CSUM_81F9.v201",      0x300000, 0x80000, CRC(ead8c733) SHA1(b99677f0dd067f5b33d28421cb2128249cf46cca) )
    ROM_LOAD( "4008_102_58862_CSUM_81F9.v203",      0x380000, 0x80000, CRC(fec4ba06) SHA1(583473f70d16aff773b3433d8aa3874ab20535c0) )
    ROM_LOAD( "4008_102_58872_CSUM_B4C3.v301",     0x400000, 0x80000, CRC(086a26ad) SHA1(737a8575f09734feec8120cada62332cff420987) )
    ROM_LOAD( "4008_102_58882_CSUM_B4C3.v303",     0x480000, 0x80000, CRC(5cf8ffa9) SHA1(0528803574a8a5240107b0ebb0aee2170c239311) )
ROM_END

} // anonymous namespace


SYST( 1989, pm5644g913,  0,       0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (PAL-G 4:3 Indian Head)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
