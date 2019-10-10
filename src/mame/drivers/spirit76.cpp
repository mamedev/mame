// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
// PINBALL
/*********************************************************************************************************************
Skeleton driver for Mirco's Spirit of 76, one of the first if not the first commercial solid-state pinball game.
Hardware listing and ROM definitions from PinMAME.

No schematic has been located as yet.

   Hardware:
CPU:   1 x M6800
IO:    1x PIA 6821
SOUND: Knocker, 3 Chimes (10, 100, 1000) 10 has highest tone.
LED DISPLAYS: 2x 6-digit displays (player scores), 2x 2-digit displays (in Credits area - Credits and Match).
OTHER INDICATORS:  5 red leds for the ball in play
  Tilt light
  Game over light
  5x lights for Drum Bonus, 13 lights for the Cannon
Settings are done with 2x 8 dipswitches
Switch at G6 (sw1-4 = free game scores, sw5-8 = config), at G8 (coin chute settings)
*********************************************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"

class spirit76_state : public genpin_class
{
public:
	spirit76_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

	void spirit76(machine_config &config);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_READ8_MEMBER(unk_r);
	void maincpu_map(address_map &map);

	u8 m_t_c;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

void spirit76_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
//  map.global_mask(0xfff); // this could most likely go in once the memory map is sorted
	map(0x0000, 0x00ff).ram(); // 2x 2112
	map(0x2200, 0x2203).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // 6820
	map(0x2400, 0x2400).r(FUNC(spirit76_state::unk_r));
	map(0x2401, 0x2401).w(FUNC(spirit76_state::unk_w));
	map(0x0600, 0x0fff).rom().region("roms", 0);
	map(0xfe00, 0xffff).rom().region("roms", 0x800);
}


static INPUT_PORTS_START( spirit76 )
	PORT_START("DSW0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER( spirit76_state::irq )
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

// continual write in irq routine
WRITE8_MEMBER( spirit76_state::porta_w )
{
	printf("PORT A=%X\n",data);
}

// continual write in irq routine
WRITE8_MEMBER( spirit76_state::portb_w )
{
	printf("PORT B=%X\n",data);
}

// continual read in irq routine
READ8_MEMBER( spirit76_state::porta_r )
{
	printf("Read PORT A\n");
	return 0xff;
}

// might not be used?
READ8_MEMBER( spirit76_state::portb_r )
{
	printf("Read PORT B\n");
	return 0xff;
}

// writes here once at start
WRITE8_MEMBER( spirit76_state::unk_w )
{
	printf("UNK PORT=%X\n",data);
}

// continual read in irq routine
READ8_MEMBER( spirit76_state::unk_r )
{
	return 0;
}

void spirit76_state::machine_reset()
{
	m_t_c = 0;
}

void spirit76_state::spirit76(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &spirit76_state::maincpu_map);

	TIMER(config, "irq").configure_periodic(FUNC(spirit76_state::irq), attotime::from_hz(120));

	/* video hardware */
	//config.set_default_layout();

	//6821pia
	pia6821_device &pia(PIA6821(config, "pia", 0));
	pia.writepa_handler().set(FUNC(spirit76_state::porta_w));
	pia.writepb_handler().set(FUNC(spirit76_state::portb_w));
	pia.readpa_handler().set(FUNC(spirit76_state::porta_r));
	pia.readpb_handler().set(FUNC(spirit76_state::portb_r));
//  pia.ca2_handler().set(FUNC(spirit76_state::pia22_ca2_w));
//  pia.cb2_handler().set(FUNC(spirit76_state::pia22_cb2_w));
//  pia.irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
//  pia.irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	/* sound hardware */
	genpin_audio(config);
}


ROM_START(spirit76)
	ROM_REGION(0xa00, "roms", 0)
	ROM_LOAD_NIB_LOW("1g.bin",  0x0000, 0x0200, CRC(57d7213c) SHA1(0897876f5c662b2518a680bcbfe282bb3a19a161))
	ROM_LOAD_NIB_HIGH("5g.bin", 0x0000, 0x0200, CRC(90e22786) SHA1(da9e0eae1e8576c6c8ac734a9557784d9e59c141))
	ROM_LOAD_NIB_LOW("2c.bin",  0x0200, 0x0200, CRC(4b996a52) SHA1(c73378e61598f84e20c1022b811780e300b01cd1))
	ROM_LOAD_NIB_HIGH("3c.bin", 0x0200, 0x0200, CRC(448626fa) SHA1(658b9589ba60ef62ff692192f743038d622776ba))
	ROM_LOAD_NIB_LOW("2e.bin",  0x0400, 0x0200, CRC(faaa907e) SHA1(ee9227944911a7c068216dd7b1b8dec284f90e3b))
	ROM_LOAD_NIB_HIGH("3e.bin", 0x0400, 0x0200, CRC(3463168e) SHA1(d98643179eac5ecbf1a559df59da620ea544bdee))
	ROM_LOAD_NIB_LOW("2f.bin",  0x0600, 0x0200, CRC(4d1a71ec) SHA1(6d3aa8fc4f7cec27d7fae2ecc73425388f8d9d52))
	ROM_LOAD_NIB_HIGH("3f.bin", 0x0600, 0x0200, CRC(bf23f0fd) SHA1(62e2ef7df0c057f25685a99e57cf95aae2e75cdb))
	ROM_LOAD_NIB_LOW("2g.bin",  0x0800, 0x0200, CRC(6236f053) SHA1(6183c8fa7dbd32ec40c4668cab8010b5e8c49949))
	ROM_LOAD_NIB_HIGH("3g.bin", 0x0800, 0x0200, CRC(ae7192cd) SHA1(9ba76e81b8603163c22f47f1a99da310b4325e84))
ROM_END


GAME( 1975, spirit76, 0, spirit76, spirit76, spirit76_state, empty_init, ROT0, "Mirco", "Spirit of 76", MACHINE_IS_SKELETON_MECHANICAL )
