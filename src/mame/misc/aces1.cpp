// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
 Ace System One Hardware
 Fruit Machines

 skeleton driver!

sets placed here on
total rom size 0x8000
ram at 0x8000-0x87ff
lots of reads from 0xe000 at the start

JPM style Reel MCU? Certainly reel data seems to be muxed together in a weird way

 Hardware overview
  - Z80 (Z8400AB1)
  - 8 MHz XTAL
  - 3 x TMP8255
  - 2 x 8 dip-switches banks
  - 2 timed interrupts (IRQ and NMI) (can be reset)
  - AY8910 for sound
  - TC5517CPL-20


 - some of the roms appear to have been merged to larger files, or there are two versions of the board?
 - Looks like the merging is to account for an MFME issue - no known ROM board supports an all in one set.

*/

#include "emu.h"

#include "awpvid.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/steppers.h"
#include "sound/ay8910.h"

#include "speaker.h"

#include "aces1.lh"

// MFME2MAME layouts:
#include "ac1clbmn.lh"
#include "ac1gogld.lh"
#include "ac1pster.lh"
#include "ac1pstrt.lh"
#include "ac1primt.lh"
#include "ac1taklv.lh"


namespace {

class aces1_state : public driver_device
{
public:
	aces1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_reel(*this, "reel%u", 0U),
			m_io_ports(*this, {"IO1", "IO2", "IO3", "IO4", "IO5", "IO6", "IO7", "IO8"}),
			m_lamps(*this, "lamp%u", 0U),
			m_digits(*this, "digit%u", 0U)
	{ }

	void init_aces1();
	void aces1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	int m_input_strobe = 0;
	int m_lamp_strobe = 0;
	int m_led_strobe = 0;
	int m_reel_clock[4]{};
	int m_reel_phase[4]{};
	int m_reel_count[4]{};
	int m_optic_pattern = 0;

	template <unsigned N> void reel_optic_cb(int state) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }

	uint8_t aces1_unk_r()
	{
		return 0x00;
	}

	uint8_t aces1_unk_port00_r()
	{
		return 0x00;
	}

	uint8_t aces1_nmi_counter_reset_r()
	{
		aces1_reset_nmi_timer();
		return 0x00;
	}

	void aces1_nmi_counter_reset_w(uint8_t data)
	{
		aces1_reset_nmi_timer();
	}

	void aces1_reset_nmi_timer(void)
	{
		m_aces1_nmi_timer->adjust(m_maincpu->cycles_to_attotime(3072));
	}

	void aces1_reset_irq_timer(void)
	{
		m_aces1_irq_timer->adjust(m_maincpu->cycles_to_attotime(160000));
	}


	emu_timer *m_aces1_irq_timer = nullptr;
	emu_timer *m_aces1_nmi_timer = nullptr;


	void ic24_write_a(uint8_t data)
	{
		if (m_led_strobe != m_input_strobe)
		{
			m_digits[m_input_strobe] = data;
			m_led_strobe = m_input_strobe;
		}
	}

	void ic24_write_b(uint8_t data)
	{
		//cheating a bit here, need persistence
		if (m_lamp_strobe != m_input_strobe)
		{
			// Because of the nature of the lamping circuit, there is an element of persistance where the lamp retains residual charge
			// As a consequence, the lamp column data can change before the input strobe (effectively writing 0 to the previous strobe)
			// without causing the relevant lamps to black out.

			for (int i = 0; i < 8; i++)
			{
				m_lamps[8 * m_input_strobe + i] = BIT(data, i);
			}
			m_lamp_strobe = m_input_strobe;
		}
	}

	void ic24_write_c(uint8_t data)
	{
		m_input_strobe = (data & 0x0f);
	}

	void ic25_write_a(uint8_t data)
	{
	//  printf("extender lamps %02x\n", data);
	}

	void ic25_write_b(uint8_t data)
	{
	//  printf("meters, extender select %02x\n", data);
	}

	void ic25_write_c(uint8_t data)
	{
		//There needs to be some way of connecting these values to stepper coils, or doing the MCU properly
		// We should be able to see an enable clock, a sense and a full/half step selector, we don't have the half step visible it seems.

		//3 1 16 14
		int phases[] = {0x05,0x01,0x09,0x08,0x0a,0x02,0x06,0x04,};
		for (int reel=0; reel <4; reel++)
		{
			int clock = (data & (1<<reel));
			if (m_reel_clock[reel] != clock)
			{
				if (clock != 0)
				{
					int sense = ((data & (4 + (1<<reel))) ? -2:2);
					m_reel_phase[reel] = ((m_reel_phase[reel] + sense + 8) % 8);
					switch (reel)
					{
					case 0: m_reel[0]->update(phases[m_reel_phase[reel]]); break;
					case 1: m_reel[1]->update(phases[m_reel_phase[reel]]); break;
					case 2: m_reel[2]->update(phases[m_reel_phase[reel]]); break;
					case 3: m_reel[3]->update(phases[m_reel_phase[reel]]); break;
					}
					m_reel_clock[reel] = clock;
					if ( m_reel_phase[reel] % 4 ==0)
					{
						m_reel_count[reel]=1;
					}
					else
					{
						m_reel_count[reel]=0;
					}
					logerror("Reel %x Enable %x Sense %i Phase %x Data  %x\n",reel, clock, sense, m_reel_phase[reel],phases[m_reel_phase[reel]]  );
				}
				else
				{
					logerror("Reel %x Enable %x \n",reel, clock  );
				}
			}
//          logerror("Reel %x Enable %x Sense %i \n",reel, (data & (1<<reel)), (data & (4 + (1<<reel))) ? 1:-1  );
		}


//    printf("reels, extender strobe %02x\n", data);
	}

	uint8_t ic37_read_a()
	{
		//Should be coins and doors
		return ioport("COINS")->read();
	}

	uint8_t ic37_read_b()
	{
		return (m_io_ports[m_input_strobe & 7])->read();
	}

	uint8_t ic37_read_c()
	{
		int action =0;
		for (int reel = 0; reel < 4; reel++)
		{
			if (m_reel_count[reel]) action |= 1<<reel;
		}

		return ((m_optic_pattern << 4) | action);
	}

	// devices
	required_device<cpu_device> m_maincpu;
	required_device_array<stepper_device, 4> m_reel;
	required_ioport_array<8> m_io_ports;
	output_finder<128> m_lamps;
	output_finder<16> m_digits;

	TIMER_CALLBACK_MEMBER(m_aces1_irq_timer_callback);
	TIMER_CALLBACK_MEMBER(m_aces1_nmi_timer_callback);
	void aces1_map(address_map &map) ATTR_COLD;
	void aces1_portmap(address_map &map) ATTR_COLD;
};





TIMER_CALLBACK_MEMBER(aces1_state::m_aces1_irq_timer_callback)
{
//  printf("irq\n");
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	aces1_reset_irq_timer();
}

TIMER_CALLBACK_MEMBER(aces1_state::m_aces1_nmi_timer_callback)
{
//  printf("nmi\n");
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	aces1_reset_nmi_timer();
}

void aces1_state::machine_start()
{
	for (int reel=0; reel <4; reel++)
	{
		m_reel_clock[reel] =0;
		m_reel_phase[reel] =0;
	}
	m_aces1_irq_timer = timer_alloc(FUNC(aces1_state::m_aces1_irq_timer_callback), this);
	m_aces1_nmi_timer = timer_alloc(FUNC(aces1_state::m_aces1_nmi_timer_callback), this);
	m_digits.resolve();
	m_lamps.resolve();

	save_item(NAME(m_input_strobe));
	save_item(NAME(m_lamp_strobe));
	save_item(NAME(m_led_strobe));
	save_item(NAME(m_reel_clock));
	save_item(NAME(m_reel_phase));
	save_item(NAME(m_reel_count));
	save_item(NAME(m_optic_pattern));
}

void aces1_state::machine_reset()
{
	aces1_reset_nmi_timer();
	aces1_reset_irq_timer();
}

void aces1_state::aces1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0xadf0, 0xadf3).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)); //  Dips, Sound
	map(0xafb0, 0xafb3).rw("ic24", FUNC(i8255_device::read), FUNC(i8255_device::write)); // IC24 - lamps, 7segs
	map(0xafd0, 0xafd3).rw("ic25", FUNC(i8255_device::read), FUNC(i8255_device::write)); // IC25 - lamps, meters, reel comms (writes)
	map(0xafe0, 0xafe3).rw("ic37", FUNC(i8255_device::read), FUNC(i8255_device::write));//  IC37 - doors, coins, reel optics (reads)
	map(0xc000, 0xc000).r(FUNC(aces1_state::aces1_unk_r)); // illegal or reset irq?
	map(0xe000, 0xe000).rw(FUNC(aces1_state::aces1_nmi_counter_reset_r), FUNC(aces1_state::aces1_nmi_counter_reset_w));
}


void aces1_state::aces1_portmap(address_map &map)
{
	map(0x00, 0x00).r(FUNC(aces1_state::aces1_unk_port00_r)); // read before enabling interrupts?
}


static INPUT_PORTS_START( aces1 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DSWB" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void aces1_state::aces1(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000) / 2); /* XTAL verified, divisor not */
	m_maincpu->set_addrmap(AS_PROGRAM, &aces1_state::aces1_map);
	m_maincpu->set_addrmap(AS_IO, &aces1_state::aces1_portmap);

	// 0xafb0 IC24 - lamps, 7segs
	i8255_device &ic24(I8255A(config, "ic24"));
	ic24.out_pa_callback().set(FUNC(aces1_state::ic24_write_a));  // 7segs
	ic24.out_pb_callback().set(FUNC(aces1_state::ic24_write_b));  // lamps
	ic24.out_pc_callback().set(FUNC(aces1_state::ic24_write_c));  // strobe

	// 0xafd0 IC25 - lamps, meters, reel comms (writes)
	i8255_device &ic25(I8255A(config, "ic25"));
	ic25.out_pa_callback().set(FUNC(aces1_state::ic25_write_a));  // extra lamps
	ic25.out_pb_callback().set(FUNC(aces1_state::ic25_write_b));  // meters, extra lamp select
	ic25.out_pc_callback().set(FUNC(aces1_state::ic25_write_c));  // reel write, extra lamp strobe

	// 0xafe0 IC37 - doors, coins, reel optics (reads)
	i8255_device &ic37(I8255A(config, "ic37"));
	ic37.in_pa_callback().set(FUNC(aces1_state::ic37_read_a)); // extra lamps
	ic37.in_pb_callback().set(FUNC(aces1_state::ic37_read_b)); // meters, extra lamp select
	ic37.in_pc_callback().set(FUNC(aces1_state::ic37_read_c)); // reel write, extra lamp strobe

	config.set_default_layout(layout_aces1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// 0xadf0 - Dips, Sound
	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(8'000'000) / 8)); /* XTAL verified, divisor not */
	aysnd.port_a_read_callback().set_ioport("DSWA");
	aysnd.port_b_read_callback().set_ioport("DSWB");
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.00);

	/* steppers */
	REEL(config, m_reel[0], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[0]->optic_handler().set(FUNC(aces1_state::reel_optic_cb<0>));
	REEL(config, m_reel[1], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[1]->optic_handler().set(FUNC(aces1_state::reel_optic_cb<1>));
	REEL(config, m_reel[2], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[2]->optic_handler().set(FUNC(aces1_state::reel_optic_cb<2>));
	REEL(config, m_reel[3], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[3]->optic_handler().set(FUNC(aces1_state::reel_optic_cb<3>));
}


ROM_START( ac1clbmn )
	ROM_REGION( 0x8000, "maincpu", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "cm388p71.bin", 0x0000, 0x2000, CRC(c686c1e3) SHA1(c60bf1616b30413f8ecd4a2c8b75d1b37f456c1f) )
	ROM_LOAD( "cm388p72.bin", 0x2000, 0x2000, CRC(f2cb5fdf) SHA1(fb87865b366cde88b5b4b8cec64d52d2c15a3ee5) )
	ROM_LOAD( "cm388p73.bin", 0x4000, 0x2000, CRC(6ee672c3) SHA1(2258088ae4224f06a250040d7c0b8fd964a9e56c) )
	ROM_LOAD( "cm388p74.bin", 0x6000, 0x2000, CRC(db3e2581) SHA1(26d1b58318f126e88190b67d87ba5bbb802d45ba) )
ROM_END

ROM_START( ac1gogld )
	ROM_REGION( 0x8000, "maincpu", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "370gg_1.bin", 0x0000, 0x2000, CRC(c1337c4c) SHA1(e8d372e2faeb84eec50e297b183f2416891bd2ec) )
	ROM_LOAD( "370gg_2.bin", 0x2000, 0x2000, CRC(2c31bfb1) SHA1(9b95be6839a25c906d4ea9cea70bb641ecaf77e5) )
	ROM_LOAD( "370gg_3.bin", 0x4000, 0x2000, CRC(f6a6dd6e) SHA1(adaae3a15d03e41f192bea33b3d2acca0488b0c6) )
	ROM_LOAD( "370gg_4.bin", 0x6000, 0x2000, CRC(0af03fb2) SHA1(2966f5954635d96287a9bca8ea33bd0b55ad51ce) )
ROM_END

ROM_START( ac1hotpf )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ace_hot_profit_systemone.bin", 0x0000, 0x8000, CRC(951a750d) SHA1(feff32617321c5403f8e38a3aca1b49d065d5616) )
ROM_END

ROM_START( ac1pster )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pstrling.bin", 0x0000, 0x8000, CRC(ae46e199) SHA1(4cbe1205fa22e54b730f1fa5c01151368f35ed5f) )
ROM_END

ROM_START( ac1pstrt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pst.bin", 0x0000, 0x8000, CRC(8e2ee921) SHA1(100de5ab0420d6c0196d90da4412a7d2c24a0912) )
ROM_END

ROM_START( ac1primt )
	ROM_REGION( 0x8000, "maincpu", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "403ptp21.bin", 0x0000, 0x2000, CRC(5437973c) SHA1(cd42fe09a75ea8bf8efd25c1ca7b12c4db029f31) )
	ROM_LOAD( "403ptp22.bin", 0x2000, 0x2000, CRC(a0a800a1) SHA1(348726be2e0161f2bfe63ca80e5193609c4f4211) )
	ROM_LOAD( "403ptp23.bin", 0x4000, 0x2000, CRC(69b6df2a) SHA1(2ecfce178b4fa22e2b8a3855171cf7e06ac0dc6d) )
	ROM_LOAD( "403ptp24.bin", 0x6000, 0x2000, CRC(12578388) SHA1(7e16dad8bc19c34c23f7fa3e627a1c85f669a19e) )
ROM_END

ROM_START( ac1taklv )
	ROM_REGION( 0x8000, "maincpu", 0 ) // same thing but in smaller roms? should they all really be like this?
	ROM_LOAD( "430tlp11.bin", 0x0000, 0x2000, CRC(32241ccd) SHA1(5aa46b0f45ab92ad8d1b9d500a6f8416888e4094) )
	ROM_LOAD( "430tlp12.bin", 0x2000, 0x2000, CRC(017479b6) SHA1(6ea72c1cd1866b6469eef51a841ca12720af0121) )
	ROM_LOAD( "430tlp13.bin", 0x4000, 0x2000, CRC(c70082c2) SHA1(2b7e901a6eb31871f83d835288025d256775c11b) )
	ROM_LOAD( "430tlp14.bin", 0x6000, 0x2000, CRC(09008e12) SHA1(f3f6dd3bafdcf7187148fed914d7c43caf53d48a) )
ROM_END

ROM_START( ac1cshtw ) // Cash Towers, same ROM as above, original machine apparently plays the same, reskinned machine?
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ctp1.bin", 0x0000, 0x8000, CRC(2fabb08f) SHA1(b737930e428f9258ab22394229c2b5039edf8f97) )
ROM_END


ROM_START( ac1bbclb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bb587p7a", 0x0000, 0x2000, CRC(906142a6) SHA1(395bfa7277a7953d47eb77a6689964e79e075b9c) )
	ROM_LOAD( "bb587p7c", 0x2000, 0x2000, CRC(53b462e4) SHA1(ead45992ab8d32ff931d6a262696395e03af0181) )
	ROM_LOAD( "bb587p7d", 0x4000, 0x2000, CRC(daff7083) SHA1(9a5dd2153b3e3b3c2b4edf0a52a9c07504961b4a) )
	ROM_LOAD( "bb587p7e", 0x6000, 0x2000, CRC(edf56e20) SHA1(7790851d8ddf599694846b4ddcfe2669a0ef05cc) )
ROM_END

ROM_START( ac1bbclba )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bb587p7b", 0x0000, 0x2000, CRC(e229c32d) SHA1(945f9492f642979009fa0dd309a5c1b3e43a671b) ) // alt rom at 0x0000
	ROM_LOAD( "bb587p7c", 0x2000, 0x2000, CRC(53b462e4) SHA1(ead45992ab8d32ff931d6a262696395e03af0181) )
	ROM_LOAD( "bb587p7d", 0x4000, 0x2000, CRC(daff7083) SHA1(9a5dd2153b3e3b3c2b4edf0a52a9c07504961b4a) )
	ROM_LOAD( "bb587p7e", 0x6000, 0x2000, CRC(edf56e20) SHA1(7790851d8ddf599694846b4ddcfe2669a0ef05cc) )
ROM_END


ROM_START( ac1clbsv )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "411cs_1.bin", 0x0000, 0x2000, CRC(0f1df1ea) SHA1(2d908838de301c62c4e334626d8ada5ad544d518) )
	ROM_LOAD( "411cs_2.bin", 0x2000, 0x2000, CRC(9bfd03d3) SHA1(ee83783bfaeca685427d5d674148df49af2b4647) )
	ROM_LOAD( "411cs_3.bin", 0x4000, 0x2000, CRC(a105c0c5) SHA1(0e9de822e8e6707a00f321e9664d50c117713abf) )
	ROM_LOAD( "411cs_4.bin", 0x6000, 0x2000, CRC(750a1ab9) SHA1(8c6e7aa3526ee807ce4edc4b64a9c21de67d7985) )
ROM_END


ROM_START( ac1clbxt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "xt545p2c", 0x0000, 0x2000, CRC(78d7335a) SHA1(d06a67c7074f264940314de089341df0de73f3cc) )
	ROM_LOAD( "xt545p2e", 0x2000, 0x2000, CRC(97681816) SHA1(e456420b88676b32721921dc9e43279b006e66ec) )
	ROM_LOAD( "xt545p2f", 0x4000, 0x2000, CRC(cc7d9052) SHA1(d64ad5106adaaeb5665462c3ec8a2985693ab5ff) )
	ROM_LOAD( "xt545p2g", 0x6000, 0x2000, CRC(af26cdd8) SHA1(c4011f54bf669a4ac8e04691303c9d593399f9e5) )
ROM_END

ROM_START( ac1clbxta )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "xt545p2d", 0x0000, 0x2000, CRC(a7fe418d) SHA1(96c1dfb878ed0ca585fdfc5555255f334d8b5f99) ) // alt rom at 0x0000
	ROM_LOAD( "xt545p2e", 0x2000, 0x2000, CRC(97681816) SHA1(e456420b88676b32721921dc9e43279b006e66ec) )
	ROM_LOAD( "xt545p2f", 0x4000, 0x2000, CRC(cc7d9052) SHA1(d64ad5106adaaeb5665462c3ec8a2985693ab5ff) )
	ROM_LOAD( "xt545p2g", 0x6000, 0x2000, CRC(af26cdd8) SHA1(c4011f54bf669a4ac8e04691303c9d593399f9e5) )
ROM_END



ROM_START( ac1piaca )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pia casino 5p 5-1.bin", 0x0000, 0x2000, CRC(216b0719) SHA1(11a1a04edd981922e2a6756709168c33e1d437c7) ) // aka pd566p5a
	ROM_LOAD( "pia casino 5p 5-2.bin", 0x2000, 0x2000, CRC(5baad962) SHA1(3afd8e0f76e82cd7fd028d9a627a9ca5a7704bd7) ) // aka pd566p5b
	ROM_LOAD( "pia casino 5p 5-3.bin", 0x4000, 0x2000, CRC(d6670a6a) SHA1(30de7ba1534351ec96c803530fcdacc234ee7454) ) // aka pd566p5c
	ROM_LOAD( "pia casino 5p 5-4.bin", 0x6000, 0x2000, CRC(7987db2d) SHA1(ee6ed7617f64faaa12182a8f12a96adb8b0ce32d) ) // aka pd566p5d
ROM_END



ROM_START( ac1piacl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cpia7_1.bin", 0x0000, 0x2000, CRC(b8f4e481) SHA1(296aebd8ead9d87b050e6af4535eb81b7dc0f574) )
	ROM_LOAD( "cpia7_2.bin", 0x2000, 0x2000, CRC(09520caa) SHA1(cb2f0f88e47a2bb5d23817c1c757925ef4278ec9) )
	ROM_LOAD( "cpia7_3.bin", 0x4000, 0x2000, CRC(8b606921) SHA1(2309536ce500f8a42ef2eb2e634211875f0c97fe) )
	ROM_LOAD( "cpia7_4.bin", 0x6000, 0x2000, CRC(20e403a0) SHA1(6703459d3354ee6069e10c60498943912316cd62) )
ROM_END

ROM_START( ac1piacla )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cp513p7a", 0x0000, 0x8000, CRC(89b7d808) SHA1(26ad587e5d5f788c2c4e075e27c385ff67453c53) )
ROM_END

ROM_START( ac1piaclb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cp513p7b", 0x0000, 0x8000, CRC(05c5241b) SHA1(eeb2bcda1a3a203c749667fb374c5d32ace585c7) )
ROM_END


ROM_START( ac1piaclc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cpia7.bin", 0x0000, 0x8000, CRC(718b10f0) SHA1(0fe8ee1d14e5f22d27057f53ef2a2690cf02cab2) )
ROM_END



ROM_START( ac1prmcl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr535p7a", 0x0000, 0x2000, CRC(52167b25) SHA1(59081de590a72adbc826c16067920d145d5cf1ae) )
	ROM_LOAD( "pr535p7e", 0x2000, 0x2000, CRC(3cc34b9a) SHA1(597c75814593ec60cfb840d98dcd76df68815245) )
	ROM_LOAD( "pr535p7f", 0x4000, 0x2000, CRC(0346857f) SHA1(58cf5a9bcb2f4e51977c0c7ec732a5458f5dac5d) )
	ROM_LOAD( "pr535p7g", 0x6000, 0x2000, CRC(848888e3) SHA1(c7d1ed2153981cf45c68bb58b0220d0238edca49) )
ROM_END

ROM_START( ac1prmcla )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr535p7b", 0x0000, 0x2000, CRC(b656cd89) SHA1(a26cb18ad23d25d0d631244f3a7d32914ba08750) ) // alt rom at 0x0000
	ROM_LOAD( "pr535p7e", 0x2000, 0x2000, CRC(3cc34b9a) SHA1(597c75814593ec60cfb840d98dcd76df68815245) )
	ROM_LOAD( "pr535p7f", 0x4000, 0x2000, CRC(0346857f) SHA1(58cf5a9bcb2f4e51977c0c7ec732a5458f5dac5d) )
	ROM_LOAD( "pr535p7g", 0x6000, 0x2000, CRC(848888e3) SHA1(c7d1ed2153981cf45c68bb58b0220d0238edca49) )
ROM_END

ROM_START( ac1prmclb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr535p7c", 0x0000, 0x2000, CRC(c01f02ea) SHA1(e879c0a62229f8adb3ae389c07d27b3f568f87e3) ) // alt rom at 0x0000
	ROM_LOAD( "pr535p7e", 0x2000, 0x2000, CRC(3cc34b9a) SHA1(597c75814593ec60cfb840d98dcd76df68815245) )
	ROM_LOAD( "pr535p7f", 0x4000, 0x2000, CRC(0346857f) SHA1(58cf5a9bcb2f4e51977c0c7ec732a5458f5dac5d) )
	ROM_LOAD( "pr535p7g", 0x6000, 0x2000, CRC(848888e3) SHA1(c7d1ed2153981cf45c68bb58b0220d0238edca49) )
ROM_END

ROM_START( ac1prmclc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pr535p7d", 0x0000, 0x2000, CRC(245fb446) SHA1(6152e89df3707fca3e576f238420f9790ab47cff) ) // alt rom at 0x0000
	ROM_LOAD( "pr535p7e", 0x2000, 0x2000, CRC(3cc34b9a) SHA1(597c75814593ec60cfb840d98dcd76df68815245) )
	ROM_LOAD( "pr535p7f", 0x4000, 0x2000, CRC(0346857f) SHA1(58cf5a9bcb2f4e51977c0c7ec732a5458f5dac5d) )
	ROM_LOAD( "pr535p7g", 0x6000, 0x2000, CRC(848888e3) SHA1(c7d1ed2153981cf45c68bb58b0220d0238edca49) )
ROM_END


ROM_START( ac1rundx )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rd560d3b", 0x0000, 0x8000, CRC(e86b735d) SHA1(10ed7ffd60bfd8218007b8a2a5ebacbc5b241aaa) )
ROM_END

ROM_START( ac1rundxa )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rd560d3d", 0x0000, 0x8000, CRC(a8ec49ab) SHA1(503e7b0a5404f0ea06b95cdce372c7cb01b2a309) )
ROM_END

ROM_START( ac1totb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tbp3.1", 0x0000, 0x2000, CRC(abf58311) SHA1(26e2b9d8041f048050e23a8c1e6f6a7ac5660597) )
	ROM_LOAD( "tbp3.2", 0x2000, 0x2000, CRC(04d04f0a) SHA1(d6cdbc6306b956de47c599b1c2c3df529d372538) )
	ROM_LOAD( "tbp3.3", 0x4000, 0x2000, CRC(c5c669d3) SHA1(a54014f2f50fc7dda8cf7a8b40f9451b3fe317ae) )
	ROM_LOAD( "tbp3.4", 0x6000, 0x2000, CRC(f2d31ff4) SHA1(445d21ab0d413c23e0ef7bc00f940cdff5142cdd) )
ROM_END


ROM_START( ac1shid )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sh564p4a", 0x0000, 0x8000, CRC(6e6fe7d8) SHA1(7bd21eabcef5c5c74f6d20c9b43dd0e975bb958f) )
ROM_END

ROM_START( ac1shida )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sh551d4a", 0x0000, 0x010000, CRC(5cf109df) SHA1(dd52ed897417cf4eb7b0ba60c8f0a6692c5f5e76) ) // 1xxxxxxxxxxxxxxx = 0x00 - overdump?
ROM_END



ROM_START( ac1bluec )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bchip.p1", 0x0000, 0x2000, CRC(fe41206d) SHA1(41c9ecc26a26def5cf63209274f6b08e00de7889) )
	ROM_LOAD( "bchip.p2", 0x2000, 0x2000, CRC(c70dfaf1) SHA1(6628a7e66f7e22c3a45515c8be6ed88e0e19b10c) )
	ROM_LOAD( "bchip.p3", 0x4000, 0x2000, CRC(6649e6fb) SHA1(628594c924fc5029add5ddedd7c4f3aefbdbd0b6) )
	ROM_LOAD( "bchip.p4", 0x6000, 0x2000, CRC(5fb53b5a) SHA1(d6449d19ea440a51ce13564b34fbbab53a216a18) )
ROM_END

ROM_START( ac1blueca )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bluechiparcadeirelandp1.bin", 0x0000, 0x2000, CRC(dbeaf017) SHA1(e5020acdb4248a6373aca475ee2aab042860052c) )
	ROM_LOAD( "bluechiparcadeirelandp2.bin", 0x2000, 0x2000, CRC(c70dfaf1) SHA1(6628a7e66f7e22c3a45515c8be6ed88e0e19b10c) )
	ROM_LOAD( "bluechiparcadeirelandp3.bin", 0x4000, 0x2000, CRC(cd39fb8a) SHA1(b604dfe105b530431d4f5b7d6f839f6169b02c37) )
	ROM_LOAD( "bluechiparcadeirelandp4.bin", 0x6000, 0x2000, CRC(daa1e5bb) SHA1(1f629bc8f9015b73cc9b5b798a98f4479644ac8b) )
ROM_END

ROM_START( ac1bluecb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bluechiparcadestdp1.bin", 0x0000, 0x2000, CRC(dafa4a2b) SHA1(bd4aaaffb6980de5d6279a004c07a62e331e47f4) )
	ROM_LOAD( "bluechiparcadestdp2.bin", 0x2000, 0x2000, CRC(e18108f8) SHA1(4f94c8d35094ca5ae29e6ba61dd0697515316e21) )
	ROM_LOAD( "bluechiparcadestdp3.bin", 0x4000, 0x2000, CRC(bb6cc903) SHA1(cb8d87d60d9925338cd00cdf02cf324f6eaa077c) )
	ROM_LOAD( "bluechiparcadestdp4.bin", 0x6000, 0x2000, CRC(0acc6341) SHA1(106f924458fd0fdd40f40714ee97e96a89ef73e0) )
ROM_END

ROM_START( ac1bluecc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bluechipp1.bin", 0x0000, 0x2000, CRC(dbc8c9a8) SHA1(d17cc912c2eb5a46a14b27a6ad4b3ec155a9a653) )
	ROM_LOAD( "bluechipp2.bin", 0x2000, 0x2000, CRC(41a7dd01) SHA1(43fa39fde4cdb5c8d5aaaa8733e43cb925fb1c41) )
	ROM_LOAD( "bluechipp3.bin", 0x4000, 0x2000, CRC(cee5f9cc) SHA1(b38bfe0222554b743a7ae20ee74d029293c17424) )
	ROM_LOAD( "bluechipp4.bin", 0x6000, 0x2000, CRC(89278a76) SHA1(6effb53a0ff9ff04125b47bf12ceebe6347e080a) )
ROM_END

ROM_START( ac1bluecd )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bluechipcombined.bin", 0x0000, 0x8000, CRC(24af158c) SHA1(74bc19d4a4d6d34f35dfaf33c75b2e46d87e8ac5) )
ROM_END


ROM_START( ac1dbldx )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "double deluxe 14.2.90 p1.bin", 0x0000, 0x2000, CRC(d04288e1) SHA1(0101aa58b327322b192bef20ca698c0d3a9a02d0) )
	ROM_LOAD( "double deluxe 14.2.90 p2.bin", 0x2000, 0x2000, CRC(210607a1) SHA1(e08866fa18d4102a29a40c78e0e682232dcdaf19) )
	ROM_LOAD( "double deluxe 14.2.90 p3.bin", 0x4000, 0x2000, CRC(637e310a) SHA1(58801577b684d3fb2e928659871960a46d39aa78) )
	ROM_LOAD( "double deluxe 14.2.90 p4.bin", 0x6000, 0x2000, CRC(df027038) SHA1(ec9fda9dc53a628ea476f593f73cb42c8e227f05) )
ROM_END


ROM_START( ac1nudbk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "nbreak 10p 4.80 p1 jan 1990.bin", 0x0000, 0x2000, CRC(8438b94b) SHA1(5791069ddb986321ed1abe03e234c503c3c40d0c) )
	ROM_LOAD( "nbreak 10p 4.80 p2 jan 1990.bin", 0x2000, 0x2000, CRC(e9718ee6) SHA1(ad3a309cb0ed29f458187810a6606632a8019d70) )
	ROM_LOAD( "nbreak 10p 4.80 p3 jan 1990.bin", 0x4000, 0x2000, CRC(b2e6d735) SHA1(979f48e5d8503a517fd936fbcaad84804e4b52ab) )
	ROM_LOAD( "nbreak 10p 4.80 p4 jan 1990.bin", 0x6000, 0x2000, CRC(7c2e67dd) SHA1(0d2f15793e586a6a0e08e22b207ec37d64fb8402) )
ROM_END


ROM_START( ac1nudbka )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "nbreak 2p 2.40 p1 jan 1990.bin", 0x0000, 0x2000, CRC(ee1d3a85) SHA1(f26e07e975a8d640f03bb938ba4c8b4920b39fbb) )
	ROM_LOAD( "nbreak 2p 2.40 p2 jan 1990.bin", 0x2000, 0x2000, CRC(f297ce8b) SHA1(760bcdd6acedee7ea9e2847be5de045da96dd0c6) )
	ROM_LOAD( "nbreak 2p 2.40 p3 jan 1990.bin", 0x4000, 0x2000, CRC(a85092c6) SHA1(cd0080f3d6743c76f9f13cacd3b61a7f39b24908) )
	ROM_LOAD( "nbreak 2p 2.40 p4 jan 1990.bin", 0x6000, 0x2000, CRC(0bab299e) SHA1(b3ab09d94cd1da4e5b38a0f757579a8da2a2063d) )
ROM_END

ROM_START( ac1nudbkb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "nbreak 5p 2.40 p1 jan 1990.bin", 0x0000, 0x2000, CRC(6ea4df7f) SHA1(2dc221ede6d91c5e28988c08a37b7458b29c0994) )
	ROM_LOAD( "nbreak 5p 2.40 p2 jan 1990.bin", 0x2000, 0x2000, CRC(b58e988a) SHA1(bea5796a7d1e111aba0c9daefa5c9a9332d49490) )
	ROM_LOAD( "nbreak 5p 2.40 p3 jan 1990.bin", 0x4000, 0x2000, CRC(1e80969b) SHA1(7aa465ec52f5e8691c62bb82ad7d085bc132dcd4) )
	ROM_LOAD( "nbreak 5p 2.40 p4 jan 1990.bin", 0x6000, 0x2000, CRC(0bab299e) SHA1(b3ab09d94cd1da4e5b38a0f757579a8da2a2063d) )
ROM_END

ROM_START( ac1nudbkc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "nudgebreak 2p play 2 p1.02 22.11.89.bin", 0x0000, 0x2000, CRC(b712ec30) SHA1(f80f2946f3f9aca62392740d28cc8091e156223d) )
	ROM_LOAD( "nudgebreak 2p play 2 p2.02 22.11.89.bin", 0x2000, 0x2000, CRC(b473f132) SHA1(bbfe8b55edfe9c02b0c9f666dd3ccfd41e22ce16) )
	ROM_LOAD( "nudgebreak 2p play 2 p3.02 22.11.89.bin", 0x4000, 0x2000, CRC(83148533) SHA1(88604433a1d39c57e1105bab6320e9f93bf59c2e) )
	ROM_LOAD( "nudgebreak 2p play 2 p4.02 22.11.89.bin", 0x6000, 0x2000, CRC(34bd98fb) SHA1(40038c14a8f43f8bf78e8eab1cd2f309a0213d33) )
ROM_END

ROM_START( ac1nudbkd )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "nudgebreak 5p play 2 p1.bin",             0x0000, 0x2000, CRC(8409469d) SHA1(dbf5654b2bd112409084977523e7ac39e5676626) )
	ROM_LOAD( "nudgebreak 5p play 2 p2.01 22.11.89.bin", 0x2000, 0x2000, CRC(4ad98f24) SHA1(deb74daa3c42a1acdefd64c77887b1dafb801ccc) )
	ROM_LOAD( "nudgebreak 5p play 2 p3.01 22.11.89.bin", 0x4000, 0x2000, CRC(9ede1bed) SHA1(352cd4deb3ba0e59a315f54590c62d5f95786396) )
	ROM_LOAD( "nudgebreak 5p play 2 p4.01 22.11.89.bin", 0x6000, 0x2000, CRC(2528204b) SHA1(b77b04f74ddf38430c5c7f9a08230a705047164d) )
ROM_END


ROM_START( ac1sstrk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "starstruck 4.80 cash p1.bin", 0x0000, 0x2000, CRC(4b504e9b) SHA1(0d4de867b5373c944b9819479cb0aeb9786ceac9) )
	ROM_LOAD( "starstruck 4.80 cash p2.bin", 0x2000, 0x2000, CRC(b13fb64a) SHA1(e01ae32968d6c4c086c5f47ef4f525c10dfb3319) )
	ROM_LOAD( "starstruck 4.80 cash p3.bin", 0x4000, 0x2000, CRC(80ecc41e) SHA1(7f9494e8892ff7c36d092a56380fd0ad20724f55) )
	ROM_LOAD( "starstruck 4.80 cash p4.bin", 0x6000, 0x2000, CRC(0bc4979e) SHA1(a59bb6a841ed34cf1c23913ceaa4ede8c6973a2c) )
ROM_END

ROM_START( ac1sstrka )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "starstruck irish iom p1 3.5.90 4.bin", 0x0000, 0x2000, CRC(95127749) SHA1(0e806181149995ca30342d2dcdea106fc91c7f17) )
	ROM_LOAD( "starstruck irish iom p2 3.5.90 4.bin", 0x2000, 0x2000, CRC(182b71fe) SHA1(5bd9957b6d78bfdbad805ddaa35c089e393ab0c7) )
	ROM_LOAD( "starstruck irish iom p3 3.5.90 4.bin", 0x4000, 0x2000, CRC(0fd123a3) SHA1(5f2805822d5167f6e3d99fec87c9219892c8cc7d) )
	ROM_LOAD( "starstruck irish iom p4 3.5.90 4.bin", 0x6000, 0x2000, CRC(1469ff05) SHA1(2abc0afdc113f3bad57b685980754d6b8d546d28) )
ROM_END

ROM_START( ac1sstrkb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "starstruck p1 v2 23.4.90 std.bin", 0x0000, 0x2000, CRC(9ed618df) SHA1(5838c8006aaf7ca37c5ef97373c40f69ce1d54bb) )
	ROM_LOAD( "starstruck p2 v2 23.4.90 std.bin", 0x2000, 0x2000, CRC(f364a450) SHA1(35123f645396ec8ed0de64c0b0dbc55c0bea3db9) )
	ROM_LOAD( "starstruck p3 v2 23.4.90 std.bin", 0x4000, 0x2000, CRC(d07764b3) SHA1(18707df2330d3ba9793511fc06c3d79a9a40cce9) )
	ROM_LOAD( "starstruck p4 v2 23.4.90 std.bin", 0x6000, 0x2000, CRC(ba2533f0) SHA1(db9364c328d11955df4adde280c312e5eb9415ce) )
ROM_END


ROM_START( ac1xpres )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "xpressp1.bin", 0x0000, 0x2000, CRC(f5acbde7) SHA1(c88f5768c488663c23fd4b6a62ffeb7d07dbb317) )
	ROM_LOAD( "xpressp2.bin", 0x2000, 0x2000, CRC(dc4fa3a8) SHA1(bca83eb7cb7443206da13401f82347fcbee353ba) )
	ROM_LOAD( "xpressp3.bin", 0x4000, 0x2000, CRC(dca1495f) SHA1(ac8adbeb1461deb56377349dedcf10a853042dbf) )
	ROM_LOAD( "xpressp4.bin", 0x6000, 0x2000, CRC(31f654c1) SHA1(0ff6516cc60369f28555e81018865d3eb7e21ffd) )
ROM_END


ROM_START( ac1roll )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "roll up 10p 4.80 p1 26.1.90.bin", 0x0000, 0x2000, CRC(e985005a) SHA1(62a7e5afaf9d1d73073fc94e716f87bc4b9609c0) )
	ROM_LOAD( "roll up 10p 4.80 p2 26.1.90.bin", 0x2000, 0x2000, CRC(a4d4e9f6) SHA1(0f921e048772e1e04e20d25606621fe63fdacae5) )
	ROM_LOAD( "roll up 10p 4.80 p3 26.1.90.bin", 0x4000, 0x2000, CRC(daba0c2e) SHA1(44105352f901f8d97a15dcf90c9d9a50fda363e7) )
	ROM_LOAD( "roll up 10p 4.80 p4 26.1.90.bin", 0x6000, 0x2000, CRC(79093603) SHA1(30c811405951b44f3280326bb41880cb88cc2a4c) )
ROM_END

ROM_START( ac1rolla )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "roll up 2p 2.40 p1.2 23.1.90.bin", 0x0000, 0x2000, CRC(eae5cb06) SHA1(7b460e7a0a42e6adf327908acf906dfa94566d36) )
	ROM_LOAD( "roll up 2p 2.40 p2.2 23.1.90.bin", 0x2000, 0x2000, CRC(ad0086ed) SHA1(bdb5f42de8e30de84ec0482fb63dcbbd403804f2) )
	ROM_LOAD( "roll up 2p 2.40 p3.2 23.1.90.bin", 0x4000, 0x2000, CRC(72adf89b) SHA1(aedca79946e3f756ba8e14a89c4e868e649d9c45) )
	ROM_LOAD( "roll up 2p 2.40 p4.2 23.1.90.bin", 0x6000, 0x2000, CRC(22bd5c84) SHA1(f4d0739e6981bc96095a81661a9a4d2a863349fb) )
ROM_END

ROM_START( ac1rollb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "roll up 5p 2.40 p1 19.1.90.bin", 0x0000, 0x2000, CRC(72adf89b) SHA1(aedca79946e3f756ba8e14a89c4e868e649d9c45) )
	ROM_LOAD( "roll up 5p 2.40 p2 19.1.90.bin", 0x2000, 0x2000, CRC(4181ee7b) SHA1(cb5ad6a87bcbd387343c04a6b09a2b56bef539e5) )
	ROM_LOAD( "roll up 5p 2.40 p3 19.1.90.bin", 0x4000, 0x2000, CRC(225aaec7) SHA1(d444268935d260006fedff0079dfa0d08a3a69f3) )
	ROM_LOAD( "roll up 5p 2.40 p4 19.1.90.bin", 0x6000, 0x2000, CRC(8d2ddb6c) SHA1(4c0e8975c9b65fbacb519455e06a6cca9c3993c1) )
ROM_END

ROM_START( ac1hideh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh541d8a", 0x0000, 0x010000, CRC(be4d9ca9) SHA1(948471938f58e50be6e75c1a226947fc14f7eebf) ) // 2nd half blank
ROM_END

ROM_START( ac1hideha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh563p4r", 0x0000, 0x8000, CRC(c82aabb1) SHA1(6a94cbae10edc544117a6bc5849ac8c9ad80a333) )
ROM_END

ROM_START( ac1unk ) // System One, Race AGO182, (c) Race Electronics 1983
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "system_one_prom1_2764.bin", 0x0000, 0x2000, CRC(ea532fe4) SHA1(8b77d9e8fad0cd022c8386c509ad2ecbc7032d90) )
	ROM_LOAD( "system_one_prom2_2764.bin", 0x2000, 0x2000, CRC(729599b7) SHA1(da9aedc50a281cb6626a4c03fc06e5dd62b4edd1) )
	ROM_LOAD( "system_one_prom3_2764.bin", 0x4000, 0x2000, CRC(29b644e6) SHA1(fd7c82086a4812b26ff673f7734fe50a398aa063) )
	// 4th socket not populated.
ROM_END

void aces1_state::init_aces1()
{
}

} // anonymous namespace


GAMEL( 199?, ac1clbmn,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Club Money (Ace) (ACESYS1) (set 1)",          MACHINE_IS_SKELETON_MECHANICAL, layout_ac1clbmn )
GAMEL( 199?, ac1gogld,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Go For Gold (Ace) (ACESYS1) (set 1)",         MACHINE_IS_SKELETON_MECHANICAL, layout_ac1gogld )
GAME(  199?, ac1hotpf,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Hot Profit (Ace) (ACESYS1)",                  MACHINE_IS_SKELETON_MECHANICAL )
GAMEL( 199?, ac1pster,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Pound Sterling (Ace) (ACESYS1)",              MACHINE_IS_SKELETON_MECHANICAL, layout_ac1pster )
GAMEL( 199?, ac1pstrt,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Pound Stretcher (Pcp) (ACESYS1)",             MACHINE_IS_SKELETON_MECHANICAL, layout_ac1pstrt )
GAMEL( 199?, ac1primt,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Primetime (Ace) (ACESYS1) (set 1)",           MACHINE_IS_SKELETON_MECHANICAL, layout_ac1primt )
GAMEL( 199?, ac1taklv,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Take It Or Leave It (Ace) (ACESYS1) (set 1)", MACHINE_IS_SKELETON_MECHANICAL, layout_ac1taklv )
GAME(  199?, ac1cshtw,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Cash Towers (Ace) (ACESYS1)",                 MACHINE_IS_SKELETON_MECHANICAL ) // same ROM as above, combined, original machine apparently plays the same, reskinned machine?
GAME(  199?, ac1bbclb,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Big Break Club (Ace) (ACESYS1) (set 1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1bbclba, ac1bbclb, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Big Break Club (Ace) (ACESYS1) (set 2)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1clbsv,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Club Sovereign (Ace) (ACESYS1)",              MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1clbxt,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Club Xtra (Ace) (ACESYS1) (set 1)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1clbxta, ac1clbxt, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Club Xtra (Ace) (ACESYS1) (set 2)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1piaca,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Play It Again Casino (Ace) (ACESYS1)",        MACHINE_IS_SKELETON_MECHANICAL ) // Same ROMs were in 'Play It Again Deluxe'
GAME(  199?, ac1piacl,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Play It Again Club (Ace) (ACESYS1) (set 1)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1piacla, ac1piacl, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Play It Again Club (Ace) (ACESYS1) (set 2)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1piaclb, ac1piacl, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Play It Again Club (Ace) (ACESYS1) (set 3)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1piaclc, ac1piacl, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Play It Again Club (Ace) (ACESYS1) (set 4)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1prmcl,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Premier Club (Ace) (ACESYS1) (set 1)",        MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1prmcla, ac1prmcl, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Premier Club (Ace) (ACESYS1) (set 2)",        MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1prmclb, ac1prmcl, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Premier Club (Ace) (ACESYS1) (set 3)",        MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1prmclc, ac1prmcl, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Premier Club (Ace) (ACESYS1) (set 4)",        MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1rundx,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Runner Deluxe Club (Ace) (ACESYS1) (set 1)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1rundxa, ac1rundx, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Runner Deluxe Club (Ace) (ACESYS1) (set 2)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1totb,   0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Top Of The Bill (Ace) (ACESYS1)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1shid,   0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Super Hi De Hi (Ace) (ACESYS1) (set 1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1shida,  ac1shid , aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Super Hi De Hi (Ace) (ACESYS1) (set 2)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1dbldx,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Double Deluxe (Pcp) (ACESYS1)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1nudbk,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Nudge Break (Pcp) (ACESYS1) (set 1)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1nudbka, ac1nudbk, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Nudge Break (Pcp) (ACESYS1) (set 2)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1nudbkb, ac1nudbk, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Nudge Break (Pcp) (ACESYS1) (set 3)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1nudbkc, ac1nudbk, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Nudge Break (Pcp) (ACESYS1) (set 4)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1nudbkd, ac1nudbk, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Nudge Break (Pcp) (ACESYS1) (set 5)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1sstrk,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Starstruck (Pcp) (ACESYS1) (set 1)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1sstrka, ac1sstrk, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Starstruck (Pcp) (ACESYS1) (set 2)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1sstrkb, ac1sstrk, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Starstruck (Pcp) (ACESYS1) (set 3)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1xpres,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Xpress (Pcp) (ACESYS1)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1roll,   0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Roll Up (Pcp) (ACESYS1) (set 1)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1rolla,  ac1roll , aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Roll Up (Pcp) (ACESYS1) (set 2)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1rollb,  ac1roll , aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Roll Up (Pcp) (ACESYS1) (set 3)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1bluec,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Blue Chip (Pcp) (ACESYS1) (set 1)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1blueca, ac1bluec, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Blue Chip (Pcp) (ACESYS1) (set 2)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1bluecb, ac1bluec, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Blue Chip (Pcp) (ACESYS1) (set 3)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1bluecc, ac1bluec, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Blue Chip (Pcp) (ACESYS1) (set 4)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1bluecd, ac1bluec, aces1, aces1, aces1_state, init_aces1, ROT0, "Pcp", "Blue Chip (Pcp) (ACESYS1) (set 5)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(  199?, ac1hideh,  0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Hi De Hi Deluxe (Ace) (ACESYS1) (set 1)",     MACHINE_IS_SKELETON_MECHANICAL ) // was in Hi De Hi (Ace) (sp.ACE) set
GAME(  199?, ac1hideha, sp_hideh, aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "Hi De Hi Deluxe (Ace) (ACESYS1) (set 2)",     MACHINE_IS_SKELETON_MECHANICAL ) //  ^^
GAME(  199?, ac1unk,    0,        aces1, aces1, aces1_state, init_aces1, ROT0, "Ace", "unknown Ace System 1 game",                   MACHINE_IS_SKELETON_MECHANICAL )
