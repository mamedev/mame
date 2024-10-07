// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************************************
PINBALL
Driver for Barni pinballs.
Known pinballs to be dumped: Shield (1985) - different electronics
Hardware listing and ROM definitions from PinMAME.

Hardware:
    CPU: 2 x 6809E, optional MC6802 which may replace second 6809E
    INT: IRQ on CPU 0, FIRQ on CPU 1
    IO: 2x PIA 6821
        1x VIA 6522
    DISPLAY: 5x6 digit 7 or 16 segment display
    SOUND: basically the same as Bally's Squalk & Talk -61 board but missing AY8912 synth chip

Undumped 16L8 at position U9

Status:
- Skeletons

TODO:
- Inputs
- Outputs
- Save states
- Sound
- Mechanical sounds
- Almost everything
- Value of crystal on audio cpu
- Outputs (solenoids, lamps)
- Inputs (switches, dips)
- Display needs to be done properly, SDA2131 to be a device.
- How to trigger sounds?
- Does speech work? DAC works if you poke data into $5D in audio cpu.
- Manuals are difficult to read, and don't show everything we need.

**************************************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "sound/tms5220.h"
#include "sound/dac.h"
#include "speaker.h"
#include "barni.lh"

namespace {

class barni_state : public genpin_class
{
public:
	barni_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_audiocpu(*this, "audiocpu")
		, m_pia1(*this, "pia1")
		, m_pia2(*this, "pia2")
		, m_pias1(*this, "pias1")
		, m_pias2(*this, "pias2")
		, m_via(*this, "via")
		, m_speech(*this, "tms5220")
		, m_dac(*this, "dac")
		, m_dac2(*this, "dac2")
		, m_digits(*this, "digit%d", 0U)
		{ }

	void barni(machine_config &config);
	void audiocpu_map(address_map &map) ATTR_COLD;
	void maincpu_map(address_map &map) ATTR_COLD;
	void subcpu_map(address_map &map) ATTR_COLD;
private:
	u8 soundcmd_r();
	u8 set_firq();
	void clear_firq(u8);
	void via_pa_w(u8);
	void via_pb_w(u8);
	void pias1_pb_w(u8);
	void pias2_pb_w(u8);
	void showseg(u8, u8);
	u8 m_via_pa = 0U;
	u8 m_bitcount = 0U;
	u8 m_soundcmd = 0U;
	//void machine_reset() override ATTR_COLD;
	void machine_start() override { m_digits.resolve(); }
	required_device<mc6809e_device> m_maincpu;
	required_device<mc6809e_device> m_subcpu;
	required_device<m6802_cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<pia6821_device> m_pias1;
	required_device<pia6821_device> m_pias2;
	required_device<via6522_device> m_via;
	required_device<tms5220_device> m_speech;
	required_device<dac_byte_interface> m_dac;
	required_device<dac_4bit_r2r_device> m_dac2;
	output_finder<90> m_digits;
};

void barni_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).mirror(0x1c00).ram().share("common");  // U21,U22 2x 2114
	map(0x2000, 0x3fff).nopw();    // goes to 16L8
	map(0x4000, 0x5fff).r(FUNC(barni_state::set_firq));    // set FIRQ
	map(0x6000, 0x7fff).nopr();    // watchdog
	map(0x8000, 0x800f).mirror(0x1ff0).m(m_via, FUNC(via6522_device::map));  // U32
	map(0xa000, 0xa7ff).mirror(0x1800).ram();  // U14 5128 battery-backed
	map(0xc000, 0xffff).rom().region("maincpu",0);  // U15,U16
}

void barni_state::subcpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).mirror(0x1c00).ram().share("common");  // same ram as above
	map(0x2000, 0x3fff).w(FUNC(barni_state::clear_firq));    // clear FIRQ
	map(0x4000, 0x5fff);    // goes to 16L8
	map(0x6000, 0x7fff);    // goes to 16L8
	map(0x8000, 0x8003).mirror(0x1ffc).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));  // U38, also goes to 16L8
	map(0xa000, 0xa003).mirror(0x1ffc).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));  // U41, also goes to 16L8
	map(0xc000, 0xdfff);    // ROM not fitted U28
	map(0xe000, 0xffff).rom().region("subcpu",0);  // U27
}

void barni_state::audiocpu_map(address_map &map)
{
	map.global_mask(0xbfff);  // A14 not connected?
	map(0x0080, 0x0083).mirror(0x0c).rw(m_pias2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x0c).rw(m_pias1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1000).w(m_dac, FUNC(dac_byte_interface::data_w));
	map(0x2000, 0x2000).r(FUNC(barni_state::soundcmd_r));
	map(0x8000, 0xbfff).rom().region("audiocpu",0);
}

static INPUT_PORTS_START( barni )
INPUT_PORTS_END

u8 barni_state::set_firq()
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	return 0xff;
}

void barni_state::clear_firq(u8 data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

void barni_state::via_pa_w(u8 data)
{
	//printf("VIA PA = %X  ",data);
	m_via_pa = data;
	m_bitcount = 0;
}

void barni_state::via_pb_w(u8 data)
{
	//printf("VIA PB = %X  ",data);
	switch (BIT(m_via_pa, 4, 4))
	{
		case 0:
			// snd command
			m_soundcmd = data;
			m_pias2->cb1_w(1);
			break;
		case 1:
			if (!(m_bitcount % 8))
			{
				u8 segnum = 31 - (m_bitcount ? 0 : 1) - (2 * m_via_pa & 15);
				showseg(segnum, data);
			}
			m_bitcount++;
			break;
		case 5:
			// solenoids, lamps
			break;
		case 7:
			// something...
			break;
		default:
			break;
	}
}

void barni_state::showseg(u8 segnum, u8 data)
{
	u8 seg = 32 + segnum / 6;
	u8 bit = 1 << (segnum % 6);
	// player scores
	m_digits[segnum] = data & 0x7f;
	// digits in the middle
	if (BIT(data , 7))
		m_digits[seg] = m_digits[seg] | bit;
	else
		m_digits[seg] = (m_digits[seg] & 0x3f) ^ bit;
}

void barni_state::pias1_pb_w(u8 data)
{
	m_speech->rsq_w(BIT(data, 0));
	m_speech->wsq_w(BIT(data, 1));
}

void barni_state::pias2_pb_w(u8 data)
{
	m_dac2->write(BIT(data, 4, 4));
}

u8 barni_state::soundcmd_r()
{
	m_pias2->cb1_w(0);
	return m_soundcmd;
}

void barni_state::barni(machine_config &config)
{
	// MAIN BOARD
	MC6809E(config, m_maincpu, XTAL(4'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &barni_state::maincpu_map);

	MC6809E(config, m_subcpu, XTAL(4'000'000) / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &barni_state::subcpu_map);

	/* video hardware */
	config.set_default_layout(layout_barni);

	PIA6821(config, m_pia1);
//  m_pia1->writepa_handler().set(FUNC(barni_state::pia1_pa_w));
//  m_pia1->writepb_handler().set(FUNC(barni_state::pia1_pb_w));
//  m_pia1->readpa_handler().set(FUNC(barni_state::pia1_pa_r));

	PIA6821(config, m_pia2);
//  m_pia2->writepa_handler().set(FUNC(barni_state::pia2_pa_w));
//  m_pia2->writepb_handler().set(FUNC(barni_state::pia2_pb_w));
//  m_pia2->readpa_handler().set(FUNC(barni_state::pia2_pa_r));

	MOS6522(config, m_via, 4'000'000 / 4);  // to check
	m_via->irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_via->writepa_handler().set(FUNC(barni_state::via_pa_w));
	m_via->writepb_handler().set(FUNC(barni_state::via_pb_w));
//  m_via->ca2_handler().set(FUNC(barni_state::via_ca2_w));
//  m_via->cb2_handler().set(FUNC(barni_state::via_cb2_w));

	// SOUND BOARD
	M6802(config, m_audiocpu, 4000000); // guess - crystal value not shown
	m_audiocpu->set_addrmap(AS_PROGRAM, &barni_state::audiocpu_map);

	PIA6821(config, m_pias1); // U12
	m_pias1->writepa_handler().set(m_speech, FUNC(tms5220_device::data_w));
	m_pias1->writepb_handler().set(FUNC(barni_state::pias1_pb_w));
	m_pias1->irqa_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);
	m_pias1->irqb_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);

	PIA6821(config, m_pias2); // U17
	m_pias2->writepb_handler().set(FUNC(barni_state::pias2_pb_w));
	m_pias2->irqa_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);
	m_pias2->irqb_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);

	SPEAKER(config, "mono").front_center();
	TMS5220C(config, m_speech, 640000);   // unknown clock
	m_speech->irq_cb().set(m_pias1, FUNC(pia6821_device::cb1_w));
	m_speech->ready_cb().set(m_pias1, FUNC(pia6821_device::ca2_w));
	m_speech->add_route(ALL_OUTPUTS, "mono", 1.0);
	//dac
	MC1408(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.275);
	DAC_4BIT_R2R(config, m_dac2, 0).add_route(ALL_OUTPUTS, "mono", 0.5); // discrete parts
	genpin_audio(config);
}


/*--------------------------------
/ Champion 85
/-------------------------------*/
ROM_START(champion)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("chc.u15", 0x0000, 0x2000, CRC(6ab0f232) SHA1(0638d33f86c62ee93dff924a16a5b9309392d9e8))
	ROM_LOAD("che.u16", 0x2000, 0x2000, CRC(c5dc9228) SHA1(5306980a9c73118cfb843dbce0d56f516d054220))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("chan.bin", 0x0000, 0x2000, CRC(3f148587) SHA1(e44dc9cce15830f522dc781aaa13c659a43371f3))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("voz1.bin", 0x3000, 0x1000, CRC(48665778) SHA1(c295dfe7f4a98756f508391eb326f37a5aac37ff))
	ROM_LOAD("voz2.bin", 0x2000, 0x1000, CRC(30e7da5e) SHA1(3054cf9b09e0f89c242e1ad35bb31d9bd77248e4))
	ROM_LOAD("voz3.bin", 0x1000, 0x1000, CRC(3cd8058e) SHA1(fa4fd0cf4124263d4021c5a86033af9e5aa66eed))
	ROM_LOAD("voz4.bin", 0x0000, 0x1000, CRC(0d00d8cc) SHA1(10f64d2fc3fc3e276bbd0e108815a3b395dcf0c9))
ROM_END


/*--------------------------------
/ Red Baron
/-------------------------------*/
ROM_START(redbarnp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("redbaron.r2", 0x0000, 0x2000, CRC(0506e53e) SHA1(a1eaa39181cb3e5a1c281d217d680a42e39c856a))
	ROM_LOAD("redbaron.r1", 0x2000, 0x2000, CRC(fd401d3f) SHA1(33c0f178c798e16a9b4489a0e469f0a227882e79))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("redbaron.r3", 0x0000, 0x2000, CRC(45bca0b8) SHA1(77e2d6b04ea8d6fa7e30b59232696b9aa5307286))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("rbsnd1.732", 0x3000, 0x1000, CRC(674389ce) SHA1(595bbfe51dc3af266f4344e3865c0e48dd96acea))
	ROM_LOAD("rbsnd2.732", 0x2000, 0x1000, CRC(30e7da5e) SHA1(3054cf9b09e0f89c242e1ad35bb31d9bd77248e4))
	ROM_LOAD("rbsnd3.732", 0x1000, 0x1000, CRC(a4ba0f72) SHA1(e46148a2f5125914944973f37e73a62001c76aaa))
	ROM_LOAD("rbsnd4.732", 0x0000, 0x1000, CRC(fd8db899) SHA1(0978213f14f73ccc4a24eb42a39db00d9299c5d0))
ROM_END

} // Anonymous namespace

GAME( 1985, champion, 0, barni, barni, barni_state, empty_init, ROT0, "Barni", "Champion 85",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, redbarnp, 0, barni, barni, barni_state, empty_init, ROT0, "Barni", "Red Baron (Pinball)", MACHINE_IS_SKELETON_MECHANICAL )
