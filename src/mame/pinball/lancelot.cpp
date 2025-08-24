// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************************

PINBALL

Sir Lancelot (Peyper, 1994)
- Main CPU is a Z80 (shown as 8400)
- Audio CPU is a TMP91P640F-10. Other audio chips are MSM6295QFP44, YMF262, YAC512, LM1036N.
- A video shows 60x 7-segment display digits, plus 5 round LEDs.
- According to the manual, the sound roms are 27C020 (size 0x40000), but the dumps we have are all
   half that size (0x20000). Are they bad?
- Crystals: 10MHz (for Z80), 9'830'400 (for TMP91P640F-10), 14'318'180 (for YMF262), 1'656'000 (for MSM6295).

Status:
- Skeleton

ToDo:
- The internal rom of the audio cpu is not dumped, so there's no possibility of sound.
- Mechanical sounds
- Outputs
- Inputs
- Layout
- Everything!

***************************************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/tlcs90/tlcs90.h"
#include "cpu/z80/z80.h"
#include "speaker.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
//#include "lancelot.lh"

namespace {

class lancelot_state : public genpin_class
{
public:
	lancelot_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		//, m_io_keyboard(*this, "X%d", 0U)
		//, m_leds(*this, "led%d", 0U)
		//, m_digits(*this, "digit%d", 0U)
		//, m_io_outputs(*this, "out%d", 0U)
		{ }

	void lancelot(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
	void p3_w(u8) { }
	void p4_w(u8) { }
	void p6_w(u8) { }
	void p7_w(u8) { }
	void p8_w(u8) { }
	u8 p3_r() { return 0; }
	u8 p5_r() { return 0; }
	u8 p7_r() { return 0; }
	u8 p8_r() { return 0; }

	required_device<z80_device> m_maincpu;
	required_device<tmp91640_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	//required_ioport_array<4> m_io_keyboard;
	//output_finder<5> m_leds;
	//output_finder<80> m_digits;
	//output_finder<80> m_io_outputs;   // 16 solenoids + 64 lamps + unknown others
};

void lancelot_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7FFF).rom();
	map(0x8000, 0x87FF).mirror(0x7800).ram().share("nvram");  // 6116, battery-backed
}

void lancelot_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x07); // SD1,2,3,4,5,6,SLD1,2
	map(0x08, 0x0f); // Y1 // in=??
	map(0x10, 0x17); // Y2 //??
	map(0x18, 0x1f); // Y3 // 16 solenoids
	map(0x20, 0x27); // Y4 // DSW0, DSW1
	map(0x28, 0x2f); // Y5 // out=something, in=active-low-contactors?
	map(0x30, 0x37); // Y6 // 64 lamps
	map(0x38, 0x3f); // Y7 //??
	map(0x40, 0x47); // Y8 // LD1-8
	map(0x50, 0x57); // WDOG
}

void lancelot_state::audio_map(address_map &map)
{
	map(0x00000, 0x03fff).rom(); // internal rom
	map(0x0ffc0, 0x0ffef).ram(); // internal io
	map(0x40000, 0x7ffff).rom().region("audiorom", 0); // u5 external rom
	map(0x80000, 0xbffff).rw("ymf", FUNC(ymf262_device::read), FUNC(ymf262_device::write)); // PS2: u8 YMF262(OPL chip) talks with u9 YAC512(DAC) which in turn creates audio
	map(0xc0000, 0xfffff).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // PS3: u6 MSM6295QFP44 uses u3,u4 roms and creates audio
}

static INPUT_PORTS_START( lancelot )
	PORT_START("DSW1") // closed = low
	PORT_START("DSW2") // closed = low
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
	PORT_START("X7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_KEYPAD )
INPUT_PORTS_END


void lancelot_state::machine_start()
{
	genpin_class::machine_start();

	//m_leds.resolve();
	//m_digits.resolve();
	//m_io_outputs.resolve();

	//save_item(NAME(m_row));
}

void lancelot_state::machine_reset()
{
	genpin_class::machine_reset();
	//for (u8 i = 0; i < m_io_outputs.size(); i++)
		//m_io_outputs[i] = 0;

	//m_row = 0;
}


void lancelot_state::lancelot(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 10_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &lancelot_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lancelot_state::io_map);
	m_maincpu->set_periodic_int(FUNC(lancelot_state::irq0_line_hold), attotime::from_hz(1056)); // frequency from online 555 calculator (duty cycle 96.5% high).

	tmp91640_device &tmp(TMP91640(config, m_audiocpu, 9.8304_MHz_XTAL));
	tmp.set_addrmap(AS_PROGRAM, &lancelot_state::audio_map);
	tmp.port_read<3>().set(FUNC(lancelot_state::p3_r));
	tmp.port_write<3>().set(FUNC(lancelot_state::p3_w));
	tmp.port_write<4>().set(FUNC(lancelot_state::p4_w));
	tmp.port_read<5>().set(FUNC(lancelot_state::p5_r));
	tmp.port_write<6>().set(FUNC(lancelot_state::p6_w));
	tmp.port_write<7>().set(FUNC(lancelot_state::p7_w));
	tmp.port_read<8>().set(FUNC(lancelot_state::p8_r));
	tmp.port_write<8>().set(FUNC(lancelot_state::p8_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	//config.set_default_layout(layout_lancelot);

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();   // MSM6295
	OKIM6295(config, m_oki, 1'656'000, okim6295_device::PIN7_HIGH); // pin7 is controlled by P63 from the audio cpu
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);

	SPEAKER(config, "speaker", 2).front();  // YAC512 left
  // YAC512 right
	ymf262_device &ymf(YMF262(config, "ymf", 14'318'180));
	ymf.irq_handler().set_inputline("audiocpu", 2); // to P82/INT2 of the audio cpu
	ymf.add_route(0, "speaker", 0.50, 0);
	ymf.add_route(1, "speaker", 0.50, 1);
	ymf.add_route(2, "speaker", 0.50, 0);
	ymf.add_route(3, "speaker", 0.50, 1);
}

/*-------------------------------------------------------------------
/ Sir Lancelot (1994)
/-------------------------------------------------------------------*/
ROM_START(lancelot)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("lancelot.u39", 0x0000, 0x8000, CRC(26c10926) SHA1(ad032b43c15b1d7a7f32a12ca09ea3344d75105b))

	ROM_REGION(0x4000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tmp91640.u1",  0x0000, 0x4000, NO_DUMP)

	ROM_REGION(0x40000, "audiorom", ROMREGION_ERASEFF)
	ROM_LOAD("snd_u5.u5",    0x00000, 0x20000, CRC(bf141441) SHA1(630b852bb3bba0fcdae13ae548b1e9810bc64d7d))

	ROM_REGION(0x80000, "oki", ROMREGION_ERASEFF)
	ROM_LOAD("snd_u3.u3",    0x00000, 0x20000, CRC(db88c28d) SHA1(35a80509c4a1f931d07af2fc74adbafc11af5639))
	ROM_LOAD("snd_u4.u4",    0x20000, 0x20000, CRC(5cebed6e) SHA1(d11cc57fadee95f056fc65927fa1f6ff0f337446))

ROM_END

} // Anonymous namespace

GAME( 1994, lancelot, 0, lancelot, lancelot, lancelot_state, empty_init, ROT0, "Peyper", "Sir Lancelot", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

