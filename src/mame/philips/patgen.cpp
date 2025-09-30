// license:GPL-2.0+
// copyright-holders: stonedDiscord
/***************************************************************************

  Philips PM-5xxx test pattern generators

================

V12 80C31 Firmware
V101 Y Luminance
V103 Y Luminance
V105 Y Luminance
V107 Y Luminance
V109 Y Luminance
V201 V Chrominance
V203 V Chrominance
V301 U Chrominance
V303 U Chrominance

TODO: add GAL logic to route the ROMs to the DAC and output that onto a screen
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/saa1043.h"

#include "pm5644.lh"

//#define VERBOSE 1
#include "logmacro.h"

namespace {

class patgen_state : public driver_device
{
public:
	patgen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io_keyboard(*this, "COL%u", 0U),
		m_io_dsw(*this, "DSW"),
		m_leds(*this, "led%u%u", 0U, 0U)
	{ }

	void patgen(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void i80c31_data(address_map &map) ATTR_COLD;
	void i80c31_io(address_map &map) ATTR_COLD;
	void i80c31_prg(address_map &map) ATTR_COLD;

	u8 i80c31_p1_r();
	u8 keyboard_r();

	void led_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	void ch1_w(uint8_t data);
	void ch23_w(uint8_t data);

	u8 m_port1;

	required_device<i80c31_device> m_maincpu;
	required_ioport_array<4> m_io_keyboard;
	required_ioport m_io_dsw;
	output_finder<3, 8> m_leds;
};

void patgen_state::i80c31_prg(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void patgen_state::i80c31_io(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x8000, 0x8000).r(FUNC(patgen_state::keyboard_r));
	map(0x8000, 0x8002).w(FUNC(patgen_state::led_w));
	map(0x8004, 0x8004).w(FUNC(patgen_state::control_w));
	map(0x8005, 0x8005).w(FUNC(patgen_state::ch1_w));
	map(0x8006, 0x8006).w(FUNC(patgen_state::ch23_w));
}

void patgen_state::i80c31_data(address_map &map)
{
	map(0x000, 0x1ff).ram();
}

u8 patgen_state::i80c31_p1_r()
{
	m_port1 = m_io_dsw->read();
	//P1.4 2-WIRE SELECT
	//P1.5 FIELD1
	m_port1 = m_port1 | 0xc0;
	//P1.6 SCL pullup
	//P1.7 SDA pullup
	return m_port1;
}

u8 patgen_state::keyboard_r()
{
	u8 const col0 = m_io_keyboard[0]->read();
	u8 const col1 = m_io_keyboard[1]->read();
	u8 const col2 = m_io_keyboard[2]->read();
	u8 const col3 = m_io_keyboard[3]->read();

	u8 kb_state = col0 && col1 && col2 && col3;

	// FIXME: this block has no effect because the statement about always yields 0 or 1 from the Boolean operators
	if (col1 != 0xff)
		kb_state &= 0xdf;
	if (col2 != 0xff)
		kb_state &= 0xbf;
	if (col3 != 0xff)
		kb_state &= 0x7f;

	return kb_state;
}

void patgen_state::led_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		bool led_value = BIT(data, i);
		LOG("LED %d%d is %d\n", offset & 0x0f, i, led_value);
		m_leds[offset & 0x0f][i] = !led_value;
	}
}

void patgen_state::control_w(uint8_t data)
{

}

void patgen_state::ch1_w(uint8_t data)
{

}

void patgen_state::ch23_w(uint8_t data)
{

}

void patgen_state::machine_start()
{
	m_leds.resolve();
	m_port1 = 0x00;
}

void patgen_state::machine_reset()
{
}

static INPUT_PORTS_START( patgen )
	PORT_START("DSW")
	PORT_DIPNAME(0x00, 0x01, "Sync mode select 1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x00, 0x02, "Sync mode select 2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x00, 0x04, "Test mode enable")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))

	// D5 goes low when 3 7 11 or 15 are pressed
	// D6 goes low when 2 6 10 14 or 16 are pressed
	// D7 goes low when 1 5 9 13 17 are pressed

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WINDOW")  PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SIN X/Y")  PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S12-A") PORT_CODE(KEYCODE_C) //unpopulated
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1)  PORT_NAME("VERT ID SELECT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2)  PORT_NAME("SW")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CROSS HATCH")  PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINEARITY")  PORT_CODE(KEYCODE_7) //unpopulated
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S11-A") PORT_CODE(KEYCODE_B) //unpopulated
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S15-A") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COLOR BAR")  PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PULSE & BAR")  PORT_CODE(KEYCODE_6) //unpopulated
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("APL") PORT_CODE(KEYCODE_A) //unpopulated
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S14-A") PORT_CODE(KEYCODE_E) //unpopulated
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S16-A") PORT_CODE(KEYCODE_G) //unpopulated
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEST PATTERN")  PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MULTI BURST")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LIVE")  PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S13-A") PORT_CODE(KEYCODE_D) //unpopulated
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S17-A") PORT_CODE(KEYCODE_H)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

void patgen_state::patgen(machine_config &config)
{
	I80C31(config, m_maincpu, 16_MHz_XTAL); // Philips PCB80C31BH
	m_maincpu->set_addrmap(AS_PROGRAM, &patgen_state::i80c31_prg);
	m_maincpu->set_addrmap(AS_DATA, &patgen_state::i80c31_data);
	m_maincpu->set_addrmap(AS_IO, &patgen_state::i80c31_io);
	m_maincpu->port_in_cb<1>().set(FUNC(patgen_state::i80c31_p1_r));

	saa1043_device &saa1043(SAA1043(config, "saa1043", XTAL(5'000'000)));
	saa1043.ri_callback().set_inputline(m_maincpu, MCS51_INT1_LINE);

	config.set_default_layout(layout_pm5644);
}

ROM_START( pm5644g00 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_102_59371_csum_a100.v12",   0x000000, 0x10000, CRC(2b98e429) SHA1(18919af966316dad7fe612e394f6c663f4b3fae2) )

	ROM_REGION( 0x50000, "luma", 0 )
	ROM_LOAD( "4008_102_56191_csum_9a6a.v101",  0x000000, 0x10000, CRC(e307189b) SHA1(dc4b670752d77ec24bdf7b8d4981e84cf9e4b43a) )
	ROM_LOAD( "4008_102_56201_csum_4e67.v103",  0x010000, 0x10000, CRC(b00dc5c5) SHA1(4535de542b232fae6be4dda465283c6a65b5a199) )
	ROM_LOAD( "4008_102_56211_csum_3172.v105",  0x020000, 0x10000, CRC(d8bb3fd1) SHA1(9e37dc93f2b0902acafd23667b1cfabc5df1f66e) )
	ROM_LOAD( "4008_102_56221_csum_7f95.v107",  0x030000, 0x10000, CRC(b7eadaf8) SHA1(b40d3209203eae34d477660b37c9142cb501b289) )
	ROM_LOAD( "4008_102_56231_csum_78c4.v109",  0x040000, 0x10000, CRC(da70f99d) SHA1(e2fbbb8c190e3dfa35c242e8809b5d328d695166) )

	ROM_REGION( 0x20000, "chromav", 0 )
	ROM_LOAD( "4008_102_56241_csum_f0df.v201",  0x000000, 0x10000, CRC(73043b38) SHA1(e7299534fc6e069ca750d4b1eda0ee8ca4fe525c) )
	ROM_LOAD( "4008_102_56251_csum_f397.v203",  0x010000, 0x10000, CRC(8df2f123) SHA1(62177b44437d5dc213543bc838c792877031c9de) )

	ROM_REGION( 0x20000, "chromau", 0 )
	ROM_LOAD( "4008_102_56261_csum_2da9.v301",  0x000000, 0x10000, CRC(b4986372) SHA1(80522bd161a9385efa93fddee4d2ee9171504a03) )
	ROM_LOAD( "4008_102_56271_csum_2e0f.v303",  0x010000, 0x10000, CRC(97afd029) SHA1(b35b8bd21f5f0d1817d3d14233629b600d59f177) )

	ROM_REGION( 0x15000, "pal", 0 )
	ROM_LOAD( "pal_4008_102_55651_csum_6af0.jed",  0x000000, 0x01c46, CRC(a48f406d) SHA1(47b88554ac5655b026df17e4ae2902852c0f60ee) )
	ROM_LOAD( "pal_4008_102_56141_csum_e9f8.jed",  0x002000, 0x01c45, CRC(7adefbeb) SHA1(d57021f1579aa3bbd0713b87a3bc7466d26620ea) )
	ROM_LOAD( "pal_4008_102_56151_csum_e248.jed",  0x004000, 0x01c45, CRC(36346688) SHA1(f4b5302c851f12cbf82878ee86fc6e4c7e5680ab) )
	ROM_LOAD( "pal_4008_102_56161_csum_87e0.v40.jed",  0x006000, 0x01c45, CRC(45b03157) SHA1(a0a8f1633ac3ba8b123e470259ee51098c2a857c) )
	ROM_LOAD( "pal_4008_102_56171_csum_18d9.v205.jed", 0x008000, 0x01c46, CRC(5a2d22a2) SHA1(f274e38aac896b802eb44f1004d72b9d8d090eef) )
	ROM_LOAD( "pal_4008_102_56181_csum_10a1.v305.jed", 0x010000, 0x01c46, CRC(2e331f24) SHA1(41ceb44b6af43abc4a19c679255fbc763e68e2c5) )

	ROM_LOAD( "peel_4008_102_55441_csum_4a64.jed",  0x012000, 0x00dc5, CRC(cbd86564) SHA1(21b8268ba93a52f9efe0f481276a2862d4fb58dd) )
	ROM_LOAD( "peel_4008_102_55661_csum_6055.jed",  0x014000, 0x00dc5, CRC(3418434f) SHA1(2570d8f2138ae9ca182ac4ac6f8fe9b0973f4bb9) )
ROM_END


ROM_START( pm5644g913 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_102_58761_csum_1800.v12",   0x000000, 0x10000, CRC(a2091bcf) SHA1(53872f64a5f3e574731ce33f2cb0a6107b0d7b99) )

	ROM_REGION( 0x280000, "luma", 0 )
	ROM_LOAD( "4008_102_58802_csum_a64a.v101",  0x000000, 0x80000, CRC(3c1297fb) SHA1(9121da63c8990ed05c928f50d4027f06adb38606) )
	ROM_LOAD( "4008_102_58812_csum_61e5.v103",  0x080000, 0x80000, CRC(546d325e) SHA1(8896540598b9ad5969d714d634ca48f9056523ab) )
	ROM_LOAD( "4008_102_58822_csum_3fca.v105",  0x100000, 0x80000, CRC(67d34ad4) SHA1(6bae1bf35dab6ac56d1bf4f87b8b71644cfa3a57) )
	ROM_LOAD( "4008_102_58832_csum_757d.v107",  0x180000, 0x80000, CRC(aa0556ef) SHA1(12b2b01d0c77ac84c428eb2f037c649eb1189b52) )
	ROM_LOAD( "4008_102_58842_csum_e882.v109",  0x200000, 0x80000, CRC(086a26ad) SHA1(737a8575f09734feec8120cada62332cff420987) )

	ROM_REGION( 0x100000, "chromav", 0 )
	ROM_LOAD( "4008_102_58852_csum_81f9.v201",  0x000000, 0x80000, CRC(ead8c733) SHA1(b99677f0dd067f5b33d28421cb2128249cf46cca) )
	ROM_LOAD( "4008_102_58862_csum_81f9.v203",  0x080000, 0x80000, CRC(fec4ba06) SHA1(583473f70d16aff773b3433d8aa3874ab20535c0) )

	ROM_REGION( 0x100000, "chromau", 0 )
	ROM_LOAD( "4008_102_58872_csum_b4c3.v301",  0x000000, 0x80000, CRC(086a26ad) SHA1(737a8575f09734feec8120cada62332cff420987) )
	ROM_LOAD( "4008_102_58882_csum_b4c3.v303",  0x080000, 0x80000, CRC(5cf8ffa9) SHA1(0528803574a8a5240107b0ebb0aee2170c239311) )

	ROM_REGION( 0x15000, "pal", 0 )
	ROM_LOAD( "pal_4008_102_56171_csum_1809.v205.jed", 0x000000, 0x01c46, CRC(3163dc7b) SHA1(61788da7093836af4526d8586a35ee6139f81602) )
	ROM_LOAD( "pal_4008_102_56181_csum_10a1.v305.jed", 0x002000, 0x01c46, CRC(849ca310) SHA1(3e00d3cb11ce29b4a63679771a66cb9da2524302) )
	ROM_LOAD( "pal_4008_102_58291_csum_6653.v310.jed", 0x004000, 0x01c46, CRC(77bec308) SHA1(850a8362b0ce52ffdb598c8adc561a11a4a959fc) )
	ROM_LOAD( "pal_4008_102_58771_csum_e165.v37.jed",  0x006000, 0x01c93, CRC(1c6d421e) SHA1(8962207496c0434b63e2739c4fc996df39bfcbec) )
	ROM_LOAD( "pal_4008_102_58791_csum_681f.v40.jed",  0x008000, 0x01c45, CRC(e314503e) SHA1(4b5bf8a199203d7e45838e8221a26063a387bc97) )
	ROM_LOAD( "pal_4008_102_59681_csum_2c6d.v39.jed",  0x010000, 0x01c45, CRC(7c417cc2) SHA1(1015a222ed1c304ddc0923256c63c1b8b6519974) )

	ROM_LOAD( "peel_4008_102_55441_csum_4a64.jed",  0x012000, 0x00dc5, CRC(cbd86564) SHA1(21b8268ba93a52f9efe0f481276a2862d4fb58dd) )
	ROM_LOAD( "peel_4008_102_55661_csum_6055.jed",  0x014000, 0x00dc5, CRC(3418434f) SHA1(2570d8f2138ae9ca182ac4ac6f8fe9b0973f4bb9) )
ROM_END

ROM_START( pm5644g924 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_002_00541_csum_b700.v12",  0x000000, 0x10000, CRC(837d2c2c) SHA1(09be6adc54bfb3ed832d56b38063c186956a0043) )

	ROM_REGION( 0x280000, "luma", 0 )
	ROM_LOAD( "4008_002_00551_csum_9f93.v101",  0x000000, 0x80000, CRC(1314362d) SHA1(1f38a6a3f5a51bcd00cdd6408087b2ce91754c00) )
	ROM_LOAD( "4008_002_00561_csum_3998.v103",  0x080000, 0x80000, CRC(141f9c8f) SHA1(73780908900e5cb5c8abd8a1b9014c0eb1ef127c) )
	ROM_LOAD( "4008_002_00571_csum_5608.v105",  0x100000, 0x80000, CRC(ded20782) SHA1(51de44186ef2f6f9ba72f5a9c4de1359d640acad) )
	ROM_LOAD( "4008_002_00581_csum_739c.v107",  0x180000, 0x80000, CRC(08319fa5) SHA1(c4fd8b28596884fc50217edeaf09460be77f1f57) )
	ROM_LOAD( "4008_002_00591_csum_1c84.v109",  0x200000, 0x80000, CRC(3d65b467) SHA1(638fc4f2c6e3a5f2a53b915f9c881147a0e38393) )

	ROM_REGION( 0x100000, "chromav", 0 )
	ROM_LOAD( "4008_002_00601_csum_f478.v201",  0x000000, 0x80000, CRC(7ee1f934) SHA1(11fa770b1a21f6a265ce97fc20f2069b404bf813) )
	ROM_LOAD( "4008_002_00611_csum_f167.v203",  0x080000, 0x80000, CRC(df5b0f23) SHA1(b2ab2c784162f11e3a2cd992bf3c5efe080ca4e5) )

	ROM_REGION( 0x100000, "chromau", 0 )
	ROM_LOAD( "4008_002_00621_csum_06e1.v301",  0x000000, 0x80000, CRC(25a095d6) SHA1(ba7d37683b8fa77122fa4b2b1df31911634254d7) )
	ROM_LOAD( "4008_002_00631_csum_ffb8.v303",  0x080000, 0x80000, CRC(af031008) SHA1(6dbaba28c9f959d8594fc0b8b874ba6e0a83edbf) )

	ROM_REGION( 0x15000, "pal", 0 )
	ROM_LOAD( "pal_4008_002_00641_csum_e123.v37.jed",   0x000000, 0x01c45, CRC(172ec12f) SHA1(2ab12bc8024c2a0b4ea7a0f8aa706e3293e366eb) )
	ROM_LOAD( "pal_4008_002_00651_csum_f80d.v38.jed",   0x002000, 0x01c45, CRC(3ce26ccb) SHA1(f16084ac0e4e19529dcd19fb091275c58c0c2be3) )
	ROM_LOAD( "pal_4008_102_56171_csum_18d9.v205.jed",  0x004000, 0x01c46, CRC(5a2d22a2) SHA1(f274e38aac896b802eb44f1004d72b9d8d090eef) )
	ROM_LOAD( "pal_4008_102_56181_csum_10a1.v305.jed",  0x006000, 0x01c46, CRC(2e331f24) SHA1(41ceb44b6af43abc4a19c679255fbc763e68e2c5) )
	ROM_LOAD( "pal_4008_102_58291_csum_6653.v310.jed",  0x008000, 0x01c46, CRC(60704fbf) SHA1(c1fa652927a610438f4eabffcbcf80922fb98e6b) )
	ROM_LOAD( "pal_4008_102_58791_csum_681f.v40.jed",   0x00a000, 0x01c45, CRC(f78297e1) SHA1(bb6c5430f66eb02f3133e95d7db268d4977a45c1) )
	ROM_LOAD( "pal_4008_102_59681_csum_2c6d.v39.jed",   0x00c000, 0x01c45, CRC(f0921c84) SHA1(81cb2f8aa5651c47b79ec25a852d6fb78e9811fe) )

	ROM_LOAD( "peel_4008_102_55441_csum_4a64.v15.jed",  0x00e000, 0x00dc5, CRC(cbd86564) SHA1(21b8268ba93a52f9efe0f481276a2862d4fb58dd) )
	ROM_LOAD( "peel_4008_102_55661_csum_6055.v111.jed", 0x010000, 0x00dc5, CRC(3418434f) SHA1(2570d8f2138ae9ca182ac4ac6f8fe9b0973f4bb9) )
ROM_END

ROM_START( pm5644g924b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_002_00542_csum_b800.v12",  0x000000, 0x10000, CRC(5a63d539) SHA1(c969cd4c214b9fa485aeac03f42d3d4d5c9f7ae0) )

	ROM_REGION( 0x280000, "luma", 0 )
	ROM_LOAD( "4008_002_00552_csum_9f93.v101",  0x000000, 0x80000, CRC(b6034502) SHA1(a3c0c9f4ddc56412eb20208999f86db10c6b860f) )
	ROM_LOAD( "4008_002_00562_csum_3b85.v103",  0x080000, 0x80000, CRC(f13e8861) SHA1(3881253065ac5ecb72971d8088ea0f702658a176) )
	ROM_LOAD( "4008_002_00572_csum_5608.v105",  0x100000, 0x80000, CRC(61178f37) SHA1(131f9936f349a5f5189a33adc96f776cb172a71a) )
	ROM_LOAD( "4008_002_00582_csum_7589.v107",  0x180000, 0x80000, CRC(5e8ffcb2) SHA1(c9e44e6c39256dcf897e414290c3c7083ef8f66e) )
	ROM_LOAD( "4008_002_00592_csum_c1e0.v109",  0x200000, 0x80000, CRC(7fac919a) SHA1(03493ae0e6c6c44dd2fcb8ed3368c802ae812d7c) )

	ROM_REGION( 0x100000, "chromav", 0 )
	ROM_LOAD( "4008_002_00602_csum_f4c8.v201",  0x000000, 0x80000, CRC(202b5d66) SHA1(64c6e92fdaddd79505e9acb4b0ab20ed25934c79) )
	ROM_LOAD( "4008_002_00612_csum_f167.v203",  0x080000, 0x80000, CRC(4ccbf510) SHA1(e26d5cc44f2725afed433a70f36b89c3841b3ed3) )

	ROM_REGION( 0x100000, "chromau", 0 )
	ROM_LOAD( "4008_002_00621_csum_06e1.v301",  0x000000, 0x80000, CRC(25a095d6) SHA1(ba7d37683b8fa77122fa4b2b1df31911634254d7) )
	ROM_LOAD( "4008_002_00631_csum_ffb8.v303",  0x080000, 0x80000, CRC(af031008) SHA1(6dbaba28c9f959d8594fc0b8b874ba6e0a83edbf) )

	ROM_REGION( 0x17000, "pal", 0 )
	ROM_LOAD( "pal_4008_002_00641_csum_e123.v37.jed",   0x000000, 0x01c45, CRC(172ec12f) SHA1(2ab12bc8024c2a0b4ea7a0f8aa706e3293e366eb) )
	ROM_LOAD( "pal_4008_002_00651_csum_f80d.v38.jed",   0x002000, 0x00e29, CRC(3aef572a) SHA1(0aa4d946b91550b102f7f098fe1484d2b226a017) )
	ROM_LOAD( "pal_4008_102_56171_csum_18d9.v205.jed",  0x004000, 0x01c46, CRC(5a2d22a2) SHA1(f274e38aac896b802eb44f1004d72b9d8d090eef) )
	ROM_LOAD( "pal_4008_102_56181_csum_10a1.v305.jed",  0x006000, 0x01c46, CRC(2e331f24) SHA1(41ceb44b6af43abc4a19c679255fbc763e68e2c5) )
	ROM_LOAD( "pal_4008_102_58291_csum_6653.v310.jed",  0x008000, 0x01c46, CRC(60704fbf) SHA1(c1fa652927a610438f4eabffcbcf80922fb98e6b) )
	ROM_LOAD( "pal_4008_102_58791_csum_681f.v40.jed",   0x010000, 0x01c45, CRC(f78297e1) SHA1(bb6c5430f66eb02f3133e95d7db268d4977a45c1) )
	ROM_LOAD( "pal_4008_102_59681_csum_2c6d.v39.jed",   0x012000, 0x01c45, CRC(f0921c84) SHA1(81cb2f8aa5651c47b79ec25a852d6fb78e9811fe) )
	ROM_LOAD( "peel_4008_102_55441_csum_4a64.v15.jed",  0x014000, 0x00dc5, CRC(cbd86564) SHA1(21b8268ba93a52f9efe0f481276a2862d4fb58dd) )
	ROM_LOAD( "peel_4008_102_55661_csum_6055.v111.jed", 0x016000, 0x00dc5, CRC(3418434f) SHA1(2570d8f2138ae9ca182ac4ac6f8fe9b0973f4bb9) )
ROM_END

ROM_START( pm5644l00 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_002_01671_csum_cd00.v12",  0x000000, 0x10000, CRC(f06acb17) SHA1(9e7da23c3d6624190561cfc89f86fb5c9b563a63) )

	ROM_REGION( 0x280000, "luma", 0 )
	ROM_LOAD( "4008_002_01621_csum_c923.v101",  0x000000, 0x80000, CRC(5b2834d9) SHA1(483d285445e935cdbd05e4fef0e1c2cb5ce890c8) )
	ROM_LOAD( "4008_002_01631_csum_00d6.v103",  0x080000, 0x80000, CRC(ba4fb4d7) SHA1(acf357b0ffea7bdd3259dae3fc9292dcb799e481) )
	ROM_LOAD( "4008_002_01641_csum_c9d6.v105",  0x100000, 0x80000, CRC(b4a6e5c7) SHA1(329a93a8466eee4260e97a15c514d5da01211019) )
	ROM_LOAD( "4008_002_01651_csum_9c03.v107",  0x180000, 0x80000, CRC(2cd4931f) SHA1(1270f8ea23a68c14b2373a05dab394921c4ca11d) )
	ROM_LOAD( "4008_002_01661_csum_00e2.v109",  0x200000, 0x80000, CRC(1a85ff7b) SHA1(edab58f288f8be70a25488a38fe80ac899d9c21b) )

	ROM_REGION( 0x100000, "chromav", ROMREGION_ERASEFF )
	ROM_REGION( 0x100000, "chromau", ROMREGION_ERASEFF )

	ROM_REGION( 0x15000, "pal", 0 )
	ROM_LOAD( "pal_4008_102_56881_csum_0483.v37.jed", 0x000000, 0x01c46, CRC(c9e21167) SHA1(6a21419a0aa8f22b45cf03845c2e42d4ab4c28bf) )
	ROM_LOAD( "pal_4008_102_56891_csum_8669.v38.jed", 0x002000, 0x01c46, CRC(4b6be56d) SHA1(927fa0a6d3de55ec141df1ac5b76d1331f5175fe) )
	ROM_LOAD( "pal_4008_102_56901_csum_72e4.v40.jed", 0x004000, 0x01c45, CRC(354a13be) SHA1(d0b2965e3b24fa47fef345c9068c26118bf3bc45) )

	ROM_LOAD( "peel_4008_102_56301_csum_4ed3.jed",    0x006000, 0x00dc5, CRC(2a81d3e0) SHA1(62aa1647522940233de1f2d58103ee74f36b7d97) )
ROM_END

ROM_START( pm5644m00 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_102_59401_csum_7300.v12",  0x000000, 0x10000, CRC(d02ddfde) SHA1(2431e0c4d006264c0fd97a03763666ecd4ef660a) )

	ROM_REGION( 0x50000, "luma", 0 )
	ROM_LOAD( "4008_102_56781_csum_8e52.v101",  0x000000, 0x10000, CRC(f43e33e0) SHA1(5586f87966d062e18cd205a9e17a992043b80585) )
	ROM_LOAD( "4008_102_56741_csum_0d4a.v103",  0x010000, 0x10000, CRC(d246990f) SHA1(7f9a8b7d43a7d585d68f46e007a087c984f358a2) )
	ROM_LOAD( "4008_102_56751_csum_f7b6.v105",  0x020000, 0x10000, CRC(adfbcd39) SHA1(168cb18c83a7d6855d7fcf48fa8e8ccc082ec8f9) )
	ROM_LOAD( "4008_102_56761_csum_03df.v107",  0x030000, 0x10000, CRC(90ad9096) SHA1(eaede5bd2698a69c934f6737ef75636f6ce9b835) )
	ROM_LOAD( "4008_102_56771_csum_1a71.v109",  0x040000, 0x10000, CRC(98cdaeae) SHA1(fd617bf16912c824d0e425cf12e19293161e2276) )

	ROM_REGION( 0x20000, "chromav", 0 )
	ROM_LOAD( "4008_102_56791_csum_c1ea.v201",  0x000000, 0x10000, CRC(ba80ead8) SHA1(89f1215acaaa06f583926ee5a790801546680ff4) )
	ROM_LOAD( "4008_102_56801_csum_c1d0.v203",  0x010000, 0x10000, CRC(1226f352) SHA1(fb9c6777fb78268d42ce5caed7d29ee4630f0bb8) )

	ROM_REGION( 0x20000, "chromau", 0 )
	ROM_LOAD( "4008_102_56811_csum_b3ac.v301",  0x000000, 0x10000, CRC(1107cd24) SHA1(e97c39cffab7d49805383ba28b6506538f087547) )
	ROM_LOAD( "4008_102_56821_csum_b3ec.v303",  0x010000, 0x10000, CRC(d0fd4cd6) SHA1(2af1313dc4a6f0193f6e499fef7665d0ac7a9aa4) )

	ROM_REGION( 0x15000, "pal", 0 )
	ROM_LOAD( "pal_4008_102_55651_csum_6af0.v310.jed",  0x000000, 0x01c46, CRC(a48f406d) SHA1(47b88554ac5655b026df17e4ae2902852c0f60ee) )
	ROM_LOAD( "pal_4008_102_56171_csum_18d9.v205.jed",  0x002000, 0x01c46, CRC(5a2d22a2) SHA1(f274e38aac896b802eb44f1004d72b9d8d090eef) )
	ROM_LOAD( "pal_4008_102_56181_csum_10a1.v305.jed",  0x004000, 0x01c46, CRC(2e331f24) SHA1(41ceb44b6af43abc4a19c679255fbc763e68e2c5) )
	ROM_LOAD( "pal_4008_102_56711_csum_e9f8.v37.jed",   0x006000, 0x01c45, CRC(a90ebee5) SHA1(660e153df9dc006a25de5b38d39d7ed6c8ef2820) )
	ROM_LOAD( "pal_4008_102_56721_csum_dc9a.v38.jed",   0x008000, 0x01c45, CRC(a5369c41) SHA1(bbc6341ed47f845b21bab70f1c6fc504f327e4fc) )
	ROM_LOAD( "pal_4008_102_56731_csum_879c.v40.jed",   0x010000, 0x01c45, CRC(78995893) SHA1(5e69ca70caa1d14330a0ec0f3cbcb075fcae7274) )

	ROM_LOAD( "peel_4008_102_55441_csum_4a64.v15.jed",  0x012000, 0x00dc5, CRC(cbd86564) SHA1(21b8268ba93a52f9efe0f481276a2862d4fb58dd) )
	ROM_LOAD( "peel_4008_102_55661_csum_6055.v111.jed", 0x014000, 0x00dc5, CRC(3418434f) SHA1(2570d8f2138ae9ca182ac4ac6f8fe9b0973f4bb9) )
ROM_END

ROM_START( pm5644p00 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4008_102_59391_csum_0d00.v12",  0x000000, 0x10000, CRC(9e30f0dd) SHA1(3277db6f5c890897c6454b41a60103de6f652690) )

	ROM_REGION( 0x50000, "luma", 0 )
	ROM_LOAD( "4008_102_56831_csum_6c99.v101",  0x000000, 0x10000, CRC(dce2ec5a) SHA1(5fd53a091b39164a36c3109bac41d7f80f50c982) )
	ROM_LOAD( "4008_102_56841_csum_6cf9.v103",  0x010000, 0x10000, CRC(e31a3ebc) SHA1(24e7580ebf9d10f21a3a207b73b404f1c07300e0) )
	ROM_LOAD( "4008_102_56851_csum_0379.v105",  0x020000, 0x10000, CRC(73ef8961) SHA1(03f8703df4f1c13a70b0c397a26eff8308a27685) )
	ROM_LOAD( "4008_102_56861_csum_0393.v107",  0x030000, 0x10000, CRC(fe4477d4) SHA1(998c4a28c3a80e756cdaa1016b0019bc47e9995f) )
	ROM_LOAD( "4008_102_57161_csum_e601.v109",  0x040000, 0x10000, CRC(3e71ec87) SHA1(8342e0bbc768dd70ce5d484dd3e0f39a83c7b9a5) )

	ROM_REGION( 0x20000, "chromav", 0 )
	ROM_LOAD( "4008_102_57171_csum_d3b6.v201",  0x000000, 0x10000, CRC(6bab160c) SHA1(297f35e39adc6794bda82ca351cb9c7f8499f1f3) )
	ROM_LOAD( "4008_102_57181_csum_de42.v203",  0x010000, 0x10000, CRC(d849cbf4) SHA1(586251fc76f6ec5fa3bca87a3115d594536d41e3) )

	ROM_REGION( 0x20000, "chromau", 0 )
	ROM_LOAD( "4008_102_57191_csum_f178.v301",  0x000000, 0x10000, CRC(378419c7) SHA1(6a1aaf6c20e9ef13a1a72a787b1822a4118d3f68) )
	ROM_LOAD( "4008_102_57201_csum_e138.v303",  0x010000, 0x10000, CRC(80c9b2fe) SHA1(c0241458a51a9790ed4c977f667a94a40e612982) )

ROM_END

} // anonymous namespace

SYST( 1989, pm5644g00,   0,           0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (PAL-G 4:3 Colour Circle)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
SYST( 1989, pm5644g913,  0,           0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (PAL-G 4:3 Indian Head)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
SYST( 1989, pm5644g924,  pm5644g924b, 0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (PAL 16:9 Colour Circle Rev 1)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
SYST( 1989, pm5644g924b, 0,           0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (PAL 16:9 Colour Circle Rev 2)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
SYST( 1989, pm5644l00,   0,           0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (SECAM 4:3 Colour Circle)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
SYST( 1989, pm5644m00,   0,           0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (NTSC 4:3 Colour Circle)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
SYST( 1989, pm5644p00,   0,           0, patgen, patgen, patgen_state, empty_init, "Philips", "PM 5644 color pattern generator (PAL-M 4:3 Colour Circle)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
