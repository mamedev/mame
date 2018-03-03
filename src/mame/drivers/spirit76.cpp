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

	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_READ8_MEMBER(unk_r);
	void spirit76(machine_config &config);
	void maincpu_map(address_map &map);
private:
	u8 m_t_c;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_START(spirit76_state::maincpu_map)
	ADDRESS_MAP_UNMAP_HIGH
//  ADDRESS_MAP_GLOBAL_MASK(0xfff) // this could most likely go in once the memory map is sorted
	AM_RANGE(0x0000, 0x00ff) AM_RAM // 2x 2112
	AM_RANGE(0x2200, 0x2203) AM_DEVREADWRITE("pia", pia6821_device, read, write) // 6820
	AM_RANGE(0x2400, 0x2400) AM_READ(unk_r)
	AM_RANGE(0x2401, 0x2401) AM_WRITE(unk_w)
	AM_RANGE(0x0600, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xfe00, 0xffff) AM_ROM AM_REGION("roms", 0x800)
ADDRESS_MAP_END


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

MACHINE_CONFIG_START(spirit76_state::spirit76)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 500000)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", spirit76_state, irq, attotime::from_hz(120))

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	//6821pia
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(spirit76_state, porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(spirit76_state, portb_w))
	MCFG_PIA_READPA_HANDLER(READ8(spirit76_state, porta_r))
	MCFG_PIA_READPB_HANDLER(READ8(spirit76_state, portb_r))
//  MCFG_PIA_CA2_HANDLER(WRITELINE(spirit76_state, pia22_ca2_w))
//  MCFG_PIA_CB2_HANDLER(WRITELINE(spirit76_state, pia22_cb2_w))
//  MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
//  MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))

	/* sound hardware */
	genpin_audio(config);
MACHINE_CONFIG_END


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


GAME( 1975, spirit76, 0, spirit76, spirit76, spirit76_state, 0, ROT0, "Mirco", "Spirit of 76", MACHINE_IS_SKELETON_MECHANICAL )
